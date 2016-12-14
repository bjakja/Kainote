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
	(style & wxHORIZONTAL)? wxPoint(0, parent->GetClientSize().y-19) : wxPoint(parent->GetClientSize().x-19, 0), 
	(style & wxHORIZONTAL)? wxSize(parent->GetClientSize().x, 19) : wxSize(19, parent->GetClientSize().y)) 
	,isVertical((style & wxVERTICAL) != 0)
	,holding(false)
	,bmp(NULL)
{
	oldSize = GetClientSize();
	sendEvent.SetOwner(this, 2345);
	Bind(wxEVT_TIMER, [=](wxTimerEvent &evt){
		wxScrollEvent evt2(wxEVT_SCROLL_CHANGED, GetId()); 
		evt2.SetPosition(unitPos);
		AddPendingEvent(evt2);
	}, 2345);
	//Bind(wxEVT_LEFT_DOWN, &KaiScrollbar::OnMouseEvent, this);
	//Bind(wxEVT_LEFT_UP, &KaiScrollbar::OnMouseEvent, this);
	//Bind(wxEVT_MOTION, &KaiScrollbar::OnMouseEvent, this);
	Bind(wxEVT_MOUSE_CAPTURE_LOST, &KaiScrollbar::OnMouseLost, this);
	Bind(wxEVT_SIZE, &KaiScrollbar::OnSize, this);
	Bind(wxEVT_PAINT, &KaiScrollbar::OnPaint, this);
	Bind(wxEVT_ERASE_BACKGROUND, &KaiScrollbar::OnErase, this);
}


void KaiScrollbar::SetScrollbar(int pos, int visible, int range)
{
	if(holding){return;}
	unitPos = pos;
	visibleSize = visible;
	allSize = range;
	float divScroll = (float)visibleSize / (float)allSize;
	/*if(oldSize != GetParent()->GetClientSize()){
		oldSize = GetParent()->GetClientSize();
		SetPosition((!isVertical)? wxPoint(0, oldSize.y-19) : wxPoint(oldSize.x-19, 0));
		SetSize((!isVertical)? wxSize(oldSize.x, 19) : wxSize(19, oldSize.y));
	}*/
	thumbSize = (oldSize.y-38) * divScroll;
	if(thumbSize < 20){thumbSize=20;}
	thumbRange = (oldSize.y-38) - thumbSize;
	float uintToPixel = (float)thumbSize / (float)thumbRange;
	thumbPos = (unitPos * uintToPixel)+19;
	Refresh(false);
}


void KaiScrollbar::OnSize(wxSizeEvent& evt)
{
	oldSize = GetClientSize();
	float divScroll = (float)visibleSize / (float)allSize;
	thumbSize = (oldSize.y-38) * divScroll;
	if(thumbSize < 20){thumbSize=20;}
	thumbRange = (oldSize.y-38) - thumbSize;
	float uintToPixel = (float)thumbSize / (float)thumbRange;
	thumbPos = (unitPos * uintToPixel)+19;
	Refresh(false);
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
	wxColour highlight = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT);
	wxColour text = wxSystemSettings::GetColour(wxSYS_COLOUR_MENUTEXT);
	wxColour graytext = wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT);
	wxColour background = wxSystemSettings::GetColour(wxSYS_COLOUR_MENUBAR);
	tdc.SetPen(wxPen(graytext));
	tdc.SetBrush(wxBrush(graytext));
	tdc.DrawRectangle(0, 0, w, h);
	wxBitmap scrollArrow = wxBITMAP_PNG("arrow_list");
	wxImage img = scrollArrow.ConvertToImage();
	img = img.Rotate180();
	tdc.DrawBitmap(wxBitmap(img), 4, 4);
	tdc.DrawBitmap(scrollArrow, 4, h-15);
	tdc.SetPen(wxPen(text));
	tdc.SetBrush(wxBrush(text));
	tdc.DrawRectangle(2, thumbPos, 15, thumbSize);

	wxPaintDC dc(this);
	dc.Blit(0,0,w,h,&tdc,0,0);
}

void KaiScrollbar::OnMouseEvent(wxMouseEvent &evt)
{
	int x = evt.GetX();
	int y = evt.GetY();
	int w=0;
	int h=0;
	GetClientSize (&w, &h);

	
	if(evt.LeftUp()){
		holding = false;
		if(HasCapture()){ReleaseMouse();}
	}
	
	if(evt.LeftDown()){
		//wxLogStatus("leftdown");
		
		if(y >= thumbPos && y <= thumbPos+thumbSize){
			element = ELEMENT_THUMB;
		}else{
			element = ELEMENT_BUTTON_BOTTOM|ELEMENT_BUTTON_TOP|ELEMENT_BETWEEN_THUMB;
		}
		diff = y - thumbPos;
		holding = true;
		if(!HasCapture()){CaptureMouse();}
	}
		
	if(holding){
		//float unitToPixel = (float)thumbRange / (float)allSize;
		float unitToPixel = (float)thumbSize / (float)thumbRange;
		//wxLogStatus("leftisdown");
		if(element & ELEMENT_THUMB){
			thumbPos = y - diff;
			//thumbPos=20; sendEvent.Start(1,true); return;}
			//if(thumbPos>thumbRange+20){thumbPos=thumbRange+20; sendEvent.Start(1,true); return;}
			thumbPos = MID(19, thumbPos, thumbRange+19);
			//wxLogStatus("thumbpos %i", thumbPos);
			//wxLogStatus("thumbpos %i %i", thumbPos, thumbRange+20);
			unitPos = (thumbPos-19) / unitToPixel;
		}else if(y >= h - 20 && y <= h){
			//wxLogStatus("top");
			if(!(element & ELEMENT_BUTTON_BOTTOM)){return;}
			//wxLogStatus("top1");
			unitPos+=1;
			int diff = allSize - visibleSize;
			if(unitPos> diff){unitPos = diff; Refresh(false); return;}
			thumbPos = (unitPos * unitToPixel)+20;
			element = ELEMENT_BUTTON_BOTTOM;
		}else if(y>=0 && y < 20){
			//wxLogStatus("bottom");
			if(!(element & ELEMENT_BUTTON_TOP)){return;}
			//wxLogStatus("bottom1");
			unitPos-=1;
			if(unitPos<0){unitPos = 0; Refresh(false); return;}
			thumbPos = (unitPos * unitToPixel)+20;
			element = ELEMENT_BUTTON_TOP;
		}else{
			//wxLogStatus("rest");
			if(!(element & ELEMENT_BETWEEN_THUMB)){return;}
			thumbPos = y-(thumbSize/2);
			//thumbPos=20; sendEvent.Start(1,true); return;}
			//if(thumbPos>thumbRange+20){thumbPos=thumbRange+20; sendEvent.Start(1,true); return;}
			thumbPos = MID(19, thumbPos, thumbRange+19);
			//wxLogStatus("thumbpos %i", thumbPos);
			//wxLogStatus("thumbpos %i %i", thumbPos, thumbRange+20);
			unitPos = (thumbPos-19) / unitToPixel;
			//wxLogStatus("rest1");
			element = ELEMENT_BETWEEN_THUMB;
			//return;
		}
		
		Refresh(false);
		sendEvent.Start(1,true);
	}
}

BEGIN_EVENT_TABLE(KaiScrollbar, wxWindow)
	EVT_MOUSE_EVENTS(KaiScrollbar::OnMouseEvent)
END_EVENT_TABLE()
