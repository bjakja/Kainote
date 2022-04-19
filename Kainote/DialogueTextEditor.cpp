//  Copyright (c) 2016-2020, Marcin Drob

//  Kainote is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.

//  Kainote is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.

//  You should have received a copy of the GNU General Public License
//  along with Kainote.  If not, see <http://www.gnu.org/licenses/>.



#include "DialogueTextEditor.h"
#include "GraphicsD2D.h"
#include "BidiConversion.h"
#include "EditBox.h"
#include "Spellchecker.h"
#include "config.h"
#include "Menu.h"
#include <wx/regex.h>
#include <wx/clipbrd.h>
#include "KaiMessageBox.h"
#include "Stylelistbox.h"
#include "SubsFile.h"
#include <regex>
#undef DRAWTEXT;


#undef DrawText

wxDEFINE_EVENT(CURSOR_MOVED, wxCommandEvent);

wxString TextEditor::LuaKeywords[] = { L"function", L"for", L"if", L"while", L"do", L"then", L"end", L"or", L"and", L"repeat", L"until", L"math", L"local" };

TextEditor::TextEditor(wxWindow *parent, int id, bool _spell, const wxPoint& pos, const wxSize& size, long style)
	:wxWindow(parent, id, pos, size, style)
{
	useSpellchecker = _spell;
	SpellCheckerOnOff = (_spell)? Options.GetBool(SPELLCHECKER_ON) : false;
	MText = emptyString;
	bmp = nullptr;
	//fontSize = 10;
	posY = 0;
	scrollPositionV = 0;
	SetCursor(wxCURSOR_IBEAM);
	wxAcceleratorEntry entries[34];
	entries[0].Set(wxACCEL_NORMAL, WXK_DELETE, ID_DEL);
	entries[1].Set(wxACCEL_NORMAL, WXK_BACK, ID_BACK);
	entries[2].Set(wxACCEL_CTRL, WXK_BACK, ID_CBACK);
	entries[3].Set(wxACCEL_CTRL, WXK_DELETE, ID_CDELETE);
	entries[4].Set(wxACCEL_NORMAL, WXK_LEFT, ID_LEFT);
	entries[5].Set(wxACCEL_NORMAL, WXK_RIGHT, ID_RIGHT);
	entries[6].Set(wxACCEL_NORMAL, WXK_UP, ID_UP);
	entries[7].Set(wxACCEL_NORMAL, WXK_DOWN, ID_DOWN);
	entries[8].Set(wxACCEL_CTRL, WXK_LEFT, ID_CLEFT);
	entries[9].Set(wxACCEL_CTRL, WXK_RIGHT, ID_CRIGHT);
	entries[10].Set(wxACCEL_SHIFT, WXK_LEFT, ID_SLEFT);
	entries[11].Set(wxACCEL_SHIFT, WXK_RIGHT, ID_SRIGHT);
	entries[12].Set(wxACCEL_SHIFT, WXK_UP, ID_SUP);
	entries[13].Set(wxACCEL_SHIFT, WXK_DOWN, ID_SDOWN);
	entries[14].Set(wxACCEL_SHIFT | wxACCEL_CTRL, WXK_LEFT, ID_CSLEFT);
	entries[15].Set(wxACCEL_SHIFT | wxACCEL_CTRL, WXK_RIGHT, ID_CSRIGHT);
	entries[16].Set(wxACCEL_CTRL, L'A', ID_CTLA);
	entries[17].Set(wxACCEL_CTRL, L'V', ID_CTLV);
	entries[18].Set(wxACCEL_CTRL, L'C', ID_CTLC);
	entries[19].Set(wxACCEL_CTRL, L'X', ID_CTLX);
	entries[20].Set(wxACCEL_NORMAL, WXK_WINDOWS_MENU, ID_WMENU);
	
	// fix for not working enter for confirm and go to next line
	// but it's not a ideal fix, after change enter it will still work in this field
	entries[21].Set(wxACCEL_NORMAL, WXK_RETURN, EDITBOX_COMMIT_GO_NEXT_LINE);
	int numEntries = 22;
	bool setNumpadAccels = !Options.GetBool(TEXT_FIELD_ALLOW_NUMPAD_HOTKEYS);
	if (setNumpadAccels){
		entries[22].Set(wxACCEL_NORMAL, WXK_NUMPAD0, WXK_NUMPAD0 + 10000);
		entries[23].Set(wxACCEL_NORMAL, WXK_NUMPAD1, WXK_NUMPAD1 + 10000);
		entries[24].Set(wxACCEL_NORMAL, WXK_NUMPAD2, WXK_NUMPAD2 + 10000);
		entries[25].Set(wxACCEL_NORMAL, WXK_NUMPAD3, WXK_NUMPAD3 + 10000);
		entries[26].Set(wxACCEL_NORMAL, WXK_NUMPAD4, WXK_NUMPAD4 + 10000);
		entries[27].Set(wxACCEL_NORMAL, WXK_NUMPAD5, WXK_NUMPAD5 + 10000);
		entries[28].Set(wxACCEL_NORMAL, WXK_NUMPAD6, WXK_NUMPAD6 + 10000);
		entries[29].Set(wxACCEL_NORMAL, WXK_NUMPAD7, WXK_NUMPAD7 + 10000);
		entries[30].Set(wxACCEL_NORMAL, WXK_NUMPAD8, WXK_NUMPAD8 + 10000);
		entries[31].Set(wxACCEL_NORMAL, WXK_NUMPAD9, WXK_NUMPAD9 + 10000);
		numEntries = 32;
	}
	wxAcceleratorTable accel(numEntries, entries);
	SetAcceleratorTable(accel);
	Connect(ID_DEL, ID_WMENU, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&TextEditor::OnAccelerator);
	Connect(EDITBOX_COMMIT_GO_NEXT_LINE, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&TextEditor::OnAccelerator);
	if (setNumpadAccels){
		Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &evt){
			int key = evt.GetId() - 10276;
			wxKeyEvent kevt;
			kevt.m_uniChar = key;
			OnCharPress(kevt);
		}, WXK_NUMPAD0 + 10000, WXK_NUMPAD9 + 10000);
	}
	Cursor.x = Cursor.y = Selend.x = Selend.y = oldstart = oldend = 0;

	holding = dholding = firstdhold = modified = wasDoubleClick = false;
	font = *Options.GetFont();
	fontSize = Options.GetInt(TEXT_EDITOR_FONT_SIZE);
	if (fontSize > 6 && fontSize <= 70 && fontSize != font.GetPointSize()) {
		font.SetPointSize(fontSize);
	}
	else
		fontSize = 10;

	int fw, fh;
	GetTextExtent(L"#TWFfGH", &fw, &fh, nullptr, nullptr, &font);
	fontHeight = fh;
	scroll = new KaiScrollbar(this, 3333, wxDefaultPosition, wxDefaultSize, wxSB_VERTICAL);
	scroll->SetCursor(wxCURSOR_DEFAULT);
	scroll->SetScrollRate(30);
	statusBarHeight = (Options.GetBool(TEXT_EDITOR_HIDE_STATUS_BAR)) ? 0 : fontHeight + 8;
	changeQuotes = Options.GetBool(TEXT_EDITOR_CHANGE_QUOTES);
	caret = new wxCaret(this, 1, fontHeight);
	SetCaret(caret);
	caret->Move(3, 2);
	caret->Show();
	Bind(wxEVT_COMMAND_CHOICE_SELECTED, [=](wxCommandEvent &evt) {
		PutTag();
	}, GetId());
}

TextEditor::~TextEditor()
{
	if (bmp)delete bmp;
}

void TextEditor::SetTextS(const wxString &text, bool modif, bool resetsel, bool noevent, bool BIDIConversion)
{
	modified = modif;
	isRTL = CheckRTL(&text);
	if (isRTL && BIDIConversion) {
		RTLText = text;
		ConvertToLTRChars(&RTLText, &MText);
	}else
		MText = text;

	CalcWraps(modif, (noevent) ? false : modif, BIDIConversion);
	if (!modif)
		CheckText();

	if (resetsel){ SetSelection(0, 0); }
	else{
		if ((size_t)Cursor.x > MText.length()){ Cursor.x = MText.length(); Cursor.y = FindY(Cursor.x); }
		if ((size_t)Selend.x > MText.length()){ Selend.x = MText.length(); Selend.y = FindY(Selend.x); }
		Refresh(false);
	}
}

void TextEditor::CalcWraps(bool updatechars, bool sendevent, bool dontConvertRTL)
{
	
	if (selectionWords.size())
		selectionWords.clear();

	// make it uses every text normally, cause it have to be switched before calc wraps
	if (hasRTL && !dontConvertRTL)
		ConvertToRTL(&MText, &RTLText);
	else if(isRTL && !dontConvertRTL)
		ConvertToRTLChars(&MText, &RTLText);

	wraps.clear();
	wraps.push_back(0);
	size_t textLen = (hasRTL) ? RTLText.length() : MText.length();
	if (textLen > 0){
		int w, h, fw = 0, fh = 0;
		double gcfw = 0.f, gcfh = 0.f;
		GetClientSize(&w, &h);
		if (scroll->IsShown()){
			int sw, sh;
			scroll->GetSize(&sw, &sh);
			w -= sw;
		}
		size_t i = 0;
		/*if (w < 20){
			while (i < textLen){ i++; wraps.push_back(i); }
		}
		else{*/
			GraphicsContext* gc = GetGraphicsContext();
			if (gc){
				gc->SetFont(font, L"#000000");
				CalcWrapsD2D(gc, w);
				delete gc;
			}
			else{
				CalcWrapsGDI(w);
			}
		//}
	}
	else{
		wraps.push_back(textLen);
	}
		
	if (updatechars){
		//check text to generate wraps and cps
		CheckText();

		EB->UpdateChars(); 
	}
	if (sendevent){ wxCommandEvent evt2(wxEVT_COMMAND_TEXT_UPDATED, GetId()); AddPendingEvent(evt2); }
}

void TextEditor::CalcWrapsGDI(int windowWidth)
{
	wxString& text = (hasRTL || isRTL) ? RTLText : MText;
	int gfw, gfh;
	size_t i = 0;
	size_t textLen = text.length();
	size_t lastI = 0;
	int widthCount = 0;
	while (i < textLen)
	{
		const wxUniChar &ch = text[i];
		if (ch == L'\t') {
			i++;
			continue;
		}
		
		const auto &it = fontGDISizes.find(ch);
		if (it != fontGDISizes.end()) {
			widthCount += it->second;
		}
		else {
			GetTextExtent(ch, &gfw, &gfh, nullptr, nullptr, &font);
			fontGDISizes.insert(std::pair<wxUniChar, int>(ch, gfw));
			widthCount += gfw;
		}
		if (widthCount > windowWidth) {
			size_t wrapsSize = wraps.size();
			int minChar = wrapsSize ? wraps[wrapsSize - 1] : 0;
			int j = i - 2;
			bool foundWrap = false;
			//check for possible wraps else return char wrap
			while (minChar < j) {
				const wxUniChar &ch = text[j];
				if (ch == L' ' || ch == L'\\' || ch == L'{' || ch == L'}' || ch == L'(' || ch == L')') {
					size_t g = ch == L'{' || ch == L'\\' || ch == L'(' ? j : j + 1;
					wraps.push_back(g);
					foundWrap = true;
					i = g - 1;
					break;
				}
				j--;
			}
			if (lastI < i || foundWrap) {
				if (!foundWrap) {
					lastI = i;
					wraps.push_back(i - 1);
					i = i - 2;
				}
				widthCount = 0;
			}
		}
		i++;
	}
	if (wraps[wraps.size() - 1] != textLen)
		wraps.push_back(textLen);
}

void TextEditor::CalcWrapsD2D(GraphicsContext *gc, int windowWidth)
{
	wxString& text = (hasRTL || isRTL) ? RTLText : MText;
	double gfw, gfh;
	size_t i = 0;
	size_t textLen = text.length();
	size_t lastI = 1;
	double widthCount = 0.0;
	while (i < textLen)
	{
		//for RTL have to calc wraps from end
		//int RTLi = (hasRTL) ? textLen - 1 - i : i;
		const wxUniChar &ch = text[i];
		if (ch == L'\t') {
			i++;
			continue;
		}

		const auto &it = fontSizes.find(ch);
		if (it != fontSizes.end()){
			widthCount += it->second;
		}
		else{
			gc->GetTextExtent(ch, &gfw, &gfh);
			fontSizes.insert(std::pair<wxUniChar, double>(ch, gfw));
			widthCount += gfw;
		}
		
		if (widthCount > windowWidth){
			size_t wrapsSize = wraps.size();
			int minChar = wrapsSize ? wraps[wrapsSize - 1] : 0;
			int j = i - 2;
			bool foundWrap = false;
			//check for possible wraps else return char wrap
			while (minChar < j) {
				//int RTLj = (hasRTL) ? textLen - 1 - j : j;
				const wxUniChar &ch = text[j];
				if (ch == L' ' || ch == L'\\' || ch == L'{' || ch == L'}' || ch == L'(' || ch == L')') {
					size_t g = ch == L'{' || ch == L'\\' || ch == L'(' ? j : j + 1;
					wraps.push_back(g);
					foundWrap = true;
					//++ is below
					i = g - 1;
					break;
				}
				j--;
			}
			if (lastI < i || foundWrap) {
				if (!foundWrap) {
					lastI = i;
					wraps.push_back(i - 1);
					i = i - 2;
				}
				widthCount = 0.;
			}
			
		}
		i++;
		
	}
	//need to think about this there is too many times used wraps to change it everytime when
	//rtl is used
	if (wraps[wraps.size() - 1] != textLen)
		wraps.push_back(textLen);
}


