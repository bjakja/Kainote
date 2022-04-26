//  Copyright (c) 2017 - 2020, Marcin Drob

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

#include "SubsGridBase.h"
#include "SubsGridPreview.h"
#include "SubsGrid.h"
//#include "Graphicsd2d.h"
#include <wx/window.h>

//class SubsGrid;
class GraphicsContext;

class SubsGridWindow : public SubsGridBase
{
public:
	SubsGridWindow(wxWindow *parent, const long int id, const wxPoint& pos, const wxSize& size, long style);
	virtual ~SubsGridWindow();
	void AdjustWidths(int cell = 16383);
	void AdjustWidthsD2D(GraphicsContext *gc, int cell);
	void ChangeActiveLine(int newActiveLine, bool refresh = false, bool scroll = false, bool changeEditboxLine = true);
	void ChangeTimeDisplay(bool frame);
	void HideOverrideTags();
	void RefreshColumns(int cell = 16383);
	void RefreshIfVisible(int time);
	void ScrollTo(int y, bool center = false, int offset = 0, bool useUpdate = false);
	// default value -1 can cause problems with scrolling
	// that I removed it currentLine is easy to obtain
	void MakeVisible(int rowKey);
	void SelectRow(int row, bool addToSelected = false, bool select = true, bool norefresh = false);
	void SelVideoLine(int time = -1);
	void SetStyle();
	void SetVideoLineTime(wxMouseEvent &evt, int mvtal);
	void SetActive(int line);
	void ShowSecondComparedLine(int Line, bool showPreview = false, bool fromPreview = false, bool setViaScroll = false);
	void RefreshPreview();

	SubsGridPreview *preview = nullptr;

protected:
	void OnKeyPress(wxKeyEvent &event);
	void OnMouseEvent(wxMouseEvent &event);
	void OnPaint(wxPaintEvent& event);
	void OnScroll(wxScrollWinEvent& event);
	void OnSize(wxSizeEvent& event);
	void OnLostCapture(wxMouseCaptureLostEvent& evt);
	bool ShowPreviewWindow(SubsGridWindow *previewGrid, SubsGridWindow *windowToDraw,
		int activeLine, int diffPosition);
	void PaintD2D(GraphicsContext *gc, int w, int h, int size, int scrows, 
		wxPoint previewpos, wxSize previewsize, bool bg);
	int GridWidth[14];
	int posY = 0;
	int posX = 0;
	int row = 0;
	int extendRow = -1;
	int lastsel = -1;
	int oldX = -1;
	bool holding = false;
	int lastWidth = 0;
	int lastHeight = 0;

	wxBitmap* bmp;
	wxFont font;
	SubsGridPreview *thisPreview = nullptr;
private:
	virtual void ContextMenu(const wxPoint &pos) {};
	virtual void ContextMenuTree(const wxPoint &pos, int treeLine) {};
};

