
#include "AutomationToFile.h"
#include "kainoteApp.h"
#include <wx/regex.h>

namespace Auto{

	SubsEntry::SubsEntry()
	{
		adial=NULL;
		astyle=NULL;
		info= NULL;
	}
	SubsEntry::~SubsEntry()
	{
		wxDELETE(adial);
		wxDELETE(astyle);
		wxDELETE(info);
	}


AutoToFile *AutoToFile::laf=NULL;

AutoToFile::~AutoToFile()
	{
		////wxLogStatus("auto to file dest");
	}

void AutoToFile::CheckAllowModify()
	{
		if (can_modify)
			return;
		lua_pushstring(L, "Probujesz zmodyfikowac napisy tylko do odczytu.");
		lua_error(L);
	}




bool AutoToFile::LineToLua(lua_State *L, int i)
	{
		//AutoToFile *laf = GetObjPointer(L, 1);
		int sinfo=laf->File->subs->sinfo.size();
		int styles=sinfo+laf->File->subs->styles.size();
		int dials=styles+laf->File->subs->dials.size();
		//wxLogStatus("iii %i %i %i %i", i, sinfo, styles, dials);
		if(i<0||i>=dials){
			return false;
		}

		lua_newtable(L);
		if(i<sinfo){
			//wxLogStatus("sinfo %i %i", i, sinfo );
			//to jest odczyt wiêc nie kopiujemy
			SInfo *info=laf->File->subs->sinfo[i];

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
			//wxLogStatus("styles %i %i", i-sinfo, dials-sinfo );
			//to jest odczyt wiêc nie kopiujemy
			Styles *astyle=laf->File->subs->styles[i-sinfo];
			
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

			lua_pushstring(L, "style");
			
		}
		else if(i<dials)
		{
			//wxLogStatus("dials %i %i", i-styles, dials-styles );
			//to jest odczyt wiêc nie kopiujemy
			Dialogue *adial=laf->File->subs->dials[i-styles];

			wxString raw(adial->GetRaw());

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

			lua_pushnumber(L, adial->MarginL);
			lua_setfield(L, -2, "margin_l");
			lua_pushnumber(L, adial->MarginR);
			lua_setfield(L, -2, "margin_r");
			lua_pushnumber(L, adial->MarginV);
			lua_setfield(L, -2, "margin_t");
			lua_pushnumber(L, adial->MarginV);
			lua_setfield(L, -2, "margin_b");

			lua_pushstring(L, adial->Effect.mb_str(wxConvUTF8).data());
			lua_setfield(L, -2, "effect");

			lua_pushstring(L, adial->Text.mb_str(wxConvUTF8).data());
			lua_setfield(L, -2, "text");

			lua_pushstring(L, "dialogue");
		}


		lua_setfield(L, -2, "class");
		return true;
	}

SubsEntry *AutoToFile::LuaToLine(lua_State *L)
	{
		SubsEntry *e=NULL;
		
		if (!lua_istable(L, -1)) {
			lua_pushstring(L, "nie mozna przekonwertowac wartosci ktora nie jest tablica");//nie mo¿na przekonwertowaæ wartoœci która nie jest tablic¹");
			lua_error(L);
			return e;
		}

		lua_getfield(L, -1, "class");
		if (!lua_isstring(L, -1)) {
			lua_pushstring(L, "Tablicy brakuje pola class");
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
			GETINT(margb, "margin_b", "dialogue")
			GETSTRING(Effect, "effect", "dialogue")
			GETSTRING(Text , "text", "dialogue")
			e->adial=new Dialogue();
			e->adial->IsComment=IsComment;
			e->adial->Layer=Layer;
			e->adial->Start=Start;
			e->adial->End=End;
			e->adial->Style=Style;
			e->adial->Actor=Actor;
			e->adial->MarginL=MarginL;
			e->adial->MarginR=MarginR;
			e->adial->MarginV=(margt>margb)?margt : margb;
			e->adial->Effect=Effect;
			e->adial->Text=Text;
			e->adial->State=1;
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
		//AutoToFile *laf = GetObjPointer(L, 1);

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
						lua_pushnumber(L, laf->File->subs->dials.size()+laf->File->subs->sinfo.size()+laf->File->subs->styles.size());
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

					} else {
						// idiot
						lua_pushfstring(L, "Nieodpowiedni obiekt indeksu napisow: '%s'", idx);
						lua_error(L);
						// should never return
					}
					assert(false);
				}