void TextEditor::OnCharPress(wxKeyEvent& event)
{
	wxUniChar wkey = event.GetUnicodeKey();
	wxUniChar key = wkey;
	/*if (hasRTL && lastKey) {
		key = wxString(lastKey).Lower().at(0);
	}*/
	if (wkey == L'\t'){ return; }
	if (wkey){
		wxString& text = (hasRTL || isRTL) ? RTLText : MText;
		if (Cursor != Selend){
			int curx = Cursor.x;
			int selx = Selend.x;
			if (curx > selx){ int tmp = curx; curx = selx; selx = tmp; }
			text.Remove(curx, selx - curx);
			if (Cursor.x < Selend.x){ Selend = Cursor; }
			else{ Cursor = Selend; }
		}
		
		if (wkey == L'"' && changeQuotes)
			wkey = CheckQuotes();

		bool hasRTLChar = IsRTLCharacter(wkey);
		if (hasRTLChar && !(hasRTL || isRTL)) {
			RTLText = MText;
			isRTL = true;
		}

		bool isInBracket = false;
		if (hasRTL || isRTL) {
			int len = RTLText.length();
			/*size_t startBracket = RTLText.Mid(0, (Cursor.x == 0)? 0 : Cursor.x).Find(L'{', true);
			size_t endBracket = RTLText.Mid((Cursor.x - 2 < 0) ? 0 : Cursor.x - 2).Find(L'}');
			if(endBracket != -1)
				endBracket += (Cursor.x - 2);
			
			if (startBracket < endBracket && startBracket != -1 && endBracket != -1) {
				if (Cursor.x >= len) { RTLText << key; }
				else { RTLText.insert(Cursor.x, 1, key); }
				isInBracket = true;
			}
			else */if(hasRTLChar){
				bool needToReplace = !tempRTLtext.empty();
				tempRTLtext << wkey;
				wxString textConverted;
				ConvertToRTLChars(&tempRTLtext, &textConverted);
				if (Cursor.x >= len) { 
					RTLText << textConverted; 
				}
				else if (needToReplace) {
					RTLText.replace(RTLtextPos.x, RTLtextPos.y, textConverted);
				}
				else { 
					RTLText.insert(Cursor.x, textConverted); 
				}
				RTLtextPos = wxPoint(Cursor.x, textConverted.length());
			}
			else {
				//here have to be written a fancy version ltr for rtl
				tempRTLtext.clear();
				if (Cursor.x >= len) { RTLText << wkey; }
				else { RTLText.insert(Cursor.x, 1, wkey); }
			}
			if (hasRTL)
				ConvertToLTR(&RTLText, &MText);
			else if (isRTL)
				ConvertToLTRChars(&RTLText, &MText);
		}
		else {
			int len = MText.length();
			if (Cursor.x >= len) { MText << wkey; }
			else { MText.insert(Cursor.x, 1, wkey); }
		}
		CalcWraps();
		if (hasRTL || isRTL) {
			int res = iswctype(wint_t(wkey), _SPACE | _PUNCT);
			int nextCharPos = Cursor.x + 1 < text.length() ? Cursor.x + 1 :
				(Cursor.x > 0) ? Cursor.x - 1 : Cursor.x;
			if (isInBracket || !hasRTLChar &&
				!(res != 0 && IsRTLCharacter(text[nextCharPos]))) {
				if (Cursor.x + 1 > wraps[Cursor.y + 1]) { Cursor.y++; }
				Cursor.x++;
			}
			Selend = Cursor;
		}
		else {
			if (Cursor.x + 1 > wraps[Cursor.y + 1]) { Cursor.y++; }
			Cursor.x++;
			Selend = Cursor;
		}
		Refresh(false);
		Update();
		modified = true;
		//tag list
		if (!tagList && (key == L'\\' || (Cursor.x - 2 >= 0 && text[Cursor.x - 2] == L'\\') ||
			(Cursor.x < text.length() && (text[Cursor.x] == L'\\' || text[Cursor.x] == L'}')))){
			//No need to check end cause when there's no end start will take all line
			//No need to show list when it's plain text, someone want to write \h or \N
			wxString partTag;
			bool hasPartTag = false;
			for (int i = Cursor.x - 1; i >= 0; i--){
				if (text[i] == L'}')
					break;
				if (text[i] == L'{'){
					tagList = new PopupTagList(this);
					if (key != L'\\') {
						if (partTag.empty())
							tagList->AppendToKeyword(key);
						else
							tagList->FilterListViaKeyword(partTag);
					}

					/*if (!tagList->GetCount()){
						delete tagList;
						tagList = nullptr;
						return;
					}*/

					//calculate position of popup list
					wxPoint pos;
					pos.y = (Cursor.y * fontHeight) + fontHeight + 5;
					// it should be changed to constant int for avoid bugs
					pos.x = 3;
					int wrap = wraps[Cursor.y];
					if (wrap < Cursor.x){
						GraphicsContext* gc = GetGraphicsContext();
						wxString textBeforeCursor = text.Mid(wrap, Cursor.x - wrap + 1);
						if (gc){
							gc->SetFont(font, L"#FFFFFF");
							double fw, fh;
							gc->GetTextExtent(textBeforeCursor, &fw, &fh);
							pos.x += fw;
							delete gc;
						}
						else{
							wxSize te = GetTextExtent(textBeforeCursor);
							pos.x += te.x;
						}
					}
					tagList->Popup(pos, wxSize(100, fontHeight+10), 0);
					break;
				}
				else if(!hasPartTag){
					if (text[i] == L'\\') {
						hasPartTag = true;
						continue;
					}
					partTag.insert(0, 1, text[i]);
				}
			}
			
		}
		else if (tagList){
			tagList->AppendToKeyword(key);
			if (!tagList->GetCount()){
				delete tagList;
				tagList = nullptr;
				//return;
			}
		}
	}

}

void TextEditor::OnKeyPress(wxKeyEvent& event)
{
	int key = event.GetKeyCode();
	bool ctrl = event.ControlDown();
	bool alt = event.AltDown();
	bool shift = event.ShiftDown();
	if (ctrl && key == L'0'){
		font.SetPointSize(10);
		int fw, fh;
		GetTextExtent(L"#TWFfGH", &fw, &fh, nullptr, nullptr, &font);
		fontHeight = fh;
		caret->SetSize(1, fh);
		CalcWraps(false, false);
		Refresh(false);
		return;
	}

	if ((key == WXK_HOME || key == WXK_END) && !alt){
		Cursor.x = wraps[(key == WXK_END && !ctrl) ? Cursor.y + 1 : Cursor.y];
		if (key == WXK_END && ctrl){ Cursor.x = MText.length(); Cursor.y = wraps.size() - 2; }
		else if (key == WXK_HOME && ctrl){ Cursor.x = 0; Cursor.y = 0; }
		if (!shift){ Selend = Cursor; }
		if (selectionWords.size())
			selectionWords.clear();
		MakeCursorVisible();
	}
	if (key == WXK_PAGEDOWN || key == WXK_PAGEUP || key == WXK_INSERT){
		return;
	}
	if (tagList){
		if (key == WXK_ESCAPE || key == WXK_HOME || key == WXK_END || (ctrl && key == L'0')){
			delete tagList;
			tagList = nullptr;
		}
	}
	if (key == WXK_TAB) {
		wxNavigationKeyEvent evt;
		evt.SetDirection(!event.ShiftDown());
		evt.SetWindowChange(event.ControlDown());
		evt.SetFromTab(true);
		evt.SetEventObject(this);
		wxWindow* win = GetParent();
		while (win) {
			if (win->GetEventHandler()->ProcessEvent(evt))
				break;
			win = win->GetParent();
		}
		return;
	}
	if (((!ctrl && !alt) || (ctrl && alt)) && (key > 30 || key == 0) || (alt && (key >= WXK_NUMPAD0 && key <= WXK_NUMPAD9))){
		lastKey = key;
		event.Skip(); return; 
	}
	lastKey = 0;
}

void TextEditor::OnAccelerator(wxCommandEvent& event)
{
	int step = 0;
	int len;
	int ID = event.GetId();
	if (selectionWords.size())
		selectionWords.clear();
	//maybe only for now
	if (tagList && ID != ID_DOWN && ID != ID_UP && ID != EDITBOX_COMMIT_GO_NEXT_LINE){
		delete tagList;
		tagList = nullptr;
	}
	if (hasRTL) {
		int originalID = ID;
		switch (originalID) {
		case ID_CDELETE:
			ID = ID_CBACK;
			break;
		case ID_CBACK:
			ID = ID_CDELETE;
			break;
		case ID_DEL:
			ID = ID_BACK;
			break;
		case ID_BACK:
			ID = ID_DEL;
			break;
		case ID_LEFT:
			ID = ID_RIGHT;
			break;
		case ID_CLEFT:
			ID = ID_CRIGHT;
			break;
		case ID_SLEFT:
			ID = ID_SRIGHT;
			break;
		case ID_CSLEFT:
			ID = ID_CSRIGHT;
			break;
		case ID_RIGHT:
			ID = ID_LEFT;
			break;
		case ID_CRIGHT:
			ID = ID_CLEFT;
			break;
		case ID_SRIGHT:
			ID = ID_SLEFT;
			break;
		case ID_CSRIGHT:
			ID = ID_CSLEFT;
			break;
		}
	}
	else if(isRTL && ID >= ID_DEL && ID <= ID_CDELETE){
		bool needSwitch = (ID == ID_BACK || ID == ID_CBACK)? IsNextRTLChar(Cursor.x) : IsPrevRTLChar(Cursor.x);

		if (needSwitch) {
			int originalID = ID;
			switch (originalID) {
			case ID_CDELETE:
				ID = ID_CBACK;
				break;
			case ID_CBACK:
				ID = ID_CDELETE;
				break;
			case ID_DEL:
				ID = ID_BACK;
				break;
			case ID_BACK:
				ID = ID_DEL;
				break;
			}
		}
	}
	wxString& text = (hasRTL || isRTL) ? RTLText : MText;
	switch (ID){
	case ID_CDELETE:
	case ID_CBACK:
		//len = MText.length();
		if ((ID == ID_CBACK && Cursor.x == 0) || (ID == ID_CDELETE && Cursor.x >= (int)text.length())){ return; }
		Selend.x = Cursor.x;
		if (ID == ID_CBACK){
			FindWord((Cursor.x < 2) ? 0 : Cursor.x - 1, &Cursor.x, &len);
			if (Cursor.x == 1 && text[0] == L' '){ Cursor.x--; }
		}
		else{
			FindWord(Cursor.x, &len, &Selend.x);
		}
	case ID_DEL:
	case ID_BACK:
		if (Cursor != Selend){
			int curx = Cursor.x;
			int selx = Selend.x;
			if (curx > selx){ int tmp = curx; curx = selx; selx = tmp; }
			text.Remove(curx, selx - curx);
			Selend = Cursor;
			/*if (hasRTL)
				ConvertToLTR(&RTLText, &MText);
			CalcWraps(true, true, true);*/
			SetSelection(curx, curx);
		}
		else
		{
			if (ID == ID_BACK){
				if (Cursor.x < 1){ return; }
				Cursor.x--;
			}
			if (ID == ID_DEL && Cursor.x >= (int)text.length()){ return; }
			text.Remove(Cursor.x, 1);
		}
		len = wraps.size();
		if(hasRTL)
			ConvertToLTR(&RTLText, &MText);
		else if(isRTL)
			ConvertToLTRChars(&RTLText, &MText);

		CalcWraps();

		if (Cursor.x < wraps[Cursor.y] || (Cursor.x == wraps[Cursor.y] && len != wraps.size())){ 
			//block to go cursor.y to -1
			if(Cursor.y > 0)
				Cursor.y--; 
		}
		else if (Cursor.x>wraps[Cursor.y + 1]){ Cursor.y++; }
		Selend = Cursor;
		Refresh(false);
		Update();
		modified = true;
		break;

	case ID_LEFT:
	case ID_CLEFT:
	case ID_SLEFT:
	case ID_CSLEFT:
		if (ID == ID_LEFT && Selend.x < Cursor.x){ Cursor = Selend; Refresh(false); return; }
		if (Cursor.x < 1){ return; }
		if (ID == ID_CLEFT || ID == ID_CSLEFT){
			FindWord(Cursor.x - 1, &Cursor.x, 0);
		}
		if (Cursor.x - 1 < wraps[Cursor.y] && Cursor.x != 0){ Cursor.y--; }
		else if (ID != ID_CLEFT && ID != ID_CSLEFT){ Cursor.x--; }


		if (ID < ID_SLEFT){ Selend = Cursor; }
		Refresh(false);
		break;

	case ID_RIGHT:
	case ID_CRIGHT:
	case ID_SRIGHT:
	case ID_CSRIGHT:
		if (ID == ID_RIGHT && Selend.x>Cursor.x){ Cursor = Selend; Refresh(false); return; }
		if (Cursor.x >= (int)text.length()){ return; }
		if (ID == ID_CRIGHT || ID == ID_CSRIGHT){
			if (Cursor.x == text.length() - 1){
				Cursor.x++;
			}
			else{
				FindWord(Cursor.x + 1, 0, &Cursor.x);
			}
		}
		if (Cursor.x + 1 > wraps[Cursor.y + 1] && Cursor.y < (int)wraps.size() - 2){ Cursor.y++; }
		else if (ID != ID_CRIGHT && ID != ID_CSRIGHT){ Cursor.x++; }

		if (ID < ID_SRIGHT){ Selend = Cursor; }
		Refresh(false);
		break;

	case ID_DOWN:
	case ID_SDOWN:
		if (tagList && ID == ID_DOWN){
			int sel = tagList->GetSelection();
			sel += 1;
			tagList->SetSelection(sel);
			break;
		}
		len = text.length();
		if (Cursor.y >= (int)wraps.size() - 2){ Cursor.y = wraps.size() - 2; Cursor.x = len; }
		else{
			Cursor.x -= wraps[Cursor.y];
			Cursor.y++;
			Cursor.x += wraps[Cursor.y];
			if (Cursor.x>len){ Cursor.x = len; }
		}

		if (ID < ID_SDOWN){ Selend = Cursor; }
		Refresh(false);
		break;

	case ID_UP:
	case ID_SUP:
		if (tagList && ID == ID_UP){
			int sel = tagList->GetSelection();
			sel -= 1;
			tagList->SetSelection(sel);
			break;
		}
		//if(Cursor.y<1){return;}
		Cursor.x -= wraps[Cursor.y];
		Cursor.y--;
		if (Cursor.y < 1){ Cursor.y = 0; Cursor.x = 0; }
		else{ Cursor.x += wraps[Cursor.y]; }

		if (ID < ID_SUP){ Selend = Cursor; }
		Refresh(false);

		break;
	case ID_CTLA:

		Cursor.x = Cursor.y = 0;
		Selend.x = text.length(); Selend.y = wraps.size() - 2;
		Refresh(false);
		break;
	case ID_CTLV:
		Paste();
		break;
	case ID_CTLC:
	case ID_CTLX:

		Copy(ID > ID_CTLC);

		break;
	case ID_WMENU:
		//Selend=Cursor;
		ContextMenu(PosFromCursor(Cursor), FindError(Cursor, false));
		break;
	/*case ID_CTRL_ALT_R:
		if (!hasRTL) {
			hasRTL = true;
			CalcWraps(false, false);
			CheckText();
			Refresh(false);
		}
		else {
			event.Skip();
		}
		break;
	case ID_CTRL_ALT_L:
		if (hasRTL) {
			hasRTL = false;
			CalcWraps(false, false);
			CheckText();
			Refresh(false);
		}
		else {
			event.Skip();
		}
		break;*/
	case EDITBOX_COMMIT_GO_NEXT_LINE:
		if (tagList)
			PutTag();
		else
			event.Skip();
			//EB->OnNewline(event);

		return;
	default:

		break;
	}
	if (ID != ID_CTLC || ID != ID_WMENU){ 
		if (isRTL || hasRTL)
			tempRTLtext.clear();
		wxCommandEvent evt(CURSOR_MOVED, GetId()); AddPendingEvent(evt); 
	}
}

