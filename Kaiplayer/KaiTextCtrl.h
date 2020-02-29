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

#pragma once

#include <wx/wx.h>
#include <wx/caret.h>
#include "KaiTextValidator.h"
#include "KaiScrollbar.h"
#include "config.h"

class TextStyle{
public:
	TextStyle(){};
	TextStyle(size_t _from, size_t _to, const wxColour & col){ from = _from; to = _to; color = col; };

	size_t from=0;
	size_t to=0;
	wxColour color;
	//maybe later I add font when I make custom dc class with GDI plus
};
class wxGraphicsContext;

class KaiTextCtrl : public KaiScrolledWindow
{
public:
	KaiTextCtrl(wxWindow *parent, int id, const wxString &text="", const wxPoint& pos=wxDefaultPosition,
		const wxSize& size=wxDefaultSize, long style=0, const wxValidator & validator = wxDefaultValidator, const wxString & name = ""); 
	virtual ~KaiTextCtrl();
	void SetValue(const wxString &text,bool modif=false, bool newSel = true);
	bool Modified();
	void GetSelection(long *start, long* end);
	void SetSelection(unsigned int start, unsigned int end, bool noEvent = false);
	void SetWindowStyle(long style);
	void Replace(int start, int end, const wxString & rep, bool sendEvent = true);
	void Copy(bool cut=false);
	void Paste();
	wxString GetValue() const;
	bool IsModified(){return modified;};
	void MarkDirty(){modified=true;}
	bool modified;
	bool SetForegroundColour(COLOR color){ foreground = color; Refresh(false); return true; }
	bool SetBackgroundColour(COLOR color){ background = color; Refresh(false); return true; }
	bool SetBackgroundColour(const wxColour & color){
		wxWindowBase::SetBackgroundColour(color);
		Refresh(false); return true;
	}
	bool SetForegroundColour(const wxColour & color){
		wxWindowBase::SetForegroundColour(color);
		foreground = (COLOR)0;
		Refresh(false); return true;
	}
	COLOR GetForegroundColour() const {return foreground;}
	COLOR GetBackgroundColour() const {return background;}
	void SetModified(bool modif){modified = modif;}
	void SetMaxLength(int maxLen);
	void AppendText(const wxString &text);
	//now only color;font later
	void AppendTextWithStyle(const wxString &text, const wxColour &color);
	void SetStyle(size_t from, size_t to, const wxColour &color);
	bool FindStyle(size_t pos, size_t *ret, bool returnSize = false);
	void MoveStyles(size_t textPos, int moveIndex);
	void DeleteStyles(size_t textStart, size_t textEnd);
	//void SetValidator(const wxValidator &validator){};
	size_t GetLength(){ return KText.Len(); }
	bool Enable(bool enable=true);
	bool HitTest(wxPoint pos, wxPoint *cur);
	void FindWord(int pos, int *start, int *end);

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
	void DrawFld(wxDC &dc,int w, int h);
	void DrawFieldD2D(wxGraphicsContext *gc, int w, int h);
	void CalcWrap(bool sendevent=true, size_t position = 0);
	void SendEvent();
	void GetTextExtent(const wxString &textToMesure, int *textWidth, int *textHeight);
	void GetTextExtent(wxGraphicsContext *gc, const wxString &textToMesure, double *textWidth, double *textHeight);
	void MakeCursorVisible(bool refresh=true);
	wxString KText;
	wxBitmap* bmp;
	wxFont font;
	wxCaret *caret;
	wxTimer timer;

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
	std::vector<int> wraps;
	wxArrayInt positioning;
	std::vector<TextStyle> textStyles;
	std::vector<int> charmap;
	wxMutex mutex;
	wxSize lastSize;
	long style;
	COLOR background;
	COLOR foreground;
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

