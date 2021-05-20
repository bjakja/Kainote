//  Copyright (c) 2016 - 2020, Marcin Drob

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

#include "Utils.h"
#include "KaiScrollbar.h"
#include "config.h"
#include <wx/dc.h>
#include <wx/dcmemory.h>
#include <wx/dcclient.h>

int KaiScrollbar::thickness = 17;

KaiScrollbar::KaiScrollbar(wxWindow *parent, int id, const wxPoint &pos, const wxSize &size, int style)
	: wxWindow(parent, id, pos, size)
	, isVertical((style & wxVERTICAL) != 0)
	, holding(false)
	, rholding(false)
	, bmp(NULL)
	, enter(false)
	, integrated(false)
	, pushed(false)
	, twoScrollbars(false)
	, unitPos(0)
	, scrollRate(1)
	, element(0)
{
	thickness = CalculateThickness(parent);

	if (pos == wxDefaultPosition){
		SetPosition((style & wxHORIZONTAL) ? wxPoint(0, parent->GetClientSize().y - thickness) :
			wxPoint(parent->GetClientSize().x - thickness, 0));
	}
	if (size == wxDefaultSize){
		SetSize((style & wxHORIZONTAL) ? wxSize(parent->GetClientSize().x, thickness) :
			wxSize(thickness, parent->GetClientSize().y));
	}
	pageLoop.SetOwner(this, 2345);
	Bind(wxEVT_TIMER, [=](wxTimerEvent &evt){
		if (unitPos == 0 || unitPos == allVisibleSize){ pageLoop.Stop(); return; }
		if (diff <= thumbPos){ unitPos -= pageSize; }
		else if (diff >= thumbPos + thumbSize){ unitPos += pageSize; }
		else{ pageLoop.Stop(); pushed = true; Refresh(false); return; }
		if (unitPos < 0){ unitPos = 0; }
		if (unitPos > allVisibleSize){ unitPos = allVisibleSize; }
		if (allVisibleSize)
			thumbPos = (((float)unitPos / (float)allVisibleSize) * thumbRange) + thickness;
		else
			thumbPos = 0;
		SendEvent();
		if (pageLoop.GetInterval() == 500){ pageLoop.Start(100); }
	}, 2345);
	arrowLoop.SetOwner(this, 2346);
	Bind(wxEVT_TIMER, [=](wxTimerEvent &evt){
		//float unitToPixel = (float)thumbSize / (float)thumbRange;
		if (element & ELEMENT_BUTTON_BOTTOM && unitPos < allVisibleSize){
			unitPos += scrollRate;
			if (unitPos > allVisibleSize){ unitPos = allVisibleSize; }
			//thumbPos = (unitPos * unitToPixel)+17;
			if (allVisibleSize)
				thumbPos = (((float)unitPos / (float)allVisibleSize) * thumbRange) + thickness;
			else
				thumbPos = 0;
			SendEvent();
		}
		else if (element & ELEMENT_BUTTON_TOP && unitPos > 0){
			unitPos -= scrollRate;
			if (unitPos < 0){ unitPos = 0; }
			//thumbPos = (unitPos * unitToPixel)+17;
			if (allVisibleSize)
				thumbPos = (((float)unitPos / (float)allVisibleSize) * thumbRange) + thickness;
			else
				thumbPos = 0;
			SendEvent();
		}
		if (arrowLoop.GetInterval() == 500){ arrowLoop.Start(50); }
	}, 2346);
	//Bind(wxEVT_LEFT_DOWN, &KaiScrollbar::OnMouseEvent, this);
	//Bind(wxEVT_LEFT_UP, &KaiScrollbar::OnMouseEvent, this);
	//Bind(wxEVT_MOTION, &KaiScrollbar::OnMouseEvent, this);
	//Bind(wxEVT_LEAVE_WINDOW, &KaiScrollbar::OnMouseEvent, this);
	Bind(wxEVT_MOUSE_CAPTURE_LOST, &KaiScrollbar::OnMouseLost, this);
	Bind(wxEVT_SIZE, &KaiScrollbar::OnSize, this);
	Bind(wxEVT_PAINT, &KaiScrollbar::OnPaint, this);
	Bind(wxEVT_ERASE_BACKGROUND, &KaiScrollbar::OnErase, this);
}