void TextEditor::OnMouseEvent(wxMouseEvent& event)
{
	bool click = event.LeftDown();
	bool leftup = event.LeftUp();
	if (event.ButtonDown()){ 
		if (tagList){
			delete tagList;
			tagList = nullptr;
		}
		SetFocus(); 
		if (!click){ Refresh(false); } 
	}
	wxSize size = GetClientSize();
	size.y -= statusBarHeight;
	wxPoint mousePosition = event.GetPosition();
	bool isInField = (size.y >= mousePosition.y);

	if (numberChangingMousePos != -1 && !event.ShiftDown())
		numberChangingMousePos = -1;

	if (leftup && (holding || dholding)){
		holding = dholding = false;
		if (HasCapture()){ ReleaseMouse(); }
		return;
	}



	if (event.LeftDClick() && MText != emptyString && isInField){
		if (isRTL || hasRTL)
			tempRTLtext.clear();
		wasDoubleClick = true;
		dclickCurPos = mousePosition;
		time = timeGetTime();
		int errn = FindError(mousePosition);
		if (Options.GetBool(EDITBOX_SUGGESTIONS_ON_DOUBLE_CLICK) && errn >= 0){
			wxString err = misspells[errn].misspell;

			wxArrayString suggs;
			SpellChecker::Get()->Suggestions(err, suggs);

			KaiListBox lw(this, suggs, _("Sugestie poprawy"));
			if (lw.ShowModal() == wxID_OK)
			{
				wxString suggestion = lw.GetSelection();
				int newto = 0;
				SpellChecker::Get()->ReplaceMisspell(err, suggestion,
					misspells[errn].posStart, misspells[errn].posEnd, &MText, &newto);
				modified = true;
				CalcWraps();
				SetSelection(newto, newto);
				EB->Send(EDITBOX_SPELL_CHECKER, false);
				modified = false;
			}
			return;
		}
		int start, end;
		wxPoint ht;
		if (Cursor.x < Selend.x){ Selend = Cursor; }
		else{ Cursor = Selend; }
		HitTest(mousePosition, &ht);
		FindWord(ht.x, &start, &end);
		wxString wordstriped = MText.SubString(start, end - 1);
		size_t wlen = wordstriped.length();
		wordstriped.Trim();
		SeekSelected(wordstriped);
		end -= (wlen - wordstriped.length());
		oldend = tmpend = end;
		oldstart = tmpstart = start;
		SetSelection(start, end);
		firstdhold = dholding = true;
		CaptureMouse();
		return;
	}

	if (click && isInField){
		if (selectionWords.size())
			selectionWords.clear();
		wxPoint cur;
		HitTest(mousePosition, &cur);
		if (cur != Cursor){ 
			if (isRTL || hasRTL)
				tempRTLtext.clear();
			wxCommandEvent evt(CURSOR_MOVED, GetId()); 
			AddPendingEvent(evt); 
		}
		Cursor = cur;
		if (!event.ShiftDown()){ Selend = Cursor; }
		if (wasDoubleClick){
			wasDoubleClick = false;

			if (timeGetTime() - time < 800 && dclickCurPos == mousePosition){
				Cursor.x = 0;
				Cursor.y = 0;
				Selend.x = MText.length();
				Selend.y = FindY(Selend.x);
				MakeCursorVisible();
				return;
			}
		}
		Refresh(false);
		holding = true;
		CaptureMouse();
	}

	if (holding){
		wxPoint cur;
		HitTest(mousePosition, &cur);
		Cursor = cur;
		MakeCursorVisible();
	}
	if (dholding){
		wxPoint cur;
		int start, end;
		HitTest(mousePosition, &cur);
		FindWord(cur.x, &start, &end);
		if ((start == tmpstart && end == tmpend)){ return; }
		tmpstart = start; tmpend = end;

		if (start < oldstart){
			if (end == oldstart){
				Selend.x = oldend; 
				Selend.y = FindY(oldend); 
			}
			Cursor.x = start;
			Cursor.y = FindY(start);
			selectionWords.clear();
		}
		else{
			if (oldstart == start){ 
				Selend.x = oldstart; 
				Selend.y = FindY(oldstart); 
			}
			Cursor.x = end;
			Cursor.y = FindY(end);
			selectionWords.clear();
		}
		MakeCursorVisible();
	}

	if (event.RightUp() && isInField && event.GetModifiers() == 0)
		ContextMenu(mousePosition, FindError(mousePosition));
	if (event.RightUp() && HasCapture())
		ReleaseMouse();

	bool mouseWheel = event.GetWheelRotation() != 0;
	bool rightdown = event.RightDown();
	if (event.ShiftDown() && (mouseWheel || rightdown || event.RightIsDown())){
		//if some number existing
		if (rightdown){
			dclickCurPos = mousePosition;
			CaptureMouse();
			return;
		}
		else if (!mouseWheel && (dclickCurPos.y < mousePosition.y + 8 && dclickCurPos.y > mousePosition.y - 8) && (dclickCurPos.x < mousePosition.x + 10 && dclickCurPos.x > mousePosition.x - 10)){
			return;
		}
		wxPoint CursorPos(numberChangingMousePos, -1);
		if (numberChangingMousePos != -1 || HitTest(mousePosition, &CursorPos)){
			numberChangingMousePos = CursorPos.x;
			wxPoint numberPos;
			float floatNumber;
			float step;
			if (GetNumberFromCursor(CursorPos.x, numberPos, floatNumber, step)){
				if (mouseWheel){
					int mouseStep = event.GetWheelRotation() / event.GetWheelDelta();
					if (mouseStep < 0)
						step = -step;
				}
				else{
					if (abs(dclickCurPos.y - mousePosition.y) < abs(dclickCurPos.x - mousePosition.x)) {
						if(dclickCurPos.x > mousePosition.x)
							step = -(step * 10);
						else
							step = (step * 10);
					}
					else if (dclickCurPos.y < mousePosition.y)
						step = -step;

					dclickCurPos = mousePosition;
				}

				floatNumber += step;

				Replace(numberPos.x, numberPos.y + 1, getfloat(floatNumber, L"10.3f"));
			}
		}
	}

	if (mouseWheel) {
		if (event.ControlDown()){
			fontSize += event.GetWheelRotation() / event.GetWheelDelta();
			if (fontSize < 7 || fontSize > 70){ fontSize = MID(7, fontSize, 70); return; }
			font.SetPointSize(fontSize);
			int fw, fh;
			GetTextExtent(L"#TWFfGH", &fw, &fh, nullptr, nullptr, &font);
			fontHeight = fh;
			caret->SetSize(1, fh);
			fontSizes.clear();
			CalcWraps(false, false);
			Refresh(false);
			statusBarHeight = (Options.GetBool(TEXT_EDITOR_HIDE_STATUS_BAR)) ? 0 : fontHeight + 8;
			Options.SetInt(TEXT_EDITOR_FONT_SIZE, fontSize);
		}
		else if (event.GetModifiers() == 0){
			int step = 30 * event.GetWheelRotation() / event.GetWheelDelta();
			if (step > 0 && scrollPositionV == 0){ return; }
			scrollPositionV = MAX(scrollPositionV - step, 0);
			Refresh(false);
		}
	}
}

void TextEditor::OnSize(wxSizeEvent& event)
{
	wxSize size = GetClientSize();
	if (size.y < 80 && statusBarHeight > 0)
		statusBarHeight = 0;
	else if (size.y >= 80 && !Options.GetBool(TEXT_EDITOR_HIDE_STATUS_BAR))
		statusBarHeight = fontHeight + 8;

	CalcWraps(false, false);
	Cursor.y = FindY(Cursor.x);
	Selend.y = FindY(Selend.x);
	MakeCursorVisible();
}

int TextEditor::FindY(int x)
{
	for (size_t p = 1; p < wraps.size(); p++){ if (x < wraps[p]){ return (p - 1); } }
	return wraps.size() - 2;
}

void TextEditor::OnPaint(wxPaintEvent& event)
{
	int w = 0, h = 0;
	GetClientSize(&w, &h);
	if (w < 1 || h < 1){ return; }
	wxRegionIterator upd(GetUpdateRegion());
	while (upd) {
		wxRect rect(upd.GetRect());
		upd++;
		//draw bitmap fragment, only use for
		//Windows repaint draw existed bitmap
		//instead of making a new
		//when window size changed don't use part redraw
		if ((rect.width < w || rect.height < h) && bmp && lastWidth == w && lastHeight == h) {
			wxMemoryDC tdc;
			tdc.SelectObject(*bmp);
			wxPaintDC dc(this);
			dc.Blit(rect.x, rect.y, rect.width, rect.height, &tdc, rect.x, rect.y);
			if (!upd) return;
		}
	}
	lastWidth = w;
	lastHeight = h;
	int bitmaph = (wraps.size() * fontHeight) + 4;
	int windoww = w;
	if (bitmaph > h){
		if (!scroll->IsShown()){
			scroll->Show();
			CalcWraps(false, false);
			Cursor.y = FindY(Cursor.x);
			Selend.y = FindY(Selend.x);
			bitmaph = (wraps.size() * fontHeight) + 4;
		}
		int sw = scroll->GetThickness();
		scroll->SetSize(w - sw, 0, sw, h);
		int diff = h - statusBarHeight;
		int diff2 = bitmaph;
		if (scrollPositionV > diff2 - diff){ scrollPositionV = diff2 - diff; }
		scroll->SetScrollbar(scrollPositionV, diff, diff2, diff - 2);
		w -= sw;
		if (w < 0){ return; }
	}
	else{
		if (scroll->IsShown()){
			scroll->Hide();
			CalcWraps(false, false);
			Cursor.y = FindY(Cursor.x);
			Selend.y = FindY(Selend.x);
		}
		bitmaph = h;
		scrollPositionV = 0;
	}

	if (bmp) {
		if (bmp->GetWidth() < w || bmp->GetHeight() < h) {
			delete bmp;
			bmp = nullptr;
		}
	}

	if (!bmp) bmp = new wxBitmap(w, h);
	// Draw bitmap
	wxMemoryDC bmpDC;
	bmpDC.SelectObject(*bmp);

	GraphicsContext* gc = GetGraphicsContext(bmpDC);

	if (!gc){
		DrawFieldGDI(bmpDC, w, h - statusBarHeight, h);
		
	}
	else{
		DrawFieldD2D(gc, w, h - statusBarHeight, h);
		delete gc;
	}
	wxPaintDC dc(this);
	dc.Blit(0, 0, w, h, &bmpDC, 0, 0);
	//caret->Move(3, 3);
}

