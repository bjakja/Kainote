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

#include "KaiSlider.h"
#include "config.h"
#include <wx\bitmap.h>
#include <wx\slider.h>
#include <wx\dcmemory.h>
#include <wx\dcclient.h>
#include <wx\log.h>

KaiSlider::KaiSlider(wxWindow *parent, int id, int _value, int _minRange, int _maxRange,
             const wxPoint& pos, const wxSize& size, long _style)
			 : wxWindow(parent, id, pos, size)
			 ,style(_style)
			 ,minRange(_minRange)
			 ,maxRange(_maxRange)
			 ,thumbPos(0)
			 ,thumbSize(10)
			 ,bmp(NULL)
			 ,enter(false)
			 ,holding(false)
			 ,pushed(false)
			 ,isUpDirection(false)
{
	SetBackgroundColour(parent->GetBackgroundColour());
	if(style & wxVERTICAL){
		SetMinSize(wxSize((size.x<1)? 26 : size.x, (size.y<1)? 150 : size.y));
	}else{
		SetMinSize(wxSize((size.x<1)? 150 : size.x, (size.y<1)? 26 : size.y));
	}
	if(maxRange < minRange){maxRange = _minRange; minRange = _maxRange;}
	value = MID(0.f, ((float)(_value - minRange)/(float)(maxRange - minRange)), 1.f);
	if(style & wxSL_INVERSE){
		value = 1.f - value;
	}
	
	pageLoop.SetOwner(this, 2345);
	Bind(wxEVT_TIMER, [=](wxTimerEvent &evt){
		if(value == 0.f || value == 1.f){pageLoop.Stop();return;}
		//int thumbRange = size - thumbSize;
		float pageSize = 1.f / 10.f;
		if(diff<=thumbPos && isUpDirection){value -= pageSize;}
		else if(diff>=thumbPos+thumbSize && !isUpDirection){value += pageSize;}
		else{pageLoop.Stop();pushed = true;Refresh(false);return;}
		if(value < 0){value = 0.f;}
		if(value> 1){value = 1.f;}
		thumbPos = ((value / 1.f) * thumbRange);
		SendEvent();
		if(pageLoop.GetInterval()==500){pageLoop.Start(100);}
	}, 2345);

	//Bind(wxEVT_LEFT_DOWN, &KaiSlider::OnMouseEvent, this);
	//Bind(wxEVT_LEFT_UP, &KaiSlider::OnMouseEvent, this);
	////Bind(wxEVT_ENTER_WINDOW, &KaiCheckBox::OnMouseEvent, this);
	//Bind(wxEVT_LEAVE_WINDOW, &KaiSlider::OnMouseEvent, this);
	//Bind(wxEVT_SIZE, &KaiSlider::OnSize, this);
	//Bind(wxEVT_PAINT, &KaiSlider::OnPaint, this);
}
	
void KaiSlider::OnSize(wxSizeEvent& evt)
{
	int w=0;
	int h=0;
	GetClientSize (&w, &h);
	int size = ((style & wxVERTICAL)!=0)? h : w;
	thumbRange = size - thumbSize;
	thumbPos = ((value / 1.f) * thumbRange);
	Refresh(false);
}
	
void KaiSlider::OnPaint(wxPaintEvent& evt)
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
	if(!bmp){bmp=new wxBitmap(w,h);}
	tdc.SelectObject(*bmp);
	tdc.SetPen(*wxTRANSPARENT_PEN);
	tdc.SetBrush(GetBackgroundColour());
	tdc.DrawRectangle(0,0,w,h);
	bool enabled = IsThisEnabled();
	wxColour slider = (enter && !pushed)? Options.GetColour("Button Background Hover") :
		(pushed)? Options.GetColour("Button Background Pushed") : 
		(enabled)? Options.GetColour("Button Background") : 
		Options.GetColour("Window Inactive Background");
	wxColour sliderBorder = (enter && !pushed)? Options.GetColour("Button Border Hover") : 
		(pushed)? Options.GetColour("Button Border Pushed") : 
		(enabled)? Options.GetColour("Button Border") : 
		Options.GetColour("Button Inactive Border");
	tdc.SetPen(wxPen(Options.GetColour("Scrollbar Scroll Pushed")));
	tdc.SetBrush(wxBrush(Options.GetColour("Scrollbar Background")));
	if(style & wxVERTICAL){
		
		tdc.DrawRectangle((w/2)-2, thumbSize/2, 4, h - thumbSize);
		tdc.SetPen(wxPen(sliderBorder));
		tdc.SetBrush(wxBrush(slider));
		tdc.DrawRectangle(2, thumbPos+1, w-4, thumbSize-2);


	}else{
		tdc.DrawRectangle(thumbSize/2, (h/2)-2, w - thumbSize, 4);
		tdc.SetPen(wxPen(sliderBorder));
		tdc.SetBrush(wxBrush(slider));
		tdc.DrawRectangle(thumbPos+1,2 , thumbSize-1, h-4);


	}

	wxPaintDC dc(this);
	dc.Blit(0,0,w,h,&tdc,0,0);
}
	
