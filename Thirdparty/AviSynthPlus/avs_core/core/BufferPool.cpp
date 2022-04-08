#include "BufferPool.h"
#include <map>
#include <avisynth.h>
#include <avs/alignment.h>
#include <avs/minmax.h>
#include "InternalEnvironment.h"

#define BUFFER_GUARD_VALUE  0x55555555

struct BufferPool::BufferDesc
{
  void* ptr;
  size_t size;
  size_t alignment;
  bool in_use;
};


static inline void CheckGuards(void* ptr)
{
  #ifndef NDEBUG
  size_t lower_guard = (size_t)(((void**)ptr)[-5]);
  assert(lower_guard == BUFFER_GUARD_VALUE);
  size_t upper_guard = (size_t)(((void**)ptr)[-1]);
  assert(upper_guard == BUFFER_GUARD_VALUE);
  #endif
}

static inline void* GetUserData(void* ptr)
{
  return ((void**)ptr)[-4];
}

static inline size_t GetRealSize(void* ptr)
{
  return (size_t)(((void**)ptr)[-3]);
}

static inline void* GetRealPtr(void* ptr)
{
  return ((void**)ptr)[-2];
}

void* BufferPool::PrivateAlloc(size_t nBytes, size_t alignment, void* user)
{
  /* Number of extra data fields to allocate.
    * Current field assignment:
    *  [-1] = upper guard (size_t)
    *  [-2] = original buffer pointer (void*)
    *  [-3] = original buffer size (size_t)
    *  [-4] = user data (void*)
    *  [-5] = lower guard (size_t)
    */
  const int NUM_EXTRA_FIELDS = 5;

  alignment = max(alignment, sizeof(void*));
  if (!IS_POWER2(alignment))
    return NULL;

  size_t offset = NUM_EXTRA_FIELDS * sizeof(void*) + alignment - 1;
  nBytes += offset;

  void *orig = malloc(nBytes);
  if (orig == NULL)
    return NULL;

  void **aligned = (void**)(((uintptr_t)orig + (uintptr_t)offset) & (~(uintptr_t)(alignment-1)));
  aligned[-5] = (void*)BUFFER_GUARD_VALUE;
  aligned[-4] = user;
  aligned[-3] = (void*)nBytes;
  aligned[-2] = orig;
  aligned[-1] = (void*)BUFFER_GUARD_VALUE;

  Env->AdjustMemoryConsumption(nBytes, false);
  return aligned;
}

void BufferPool::PrivateFree(void* buffer)
{
  CheckGuards(buffer);
  Env->AdjustMemoryConsumption(GetRealSize(buffer), true);
  free(GetRealPtr(buffer));
}


BufferPool::BufferPool(InternalEnvironment* env) :
  Env(env)
{
}

BufferPool::~BufferPool()
{
  const MapType::iterator end_it = Map.end();
  for (
    MapType::iterator it = Map.begin();
    it != end_it;
    ++it)
  {
      BufferDesc* desc = it->second;
      PrivateFree(desc->ptr);
      delete desc;
  }
}

void* BufferPool::Allocate(size_t nBytes, size_t alignment, bool pool)
{
  if (pool)
  {
    // First, check if we can return a buffer from the pool
    const MapType::iterator end_it = Map.end();
    for (
      MapType::iterator it = Map.lower_bound(nBytes);
      it != end_it;
      ++it)
    {
      BufferDesc* desc = it->second;
      if ( !desc->in_use
        && (desc->alignment >= alignment)
      )
      {
        desc->in_use = true;
        return desc->ptr;
      }
    }

    // None found, allocate new one
    BufferDesc* desc = new BufferDesc();

    void* ptr = PrivateAlloc(nBytes, alignment, reinterpret_cast<void*>(desc));
    if (ptr == NULL)
    {
      delete desc;
      return NULL;
    }

    desc->alignment = alignment;
    desc->in_use = true;
    desc->ptr = ptr;
    desc->size = nBytes;
    Map.emplace(nBytes, desc);
    return ptr;
  }
  else
  {
    return PrivateAlloc(nBytes, alignment, NULL);
  }
}

void BufferPool::Free(void* ptr)
{
  // Mirroring free()'s semantic requires us to accept NULLs
  if (ptr == NULL)
    return;

  CheckGuards(ptr);

  BufferDesc* data = reinterpret_cast<BufferDesc*>(GetUserData(ptr));

  if (data != NULL)
  { // Getting into this branch means this buffer is pooled
    data->in_use = false;
  }
  else
  {
    PrivateFree(ptr);
  }
}
