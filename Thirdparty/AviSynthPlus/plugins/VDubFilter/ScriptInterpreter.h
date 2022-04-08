#ifndef f_SYLIA_SCRIPTINTERPRETER_H
#define f_SYLIA_SCRIPTINTERPRETER_H

class VDScriptValue;
class VDScriptError;
struct VDScriptObject;
class IVDScriptInterpreter;
struct VDScriptFunctionDef;

typedef VDScriptValue (*VDScriptRootHandlerPtr)(IVDScriptInterpreter *,char *,void *);

class IVDScriptInterpreter {
public:
	virtual	~IVDScriptInterpreter() {}

	virtual void __cdecl SetRootHandler(VDScriptRootHandlerPtr, void *)	=0;

	virtual void __cdecl ExecuteLine(const char *s)						=0;

	virtual void __cdecl ScriptError(int e)								=0;
	virtual const char* __cdecl TranslateScriptError(const VDScriptError& cse)	=0;
	virtual char** __cdecl AllocTempString(long l)						=0;

	virtual VDScriptValue __cdecl LookupObjectMember(const VDScriptObject *obj, void *, char *szIdent) = 0;

	virtual const VDScriptFunctionDef * __cdecl GetCurrentMethod() = 0;
	virtual int __cdecl GetErrorLocation() = 0;
	virtual VDScriptValue	__cdecl DupCString(const char *) = 0;
};

IVDScriptInterpreter *VDCreateScriptInterpreter();

#define VDSCRIPT_EXT_ERROR(x)	if(true){isi->ScriptError(VDScriptError::x); VDNEVERHERE;}else

#endif