void TextEditor::DrawFieldD2D(GraphicsContext *gc, int w, int h, int windowh)
{
	double fw = 0.f, fh = 0.f;
	bool tags = false;
	bool slash = false;
	bool val = false;
	bool templateString = false;
	bool templateCode = state == 3;
	bool isTemplateLine = state > 1;

	const wxColour & ctvariables = Options.GetColour(EDITOR_TEMPLATE_VARIABLES);
	const wxColour & ctext = templateCode ? ctvariables : Options.GetColour(EDITOR_TEXT);
	const wxColour & ccurlybraces = Options.GetColour(EDITOR_CURLY_BRACES);
	const wxColour & coperators = Options.GetColour(EDITOR_TAG_OPERATORS);
	const wxColour & cnames = Options.GetColour(EDITOR_TAG_NAMES);
	const wxColour & cvalues = Options.GetColour(EDITOR_TAG_VALUES);
	const wxColour & bgbraces = Options.GetColour(EDITOR_BRACES_BACKGROUND);
	const wxColour & cbackground = Options.GetColour(EDITOR_BACKGROUND);
	const wxColour & cselection = Options.GetColour(EDITOR_SELECTION);
	const wxColour & cselnofocus = Options.GetColour(EDITOR_SELECTION_NO_FOCUS);
	const wxColour & cspellerrors = Options.GetColour(EDITOR_SPELLCHECKER);
	const wxColour & ctcodemarks = Options.GetColour(EDITOR_TEMPLATE_CODE_MARKS);
	const wxColour & ctfunctions = Options.GetColour(EDITOR_TEMPLATE_FUNCTIONS);
	const wxColour & ctkeywords = Options.GetColour(EDITOR_TEMPLATE_KEYWORDS);
	const wxColour & ctstrings = Options.GetColour(EDITOR_TEMPLATE_STRINGS);
	const wxColour & cphrasesearch = Options.GetColour(EDITOR_PHRASE_SEARCH);
	const wxColour & csplitanddrawings = Options.GetColour(EDITOR_SPLIT_LINES_AND_DRAWINGS);


	gc->SetBrush(cbackground);
	gc->SetPen(*wxTRANSPARENT_PEN);
	gc->DrawRectangle(0, 0, w, h);

	wxString digits = L"(0123456789-&+";
	wxString tagtest;
	wxString parttext;
	wxString measureText;

	posY = 2;
	posY -= scrollPositionV;
	int posX = 3;
	if (hasRTL)
		posX = w - 3;

	bool isfirst = true;
	int wline = 1;
	int rtlwline = 0;
	int wchar = 0;
	bool hasFocus = HasFocus();
	const wxString& fieldText = (hasRTL || isRTL) ? RTLText : MText;
	wxString alltext = fieldText + L" ";
	int len = alltext.length();
	const wxUniChar &bchar = alltext[Cursor.x];
	if (bchar == L'{')
	{
		Brackets.x = Cursor.x;
		Brackets.y = FindBracket(L'{', L'}', Cursor.x + 1);
	}
	else if (bchar == L'}')
	{
		Brackets.y = Cursor.x;
		Brackets.x = FindBracket(L'}', L'{', Cursor.x - 1, true);
	}
	else if (bchar == L'(')
	{
		Brackets.x = Cursor.x;
		Brackets.y = FindBracket(L'(', L')', Cursor.x + 1);
	}
	else if (bchar == L')')
	{
		Brackets.y = Cursor.x;
		Brackets.x = FindBracket(L')', L'(', Cursor.x - 1, true);
	}
	else{ Brackets.x = -1; Brackets.y = -1; }

	gc->SetFont(font, L"#000000");
	double fww;
	gc->SetPen(*wxTRANSPARENT_PEN);
	//drawing spellchecker
	if (SpellCheckerOnOff){
		gc->SetBrush(cspellerrors);
		DrawWordRectangles(0, gc, h, posX);
	}
	if (selectionWords.size()){
		gc->SetBrush(cphrasesearch);
		DrawWordRectangles(1, gc, h, posX);
	}
	else if (Cursor.x != Selend.x || Cursor.y != Selend.y){
		Brackets.x = -1; Brackets.y = -1;
		wxPoint fst, scd;
		if ((Cursor.x + Cursor.y) > (Selend.x + Selend.y)){ fst = Selend; scd = Cursor; }
		else{ fst = Cursor, scd = Selend; }

		gc->SetBrush(wxBrush(wxColour(hasFocus ? cselection : cselnofocus)));
		fww = 0.0;
		//skip unneeded selection drawing
		int lineStart = (scrollPositionV - 2) / fontHeight;
		int charStart = wraps[lineStart];
		int lineEnd = ((scrollPositionV - 2 + h) / fontHeight) + 1;
		int charEnd = (lineEnd < wraps.size()) ? wraps[lineEnd] : fieldText.length();
		//drawing selection
		for (int j = fst.y; j <= scd.y; j++){
			int endWrap = wraps[j + 1];
			if (endWrap < charStart)
				continue;

			int startWrap = wraps[j];
			if (startWrap > charEnd)
				break;

			if (j == fst.y){
				fw = 0.0;
				/*if (hasRTL) {
					if (fst.y == scd.y) {
						wxString ftext = fieldText.SubString(startWrap, fst.x - 1);
						ftext.Replace(L"\t", emptyString);
						if (fst.x - 1 <= endWrap){ gc->GetTextExtent(ftext, &fw, &fh); }
					}
					wxString stext = fieldText.SubString((fst.y == scd.y)? fst.x : startWrap, (fst.y == scd.y) ? scd.x - 1 : fst.x - 1);
					stext.Replace(L"\t", emptyString);
					gc->GetTextExtent(stext, &fww, &fh);
				}
				else {*/
					wxString ftext = fieldText.SubString(startWrap, fst.x - 1);
					ftext.Replace(L"\t", emptyString);
					if (startWrap <= fst.x - 1) { gc->GetTextExtent(ftext, &fw, &fh); }
					wxString stext = fieldText.SubString(fst.x, (fst.y == scd.y) ? scd.x - 1 : endWrap - 1);
					stext.Replace(L"\t", emptyString);
					gc->GetTextExtent(stext, &fww, &fh);
				//}
			}
			else{
				fw = 0.0;
				/*if(hasRTL){
					if (j == scd.y) {
						wxString ftext = fieldText.SubString(startWrap, scd.x - 1);
						ftext.Replace(L"\t", emptyString);
						if (fst.x - 1 <= endWrap) { gc->GetTextExtent(ftext, &fw, &fh); }
					}
					wxString stext = fieldText.SubString((j == scd.y) ? scd.x - 1 : startWrap, endWrap - 1);
					stext.Replace(L"\t", emptyString);
					gc->GetTextExtent(stext, &fww, &fh);
				}
				else {*/
					wxString stext = fieldText.SubString(startWrap, (j == scd.y) ? scd.x - 1 : endWrap - 1);
					stext.Replace(L"\t", emptyString);
					gc->GetTextExtent(stext, &fww, &fh);
				//}
			}
			if (hasRTL) {
				wxString rowText = fieldText.Mid(startWrap, endWrap - startWrap);
				double fww1 = 0.f;
				gc->GetTextExtent(rowText, &fww1, &fh);
				gc->DrawRectangle(fw + posX - fww1, ((j * fontHeight) + 1) - scrollPositionV, fww, fontHeight);
			}
			else {
				gc->DrawRectangle(fw + posX, ((j * fontHeight) + 1) - scrollPositionV, fww, fontHeight);
			}
		}
	}
	//skip unneeded lines, start one line before to avoid not colored values on start
	bool hasDrawing = false;
	int lineStart = ((scrollPositionV - 2) / fontHeight) - 1;
	int charStart = 0;
	if (lineStart > 0){
		charStart = wraps[lineStart];
		
		wxString seekingPart = fieldText.Mid(0, charStart);
		size_t endBracket = seekingPart.Find(L'}', true);
		size_t startBracket = seekingPart.Find(L'{', true);
		if ((endBracket == -1 && startBracket != -1) || startBracket > endBracket){
			tags = slash = val = true;
		}
		if (startBracket == -1 || startBracket < endBracket) {
			wxString tagText = fieldText.Mid(startBracket, endBracket - startBracket + 1);
			wxRegEx re("\\\\p([0-9]+)", wxRE_ADVANCED | wxRE_ICASE);
			if (re.IsValid() && re.Matches(tagText)) {
				wxString result = re.GetMatch(tagText, 1);
				if (!result.empty()) {
					if (result[0] > L'0' && result[0] < L'9') {
						hasDrawing = true;
					}
				}
			}
		}
		wline = lineStart + 1;
		rtlwline = wline - 1;
		if (rtlwline < 0)
			rtlwline = 0;
		posY += (lineStart * fontHeight);
		wchar = charStart + lineStart;
	}

	bool cursorWasSet = false;
	bool hasSplit = false;
	//Drawing text
	for (int i = charStart; i < len; i++){
		if (posY > h)
			break;

		const wxUniChar &ch = alltext[i];
		
		if (wline < wraps.size() && i == wraps[wline]){
			if (Cursor.x + Cursor.y == wchar){
				double fww = 0.f;
				wxString text = measureText + parttext;
				if (!text.empty())
					gc->GetTextExtent(text, &fww, &fh);
				caret->Move(fww + posX, posY);
				cursorWasSet = true;
			}

			if (parttext != emptyString){
				gc->GetTextExtent(measureText, &fw, &fh);
				wxColour fontColor = (val || (isTemplateLine && IsNumberFloat(parttext) && (tags || templateCode))) ? cvalues : (slash) ? cnames :
					(templateString) ? ctstrings : (isTemplateLine && ch == L'(') ? ctfunctions :
					(isTemplateLine && CheckIfKeyword(parttext)) ? ctkeywords : 
					templateCode ? ctvariables : hasDrawing ? csplitanddrawings : ctext;
				gc->SetFont(font, fontColor);
				if (hasRTL || isRTL) {
					double chfw = 0.;
					double chpos = 0.;
					for (size_t i = 0; i < parttext.size(); i++) {
						const wxUniChar& ch = parttext[i];
						gc->DrawTextU(ch, fw + posX + chpos, posY);
						gc->GetTextExtent(ch, &chfw, &fh);
						chpos += chfw;
					}
				}
				else {
					gc->DrawTextU(parttext, fw + posX, posY);
				}
			}

			posY += fontHeight;
			wline++;
			wchar++;
			parttext.clear();
			measureText.clear();
		}

		if (hasRTL && wline > 0 && wline < wraps.size() && i == wraps[wline - 1]) {
			int startWrap = wraps[wline - 1];
			wxString rowText = fieldText.Mid(startWrap, wraps[wline] - startWrap);
			double fww = 0.f;
			gc->GetTextExtent(rowText, &fww, &fh);
			posX = w - fww - 3;
			//rtlwline++;
		}

		if (hasFocus && (Cursor.x + Cursor.y == wchar)){
			if (measureText + parttext == emptyString) { fw = 0.0; }
			else {
				if (hasRTL) {
					int start = i - (measureText + parttext).length();
					wxString measureTxt = RTLText.Mid(start, i - start);
					gc->GetTextExtent(measureTxt, &fw, &fh);
				}
				else {
					gc->GetTextExtent(measureText + parttext, &fw, &fh); 
				}
			}
			caret->Move(fw + posX, posY);
			cursorWasSet = true;
		}
		if (hasFocus && (i == Brackets.x || i == Brackets.y)){
			int bry = FindY(i);
			wxColour col = bgbraces;
			if (Brackets.x == -1 || Brackets.y == -1){ col = cspellerrors; }
			gc->SetBrush(wxBrush(col));
			wxString text = fieldText.SubString(wraps[bry], i - 1);
			text.Replace(L"\t", emptyString);
			if (i > 0){ gc->GetTextExtent(text, &fw, &fh); }
			else{ fw = 0; }
			gc->GetTextExtent(fieldText[i], &fww, &fh);
			gc->DrawRectangle(fw + posX, ((bry*fontHeight) + 2) - scrollPositionV, fww, fontHeight);
			wxFont fnt = font;
			fnt = fnt.Bold();
			gc->SetFont(fnt, (ch == L'{' || ch == L'}') ? ccurlybraces : coperators);
			gc->DrawTextU(fieldText[i], fw + posX, ((bry*fontHeight) + 2) - scrollPositionV);
			gc->SetFont(font, ctext);

		}
		if (isTemplateLine){
			if (!templateString && (ch == L'!' || (ch == L'.' && !(IsNumberFloat(parttext) || val)) || ch == L',' ||
				ch == L'+' || ch == L'-' || ch == L'=' || ch == L'(' || ch == L')' || ch == L'>' || ch == L'<' || 
				ch == L'[' || ch == L']' || ch == L'*' || ch == L'/' || ch == L':' || ch == L';' || ch == L'~')){
				gc->GetTextExtent(measureText, &fw, &fh);
				gc->SetFont(font, (IsNumberFloat(parttext) || val) ? cvalues : (slash) ? cnames :
					(ch == L'(' && !slash) ? ctfunctions : (CheckIfKeyword(parttext)) ? ctkeywords : ctvariables);
				gc->DrawTextU(parttext, fw + posX, posY);
				measureText << parttext;
				parttext.clear();
				gc->GetTextExtent(measureText, &fw, &fh);
				gc->SetFont(font, (ch == L'!') ? ctcodemarks : coperators);
				gc->DrawTextU(ch, fw + posX, posY);
				measureText << ch;

				if (state == 2 && ch == L'!')
					templateCode = !templateCode;
				slash = val = false;
				wchar++;
				continue;
			}

			if (ch == L'"'){
				if (templateString){
					parttext << ch;
					gc->GetTextExtent(measureText, &fw, &fh);
					gc->SetFont(font, ctstrings);
					gc->DrawTextU(parttext, fw + posX, posY);
					measureText << parttext;
					parttext.clear();
					templateString = !templateString;
					wchar++;
					continue;
				}
				templateString = !templateString;
			}
			if (!templateString && ch == L' '){
				gc->GetTextExtent(measureText, &fw, &fh);
				gc->SetFont(font, (!templateCode && !val && !slash) ? ctext : (IsNumberFloat(parttext) || val) ? cvalues :
					(slash) ? cnames : (CheckIfKeyword(parttext)) ? ctkeywords : ctvariables);
				gc->DrawTextU(parttext, fw + posX, posY);
				measureText << parttext;
				parttext.clear();
				measureText << ch;

				slash = val = false;
				wchar++;
				continue;
			}
		}
		if (ch != L'\t'){
			parttext << ch;
		}

		if (templateString){
			wchar++;
			continue;
		}
		if (hasSplit) {
			if (ch == L'N' || ch == L'n' || ch == L'h') {
				gc->GetTextExtent(measureText, &fw, &fh);
				gc->SetFont(font, csplitanddrawings);
				gc->DrawTextU(parttext, fw + posX, posY);
				measureText << parttext;
				parttext.clear();
			}
			hasSplit = false;
		}

		if (ch == L'{' || ch == L'}' || (ch == L'\\' && !tags)){
			if (ch == L'{' || ch == L'\\'){
				tags = ch == L'{';
				hasSplit = ch == L'\\';
				wxString bef = parttext.BeforeLast(ch);
				gc->GetTextExtent(measureText, &fw, &fh);
				gc->SetFont(font, hasDrawing? csplitanddrawings : ctext);
				if (hasRTL || isRTL) {
					double chfw = 0.;
					double chpos = 0.;
					for (size_t i = 0; i < bef.size(); i++) {
						const wxUniChar& ch1 = bef[i];
						gc->DrawTextU(ch1, fw + posX + chpos, posY);
						gc->GetTextExtent(ch1, &chfw, &fh);
						chpos += chfw;
					}
				}
				else {
					gc->DrawTextU(bef, fw + posX, posY);
				}
				measureText << bef;
				parttext = ch;
			}
			else{
				wxString &tmp = parttext.RemoveLast(1);
				gc->GetTextExtent(measureText, &fw, &fh);
				gc->SetFont(font, (val) ? cvalues : (slash) ? cnames : ctext);
				gc->DrawTextU(tmp, fw + posX, posY);
				measureText << tmp;
				parttext = ch;
				tags = slash = val = false;
			}
			if (ch != L'\\') {
				gc->GetTextExtent(measureText, &fw, &fh);
				gc->SetFont(font, ccurlybraces);
				gc->DrawTextU(parttext, fw + posX, posY);
				measureText << parttext;
				parttext.clear();
				val = false;
			}
		}

		if (slash){
			tagtest += ch;
			if ((digits.Find(ch) != -1 && tagtest != L"1" && tagtest != L"2" && tagtest != L"3" && tagtest != L"4") || tagtest == L"fn" || ch == L'('){
				slash = false;
				//block pos tag
				if (tagtest.StartsWith("p") && ch != L'(') {
					wxString pnumstr = tagtest.Mid(1);
					int pnum = wxAtoi(pnumstr);
					hasDrawing = pnum > 0;
				}
				wxString tmp = (tagtest == L"fn") ? parttext : parttext.RemoveLast(1);
				gc->GetTextExtent(measureText, &fw, &fh);
				gc->SetFont(font, cnames);
				gc->DrawTextU(tmp, fw + posX, posY);
				measureText << tmp;
				if (tagtest == L"fn"){ parttext.clear(); }
				else{ parttext = ch; }
				val = true;
				tagtest.clear();
			}
		}

		if ((ch == L'\\' || ch == L'(' || ch == L')' || ch == L',') && tags){
			wxString tmp = parttext.RemoveLast(1);
			gc->GetTextExtent(measureText, &fw, &fh);
			gc->SetFont(font, (val && (ch == L'\\' || ch == L')' || ch == L',')) ? cvalues : slash ? cnames : ctext);
			gc->DrawTextU(tmp, fw + posX, posY);
			measureText << tmp;
			parttext = ch;
			if (ch == L'\\'){ slash = true; }
			gc->GetTextExtent(measureText, &fw, &fh);
			gc->SetFont(font, coperators);
			gc->DrawTextU(parttext, fw + posX, posY);
			measureText << parttext;
			parttext.clear();
			if (ch == L'('){ val = true; slash = false; }
			else if (ch != L','){ val = false; }
			//continue;
		}

		wchar++;
	}
	if (!cursorWasSet){
		caret->Move(0, -50);
	}
	const wxColour &border = Options.GetColour(hasFocus ? EDITOR_BORDER_ON_FOCUS : EDITOR_BORDER);
	//here we go our status bar
	if (statusBarHeight > 0){
		gc->SetBrush(cbackground);
		gc->SetPen(wxPen(border));
		gc->SetFont(font, ctext);
		gc->DrawRectangle(0, h, w, statusBarHeight);
		int ypos = ((statusBarHeight - fontHeight) / 2) + h;
		double lnfw, lifw, lfw, colfw, selfw;
		gc->GetTextExtent(L"Length: 10000000", &lnfw);
		gc->GetTextExtent(L"Lines: 100000", &lifw);
		gc->GetTextExtent(L"Ln: 100000", &lfw);
		gc->GetTextExtent(L"Col: 10000", &colfw);
		gc->GetTextExtent(L"Sel: 10000000", &selfw);
		gc->DrawTextU(wxString::Format(L"Length: %i", (int)fieldText.length()), 5, ypos);
		gc->DrawTextU(wxString::Format(L"Lines: %i", (int)wraps.size() - 1), lnfw + 5, ypos);
		gc->DrawTextU(wxString::Format(L"Ln: %i", Cursor.y + 1), lnfw + lifw + 5, ypos);
		gc->DrawTextU(wxString::Format(L"Col: %i", Cursor.x - wraps[Cursor.y] + 1), lnfw + lifw + lfw + 5, ypos);
		gc->DrawTextU(wxString::Format(L"Sel: %i", abs(Selend.x - Cursor.x)), lnfw + lifw + lfw + colfw + 5, ypos);
		gc->DrawTextU(wxString::Format(L"Ch: %i", Cursor.x + 1), lnfw + lifw + lfw + colfw + selfw + 5, ypos);
	}
	//text field border
	gc->SetBrush(*wxTRANSPARENT_BRUSH);
	gc->SetPen(wxPen(border));
	gc->DrawRectangle(0, 0, w - 1, windowh - 1);
}