void KaiScrollbar::SetScrollbar(int pos, int visible, int range, int _pageSize, bool refresh)
{
	wxSize oldSize = GetClientSize();
	int doubleThickness = thickness * 2;
	if (twoScrollbars){
		if (isVertical){ oldSize.y -= thickness; }
		else{ oldSize.x -= thickness; }
	}
	if (holding /*|| (unitPos >= range-visible && pos >= unitPos)*/){ return; }
	allVisibleSize = range - visible;
	unitPos = pos;
	visibleSize = visible;
	allSize = range;
	pageSize = _pageSize;
	float divScroll = (float)visibleSize / (float)allSize;
	if (pos >= allVisibleSize){ unitPos = allVisibleSize; }
	int paneSize = (isVertical) ? (oldSize.y - doubleThickness) : (oldSize.x - doubleThickness);
	thumbSize = paneSize * divScroll;
	if (thumbSize < 16){ thumbSize = 16; }
	thumbRange = paneSize - thumbSize;
	if (allVisibleSize)
		thumbPos = (((float)unitPos / (float)allVisibleSize) * thumbRange) + thickness;
	else
		thumbPos = 0;

	Refresh(false);
	Update();
}

void KaiScrollbar::SetTwoscrolbars(bool _twoScrollbars)
{
	twoScrollbars = _twoScrollbars;
	wxSize oldSize = GetClientSize();
	int doubleThickness = thickness * 2;
	if (twoScrollbars){
		float div = 1;
		if (isVertical){ div = (float)oldSize.y / (float)(oldSize.y - thickness); oldSize.y -= thickness; }
		else{ div = (float)oldSize.x / (float)(oldSize.x - thickness); oldSize.x -= thickness; }
		allVisibleSize *= div;
		visibleSize /= div;
	}
	float divScroll = (float)visibleSize / (float)allSize;
	int paneSize = (isVertical) ? (oldSize.y - doubleThickness) : (oldSize.x - doubleThickness);
	thumbSize = paneSize * divScroll;
	if (thumbSize < 16){ thumbSize = 16; }
	thumbRange = paneSize - thumbSize;
	if (allVisibleSize)
		thumbPos = (((float)unitPos / (float)allVisibleSize) * thumbRange) + thickness;
	else
		thumbPos = 0;
	Refresh(false);
	Update();
}

int KaiScrollbar::SetScrollPos(int pos)
{
	unitPos = MID(0, pos, allVisibleSize);
	if (allVisibleSize)
		thumbPos = (((float)unitPos / (float)allVisibleSize) * thumbRange) + thickness;
	else
		thumbPos = 0;
	Refresh(false);
	Update();
	return unitPos;
}

int KaiScrollbar::CalculateThickness(wxWindow *win)
{
	int fw, fh;
	win->GetTextExtent(L"XX#%", &fw, &fh, NULL, NULL, Options.GetFont());
	return fh + 1;
}

void KaiScrollbar::SetThickness(wxWindow * win)
{
	thickness = CalculateThickness(win);
}

void KaiScrollbar::SendEvent()
{
	Refresh(false);
	Update();
	if (integrated){
		wxScrollWinEvent evt2(wxEVT_SCROLLWIN_THUMBTRACK, unitPos, (isVertical) ? wxVERTICAL : wxHORIZONTAL);
		((KaiScrolledWindow*)GetParent())->ProcessEvent(evt2);
	}
	else{
		wxScrollEvent evt2(wxEVT_SCROLL_THUMBTRACK, GetId());
		evt2.SetPosition(unitPos);
		AddPendingEvent(evt2);
	}
}

