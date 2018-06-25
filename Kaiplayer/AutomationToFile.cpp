
// Copyright (c) 2006, 2007, Niels Martin Hansen
// Copyright (c) 2016, Marcin Drob
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

#include "AutomationToFile.h"
#include "AutomationUtils.h"
#include "KainoteApp.h"
#include "AudioSpectrum.h"
#include <wx/regex.h>

//template<int (AutoToFile::*closure)(lua_State *)>
//	int closure_wrapper(lua_State *L)
//	{
//		return (AutoToFile::GetObjPointer(L, lua_upvalueindex(1), false)->*closure)(L);
//	}

namespace Auto{

	SubsEntry::SubsEntry()
	{
		adial=NULL;
		astyle=NULL;
		info= NULL;
	}
	SubsEntry::~SubsEntry()
	{
		SAFE_DELETE(adial);
		SAFE_DELETE(astyle);
		SAFE_DELETE(info);
	}

	AutoToFile *AutoToFile::laf = NULL;

	AutoToFile::~AutoToFile()
	{
		if (spectrum){
			delete spectrum;
			spectrum = NULL;
		}
	}

	void AutoToFile::CheckAllowModify()
	{
		if (can_modify)
			return;
		lua_pushstring(L, "You cannot modify read-only subtitles");
		lua_error(L);
	}




