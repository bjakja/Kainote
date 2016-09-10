
#include "AutomationToFile.h"
#include "AutomationUtils.h"
#include "KainoteApp.h"
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
		File *Subs=laf->file;

		int sinfo=Subs->sinfo.size();
		int styles=sinfo+Subs->styles.size();
		int dials=styles+Subs->dials.size();
		//wxLogStatus("iii %i %i %i %i", i, sinfo, styles, dials);
		if(i<0||i>=dials){
			return false;
		}

		lua_newtable(L);
		if(i<sinfo){
			//wxLogStatus("sinfo %i %i", i, sinfo );
			//to jest odczyt wiêc nie kopiujemy
			SInfo *info=Subs->sinfo[i];

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
			Styles *astyle=Subs->styles[i-sinfo];
			
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
			Dialogue *adial=Subs->dials[i-styles];

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
				int sinfo=Subs->sinfo.size();
				int styles=sinfo+Subs->styles.size();
				int dials=styles+Subs->dials.size();
				if(i<0 || i>=dials){
					lua_pushstring(L, "Out of range");//"Próbujesz zmodyfikowaæ linijkê o indeksie przekraczaj¹cym wielkoœæ napisów");
					lua_error(L);
					return 0;}
				if(i<sinfo && e->lclass=="info"){
					//wxLogStatus("Sifno nadpisanie %i, %i", i, sinfo);
					SInfo *inf=e->info->Copy();
					Subs->dsinfo.push_back(inf);
					Subs->sinfo[i]=inf;
					}
				else if(i<styles && e->lclass=="style")
					{
					//wxLogStatus("Styles nadpisanie %i, %i", i, styles);
					Styles *styl = e->astyle->Copy();
					Subs->dstyles.push_back(styl);
					Subs->styles[i-sinfo]=styl;
					//wxLogStatus("style");
					}
				else if(i<dials && e->lclass=="dialogue")
					{
					//wxLogStatus("Dials nadpisanie %i, %i", i, dials);
					Dialogue *dial=e->adial->Copy();
					Subs->ddials.push_back(dial);
					Subs->dials[i-styles]=dial;
					//wxLogStatus(dial->GetRaw());
					}
				else
					{
					wxString fclass=(e->lclass=="info")? "info" : (e->lclass=="style")? "styli" : "dialogów";
					wxString sclass=(i<sinfo)? "info" : (i<styles)? "styli" : "dialogów";
					wxString all = _("Próbujesz wstawiæ linie klasy ") + fclass + _(" w przedzia³ klasy ") + sclass;
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
				//wxLogStatus("delete");
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
		//wxLogStatus("itemcount %i", itemcount);
		std::vector<int> ids;
		//ids.reserve(itemcount);
		
		//dorobiæ wstawianie do tablic spellerrors i charspersec podczas rysowania
		int sinfo=Subs->sinfo.size();
		int styles=sinfo+Subs->styles.size();
		int dials=styles+Subs->dials.size();
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
				Subs->sinfo.erase(Subs->sinfo.begin()+ids[i]);
			}
			else if(ids[i]<styles){
				//wxLogStatus("delete st %i %i", ids[i], styles);
				Subs->styles.erase(Subs->styles.begin()+(ids[i]-sinfo));
			}
			else if(ids[i]<dials){
				//wxLogStatus("delete dial %i %i", ids[i]-styles, dials-styles);
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
			lua_pushstring(L, "Nienumeryczny argument uzyty w funkcji DeleteRange");
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
				//wxLogStatus("deleterange si %i %i", i, sinfo); 
				Subs->sinfo.erase(Subs->sinfo.begin()+i);
			}else if(i<styles){
				//wxLogStatus("deleterange st %i %i", i, styles);
				Subs->styles.erase(Subs->styles.begin()+(i-sinfo));
			}else{
				//wxLogStatus("deleterange dial %i %i", i, dials);
				Subs->dials.erase(Subs->dials.begin()+(i-styles));
			}
		}	
		//wxLogStatus(" lengths %i %i %i %i %i", Subs->sinfo.size(), Subs->styles.size(),Subs->dials.size(),all, b);
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
				//wxLogStatus("stylesstart %i %i", start-sinfo, stylsize);
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
			lua_pushstring(L, "Nie mozna wstawic nienumerycznego indeksu");
			lua_error(L);
			return 0;
		}

		int n = lua_gettop(L);

		int start = int(lua_tonumber(L, 1)-1);
		
		if(start<0 || start>(int)(Subs->sinfo.size()+Subs->styles.size()+Subs->dials.size()))
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
			int sinfo=Subs->sinfo.size();
			int stylsize=Subs->styles.size();
			int styles=sinfo+stylsize;
			int dialsize=Subs->dials.size();
			int dials=styles+dialsize;
			
			if(e->lclass=="info")
			{
				SInfo *inf=e->info->Copy();
				//wxLogStatus("sinfostart %i %i", start, sinfo);
				if(start >= sinfo){Subs->sinfo.push_back(inf);}
				else{Subs->sinfo.insert(Subs->sinfo.begin()+start, inf);}
				Subs->dsinfo.push_back(inf);
			}
			else if(e->lclass=="style")
			{
				//wxLogStatus("stylesstart %i %i", start-sinfo, stylsize);
				Styles *styl=e->astyle->Copy();
				if(start-sinfo >= stylsize){Subs->styles.push_back(styl);}
				else{Subs->styles.insert(Subs->styles.begin()+(start-sinfo), styl);}
				Subs->dstyles.push_back(styl);
			}
			else if(e->lclass=="dialogue")
			{
				//wxLogStatus("dialstart %i %i", start-styles, Subs->dials.size());
				Dialogue *dial=e->adial->Copy();
				if(start-styles >= dialsize){Subs->dials.push_back(dial);}
				else{Subs->dials.insert(Subs->dials.begin()+(start-styles), dial);}
				Subs->ddials.push_back(dial);
			}
			else{
				
				wxString sclass= "Probujesz wstawiæ linijkê nieznanej klasy";
				lua_pushstring(L, sclass.mb_str(wxConvUTF8).data());
				lua_error(L);
			}
			start++;
			SAFE_DELETE(e);
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

		lua_createtable(L, 0, 6);
		set_field(L, "duration", 0);
		set_field(L, "start_time", 0);
		set_field(L, "end_time", 0);
		set_field(L, "tag", "");
		set_field(L, "text", "");
		set_field(L, "text_stripped", "");
		lua_rawseti(L, -2, kcount++);

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
						lua_createtable(L, 0, 6);
						set_field(L, "duration", kdur);
						set_field(L, "start_time", ktime);
						set_field(L, "end_time", ktime+=kdur);
						set_field(L, "tag", ktag.mb_str(wxConvUTF8).data());
						set_field(L, "text", ktext.mb_str(wxConvUTF8).data());
						set_field(L, "text_stripped", ktext_stripped.mb_str(wxConvUTF8).data());
						lua_rawseti(L, -2, kcount++);
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
		if(kcount<2){
			ktext_stripped = e->adial->Text;
			reg.ReplaceAll(&ktext_stripped,_T(""));

			lua_createtable(L, 0, 6);
			set_field(L, "duration", (e->adial->End.mstime-e->adial->Start.mstime));
			set_field(L, "start_time", e->adial->Start.mstime);
			set_field(L, "end_time", e->adial->End.mstime);
			set_field(L, "tag", "k");
			set_field(L, "text", e->adial->Text.mb_str(wxConvUTF8).data());
			set_field(L, "text_stripped", ktext_stripped.mb_str(wxConvUTF8).data());
			lua_rawseti(L, -2, kcount++);
		}
		//wxLogStatus("text "+e->adial->Text+" stripped "+ktext_stripped+" %i", kcount-1);
		//wxLogStatus(deb);
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
		Notebook::GetTab()->Grid1->GetASSRes(&w, &h);
		push_value(L, w);
		push_value(L, h);
		return 2;
	}

	AutoToFile::AutoToFile(lua_State *_L, File *subsfile, bool _can_modify)
	{
		L=_L;
		can_modify=_can_modify;
		file =subsfile;
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
		lua_getglobal(L, "kainote");

		set_field<&AutoToFile::LuaParseKaraokeData>(L, "parse_karaoke_data");
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