void KaiScrollbar::OnSize(wxSizeEvent& evt)
{
	wxSize oldSize = GetClientSize();
	int doubleThickness = thickness * 2;
	if (twoScrollbars){
		if (isVertical){ oldSize.y -= thickness; }
		else{ oldSize.x -= thickness; }
	}
	int paneSize = (isVertical) ? (oldSize.y - doubleThickness) : (oldSize.x - doubleThickness);
	float divScroll = (float)visibleSize / (float)allSize;
	thumbSize = paneSize * divScroll;
	if (thumbSize < 16){ thumbSize = 16; }
	thumbRange = paneSize - thumbSize;
	if (allVisibleSize)
		thumbPos = (((float)unitPos / (float)allVisibleSize) * thumbRange) + thickness;
	else
		thumbPos = 0;
	Refresh(false);
	Update();
}

void KaiScrollbar::OnPaint(wxPaintEvent& evt)
{
	int w = 0;
	int h = 0;
	int ow, oh;
	int thumbSize1 = thickness * 0.78f;//13
	int thumbPos1 = (thickness - thumbSize1) / 2;//2
	int arrowPos1 = (thickness - 11) / 2;//2
	int doubleThumbSize = thumbSize1 * 2;//26
	GetClientSize(&ow, &oh);
	if (ow == 0 || oh == 0){ return; }
	if (twoScrollbars){
		if (isVertical){ h = oh - thickness; w = ow; }
		else{ w = ow - thickness; h = oh; }
	}
	else{ w = ow; h = oh; }
	wxMemoryDC tdc;
	if (bmp && (bmp->GetWidth() < ow || bmp->GetHeight() < oh)) {
		delete bmp;
		bmp = NULL;
	}
	if (!bmp){ bmp = new wxBitmap(ow, oh); }
	tdc.SelectObject(*bmp);
	const wxColour & background = Options.GetColour(SCROLLBAR_BACKGROUND);
	wxColour scroll = (enter && !pushed) ? Options.GetColour(SCROLLBAR_THUMB_HOVER) :
		(pushed) ? Options.GetColour(SCROLLBAR_THUMB_PUSHED) :
		Options.GetColour(SCROLLBAR_THUMB);
	tdc.SetPen(wxPen(background));
	tdc.SetBrush(wxBrush(background));
	tdc.DrawRectangle(0, 0, ow, oh);
	wxBitmap scrollArrow = wxBITMAP_PNG("arrow_list");
	wxBitmap scrollArrowPushed = wxBITMAP_PNG("arrow_list_pushed");
	if (isVertical){
		if (h > thumbSize + doubleThumbSize){
			tdc.SetPen(wxPen(scroll));
			tdc.SetBrush(wxBrush(scroll));
			tdc.DrawRectangle(thumbPos1, thumbPos, thumbSize1, thumbSize);
		}
		wxImage img = (holding && element & ELEMENT_BUTTON_TOP) ? scrollArrowPushed.ConvertToImage() : scrollArrow.ConvertToImage();
		img = img.Rotate180();
		tdc.DrawBitmap(wxBitmap(img), arrowPos1, arrowPos1);
		tdc.DrawBitmap((holding && element & ELEMENT_BUTTON_BOTTOM) ? scrollArrowPushed : scrollArrow, arrowPos1, h - (10 + arrowPos1));
	}
	else{
		if (w > thumbSize + doubleThumbSize && thumbPos){
			tdc.SetPen(wxPen(scroll));
			tdc.SetBrush(wxBrush(scroll));
			tdc.DrawRectangle(thumbPos, thumbPos1, thumbSize, thumbSize1);
		}
		wxImage img = (holding && element & ELEMENT_BUTTON_TOP) ? scrollArrowPushed.ConvertToImage() : scrollArrow.ConvertToImage();
		img = img.Rotate90();
		tdc.DrawBitmap(wxBitmap(img), arrowPos1, arrowPos1);
		img = (holding && element & ELEMENT_BUTTON_BOTTOM) ? scrollArrowPushed.ConvertToImage() : scrollArrow.ConvertToImage();
		img = img.Rotate180().Rotate90();
		tdc.DrawBitmap(wxBitmap(img), w - (11 + arrowPos1), arrowPos1);
	}
	wxPaintDC dc(this);
	dc.Blit(0, 0, ow, oh, &tdc, 0, 0);
}

