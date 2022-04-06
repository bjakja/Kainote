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


#include "Videobox.h"
#include "SubsTime.h"
//#include <wx/msw/winundef.h>
#include <wx/dc.h>
#include <wx/dcmemory.h>
#include <wx/dcclient.h>
//#include "Config.h"
//#include "Utils.h"
#include "UtilsWindows.h"

VideoSlider::VideoSlider(wxWindow *parent, const long int id, const wxPoint& pos, const wxSize& size, long style, const wxString& name)
	: wxWindow(parent, id, pos, size, style, name)
{
	holding = block = false;
	position = 0;
	//bmp=0;
	prb = CreateBitmapFromPngResource(L"pbar");
	prbh = CreateBitmapFromPngResource(L"pbarhandle");
	showlabel = false;
	onslider = false;
}

VideoSlider::~VideoSlider()
{
	//wxDELETE(prb);
	//wxDELETE(prbh);
	//wxDELETE(bmp);
}

void VideoSlider::OnPaint(wxPaintEvent& event)
{
	int w = 0;
	int h = 0;
	GetClientSize(&w, &h);
	if (w == 0 || h == 0){ return; }
	wxMemoryDC tdc;
	tdc.SelectObject(wxBitmap(w, h));
	tdc.SetFont(*Options.GetFont());
	wxColour background = GetParent()->GetBackgroundColour();
	tdc.SetBrush(wxBrush(background));
	tdc.SetPen(wxPen(background));
	tdc.DrawRectangle(0, 0, w, h);
	int posY = ((h - 12) / 2) + 4;
	if (VB->GetState() != None){
		tdc.SetPen(wxPen(L"#2EA6E2"));
		int duration = VB->GetDuration();
		float factor = (float)(w - 30) / (float)duration;
		RendererVideo *renderer = VB->GetRenderer();
		if (renderer){
			isChapter = false;
			for (auto & ch : renderer->m_Chapters){
				int chpos = (ch.time * factor) + 15;
				if (abs(labelpos - chpos) < 3){
					isChapter = true;
					chapterTime = ch.time;
					chapterPos = chpos - 15;
					STime kkk;
					kkk.mstime = chapterTime;
					wxString time = kkk.raw(SRT);
					time.Prepend(ch.name + L" ");
					label = time;
					tdc.SetPen(wxPen(L"#2583C8"));
				}
				tdc.DrawLine(chpos, 0, chpos, posY);
			}
		}
	}
	wxBitmap start = prb.GetSubBitmap(wxRect(0, 0, 10, 5));
	tdc.DrawBitmap(start, 10, posY);
	wxBitmap px = prb.GetSubBitmap(wxRect(3, 0, 80, 5));
	
	int i = 20;
	for (; i < w - 90; i += 80){
		tdc.DrawBitmap(px, i, posY);
	}
	int diff = w - i - 10;
	wxBitmap end = prb.GetSubBitmap(wxRect(86 - diff, 0, diff, 5));
	tdc.DrawBitmap(end, w - diff - 10, posY);
	if (position > 5){
		tdc.SetPen(wxPen(L"#2583C8"));
		tdc.DrawLine(11, posY + 1, position + 8, posY + 1);
		tdc.DrawLine(11, posY + 3, position + 8, posY + 3);
		tdc.SetPen(wxPen(L"#2EA6E2"));
		tdc.DrawLine(11, posY + 2, position + 8, posY + 2);
	}
	tdc.DrawBitmap(prbh, position + 5, posY - 4);
	if (showlabel){
		int fw, fh;
		tdc.GetTextExtent(label, &fw, &fh);
		int posX = labelpos;
		if (posX < w / 2){ posX += 15; }
		else{ posX -= (fw + 15); }
		tdc.SetTextForeground(wxColour(L"#0C2B87"));
		tdc.DrawText(label, posX + 1, -1);
		tdc.DrawText(label, posX - 1, -1);
		tdc.DrawText(label, posX + 1, -3);
		tdc.DrawText(label, posX - 1, -3);
		tdc.SetTextForeground(wxColour(L"#FFFFFF"));
		tdc.DrawText(label, posX, -2);
	}
	wxPaintDC dc(this);
	dc.Blit(0, 0, w, h, &tdc, 0, 0);
}

void VideoSlider::SetValue(float pos)
{
	if (!block){
		int w = 0;
		int h = 0;
		GetClientSize(&w, &h);
		int calc = (w - 30);
		position = pos*calc;
		Refresh(false);
	}
}

