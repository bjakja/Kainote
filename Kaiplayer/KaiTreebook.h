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

#include <wx/window.h>
#include <vector>

class Page{
public:
	Page(wxWindow *_page, const wxString &_name, int _whichSupbage = 0){
		page = _page; name = _name; whichSubpage = _whichSupbage;
		collapsed = false; canCollapse = false;
	}
	~Page(){ page->Destroy(); }
	bool SetFont(const wxFont &font);
	wxWindow *page;
	wxString name;
	bool collapsed;
	bool canCollapse;
	int whichSubpage;
};

class KaiTreebook :public wxWindow
{
public:
	KaiTreebook(wxWindow *parent, int id,
		const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0);
	virtual ~KaiTreebook();
	void AddPage(wxWindow *page, const wxString &name, bool selected = false);
	void AddSubPage(wxWindow *page, const wxString &name, bool nextTree = false, bool selected = false);
	void Fit();

	int GetSelection();
	void ChangeSelection(int sel);
	void RefreshTree();
	void SetColours(const wxColour &bgcol, const wxColour &fgcol);
	//bool SetFont(const wxFont &font);
private:
	void OnKeyPress(wxKeyEvent& event);
	void OnMouseEvent(wxMouseEvent& event);
	void OnSize(wxSizeEvent& event);
	void OnPaint(wxPaintEvent& event);
	void ChangePage(int page);
	void CalcWidth();
	int CalcElement(int element, int *lastPage = NULL);
	std::vector<Page *> Pages;
	wxBitmap *bmp;
	int treeWidth;
	int selection;
	int textHeight = 12;
	DECLARE_EVENT_TABLE()
};