void KaiScrollbar::OnMouseEvent(wxMouseEvent &evt)
{
	if (evt.Leaving()){
		if (pageLoop.IsRunning()){
			pageLoop.Stop();
		}
		if (!holding){
			enter = false;
			Refresh(false);
			return;
		}
	}

	int x = evt.GetX();
	int y = evt.GetY();
	int w = 0;
	int h = 0;
	GetClientSize(&w, &h);
	if (twoScrollbars){
		if (isVertical){ h -= thickness; }
		else{ w -= thickness; }
	}
	int coord = (isVertical) ? y : x;
	int coord2 = (isVertical) ? x : y;
	int size = (isVertical) ? h : w;
	int size2 = (isVertical) ? w : h;
	if (evt.GetWheelRotation() != 0) {
		int step = (evt.GetWheelRotation() / evt.GetWheelDelta()) * scrollRate;
		unitPos -= step;
		if (unitPos < 0){ unitPos = 0; SendEvent(); return; }
		if (unitPos > allVisibleSize){ unitPos = allVisibleSize;  SendEvent(); return; }
		if (allVisibleSize)
			thumbPos = (((float)unitPos / (float)allVisibleSize) * thumbRange) + thickness;
		else
			thumbPos = 0;
		if (coord >= thumbPos && coord <= thumbPos + thumbSize){
			enter = true;
		}
		else if (enter){ enter = false; }
		SendEvent();
		return;
	}
	if (evt.ButtonUp()){
		if (pageLoop.IsRunning()){ pageLoop.Stop(); }
		if (arrowLoop.IsRunning()){ arrowLoop.Stop(); }
		if (HasCapture()){ ReleaseMouse(); }
		element = 0;
		enter = false;
		holding = false;
		pushed = false;
		Refresh(false);
		//return;
	}
	if ((evt.RightDown() || evt.RightDClick() || //prawy przycisk
		(evt.ShiftDown() && (evt.LeftDown() || evt.LeftDClick()))) && //lewy + shift
		(coord < size - thickness && coord > thickness)){ //blokada by nie dzia³a³o na przyciskach
		thumbPos = coord - (thumbSize / 2);
		thumbPos = MID(thickness, thumbPos, thumbRange + thickness);
		unitPos = ((thumbPos - thickness) / (float)thumbRange) * allVisibleSize;
		enter = true;
		pushed = true;
		holding = true;
		SendEvent();
		if (!HasCapture()){ CaptureMouse(); }
		return;
	}

	if (!holding){
		if (!enter && coord >= thumbPos && coord <= thumbPos + thumbSize && coord2 >= 0 && coord2 <= size2){
			enter = true;
			Refresh(false);
		}
		else if (enter && (coord < thumbPos || coord > thumbPos + thumbSize || coord2 < 0 || coord2 > size2)){
			enter = false;
			Refresh(false);
		}
	}

	if (evt.LeftDown() || evt.LeftDClick()){
		holding = true;
		if (coord >= thumbPos && coord <= thumbPos + thumbSize){
			element = ELEMENT_THUMB;
			pushed = true;
		}
		else if (coord >= size - (thickness + 1) && coord <= size && unitPos != allVisibleSize){
			unitPos += scrollRate;
			if (unitPos > allVisibleSize){ unitPos = allVisibleSize; }
			if (allVisibleSize)
				thumbPos = (((float)unitPos / (float)allVisibleSize) * thumbRange) + thickness;
			else
				thumbPos = 0;
			element = ELEMENT_BUTTON_BOTTOM;
			SendEvent();
			arrowLoop.Start(500);
		}
		else if (coord >= 0 && coord <= thickness && unitPos != 0){
			unitPos -= scrollRate;
			if (unitPos < 0){ unitPos = 0; }
			if (allVisibleSize)
				thumbPos = (((float)unitPos / (float)allVisibleSize) * thumbRange) + thickness;
			else
				thumbPos = 0;
			element = ELEMENT_BUTTON_TOP;
			//Refresh(false);
			SendEvent();
			arrowLoop.Start(500);
		}
		else{
			unitPos += (coord < thumbPos) ? -pageSize : pageSize;
			if (unitPos < 0){ unitPos = 0; }
			if (unitPos > allVisibleSize){ unitPos = allVisibleSize; }
			if (allVisibleSize)
				thumbPos = (((float)unitPos / (float)allVisibleSize) * thumbRange) + thickness;
			else
				thumbPos = 0;

			element = ELEMENT_BETWEEN_THUMB;
			diff = coord;
			SendEvent();
			pageLoop.Start(500);
			return;
		}
		diff = coord - thumbPos;
		if (!HasCapture()){ CaptureMouse(); }
	}

	if (holding && element & ELEMENT_THUMB){
		thumbPos = coord - diff;
		thumbPos = MID(thickness, thumbPos, thumbRange + thickness);
		unitPos = ((thumbPos - thickness) / (float)thumbRange) * allVisibleSize;
		SendEvent();
	}
}

