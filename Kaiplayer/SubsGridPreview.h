//  Copyright (c) 2012-2017, Marcin Drob

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
#include "KaiScrollbar.h"


class SubsGrid;

class SubsGridPreview : public wxWindow
{
	friend class SubsGrid;
public:
	SubsGridPreview(SubsGrid *_previewGrid, SubsGrid *windowToDraw, int posY, const wxSize &size);
	virtual ~SubsGridPreview();
	void MakeVisible();
	void DestroyPreview(bool refresh = false);
private:
	void OnPaint(wxPaintEvent &evt);
	void OnMouseEvent(wxMouseEvent &evt);
	void OnSize(wxSizeEvent &evt);
	void OnScroll(wxScrollEvent &evt);
	void OnLostCapture(wxMouseCaptureLostEvent &evt){ if (HasCapture()){ ReleaseMouse(); } holding = false; };

	SubsGrid *previewGrid;
	SubsGrid *parent;
	wxBitmap *bmp=NULL;
	int scPos = 0;
	int scHor = 0;
	int oldX = -1;
	bool holding = false;
	bool onX = false;
	bool pushedX = false;
	KaiScrollbar *scrollbar=NULL;
};