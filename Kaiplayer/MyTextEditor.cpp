//  Copyright (c) 2016, Marcin Drob

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

#include "MyTextEditor.h"
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
#include "GraphicsD2D.h"
#include "Utils.h"

#undef DrawText

wxDEFINE_EVENT(CURSOR_MOVED, wxCommandEvent);

wxString TextEditor::LuaKeywords[] = { L"function", L"for", L"if", L"while", L"do", L"then", L"end", L"or", L"and", L"repeat", L"until", L"math", L"local" };

TextEditor::TextEditor(wxWindow *parent, int id, bool _spell, const wxPoint& pos, const wxSize& size, long style)
	:wxWindow(parent, id, pos, size, style)
{
	useSpellchecker = _spell;
	SpellCheckerOnOff = (_spell)? Options.GetBool(SPELLCHECKER_ON) : false;
	MText = L"";
	bmp = NULL;
	fsize = 10;
	posY = 0;
	scrollPositionV = 0;
	SetCursor(wxCURSOR_IBEAM);
	wxAcceleratorEntry entries[32];
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
	//font = wxFont(10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, "Verdana");
	font = *Options.GetFont();//wxFont(10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, L"Tahoma", wxFONTENCODING_DEFAULT);
	int fw, fh;
	GetTextExtent(L"#TWFfGH", &fw, &fh, NULL, NULL, &font);
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
}

TextEditor::~TextEditor()
{
	if (bmp)delete bmp;
}

void TextEditor::SetTextS(const wxString &text, bool modif, bool resetsel, bool noevent)
{
	modified = modif;
	MText = text;
	CalcWrap(modif, (noevent) ? false : modif);
	if (SpellCheckerOnOff){ CheckText(); }
	if (resetsel){ SetSelection(0, 0); }
	else{
		if ((size_t)Cursor.x > MText.length()){ Cursor.x = MText.length(); Cursor.y = FindY(Cursor.x); }
		if ((size_t)Selend.x > MText.length()){ Selend.x = MText.length(); Selend.y = FindY(Selend.x); }
		Refresh(false);
	}
	//else{Refresh(false);}
}

void TextEditor::CalcWrap(bool updatechars, bool sendevent)
{
	
	if (selectionWords.size())
		selectionWords.clear();

	wraps.clear();
	wraps.push_back(0);
	if (MText != L""){
		int w, h, fw = 0, fh = 0;
		double gcfw = 0.f, gcfh = 0.f;
		GetClientSize(&w, &h);
		if (scroll->IsShown()){
			int sw, sh;
			scroll->GetSize(&sw, &sh);
			w -= sw;
		}
		size_t i = 0;
		size_t textLen = MText.length();
		if (w < 20){
			while (i < textLen){ i++; wraps.push_back(i); }
		}
		else{
			GraphicsRenderer *renderer = GraphicsRenderer::GetDirect2DRenderer();
			GraphicsContext *gc = renderer->CreateMeasuringContext();
			if (gc){
				gc->SetFont(font, L"#000000");
				CalcWrapsD2D(gc, w, h);
				delete gc;
			}
			else{
				//CalcWrapsGDI(w, h);
				int podz = 0;
				wxString wrapchars = L" \\,;:}{()";

				int nwrap = -1;
				int allwrap = 0;
				int approxSize = w / fsize;
				while (i < textLen)
				{
					i = podz + approxSize;
					if (i >= textLen){ i = textLen - 1; }
					bool forward = false;
					bool forwardNotFound = true;
					bool backward = false;
					bool backwardNotFound = true;
					while (1)
					{
						wxString wrap = MText.SubString(podz, i);
						GetTextExtent(wrap, &fw, &fh, NULL, NULL, &font);
						
						allwrap = i;
						if (fw < w - 7 && !backward && backwardNotFound && i < textLen - 1){
							int j = i + 1;
							while (j < textLen && forwardNotFound){
								if (wrapchars.Find(MText[j]) != -1){
									nwrap = j;
									if (MText[j] == L' '){ nwrap++; }
									forward = true;
									break;
								}
								j++;
							}
							if (!forward){ i++; forwardNotFound = false; }
							else{ i = j; }
						}
						else if (fw > w - 7 && i > podz){
							size_t k = i - 1;
							while (k > podz && backwardNotFound){
								if (wrapchars.Find(MText[k]) != -1){
									nwrap = k;
									if (MText[k] == L' '){ nwrap++; }
									backward = true;
									break;
								}
								k--;
							}
							if (!backward){ if (i > podz) i--; backwardNotFound = false; }
							else{ i = k; }
						}
						else{
							int wwrap = (nwrap > 0 && nwrap > wraps[wraps.size() - 1] && nwrap <= allwrap && i < textLen - 1) ? nwrap : (i >= textLen - 1) ? textLen : allwrap + 1;
							wraps.push_back(wwrap); podz = wwrap; nwrap = -1; allwrap = i;
							i++;
							break;
						}
					}
				}
			}
		}
	}
	else{
		wraps.push_back(MText.length());
	}
		
	if (updatechars){ EB->UpdateChars(); }
	if (sendevent){ wxCommandEvent evt2(wxEVT_COMMAND_TEXT_UPDATED, GetId()); AddPendingEvent(evt2); }
}

void TextEditor::CalcWrapsD2D(GraphicsContext *gc, int w, int h)
{
	//wxString wrapchars = L" \\,;:}{()";
	double gfw, gfh;
	size_t i = 0;
	size_t textLen = MText.length();
	double widthCount = 0.0;
	while (i < textLen)
	{
		const wxUniChar &ch = MText[i];
		auto &it = fontSizes.find(ch);
		if (it != fontSizes.end()){
			widthCount += it->second;
		}
		else{
			gc->GetTextExtent(ch, &gfw, &gfh);
			fontSizes.insert(std::pair<wxUniChar, double>(ch, gfw));
			widthCount += gfw;
		}
		if (widthCount > w - 7){
			wraps.push_back(i - 1);
			widthCount = 0;
			i--;
		}
		i++;
	}
	if (wraps[wraps.size() - 1] != textLen)
		wraps.push_back(textLen);
}

//void TextEditor::CalcWrapsGDI(int w, int h)
//{
//	//wxString wrapchars = L" \\,;:}{()";
//	int gfw, gfh;
//	size_t i = 0;
//	size_t textLen = MText.length();
//	int widthCount = 0;
//	while (i < textLen)
//	{
//		const wxUniChar &ch = MText[i];
//		auto &it = fontSizes.find(ch);
//		if (it != fontSizes.end()){
//			widthCount += it->second;
//		}
//		else{
//			GetTextExtent(ch, &gfw, &gfh);
//			if (ch == L'a' || ch == L' ')
//				gfw++;
//
//			fontSizes.insert(std::pair<wxUniChar, double>(ch, gfw));
//			widthCount += gfw;
//		}
//		if (widthCount > w - 16){
//			wraps.push_back(i - 1);
//			widthCount = 0;
//			i--;
//		}
//		i++;
//	}
//	if (wraps[wraps.size() - 1] != textLen)
//		wraps.push_back(textLen);
//}