	bool AutoToFile::LineToLua(lua_State *L, int i)
	{
		//AutoToFile *laf = GetObjPointer(L, 1);
		File *Subs=laf->file;

		int sinfo=Subs->sinfo.size();
		int styles=sinfo+Subs->styles.size();
		int dials=styles+Subs->dials.size();
		if(i<0||i>=dials){
			return false;
		}

		lua_newtable(L);
		if(i<sinfo){
			//to jest odczyt wiêc nie kopiujemy
			SInfo *info=Subs->sinfo[i];

			lua_pushstring(L, "[Script Info]");
			lua_setfield(L, -2, "section");

			wxString raw=info->Name+": "+info->Val;
			lua_pushstring(L, raw.mb_str(wxConvUTF8).data());
			lua_setfield(L, -2, "raw");

			lua_pushstring(L, info->Name.mb_str(wxConvUTF8).data());
			lua_setfield(L, -2, "key");

			// then "value"
			lua_pushstring(L, info->Val.mb_str(wxConvUTF8).data());
			lua_setfield(L, -2, "value");

			lua_pushstring(L, "info");
		}
		else if(i<styles)
		{
			//to jest odczyt wiêc nie kopiujemy
			Styles *astyle=Subs->styles[i-sinfo];

			lua_pushstring(L, "[V4+ Styles]");
			lua_setfield(L, -2, "section");

			wxString raw=astyle->styletext();
			lua_pushstring(L, raw.mb_str(wxConvUTF8).data());
			lua_setfield(L, -2, "raw");

			lua_pushstring(L, astyle->Name.mb_str(wxConvUTF8).data());
			lua_setfield(L, -2, "name");

			lua_pushstring(L, astyle->Fontname.mb_str(wxConvUTF8).data());
			lua_setfield(L, -2, "fontname");
			lua_pushnumber(L, wxAtoi(astyle->Fontsize));
			lua_setfield(L, -2, "fontsize");

			lua_pushstring(L, astyle->PrimaryColour.GetAss(true).mb_str(wxConvUTF8).data());
			lua_setfield(L, -2, "color1");
			lua_pushstring(L, astyle->SecondaryColour.GetAss(true).mb_str(wxConvUTF8).data());
			lua_setfield(L, -2, "color2");
			lua_pushstring(L, astyle->OutlineColour.GetAss(true).mb_str(wxConvUTF8).data());
			lua_setfield(L, -2, "color3");
			lua_pushstring(L, astyle->BackColour.GetAss(true).mb_str(wxConvUTF8).data());
			lua_setfield(L, -2, "color4");

			lua_pushboolean(L, (int)astyle->Bold);
			lua_setfield(L, -2, "bold");
			lua_pushboolean(L, (int)astyle->Italic);
			lua_setfield(L, -2, "italic");
			lua_pushboolean(L, (int)astyle->Underline);
			lua_setfield(L, -2, "underline");
			lua_pushboolean(L, (int)astyle->StrikeOut);
			lua_setfield(L, -2, "strikeout");

			lua_pushnumber(L, wxAtoi(astyle->ScaleX));
			lua_setfield(L, -2, "scale_x");
			lua_pushnumber(L, wxAtoi(astyle->ScaleY));
			lua_setfield(L, -2, "scale_y");

			double fl=0.0;
			astyle->Spacing.ToDouble(&fl);
			lua_pushnumber(L, fl);
			lua_setfield(L, -2, "spacing");
			astyle->Angle.ToDouble(&fl);
			lua_pushnumber(L, fl);
			lua_setfield(L, -2, "angle");

			lua_pushnumber(L, astyle->BorderStyle);
			lua_setfield(L, -2, "borderstyle");
			astyle->Outline.ToDouble(&fl);
			lua_pushnumber(L, fl);
			lua_setfield(L, -2, "outline");
			astyle->Shadow.ToDouble(&fl);
			lua_pushnumber(L, fl);
			lua_setfield(L, -2, "shadow");

			lua_pushnumber(L, wxAtoi(astyle->Alignment));
			lua_setfield(L, -2, "align");

			lua_pushnumber(L, wxAtoi(astyle->MarginL));
			lua_setfield(L, -2, "margin_l");
			lua_pushnumber(L, wxAtoi(astyle->MarginR));
			lua_setfield(L, -2, "margin_r");
			lua_pushnumber(L, wxAtoi(astyle->MarginV));
			lua_setfield(L, -2, "margin_t");
			lua_pushnumber(L, wxAtoi(astyle->MarginV));
			lua_setfield(L, -2, "margin_b");

			lua_pushnumber(L, wxAtoi(astyle->Encoding));
			lua_setfield(L, -2, "encoding");

			set_field(L, "relative_to", 2);

			lua_pushstring(L, "style");

		}
		else if(i<dials)
		{
			//to jest odczyt wiêc nie kopiujemy
			Dialogue *adial=Subs->dials[i-styles];

			lua_pushstring(L, "[Events]");
			lua_setfield(L, -2, "section");

			wxString raw;
			adial->GetRaw(&raw);

			lua_pushstring(L, raw.mb_str(wxConvUTF8).data());
			lua_setfield(L, -2, "raw");

			lua_pushboolean(L, (int)adial->IsComment);
			lua_setfield(L, -2, "comment");

			lua_pushnumber(L, adial->Layer);
			lua_setfield(L, -2, "layer");

			lua_pushnumber(L, adial->Start.mstime);
			lua_setfield(L, -2, "start_time");
			lua_pushnumber(L, adial->End.mstime);
			lua_setfield(L, -2, "end_time");

			lua_pushstring(L, adial->Style.mb_str(wxConvUTF8).data());
			lua_setfield(L, -2, "style");
			lua_pushstring(L, adial->Actor.mb_str(wxConvUTF8).data());
			lua_setfield(L, -2, "actor");

			lua_pushnumber(L, (int)adial->MarginL);
			lua_setfield(L, -2, "margin_l");
			lua_pushnumber(L, (int)adial->MarginR);
			lua_setfield(L, -2, "margin_r");
			lua_pushnumber(L, (int)adial->MarginV);
			lua_setfield(L, -2, "margin_t");
			lua_pushnumber(L, (int)adial->MarginV);
			lua_setfield(L, -2, "margin_b");

			lua_pushstring(L, adial->Effect.mb_str(wxConvUTF8).data());
			lua_setfield(L, -2, "effect");

			bool isTl = false;
			const StoreTextHelper & text = (isTl = adial->TextTl != "") ? adial->TextTl : adial->Text;
			lua_pushstring(L, text.mb_str(wxConvUTF8).data());
			lua_setfield(L, -2, "text");

			lua_pushboolean(L, isTl);
			lua_setfield(L, -2, "is_translation");

			lua_newtable(L);
			lua_setfield(L, -2, "extra");

			lua_pushstring(L, "dialogue");
		}


		lua_setfield(L, -2, "class");
		return true;
	}

