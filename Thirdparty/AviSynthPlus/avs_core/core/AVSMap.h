#pragma once

#include <map>
#include <mutex>
#include <string>
#include "avisynth.h"
#include <atomic>
#include <vector>
#include <memory>

// Helper structures for frame properties. Borrowed from VapourSynth
// node-clip, VSVariant-FramePropVariant VSMap-AVSMap
// See also in Avisynth.cpp

// variant types
typedef std::shared_ptr<std::string> VSMapData;
typedef std::vector<int64_t> IntList;
typedef std::vector<double> FloatList;
typedef std::vector<VSMapData> DataList;
typedef std::vector<PClip> ClipList;
typedef std::vector<PVideoFrame> FrameList;
//typedef std::vector<PExtFunction> FuncList;

#define AVS_NOEXCEPT noexcept

class FramePropVariant {
public:
  enum FramePropVType { vUnset, vInt, vFloat, vData, vClip, vFrame/*, vMethod*/ };
  FramePropVariant(FramePropVType vtype = vUnset);
  FramePropVariant(const FramePropVariant& v);
  FramePropVariant(FramePropVariant&& v);
  ~FramePropVariant();

  size_t size() const;
  FramePropVType getType() const;

  void append(int64_t val);
  void append(double val);
  void append(const std::string& val);
  void append(const PClip& val);
  void append(const PVideoFrame& val);
  //void append(const PExtFunction& val); // not in avs+

  template<typename T>
  const T& getValue(size_t index) const {
    return reinterpret_cast<std::vector<T>*>(storage)->at(index);
  }

  template<typename T>
  const T* getArray() const {
    return reinterpret_cast<std::vector<T>*>(storage)->data();
  }

  template<typename T>
  void setArray(const T* val, size_t size) {
    assert(val && !storage);
    std::vector<T>* vect = new std::vector<T>(size);
    if (size)
      memcpy(vect->data(), val, size * sizeof(T));
    internalSize = size;
    storage = vect;
  }

private:
  FramePropVType vtype;
  size_t internalSize;
  void* storage;

  void initStorage(FramePropVType t);
};

class VSMapStorage {
private:
  std::atomic<int> refCount;
public:
  std::map<std::string, FramePropVariant> data;
  bool error;

  VSMapStorage() : refCount(1), error(false) {}

  VSMapStorage(const VSMapStorage& s) : refCount(1), data(s.data), error(s.error) {}

  bool unique() {
    return (refCount == 1);
  };

  void addRef() {
    ++refCount;
  }

  void release() {
    if (!--refCount)
      delete this;
  }
};



class AVSMap {
private:
  VSMapStorage* data;

  void detach() {
    if (!data->unique()) {
      VSMapStorage* old = data;
      data = new VSMapStorage(*data);
      old->release();
    }
  }
public:
  AVSMap() : data(new VSMapStorage()) {}

  AVSMap(const AVSMap& map) : data(map.data) {
    data->addRef();
  }

  AVSMap(AVSMap&& map) : data(map.data) {
    map.data = new VSMapStorage();
  }

  ~AVSMap() {
    data->release();
  }

  AVSMap& operator=(const AVSMap& map) {
    data->release();
    data = map.data;
    data->addRef();
    return *this;
  }

  bool contains(const std::string& key) const {
    return !!data->data.count(key);
  }

  FramePropVariant& at(const std::string& key) const {
    return data->data.at(key);
  }

  FramePropVariant& operator[](const std::string& key) const {
    // implicit creation is unwanted so make sure it doesn't happen by wrapping at() instead
    return data->data.at(key);
  }

  FramePropVariant* find(const std::string& key) const {
    auto it = data->data.find(key);
    return it == data->data.end() ? nullptr : &it->second;
  }

  bool erase(const std::string& key) {
    detach();
    return data->data.erase(key) > 0;
  }

  bool insert(const std::string& key, FramePropVariant&& v) {
    detach();
    data->data.erase(key);
    data->data.insert(std::make_pair(key, v));
    return true;
  }

  // make append safe like erase and insert
  // or else append from different threads to the same key (array) would be possible
  bool append(const std::string& key, int64_t i) {
    detach();
    (data->data.find(key)->second).append(i);
    return true;
  }

  bool append(const std::string& key, double d) {
    detach();
    (data->data.find(key)->second).append(d);
    return true;
  }

  bool append(const std::string& key, const std::string &s) {
    detach();
    (data->data.find(key)->second).append(s);
    return true;
  }

  bool append(const std::string& key,const  PClip& c) {
    detach();
    (data->data.find(key)->second).append(c);
    return true;
  }

  bool append(const std::string& key, const PVideoFrame& f) {
    detach();
    (data->data.find(key)->second).append(f);
    return true;
  }

  size_t size() const {
    return data->data.size();
  }

  void clear() {
    data->release();
    data = new VSMapStorage();
  }

  const char* key(int n) const {
    if (n >= static_cast<int>(size()))
      return nullptr;
    auto iter = data->data.cbegin();
    std::advance(iter, n);
    return iter->first.c_str();
  }

  const std::map<std::string, FramePropVariant>& getStorage() const {
    return data->data;
  }

  void setError(const std::string& errMsg) {
    clear();
    FramePropVariant v(FramePropVariant::vData);
    v.append(errMsg);
    insert("_Error", std::move(v));
    data->error = true;
  }

  bool hasError() const {
    return data->error;
  }

  const std::string& getErrorMessage() const {
    return *((*this)["_Error"].getValue<VSMapData>(0).get());
  }
};
