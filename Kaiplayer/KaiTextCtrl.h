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

#ifndef __KAI_TEXT_CTRL__
#define __KAI_TEXT_CTRL__

#include <wx/wx.h>
#include <wx/caret.h>
#include "KaiTextValidator.h"
#include "KaiScrollbar.h"

class KaiTextCtrl : public KaiScrolledWindow
{
public:
	KaiTextCtrl(wxWindow *parent, int id, const wxString &text="", const wxPoint& pos=wxDefaultPosition,
		const wxSize& size=wxDefaultSize, long style=0, const wxValidator & validator = wxDefaultValidator, const wxString & name = ""); 
	virtual ~KaiTextCtrl();
	void SetValue(const wxString &text,bool modif=false, bool newSel = true);
	bool Modified();
	void GetSelection(long *start, long* end);
	void SetSelection(int start, int end, bool noEvent=false);
	void SetWindowStyle(long style);
	void Replace(int start, int end, wxString rep);
	void Copy(bool cut=false);
	void Paste();
	wxString GetValue() const;
	bool IsModified(){return modified;};
	void MarkDirty(){modified=true;}
	bool modified;
	bool SetForegroundColour(const wxColour &color){foreground = color; Refresh(false);return true;}
	bool SetBackgroundColour(const wxColour &color){background = color; Refresh(false);return true;}
	wxColour GetForegroundColour() const {return foreground;}
	wxColour GetBackgroundColour() const {return background;}
	void SetModified(bool modif){modified = modif;}
	void SetMaxLength(int maxLen){maxSize = maxLen;}
	void AppendText(const wxString &text);
	//void SetValidator(const wxValidator &validator){};
	bool Enable(bool enable=true);
protected:
	void ContextMenu(wxPoint mpos);
	inline int FindY(int x);
	wxPoint PosFromCursor(wxPoint cur, bool correctToScroll = true);
	void OnKeyPress(wxKeyEvent& event);
	void OnAccelerator(wxCommandEvent& event);
	void OnCharPress(wxKeyEvent& event);
	void OnMouseEvent(wxMouseEvent& event);
    void OnSize(wxSizeEvent& event);
	void OnPaint(wxPaintEvent& event);
	void OnKillFocus(wxFocusEvent& event);
	void OnEraseBackground(wxEraseEvent& event){};
	void OnLostCapture(wxMouseCaptureLostEvent &evt){if(HasCapture()){ReleaseMouse();}};
	void OnScroll(wxScrollWinEvent& event);
	void DrawFld(wxDC &dc,int w, int h, int windoww, int windowh);
	bool HitTest(wxPoint pos, wxPoint *cur);
	void CalcWrap(bool sendevent=true);
	void FindWord(int pos,int *start, int *end);
	void GetTextExtent(const wxString &textToMesure, int *textWidth, int *textHeight, wxFont *textFont=NULL, bool correct=false);
	void MakeCursorVisible(bool refresh=true);
    wxString KText;
	wxBitmap* bmp;
	wxFont font;
	wxCaret *caret;

	int oldstart, oldend;
	int posY;
	wxPoint Cursor;
	wxPoint Selend;
	bool holding;
	bool dholding;
	bool firstdhold;
	int Fheight;
	int scPos;
	int fsize;
	int tmpstart, tmpend;
	size_t maxSize;
	wxArrayInt wraps;
	wxArrayInt positioning;
	long style;
	wxColour background;
	wxColour foreground;
	wxDECLARE_ABSTRACT_CLASS(KaiTextCtrl);
	DECLARE_EVENT_TABLE()

};

//class KaiTextValidator : public wxValidator
//{
//public:
//	KaiTextValidator(long style=wxFILTER_NONE, wxString *valPtr=NULL) :wxTextValidator(style, valPtr){};
//	virtual ~KaiTextValidator(){}
//	virtual bool Validate(wxWindow *parent);
//    virtual bool TransferToWindow();
//    virtual bool TransferFromWindow();
//	KaiTextCtrl *GetKaiTextCtrl();
//};

enum{
	SCROLL_ON_FOCUS=2,
	TEXT_COPY=16545,
	TEXT_PASTE,
	TEXT_CUT,
	TEXT_DEL,
	TEXT_ADD,
	TEXT_SEEKWORDL,
	TEXT_SEEKWORDB,
	TEXT_SEEKWORDG,
	ID_TDEL,
	ID_TBACK,
	ID_TCBACK,
	ID_TLEFT,
	ID_TCLEFT,
	ID_TSLEFT,
	ID_TCSLEFT,
	ID_TRIGHT,
	ID_TCRIGHT,
	ID_TSRIGHT,
	ID_TCSRIGHT,
	ID_TUP,
	ID_TSUP,
	ID_TDOWN,
	ID_TSDOWN,
	ID_THOME,
	ID_TEND,
	ID_TSHOME,
	ID_TSEND,
	ID_TCSEND,
	ID_TCTLA,
	ID_TCTLV,
	ID_TCTLC,
	ID_TCTLX,
	ID_TWMENU,
	ID_TPDOWN,
	ID_TPUP,
	ID_TRETURN
};

#endif