	SubsEntry *AutoToFile::LuaToLine(lua_State *L)
	{
		SubsEntry *e=NULL;

		if (!lua_istable(L, -1)) {
			lua_pushstring(L, "Cannot convert non table value");//nie mo¿na przekonwertowaæ wartoœci która nie jest tablic¹");
			lua_error(L);
			return e;
		}

		lua_getfield(L, -1, "class");
		if (!lua_isstring(L, -1)) {
			lua_pushstring(L, "Table do not have class field");
			lua_error(L);
			return e;
		}
		e=new SubsEntry();
		e->lclass = wxString(lua_tostring(L, -1), wxConvUTF8);
		e->lclass.MakeLower();
		lua_pop(L, 1);

#define GETSTRING(varname, fieldname, lineclass)		\
	lua_getfield(L, -1, fieldname);						\
	if (!lua_isstring(L, -1)) {							\
	lua_pushstring(L, "Invalid string '" fieldname "' field in '" lineclass "' class subtitle line"); \
	lua_error(L);									\
	return e;										\
	}													\
	wxString varname (lua_tostring(L, -1), wxConvUTF8);	\
	lua_pop(L, 1);
#define GETFLOAT(varname, fieldname, lineclass)			\
	lua_getfield(L, -1, fieldname);						\
	if (!lua_isnumber(L, -1)) {							\
	lua_pushstring(L, "Invalid number '" fieldname "' field in '" lineclass "' class subtitle line"); \
	lua_error(L);									\
	return e;										\
	}													\
	float varname = lua_tonumber(L, -1);				\
	lua_pop(L, 1);
#define GETINT(varname, fieldname, lineclass)			\
	lua_getfield(L, -1, fieldname);						\
	if (!lua_isnumber(L, -1)) {							\
	lua_pushstring(L, "Invalid number '" fieldname "' field in '" lineclass "' class subtitle line"); \
	lua_error(L);									\
	return e;										\
	}													\
	int varname = lua_tointeger(L, -1);					\
	lua_pop(L, 1);
#define GETBOOL(varname, fieldname, lineclass)			\
	lua_getfield(L, -1, fieldname);						\
	if (!lua_isboolean(L, -1)) {						\
	lua_pushstring(L, "Invalid boolean '" fieldname "' field in '" lineclass "' class subtitle line"); \
	lua_error(L);									\
	return e;										\
	}													\
	bool varname = !!lua_toboolean(L, -1);				\
	lua_pop(L, 1);

		if(e->lclass=="dialogue"){
			GETBOOL(IsComment, "comment","dialogue")
			GETINT(Layer, "layer", "dialogue")
			GETINT(Start, "start_time", "dialogue")
			GETINT(End, "end_time", "dialogue")
			GETSTRING(Style, "style", "dialogue")
			GETSTRING(Actor, "actor", "dialogue")
			GETINT(MarginL, "margin_l", "dialogue")
			GETINT(MarginR, "margin_r", "dialogue")
			GETINT(margt, "margin_t", "dialogue")
			GETSTRING(Effect, "effect", "dialogue")
			GETSTRING(Text , "text", "dialogue")
			GETBOOL(IsTranslation, "is_translation", "dialogue")

			e->adial=new Dialogue();
			e->adial->IsComment=IsComment;
			e->adial->Layer=Layer;
			e->adial->Start=Start;
			e->adial->End=End;
			if (laf->subsFormat != ASS){
				e->adial->Format = laf->subsFormat;
				e->adial->Start.ChangeFormat(laf->subsFormat);
				e->adial->End.ChangeFormat(laf->subsFormat);
			}
			e->adial->Style=Style;
			e->adial->Actor=Actor;
			e->adial->MarginL=MarginL;
			e->adial->MarginR=MarginR;
			e->adial->MarginV=margt;
			e->adial->Effect=Effect;
			if (IsTranslation)
				e->adial->TextTl = Text;
			else
				e->adial->Text = Text;
			e->adial->ChangeDialogueState(1);
		}

		else if(e->lclass=="style"){

			GETSTRING(name, "name", "style")
				GETSTRING(fontname, "fontname", "style")
				GETFLOAT(fontsize, "fontsize", "style")
				GETSTRING(color1, "color1", "style")
				GETSTRING(color2, "color2", "style")
				GETSTRING(color3, "color3", "style")
				GETSTRING(color4, "color4", "style")
				GETBOOL(bold, "bold", "style")
				GETBOOL(italic, "italic", "style")
				GETBOOL(underline, "underline", "style")
				GETBOOL(strikeout, "strikeout", "style")
				GETFLOAT(scale_x, "scale_x", "style")
				GETFLOAT(scale_y, "scale_y", "style")
				GETFLOAT(spacing, "spacing", "style")
				GETFLOAT(angle, "angle", "style")
				GETINT(borderstyle, "borderstyle", "style")
				GETFLOAT(outline, "outline", "style")
				GETFLOAT(shadow, "shadow", "style")
				GETINT(align, "align", "style")
				GETINT(margin_l, "margin_l", "style")
				GETINT(margin_r, "margin_r", "style")
				GETINT(margin_t, "margin_t", "style")
				GETINT(margin_b, "margin_b", "style")
				GETINT(encoding, "encoding", "style")
				e->astyle=new Styles();
			e->astyle->Name = name;
			e->astyle->Fontname = fontname;
			e->astyle->Fontsize ="";
			e->astyle->Fontsize << fontsize;
			e->astyle->PrimaryColour=color1;
			e->astyle->SecondaryColour=color2;
			e->astyle->OutlineColour=color3;
			e->astyle->BackColour =color4;
			e->astyle->Bold = bold;
			e->astyle->Italic = italic;
			e->astyle->Underline = underline;
			e->astyle->StrikeOut = strikeout;
			e->astyle->ScaleX = "";
			e->astyle->ScaleX<<scale_x;
			e->astyle->ScaleY = "";
			e->astyle->ScaleY<< scale_y;
			e->astyle->Spacing = "";
			e->astyle->Spacing<< spacing;
			e->astyle->Angle = "";
			e->astyle->Angle << angle;
			e->astyle->BorderStyle = (borderstyle==-3);
			e->astyle->Outline = ""; 
			e->astyle->Outline<< outline;
			e->astyle->Shadow = "";
			e->astyle->Shadow << shadow;
			e->astyle->Alignment = "";
			e->astyle->Alignment<< align;
			e->astyle->MarginL = "";
			e->astyle->MarginL<<margin_l;
			e->astyle->MarginR = "";
			e->astyle->MarginR<<margin_r;
			e->astyle->MarginV = "";
			int marg=(margin_t>margin_b)?margin_t:margin_b;
			e->astyle->MarginV<<marg;
			e->astyle->Encoding = "";
			e->astyle->Encoding<<encoding;

		}else if(e->lclass=="info"){

			GETSTRING(key, "key","info");
			GETSTRING(value, "value","info");
			e->info=new SInfo();
			e->info->Name=key;
			e->info->Val=value;
		}
#undef GETSTRING
#undef GETFLOAT
#undef GETINT
#undef GETBOOL

		return e;
	}



