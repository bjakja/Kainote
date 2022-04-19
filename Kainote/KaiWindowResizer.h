//  Copyright (c) 2018 - 2020, Marcin Drob

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

//#include "wx/window.h"
//#include "wx/dialog.h"
#include <functional>


class KaiWindowResizer : public wxWindow
{
public:
	KaiWindowResizer(wxWindow *parent/*, wxWindow *_windowBeforeResizer*/, std::function<bool(int)> _canResize, std::function<void(int, bool)> _doResize);
	virtual ~KaiWindowResizer();

private:
	void OnMouseEvent(wxMouseEvent &evt);
	void OnPaint(wxPaintEvent& evt);
	bool AcceptsFocus() const { return false; }
	bool AcceptsFocusFromKeyboard() const { return false; }
	bool AcceptsFocusRecursively() const { return false; }
	wxDialog* splitLine = nullptr;
	std::function<bool(int)> canResize;
	std::function<void(int, bool)> doResize;
	wxWindow *resizerParent;
	bool holding = false;
	int oldy = -1;
	wxBitmap *bmp = nullptr;
};