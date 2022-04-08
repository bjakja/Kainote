#ifndef f_SYLIA_SCRIPTVALUE_H
#define f_SYLIA_SCRIPTVALUE_H

// diff from original VDub: direct include
// Copied from vdtypes.h.  Must be in sync.
#ifndef VD_STANDARD_TYPES_DECLARED
#if defined(_MSC_VER)
typedef signed __int64		sint64;
typedef unsigned __int64	uint64;
#elif defined(__GNUC__)
typedef signed long long	sint64;
typedef unsigned long long	uint64;
#endif
typedef signed int			sint32;
typedef unsigned int		uint32;
typedef signed short		sint16;
typedef unsigned short		uint16;
typedef signed char			sint8;
typedef unsigned char		uint8;

typedef sint64				int64;
typedef sint32				int32;
typedef sint16				int16;
typedef sint8				int8;

typedef ptrdiff_t			sintptr;
typedef size_t				uintptr;
#endif


class VDScriptArray;
struct VDScriptObject;
class VDScriptValue;
class IVDScriptInterpreter;
class VariableTableEntry;

// Note: These objects must match the corresponding objects from the plugin interface.

typedef VDScriptValue (*VDScriptObjectLookupFuncPtr)(IVDScriptInterpreter *, const VDScriptObject *, void *lpVoid, char *szName);
typedef void (*VDScriptFunction)(IVDScriptInterpreter *, VDScriptValue *, int);

struct VDScriptFunctionDef {
	VDScriptFunction	func_ptr;
	const char *name;
	const char *arg_list;
};

struct VDScriptObjectDef {
	const char *name;
	const VDScriptObject *obj;
};

struct VDScriptObject {
	const char *mpName;
	VDScriptObjectLookupFuncPtr Lookup;
	const VDScriptFunctionDef		*func_list;
	const VDScriptObjectDef			*obj_list;
	const VDScriptObject		*pNextObject;
	const VDScriptFunctionDef	*prop_list;
};

class VDScriptValue {
public:
	enum { T_VOID, T_INT, T_PINT, T_STR, T_ARRAY, T_OBJECT, T_FNAME, T_FUNCTION, T_VARLV, T_LONG, T_DOUBLE } type;
	const VDScriptObject *thisPtr;
	union {
		int i;
		char **s;
		struct {
			const VDScriptObject *def;
			void *p;
		} obj;
		struct {
			const VDScriptFunctionDef *pfn;
			void *p;
		} method;
		VariableTableEntry *vte;
		sint64 l;
		double d;
	} u;

	VDScriptValue()						{ type = T_VOID; }
	explicit VDScriptValue(int i)				{ type = T_INT;			u.i = i; }
	explicit VDScriptValue(sint64 l)				{ type = T_LONG;		u.l = l; }
	explicit VDScriptValue(double d)				{ type = T_DOUBLE;		u.d = d; }
	explicit VDScriptValue(char **s)				{ type = T_STR;			u.s = s; }
	explicit VDScriptValue(void *p, const VDScriptObject *obj)	{ type = T_OBJECT;		u.obj.def = obj; u.obj.p = p; }
	explicit VDScriptValue(void *p, const VDScriptObject *obj, const VDScriptFunctionDef *pf)	{ type = T_FNAME;		thisPtr = obj; u.method.pfn = pf; u.obj.p = p; }
	explicit VDScriptValue(VariableTableEntry *vte) { type = T_VARLV;		u.vte = vte; }

	VDScriptValue& operator=(int i) { type = T_INT; u.i = i; return *this; }
	VDScriptValue& operator=(sint64 l) { type = T_LONG; u.l = l; return *this; }
	VDScriptValue& operator=(double d) { type = T_DOUBLE; u.d = d; return *this; }

	bool isVoid() const			{ return type == T_VOID; }
	bool isInt() const			{ return type == T_INT; }
	bool isLong() const			{ return type == T_LONG; }
	bool isDouble() const		{ return type == T_DOUBLE; }
	bool isString() const		{ return type == T_STR; }
	bool isObject() const		{ return type == T_OBJECT; }
	bool isVarLV() const		{ return type == T_VARLV; }
	bool isMethod() const		{ return type == T_FNAME; }

	int						asInt() const			{ return u.i; }
	sint64					asLong() const			{ return u.l; }
	double					asDouble() const		{ return u.d; }
	char **					asString() const		{ return u.s; }
	const VDScriptObject *	asObjectDef() const		{ return u.obj.def; }
	void *					asObjectPtr() const		{ return u.obj.p; }
	VariableTableEntry*		asVarLV() const		{ return u.vte; }
};

#endif