			default:
				{
					// crap, user is stupid!
					lua_pushfstring(L, "Indeks napisow typu '%s'.", lua_typename(L, lua_type(L, 2)));
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
			lua_pushstring(L, "Probujesz zapisac nieliczbowy indeks do indeksu napisow");
			lua_error(L);
			return 0;
		}

		//AutoToFile *laf = GetObjPointer(L, 1);
		laf->CheckAllowModify();

		int n = lua_tointeger(L, 2);

		if (n < 0) {
			// insert line so new index is n
			lua_pushvalue(L, 1);
			lua_pushcclosure(L, ObjectInsert, 1);
			lua_pushinteger(L, -n);
			lua_pushvalue(L, 3);
			lua_call(L, 2, 0);
			//wxLogStatus("n mniejsze od zera");
			return 0;

		} else if (n == 0) {
			// append line to list
			lua_pushvalue(L, 1);
			lua_pushcclosure(L, ObjectAppend, 1);
			lua_pushvalue(L, 3);
			lua_call(L, 1, 0);
			//wxLogStatus("n = 0");
			return 0;

		} else {
			// replace line at index n or delete
			if (!lua_isnil(L, 3)) {
				// insert
				SubsEntry *e=LuaToLine(L);
				if(!e){return 0;}
				int i=n-1;
				int sinfo=laf->File->subs->sinfo.size();
				int styles=sinfo+laf->File->subs->styles.size();
				int dials=styles+laf->File->subs->dials.size();
				if(i<0 || i>=dials){
					lua_pushstring(L, "Out of range");//"Próbujesz zmodyfikowaæ linijkê o indeksie przekraczaj¹cym wielkoœæ napisów");
					lua_error(L);
					return 0;}
				if(i<sinfo && e->lclass=="info"){
					//wxLogStatus("Sifno nadpisanie %i, %i", i, sinfo);
					SInfo *inf=e->info->Copy();
					laf->File->subs->dsinfo.push_back(inf);
					laf->File->subs->sinfo[i]=inf;
					}
				else if(i<styles && e->lclass=="style")
					{
					//wxLogStatus("Styles nadpisanie %i, %i", i, styles);
					Styles *styl = e->astyle->Copy();
					laf->File->subs->dstyles.push_back(styl);
					laf->File->subs->styles[i-sinfo]=styl;
					//wxLogStatus("style");
					}
				else if(i<dials && e->lclass=="dialogue")
					{
					//wxLogStatus("Dials nadpisanie %i, %i", i, dials);
					Dialogue *dial=e->adial->Copy();
					laf->File->subs->ddials.push_back(dial);
					laf->File->subs->dials[i-styles]=dial;
					
					//wxLogStatus(dial->GetRaw());
					}
				else
					{
					wxString fclass=(e->lclass=="info")? "info" : (e->lclass=="style")? "styli" : "dialogow";
					wxString sclass=(i<sinfo)? "info" : (i<styles)? "styli" : "dialogow";
					wxString all = _("Probujesz wstawic linie klasy ") + fclass + _(" w przedzial klasy ") + sclass;
					lua_pushstring(L,all.mb_str(wxConvUTF8).data());
					lua_error(L);
					}
				wxDELETE(e);
				return 0;

			} else {
				// delete
				lua_pushvalue(L, 1);
				lua_pushcclosure(L, ObjectDelete, 1);
				lua_pushvalue(L, 2);
				lua_call(L, 1, 0);
				//wxLogStatus("delete");
				return 0;

			}
		}
	}

	int AutoToFile::ObjectGetLen(lua_State *L)
	{
		//AutoToFile *laf = GetObjPointer(L, 1);
		lua_pushnumber(L, laf->File->subs->dials.size()+laf->File->subs->sinfo.size()+laf->File->subs->styles.size());
		return 1;
	}

	int AutoToFile::ObjectLens(lua_State *L)
	{
		//AutoToFile *laf = GetObjPointer(L, lua_upvalueindex(1));
		lua_pushinteger(L, (int)laf->File->subs->sinfo.size());
		lua_pushinteger(L, (int)laf->File->subs->styles.size());
		lua_pushinteger(L, (int)laf->File->subs->dials.size());

		return 3;
	}

	int AutoToFile::ObjectDelete(lua_State *L)
	{
	//wxLogStatus("wesz³o");
		//AutoToFile *laf = GetObjPointer(L, lua_upvalueindex(1));

		laf->CheckAllowModify();
		
		// get number of items to delete
		int itemcount = lua_gettop(L);
		//wxLogStatus("itemcount %i", itemcount);
		std::vector<int> ids;
		//ids.reserve(itemcount);
		

		int sinfo=laf->File->subs->sinfo.size();
		int styles=sinfo+laf->File->subs->styles.size();
		int dials=styles+laf->File->subs->dials.size();
		// sort the item id's so we can delete from last to first to preserve original numbering
		while (itemcount > 0) {
			if (!lua_isnumber(L, itemcount)) {
				lua_pushstring(L, "Nonnumeric");//Próbujesz usun¹æ nienumeryczn¹ linijkê z indeksu napisów");
				lua_error(L);
				return 0;
			}
			int n = lua_tointeger(L, itemcount);
			if (n > dials || n < 1) {
				lua_pushstring(L, "Out of range");//Próbujesz usun¹æ liniê która przekracza iloœæ linii napisów");
				lua_error(L);
				return 0;
			}
			ids.push_back(n-1); // make C-style line ids
			--itemcount;
		}
		std::sort(ids.begin(), ids.end());
		
		//for(size_t i=0;i<ids.size();i++)
			//{
			//kkk<<ids[i]<<", ";
			//}
		//wxLogStatus(kkk);
		

		for(int i= ids.size()-1 ; i>=0; i--)
		{
			if(ids[i]<sinfo){
				//wxLogStatus("delete si %i %i", ids[i], sinfo);
				laf->File->subs->sinfo.erase(laf->File->subs->sinfo.begin()+ids[i]);
			}
			else if(ids[i]<styles){
				//wxLogStatus("delete st %i %i", ids[i], styles);
				laf->File->subs->styles.erase(laf->File->subs->styles.begin()+(ids[i]-sinfo));
			}
			else if(ids[i]<dials){
				//wxLogStatus("delete dial %i %i", ids[i]-styles, dials-styles);
				laf->File->subs->dials.erase(laf->File->subs->dials.begin()+(ids[i]-styles));
			}
		}
		 
		return 0;
	}

	int AutoToFile::ObjectDeleteRange(lua_State *L)
	{
		//AutoToFile *laf = GetObjPointer(L, lua_upvalueindex(1));

		laf->CheckAllowModify();
		
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2)) {
			lua_pushstring(L, "Nienumeryczny argument uzyty w funkcji DeleteRange");
			lua_error(L);
			return 0;
		}

		int a = lua_tointeger(L, 1), b = lua_tointeger(L, 2);

		if (a < 1) a = 1;
		if (b > (int)laf->File->subs->dials.size()) b = (int)laf->File->subs->dials.size();

		if (b < a) return 0;
		a--;b--;
		int sinfo=laf->File->subs->sinfo.size();
		int styles=sinfo+laf->File->subs->styles.size();
		int dials=styles+laf->File->subs->dials.size();


		for(int i=b-1; i >= a; i--){
			if(i<sinfo){
				//wxLogStatus("deleterange si %i %i", i, sinfo); 
				laf->File->subs->sinfo.erase(laf->File->subs->sinfo.begin()+i);
			}else if(i<styles){
				//wxLogStatus("deleterange st %i %i", i, styles);
				laf->File->subs->styles.erase(laf->File->subs->styles.begin()+i-sinfo);
			}else if(i<dials){
				//wxLogStatus("deleterange dial %i %i", i, dials);
				laf->File->subs->dials.erase(laf->File->subs->dials.begin()+i-styles);}
		}	
		return 0;
	}

	int AutoToFile::ObjectAppend(lua_State *L)
	{
		//AutoToFile *laf = GetObjPointer(L, lua_upvalueindex(1));

		laf->CheckAllowModify();
		
		int n = lua_gettop(L);

		for (int i = 1; i <= n; i++) {
			lua_pushvalue(L, i);
			SubsEntry *e = LuaToLine(L);
			if(!e){return 0;}
			if(e->lclass=="dialogue"){
				Dialogue *dial=e->adial->Copy();
				laf->File->subs->ddials.push_back(dial);
				laf->File->subs->dials.push_back(dial);
			}
			wxDELETE(e);
		}

		return 0;
	}

	int AutoToFile::ObjectInsert(lua_State *L)
	{
		//AutoToFile *laf = GetObjPointer(L, lua_upvalueindex(1));

		laf->CheckAllowModify();
		
		if (!lua_isnumber(L, 1)) {
			lua_pushstring(L, "Nie mozna wstawic nienumerycznego indeksu");
			lua_error(L);
			return 0;
		}

		int n = lua_gettop(L);

		int start = int(lua_tonumber(L, 1)-1);
		
		if(start<0 || start>(int)(laf->File->subs->sinfo.size()+laf->File->subs->styles.size()+laf->File->subs->dials.size()))
		{
			lua_pushstring(L, "Indeks przekracza wielkosc tablicy z napiami");
			lua_error(L);
			return 0;
		}

		for (int i = 2; i <= n; i++) {
			lua_pushvalue(L, i);
			SubsEntry *e = LuaToLine(L);
			if(!e){return 0;}
			lua_pop(L, 1);
			int sinfo=laf->File->subs->sinfo.size();
			int stylsize=laf->File->subs->styles.size();
			int styles=sinfo+stylsize;
			int dialsize=laf->File->subs->dials.size();
			int dials=styles+dialsize;
			
			if(e->lclass=="info")
			{
				SInfo *inf=e->info->Copy();
				//wxLogStatus("sinfostart %i %i", start, sinfo);
				if(start >= sinfo){laf->File->subs->sinfo.push_back(inf);}
				else{laf->File->subs->sinfo.insert(laf->File->subs->sinfo.begin()+start, inf);}
				laf->File->subs->dsinfo.push_back(inf);
			}
			else if(e->lclass=="style")
			{
				//wxLogStatus("stylesstart %i %i", start-sinfo, stylsize);
				Styles *styl=e->astyle->Copy();
				if(start-sinfo >= stylsize){laf->File->subs->styles.push_back(styl);}
				else{laf->File->subs->styles.insert(laf->File->subs->styles.begin()+(start-sinfo), styl);}
				laf->File->subs->dstyles.push_back(styl);
			}
			else if(e->lclass=="dialogue")
			{
				//wxLogStatus("dialstart %i %i", start-styles, laf->File->subs->dials.size());
				Dialogue *dial=e->adial->Copy();
				if(start-styles >= dialsize){laf->File->subs->dials.push_back(dial);}
				else{laf->File->subs->dials.insert(laf->File->subs->dials.begin()+(start-styles), dial);}
				laf->File->subs->ddials.push_back(dial);
			}
			else{
				
				wxString sclass= "Probujesz wstawiæ linijkê nieznanej klasy";
				lua_pushstring(L, sclass.mb_str(wxConvUTF8).data());
				lua_error(L);
			}
			start++;
			wxDELETE(e);
		}
		//wxLogStatus("inserted");
		return 0;
	}

	bool static inline IsKtag(wxString text, wxString tag, wxString *ktag, wxString *rest)
	{
		bool isk=text.StartsWith(tag,rest);
		if(isk){*ktag=tag;}
		return isk;
	}

	int AutoToFile::LuaParseKaraokeData(lua_State *L)
	{//wxLogStatus("kara wesz³o");
		SubsEntry *e = LuaToLine(L);
		if(!e){return 0;}
		else if(e->lclass!="dialogue"){
			lua_pushstring(L, "Probujesz podzielic na sylaby linie ktora nie jest dialogiem");//"Próbujesz podzieliæ na sylaby liniê która nie jest dialogiem");
			lua_error(L);
			return 0;
		}

		int kcount = 0;
		int kdur = 0;
		int ktime = 0;
		wxString ktag = _T("");
		wxString ktext = _T("");
		wxString ktext_stripped = _T("");

		lua_newtable(L);
		wxStringTokenizer ktok(e->adial->Text,"\\",wxTOKEN_STRTOK);

		bool inside=false;
		bool valid=false;
		wxString rest;
		//wxString deb;
		wxRegEx reg(_T("\\{[^\\{]*\\}"),wxRE_ADVANCED);
   
		while (ktok.HasMoreTokens()) {
			wxString tekst = ktok.NextToken();
			//deb<<tekst<<"@";
			if(inside){
				if(IsKtag(tekst, "kf", &ktag, &rest)||IsKtag(tekst, "ko", &ktag, &rest)||
					IsKtag(tekst, "k", &ktag, &rest)||IsKtag(tekst, "K", &ktag, &rest)){
						if(rest.Find('}') != -1){tekst="}"+rest.AfterFirst('}');}
						kdur=wxAtoi(rest);
						kdur*=10;
						valid=true;

					}else{
						ktext << "\\" << tekst.BeforeFirst('}') <<"}";
						if(tekst.Find('}') != -1){tekst="}"+tekst.AfterFirst('}');}
					}

				if(tekst.Find('}') != -1){ 
					if(tekst.Find('{',true) != -1){ tekst=tekst.BeforeLast('{');}else{inside=false;}
					if(tekst.StartsWith("}")){tekst=tekst.Mid(1);}
					ktext+=tekst;
					if(ktext.StartsWith("\\")){ktext.Prepend("{");}
					
					ktext_stripped = ktext;
					reg.ReplaceAll(&ktext_stripped,_T(""));
					//wxLogMessage(ktext_stripped+", "+ktext);
					//deb << ktext<<"@";
					if(valid){
					kcount++;
						lua_newtable(L);
						lua_pushnumber(L, kdur);
						lua_setfield(L, -2, "duration");
						lua_pushnumber(L, ktime);
						lua_setfield(L, -2, "start_time");
						lua_pushnumber(L, ktime+=kdur);
						lua_setfield(L, -2, "end_time");
						lua_pushstring(L, ktag.mb_str(wxConvUTF8).data());
						lua_setfield(L, -2, "tag");
						lua_pushstring(L, ktext.mb_str(wxConvUTF8).data());
						lua_setfield(L, -2, "text");
						lua_pushstring(L, ktext_stripped.mb_str(wxConvUTF8).data());
						lua_setfield(L, -2, "text_stripped");
						lua_rawseti(L, -2, kcount);
						valid=false;
					}
					ktext_stripped="";
					ktext="";
				}

				
			}

			else if(tekst.Find('{') != -1){
				inside=true;
				tekst=tekst.BeforeFirst('{');
				ktext+=tekst;
				ktext_stripped=tekst;
			}
			else{
				ktext+=tekst;
				ktext_stripped=tekst;
			}

		}
		//wxLogStatus(deb);
		wxDELETE(e);
		
		return 1;
	}