void VideoSlider::OnMouseEvent(wxMouseEvent& event)
{
	int w = 0;
	int h = 0;
	GetClientSize(&w, &h);
	bool click = event.LeftDown();
	bool left_up = event.LeftUp();
	bool dclick = event.LeftDClick();
	int curY = (event.GetY());
	int curX = (event.GetX());
	float calc = (w - 30);

	

	if (VB->GetState() != None){

		bool isOnSlider = curX > position + 4 && curX < position + 25;
		bool isInSliderRange = curX > 4 && curX < w - 6;
		float dur = VB->GetDuration();
		msTimePosition = (MID(0, curX - 15, calc) / calc) * dur;
		
		if (left_up && holding) {
			holding = false;
			if (block){
				SendTime(msTimePosition);
				block = false;
			}
			ReleaseMouse();
		}
		//hover on thumb
		if (!onslider && isOnSlider){
			wxImage img = prbh.ConvertToImage();
			int size = prbh.GetWidth()*prbh.GetHeight() * 3;
			byte *data = img.GetData();
			for (int i = 0; i < size; i++)
			{
				if (data[i] < 226){ data[i] += 30; }
			}
			prbh = wxBitmap(img);
			onslider = true;
			//Refresh(false);
		}//move mouse of the thumb
		else if (onslider && (!isOnSlider || event.Leaving())){
			prbh = CreateBitmapFromPngResource(L"pbarhandle");
			onslider = false;
			//Refresh(false);
		}
		//move thumb dragging by mouse
		if (holding && isOnSlider || (holding && block)){
			block = true;
			position = MID(0, curX - 15 + positionDiff, w - 30);
			if (!VB->IsDirectShow()){ SendTime(msTimePosition); }
			//Refresh(false);
		}
		//move thumb by clicking on slider
		else if (click && isInSliderRange){
			if (!isOnSlider){
				block = true;
				if (isChapter){
					//float dur = VB->GetDuration();
					SendTime(chapterTime);
					position = chapterPos;
				}
				else{
					position = MID(0, curX - 15, w - 30);
					SendTime(msTimePosition);
				}

				Refresh(false);
				block = false;
				return;
			}
			else{
				positionDiff = position - (curX - 15);
			}
		}
		//hiding time label
		if ((!isInSliderRange || event.Leaving()) && !holding){
			showlabel = false;
			Refresh(false);
			return;
		}
		//showing time label
		else if (isInSliderRange){
			showlabel = true;
			labelpos = curX;

			STime kkk;
			kkk.mstime = msTimePosition;
			label = kkk.raw(SRT);
			Refresh(false);
		}

	}
	// Click type
	if (click) {
		holding = true;
		CaptureMouse();
	}

	if (left_up && !holding) {
		return;
	}

	// Get focus
	if (event.ButtonDown()) {
		SetFocus();
	}
	
	if (event.GetWheelRotation() != 0 && VB->HasFFMS2()) {
#include "UtilsWindows.h"
		
		if (eventCounter == newEventCounter) {
			int step = event.GetWheelRotation() / event.GetWheelDelta();
			if (step > 0) {
				VB->GoToNextKeyframe();
			}
			else {
				VB->GoToPrevKeyframe();
			}
			eventCounter++;
		}
		newEventCounter++;
		return;
	}
	
	if (event.GetWheelRotation() != 0){ event.Skip(); }


}

void VideoSlider::SendTime(int msTimePos)
{
	VB->Seek(msTimePos, true, true, true, false);
}
void VideoSlider::OnMouseLeave(wxMouseCaptureLostEvent& event)
{
	if (HasCapture()){
		ReleaseMouse();
	}
	holding = false;
	block = false;
	showlabel = false;
	if (onslider)
	{
		prbh = CreateBitmapFromPngResource(L"pbarhandle");
		onslider = false;
	}
	Refresh(false);
}
//void VideoSlider::OnKeyPress(wxKeyEvent& event)
//{
//	VB->GetEventHandler()->ProcessEvent(event);
//}

void VideoSlider::OnSize(wxSizeEvent& event)
{
	float dur = 0;
	float apos = 0;
	if (VB->GetState() != None){
		dur = VB->GetDuration();
		apos = VB->Tell();
	}
	SetValue((dur > 0) ? apos / dur : 0);
}



VolSlider::VolSlider(wxWindow *parent, const long int id, int apos, const wxPoint& pos, const wxSize& size, long style, const wxString& name)
	: wxWindow(parent, id, pos, size, style, name)
{
	holding = block = false;
	position = apos + 86;
	wxBitmap prb = CreateBitmapFromPngResource(L"pbar");
	start = wxBitmap(prb.GetSubBitmap(wxRect(0, 0, 80, 5)));
	end = wxBitmap(prb.GetSubBitmap(wxRect(76, 0, 10, 5)));
	prbh = CreateBitmapFromPngResource(L"pbarhandle");
	onslider = false;
	bmp = NULL;
}

VolSlider::~VolSlider()
{
	wxDELETE(bmp);
}

