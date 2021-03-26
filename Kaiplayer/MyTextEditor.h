//  Copyright (c) 2016 - 2020, Marcin Drob

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

#pragma once

#include <wx/wx.h>
#include <wx/caret.h>
#include "KaiScrollbar.h"
#include "TextEditorTagList.h"
#include <map>
#include "LineParse.h"

class EditBox;
class KainoteFrame;
class GraphicsContext;
wxDECLARE_EVENT(CURSOR_MOVED, wxCommandEvent);

class TextEditor : public wxWindow
{
public:
	TextEditor(wxWindow *parent, int id, bool spell, const wxPoint& pos = wxDefaultPosition, 
		const wxSize& size = wxDefaultSize, long style = wxWANTS_CHARS);
	virtual ~TextEditor();
	void SetTextS(const wxString &text, bool modif = false, bool resetsel = true, bool noevent = false);
	bool IsModified();
	void SetModified(bool _modified = true);
	void GetSelection(long *start, long* end);
	void SetSelection(int start, int end, bool noEvent = false);
	void Replace(int start, int end, const wxString &rep);
	void Copy(bool cut = false);
	void Paste();
	wxString GetValue() const;
	void SpellcheckerOnOff(bool on);
	void ClearSpellcheckerTable();
	//0-normal, 1-comment, 2-template line, 3-code template line
	void SetState(int _state, bool refresh = false);
	bool SetFont(const wxFont &font);
	const TextData &GetTextData();
	EditBox* EB;

protected:
	void CheckText();
	wxUniChar CheckQuotes();
	void ContextMenu(wxPoint mpos, int error);
	inline int FindY(int x);
	int FindError(wxPoint mpos, bool mouse = true);
	wxPoint PosFromCursor(wxPoint cur);
	void OnKeyPress(wxKeyEvent& event);
	void OnAccelerator(wxCommandEvent& event);
	void OnCharPress(wxKeyEvent& event);
	void OnMouseEvent(wxMouseEvent& event);
	void OnSize(wxSizeEvent& event);
	void OnPaint(wxPaintEvent& event);
	void OnKillFocus(wxFocusEvent& event);
	void OnEraseBackground(wxEraseEvent& event){};
	void OnLostCapture(wxMouseCaptureLostEvent &evt){ if (HasCapture()){ ReleaseMouse(); } };
	void OnScroll(wxScrollEvent& event);
	void DrawFieldGDI(wxDC &dc, int w, int h, int windowh);
	void DrawFieldD2D(GraphicsContext *gc, int w, int h, int windowh);
	bool HitTest(wxPoint pos, wxPoint *cur);
	void CalcWrap(bool updatechars = true, bool sendevent = true);
	void CalcWrapsGDI(int windowWidth);
	void CalcWrapsD2D(GraphicsContext *gc, int windowWidth);
	void FindWord(int pos, int *start, int *end);
	int FindBracket(wxUniChar sbrkt, wxUniChar ebrkt, int pos, bool fromback = false);
	void MakeCursorVisible();
	bool CheckIfKeyword(const wxString &word);
	void SeekSelected(const wxString &word);
	void DrawWordRectangles(int type, wxDC &dc, int h);
	void DrawWordRectangles(int type, GraphicsContext *gc, int h, int posX);
	bool GetNumberFromCursor(int cursorPos, wxPoint &numberPos, float &number, float &step);
	void PutTag();

	bool SpellCheckerOnOff;
	bool useSpellchecker;
	bool changeQuotes;
	wxString MText;
	wxBitmap* bmp;
	KaiScrollbar *scroll;
	PopupTagList *tagList = NULL;
	wxFont font;
	std::vector<MisspellData> misspells;
	wxCaret *caret;
	std::vector<int> wraps;
	TextData errors;
	wxArrayInt selectionWords;
	//adding new keywords change its num in CheckIfKeyword
	static wxString LuaKeywords[13];
	int oldstart, oldend;
	int posY;
	wxPoint Cursor;
	wxPoint Selend;
	wxPoint Brackets;
	wxPoint dclickCurPos;
	bool holding;
	bool dholding;
	bool firstdhold;
	bool wasDoubleClick;
	bool modified;
	bool hasRTL = false;
	int numberChangingMousePos = -1;
	int state = 0;
	int time;
	int fontHeight;
	int scrollPositionV;
	int fontSize;
	int tmpstart, tmpend;
	int statusBarHeight;
	int lastWidth = 0;
	int lastHeight = 0;
	std::map<wxUniChar, double> fontSizes;
	std::map<wxUniChar, int> fontGDISizes;
	DECLARE_EVENT_TABLE()
};

enum{
	TEXTM_COPY = 16545,
	TEXTM_PASTE,
	TEXTM_CUT,
	TEXTM_DEL,
	TEXTM_ADD,
	TEXTM_SEEKWORDL,
	TEXTM_SEEKWORDB,
	TEXTM_SEEKWORDG,
	TEXTM_SEEKWORDS,
	MENU_SHOW_STATUS_BAR,
	MENU_CHANGE_QUOTES,
	MENU_SPELLCHECKER_ON = 18000,
	ID_DEL,
	ID_BACK,
	ID_CBACK,
	ID_CDELETE,
	ID_LEFT,
	ID_CLEFT,
	ID_SLEFT,
	ID_CSLEFT,
	ID_RIGHT,
	ID_CRIGHT,
	ID_SRIGHT,
	ID_CSRIGHT,
	ID_UP,
	ID_SUP,
	ID_DOWN,
	ID_SDOWN,
	ID_CTLA,
	ID_CTLV,
	ID_CTLC,
	ID_CTLX,
	ID_ENTER,
	ID_WMENU,
};