void TextEditor::DrawFieldGDI(wxDC &dc, int w, int h, int windowh)
{
	int fw = 0, fh = 0;
	bool tags = false;
	bool slash = false;
	bool val = false;
	bool templateString = false;
	bool templateCode = state == 3;
	bool isTemplateLine = state > 1;

	const wxColour & ctvariables = Options.GetColour(EDITOR_TEMPLATE_VARIABLES);
	const wxColour & ctext = templateCode ? ctvariables : Options.GetColour(EDITOR_TEXT);
	const wxColour & ccurlybraces = Options.GetColour(EDITOR_CURLY_BRACES);
	const wxColour & coperators = Options.GetColour(EDITOR_TAG_OPERATORS);
	const wxColour & cnames = Options.GetColour(EDITOR_TAG_NAMES);
	const wxColour & cvalues = Options.GetColour(EDITOR_TAG_VALUES);
	const wxColour & bgbraces = Options.GetColour(EDITOR_BRACES_BACKGROUND);
	const wxColour & cbackground = Options.GetColour(EDITOR_BACKGROUND);
	const wxColour & cselection = Options.GetColour(EDITOR_SELECTION);
	const wxColour & cselnofocus = Options.GetColour(EDITOR_SELECTION_NO_FOCUS);
	const wxColour & cspellerrors = Options.GetColour(EDITOR_SPELLCHECKER);
	const wxColour & ctcodemarks = Options.GetColour(EDITOR_TEMPLATE_CODE_MARKS);
	const wxColour & ctfunctions = Options.GetColour(EDITOR_TEMPLATE_FUNCTIONS);
	const wxColour & ctkeywords = Options.GetColour(EDITOR_TEMPLATE_KEYWORDS);
	const wxColour & ctstrings = Options.GetColour(EDITOR_TEMPLATE_STRINGS);
	const wxColour & cphrasesearch = Options.GetColour(EDITOR_PHRASE_SEARCH);
	const wxColour & csplitanddrawings = Options.GetColour(EDITOR_SPLIT_LINES_AND_DRAWINGS);

	dc.SetBrush(cbackground);
	dc.SetPen(*wxTRANSPARENT_PEN);
	dc.DrawRectangle(0, 0, w, h);

	wxString digits = L"(0123456789-&+";
	wxString tagtest;
	wxString parttext;
	wxString mestext;

	posY = 2;
	posY -= scrollPositionV;
	int posX = 3;
	if (hasRTL)
		posX = w - 3;

	bool isfirst = true;
	int wline = 1;
	int wchar = 0;
	bool hasFocus = HasFocus();

	dc.SetFont(font);
	const wxString& fieldText = (hasRTL || isRTL) ? RTLText : MText;
	wxString alltext = fieldText + L" ";
	int len = alltext.length();
	const wxUniChar &bchar = alltext[Cursor.x];
	if (bchar == L'{')
	{
		Brackets.x = Cursor.x;
		Brackets.y = FindBracket(L'{', L'}', Cursor.x + 1);
	}
	else if (bchar == L'}')
	{
		Brackets.y = Cursor.x;
		Brackets.x = FindBracket(L'}', L'{', Cursor.x - 1, true);
	}
	else if (bchar == L'(')
	{
		Brackets.x = Cursor.x;
		Brackets.y = FindBracket(L'(', L')', Cursor.x + 1);
	}
	else if (bchar == L')')
	{
		Brackets.y = Cursor.x;
		Brackets.x = FindBracket(L')', L'(', Cursor.x - 1, true);
	}
	else{ Brackets.x = -1; Brackets.y = -1; }

	int fww;
	dc.SetPen(*wxTRANSPARENT_PEN);
	//rysowanie spellcheckera
	if (SpellCheckerOnOff){
		dc.SetBrush(cspellerrors);
		DrawWordRectangles(0, dc, h, posX);
	}
	if (selectionWords.size()){
		dc.SetBrush(cphrasesearch);
		DrawWordRectangles(1, dc, h, posX);
	}
	else if (Cursor.x != Selend.x || Cursor.y != Selend.y){
		Brackets.x = -1; Brackets.y = -1;
		wxPoint fst, scd;
		if ((Cursor.x + Cursor.y) > (Selend.x + Selend.y)){ fst = Selend; scd = Cursor; }
		else{ fst = Cursor, scd = Selend; }

		dc.SetBrush(wxBrush(wxColour(hasFocus ? cselection : cselnofocus)));
		fww = 0;
		//skip unnided selection drawing
		int lineStart = (scrollPositionV - 2) / fontHeight;
		int charStart = wraps[lineStart];
		int lineEnd = ((scrollPositionV - 2 + h) / fontHeight) + 1;
		int charEnd = (lineEnd < wraps.size()) ? wraps[lineEnd] : fieldText.length();
		//drawing selection
		for (int j = fst.y; j <= scd.y; j++){
			int endWrap = wraps[j + 1];
			if (endWrap < charStart)
				continue;

			int startWrap = wraps[j];
			if (startWrap > charEnd)
				break;

			if (j == fst.y){
				wxString ftext = fieldText.SubString(startWrap, fst.x - 1);
				ftext.Replace(L"\t", emptyString);
				if (startWrap > fst.x - 1){ fw = 0; }
				else{ GetTextExtent(ftext, &fw, &fh, nullptr, nullptr, &font); }
				wxString stext = fieldText.SubString(fst.x, (fst.y == scd.y) ? scd.x - 1 : endWrap - 1);
				stext.Replace(L"\t", emptyString);
				GetTextExtent(stext, &fww, &fh, nullptr, nullptr, &font);
			}
			else{
				fw = 0;
				wxString stext = fieldText.SubString(startWrap, (j == scd.y) ? scd.x - 1 : endWrap - 1);
				stext.Replace(L"\t", emptyString);
				GetTextExtent(stext, &fww, &fh, nullptr, nullptr, &font);
			}
			if (hasRTL) {
				wxString rowText = fieldText.Mid(startWrap, endWrap - startWrap);
				int fww1 = 0.f;
				GetTextExtent(rowText, &fww1, &fh, nullptr, nullptr, &font);
				dc.DrawRectangle(fw + posX - fww1, ((j * fontHeight) + 1) - scrollPositionV, fww, fontHeight);
			}
			else {
				dc.DrawRectangle(fw + 3, ((j * fontHeight) + 1) - scrollPositionV, fww, fontHeight);
			}
		}
	}
	//skip unneeded lines, start one line before to avoid not colored values on start
	int lineStart = ((scrollPositionV - 2) / fontHeight) - 1;
	int charStart = 0;
	if (lineStart > 0){
		charStart = wraps[lineStart];

		wxString seekingPart = fieldText.Mid(0, charStart);
		size_t endBracket = seekingPart.Find(L'}', true);
		size_t startBracket = seekingPart.Find(L'{', true);
		if (endBracket == -1 || startBracket > endBracket)
			tags = slash = val = true;

		wline = lineStart + 1;
		posY += (lineStart * fontHeight);
		wchar = charStart + lineStart;
	}
	bool cursorWasSet = false;
	bool hasSplit = false;
	bool hasDrawing = false;
	//Drawing text
	for (int i = charStart; i < len; i++){
		if (posY > h)
			break;

		const wxUniChar &ch = alltext[i];


		if (i == wraps[wline]){
			if (Cursor.x + Cursor.y == wchar){
				int fww = 0;
				GetTextExtent(mestext + parttext, &fww, &fh, nullptr, nullptr, &font);
				caret->Move(fww + posX, posY);
				cursorWasSet = true;
			}

			if (parttext != emptyString){
				GetTextExtent(mestext, &fw, &fh, nullptr, nullptr, &font);
				wxColour kol = (val || (isTemplateLine && IsNumberFloat(parttext) && (tags || templateCode))) ? cvalues : (slash) ? cnames :
					(templateString) ? ctstrings : (isTemplateLine && ch == L'(') ? ctfunctions :
					(isTemplateLine && CheckIfKeyword(parttext)) ? ctkeywords : templateCode ? ctvariables : 
					hasDrawing ? csplitanddrawings : ctext;
				dc.SetTextForeground(kol);
				mestext << parttext;
				dc.DrawText(parttext, fw + posX, posY);
				if (hasRTL || isRTL) {
					int chfw = 0;
					int chpos = 0;
					for (size_t i = 0; i < parttext.size(); i++) {
						const wxUniChar& ch = parttext[i];
						dc.DrawText(ch, fw + posX + chpos, posY);
						GetTextExtent(ch, &chfw, &fh, nullptr, nullptr, &font);
						chpos += chfw;
					}
				}
				else {
					dc.DrawText(parttext, fw + posX, posY);
				}
			}

			posY += fontHeight;
			wline++;
			wchar++;
			parttext.clear();
			mestext.clear();
		}

		if (hasFocus && (Cursor.x + Cursor.y == wchar)){
			if (mestext + parttext == emptyString){ fw = 0; }
			else{ 
				if (hasRTL) {
					int start = i - (mestext + parttext).length();
					wxString measureTxt = RTLText.Mid(start, i - start);
					GetTextExtent(measureTxt, &fw, &fh, nullptr, nullptr, &font);
				}
				else {
					GetTextExtent(mestext + parttext, &fw, &fh, nullptr, nullptr, &font);
				}
			}
			caret->Move(fw + posX, posY);
			cursorWasSet = true;
		}
		if (hasFocus && (i == Brackets.x || i == Brackets.y)){
			int bry = FindY(i);
			wxColour col = bgbraces;
			if (Brackets.x == -1 || Brackets.y == -1){ col = cspellerrors; }
			dc.SetBrush(wxBrush(col));
			//dc.SetPen(wxPen(col));
			wxString text = fieldText.SubString(wraps[bry], i - 1);
			text.Replace(L"\t", emptyString);
			if (i > 0){ GetTextExtent(text, &fw, &fh, nullptr, nullptr, &font); }
			else{ fw = 0; }
			GetTextExtent(fieldText[i], &fww, &fh, nullptr, nullptr, &font);
			dc.DrawRectangle(fw + posX, ((bry*fontHeight) + 2) - scrollPositionV, fww, fontHeight);
			wxFont fnt = dc.GetFont();
			fnt = fnt.Bold();
			dc.SetFont(fnt);
			dc.SetTextForeground((ch == L'{' || ch == L'}') ? ccurlybraces : coperators);
			dc.DrawText(fieldText[i], fw + posX, ((bry*fontHeight) + 2) - scrollPositionV);
			dc.SetFont(font);

		}
		if (isTemplateLine){
			if (!templateString && (ch == L'!' || (ch == L'.' && !(IsNumberFloat(parttext) || val)) || ch == L',' || ch == L'+' || ch == L'-' || ch == L'=' || ch == L'(' ||
				ch == L')' || ch == L'>' || ch == L'<' || ch == L'[' || ch == L']' || ch == L'*' || ch == L'/' || ch == L':' || ch == L';')){
				GetTextExtent(mestext, &fw, &fh, nullptr, nullptr, &font);
				dc.SetTextForeground((IsNumberFloat(parttext) || val) ? cvalues : (slash) ? cnames :
					(ch == L'(' && !slash) ? ctfunctions : (CheckIfKeyword(parttext)) ? ctkeywords : ctvariables);
				dc.DrawText(parttext, fw + posX, posY);
				mestext << parttext;
				parttext.clear();
				GetTextExtent(mestext, &fw, &fh, nullptr, nullptr, &font);
				dc.SetTextForeground((ch == L'!') ? ctcodemarks : coperators);
				dc.DrawText(ch, fw + posX, posY);
				mestext << ch;
				if (state == 2 && ch == L'!')
					templateCode = !templateCode;
				slash = val = false;
				wchar++;
				continue;
			}

			if (ch == L'"'){
				if (templateString){
					parttext << ch;
					GetTextExtent(mestext, &fw, &fh, nullptr, nullptr, &font);
					dc.SetTextForeground(ctstrings);
					dc.DrawText(parttext, fw + posX, posY);
					mestext << parttext;
					parttext.clear();
					templateString = !templateString;
					wchar++;
					continue;
				}
				templateString = !templateString;
			}
			if (!templateString && ch == L' '){
				GetTextExtent(mestext, &fw, &fh, nullptr, nullptr, &font);
				dc.SetTextForeground((!templateCode && !val && !slash) ? ctext : (IsNumberFloat(parttext) || val) ? cvalues :
					(slash) ? cnames : (CheckIfKeyword(parttext)) ? ctkeywords : ctvariables);
				dc.DrawText(parttext, fw + posX, posY);
				mestext << parttext;
				parttext.clear();
				mestext << ch;
				slash = val = false;
				wchar++;
				continue;
			}
		}
		if (ch != L'\t'){
			parttext << ch;

		}

		if (templateString){
			wchar++;
			continue;
		}
		if (hasSplit) {
			if (ch == L'N' || ch == L'n' || ch == L'h') {
				GetTextExtent(mestext, &fw, &fh, nullptr, nullptr, &font);
				dc.SetTextForeground(csplitanddrawings);
				dc.DrawText(parttext, fw + posX, posY);
				mestext << parttext;
				parttext.clear();
			}
			hasSplit = false;
		}

		if (ch == L'{' || ch == L'}' || (ch == L'\\' && !tags)) {
			if (ch == L'{' || ch == L'\\') {
				tags = ch == L'{';
				hasSplit = ch == L'\\';
				wxString bef = parttext.BeforeLast(ch);
				GetTextExtent(mestext, &fw, &fh, nullptr, nullptr, &font);
				dc.SetTextForeground(hasDrawing ? csplitanddrawings : ctext);
				if (hasRTL || isRTL) {
					int chfw = 0;
					int chpos = 0;
					for (size_t i = 0; i < bef.size(); i++) {
						const wxUniChar& ch = bef[i];
						dc.DrawText(ch, fw + posX + chpos, posY);
						GetTextExtent(ch, &chfw, &fh, nullptr, nullptr, &font);
						chpos += chfw;
					}
				}
				else {
					dc.DrawText(bef, fw + posX, posY);
				}
				mestext << bef;
				parttext = ch;
			}
			else{
				wxString &tmp = parttext.RemoveLast(1);
				GetTextExtent(mestext, &fw, &fh, nullptr, nullptr, &font);
				dc.SetTextForeground((val) ? cvalues : (slash) ? cnames : ctext);
				dc.DrawText(tmp, fw + posX, posY);
				mestext << tmp;
				parttext = ch;
				tags = slash = val = false;
			}
			if (ch != L'\\') {
				GetTextExtent(mestext, &fw, &fh, nullptr, nullptr, &font);
				dc.SetTextForeground(ccurlybraces);
				dc.DrawText(parttext, fw + posX, posY);
				mestext << parttext;
				parttext.clear();
				val = false;
			}
		}

		if (slash){
			tagtest += ch;
			if ((digits.Find(ch) != -1 && tagtest != L"1" && tagtest != L"2" && tagtest != L"3" && tagtest != L"4") || 
				tagtest == L"fn" || ch == L'('){
				slash = false;
				//block pos tag
				if (tagtest.StartsWith("p") && ch != L'(') {
					wxString pnumstr = tagtest.Mid(1);
					int pnum = wxAtoi(pnumstr);
					hasDrawing = pnum > 0;
				}
				wxString tmp = (tagtest == L"fn") ? parttext : parttext.RemoveLast(1);
				GetTextExtent(mestext, &fw, &fh, nullptr, nullptr, &font);
				dc.SetTextForeground(cnames);
				dc.DrawText(tmp, fw + posX, posY);
				mestext << tmp;
				if (tagtest == L"fn"){ parttext.clear(); }
				else{ parttext = ch; }
				val = true;
				tagtest.clear();
			}
		}

		if ((ch == L'\\' || ch == L'(' || ch == L')' || ch == L',') && tags){
			wxString tmp = parttext.RemoveLast(1);
			GetTextExtent(mestext, &fw, &fh, nullptr, nullptr, &font);
			dc.SetTextForeground((val && (ch == L'\\' || ch == L')' || ch == L',')) ? cvalues : slash ? cnames : ctext);
			dc.DrawText(tmp, fw + posX, posY);
			mestext << tmp;
			parttext = ch;
			if (ch == L'\\'){ slash = true; }
			GetTextExtent(mestext, &fw, &fh, nullptr, nullptr, &font);
			dc.SetTextForeground(coperators);
			dc.DrawText(parttext, fw + posX, posY);
			mestext << parttext;
			parttext.clear();
			if (ch == L'('){ val = true; slash = false; }
			else if (ch != L','){ val = false; }
			//continue;
		}

		wchar++;
	}
	if (!cursorWasSet){
		caret->Move(0, -50);
	}
	const wxColour &border = Options.GetColour(hasFocus ? EDITOR_BORDER_ON_FOCUS : EDITOR_BORDER);
	//here we go our status bar
	if (statusBarHeight > 0){
		dc.SetBrush(cbackground);
		dc.SetPen(wxPen(border));
		dc.SetTextForeground(ctext);
		dc.DrawRectangle(0, h, w, statusBarHeight);
		int ypos = ((statusBarHeight - fontHeight) / 2) + h;
		int fh, lnfw, lifw, lfw, colfw, selfw;
		dc.GetTextExtent(L"Length: 10000000", &lnfw, &fh);
		dc.GetTextExtent(L"Lines: 100000", &lifw, &fh);
		dc.GetTextExtent(L"Ln: 100000", &lfw, &fh);
		dc.GetTextExtent(L"Col: 10000", &colfw, &fh);
		dc.GetTextExtent(L"Sel: 10000000", &selfw, &fh);
		dc.DrawText(wxString::Format(L"Length: %i", (int)fieldText.length()), 5, ypos);
		dc.DrawText(wxString::Format(L"Lines: %i", (int)wraps.size() - 1), lnfw + 5, ypos);
		dc.DrawText(wxString::Format(L"Ln: %i", Cursor.y + 1), lnfw + lifw + 5, ypos);
		dc.DrawText(wxString::Format(L"Col: %i", Cursor.x - wraps[Cursor.y] + 1), lnfw + lifw + lfw + 5, ypos);
		dc.DrawText(wxString::Format(L"Sel: %i", abs(Selend.x - Cursor.x)), lnfw + lifw + lfw + colfw + 5, ypos);
		dc.DrawText(wxString::Format(L"Ch: %i", Cursor.x + 1), lnfw + lifw + lfw + colfw + selfw + 5, ypos);
	}
	//text field border
	dc.SetBrush(*wxTRANSPARENT_BRUSH);
	dc.SetPen(wxPen(border));
	dc.DrawRectangle(0, 0, w, windowh);
}

