//  Copyright (c) 2021, Marcin Drob

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

class KaiContainer 
{
public:
	KaiContainer() {
		
	};
	void SetContainer(wxWindow* navigation) {
		container = navigation;
	};
	void OnNavigation(wxNavigationKeyEvent& evt);
	void FindFocusable(bool next, wxWindowListNode** node, wxWindow **window = NULL);
	wxWindow* FindCheckedRadiobutton(bool next, wxWindowListNode** listWithRadioButton, wxWindow* focused);
	void OnSetFocus(wxFocusEvent& evt);
	wxWindow * container;
};

template <class Window>
class KaiNavigation : public Window
{
public:
	KaiNavigation() {
		container.SetContainer(this);

		Window::Connect(wxEVT_NAVIGATION_KEY,
			wxNavigationKeyEventHandler(KaiNavigation::OnNavigationKey));

		Window::Connect(wxEVT_SET_FOCUS,
			wxFocusEventHandler(KaiNavigation::OnFocus));

		//Window::Connect(wxEVT_CHILD_FOCUS,
			//wxChildFocusEventHandler(KaiNavigation::OnChildFocus));
	}

private:
	void OnNavigationKey(wxNavigationKeyEvent& evt) {
		container.OnNavigation(evt);
	}

	void OnFocus(wxFocusEvent& evt) {
		container.OnSetFocus(evt);
	}

	KaiContainer container;
};

class KaiPanel : public KaiNavigation<wxWindow> 
{
public:
	KaiPanel(wxWindow* parent,
		wxWindowID winid = wxID_ANY,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxNO_BORDER) 
	{
		Create(parent, winid, pos, size, style | wxTAB_TRAVERSAL);
	};
};