BEGIN_EVENT_TABLE(KaiScrollbar, wxWindow)
EVT_MOUSE_EVENTS(KaiScrollbar::OnMouseEvent)
END_EVENT_TABLE();

KaiScrolledWindow::KaiScrolledWindow(wxWindow *parent, int id, const wxPoint& pos,
const wxSize& size, long style, const wxString& name)
: wxWindow(parent, id, pos, size, 0, name)
, vertical(NULL)
, horizontal(NULL)
{
	if (style & wxHORIZONTAL){
		style ^= wxHORIZONTAL;
	}
	if (style & wxVERTICAL){
		style ^= wxVERTICAL;
	}
	SetWindowStyle(style);
}

void KaiScrolledWindow::Refresh(bool eraseBackground, const wxRect *rect){
	wxSize sz = wxWindow::GetClientSize();

	wxRect rc(0, 0, (vertical) ? sz.x - vertical->GetThickness() : 
		sz.x, (horizontal) ? sz.y - horizontal->GetThickness() : sz.y);
	if (rect){
		rc.x = rect->x;
		rc.y = rect->y;
		rc.width = MIN(rect->width, rc.width);
		rc.height = MIN(rect->height, rc.height);
	}
	wxWindow::Refresh(eraseBackground, &rc);
}

bool KaiScrolledWindow::SetScrollBar(int orientation, int pos, int maxVisible, int allItems, int pageSize, bool refresh)
{
	if (orientation & wxHORIZONTAL){
		if (horizontal && maxVisible >= allItems){
			horizontal->Destroy();
			horizontal = NULL;
			if (vertical){ vertical->SetTwoscrolbars(false); }
			return true;
		}
		else if (!horizontal && maxVisible < allItems){
			wxSize size = wxWindow::GetClientSize();
			int thickness = KaiScrollbar::CalculateThickness(this);
			horizontal = new KaiScrollbar(this, -1, wxPoint(0, size.y - thickness), 
				wxSize((vertical) ? size.x - thickness : size.x, thickness));
			horizontal->integrated = true;
			horizontal->SetCursor(wxCURSOR_ARROW);
			horizontal->SetScrollRate(20);
			if (vertical){
				vertical->SetTwoscrolbars(true);
			}
			horizontal->SetScrollbar(pos, maxVisible, allItems, pageSize, refresh);
			return true;
		}
		if (horizontal){
			wxSize size = wxWindow::GetClientSize();
			int thickness = horizontal->GetThickness();
			horizontal->SetPosition(wxPoint(0, size.y - thickness));
			horizontal->SetSize(wxSize((vertical && !horizontal->twoScrollbars) ? size.x - thickness : size.x, thickness));
			horizontal->SetScrollbar(pos, maxVisible, allItems, pageSize, refresh);
		}
	}
	if (orientation & wxVERTICAL){
		if (vertical && maxVisible >= allItems){
			vertical->Destroy();
			vertical = NULL;
			if (horizontal){ horizontal->SetTwoscrolbars(false); }
			return true;
		}
		else if (!vertical && maxVisible < allItems){
			wxSize size = wxWindow::GetClientSize();
			int thickness = KaiScrollbar::CalculateThickness(this);
			vertical = new KaiScrollbar(this, -1, wxPoint(size.x - thickness, 0), wxSize(thickness, (horizontal) ? size.y - thickness : size.y), wxVERTICAL);
			vertical->integrated = true;
			vertical->SetCursor(wxCURSOR_ARROW);
			if (horizontal){
				horizontal->SetTwoscrolbars(true);
			}
			vertical->SetScrollbar(pos, maxVisible, allItems, pageSize, refresh);
			return true;
		}
		if (vertical){
			wxSize size = wxWindow::GetClientSize();
			int thickness = vertical->GetThickness();
			vertical->SetPosition(wxPoint(size.x - thickness, 0));
			vertical->SetSize(wxSize(thickness, (horizontal && !vertical->twoScrollbars) ? size.y - thickness : size.y));
			vertical->SetScrollbar(pos, maxVisible, allItems, pageSize, refresh);
		}
	}
	return false;
}

