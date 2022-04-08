#pragma once

#include <avisynth.h>

struct Function {

  typedef AVSValue(__cdecl *apply_func_t)(AVSValue args, void* user_data, IScriptEnvironment* env);

  apply_func_t apply;
  const char* name;
  const char* canon_name;
  const char* param_types;
  void* user_data;
  const char* dll_path;
  bool isAvs25; // can be filled during plugin load
};

struct CaptureVars {
  int count;
  const char** var_names;
  const AVSValue* var_data;
};

class IFunction {
public:
  IFunction() : refcnt(0) {}
  virtual ~IFunction() { }
  virtual const char* ToString(IScriptEnvironment* env) = 0;
  virtual const char* GetLegacyName() = 0;
  virtual const Function* GetDefinition() = 0;
  virtual CaptureVars GetCaptures() = 0;

private:
  friend class PFunction;
  friend class AVSValue;
  unsigned long refcnt;
  void AddRef();
  void Release();
};


class AVSFunction : public Function {

public:

  typedef Function::apply_func_t apply_func_t;

  AVSFunction(void*);
  AVSFunction(const char* _name, const char* _plugin_basename, const char* _param_types, apply_func_t _apply);
  AVSFunction(const char* _name, const char* _plugin_basename, const char* _param_types, apply_func_t _apply, void *_user_data);
  AVSFunction(const char* _name, const char* _plugin_basename, const char* _param_types, apply_func_t _apply, void *_user_data, const char* _dll_path, bool _isAvs25);
  ~AVSFunction();

  AVSFunction() = delete;
  AVSFunction(const AVSFunction&) = delete;
  AVSFunction& operator=(const AVSFunction&) = delete;
  AVSFunction(AVSFunction&&) = delete;
  AVSFunction& operator=(AVSFunction&&) = delete;

  bool empty() const;

  static bool IsScriptFunction(const Function* func);
  static bool ArgNameMatch(const char* param_types, size_t args_names_count, const char* const* arg_names);
  static bool TypeMatch(const char* param_types, const AVSValue* args, size_t num_args, bool strict, IScriptEnvironment* env);
  static bool SingleTypeMatch(char type, const AVSValue& arg, bool strict);
  static bool SingleTypeMatchArray(char type, const AVSValue& arg, bool strict);
};
