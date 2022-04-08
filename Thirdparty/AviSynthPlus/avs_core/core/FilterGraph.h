#ifndef _AVS_FILTER_GRAPH_H
#define _AVS_FILTER_GRAPH_H

#include "internal.h"
#include <vector>
#include <map>
#include <memory>
#include <mutex>

class FilterGraph;

class Device;
// no DeviceManager classic avs+

class GraphMemoryNode {
public:
  struct MemoryInfo {
    int numAllocation;
    size_t totalBytes;
  };
  std::map<Device*, MemoryInfo> memory;
  void OnAllocate(size_t bytes, Device* dev);
  void OnFree(size_t bytes, Device* dev);
private:
  friend class PGraphMemoryNode;
  int refcnt;
  void AddRef() { ++refcnt; }
  void Release() { if (--refcnt <= 0) delete this; }
};

class PGraphMemoryNode
{
public:
  PGraphMemoryNode() { Init(0); }
  PGraphMemoryNode(GraphMemoryNode* p) { Init(p); }
  PGraphMemoryNode(const PGraphMemoryNode& p) { Init(p.e); }
  PGraphMemoryNode& operator=(GraphMemoryNode* p) { Set(p); return *this; }
  PGraphMemoryNode& operator=(const PGraphMemoryNode& p) { Set(p.e); return *this; }
  int operator!() const { return !e; }
  operator void*() const { return e; }
  GraphMemoryNode* operator->() const { return e; }
  ~PGraphMemoryNode() { Release(); }

private:
  GraphMemoryNode* e;
  void Init(GraphMemoryNode* p) { e = p; if (e) e->AddRef(); }
  void Set(GraphMemoryNode* p) { if (p) p->AddRef(); if (e) e->Release(); e = p; }
  void Release() { if (e) e->Release(); }
};

class FilterGraphNode : public IClip
{
	IScriptEnvironment* Env;
  PClip child;

  std::string name;
  AVSValue args;
  std::vector<std::unique_ptr<AVSValue[]>> arrays;
  std::vector<std::string> argnames;

  PGraphMemoryNode memory;

  friend FilterGraph;
public:
  FilterGraphNode(PClip child, const char* name, const AVSValue& last,
		const AVSValue& args, const char* const* arg_names, IScriptEnvironment* env);
	~FilterGraphNode();

  virtual int __stdcall GetVersion() { return child->GetVersion(); }
  virtual PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);
  virtual bool __stdcall GetParity(int n) { return child->GetParity(n); }
  virtual void __stdcall GetAudio(void* buf, int64_t start, int64_t count, IScriptEnvironment* env)
  {
    return child->GetAudio(buf, start, count, env);
  }
  virtual int __stdcall SetCacheHints(int cachehints, int frame_range)
  {
    return child->SetCacheHints(cachehints, frame_range);
  }
  virtual const VideoInfo& __stdcall GetVideoInfo() { return child->GetVideoInfo(); }

  PGraphMemoryNode GetMemoryNode() { return memory; }
};

void DoDumpGraph(const std::vector<FilterGraphNode*>& roots, const char* path, IScriptEnvironment* env);

#endif  // _AVS_FILTER_GRAPH_H