	int AutoToFile::ObjectIndexRead(lua_State *L)
	{
		File *Subs=laf->file;
		switch (lua_type(L, 2)) {

		case LUA_TNUMBER:
			{

				// get requested index
				int reqid = lua_tointeger(L, 2);
				if(laf->LineToLua(L,reqid-1)){
					return 1;}
				else{
					return 0;}
			}

		case LUA_TSTRING:
			{
				// either return n or a function doing further stuff
				const char *idx = lua_tostring(L, 2);

				if (strcmp(idx, "n") == 0) {
					// get number of items
					lua_pushnumber(L, Subs->dials.size()+Subs->sinfo.size()+Subs->styles.size());
					return 1;

				} else if (strcmp(idx, "delete") == 0) {
					// make a "delete" function
					lua_pushvalue(L, 1);
					lua_pushcclosure(L, ObjectDelete, 1);
					return 1;

				} else if (strcmp(idx, "deleterange") == 0) {
					// make a "deleterange" function
					lua_pushvalue(L, 1);
					lua_pushcclosure(L, ObjectDeleteRange, 1);
					return 1;

				} else if (strcmp(idx, "insert") == 0) {
					// make an "insert" function
					lua_pushvalue(L, 1);
					lua_pushcclosure(L, ObjectInsert, 1);
					return 1;

				} else if (strcmp(idx, "append") == 0) {
					// make an "append" function
					lua_pushvalue(L, 1);
					lua_pushcclosure(L, ObjectAppend, 1);
					return 1;

				} else if (strcmp(idx, "lengths") == 0) {
					// make an "append" function
					lua_pushvalue(L, 1);
					lua_pushcclosure(L, ObjectLens, 1);
					return 1;

				}else if (strcmp(idx, "script_resolution") == 0) {
					lua_pushvalue(L, 1);
					lua_pushcclosure(L, LuaGetScriptResolution, 1);
					return 1;
				} else {
					// idiot
					lua_pushfstring(L, "Subtitles object do not have index: '%s'", idx);
					lua_error(L);
					// should never return
				}
				assert(false);
			}

		default:
			{
				// crap, user is stupid!
				lua_pushfstring(L, "Subtitles object do not have index type: '%s'.", lua_typename(L, lua_type(L, 2)));
				lua_error(L);
			}
		}

		assert(false);
		return 0;
	}

