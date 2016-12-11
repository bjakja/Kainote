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

#ifndef MYTEXTEDITOR
#define MYTEXTEDITOR


#include <wx/wx.h>
#include <wx/caret.h>

class EditBox;
class kainoteFrame;
wxDECLARE_EVENT(CURSOR_MOVED, wxCommandEvent);

class MTextEditor : public wxWindow
	{
	public:
	MTextEditor(wxWindow *parent, int id, bool spell, const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize, long style=wxWANTS_CHARS); 
	virtual ~MTextEditor();
	void SetTextS(const wxString &text,bool modif=false, bool resetsel = true, bool noevent=false);
	bool Modified();
	void GetSelection(long *start, long* end);
	void SetSelection(int start, int end, bool noEvent=false);
	void Replace(int start, int end, wxString rep);
	void Copy(bool cut=false);
	void Paste();
	wxString GetValue() const;
	void SpellcheckerOnOff();
	EditBox* EB;
	bool modified;
	protected:
	void CheckText();
	void ContextMenu(wxPoint mpos, int error);
	inline int FindY(int x);
	int FindError(wxPoint mpos,bool mouse=true);
	wxPoint PosFromCursor(wxPoint cur);
	void OnKeyPress(wxKeyEvent& event);
	void OnAccelerator(wxCommandEvent& event);
	void OnCharPress(wxKeyEvent& event);
	void OnMouseEvent(wxMouseEvent& event);
    void OnSize(wxSizeEvent& event);
	void OnPaint(wxPaintEvent& event);
	void OnKillFocus(wxFocusEvent& event);
	void OnEraseBackground(wxEraseEvent& event){};
	void OnLostCapture(wxMouseCaptureLostEvent &evt){if(HasCapture()){ReleaseMouse();}};
	void OnScroll(wxScrollEvent& event);
	void DrawFld(wxDC &dc,int w, int h, int windowh);
	bool HitTest(wxPoint pos, wxPoint *cur);
	void CalcWrap(bool updatechars=true, bool sendevent=true);
	void FindWord(int pos,int *start, int *end);
	int FindBracket(wxUniChar sbrkt, wxUniChar ebrkt, int pos, bool fromback=false);
	bool spell;
    wxString MText;
	wxBitmap* bmp;
	wxScrollBar *scroll;
	wxFont font;
	wxArrayString errs;
	wxCaret *caret;
	wxArrayInt wraps;
	wxArrayInt errors;

	int oldstart, oldend;
	int posY;
	wxPoint Cursor;
	wxPoint Selend;
	wxPoint Brackets;
	bool holding;
	bool dholding;
	bool firstdhold;
	bool wasDoubleClick;
	int time;
	int Fheight;
	int scPos;
	int fsize;
	int tmpstart, tmpend;

	DECLARE_EVENT_TABLE()
	};

enum{
	TEXTM_COPY=16545,
	TEXTM_PASTE,
	TEXTM_CUT,
	TEXTM_DEL,
	TEXTM_ADD,
	TEXTM_SEEKWORDL,
	TEXTM_SEEKWORDB,
	TEXTM_SEEKWORDG,
	ID_DEL,
	ID_BACK,
	ID_CBACK,
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
	ID_HOME,
	ID_END,
	ID_SHOME,
	ID_SEND,
	ID_CSEND,
	ID_CTLA,
	ID_CTLV,
	ID_CTLC,
	ID_CTLX,
	ID_WMENU,
	ID_PDOWN,
	ID_PUP
};

#endif