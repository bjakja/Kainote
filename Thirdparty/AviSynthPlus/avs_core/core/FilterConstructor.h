#ifndef _AVS_FILTER_CONSTRUCTOR_H
#define _AVS_FILTER_CONSTRUCTOR_H

#include "internal.h"
#include <vector>

class FilterConstructor
{
private:
  IScriptEnvironment2* const Env;
  IScriptEnvironment_Avs25* const Env25;
  const Function* const Func;
  const std::vector<AVSValue> ArgStorage;
  const std::vector<AVSValue> CtorArgs;

public:
  FilterConstructor(IScriptEnvironment2 * env, IScriptEnvironment_Avs25* env25, const Function *func, std::vector<AVSValue>* argStorage, std::vector<AVSValue>* ctorArgs);
  AVSValue InstantiateFilter() const;

  const char* GetFilterName() const
  {
    return Func->name;
  }

  bool IsScriptFunction() const
  {
    return AVSFunction::IsScriptFunction(Func);
  }

  const Function* GetAvsFunction() const
  {
      return Func;
  }
};

#endif  // _AVS_FILTER_CONSTRUCTOR_H
