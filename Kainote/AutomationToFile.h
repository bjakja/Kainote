// Copyright (c) 2006, 2007, Niels Martin Hansen
// Copyright (c) 2016 - 2020, Marcin Drob
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of the Aegisub Group nor the names of its contributors
//     may be used to endorse or promote products derived from this software
//     without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// Aegisub Project http://www.aegisub.org/

#pragma once

extern "C" {
#include <lua.hpp>
}

#include "SubsDialogue.h"
#include "SubsFile.h"

class AudioSpectrum;



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
	AutoToFile(lua_State *_L, File *subsfile, bool _can_modify, char subsFormat);
	~AutoToFile();
	static bool LineToLua(lua_State *L, int i); 
	static SubsEntry *LuaToLine(lua_State *L);
	void Cancel();
	
private:
	File *file;
	AudioSpectrum *spectrum = nullptr;
	lua_State *L;

	bool can_modify;
	char subsFormat = ASS;
	void CheckAllowModify(); // throws an error if modification is disallowed

		// keep a cursor of last accessed item to avoid walking over the entire file on every access
	//void InitScriptInfoIfNeeded();

	static int ObjectIndexRead(lua_State *L);
	static int ObjectIndexWrite(lua_State *L);
	static int ObjectGetLen(lua_State *L);
	static int ObjectDelete(lua_State *L);
	static int ObjectDeleteRange(lua_State *L);
	static int ObjectAppend(lua_State *L);
	static int ObjectInsert(lua_State *L);
	static int ObjectLens(lua_State *L);
	static int ObjectGarbageCollect(lua_State *L);
	static int ObjectIPairs(lua_State *L);
	static int IterNext(lua_State *L);

	static int LuaParseKaraokeData(lua_State *L);
	static int LuaGetScriptResolution(lua_State *L);
	static int LuaSetUndoPoint(lua_State *L) {return 0;};
	//static int LuaGenerateFFT(lua_State *L);
	static int LuaGetFreqencyReach(lua_State *L);
	static File* GetSubs(lua_State* L);

	static AutoToFile *laf;
};



