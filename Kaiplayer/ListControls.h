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

#include "wx/wx.h"
#include "Menu.h"

class KaiChoice :public wxWindow
{
public:
	KaiChoice(wxWindow *parent, int id, const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize, int n = 0, const wxString choices[] = NULL,
        long style = 0, const wxValidator& validator = wxDefaultValidator);

	KaiChoice(wxWindow *parent, int id, const wxPoint& pos,
        const wxSize& size, const wxArrayString &choices,
        long style = 0, const wxValidator& validator = wxDefaultValidator);

	virtual ~KaiChoice();
	void SetSelection(int sel);
	void Clear();
	void Append(wxString what);
	/*void Prepend(wxString what);
	void Insert(wxString what, int position);*/
	wxString GetString(int pos){return (*list)[pos];};
	int GetSelection(){return choice;};
	void Select(int sel){choice=sel;}
private:
	void OnSize(wxSizeEvent& event);
	void OnPaint(wxPaintEvent& event);
	void OnMouseEvent(wxMouseEvent &event);
	void OnKeyPress(wxKeyEvent &event);
	void ShowList();

	bool enter;
	bool clicked;
	wxArrayString *list;
	wxBitmap *bmp;
	Menu *listMenu;
	int choice;
	bool listIsShown;
	bool choiceChanged;
};