bool TextEditor::HitTest(wxPoint pos, wxPoint *cur)
{
	int fw = 0, fh = 0;
	pos.y += (scrollPositionV);
	pos.x -= 2;

	cur->y = (pos.y / fontHeight);
	if (cur->y < 0 || wraps.size() < 2){ cur->y = 0; cur->x = 0; return false; }
	if (cur->y >= (int)wraps.size() - 1)
	{
		cur->y = wraps.size() - 2; cur->x = wraps[cur->y];
	}
	else{ cur->x = wraps[cur->y]; }

	bool find = false;
	wxString& rtltext = (hasRTL || isRTL) ? RTLText : MText;
	wxString txt = rtltext + L" ";

	int wlen = rtltext.length();
	int fw1 = 0;
	double gcfw = 0.f, gcfh = 0.f, gcfw1 = 0.f;
	GraphicsContext* gc = GetGraphicsContext();
	if (gc)
		gc->SetFont(font, L"#000000");

	int curEnd = wraps[cur->y + 1];
	int lineLen = curEnd - cur->x;
	if (hasRTL) {
		
		wxString rowText = rtltext.Mid(cur->x, curEnd - cur->x);
		int w = 0, h = 0;
		GetSize(&w, &h);
		
		if (gc) {
			double fwidth = 0.0, fheight = 0.0;
			gc->GetTextExtent(rowText, &fwidth, &fheight);
			pos.x -= (w - fwidth - 8);
		}
		else {
			int fwidth = 0, fheight = 0;
			GetTextExtent(rowText, &fwidth, &fheight, nullptr, nullptr, &font);
			pos.x -= (w - fwidth - 8);
		}
	}
	
	for (int i = cur->x; i < curEnd + 1; i++)
	{
		wxString text = txt.SubString(cur->x, i);
		text.Replace(L"\t", emptyString);
		if (gc){
			gc->GetTextExtent(text, &gcfw, &gcfh);
			if (gcfw + 1.f - ((gcfw - gcfw1) / 2.f) > pos.x){
				cur->x = i; 
				find = true; 
				break; 
			}
			gcfw1 = gcfw;
		}
		else{
			GetTextExtent(text, &fw, &fh, nullptr, nullptr, &font);
			if (fw + 1 - ((fw - fw1) / 2) > pos.x) {
				cur->x = i;
				find = true;
				break;
			}
			fw1 = fw;
		}
	}
	if (!find){ cur->x = wraps[cur->y + 1]; }

	if (gc)
		delete gc;
	return find;
}

bool TextEditor::IsModified()
{
	return modified;
}

void TextEditor::SetModified(bool _modified)
{
	modified = _modified; 
};

void TextEditor::GetSelection(long *start, long *end)
{
	bool iscur = ((Cursor.x + Cursor.y) > (Selend.x + Selend.y));
	*start = (!iscur) ? Cursor.x : Selend.x;
	*end = (iscur) ? Cursor.x : Selend.x;
}

void TextEditor::SetSelection(int start, int end, bool noEvent)
{
	int len = MText.length();
	end = MID(0, end, len);
	start = MID(0, start, len);
	if ((Cursor.x != end || Selend.x != start) && !noEvent){ 
		wxCommandEvent evt(CURSOR_MOVED, GetId()); 
		AddPendingEvent(evt); 
	}
	Cursor.x = end;
	Selend.x = start;
	Selend.y = FindY(Selend.x);
	Cursor.y = FindY(Cursor.x);
	if(isRTL || hasRTL)
		tempRTLtext.clear();

	Refresh(false);
}

wxString TextEditor::GetValue(bool BIDIConversion) const
{
	if (BIDIConversion) {
		if (hasRTL) {
			wxString result;
			wxString MtextCopy = MText;
			ConvertToRTLChars(&MtextCopy, &result);
			return result;
		}
		else if(isRTL)
			return RTLText;

	}

	return MText;
}

void TextEditor::Replace(int start, int end, const wxString &rep)
{
	modified = true;
	wxString& text = (hasRTL || isRTL) ? RTLText : MText;
	text.replace(start, end - start, rep);
	if (hasRTL)
		ConvertToLTR(&RTLText, &MText);
	else if (isRTL)
		ConvertToLTRChars(&RTLText, &MText);

	CalcWraps(true, true, true);
	if ((size_t)Cursor.x > text.length()){ Cursor.x = text.length(); Cursor.y = FindY(Cursor.x); }
	if ((size_t)Selend.x > text.length()){ Selend.x = text.length(); Selend.y = FindY(Selend.x); }
	Refresh(false);
}

void TextEditor::CheckText()
{
	if (MText == emptyString) { errors.SetEmpty(); return; }
	errors.clear();
	misspells.clear();
	errors.Init2((hasRTL || isRTL)? RTLText : MText, SpellCheckerOnOff, EB->GetFormat(), &misspells);
}