	int AutoToFile::ObjectIndexWrite(lua_State *L)
	{
		// instead of implementing everything twice, just call the other modification-functions from here
		// after modifying the stack to match their expectations

		if (!lua_isnumber(L, 2)) {
			lua_pushstring(L, "You cannot write usnig non number index");
			lua_error(L);
			return 0;
		}

		File *Subs=laf->file;
		laf->CheckAllowModify();

		int n = lua_tointeger(L, 2);

		if (n < 0) {
			// insert line so new index is n
			lua_pushvalue(L, 1);
			lua_pushcclosure(L, ObjectInsert, 1);
			lua_pushinteger(L, -n);
			lua_pushvalue(L, 3);
			lua_call(L, 2, 0);
			return 0;

		} else if (n == 0) {
			// append line to list
			lua_pushvalue(L, 1);
			lua_pushcclosure(L, ObjectAppend, 1);
			lua_pushvalue(L, 3);
			lua_call(L, 1, 0);
			return 0;

		} else {
			// replace line at index n or delete
			if (!lua_isnil(L, 3)) {
				// insert
				SubsEntry *e=LuaToLine(L);
				if(!e){return 0;}
				int i=n-1;
				int sinfo=Subs->sinfo.size();
				int styles=sinfo+Subs->styles.size();
				int dials=styles+Subs->dials.size();
				if(i<0 || i>=dials){
					SAFE_DELETE(e);
					lua_pushstring(L, "Line index is out of range");
					lua_error(L);
					return 0;}
				if(i<sinfo && e->lclass=="info"){
					SInfo *inf=e->info->Copy();
					Subs->dsinfo.push_back(inf);
					Subs->sinfo[i]=inf;
				}
				else if(i<styles && e->lclass=="style")
				{
					Styles *styl = e->astyle->Copy();
					Subs->dstyles.push_back(styl);
					Subs->styles[i-sinfo]=styl;
				}
				else if(i<dials && e->lclass=="dialogue")
				{
					Dialogue *dial=e->adial->Copy(false,false);
					Subs->ddials.push_back(dial);
					Subs->dials[i-styles]=dial;
				}
				else
				{
					wxString fclass=(e->lclass=="info")? L"info" : (e->lclass=="style")? L"styli" : L"dialogów";
					wxString sclass=(i<sinfo)? L"info" : (i<styles)? L"styli" : L"dialogów";
					wxString all = wxString::Format(_("Nie mo¿na dodaæ linii klasy: %s w pole klasy: %s"), fclass, sclass);
					SAFE_DELETE(e);
					lua_pushstring(L,all.mb_str(wxConvUTF8).data());
					lua_error(L);
				}
				SAFE_DELETE(e);
				return 0;

			} else {
				// delete
				lua_pushvalue(L, 1);
				lua_pushcclosure(L, ObjectDelete, 1);
				lua_pushvalue(L, 2);
				lua_call(L, 1, 0);
				return 0;

			}
		}
	}

	int AutoToFile::ObjectGetLen(lua_State *L)
	{
		File *Subs=laf->file;
		lua_pushnumber(L, Subs->dials.size()+Subs->sinfo.size()+Subs->styles.size());
		return 1;
	}

	int AutoToFile::ObjectLens(lua_State *L)
	{
		File *Subs=laf->file;
		lua_pushinteger(L, (int)Subs->sinfo.size());
		lua_pushinteger(L, (int)Subs->styles.size());
		lua_pushinteger(L, (int)Subs->dials.size());

		return 3;
	}

	int AutoToFile::ObjectDelete(lua_State *L)
	{
		File *Subs=laf->file;
		laf->CheckAllowModify();

		// get number of items to delete
		int itemcount = lua_gettop(L);
		std::vector<int> ids;
		//ids.reserve(itemcount);

		//dorobiæ wstawianie do tablic spellerrors i charspersec podczas rysowania
		int sinfo=Subs->sinfo.size();
		int styles=sinfo+Subs->styles.size();
		int dials=styles+Subs->dials.size();
		// sort the item id's so we can delete from last to first to preserve original numbering
		if (itemcount == 1 && lua_istable(L, 1)) {
			lua_pushvalue(L, 1);
			lua_for_each(L, [&] {
				int n = check_uint(L, -1);
				argcheck(L, n > 0 && n <= dials, 1, "Line index is out of range");
				ids.push_back(n - 1);
			});
		}else{
			while (itemcount > 0) {
				if (!lua_isnumber(L, itemcount)) {
					wxString err("You trying to delete non number line");
					lua_pushstring(L, err.ToUTF8().data());
					lua_error(L);
					return 0;
				}
				int n = lua_tointeger(L, itemcount);
				argcheck(L, n > 0 && n <= dials, itemcount, "Out of range line index");
				ids.push_back(n - 1); // make C-style line ids
				--itemcount;
			}
		}
		std::sort(ids.begin(), ids.end());

		for(int i= ids.size()-1 ; i>=0; i--)
		{
			if(ids[i]<sinfo){
				Subs->sinfo.erase(Subs->sinfo.begin()+ids[i]);
			}
			else if(ids[i]<styles){
				Subs->styles.erase(Subs->styles.begin()+(ids[i]-sinfo));
			}
			else if(ids[i]<dials){
				Subs->dials.erase(Subs->dials.begin()+(ids[i]-styles));
			}
		}

		return 0;
	}