void VolSlider::OnPaint(wxPaintEvent& event)
{
	//if(blockpaint){return;}
	//blockpaint=true;
	int w = 0;
	int h = 0;
	GetClientSize(&w, &h);
	if (w == 0 || h == 0){ return; }
	wxMemoryDC tdc;
	if (bmp && (bmp->GetWidth() < w || bmp->GetHeight() < h)) {
		delete bmp;
		bmp = NULL;
	}
	if (!bmp){ bmp = new wxBitmap(w, h); }
	tdc.SelectObject(*bmp);
	wxColour background = GetParent()->GetBackgroundColour();
	tdc.SetBrush(wxBrush(background));
	tdc.SetPen(wxPen(background));
	tdc.DrawRectangle(0, 0, w, h);

	tdc.DrawBitmap(start, 10, 10);
	tdc.DrawBitmap(end, w - 20, 10);
	if (position > 5){
		tdc.SetPen(wxPen(L"#2583C8"));
		tdc.DrawLine(11, 11, position + 5, 11);
		tdc.DrawLine(11, 13, position + 5, 13);
		tdc.SetPen(wxPen(L"#2EA6E2"));
		tdc.DrawLine(11, 12, position + 5, 12);
	}
	tdc.DrawBitmap(prbh, position, 6);
	wxPaintDC dc(this);
	dc.Blit(0, 0, w, h, &tdc, 0, 0);
	//blockpaint=false;
}
//val from -86 to 0
void VolSlider::SetValue(int pos){
	if (!block){
		position = pos + 86;
		Refresh(false);
	}
}
//val from -86 to 0
int VolSlider::GetValue(){
	return (position - 86);
}

void VolSlider::OnMouseEvent(wxMouseEvent& event)
{
	int w = 0;
	int h = 0;
	GetClientSize(&w, &h);
	bool click = event.LeftDown();
	bool left_up = event.LeftUp();
	bool dclick = event.LeftDClick();
	int curY = (event.GetY());
	int curX = (event.GetX());

	// Get focus
	if (event.ButtonDown()) {
		SetFocus();
	}

	if (event.GetWheelRotation() != 0) {
		int step = event.GetWheelRotation() / event.GetWheelDelta();
		int pos = GetValue() + (step * 3);
		if (pos + 3 > 0){ pos = 0; }
		if (pos - 3 < (-86)){ pos = -86; }
		//wxLogMessage(wxString::Format("pos %i", pos));
		if (pos > 0 || pos < (-86)){ return; }
		wxScrollEvent evt(wxEVT_SCROLL_CHANGED, GetId());
		evt.SetPosition(pos);
		AddPendingEvent(evt);
		SetValue(pos);
		return;
	}

	if (left_up && holding) {
		holding = false;
		if (block){
			position = MID(0, curX - 10, w - 24);

			wxScrollEvent evt(wxEVT_SCROLL_CHANGED, GetId());
			evt.SetPosition(position - 86);
			AddPendingEvent(evt);
			block = false;
		}
		ReleaseMouse();
	}
	//hover on slider thumb
	if (!onslider && curX > position && curX < position + 20 && curY>5 && curY < 19){
		wxImage img = prbh.ConvertToImage();
		int size = prbh.GetWidth()*prbh.GetHeight() * 3;
		byte *data = img.GetData();
		for (int i = 0; i < size; i++)
		{
			if (data[i] < 226){ data[i] += 30; }
		}
		prbh = wxBitmap(img);
		onslider = true;
		Refresh(false);
	}//move mouse of the thumb
	else if (onslider && ((curX < position - 1 || curX > position + 19 || curY < 6 || curY > 18) || event.Leaving())){
		prbh = CreateBitmapFromPngResource(L"pbarhandle");
		onslider = false;
		Refresh(false);
	}
	//move mouse dragging of the thumb
	if (holding && curX > position && curX < position + 20 || (holding && block)){
		block = true; 
		position = MID(0, curX - 10, w - 24); 
		Refresh(false);
		wxScrollEvent evt(wxEVT_SCROLL_CHANGED, GetId());
		evt.SetPosition(position - 86);
		AddPendingEvent(evt);
	}
	//move thumb by click on slider
	else if (click && curX > 4 && curX < w - 6 && curY > h - 22 && curY < h - 2 && (curX < position || curX > position + 20)){
		block = true; 
		position = MID(0, curX - 10, w - 24);
		wxScrollEvent evt(wxEVT_SCROLL_CHANGED, GetId());
		evt.SetPosition(position - 86);
		AddPendingEvent(evt);
		Refresh(false); 
		block = false; 
		return;
	}

	if (left_up && !holding) {
		return;
	}


	// Click type
	if (click) {
		holding = true;
		CaptureMouse();
	}




}




BEGIN_EVENT_TABLE(VolSlider, wxWindow)
EVT_PAINT(VolSlider::OnPaint)
EVT_MOUSE_EVENTS(VolSlider::OnMouseEvent)
EVT_ERASE_BACKGROUND(VolSlider::OnEraseBackground)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(VideoSlider, wxWindow)
EVT_SIZE(VideoSlider::OnSize)
EVT_PAINT(VideoSlider::OnPaint)
EVT_MOUSE_EVENTS(VideoSlider::OnMouseEvent)
EVT_MOUSE_CAPTURE_LOST(VideoSlider::OnMouseLeave)
//EVT_KEY_DOWN(VideoSlider::OnKeyPress)
EVT_ERASE_BACKGROUND(VideoSlider::OnEraseBackground)
END_EVENT_TABLE()