#include "ThreadPool.h"
#include "internal.h"
#include <cassert>
#include <thread>

struct ThreadPoolGenericItemData
{
  ThreadWorkerFuncPtr Func;
  void* Params;
  AVSPromise* Promise;
  Device* device;
};

#include "mpmc_bounded_queue.h"
typedef mpmc_bounded_queue<ThreadPoolGenericItemData> MessageQueue;

class ThreadPoolPimpl
{
public:
  std::vector<std::thread> Threads;
  MessageQueue MsgQueue;
  std::mutex Mutex;
  std::condition_variable FinishCond;
  size_t NumRunning;

  ThreadPoolPimpl(size_t nThreads) :
    Threads(),
    MsgQueue(nThreads * 6)
  {}
};

void ThreadPool::ThreadFunc(size_t thread_id, ThreadPoolPimpl * const _pimpl, InternalEnvironment* env)
{
  auto EnvTLS = env->NewThreadScriptEnvironment((int)thread_id);
  PInternalEnvironment holder = PInternalEnvironment(EnvTLS);

  while (true)
  {
    ThreadPoolGenericItemData data;
    if (_pimpl->MsgQueue.pop_back(&data) == false) {
      // threadpool is canceled
      std::unique_lock<std::mutex> lock(_pimpl->Mutex);
      if (--_pimpl->NumRunning == 0) {
        _pimpl->FinishCond.notify_all();
      }
      return;
    }

    EnvTLS->SetCurrentDevice(data.device);
    EnvTLS->GetSupressCaching() = false;
    if (data.Promise != NULL)
    {
      try
      {
        data.Promise->set_value(data.Func(EnvTLS, data.Params));
      }
      catch (const AvisynthError&)
      {
        data.Promise->set_exception(std::current_exception());
      }
      catch (const std::exception&)
      {
        data.Promise->set_exception(std::current_exception());
      }
      catch (...)
      {
        data.Promise->set_exception(std::current_exception());
        //data.Promise->set_value(AVSValue("An unknown exception was thrown in the thread pool."));
      }
    }
    else
    {
      try
      {
        data.Func(EnvTLS, data.Params);
      }
      catch (...) {}
    }
  } //while
}

ThreadPool::ThreadPool(size_t nThreads, size_t nStartId, InternalEnvironment* env) :
  _pimpl(new ThreadPoolPimpl(nThreads))
{
  _pimpl->Threads.reserve(nThreads);

  std::unique_lock<std::mutex> lock(_pimpl->Mutex);

  // i is used as the thread id. Skip id zero because that is reserved for the main thread.
  // CUDA: thread id is controled by caller
  for (size_t i = 0; i < nThreads; ++i)
    _pimpl->Threads.emplace_back(ThreadFunc, i + nStartId, _pimpl, env);

  _pimpl->NumRunning = nThreads;
}

void ThreadPool::QueueJob(ThreadWorkerFuncPtr clb, void* params, InternalEnvironment* env, JobCompletion* tc)
{
  ThreadPoolGenericItemData itemData;
  itemData.Func = clb;
  itemData.Params = params;
  itemData.device = env->GetCurrentDevice();

  if (tc != NULL)
    itemData.Promise = tc->Add();
  else
    itemData.Promise = NULL;

  if (_pimpl->MsgQueue.push_front(std::move(itemData)) == false) {
    throw AvisynthError("Threadpool is cancelled");
  }
}

size_t ThreadPool::NumThreads() const
{
  return _pimpl->Threads.size();
}

std::vector<void*> ThreadPool::Finish()
{
  std::unique_lock<std::mutex> lock(_pimpl->Mutex);
  if (_pimpl->NumRunning > 0) {
    _pimpl->MsgQueue.finish();
    while (_pimpl->NumRunning > 0)
    {
      _pimpl->FinishCond.wait(lock);
    }
    std::vector<void*> ret;
    ThreadPoolGenericItemData item;
    while (_pimpl->MsgQueue.pop_remain(&item)) {
      ret.push_back(item.Params);
    }
    return ret;
  }
  return std::vector<void*>();
}

void ThreadPool::Join()
{
  if (_pimpl->Threads.size() > 0) {
    Finish();
    for (size_t i = 0; i < _pimpl->Threads.size(); ++i)
    {
      if (_pimpl->Threads[i].joinable())
        _pimpl->Threads[i].join();
    }
    _pimpl->Threads.clear();
  }
}

ThreadPool::~ThreadPool()
{
  Finish();
  Join();
  delete _pimpl;
}
