#ifndef AUTOTOFILE
#define AUTOTOFILE

extern "C" {
#include <lua.hpp>
}

#include "SubsDialogue.h"
#include "SubsFile.h"
#include <wx/wx.h>



	



namespace Auto{

	class SubsEntry{
	public:
		Dialogue *adial;
		Styles *astyle;
		SInfo *info;
		wxString lclass;
		SubsEntry();
		~SubsEntry();
	};

	class AutoToFile{

	public:
		AutoToFile(lua_State *_L, File *subsfile, bool _can_modify);
		~AutoToFile();
		static bool LineToLua(lua_State *L, int i); 
		static SubsEntry *LuaToLine(lua_State *L);
		void Cancel();
	
	private:
		File *file;
		lua_State *L;

		bool can_modify;
		void CheckAllowModify(); // throws an error if modification is disallowed

			// keep a cursor of last accessed item to avoid walking over the entire file on every access
		void InitScriptInfoIfNeeded();

		static int ObjectIndexRead(lua_State *L);
		static int ObjectIndexWrite(lua_State *L);
		static int ObjectGetLen(lua_State *L);
		static int ObjectDelete(lua_State *L);
		static int ObjectDeleteRange(lua_State *L);
		static int ObjectAppend(lua_State *L);
		static int ObjectInsert(lua_State *L);
		static int ObjectLens(lua_State *L);
		static int ObjectGarbageCollect(lua_State *L){return 0;};
		static int ObjectIPairs(lua_State *L);
		static int IterNext(lua_State *L);

		static int LuaParseKaraokeData(lua_State *L);
		static int LuaGetScriptResolution(lua_State *L);
		static int LuaSetUndoPoint(lua_State *L) {return 0;};
		

		static AutoToFile *laf;//GetObjPointer(lua_State *L, int idx);

	};

};

#endif