int KaiScrolledWindow::SetScrollpos(int orientation, int pos, bool refresh)
{
	if (orientation & wxHORIZONTAL && horizontal){
		return horizontal->SetScrollPos(pos);
	}
	if (orientation & wxVERTICAL && vertical){
		return vertical->SetScrollPos(pos);
	}
	return -1;
}

bool KaiScrolledWindow::ScrollLines(int lines)
{
	if (horizontal){
		horizontal->unitPos += lines;
		horizontal->unitPos = MID(0, horizontal->unitPos, horizontal->allVisibleSize);
		Refresh(false);
	}
	if (vertical){
		vertical->unitPos += lines;
		vertical->unitPos = MID(0, vertical->unitPos, vertical->allVisibleSize);
		Refresh(false);
	}
	return true;
}

bool KaiScrolledWindow::ScrollPages(int pages)
{
	if (horizontal){
		horizontal->unitPos += horizontal->visibleSize * pages;
		horizontal->unitPos = MID(0, horizontal->unitPos, horizontal->allVisibleSize);
		Refresh(false);
	}
	if (vertical){
		vertical->unitPos += vertical->visibleSize * pages;
		vertical->unitPos = MID(0, vertical->unitPos, vertical->allVisibleSize);
		Refresh(false);
	}
	return true;
}

void KaiScrolledWindow::GetSize(int *x, int *y)
{
	wxWindow::GetSize(x, y);
	if (horizontal && horizontal->IsShown()){ *y -= horizontal->GetThickness(); }
	if (vertical && vertical->IsShown()){
		*x -= vertical->GetThickness();
	}
}

void KaiScrolledWindow::GetClientSize(int *x, int *y)
{
	wxWindow::GetClientSize(x, y);
	if (horizontal && horizontal->IsShown()){ *y -= horizontal->GetThickness(); }
	if (vertical && vertical->IsShown()){
		*x -= vertical->GetThickness();
	}
}

wxSize KaiScrolledWindow::GetSize()
{
	wxSize size = wxWindow::GetSize();
	if (horizontal && horizontal->IsShown()){ size.y -= horizontal->GetThickness(); }
	if (vertical && vertical->IsShown()){ size.x -= vertical->GetThickness(); }
	return size;
}

wxSize KaiScrolledWindow::GetClientSize()
{
	wxSize size = wxWindow::GetClientSize();
	if (horizontal && horizontal->IsShown()){ size.y -= horizontal->GetThickness(); }
	if (vertical && vertical->IsShown()){ size.x -= vertical->GetThickness(); }
	return size;
}