wxUniChar TextEditor::CheckQuotes()
{
	wxString beforeCursor = MText.Mid(0, Cursor.x);
	long long startQuote = beforeCursor.Find(L'„', true);
	long long endQuote = beforeCursor.Find(L'”', true);
	if (startQuote > endQuote && startQuote != -1)
		return L'”';

	return L'„';
}

void TextEditor::OnKillFocus(wxFocusEvent& event)
{
	if (tagList){
		delete tagList;
		tagList = nullptr;
	}
	Refresh(false);
}

void TextEditor::FindWord(int pos, int *start, int *end)
{
	wxString& text = (hasRTL) ? RTLText : MText;
	int len = text.length();
	if (len < 1){ Cursor.x = Cursor.y = 0; *start = 0; *end = 0; return; }
	bool fromend = (start != nullptr);

	if (!fromend){ pos--; }
	pos = MID(0, pos, len - 1);
	bool hasres = false;
	int lastres = -1;
	if (fromend){
		*start = (fromend) ? 0 : len;
		for (int i = pos; i >= 0; i--){
			const wxUniChar &ch = text[i];
			int res = iswctype(wint_t(ch), _SPACE | _PUNCT);
			if (lastres == 8 && res != 0 && i + 2 <= pos && text[i + 1] == L' '){
				*start = i + 2;
				break;
			}
			if (res != 0){ lastres = res; }
			if (res != 0 && !hasres){
				if (i == pos){ hasres = true; continue; }
				bool isen = (text[i] == L'\\' && text[i + 1] == L'N');
				*start = (isen && pos == i + 1) ? i : (isen) ? i + 2 : i + 1;
				break;
			}
			else if (hasres && res == 0){
				if (lastres < 1 && (i + 2 == pos || i + 1 == pos)){ hasres = false; continue; }
				*start = ((lastres>3 && lastres < 6 && i + 2 <= pos) || i + 1 == pos || text[i + 2] == L' ') ? i + 1 : i + 2;
				break;
			}
		}
	}
	if (!end){ return; }
	*end = (fromend && end == nullptr) ? 0 : len;
	for (int i = pos; i < len; i++){
		const wxUniChar &ch = text[i];
		int res = iswctype(wint_t(ch), _SPACE | _PUNCT);
		if (res == 8 /*&& i > pos*/){
			*end = i + 1;
			break;
		}
		if (res != 0 && !hasres){
			if (i == pos){ hasres = true; continue; }
			*end = (res == 8) ? i + 1 : i;
			break;
		}
		else if (hasres && res == 0){
			*end = (i > 0 && text[i - 1] == L'\\' && text[i] == L'N') ? i + 1 : i;
			break;
		}
	}


}

void TextEditor::ContextMenu(wxPoint mpos, int error)
{
	Menu menut;
	wxString err;
	wxArrayString suggs;
	if (error >= 0){ err = misspells[error].misspell; }
	if (!err.IsEmpty()){
		SpellChecker::Get()->Suggestions(err, suggs);
		for (size_t i = 0; i < suggs.size(); i++){
			menut.Append(i + 30200, suggs[i]);
		}

		if (suggs.size() > 0){ menut.AppendSeparator(); }
	}


	menut.Append(TEXTM_COPY, _("&Kopiuj"))->Enable(Selend.x != Cursor.x);
	menut.Append(TEXTM_CUT, _("Wy&tnij"))->Enable(Selend.x != Cursor.x);
	menut.Append(TEXTM_PASTE, _("&Wklej"));

	menut.AppendSeparator();
	menut.Append(TEXTM_SEEKWORDL, _("Szukaj tłumaczenia słowa na ling.pl"))->Enable(Selend.x != Cursor.x);
	menut.Append(TEXTM_SEEKWORDB, _("Szukaj tłumaczenia słowa na pl.ba.bla"))->Enable(Selend.x != Cursor.x);
	menut.Append(TEXTM_SEEKWORDG, _("Szukaj zaznaczonej frazy w Google"))->Enable(Selend.x != Cursor.x);
	menut.Append(TEXTM_SEEKWORDS, _("Szukaj synonimu na synonimy.net"))->Enable(Selend.x != Cursor.x);

	wxArrayString dictionarySymbols;
	int numOfLanguages = 0;
	if (useSpellchecker){
		wxArrayString dics;
		SpellChecker::AvailableDics(dics, dictionarySymbols);
		numOfLanguages = dics.size();
		const wxString &language = Options.FindLanguage(Options.GetString(DICTIONARY_LANGUAGE));
		Menu *languageMenu = new Menu();
		menut.Append(MENU_SPELLCHECKER_ON, _("Sprawdzanie pisowni"), emptyString, true, 
			nullptr, nullptr, ITEM_CHECK_AND_HIDE)->Check(Options.GetBool(SPELLCHECKER_ON));
		for (int k = 0; k < numOfLanguages; k++){
			languageMenu->Append(MENU_SPELLCHECKER_ON + k + 1, dics[k], emptyString, true, 
				nullptr, nullptr, (language == dics[k])? ITEM_RADIO : ITEM_NORMAL);
		}
		menut.Append(MENU_SPELLCHECKER_ON - 1, _("Zainstalowane języki"), languageMenu);
	}

	if (!err.IsEmpty()){
		menut.Append(TEXTM_ADD, wxString::Format(_("&Dodaj słowo \"%s\" do słownika"), err));
	}

	menut.Append(TEXTM_DEL, _("&Usuń"))->Enable(Selend.x != Cursor.x);
	menut.Append(MENU_SHOW_STATUS_BAR, _("Pokaż pasek stanu"), nullptr, emptyString, 
		ITEM_CHECK)->Check(!Options.GetBool(TEXT_EDITOR_HIDE_STATUS_BAR));
	menut.Append(MENU_CHANGE_QUOTES, _("Automatycznie zamieniaj cudzysłów"), 
		nullptr, emptyString, ITEM_CHECK)->Check(Options.GetBool(TEXT_EDITOR_CHANGE_QUOTES));
	
	Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &evt){
		MenuItem * item = (MenuItem*)evt.GetClientData();
		if (!item)
			return;
		CONFIG optionName = (item->GetId() == MENU_SHOW_STATUS_BAR) ? TEXT_EDITOR_HIDE_STATUS_BAR : TEXT_EDITOR_CHANGE_QUOTES;
		//value swapped for working without default config
		bool itemChecked = item->IsChecked();
		if (optionName == TEXT_EDITOR_HIDE_STATUS_BAR){
			statusBarHeight = (itemChecked) ? fontHeight + 8 : 0;
			Options.SetBool(optionName, !itemChecked);
			Refresh(false);
		}
		else{
			Options.SetBool(optionName, itemChecked);
			changeQuotes = itemChecked;
		}
		Options.SaveOptions(true, false);

	}, ID_CHECK_EVENT);

	int id = -1;
	id = menut.GetPopupMenuSelection(mpos, this);
	if (id < 0)return;
	if (id >= 30200){
		int newto = 0;
		SpellChecker::Get()->ReplaceMisspell(err, suggs[id - 30200],
			misspells[error].posStart, misspells[error].posEnd, &MText, &newto);

		modified = true;
		CalcWraps();
		SetSelection(newto, newto);
		EB->Send(EDITBOX_SPELL_CHECKER, false);
		modified = false;
	}
	else if (id == TEXTM_COPY){
		Copy();
	}
	else if (id == TEXTM_CUT){
		Copy(true);
	}
	else if (id == TEXTM_PASTE){
		Paste();
	}
	else if (id == TEXTM_DEL){
		long from, to;
		GetSelection(&from, &to);
		MText.Remove(from, to - from);
		CalcWraps();
		SetSelection(from, from); modified = true;
	}
	else if (id == TEXTM_ADD && !err.IsEmpty()){
		bool succ = SpellChecker::Get()->AddWord(err);
		if (!succ){ KaiMessageBox(wxString::Format(_("Błąd, słowo \"%s\" nie zostało dodane."), err)); }
		else{ CheckText(); EB->ClearErrs(); Refresh(false); }
	}
	else if (id >= TEXTM_SEEKWORDL && id <= TEXTM_SEEKWORDS){
		wxString page = (id == TEXTM_SEEKWORDL) ? L"http://ling.pl/" :
			(id == TEXTM_SEEKWORDB) ? L"http://pl.bab.la/slownik/angielski-polski/" :
			(id == TEXTM_SEEKWORDG) ? L"https://www.google.com/search?q=" :
			L"http://synonim.net/synonim/";
		long from, to;
		GetSelection(&from, &to);
		wxString word = MText.SubString(from, to - 1).Trim();

		word.Replace(L" ", L"+");
		wxString url = page + word;
		OpenInBrowser(url);

	}
	else if (id >= MENU_SPELLCHECKER_ON && useSpellchecker && id <= (MENU_SPELLCHECKER_ON + numOfLanguages)){
		if (id == MENU_SPELLCHECKER_ON){
			MenuItem * item = menut.FindItem(id);
			if (item){
				SpellCheckerOnOff = item->IsChecked();
				Options.SetBool(SPELLCHECKER_ON, SpellCheckerOnOff);
				EB->ClearErrs(true, SpellCheckerOnOff);
			}
		}
		else{
			MenuItem * item = menut.FindItem(id);
			if (item){
				//first element starts from MENU_SPELLCHECKER_ON + 1
				Options.SetString(DICTIONARY_LANGUAGE, dictionarySymbols[id - MENU_SPELLCHECKER_ON - 1]);
				SpellChecker::Destroy();
				EB->ClearErrs();
			}
		}
		
	}


}

void TextEditor::Copy(bool cut)
{
	if (Selend.x == Cursor.x){ return; }
	int curx = Cursor.x; int selx = Selend.x; if (curx > selx){ int tmp = curx; curx = selx; selx = tmp; }
	wxString &text = (hasRTL || isRTL) ? RTLText : MText;
	wxString whatcopy = text.SubString(curx, selx - 1);
	if(hasRTL)
		ConvertToLTR(&whatcopy);
	else if (isRTL)
		ConvertToLTRChars(&whatcopy);

	if (wxTheClipboard->Open())
	{
		wxTheClipboard->SetData(new wxTextDataObject(whatcopy));
		wxTheClipboard->Close();
		wxTheClipboard->Flush();
	}
	if (cut){
		text.Remove(curx, selx - curx);
		if (hasRTL)
			ConvertToLTR(&RTLText, &MText);
		else if (isRTL)
			ConvertToLTRChars(&RTLText, &MText);

		CalcWraps(true, true, true);
		SetSelection(curx, curx);
		modified = true;
	}
}


void TextEditor::Paste()
{
	if (wxTheClipboard->Open())
	{
		if (wxTheClipboard->IsSupported(wxDF_TEXT))
		{
			wxTextDataObject data;
			wxTheClipboard->GetData(data);
			wxString whatpaste = data.GetText();
			whatpaste.Replace(L"\n", L" ");
			whatpaste.Replace(L"\r", emptyString);
			whatpaste.Replace(L"\f", emptyString);
			whatpaste.Replace(L"\t", L" ");
			int curx = Cursor.x;
			int selx = Selend.x; if (curx > selx){ int tmp = curx; curx = selx; selx = tmp; }
			if (hasRTL || isRTL) {
				if (hasRTL)
					ConvertToRTL(&whatpaste);
				else if (isRTL)
					ConvertToRTLChars(&whatpaste);

				if (Selend.x != Cursor.x) {
					RTLText.Remove(curx, selx - curx);
				}
				RTLText.insert(curx, whatpaste);
				if(hasRTL)
					ConvertToLTR(&RTLText, &MText);
				else if (isRTL)
					ConvertToLTRChars(&RTLText, &MText);
			}
			else {
				if (Selend.x != Cursor.x) {
					MText.Remove(curx, selx - curx);
				}
				MText.insert(curx, whatpaste);
			}
			modified = true;
			CalcWraps(true, true, true);
			int whre = curx + whatpaste.length();
			SetSelection(whre, whre);

		}
		wxTheClipboard->Close();
	}
}

int TextEditor::FindError(wxPoint mpos, bool mouse)
{
	wxPoint cpos;

	if (!mouse){
		cpos = mpos;
	}
	else if (mouse && !HitTest(mpos, &cpos)){ return-1; }
	int i = 0;
	for (auto &misspell :misspells){
		if (cpos.x >= misspell.posStart && cpos.x <= misspell.posEnd){
			return i;
		}
		i++;
	}


	return -1;
}

wxPoint TextEditor::PosFromCursor(wxPoint cur)
{
	int fw, fh;
	if (wraps.size() < 2 || wraps[cur.y] == cur.x){ fw = 0; }
	else{ 
		GraphicsContext* gc = GetGraphicsContext();
		if (gc){
			gc->SetFont(font, L"#000000");
			double gcfw, gcfh;
			gc->GetTextExtent(MText.SubString(wraps[cur.y], cur.x), &gcfw, &gcfh);
			delete gc;
			fw = gcfw + 0.5f;
		}
		else{
			GetTextExtent(MText.SubString(wraps[cur.y], cur.x), &fw, &fh, nullptr, nullptr, &font);
		}
	}
	wxPoint result;
	result.x = fw + 3;
	result.y = (cur.y + 1)*fontHeight;
	return result;
}

void TextEditor::OnScroll(wxScrollEvent& event)
{
	if (scroll->IsShown()){
		int newPos = event.GetPosition();
		if (scrollPositionV != newPos) {
			scrollPositionV = newPos;
			Refresh(false);
		}
	}
}

int TextEditor::FindBracket(wxUniChar sbrkt, wxUniChar ebrkt, int pos, bool fromback)
{
	const wxString& text = (hasRTL) ? RTLText : MText;
	int i = pos;
	int brkts = 0;
	while ((fromback) ? i >= 0 : i < (int)MText.length())
	{
		if (text[i] == sbrkt){ brkts++; }
		else if (text[i] == ebrkt){ 
			if (brkts == 0){ return i; }
			brkts--; 
		}

		if (fromback){ i--; }
		else{ i++; }
	}
	return -1;
}

