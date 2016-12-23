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

#include "KaiScrollbar.h"
#include "config.h"

KaiScrollbar::KaiScrollbar(wxWindow *parent, int id, const wxPoint &pos, const wxSize &size, int style)
	: wxWindow(parent, id,
	(style & wxHORIZONTAL)? wxPoint(0, parent->GetClientSize().y-17) : wxPoint(parent->GetClientSize().x-17, 0), 
	(style & wxHORIZONTAL)? wxSize(parent->GetClientSize().x, 17) : wxSize(17, parent->GetClientSize().y)) 
	,isVertical((style & wxVERTICAL) != 0)
	,holding(false)
	,rholding(false)
	,bmp(NULL)
	,enter(false)
	,integrated(false)
	,unitPos(0)
	,element(0)
{
	pageLoop.SetOwner(this, 2345);
	Bind(wxEVT_TIMER, [=](wxTimerEvent &evt){
		if(unitPos == 0 || unitPos == allVisibleSize){pageLoop.Stop();return;}
		if(diff<thumbPos){unitPos -= pageSize;}
		else if(diff>thumbPos+thumbSize){unitPos += pageSize;}
		else{pageLoop.Stop();return;}
		if(unitPos<0){unitPos = 0;}
		if(unitPos> allVisibleSize){unitPos = allVisibleSize;}
		thumbPos = (((float)unitPos / (float)allVisibleSize) * thumbRange) +17;
		SendEvent();
	}, 2345);
	arrowLoop.SetOwner(this, 2346);
	Bind(wxEVT_TIMER, [=](wxTimerEvent &evt){
		//float unitToPixel = (float)thumbSize / (float)thumbRange;
		if(element & ELEMENT_BUTTON_BOTTOM && unitPos < allVisibleSize){
			unitPos+=1;
			if(unitPos> allVisibleSize){unitPos = allVisibleSize;}
			//thumbPos = (unitPos * unitToPixel)+17;
			thumbPos = (((float)unitPos / (float)allVisibleSize) * thumbRange) +17;
			SendEvent();
		}else if(element & ELEMENT_BUTTON_TOP && unitPos > 0){
			unitPos-=1;
			if(unitPos<0){unitPos = 0;}
			//thumbPos = (unitPos * unitToPixel)+17;
			thumbPos = (((float)unitPos / (float)allVisibleSize) * thumbRange) +17;
			SendEvent();
		}
		if(arrowLoop.GetInterval()==500){arrowLoop.Start(50);}
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
	if(holding /*|| (element & ELEMENT_BETWEEN_THUMB)*/ || (unitPos >= range-visible && pos >= unitPos)){return;}
	allVisibleSize = range-visible;
	unitPos = pos;
	visibleSize = visible;
	allSize = range;
	pageSize = _pageSize;
	float divScroll = (float)visibleSize / (float)allSize;
	if(pos >= allVisibleSize){unitPos=allVisibleSize;}
	int paneSize = (isVertical)? (oldSize.y-34) : (oldSize.x-34);
	thumbSize = paneSize * divScroll;
	if(thumbSize < 20){thumbSize=20;}
	thumbRange = paneSize - thumbSize;
	thumbPos = (((float)unitPos / (float)allVisibleSize) * thumbRange) +17;
	Refresh(false);
	Update();
}

void KaiScrollbar::SendEvent()
{
	Refresh(false);
	Update ();
	if(integrated){
		wxScrollWinEvent evt2(wxEVT_SCROLLWIN_THUMBTRACK,unitPos,(isVertical)? wxVERTICAL : wxHORIZONTAL); 
		((KaiScrolledWindow*)GetParent())->ProcessEvent(evt2);
	}else{
		wxScrollEvent evt2(wxEVT_SCROLL_THUMBTRACK, GetId()); 
		evt2.SetPosition(unitPos);
		AddPendingEvent(evt2);
	}
}

void KaiScrollbar::OnSize(wxSizeEvent& evt)
{
	wxSize oldSize = GetClientSize();
	int paneSize = (isVertical)? (oldSize.y-34) : (oldSize.x-34);
	float divScroll = (float)visibleSize / (float)allSize;
	thumbSize = paneSize * divScroll;
	if(thumbSize < 20){thumbSize=20;}
	thumbRange = paneSize - thumbSize;
	thumbPos = (((float)unitPos / (float)allVisibleSize) * thumbRange) +17;
	Refresh(false);
	Update();
}

void KaiScrollbar::OnPaint(wxPaintEvent& evt)
{
	int w=0;
	int h=0;
	GetClientSize (&w, &h);
	if(w==0||h==0){return;}
	wxMemoryDC tdc;
	if (bmp && (bmp->GetWidth() < w || bmp->GetHeight() < h)) {
		delete bmp;
		bmp = NULL;
	}
	if(!bmp){bmp=new wxBitmap(w, h);}
	tdc.SelectObject(*bmp);
	wxColour background = Options.GetColour("Scrollbar Background");
	bool pushed = holding && (element & ELEMENT_THUMB);
	wxColour scroll = (enter && !pushed)? Options.GetColour("Scrollbar Scroll Hover") : 
		(pushed)? Options.GetColour("Scrollbar Scroll Pushed") :
		Options.GetColour("Scrollbar Scroll");
	tdc.SetPen(wxPen(background));
	tdc.SetBrush(wxBrush(background));
	tdc.DrawRectangle(0, 0, w, h);
	wxBitmap scrollArrow = wxBITMAP_PNG("arrow_list");
	wxBitmap scrollArrowPushed = wxBITMAP_PNG("arrow_list_pushed");
	if(isVertical){
		wxImage img = (holding && element & ELEMENT_BUTTON_TOP)? scrollArrowPushed.ConvertToImage() : scrollArrow.ConvertToImage();
		img = img.Rotate180();
		tdc.DrawBitmap(wxBitmap(img), 3, 3);
		tdc.DrawBitmap((holding && element & ELEMENT_BUTTON_BOTTOM)? scrollArrowPushed : scrollArrow, 3, h-13);
		tdc.SetPen(wxPen(scroll));
		tdc.SetBrush(wxBrush(scroll));
		tdc.DrawRectangle(2, thumbPos, 13, thumbSize);
	}else{
		wxImage img = (holding && element & ELEMENT_BUTTON_TOP)? scrollArrowPushed.ConvertToImage() : scrollArrow.ConvertToImage();
		img = img.Rotate90();
		tdc.DrawBitmap(wxBitmap(img), 3, 3);
		img = (holding && element & ELEMENT_BUTTON_BOTTOM)? scrollArrowPushed.ConvertToImage() : scrollArrow.ConvertToImage();
		img = img.Rotate180().Rotate90();
		tdc.DrawBitmap(wxBitmap(img), w-13, 3);
		tdc.SetPen(wxPen(scroll));
		tdc.SetBrush(wxBrush(scroll));
		tdc.DrawRectangle(thumbPos, 2, thumbSize, 13);
	}
	wxPaintDC dc(this);
	dc.Blit(0,0,w,h,&tdc,0,0);
}

void KaiScrollbar::OnMouseEvent(wxMouseEvent &evt)
{
	if(evt.Leaving()){
		if(pageLoop.IsRunning()){
			pageLoop.Stop();
		}
		if(!holding){
			enter = false;
			Refresh(false);
			return;
		}
	}
	if (evt.GetWheelRotation() != 0) {
		int step = evt.GetWheelRotation() / evt.GetWheelDelta();
		unitPos -= step;
		if(unitPos<0){unitPos = 0; SendEvent();return;}
		if(unitPos> allVisibleSize){unitPos = allVisibleSize;  SendEvent(); return;}
		thumbPos = (((float)unitPos / (float)allVisibleSize) * thumbRange) +17;
		SendEvent();
		return;
	}
	int x = evt.GetX();
	int y = evt.GetY();
	int w=0;
	int h=0;
	GetClientSize (&w, &h);
	int coord = (isVertical)? y : x;
	int size = (isVertical)? h : w;
	if(evt.ButtonUp() && pageLoop.IsRunning()){
		pageLoop.Stop();
		if(HasCapture()){ReleaseMouse();}
		holding = false;
		Refresh(false);
	}
	if(evt.RightDown() || evt.RightIsDown() || (evt.ShiftDown() && (evt.LeftDown() || evt.LeftIsDown()))){
		thumbPos = coord-(thumbSize/2);
		thumbPos = MID(17, thumbPos, thumbRange+17);
		unitPos = ((thumbPos-17) / (float)thumbRange) * allVisibleSize;
		enter = true;
		holding = true;
		SendEvent();
		if(!HasCapture()){CaptureMouse();}
		return;
	}
	
	if(!evt.LeftDown() && !holding){
		if(!enter && coord >= thumbPos && coord <= thumbPos+thumbSize){
			enter = true;
			Refresh(false);
		}else if(enter && (coord < thumbPos || coord > thumbPos+thumbSize)){
			enter = false;
			Refresh(false);
		}
	}
	if(evt.LeftUp()){
		element = 0;
		holding = enter = false;
		Refresh(false);
		if(arrowLoop.IsRunning()){arrowLoop.Stop();}
		if(HasCapture()){ReleaseMouse();}
	}

	if(evt.LeftDown() || (!holding && evt.LeftIsDown())){
		holding = true;
		if(coord >= thumbPos && coord <= thumbPos+thumbSize){
			element = ELEMENT_THUMB;
		}else if(coord >= size - 20 && coord <= size && unitPos != allVisibleSize){
			unitPos+=1;
			if(unitPos> allVisibleSize){unitPos = allVisibleSize;}
			thumbPos = (((float)unitPos / (float)allVisibleSize) * thumbRange) +17;
			element = ELEMENT_BUTTON_BOTTOM;
			SendEvent();
			arrowLoop.Start(500);
		}else if(coord>=0 && coord < 20 && unitPos != 0){
			unitPos-=1;
			if(unitPos<0){unitPos = 0;}
			thumbPos = (((float)unitPos / (float)allVisibleSize) * thumbRange) +17;
			element = ELEMENT_BUTTON_TOP;
			//Refresh(false);
			SendEvent();
			arrowLoop.Start(500);
		}else/* if(unitPos != 0 && unitPos != allVisibleSize)*/{
			unitPos+= (coord<thumbPos)? -pageSize : pageSize;
			if(unitPos<0){unitPos = 0;}
			if(unitPos> allVisibleSize){unitPos = allVisibleSize;}
			thumbPos = (((float)unitPos / (float)allVisibleSize) * thumbRange) +17;

			element = ELEMENT_BETWEEN_THUMB;
			diff = coord;
			SendEvent();
			pageLoop.Start(100);
			return;
		}
		diff = coord - thumbPos;
		if(!HasCapture()){CaptureMouse();}
	}

	if(holding && element & ELEMENT_THUMB){
		//if(isVertical){
		thumbPos = coord - diff;
		thumbPos = MID(17, thumbPos, thumbRange+17);
		//unitPos = (thumbPos-17) / unitToPixel;
		unitPos = ((thumbPos-17) / (float)thumbRange) * allVisibleSize;
		SendEvent();
	}
}

BEGIN_EVENT_TABLE(KaiScrollbar,wxWindow)
	EVT_MOUSE_EVENTS(KaiScrollbar::OnMouseEvent)
END_EVENT_TABLE()

KaiScrolledWindow::KaiScrolledWindow(wxWindow *parent, int id, const wxPoint& pos, 
									 const wxSize& size, long style, const wxString& name)
									 : wxWindow(parent, id, pos, size, 0, name)
									 ,vertical(NULL)
									 ,horizontal(NULL)
{
	if(style & wxHORIZONTAL){
		style ^= wxHORIZONTAL;
		horizontal = new KaiScrollbar(this,-1);
		horizontal->integrated=true;
		horizontal->Hide();
	}
	if(style & wxVERTICAL){
		style ^= wxVERTICAL;
		vertical = new KaiScrollbar(this,-1, wxDefaultPosition, wxDefaultSize, wxVERTICAL);
		vertical->integrated=true;
		vertical->Hide();
	}
	SetWindowStyle(style);
}

bool KaiScrolledWindow::SetScrollBar(int orientation, int pos, int maxVisible, int allItems, bool refresh)
{
	bool changeVisibility = false;
	if(orientation & wxHORIZONTAL && horizontal){
		if(horizontal->IsShown() && maxVisible >= allItems){horizontal->Hide();changeVisibility=true;}
		else if(!horizontal->IsShown() && maxVisible < allItems){horizontal->Show();changeVisibility=true;}
		if(horizontal->IsShown()){
			wxSize size = wxWindow::GetClientSize();
			horizontal->SetPosition(wxPoint(0, size.y-17));
			horizontal->SetSize(wxSize(size.x, 17));
			horizontal->SetScrollbar(pos, maxVisible, allItems, maxVisible-1, refresh);
		}
	}
	if(orientation & wxVERTICAL && vertical){
		if(vertical->IsShown() && maxVisible >= allItems){vertical->Hide();changeVisibility=true;}
		else if(!vertical->IsShown() && maxVisible < allItems){vertical->Show();changeVisibility=true;}
		if(vertical->IsShown()){
			wxSize size = wxWindow::GetClientSize();
			vertical->SetPosition(wxPoint(size.x-17, 0));
			vertical->SetSize(wxSize(17, size.y));
			vertical->SetScrollbar(pos, maxVisible, allItems, maxVisible-1, refresh);
		}
	}
	return changeVisibility;
}

void KaiScrolledWindow::SetScrollPos (int orientation, int pos, bool refresh)
{
	if(orientation & wxHORIZONTAL && horizontal){
		horizontal->unitPos = pos;
		if(refresh){Refresh(false);}
	}
	if(orientation & wxVERTICAL && vertical){
		vertical->unitPos = pos;
		if(refresh){Refresh(false);}
	}
}

bool KaiScrolledWindow::ScrollLines (int lines)
{
	if(horizontal){
		horizontal->unitPos += lines;
		horizontal->unitPos = MID(0, horizontal->unitPos, horizontal->allVisibleSize);
		Refresh(false);
	}
	if(vertical){
		vertical->unitPos += lines;
		vertical->unitPos = MID(0, vertical->unitPos, vertical->allVisibleSize);
		Refresh(false);
	}
	return true;
}

bool KaiScrolledWindow::ScrollPages (int pages)
{
	if(horizontal){
		horizontal->unitPos += horizontal->visibleSize * pages;
		horizontal->unitPos = MID(0, horizontal->unitPos, horizontal->allVisibleSize);
		Refresh(false);
	}
	if(vertical){
		vertical->unitPos += vertical->visibleSize * pages;
		vertical->unitPos = MID(0, vertical->unitPos, vertical->allVisibleSize);
		Refresh(false);
	}
	return true;
}

void KaiScrolledWindow::GetSize(int *x, int *y)
{
	wxWindow::GetSize(x, y);
	if(horizontal && horizontal->IsShown()){*y -= 17;}
	if(vertical && vertical->IsShown()){
		*x -= 17;
	}
}

void KaiScrolledWindow::GetClientSize(int *x, int *y)
{
	wxWindow::GetClientSize(x, y);
	if(horizontal && horizontal->IsShown()){*y -= 17;}
	if(vertical && vertical->IsShown()){
		*x -= 17;
	}
}

wxSize KaiScrolledWindow::GetSize()
{
	wxSize size = wxWindow::GetSize();
	if(horizontal && horizontal->IsShown()){size.y -= 17;}
	if(vertical && vertical->IsShown()){size.x -= 17;}
	return size;
}

wxSize KaiScrolledWindow::GetClientSize()
{
	wxSize size = wxWindow::GetClientSize();
	if(horizontal && horizontal->IsShown()){size.y -= 17;}
	if(vertical && vertical->IsShown()){size.x -= 17;}
	return size;
}