AutoToFile::AutoToFile(lua_State *_L, bool _can_modify)
{
	L=_L;
	void *ud = lua_newuserdata(L, sizeof(AutoToFile*));
	//*((AutoToFile**)ud) = this;
	laf=this;

	File=Notebook::GetTab()->Grid1->file;
	can_modify=_can_modify;


	// make the metatable
	lua_newtable(L);
	lua_pushcfunction(L, ObjectIndexRead);
	lua_setfield(L, -2, "__index");
	lua_pushcfunction(L, ObjectIndexWrite);
	lua_setfield(L, -2, "__newindex");
	lua_pushcfunction(L, ObjectGetLen);
	lua_setfield(L, -2, "__len");
	lua_setmetatable(L, -2);
	//wxLogStatus("Autotofile1");
		
	// assume the "kainote" global table exists
	lua_getglobal(L, "kainote");
	assert(lua_type(L, -2) == LUA_TUSERDATA);
	//wxLogStatus("kainote %i", lua_type(L, -1));
	lua_pushvalue(L, -2);
	lua_pushcclosure(L, LuaParseKaraokeData, 1);
	//lua_pushcfunction(L, LuaParseKaraokeData);
	lua_setfield(L, -2, "parse_karaoke_data");
	lua_pop(L, 1);
}

}