void TextEditor::SpellcheckerOnOff(bool on)
{
	if (useSpellchecker){
		SpellCheckerOnOff = on;
		ClearSpellcheckerTable();
	}
}

void TextEditor::ClearSpellcheckerTable()
{
	if (SpellCheckerOnOff)
		CheckText();
	else
		errors.clear();

	Refresh(false);
}

void TextEditor::MakeCursorVisible()
{
	wxSize size = GetClientSize();
	size.y -= statusBarHeight;
	wxPoint pixelPos = PosFromCursor(Cursor);
	pixelPos.y -= scrollPositionV;

	if (pixelPos.y < 3){
		scrollPositionV -= (pixelPos.y > -fontHeight) ? fontHeight : (abs(pixelPos.y) + 10);
		scrollPositionV = ((scrollPositionV / fontHeight)*fontHeight) - fontHeight;
		if (scrollPositionV<0){ scrollPositionV = 0; }
	}
	else if (pixelPos.y > size.y - 4){
		int bitmaph = (wraps.size()*fontHeight) + 4;
		int moving = pixelPos.y - (size.y - 10);
		scrollPositionV += (moving < fontHeight) ? fontHeight : moving + fontHeight;
		scrollPositionV = ((scrollPositionV / fontHeight)*fontHeight) + fontHeight;
		if (scrollPositionV > bitmaph){ scrollPositionV = bitmaph; }
	}
	Refresh(false);
	Update();
}

bool TextEditor::CheckIfKeyword(const wxString &word)
{
	for (int i = 0; i < 13; i++){
		if (word == LuaKeywords[i]){ return true; }
	}
	return false;
}

void TextEditor::SeekSelected(const wxString &word)
{
	if (word.length() < 1 || (/*word.length() < 2 && */!wxIsalnum(word[0])))
		return;

	
	wxRegEx r(L"\\m" + word + L"\\M", wxRE_ADVANCED | wxRE_ICASE); // the pattern \m and \M matches a word boundary
	if (!r.IsValid())
		return;
	wxRegEx r1(L"\\\\N" + word + L"\\M", wxRE_ADVANCED | wxRE_ICASE); // the pattern \m and \M matches a word boundary
	if (!r1.IsValid())
		return;
		
	int textPos = 0;
	wxString text = MText;

	while (r.Matches(text)) {
		size_t pos = 0, len = 0;
		if (r.GetMatch(&pos, &len)){
			pos += textPos;
			selectionWords.Add(pos);
			selectionWords.Add(pos + len - 1);
		}
		else
			break;

		
		textPos = pos + len;
		text = MText.Mid(textPos);
	}
	textPos = 0;
	text = MText;
	while (r1.Matches(text)) {
		size_t pos = 0, len = 0;
		if (r1.GetMatch(&pos, &len)){
			pos += textPos;
			//skip \N
			selectionWords.Add(pos + 2);
			selectionWords.Add(pos + len - 1);
		}
		else
			break;


		textPos = pos + len;
		text = MText.Mid(textPos);
	}
	std::sort(selectionWords.begin(), selectionWords.end());
}

void TextEditor::DrawWordRectangles(int type, wxDC &dc, int h, int posX)
{
	const wxArrayInt & words = (type == 0) ? errors.errors : selectionWords;
	const wxString& text = (hasRTL || isRTL) ? RTLText : MText;
	size_t len = words.size();
	int fw = 0, fh = 0, fww = 0, fwww = 0;
	int lineStart = (scrollPositionV - 2) / fontHeight;
	int charStart = wraps[lineStart];
	int lineEnd = ((scrollPositionV - 2 + h) / fontHeight) + 1;
	int charEnd = (lineEnd < wraps.size()) ? wraps[lineEnd] : text.length();

	for (size_t g = 0; g < len; g += 2)
	{
		int endWord = words[g + 1];
		if (endWord < charStart)
			continue;

		int startWord = words[g];
		if (startWord > charEnd)
			break;

		int fsty = FindY(startWord);
		if (wraps[fsty] >= startWord){ fw = 0; }
		else{
			wxString ftext = text.SubString(wraps[fsty], startWord - 1);
			ftext.Replace(L"\t", emptyString);
			GetTextExtent(ftext, &fw, &fh, nullptr, nullptr, &font);
		}
		int scndy = FindY(endWord);
		wxString etext = text.SubString(startWord, (fsty == scndy) ? endWord : wraps[fsty + 1] - 1);
		etext.Replace(L"\t", emptyString);
		GetTextExtent(etext, &fww, &fh, nullptr, nullptr, &font);
		for (int q = fsty + 1; q <= scndy; q++){
			int rest = (q == scndy) ? endWord : wraps[q + 1];
			wxString btext = text.SubString(wraps[q], rest);
			btext.Replace(L"\t", emptyString);
			GetTextExtent(btext, &fwww, &fh, nullptr, nullptr, &font);
			dc.DrawRectangle(3, ((q * fontHeight) + 1) - scrollPositionV, fwww, fontHeight);
		}if (hasRTL) {
			int misspellLineStart = wraps[fsty];
			int misspellLineEnd = wraps[(fsty + 1 >= wraps.size()) ? wraps.size() - 1 : fsty + 1];
			wxString rowText = text.Mid(misspellLineStart, misspellLineEnd - misspellLineStart);
			int fww1 = 0;
			GetTextExtent(rowText, &fww1, &fh);
			dc.DrawRectangle(fw + posX - fww1, ((fsty * fontHeight) + 1) - scrollPositionV, fww, fontHeight);
		}
		else {
			dc.DrawRectangle(fw + posX, ((fsty * fontHeight) + 1) - scrollPositionV, fww, fontHeight);
		}
	}
}

void TextEditor::DrawWordRectangles(int type, GraphicsContext *gc, int h, int posX)
{
	const wxArrayInt & words = (type == 0) ? errors.errors : selectionWords;
	const wxString& text = (hasRTL || isRTL) ? RTLText : MText;
	size_t len = words.size();
	double fw = 0.0, fh = 0.0, fww = 0.0, fwww = 0.0;
	int lineStart = (scrollPositionV - 2) / fontHeight;
	int charStart = wraps[lineStart];
	int lineEnd = ((scrollPositionV - 2 + h) / fontHeight) + 1;
	int charEnd = (lineEnd < wraps.size())? wraps[lineEnd] : text.length();

	for (size_t g = 0; g < len; g += 2)
	{
		int endWord = words[g + 1];
		if (endWord < charStart)
			continue;

		int startWord = words[g];
		if (startWord > charEnd)
			break;

		int fsty = FindY(startWord);
		if (wraps[fsty] >= startWord){ fw = 0.0; }
		else{
			wxString ftext = text.SubString(wraps[fsty], startWord - 1);
			ftext.Replace(L"\t", emptyString);
			gc->GetTextExtent(ftext, &fw, &fh);
		}
		int scndy = FindY(endWord);
		wxString etext = text.SubString(words[g], (fsty == scndy) ? endWord : wraps[fsty + 1] - 1);
		etext.Replace(L"\t", emptyString);
		gc->GetTextExtent(etext, &fww, &fh);
		for (int q = fsty + 1; q <= scndy; q++){
			int rest = (q == scndy) ? endWord : wraps[q + 1];
			wxString btext = text.SubString(wraps[q], rest);
			btext.Replace(L"\t", emptyString);
			gc->GetTextExtent(btext, &fwww, &fh);
			gc->DrawRectangle(3, ((q * fontHeight) + 1) - scrollPositionV, fwww, fontHeight);
		}
		if (hasRTL) {
			int misspellLineStart = wraps[fsty];
			int misspellLineEnd = wraps[(fsty + 1 >= wraps.size()) ? wraps.size() - 1 : fsty + 1];
			wxString rowText = text.Mid(misspellLineStart, misspellLineEnd - misspellLineStart);
			double fww1 = 0.f;
			gc->GetTextExtent(rowText, &fww1, &fh);
			gc->DrawRectangle(fw + posX - fww1, ((fsty * fontHeight) + 1) - scrollPositionV, fww, fontHeight);
		}
		else {
			gc->DrawRectangle(fw + posX, ((fsty * fontHeight) + 1) - scrollPositionV, fww, fontHeight);
		}
	}
}

bool TextEditor::GetNumberFromCursor(int cursorPos, wxPoint &numberPos, float &number, float &step)
{
	wxString digits = L"0123456789.-";
	int endPos = cursorPos;
	for (size_t i = endPos; i < MText.length(); i++){
		if (digits.find(MText[i]) == -1)
			break;
		else
			endPos = i;
	}
	int startPos = cursorPos;
	for (int i = startPos; i >= 0; i--){
		if (digits.find(MText[i]) == -1)
			break;
		else
			startPos = i;
	}
	if (startPos <= endPos){
		wxString strNum = MText.Mid(startPos, endPos - startPos + 1);
		if (strNum != L"." && strNum != L"-"){
			double result = 0.;
			if (!strNum.ToCDouble(&result))
				return false;

			number = result;
			numberPos = wxPoint(startPos, endPos);
			int dotfind = strNum.find(L'.');
			if (dotfind != -1 && startPos + dotfind < cursorPos)
				step = 0.1f;
			else
				step = 1.f;

			return true;
		}
	}
	return false;
}

void TextEditor::PutTag()
{
	TagListItem *item = tagList->GetItem(tagList->GetSelection());
	if (item){
		wxString& text = (hasRTL) ? RTLText : MText;
		for (int i = Cursor.x - 1; i >= 0; i--){
			if (text[i] == L'\\'){
				//It would be nice to add brackets or some else elements;
				//Looks like it have to be added for all tags separely
				wxString tag;
				item->GetTag(&tag);
				int newPosition = Cursor.x + tag.length() - (Cursor.x - 1 - i);
				if (tag.EndsWith(L')'))
					newPosition--;

				Replace(i + 1, Cursor.x, tag);
				SetSelection(newPosition, newPosition);
				delete tagList;//tagList->Destroy();
				tagList = nullptr;
				return;
			}
		}
	}
}

bool TextEditor::IsNextRTLChar(int numchar)
{
	if (numchar >= RTLText.length())
		numchar = RTLText.length() - 1;
	else if (numchar < 0)
		numchar = 0;

	for (size_t i = numchar; i < RTLText.length(); i++) {
		const wxUniChar& ch = RTLText[i];
		if (IsRTLCharacter(ch)) {
			return true;
		}
		else if (iswctype(wint_t(ch), _SPACE | _PUNCT) == 0) {
			return false;
		}
	}
	return IsPrevRTLChar(numchar);
}

bool TextEditor::IsPrevRTLChar(int numchar)
{
	if (numchar >= RTLText.length())
		numchar = RTLText.length() - 1;
	else if (numchar < 0)
		numchar = 0;

	for (size_t i = numchar; i + 1 > 0; i--) {
		const wxUniChar& ch = RTLText[i];
		if (IsRTLCharacter(ch)) {
			return true;
		}
		else if (iswctype(wint_t(ch), _SPACE | _PUNCT) == 0) {
			return false;
		}
	}
	return IsNextRTLChar(numchar);
}

//context for drawing
GraphicsContext* TextEditor::GetGraphicsContext(const wxMemoryDC& dc)
{
	if (graphicsRendering) {
		GraphicsRenderer* renderer = GraphicsRenderer::GetDirect2DRenderer();
		GraphicsContext* gc = renderer ? renderer->CreateContext(dc) : nullptr;
		return gc;
	}
	return nullptr;
}

//context for measuring
GraphicsContext* TextEditor::GetGraphicsContext()
{
	if (graphicsRendering) {
		GraphicsRenderer* renderer = GraphicsRenderer::GetDirect2DRenderer();
		GraphicsContext* gc = renderer ? renderer->CreateMeasuringContext() : nullptr;
		return gc;
	}
	return nullptr;
}

//state here is for template and for disable spellchecker and wraps
void TextEditor::SetState(int _state, bool refresh){
	if (state == _state)
		return;

	state = _state;
	//if (refresh){
		SpellCheckerOnOff = (!state) ? Options.GetBool(SPELLCHECKER_ON) : false;
		if (useSpellchecker)
			CheckText();
		EB->UpdateChars();
		Refresh(false);
	//}
};

bool TextEditor::SetFont(const wxFont &_font)
{
	int fontSize = Options.GetInt(TEXT_EDITOR_FONT_SIZE);
	font = wxFont(*Options.GetFont(fontSize - 10));
	wxWindow::SetFont(font);
	
	int fw, fh;
	GetTextExtent(L"#TWFfGH", &fw, &fh, nullptr, nullptr, &font);
	fontHeight = fh;
	caret->SetSize(1, fh);
	fontSizes.clear();
	statusBarHeight = (Options.GetBool(TEXT_EDITOR_HIDE_STATUS_BAR)) ? 0 : fontHeight + 8;
	CalcWraps();
	Refresh(false);
	return true;
}

const TextData &TextEditor::GetTextData() {
	if (!useSpellchecker) {
		errors.clear();
		errors.Init2(MText, false, EB->GetFormat(), nullptr);
	}
	return errors;
}
bool TextEditor::HasRTLText()
{
	return isRTL || hasRTL;
}
;

BEGIN_EVENT_TABLE(TextEditor, wxWindow)
EVT_PAINT(TextEditor::OnPaint)
EVT_SIZE(TextEditor::OnSize)
EVT_ERASE_BACKGROUND(TextEditor::OnEraseBackground)
EVT_MOUSE_EVENTS(TextEditor::OnMouseEvent)
EVT_CHAR(TextEditor::OnCharPress)
EVT_KEY_DOWN(TextEditor::OnKeyPress)
EVT_KILL_FOCUS(TextEditor::OnKillFocus)
EVT_SET_FOCUS(TextEditor::OnKillFocus)
EVT_COMMAND_SCROLL(3333, TextEditor::OnScroll)
EVT_MOUSE_CAPTURE_LOST(TextEditor::OnLostCapture)
END_EVENT_TABLE()




