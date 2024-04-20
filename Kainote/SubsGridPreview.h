//  Copyright (c) 2012 - 2020, Marcin Drob

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
#include "KaiListCtrl.h"
//#include "SubsGrid.h"
//#include "TabPanel.h"
#include <vector>

class SubsGrid;
class TabPanel;


class MultiPreviewData{
public:
	MultiPreviewData(){}
	MultiPreviewData(TabPanel *_tab, SubsGrid *_grid, int _lineRangeStart, int _lineRangeEnd = 1){
		tab = _tab;
		grid = _grid;
		lineRangeStart = _lineRangeStart;
		lineRangeLen = _lineRangeEnd;
	};
	bool operator == (const MultiPreviewData data){ return data.grid == grid && data.lineRangeStart == lineRangeStart; }
	TabPanel *tab = nullptr;
	SubsGrid *grid = nullptr;
	int lineRangeStart = 0;
	int lineRangeLen = 1;
};

class SubsGridPreview : public wxWindow
{
	friend class SubsGridBase;
	friend class SubsGrid;
public:
	SubsGridPreview(SubsGrid *_previewGrid, SubsGrid *windowToDraw, int posY, const wxSize &size);
	virtual ~SubsGridPreview();
	void MakeVisible();
	void DestroyPreview(bool refresh = false, bool destroyingPreviewTab = false);
	void NewSeeking(bool makeVisible = true);
private:
	void OnPaint(wxPaintEvent &evt);
	void OnMouseEvent(wxMouseEvent &evt);
	void OnSize(wxSizeEvent &evt);
	void OnScroll(wxScrollEvent &evt);
	void OnLostCapture(wxMouseCaptureLostEvent &evt){ if (HasCapture()){ ReleaseMouse(); } holding = false; };
	//void OnOccurenceChanged(wxCommandEvent &evt);
	void OnAccelerator(wxCommandEvent &evt);
	void SeekForOccurences();
	void ContextMenu(const wxPoint &pos);
	void OnFocus(wxFocusEvent &evt);
	//size_t GetKeyFromScrollPos(size_t numOfLines);

	SubsGrid *previewGrid;
	SubsGrid *parent;
	wxBitmap *bmp=nullptr;
	//int scrollPosition = 0;
	//int scrollPositionId = 0;
	int scHor = 0;
	int oldX = -1;
	//int selectedItem = 0;
	bool holding = false;
	bool onX = false;
	bool pushedX = false;
	KaiScrollbar *scrollbar=nullptr;
	KaiListCtrl *occurencesList = nullptr;
	std::vector<MultiPreviewData> previewData;
	MultiPreviewData lastData;
};

enum{
	PREVIEW_COPY=5432,
	PREVIEW_PASTE
};