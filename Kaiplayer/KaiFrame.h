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

#include <wx/toplevel.h>
#include <wx/sizer.h>
class wxGraphicsContext;


class KaiFrame : public wxTopLevelWindow
{
public:
	KaiFrame(wxWindow *parent, wxWindowID id, const wxString& title="", const wxPoint& pos=wxDefaultPosition, const wxSize& size=wxDefaultSize, long style=0, const wxString &name = "");
	virtual ~KaiFrame();
	void SetLabel(const wxString &text);
	virtual void SetStatusText(const wxString &label, int field){};
	void GetClientSize(int *w, int *h) const;
	wxSize GetClientSize() const;
	void SetClientSize(const wxSize &size);
	void SetClientSize(int x, int y);
	void GetBorders(int *borders, int *topBorder){
		*borders = frameBorder;
		*topBorder = frameTopBorder;
	};
	bool SetFont(const wxFont &font);
private:
	void OnPaint(wxPaintEvent &evt);
	void PaintD2D(wxGraphicsContext *gc, int w, int h);
	void OnSize(wxSizeEvent &evt);
	void OnMouseEvent(wxMouseEvent &evt);
	WXLRESULT MSWWindowProc(WXUINT message, WXWPARAM wParam, WXLPARAM lParam);
	void OnEraseBackground(wxEraseEvent()){}
	
	long style;
	wxPoint diff;
	bool enterClose;
	bool pushedClose;
	bool enterMaximize;
	bool pushedMaximize;
	bool enterMinimize;
	bool pushedMinimize;
	bool isActive;
	int frameBorder = 7;
	int frameTopBorder = 26;

	wxDECLARE_ABSTRACT_CLASS(KaiFrame);
};