void TextEditor::OnCharPress(wxKeyEvent& event)
{
	wxUniChar wkey = event.GetUnicodeKey();
	if (wkey == L'\t'){ return; }
	if (wkey){
		if (Cursor != Selend){
			int curx = Cursor.x;
			int selx = Selend.x;
			if (curx > selx){ int tmp = curx; curx = selx; selx = tmp; }
			MText.Remove(curx, selx - curx);
			if (Cursor.x<Selend.x){ Selend = Cursor; }
			else{ Cursor = Selend; }
		}
		int len = MText.length();
		if (wkey == L'"' && changeQuotes)
			wkey = CheckQuotes();

		if (Cursor.x >= len){ MText << wkey; }
		else{ MText.insert(Cursor.x, 1, wkey); }
		CalcWrap();
		if (Cursor.x + 1 > wraps[Cursor.y + 1]){ Cursor.y++; }
		Cursor.x++;
		Selend = Cursor;
		if (SpellCheckerOnOff){ CheckText();}
		Refresh(false);
		Update();
		modified = true;
		//tag list
		if (!tagList && (wkey == L'\\' || (Cursor.x - 2 >= 0 && MText[Cursor.x - 2] == L'\\'))){
			//No need to check end cause when there's no end start will take all line
			//No need to show list when it's plain text, someone want to write \h or \N
			for (int i = Cursor.x - 1; i >= 0; i--){
				if (MText[i] == L'}')
					break;
				if (MText[i] == L'{'){
					tagList = new PopupTagList(this);
					if (wkey != L'\\')
						tagList->AppendToKeyword(wkey);

					if (!tagList->GetCount()){
						delete tagList;
						tagList = NULL;
						return;
					}

					//calculate position of popup list
					wxPoint pos;
					pos.y = (Cursor.y * fontHeight) + fontHeight + 5;
					// it should be changed to constant int for avoid bugs
					pos.x = 3;
					int wrap = wraps[Cursor.y];
					if (wrap < Cursor.x){
						GraphicsRenderer *renderer = GraphicsRenderer::GetDirect2DRenderer();
						GraphicsContext *gc = renderer->CreateMeasuringContext();
						wxString textBeforeCursor = MText.Mid(wrap, Cursor.x - wrap + 1);
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
					Bind(wxEVT_COMMAND_CHOICE_SELECTED, [=](wxCommandEvent &evt){
						PutTag();
					}, tagList->GetId());
					break;
				}
			}
			
		}
		else if (tagList){
			tagList->AppendToKeyword(wkey);
			if (!tagList->GetCount()){
				delete tagList;
				tagList = NULL;
				return;
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
		GetTextExtent(L"#TWFfGH", &fw, &fh, NULL, NULL, &font);
		fontHeight = fh;
		caret->SetSize(1, fh);
		CalcWrap(false, false);
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
			tagList->Destroy();
			tagList = NULL;
		}
	}
	if (((!ctrl && !alt) || (ctrl && alt)) && (key > 30 || key == 0) || (alt && (key >= WXK_NUMPAD0 && key <= WXK_NUMPAD9))){
		event.Skip(); return; 
	}
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
		tagList->Destroy();
		tagList = NULL;
	}
	switch (ID){
	case ID_CDELETE:
	case ID_CBACK:
		//len = MText.length();
		if ((ID == ID_CBACK && Cursor.x == 0) || (ID == ID_CDELETE && Cursor.x >= (int)MText.length())){ return; }
		Selend.x = Cursor.x;
		if (ID == ID_CBACK){
			FindWord((Cursor.x < 2) ? 0 : Cursor.x - 1, &Cursor.x, &len);
			if (Cursor.x == 1 && MText[0] == L' '){ Cursor.x--; }
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
			MText.Remove(curx, selx - curx);
			Selend = Cursor;
			CalcWrap();
			if (SpellCheckerOnOff){ CheckText(); }
			SetSelection(curx, curx);
		}
		else
		{
			if (ID == ID_BACK){
				if (Cursor.x < 1){ return; }
				Cursor.x--;
			}
			if (ID == ID_DEL && Cursor.x >= (int)MText.length()){ return; }
			MText.Remove(Cursor.x, 1);
		}
		len = wraps.size();
		CalcWrap();

		if (Cursor.x<wraps[Cursor.y] || (Cursor.x == wraps[Cursor.y] && len != wraps.size())){ Cursor.y--; }
		else if (Cursor.x>wraps[Cursor.y + 1]){ Cursor.y++; }
		Selend = Cursor;
		if (SpellCheckerOnOff){ CheckText(); }
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
		else if (ID != ID_CLEFT&&ID != ID_CSLEFT){ Cursor.x--; }


		if (ID<ID_SLEFT){ Selend = Cursor; }
		Refresh(false);
		break;

	case ID_RIGHT:
	case ID_CRIGHT:
	case ID_SRIGHT:
	case ID_CSRIGHT:
		if (ID == ID_RIGHT && Selend.x>Cursor.x){ Cursor = Selend; Refresh(false); return; }
		if (Cursor.x >= (int)MText.length()){ return; }
		if (ID == ID_CRIGHT || ID == ID_CSRIGHT){
			if (Cursor.x == MText.length() - 1){
				Cursor.x++;
			}
			else{
				FindWord(Cursor.x + 1, 0, &Cursor.x);
			}
		}
		if (Cursor.x + 1 > wraps[Cursor.y + 1] && Cursor.y < (int)wraps.size() - 2){ Cursor.y++; }
		else if (ID != ID_CRIGHT&&ID != ID_CSRIGHT){ Cursor.x++; }

		if (ID<ID_SRIGHT){ Selend = Cursor; }
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
		len = MText.length();
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
		Selend.x = MText.length(); Selend.y = wraps.size() - 2;
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
	if (ID != ID_CTLC || ID != ID_WMENU){ wxCommandEvent evt(CURSOR_MOVED, GetId()); AddPendingEvent(evt); }
}

void TextEditor::OnMouseEvent(wxMouseEvent& event)
{
	bool click = event.LeftDown();
	bool leftup = event.LeftUp();
	if (event.ButtonDown()){ 
		if (tagList){
			tagList->Destroy();
			tagList = NULL;
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



	if (event.LeftDClick() && MText != L"" && isInField){
		wasDoubleClick = true;
		dclickCurPos = mousePosition;
		time = timeGetTime();
		int errn = FindError(mousePosition);
		if (Options.GetBool(EDITBOX_SUGGESTIONS_ON_DOUBLE_CLICK) && errn >= 0){
			wxString err = misspels[errn];

			wxArrayString suggs;
			SpellChecker::Get()->Suggestions(err, suggs);

			KaiListBox lw(this, suggs, _("Sugestie poprawy"));
			if (lw.ShowModal() == wxID_OK)
			{
				int from = errors[errn * 2];
				int to = errors[(errn * 2) + 1];
				wxString txt = lw.GetSelection();
				MText.replace(from, to - from + 1, txt);
				modified = true;
				CalcWrap();
				if (SpellCheckerOnOff){ CheckText(); }
				int newto = from + txt.length();
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
		if (cur != Cursor){ wxCommandEvent evt(CURSOR_MOVED, GetId()); AddPendingEvent(evt); }
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
			if (end == oldstart){ Selend.x = oldend; Selend.y = FindY(oldend); }
			Cursor.x = start;
			Cursor.y = FindY(start);
		}
		else{
			if (oldstart == start){ Selend.x = oldstart; Selend.y = FindY(oldstart); }
			Cursor.x = end;
			Cursor.y = FindY(end);
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
		else if (!mouseWheel && (dclickCurPos.y < mousePosition.y + 5 && dclickCurPos.y > mousePosition.y - 5)){
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
					if(dclickCurPos.y < mousePosition.y)
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
			fsize += event.GetWheelRotation() / event.GetWheelDelta();
			if (fsize < 7 || fsize > 70){ fsize = MID(7, fsize, 70); return; }
			font.SetPointSize(fsize);
			int fw, fh;
			GetTextExtent(L"#TWFfGH", &fw, &fh, NULL, NULL, &font);
			fontHeight = fh;
			caret->SetSize(1, fh);
			fontSizes.clear();
			CalcWrap(false, false);
			Refresh(false);
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

	CalcWrap(false, false);
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
	int bitmaph = (wraps.size() * fontHeight) + 4;
	int windoww = w;
	if (bitmaph > h){
		if (!scroll->IsShown()){
			scroll->Show();
			CalcWrap(false, false);
			Cursor.y = FindY(Cursor.x);
			Selend.y = FindY(Selend.x);
			bitmaph = (wraps.size() * fontHeight) + 4;
		}
		int sw = 0, sh = 0;
		scroll->GetSize(&sw, &sh);
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
			CalcWrap(false, false);
			Cursor.y = FindY(Cursor.x);
			Selend.y = FindY(Selend.x);
		}
		bitmaph = h;
		scrollPositionV = 0;
	}

	if (bmp) {
		if (bmp->GetWidth() < w || bmp->GetHeight() < h) {
			delete bmp;
			bmp = NULL;
		}
	}

	if (!bmp) bmp = new wxBitmap(w, h);
	// Draw bitmap
	wxMemoryDC bmpDC;
	bmpDC.SelectObject(*bmp);

	GraphicsRenderer *renderer = GraphicsRenderer::GetDirect2DRenderer();
	GraphicsContext *gc = renderer->CreateContext(bmpDC);

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

	const wxColour & ctvariables = Options.GetColour(EditorTemplateVariables);
	const wxColour & ctext = templateCode ? ctvariables : Options.GetColour(EditorText);
	const wxColour & ccurlybraces = Options.GetColour(EditorCurlyBraces);
	const wxColour & coperators = Options.GetColour(EditorTagOperators);
	const wxColour & cnames = Options.GetColour(EditorTagNames);
	const wxColour & cvalues = Options.GetColour(EditorTagValues);
	const wxColour & bgbraces = Options.GetColour(EditorBracesBackground);
	const wxColour & cbackground = Options.GetColour(EditorBackground);
	const wxColour & cselection = Options.GetColour(EditorSelection);
	const wxColour & cselnofocus = Options.GetColour(EditorSelectionNoFocus);
	const wxColour & cspellerrors = Options.GetColour(EditorSpellchecker);
	const wxColour & ctcodemarks = Options.GetColour(EditorTemplateCodeMarks);
	const wxColour & ctfunctions = Options.GetColour(EditorTemplateFunctions);
	const wxColour & ctkeywords = Options.GetColour(EditorTemplateKeywords);
	const wxColour & ctstrings = Options.GetColour(EditorTemplateStrings);
	const wxColour & cphrasesearch = Options.GetColour(EditorPhraseSearch);

	gc->SetBrush(cbackground);
	gc->SetPen(*wxTRANSPARENT_PEN);
	gc->DrawRectangle(0, 0, w, h);

	wxString digits = L"(0123456789-&+";
	wxString tagtest;
	wxString parttext;
	wxString mestext;

	posY = 2;
	posY -= scrollPositionV;
	bool isfirst = true;
	int wline = 1;
	int wchar = 0;
	bool hasFocus = HasFocus();

	wxString alltext = MText + L" ";
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
		DrawWordRectangles(0, gc, h);
	}
	if (selectionWords.size()){
		gc->SetBrush(cphrasesearch);
		DrawWordRectangles(1, gc, h);
	}
	else if (Cursor.x != Selend.x || Cursor.y != Selend.y){
		Brackets.x = -1; Brackets.y = -1;
		wxPoint fst, scd;
		if ((Cursor.x + Cursor.y) > (Selend.x + Selend.y)){ fst = Selend; scd = Cursor; }
		else{ fst = Cursor, scd = Selend; }

		gc->SetBrush(wxBrush(wxColour(hasFocus ? cselection : cselnofocus)));
		fww = 0.0;
		//skip unnided selection drawing
		int lineStart = (scrollPositionV - 2) / fontHeight;
		int charStart = wraps[lineStart];
		int lineEnd = ((scrollPositionV - 2 + h) / fontHeight) + 1;
		int charEnd = (lineEnd < wraps.size()) ? wraps[lineEnd] : MText.length();
		//drawing selection
		for (int j = fst.y; j <= scd.y; j++){
			int endWrap = wraps[j + 1];
			if (endWrap < charStart)
				continue;

			int startWrap = wraps[j];
			if (startWrap > charEnd)
				break;

			if (j == fst.y){
				wxString ftext = MText.SubString(startWrap, fst.x - 1);
				ftext.Replace(L"\t", L"");
				if (startWrap > fst.x - 1){ fw = 0.0; }
				else{ gc->GetTextExtent(ftext, &fw, &fh); }
				wxString stext = MText.SubString(fst.x, (fst.y == scd.y) ? scd.x - 1 : endWrap - 1);
				stext.Replace(L"\t", L"");
				gc->GetTextExtent(stext, &fww, &fh);
			}
			else{
				fw = 0.0;
				wxString stext = MText.SubString(startWrap, (j == scd.y) ? scd.x - 1 : endWrap - 1);
				stext.Replace(L"\t", L"");
				gc->GetTextExtent(stext, &fww, &fh);
			}
			gc->DrawRectangle(fw + 3, ((j * fontHeight) + 1) - scrollPositionV, fww, fontHeight);
		}
	}
	//skip unneeded lines, start one line before to avoid not colored values on start
	int lineStart = ((scrollPositionV - 2) / fontHeight) - 1;
	int charStart = 0;
	if (lineStart > 0){
		charStart = wraps[lineStart];
		
		wxString seekingPart = MText.Mid(0, charStart);
		size_t endBracket = seekingPart.Find(L'}', true);
		size_t startBracket = seekingPart.Find(L'{', true);
		if (endBracket == -1 || startBracket > endBracket)
			tags = true;

		wline = lineStart + 1;
		posY += (lineStart * fontHeight);
		wchar = charStart + lineStart;
	}

	bool cursorWasSet = false;
	//Drawing text
	for (int i = charStart; i < len; i++){
		if (posY > h)
			break;

		const wxUniChar &ch = alltext[i];


		if (i == wraps[wline]){
			if (Cursor.x + Cursor.y == wchar){
				double fww = 0.f;
				wxString text = mestext + parttext;
				if (!text.empty())
					gc->GetTextExtent(text, &fww, &fh);
				caret->Move(fww + 3, posY);
				cursorWasSet = true;
			}

			if (parttext != L""){
				gc->GetTextExtent(mestext, &fw, &fh);
				wxColour fontColor = (val || (isTemplateLine && parttext.IsNumber())) ? cvalues : (slash) ? cnames :
					(templateString) ? ctstrings : (isTemplateLine && ch == L'(') ? ctfunctions :
					(isTemplateLine && CheckIfKeyword(parttext)) ? ctkeywords : templateCode ? ctvariables : ctext;
				gc->SetFont(font, fontColor);
				mestext << parttext;
				gc->DrawTextU(parttext, fw + 3, posY);
			}

			posY += fontHeight;
			wline++;
			wchar++;
			parttext.clear();
			mestext.clear();
		}

		if (hasFocus && (Cursor.x + Cursor.y == wchar)){
			if (mestext + parttext == L""){ fw = 0.0; }
			else{ gc->GetTextExtent(mestext + parttext, &fw, &fh); }
			caret->Move(fw + 3, posY);
			cursorWasSet = true;
		}
		if (hasFocus && (i == Brackets.x || i == Brackets.y)){
			int bry = FindY(i);
			wxColour col = bgbraces;
			if (Brackets.x == -1 || Brackets.y == -1){ col = cspellerrors; }
			gc->SetBrush(wxBrush(col));
			wxString text = MText.SubString(wraps[bry], i - 1);
			text.Replace(L"\t", L"");
			if (i > 0){ gc->GetTextExtent(text, &fw, &fh); }
			else{ fw = 0; }
			gc->GetTextExtent(MText[i], &fww, &fh);
			gc->DrawRectangle(fw + 3, ((bry*fontHeight) + 2) - scrollPositionV, fww, fontHeight);
			wxFont fnt = font;
			fnt = fnt.Bold();
			gc->SetFont(fnt, (ch == L'{' || ch == L'}') ? ccurlybraces : coperators);
			gc->DrawTextU(MText[i], fw + 3, ((bry*fontHeight) + 2) - scrollPositionV);
			gc->SetFont(font, ctext);

		}
		if (isTemplateLine){
			if (!templateString && (ch == L'!' || ch == L'.' || ch == L',' || ch == L'+' || ch == L'-' || ch == L'=' || ch == L'(' ||
				ch == L')' || ch == L'>' || ch == L'<' || ch == L'[' || ch == L']' || ch == L'*' || ch == L'/' || ch == L':' || ch == L';')){
				gc->GetTextExtent(mestext, &fw, &fh);
				gc->SetFont(font, (parttext.IsNumber() || val) ? cvalues : (slash) ? cnames :
					(ch == L'(' && !slash) ? ctfunctions : (CheckIfKeyword(parttext)) ? ctkeywords : ctvariables);
				gc->DrawTextU(parttext, fw + 3, posY);
				mestext << parttext;
				parttext.clear();
				gc->GetTextExtent(mestext, &fw, &fh);
				gc->SetFont(font, (ch == L'!') ? ctcodemarks : coperators);
				gc->DrawTextU(ch, fw + 3, posY);
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
					gc->GetTextExtent(mestext, &fw, &fh);
					gc->SetFont(font, ctstrings);
					gc->DrawTextU(parttext, fw + 3, posY);
					mestext << parttext;
					parttext.clear();
					templateString = !templateString;
					wchar++;
					continue;
				}
				templateString = !templateString;
			}
			if (!templateString && ch == L' '){
				gc->GetTextExtent(mestext, &fw, &fh);
				gc->SetFont(font, (!templateCode && !val && !slash) ? ctext : (parttext.IsNumber() || val) ? cvalues :
					(slash) ? cnames : (CheckIfKeyword(parttext)) ? ctkeywords : ctvariables);
				gc->DrawTextU(parttext, fw + 3, posY);
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
		if (ch == L'{' || ch == L'}'){
			if (ch == L'{'){
				tags = true;
				wxString bef = parttext.BeforeLast(L'{');
				gc->GetTextExtent(mestext, &fw, &fh);
				gc->SetFont(font, ctext);
				gc->DrawTextU(bef, fw + 3, posY);
				mestext << bef;
				parttext = L"{";
			}
			else{
				wxString &tmp = parttext.RemoveLast(1);
				gc->GetTextExtent(mestext, &fw, &fh);
				gc->SetFont(font, (val) ? cvalues : (slash) ? cnames : ctext);
				gc->DrawTextU(tmp, fw + 3, posY);
				mestext << tmp;
				parttext = L"}";
				tags = slash = val = false;
			}
			gc->GetTextExtent(mestext, &fw, &fh);
			gc->SetFont(font, ccurlybraces);
			gc->DrawTextU(parttext, fw + 3, posY);
			mestext << parttext;
			parttext.clear();
			val = false;
		}

		if (slash){
			tagtest += ch;
			if ((digits.Find(ch) != -1 && tagtest != L"1" && tagtest != L"2" && tagtest != L"3" && tagtest != L"4") || tagtest == L"fn" || ch == L'('){
				slash = false;
				wxString tmp = (tagtest == L"fn") ? parttext : parttext.RemoveLast(1);
				gc->GetTextExtent(mestext, &fw, &fh);
				gc->SetFont(font, cnames);
				gc->DrawTextU(tmp, fw + 3, posY);
				mestext << tmp;
				if (tagtest == L"fn"){ parttext.clear(); }
				else{ parttext = ch; }
				val = true;
				tagtest.clear();
			}
		}

		if ((ch == L'\\' || ch == L'(' || ch == L')' || ch == L',') && tags){
			wxString tmp = parttext.RemoveLast(1);
			gc->GetTextExtent(mestext, &fw, &fh);
			gc->SetFont(font, (val && (ch == L'\\' || ch == L')' || ch == L',')) ? cvalues : slash ? cnames : ctext);
			gc->DrawTextU(tmp, fw + 3, posY);
			mestext << tmp;
			parttext = ch;
			if (ch == L'\\'){ slash = true; }
			gc->GetTextExtent(mestext, &fw, &fh);
			gc->SetFont(font, coperators);
			gc->DrawTextU(parttext, fw + 3, posY);
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
	const wxColour &border = Options.GetColour(hasFocus ? EditorBorderOnFocus : EditorBorder);
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
		gc->DrawTextU(wxString::Format(L"Length: %i", (int)MText.length()), 5, ypos);
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

	const wxColour & ctvariables = Options.GetColour(EditorTemplateVariables);
	const wxColour & ctext = templateCode ? ctvariables : Options.GetColour(EditorText);
	const wxColour & ccurlybraces = Options.GetColour(EditorCurlyBraces);
	const wxColour & coperators = Options.GetColour(EditorTagOperators);
	const wxColour & cnames = Options.GetColour(EditorTagNames);
	const wxColour & cvalues = Options.GetColour(EditorTagValues);
	const wxColour & bgbraces = Options.GetColour(EditorBracesBackground);
	const wxColour & cbackground = Options.GetColour(EditorBackground);
	const wxColour & cselection = Options.GetColour(EditorSelection);
	const wxColour & cselnofocus = Options.GetColour(EditorSelectionNoFocus);
	const wxColour & cspellerrors = Options.GetColour(EditorSpellchecker);
	const wxColour & ctcodemarks = Options.GetColour(EditorTemplateCodeMarks);
	const wxColour & ctfunctions = Options.GetColour(EditorTemplateFunctions);
	const wxColour & ctkeywords = Options.GetColour(EditorTemplateKeywords);
	const wxColour & ctstrings = Options.GetColour(EditorTemplateStrings);
	const wxColour & cphrasesearch = Options.GetColour(EditorPhraseSearch);

	dc.SetBrush(cbackground);
	dc.SetPen(*wxTRANSPARENT_PEN);
	dc.DrawRectangle(0, 0, w, h);

	wxString digits = L"(0123456789-&+";
	wxString tagtest;
	wxString parttext;
	wxString mestext;

	posY = 2;
	posY -= scrollPositionV;
	bool isfirst = true;
	int wline = 1;
	int wchar = 0;
	bool hasFocus = HasFocus();

	dc.SetFont(font);
	wxString alltext = MText + L" ";
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
		DrawWordRectangles(0, dc, h);
	}
	if (selectionWords.size()){
		dc.SetBrush(cphrasesearch);
		DrawWordRectangles(1, dc, h);
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
		int charEnd = (lineEnd < wraps.size()) ? wraps[lineEnd] : MText.length();
		//drawing selection
		for (int j = fst.y; j <= scd.y; j++){
			int endWrap = wraps[j + 1];
			if (endWrap < charStart)
				continue;

			int startWrap = wraps[j];
			if (startWrap > charEnd)
				break;

			if (j == fst.y){
				wxString ftext = MText.SubString(startWrap, fst.x - 1);
				ftext.Replace(L"\t", L"");
				if (startWrap > fst.x - 1){ fw = 0; }
				else{ GetTextExtent(ftext, &fw, &fh, NULL, NULL, &font); }
				wxString stext = MText.SubString(fst.x, (fst.y == scd.y) ? scd.x - 1 : endWrap - 1);
				stext.Replace(L"\t", L"");
				GetTextExtent(stext, &fww, &fh, NULL, NULL, &font);
			}
			else{
				fw = 0;
				wxString stext = MText.SubString(startWrap, (j == scd.y) ? scd.x - 1 : endWrap - 1);
				stext.Replace(L"\t", L"");
				GetTextExtent(stext, &fww, &fh, NULL, NULL, &font);
			}
			dc.DrawRectangle(fw + 3, ((j * fontHeight) + 1) - scrollPositionV, fww, fontHeight);
		}
	}
	//skip unneeded lines, start one line before to avoid not colored values on start
	int lineStart = ((scrollPositionV - 2) / fontHeight) - 1;
	int charStart = 0;
	if (lineStart > 0){
		charStart = wraps[lineStart];

		wxString seekingPart = MText.Mid(0, charStart);
		size_t endBracket = seekingPart.Find(L'}', true);
		size_t startBracket = seekingPart.Find(L'{', true);
		if (endBracket == -1 || startBracket > endBracket)
			tags = true;

		wline = lineStart + 1;
		posY += (lineStart * fontHeight);
		wchar = charStart + lineStart;
	}
	bool cursorWasSet = false;
	//Drawing text
	for (int i = charStart; i < len; i++){
		if (posY > h)
			break;

		const wxUniChar &ch = alltext[i];


		if (i == wraps[wline]){
			if (Cursor.x + Cursor.y == wchar){
				int fww = 0;
				GetTextExtent(mestext + parttext, &fww, &fh, NULL, NULL, &font);
				caret->Move(fww + 3, posY);
				cursorWasSet = true;
			}

			if (parttext != L""){
				GetTextExtent(mestext, &fw, &fh, NULL, NULL, &font);
				wxColour kol = (val || (isTemplateLine && parttext.IsNumber())) ? cvalues : (slash) ? cnames :
					(templateString) ? ctstrings : (isTemplateLine && ch == L'(') ? ctfunctions :
					(isTemplateLine && CheckIfKeyword(parttext)) ? ctkeywords : templateCode ? ctvariables : ctext;
				dc.SetTextForeground(kol);
				mestext << parttext;
				dc.DrawText(parttext, fw + 3, posY);
			}

			posY += fontHeight;
			wline++;
			wchar++;
			parttext.clear();
			mestext.clear();
		}

		if (hasFocus && (Cursor.x + Cursor.y == wchar)){
			if (mestext + parttext == L""){ fw = 0; }
			else{ GetTextExtent(mestext + parttext, &fw, &fh, NULL, NULL, &font); }
			caret->Move(fw + 3, posY);
			cursorWasSet = true;
		}
		if (hasFocus && (i == Brackets.x || i == Brackets.y)){
			int bry = FindY(i);
			wxColour col = bgbraces;
			if (Brackets.x == -1 || Brackets.y == -1){ col = cspellerrors; }
			dc.SetBrush(wxBrush(col));
			//dc.SetPen(wxPen(col));
			wxString text = MText.SubString(wraps[bry], i - 1);
			text.Replace(L"\t", L"");
			if (i > 0){ GetTextExtent(text, &fw, &fh, NULL, NULL, &font); }
			else{ fw = 0; }
			GetTextExtent(MText[i], &fww, &fh, NULL, NULL, &font);
			dc.DrawRectangle(fw + 3, ((bry*fontHeight) + 2) - scrollPositionV, fww, fontHeight);
			wxFont fnt = dc.GetFont();
			fnt = fnt.Bold();
			dc.SetFont(fnt);
			dc.SetTextForeground((ch == L'{' || ch == L'}') ? ccurlybraces : coperators);
			dc.DrawText(MText[i], fw + 3, ((bry*fontHeight) + 2) - scrollPositionV);
			dc.SetFont(font);

		}
		if (isTemplateLine){
			if (!templateString && (ch == L'!' || ch == L'.' || ch == L',' || ch == L'+' || ch == L'-' || ch == L'=' || ch == L'(' ||
				ch == L')' || ch == L'>' || ch == L'<' || ch == L'[' || ch == L']' || ch == L'*' || ch == L'/' || ch == L':' || ch == L';')){
				GetTextExtent(mestext, &fw, &fh, NULL, NULL, &font);
				dc.SetTextForeground((parttext.IsNumber() || val) ? cvalues : (slash) ? cnames :
					(ch == L'(' && !slash) ? ctfunctions : (CheckIfKeyword(parttext)) ? ctkeywords : ctvariables);
				dc.DrawText(parttext, fw + 3, posY);
				mestext << parttext;
				parttext.clear();
				GetTextExtent(mestext, &fw, &fh, NULL, NULL, &font);
				dc.SetTextForeground((ch == L'!') ? ctcodemarks : coperators);
				dc.DrawText(ch, fw + 3, posY);
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
					GetTextExtent(mestext, &fw, &fh, NULL, NULL, &font);
					dc.SetTextForeground(ctstrings);
					dc.DrawText(parttext, fw + 3, posY);
					mestext << parttext;
					parttext.clear();
					templateString = !templateString;
					wchar++;
					continue;
				}
				templateString = !templateString;
			}
			if (!templateString && ch == L' '){
				GetTextExtent(mestext, &fw, &fh, NULL, NULL, &font);
				dc.SetTextForeground((!templateCode && !val && !slash) ? ctext : (parttext.IsNumber() || val) ? cvalues :
					(slash) ? cnames : (CheckIfKeyword(parttext)) ? ctkeywords : ctvariables);
				dc.DrawText(parttext, fw + 3, posY);
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
		if (ch == L'{' || ch == L'}'){
			if (ch == L'{'){
				tags = true;
				wxString bef = parttext.BeforeLast(L'{');
				GetTextExtent(mestext, &fw, &fh, NULL, NULL, &font);
				dc.SetTextForeground(ctext);
				dc.DrawText(bef, fw + 3, posY);
				mestext << bef;
				parttext = L"{";
			}
			else{
				wxString &tmp = parttext.RemoveLast(1);
				GetTextExtent(mestext, &fw, &fh, NULL, NULL, &font);
				dc.SetTextForeground((val) ? cvalues : (slash) ? cnames : ctext);
				dc.DrawText(tmp, fw + 3, posY);
				mestext << tmp;
				parttext = L"}";
				tags = slash = val = false;
			}
			GetTextExtent(mestext, &fw, &fh, NULL, NULL, &font);
			dc.SetTextForeground(ccurlybraces);
			dc.DrawText(parttext, fw + 3, posY);
			mestext << parttext;
			parttext.clear();
			val = false;
		}

		if (slash){
			tagtest += ch;
			if ((digits.Find(ch) != -1 && tagtest != L"1" && tagtest != L"2" && tagtest != L"3" && tagtest != L"4") || 
				tagtest == L"fn" || ch == L'('){
				slash = false;
				wxString tmp = (tagtest == L"fn") ? parttext : parttext.RemoveLast(1);
				GetTextExtent(mestext, &fw, &fh, NULL, NULL, &font);
				dc.SetTextForeground(cnames);
				dc.DrawText(tmp, fw + 3, posY);
				mestext << tmp;
				if (tagtest == L"fn"){ parttext.clear(); }
				else{ parttext = ch; }
				val = true;
				tagtest.clear();
			}
		}

		if ((ch == L'\\' || ch == L'(' || ch == L')' || ch == L',') && tags){
			wxString tmp = parttext.RemoveLast(1);
			GetTextExtent(mestext, &fw, &fh, NULL, NULL, &font);
			dc.SetTextForeground((val && (ch == L'\\' || ch == L')' || ch == L',')) ? cvalues : slash ? cnames : ctext);
			dc.DrawText(tmp, fw + 3, posY);
			mestext << tmp;
			parttext = ch;
			if (ch == L'\\'){ slash = true; }
			GetTextExtent(mestext, &fw, &fh, NULL, NULL, &font);
			dc.SetTextForeground(coperators);
			dc.DrawText(parttext, fw + 3, posY);
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
	const wxColour &border = Options.GetColour(hasFocus ? EditorBorderOnFocus : EditorBorder);
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
		dc.DrawText(wxString::Format(L"Length: %i", (int)MText.length()), 5, ypos);
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
	wxString txt = MText + L" ";

	int wlen = MText.length();
	int fww = 0;
	double gcfw = 0.f, gcfh = 0.f, gcfw1 = 0.f;
	GraphicsRenderer *renderer = GraphicsRenderer::GetDirect2DRenderer();
	GraphicsContext *gc = renderer->CreateMeasuringContext();
	if (gc)
		gc->SetFont(font, L"#000000");

	for (int i = cur->x; i<wraps[cur->y + 1] + 1; i++)
	{
		wxString text = txt.SubString(cur->x, i);
		text.Replace(L"\t", L"");
		if (gc){
			gc->GetTextExtent(text, &gcfw, &gcfh);
			gc->GetTextExtent(txt[i], &gcfw1, &gcfh);
			if (gcfw + 1.f - (gcfw1 / 2.f) > pos.x){ cur->x = i; find = true; break; }
		}
		else{
			GetTextExtent(text, &fw, &fh, NULL, NULL, &font);
			GetTextExtent(txt[i], &fww, &fh, NULL, NULL, &font);
			if (fw + 1 - (fww / 2)>pos.x){ cur->x = i; find = true; break; }
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
	if ((Cursor.x != end || Selend.x != start) && !noEvent){ wxCommandEvent evt(CURSOR_MOVED, GetId()); AddPendingEvent(evt); }
	Cursor.x = end;
	Selend.x = start;
	Selend.y = FindY(Selend.x);
	Cursor.y = FindY(Cursor.x);

	Refresh(false);
}

wxString TextEditor::GetValue() const
{
	return MText;
}

void TextEditor::Replace(int start, int end, const wxString &rep)
{
	modified = true;
	MText.replace(start, end - start, rep);
	CalcWrap();
	if (SpellCheckerOnOff){ CheckText(); }
	//Cursor.x = 0; Cursor.y = 0;
	//Selend = Cursor;
	if ((size_t)Cursor.x > MText.length()){ Cursor.x = MText.length(); Cursor.y = FindY(Cursor.x); }
	if ((size_t)Selend.x > MText.length()){ Selend.x = MText.length(); Selend.y = FindY(Selend.x); }
	Refresh(false);
}

void TextEditor::CheckText()
{
	if (MText == L""){ return; }
	wxString text = MText;
	errors.clear();
	misspels.Clear();
	text += L" ";
	bool block = false;
	wxString word = L"";
	int lasti = 0;
	int firsti = 0;
	int lastStartBracket = -1;
	int lastEndBracket = -1;
	int lastStartCBracket = -1;
	int lastStartTBracket = -1;
	int lastEndCBracket = -1;
	for (size_t i = 0; i < text.length(); i++)
	{
		wxUniChar ch = text.GetChar(i);
		if (iswctype(wint_t(ch), _SPACE | _DIGIT | _PUNCT) && ch != L'\'' && !block){
			if (word.length() > 1){
				if (word.StartsWith(L"'")){ word = word.Remove(0, 1); }
				if (word.EndsWith(L"'")){ word = word.RemoveLast(1); }
				word.Trim(false); word.Trim(true);
				bool isgood = SpellChecker::Get()->CheckWord(word);
				if (!isgood){ misspels.Add(word); errors.push_back(firsti); errors.push_back(lasti); }
			}
			word = L""; firsti = i + 1;
		}
		if (block){
			if (ch == L'{'){ errors.push_back(lastStartCBracket); errors.push_back(lastStartCBracket); misspels.Add(L""); }
			if (ch == L'\\' && text[(i == 0) ? 0 : i - 1] == L'\\'){ errors.push_back(i); errors.push_back(i); misspels.Add(L""); }
			if (ch == L'('){
				if (i > 1 && text[i - 2] == L'\\' && text[i - 1]){ lastStartTBracket = i; continue; }
				if (lastStartBracket > lastEndBracket){
					errors.push_back(lastStartBracket); errors.push_back(lastStartBracket); misspels.Add(L"");
				}
				lastStartBracket = i;
			}
			if (ch == L')'){
				if (lastStartBracket < lastEndBracket || lastStartBracket < 0){
					if (lastStartTBracket > 0 && (lastStartTBracket < lastEndBracket || lastStartBracket < lastStartTBracket)){
						lastStartTBracket = -1; continue;
					}
					errors.push_back(i); errors.push_back(i); misspels.Add(L"");
				}
				lastEndBracket = i;
			}
		}
		if (!block && ch == L'}'){
			errors.push_back(i); errors.push_back(i); misspels.Add(L"");
		}
		if (lastStartTBracket >= 0 && ch == L'{' || ch == L'}'){
			errors.push_back(lastStartTBracket); errors.push_back(lastStartTBracket); misspels.Add(L"");
			lastStartTBracket = -1;
		}
		if (ch == L'{'){ block = true; lastStartCBracket = i; continue; }
		else if (ch == L'}'){ block = false; lastEndCBracket = i; firsti = i + 1; word = L""; continue; }

		if (!block && /*notchar.Find(ch)==-1*/ (!iswctype(wint_t(ch), _SPACE | _DIGIT | _PUNCT) || ch == L'\'') &&
			text.GetChar((i == 0) ? 0 : i - 1) != L'\\'){
			word << ch; lasti = i;
		}
		else if (!block && text.GetChar((i == 0) ? 0 : i - 1) == L'\\'){
			word = L"";
			if (ch == L'N' || ch == L'n' || ch == L'h'){
				firsti = i + 1;
			}
			else{
				firsti = i;
				word << ch;
			}
		}
	}

	if (lastStartCBracket > lastEndCBracket){ 
		errors.push_back(lastStartCBracket); 
		errors.push_back(lastStartCBracket); 
		misspels.Add(L""); 
	}
	if (lastStartBracket > lastEndBracket){ 
		errors.push_back(lastStartBracket); 
		errors.push_back(lastStartBracket); 
		misspels.Add(L""); 
	}
	if (lastStartTBracket >= 0){ 
		errors.push_back(lastStartTBracket); 
		errors.push_back(lastStartTBracket); 
		misspels.Add(L""); 
	}
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
		tagList->Destroy();
		tagList = NULL;
	}
	Refresh(false);
}

void TextEditor::FindWord(int pos, int *start, int *end)
{
	//wxString wfind = " ]})({[-—'`\"\\;:,.><?!*~@#$%^&/+=";
	int len = MText.length();
	if (len < 1){ Cursor.x = Cursor.y = 0; *start = 0; *end = 0; return; }
	bool fromend = (start != NULL);

	if (!fromend){ pos--; }
	pos = MID(0, pos, len - 1);
	bool hasres = false;
	int lastres = -1;
	if (fromend){
		*start = (fromend) ? 0 : len;
		for (int i = pos; i >= 0; i--){
			const wxUniChar &ch = MText[i];
			int res = iswctype(wint_t(ch), _SPACE | _PUNCT);
			if (lastres == 8 && res != 0 && i + 2 <= pos && MText[i + 1] == L' '){
				*start = i + 2;
				break;
			}
			if (res != 0){ lastres = res; }
			if (res != 0 && !hasres){
				if (i == pos){ hasres = true; continue; }
				bool isen = (MText[i] == L'\\' && MText[i + 1] == L'N');
				*start = (isen && pos == i + 1) ? i : (isen) ? i + 2 : i + 1;
				break;
			}
			else if (hasres && res == 0){
				if (lastres < 1 && (i + 2 == pos || i + 1 == pos)){ hasres = false; continue; }
				*start = ((lastres>3 && lastres < 6 && i + 2 <= pos) || i + 1 == pos || MText[i + 2] == L' ') ? i + 1 : i + 2;
				break;
			}
		}
	}
	if (!end){ return; }
	*end = (fromend && end == NULL) ? 0 : len;
	for (int i = pos; i < len; i++){
		const wxUniChar &ch = MText[i];
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
			*end = (i > 0 && MText[i - 1] == L'\\' && MText[i] == L'N') ? i + 1 : i;
			break;
		}
	}


}

void TextEditor::ContextMenu(wxPoint mpos, int error)
{
	Menu menut;
	wxString err;
	wxArrayString suggs;
	if (error >= 0){ err = misspels[error]; }
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
		menut.Append(MENU_SPELLCHECKER_ON, _("Sprawdzanie pisowni"), L"", true, 
			NULL, NULL, ITEM_CHECK_AND_HIDE)->Check(Options.GetBool(SPELLCHECKER_ON));
		for (int k = 0; k < numOfLanguages; k++){
			languageMenu->Append(MENU_SPELLCHECKER_ON + k + 1, dics[k], L"", true, 
				NULL, NULL, (language == dics[k])? ITEM_RADIO : ITEM_NORMAL);
		}
		menut.Append(MENU_SPELLCHECKER_ON - 1, _("Zainstalowane języki"), languageMenu);
	}

	if (!err.IsEmpty()){
		menut.Append(TEXTM_ADD, wxString::Format(_("&Dodaj słowo \"%s\" do słownika"), err));
	}

	menut.Append(TEXTM_DEL, _("&Usuń"))->Enable(Selend.x != Cursor.x);
	menut.Append(MENU_SHOW_STATUS_BAR, _("Pokaż pasek stanu"), NULL, L"", 
		ITEM_CHECK)->Check(!Options.GetBool(TEXT_EDITOR_HIDE_STATUS_BAR));
	menut.Append(MENU_CHANGE_QUOTES, _("Automatycznie zamieniaj cydzysłów"), 
		NULL, L"", ITEM_CHECK)->Check(Options.GetBool(TEXT_EDITOR_CHANGE_QUOTES));
	
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
		int from = errors[error * 2];
		int to = errors[(error * 2) + 1];
		MText.replace(from, to - from + 1, suggs[id - 30200]);
		modified = true;
		CalcWrap();
		if (SpellCheckerOnOff){ CheckText(); }
		int newto = from + suggs[id - 30200].length();
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
		CalcWrap();
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
		//WinStruct<SHELLEXECUTEINFO> sei;
		//sei.lpFile = url.c_str();
		//sei.lpVerb = L"open";
		//sei.nShow = SW_RESTORE;
		//sei.fMask = SEE_MASK_FLAG_NO_UI; // we give error message ourselves

		//ShellExecuteEx(&sei);
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

	wxString whatcopy = MText.SubString(curx, selx - 1);
	if (wxTheClipboard->Open())
	{
		wxTheClipboard->SetData(new wxTextDataObject(whatcopy));
		wxTheClipboard->Close();
		wxTheClipboard->Flush();
	}
	if (cut){
		MText.Remove(curx, selx - curx);
		CalcWrap();
		if (SpellCheckerOnOff){ CheckText(); }
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
			whatpaste.Replace(L"\r", L"");
			whatpaste.Replace(L"\f", L"");
			whatpaste.Replace(L"\t", L" ");
			int curx = Cursor.x;
			int selx = Selend.x; if (curx > selx){ int tmp = curx; curx = selx; selx = tmp; }
			if (Selend.x != Cursor.x){
				MText.Remove(curx, selx - curx);
			}
			MText.insert(curx, whatpaste);
			modified = true;
			CalcWrap();
			if (SpellCheckerOnOff){ CheckText(); }
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

	for (size_t i = 0; i < errors.size(); i += 2){
		if (cpos.x >= errors[i] && cpos.x <= errors[i + 1]){
			return i / 2;
		}
	}


	return -1;
}

wxPoint TextEditor::PosFromCursor(wxPoint cur)
{
	int fw, fh;
	if (wraps.size() < 2 || wraps[cur.y] == cur.x){ fw = 0; }
	else{ 
		GraphicsRenderer *renderer = GraphicsRenderer::GetDirect2DRenderer();
		GraphicsContext *gc = renderer->CreateMeasuringContext();
		if (gc){
			gc->SetFont(font, L"#000000");
			double gcfw, gcfh;
			gc->GetTextExtent(MText.SubString(wraps[cur.y], cur.x), &gcfw, &gcfh);
			delete gc;
			fw = gcfw + 0.5f;
		}
		else{
			GetTextExtent(MText.SubString(wraps[cur.y], cur.x), &fw, &fh, NULL, NULL, &font);
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
	int i = pos;
	int brkts = 0;
	while ((fromback) ? i >= 0 : i < (int)MText.length())
	{
		if (MText[i] == sbrkt){ brkts++; }
		else if (MText[i] == ebrkt){ if (brkts == 0){ return i; }brkts--; }

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
		errors.Clear();

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

void TextEditor::DrawWordRectangles(int type, wxDC &dc, int h)
{
	const wxArrayInt & words = (type == 0) ? errors : selectionWords;
	size_t len = words.size();
	int fw = 0, fh = 0, fww = 0, fwww = 0;
	int lineStart = (scrollPositionV - 2) / fontHeight;
	int charStart = wraps[lineStart];
	int lineEnd = ((scrollPositionV - 2 + h) / fontHeight) + 1;
	int charEnd = (lineEnd < wraps.size()) ? wraps[lineEnd] : MText.length();

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
			wxString ftext = MText.SubString(wraps[fsty], startWord - 1);
			ftext.Replace(L"\t", L"");
			GetTextExtent(ftext, &fw, &fh, NULL, NULL, &font);
		}
		int scndy = FindY(endWord);
		wxString etext = MText.SubString(startWord, (fsty == scndy) ? endWord : wraps[fsty + 1] - 1);
		etext.Replace(L"\t", L"");
		GetTextExtent(etext, &fww, &fh, NULL, NULL, &font);
		for (int q = fsty + 1; q <= scndy; q++){
			int rest = (q == scndy) ? endWord : wraps[q + 1];
			wxString btext = MText.SubString(wraps[q], rest);
			btext.Replace(L"\t", L"");
			GetTextExtent(btext, &fwww, &fh, NULL, NULL, &font);
			dc.DrawRectangle(3, ((q * fontHeight) + 1) - scrollPositionV, fwww, fontHeight);
		}
		dc.DrawRectangle(fw + 3, ((fsty * fontHeight) + 1) - scrollPositionV, fww, fontHeight);
	}
}

void TextEditor::DrawWordRectangles(int type, GraphicsContext *gc, int h)
{
	const wxArrayInt & words = (type == 0) ? errors : selectionWords;
	size_t len = words.size();
	double fw = 0.0, fh = 0.0, fww = 0.0, fwww = 0.0;
	int lineStart = (scrollPositionV - 2) / fontHeight;
	int charStart = wraps[lineStart];
	int lineEnd = ((scrollPositionV - 2 + h) / fontHeight) + 1;
	int charEnd = (lineEnd < wraps.size())? wraps[lineEnd] : MText.length();

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
			wxString ftext = MText.SubString(wraps[fsty], startWord - 1);
			ftext.Replace(L"\t", L"");
			gc->GetTextExtent(ftext, &fw, &fh);
		}
		int scndy = FindY(endWord);
		wxString etext = MText.SubString(words[g], (fsty == scndy) ? endWord : wraps[fsty + 1] - 1);
		etext.Replace(L"\t", L"");
		gc->GetTextExtent(etext, &fww, &fh);
		for (int q = fsty + 1; q <= scndy; q++){
			int rest = (q == scndy) ? endWord : wraps[q + 1];
			wxString btext = MText.SubString(wraps[q], rest);
			btext.Replace(L"\t", L"");
			gc->GetTextExtent(btext, &fwww, &fh);
			gc->DrawRectangle(3, ((q * fontHeight) + 1) - scrollPositionV, fwww, fontHeight);
		}
		gc->DrawRectangle(fw + 3, ((fsty * fontHeight) + 1) - scrollPositionV, fww, fontHeight);
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
		for (int i = Cursor.x - 1; i >= 0; i--){
			if (MText[i] == L'\\'){
				//It would be nice to add brackets or some else elements;
				//Looks like it have to be added for all tags separely
				wxString tag;
				item->GetTag(&tag);
				int newPosition = Cursor.x + tag.length() - (Cursor.x - 1 - i);
				if (tag.EndsWith(L')'))
					newPosition--;

				Replace(i + 1, Cursor.x, tag);
				SetSelection(newPosition, newPosition);
				tagList->Destroy();
				tagList = NULL;
				return;
			}
		}
	}
}

//state here is for template and for disable spellchecker and wraps
void TextEditor::SetState(int _state, bool refresh){
	if (state == _state)
		return;

	state = _state;
	//if (refresh){
		SpellCheckerOnOff = (!state) ? Options.GetBool(SPELLCHECKER_ON) : false;
		if (SpellCheckerOnOff)
			CheckText();
		EB->UpdateChars();
		Refresh(false);
	//}
};

bool TextEditor::SetFont(const wxFont &_font)
{
	font = _font;
	int fw, fh;
	GetTextExtent(L"#TWFfGH", &fw, &fh, NULL, NULL, &font);
	fontHeight = fh;
	fontSizes.clear();
	CalcWrap();
	Refresh(false);
	return true;
}

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




