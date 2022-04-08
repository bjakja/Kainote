#pragma once

#include <avisynth.h>
#include <vector>
#include <atomic>
#include <memory>

class InternalEnvironment;
enum DeviceOpt: int; // forward enum w/o underlying types are MS specific

struct DeviceCompleteCallbackData {
  void(*cb)(void*);
  void* user_data;
};

class Device {
protected:
    InternalEnvironment* env;

public:
    const AvsDeviceType device_type;
    const int device_id;
    const int device_index;

    uint64_t memory_max;
    std::atomic<uint64_t> memory_used;

    int  free_thresh;

    Device(AvsDeviceType type, int id, int index, InternalEnvironment* env) :
      env(env),
      device_type(type),
      device_id(id),
      device_index(index),
      memory_max(0),
      memory_used(0),
      free_thresh(0)
    { }

    virtual ~Device() { }

    virtual int SetMemoryMax(int mem) = 0;
    virtual BYTE* Allocate(size_t sz, int margin) = 0;
    virtual void Free(BYTE* ptr) = 0;
    virtual const char* GetName() const = 0;
    virtual void AddCompleteCallback(DeviceCompleteCallbackData cbdata) = 0;
    virtual std::unique_ptr<std::vector<DeviceCompleteCallbackData>> GetAndClearCallbacks() = 0;
    virtual void SetActiveToCurrentThread(InternalEnvironment* env) = 0;
    virtual void* GetComputeStream() = 0;
    virtual void SetDeviceOpt(DeviceOpt opt, int val, InternalEnvironment* env) = 0;
    virtual void GetAlignmentRequirement(int* memoryAlignment, int* pitchAlignment) = 0;
};

class DeviceManager {
private:
  InternalEnvironment *env;
  std::unique_ptr<Device> cpuDevice;
  std::vector<std::unique_ptr<Device>> cudaDevices;
  int numDevices;

public:
    DeviceManager(InternalEnvironment* env);
    ~DeviceManager() { }

    Device* GetDevice(AvsDeviceType device_type, int device_index) const;

    Device* GetCPUDevice() { return GetDevice(DEV_TYPE_CPU, 0); }

    int GetNumDevices() const { return numDevices; }
    int GetNumDevices(AvsDeviceType device_type) const;

    void SetDeviceOpt(DeviceOpt opt, int val, InternalEnvironment* env);
};

class DeviceSetter {
  InternalEnvironment* env;
  Device* downstreamDevice;
public:
  DeviceSetter(InternalEnvironment* env, Device* upstreamDevice);
  ~DeviceSetter();
};

void CheckChildDeviceTypes(const PClip& child, const char* name, const AVSValue& last,
  const AVSValue& args, const char* const* argnames, InternalEnvironment* env);

void CopyCUDAFrame(const PVideoFrame& dst, const PVideoFrame& src, InternalEnvironment* env, bool sync = false);