void KaiSlider::OnMouseEvent(wxMouseEvent &evt)
{
	if(evt.Leaving()){
		if(pageLoop.IsRunning()){
			pageLoop.Stop();
		}
		if(!pushed){
			enter = false;
			Refresh(false);
			return;
		}
	}
	
	int x = evt.GetX();
	int y = evt.GetY();
	int w=0;
	int h=0;
	GetClientSize (&w, &h);
	bool isVertical = ((style & wxVERTICAL)!=0); 
	int coord = (isVertical)? y : x;
	int coord2 = (isVertical)? x : y;
	int size = (isVertical)? h : w;
	int size2 = (isVertical)? w : h;
	thumbRange = size - thumbSize;
	if (evt.GetWheelRotation() != 0) {
		int step = evt.GetWheelRotation() / evt.GetWheelDelta();
		value -= step;
		if(value< 0.f){value = 0.f; SendEvent();return;}
		if(value> 1.f){value = 0.f;  SendEvent(); return;}
		thumbPos = ((value / 1.f) * thumbRange);
		if(coord >= thumbPos && coord <= thumbPos+thumbSize){
			enter=true;
		}else if(enter){enter=false;}
		SendEvent();
		return;
	}
	if(evt.ButtonUp()){
		if(pageLoop.IsRunning()){pageLoop.Stop();}
		if(HasCapture()){ReleaseMouse();}
		enter = false;
		pushed = false;
		holding = false;
		Refresh(false);
		
		wxScrollEvent evt2(wxEVT_SCROLL_THUMBRELEASE, GetId()); 
		evt2.SetPosition(GetValue());
		AddPendingEvent(evt2);
	}
	if(evt.RightDown() || evt.RightIsDown() || //prawy przycisk
		(evt.ShiftDown() && (evt.LeftDown() || evt.LeftIsDown()))){ 
		thumbPos = coord-(thumbSize/2);
		thumbPos = MID(0, thumbPos, thumbRange);
		value = (thumbPos / (float)thumbRange) * 1.f;
		enter = true;
		holding =true;
		pushed = true;
		SendEvent();
		if(!HasCapture()){CaptureMouse();}
		return;
	}
	
	if(!holding){
		if(!enter && coord >= thumbPos && coord <= thumbPos+thumbSize && coord2 >= 0 && coord2 <= size2 ){
			enter = true;
			Refresh(false);
		}else if(enter && (coord < thumbPos || coord > thumbPos+thumbSize || coord2 < 0 || coord2 > size2)){
			enter = false;
			Refresh(false);
		}
	}
	
	if(evt.LeftDown() || (!holding && evt.LeftIsDown())){
		
		if(coord >= thumbPos && coord <= thumbPos+thumbSize){
			pushed = true;
			holding = true;
		}else{
			float pageSize = 1.f / 10.f;
			if(coord<thumbPos){isUpDirection=true;}else{isUpDirection=false;}
			value+= (coord<thumbPos)? -pageSize : pageSize;
			if(value < 0.f){value = 0.f;}
			if(value > 1.f){value = 1.f;}
			thumbPos = ((value / 1.f) * thumbRange);
			diff = coord;
			SendEvent();
			pageLoop.Start(500);
			return;
		}
		diff = coord - thumbPos;
		if(!HasCapture()){CaptureMouse();}
	}

	if(holding){
		thumbPos = coord - diff;
		thumbPos = MID(0, thumbPos, thumbRange);
		value = ((thumbPos) / (float)thumbRange) * 1.f;
		SendEvent();
	}
}

int KaiSlider::GetValue(){
	if(style & wxSL_INVERSE){
		float invValue = 1.f - value;
		return (invValue * (float)maxRange) + minRange;
	}else{
		return (value * (float)maxRange) + minRange;
	}
}

void KaiSlider::SetValue(int _value){
	value = MID(0.f, ((float)(_value - minRange)/(float)(maxRange - minRange)), 1.f);
	if(style & wxSL_INVERSE){
		value = 1.f - value;
	}
	thumbPos = ((value / 1.f) * thumbRange);
	Refresh(false);
	Update();
}

int KaiSlider::GetThumbPosition()
{
	return thumbPos;
}
	
void KaiSlider::SetThumbPosition(int position)
{
	thumbPos = position;
	value = (float)thumbPos/(float)thumbRange;
	Refresh(false);
	Update();
}

void KaiSlider::SendEvent()
{
	Refresh(false);
	Update();
	
	wxScrollEvent evt2(wxEVT_SCROLL_THUMBTRACK, GetId()); 
	evt2.SetPosition(GetValue());
	AddPendingEvent(evt2);
	
}

BEGIN_EVENT_TABLE(KaiSlider,wxWindow)
	EVT_MOUSE_EVENTS(KaiSlider::OnMouseEvent)
	EVT_PAINT(KaiSlider::OnPaint)
	EVT_SIZE(KaiSlider::OnSize)
	EVT_MOUSE_CAPTURE_LOST(KaiSlider::OnLostCapture)
END_EVENT_TABLE()