	int AutoToFile::ObjectDeleteRange(lua_State *L)
	{
		File *Subs=laf->file;

		laf->CheckAllowModify();

		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2)) {
			lua_pushstring(L, "Non number argument of function DeleteRange");
			lua_error(L);
			return 0;
		}

		int a = lua_tointeger(L, 1), b = lua_tointeger(L, 2);
		int sinfo=Subs->sinfo.size();
		int styles=sinfo+Subs->styles.size();
		int dials=styles+Subs->dials.size();
		int all = dials+1;

		if (a < 1) a = 1;
		if (b > all ) b = all;

		if (b < a) return 0;
		a--;b--;



		for(int i=b; i >= a; i--){
			if(i<sinfo){
				Subs->sinfo.erase(Subs->sinfo.begin()+i);
			}else if(i<styles){
				Subs->styles.erase(Subs->styles.begin()+(i-sinfo));
			}else{
				Subs->dials.erase(Subs->dials.begin()+(i-styles));
			}
		}	
		return 0;
	}

	int AutoToFile::ObjectAppend(lua_State *L)
	{
		File *Subs=laf->file;

		laf->CheckAllowModify();

		int n = lua_gettop(L);

		for (int i = 1; i <= n; i++) {
			lua_pushvalue(L, i);
			SubsEntry *e = LuaToLine(L);
			if(!e){return 0;}
			if(e->lclass=="info")
			{
				SInfo *inf=e->info->Copy();
				Subs->sinfo.push_back(inf);
				Subs->dsinfo.push_back(inf);
			}
			else if(e->lclass=="style")
			{
				Styles *styl=e->astyle->Copy();
				Subs->styles.push_back(styl);
				Subs->dstyles.push_back(styl);
			}
			if(e->lclass=="dialogue"){
				Dialogue *dial=e->adial->Copy();
				Subs->ddials.push_back(dial);
				Subs->dials.push_back(dial);
			}
			SAFE_DELETE(e);
		}

		return 0;
	}

	int AutoToFile::ObjectInsert(lua_State *L)
	{
		File *Subs=laf->file;

		laf->CheckAllowModify();

		if (!lua_isnumber(L, 1)) {
			lua_pushstring(L, "Cannot put non numeric index");
			lua_error(L);
			return 0;
		}

		int n = lua_gettop(L);

		int start = int(lua_tonumber(L, 1)-1);

		if(start<0 || start>(int)(Subs->sinfo.size()+Subs->styles.size()+Subs->dials.size()))
		{
			lua_pushstring(L, "Out of range line index");
			lua_error(L);
			return 0;
		}

		for (int i = 2; i <= n; i++) {
			lua_pushvalue(L, i);
			SubsEntry *e = LuaToLine(L);
			if(!e){return 0;}
			lua_pop(L, 1);
			int sinfo=Subs->sinfo.size();
			int stylsize=Subs->styles.size();
			int styles=sinfo+stylsize;
			int dialsize=Subs->dials.size();
			int dials=styles+dialsize;

			if(e->lclass=="info")
			{
				SInfo *inf=e->info->Copy();
				if(start >= sinfo){Subs->sinfo.push_back(inf);}
				else{Subs->sinfo.insert(Subs->sinfo.begin()+start, inf);}
				Subs->dsinfo.push_back(inf);
			}
			else if(e->lclass=="style")
			{
				Styles *styl=e->astyle->Copy();
				if(start-sinfo >= stylsize){Subs->styles.push_back(styl);}
				else{Subs->styles.insert(Subs->styles.begin()+(start-sinfo), styl);}
				Subs->dstyles.push_back(styl);
			}
			else if(e->lclass=="dialogue")
			{
				int newStart = start - styles;
				Dialogue *dial = e->adial->Copy(false, newStart >= dialsize);
				if (newStart >= dialsize){ Subs->dials.push_back(dial); }
				else{ Subs->dials.insert(Subs->dials.begin() + newStart, dial); }
				Subs->ddials.push_back(dial);
			}
			else{
				SAFE_DELETE(e);
				wxString sclass= "You trying to put line of unknown class";
				lua_pushstring(L, sclass.mb_str(wxConvUTF8).data());
				lua_error(L);
			}
			start++;
			SAFE_DELETE(e);
		}
		return 0;
	}

	bool static inline IsKtag(wxString text, wxString tag, wxString *ktag, wxString *rest)
	{
		bool isk=text.StartsWith(tag,rest);
		if(isk){*ktag=tag;}
		return isk;
	}

	int AutoToFile::LuaParseKaraokeData(lua_State *L)
	{
		SubsEntry *e = LuaToLine(L);
		if(!e){return 0;}
		else if(e->lclass!="dialogue"){
			lua_pushstring(L, "You try to parse karaoke from non dialogue line");
			lua_error(L);
			return 0;
		}

		int kcount = 0;
		int kdur = 0;
		int ktime = 0;
		wxString ktag = _T("");
		wxString ktext = _T("");
		wxString ktext_stripped = _T("");

		lua_createtable(L, 0, 6);
		set_field(L, "duration", 0);
		set_field(L, "start_time", 0);
		set_field(L, "end_time", 0);
		set_field(L, "tag", "");
		set_field(L, "text", "");
		set_field(L, "text_stripped", "");
		lua_rawseti(L, -2, kcount++);
		wxString tags[] = { "kf", "ko", "k", "K" };
		e->adial->ParseTags(tags, 4);
		ParseData *Data = e->adial->parseData;
		//wxStringTokenizer ktok(e->adial->Text,"\\",wxTOKEN_STRTOK);
		const wxString & text = e->adial->Text;
		size_t lastPosition = 0;
		wxString rest;
		//wxString deb;
		wxRegEx reg(_T("\\{[^\\}]*\\}"),wxRE_ADVANCED);
		size_t tagssize = Data->tags.size();
			for (size_t i = 0; i < tagssize; i++) {

			size_t nextKstart = lastPosition;
			if(i < tagssize-1){
				nextKstart = Data->tags[i + 1]->startTextPos;
				wxString newtxt = text.Mid(lastPosition, nextKstart - lastPosition);
				size_t bracketstartpos = newtxt.Find('{', true);
				if (bracketstartpos == wxNOT_FOUND){
					size_t bracketendpos = newtxt.Find('}', true);
					size_t firstSlash = newtxt.find(L'//', bracketendpos + 1);
					if (firstSlash != wxNOT_FOUND)
						nextKstart = firstSlash - 1;
				}
				else{
					nextKstart = lastPosition + bracketstartpos - 1;
				}

			}else 
				nextKstart = text.Len();

			TagData * tdata = Data->tags[i];

			//size_t klen = tdata->value.Len() + tdata->tagName.Len();
			kdur = wxAtoi(tdata->value);
			kdur *= 10;
			ktext = text.Mid(lastPosition, tdata->startTextPos - lastPosition - tdata->tagName.Len()-1);
			size_t newStart = tdata->startTextPos + tdata->value.Len();
			ktext += text.Mid(newStart, nextKstart - newStart + 1);
			ktext.Replace("{}", "");
			ktext_stripped = ktext;
			reg.ReplaceAll(&ktext_stripped, _T(""));
			lua_createtable(L, 0, 6);
			set_field(L, "duration", kdur);
			set_field(L, "start_time", ktime);
			set_field(L, "end_time", ktime += kdur);
			set_field(L, "tag", tdata->tagName.mb_str(wxConvUTF8).data());
			set_field(L, "text", ktext.mb_str(wxConvUTF8).data());
			set_field(L, "text_stripped", ktext_stripped.mb_str(wxConvUTF8).data());
			lua_rawseti(L, -2, kcount++);
			
			lastPosition = nextKstart + 1;
			
		}
		if(kcount<2){
			ktext_stripped = e->adial->Text;
			reg.ReplaceAll(&ktext_stripped,_T(""));

			lua_createtable(L, 0, 6);
			set_field(L, "duration", (e->adial->End.mstime - e->adial->Start.mstime));
			set_field(L, "start_time", e->adial->Start.mstime);
			set_field(L, "end_time", e->adial->End.mstime);
			set_field(L, "tag", "k");
			set_field(L, "text", e->adial->Text.mb_str(wxConvUTF8).data());
			set_field(L, "text_stripped", ktext_stripped.mb_str(wxConvUTF8).data());
			lua_rawseti(L, -2, kcount++);
		}
		SAFE_DELETE(e);

		return 1;
	}

	int AutoToFile::ObjectIPairs(lua_State *L)
	{
		lua_pushvalue(L, lua_upvalueindex(1)); // push 'this' as userdata
		lua_pushcclosure(L, &AutoToFile::IterNext, 1);
		lua_pushnil(L);
		push_value(L, 0);
		return 3;
	}

	int AutoToFile::IterNext(lua_State *L)
	{
		File *Subs=laf->file;
		size_t i = check_uint(L, 2);
		if (i >= Subs->dials.size()+Subs->sinfo.size()+Subs->styles.size()) {
			lua_pushnil(L);
			return 1;
		}

		push_value(L, i + 1);
		LineToLua(L, i);
		return 2;
	}

	int AutoToFile::LuaGetScriptResolution(lua_State *L)
	{
		int w, h;
		Notebook::GetTab()->Grid->GetASSRes(&w, &h);
		push_value(L, w);
		push_value(L, h);
		return 2;
	}

	int AutoToFile::LuaGetFreqencyReach(lua_State *L)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4) || !lua_isnumber(L, 5)) {
			lua_pushstring(L, "Non number argument of function GetFreqencyReach");
			lua_error(L);
			return 0;
		}
		TabPanel *tab = Notebook::GetTab();
		VideoFfmpeg* VFF = tab->Video->VFF;
		if (!VFF){
			if (!tab->Edit->ABox){
				lua_pushstring(L, "GetFreqencyReach needs loaded audio by FFMS2");
				lua_error(L);
				return 0;
			}
			VFF = tab->Edit->ABox->audioDisplay->provider;
			if (!VFF){
				lua_pushstring(L, "GetFreqencyReach cannot get audio provider");
				lua_error(L);
				return 0;
			}
		}

		int start = lua_tointeger(L, 1), 
			end = lua_tointeger(L, 2), 
			freqstart = lua_tointeger(L, 3),
			freqend = lua_tointeger(L, 4),
			peek = MID(0,lua_tointeger(L, 5),1000);

		std::vector<int> output;
		std::vector<int> intensities;
		if (start < 0 || end < 0){
			lua_pushstring(L, "GetFreqencyReach start or end time less than zero");
			lua_error(L);
			return 0;
		}
		else if (start >= end){
			push_value(L, output);
			return 1;
		}
			
		
		if (!laf->spectrum)
			laf->spectrum = new AudioSpectrum(VFF);

		laf->spectrum->CreateRange(output, intensities, start, end, wxPoint(freqstart,freqend), peek);
		push_value(L, output);
		if (peek > 0){
			return 1;
		}
		push_value(L, intensities);
		return 2;
	}

	AutoToFile::AutoToFile(lua_State *_L, File *subsfile, bool _can_modify, char _subsFormat)
	{
		L=_L;
		can_modify=_can_modify;
		file =subsfile;
		subsFormat = _subsFormat;
		// prepare userdata object
		*static_cast<AutoToFile**>(lua_newuserdata(L, sizeof(AutoToFile*))) = this;
		laf=this;

		// make the metatable
		lua_createtable(L, 0, 5);
		set_field<&AutoToFile::ObjectIndexRead>(L, "__index");
		set_field<&AutoToFile::ObjectIndexWrite>(L, "__newindex");
		set_field<&AutoToFile::ObjectGetLen>(L, "__len");
		set_field<&AutoToFile::ObjectGarbageCollect>(L, "__gc");
		set_field<&AutoToFile::ObjectIPairs>(L, "__ipairs");
		lua_setmetatable(L, -2);

		// register misc functions
		// assume the "aegisub" global table exists
		lua_getglobal(L, "aegisub");

		set_field<&AutoToFile::LuaParseKaraokeData>(L, "parse_karaoke_data");
		set_field<&AutoToFile::LuaGetFreqencyReach>(L, "get_frequency_peeks");
		set_field<&AutoToFile::LuaSetUndoPoint>(L, "set_undo_point");

		lua_pop(L, 1); // pop "aegisub" table

		// Leaves userdata object on stack
	}

	void AutoToFile::Cancel()
	{
		delete this;
	}

	int AutoToFile::ObjectGarbageCollect(lua_State *L){
		//assert(lua_type(L, idx) == LUA_TUSERDATA);
		//auto ud = lua_touserdata(L, lua_upvalueindex(1));
		//if(!ud){return 0;}
		//auto laf = *static_cast<AutoToFile **>(ud);
		//delete laf;
		return 0;
	};
}

