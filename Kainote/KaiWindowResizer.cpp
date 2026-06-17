//  Copyright (c) 2016 - 2026, Marcin Drob

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

#include "KaiWindowResizer.h"
#include <wx/dc.h>
#include <wx/dcclient.h>
#include <wx/dcmemory.h>
#include "config.h"

KaiWindowResizer::KaiWindowResizer(wxWindow *parent, std::function<bool(int)> _canResize, std::function<void(int, bool)> _doResize)
	:wxWindow(parent, -1, wxDefaultPosition, wxSize(-1, 8))
	, canResize(_canResize)
	, doResize(_doResize)
	, resizerParent(parent)
{
	SetMaxSize(wxSize(-1, 8));
	SetMinSize(wxSize(-1, 8));
	SetCursor(wxCURSOR_SIZENS);
	SetBackgroundColour(Options.GetColour(WINDOW_BACKGROUND));
	Bind(wxEVT_MOUSE_CAPTURE_LOST, [this](wxMouseCaptureLostEvent &evt){
		holding = false;
		if (splitLine){
			splitLine->Destroy();
			splitLine = nullptr;
		}
	});
	Bind(wxEVT_PAINT, &KaiWindowResizer::OnPaint, this);
	Bind(wxEVT_LEFT_DOWN, &KaiWindowResizer::OnMouseEvent, this);
	Bind(wxEVT_LEFT_UP, &KaiWindowResizer::OnMouseEvent, this);
	Bind(wxEVT_LEFT_DCLICK, &KaiWindowResizer::OnMouseEvent, this);
	Bind(wxEVT_MOTION, &KaiWindowResizer::OnMouseEvent, this);
	Bind(wxEVT_ERASE_BACKGROUND, [=](wxEraseEvent & evt){});
}

KaiWindowResizer::~KaiWindowResizer() 
{ 
	if (bmp) { delete bmp; } 
}

void KaiWindowResizer::OnMouseEvent(wxMouseEvent &evt)
{
	const bool click = evt.LeftDown();
	const bool leftUp = evt.LeftUp();
	int newPosition = evt.GetY();

	if (holding){
		wxPoint screenPosition = wxGetMousePosition();
		resizerParent->ScreenToClient(&screenPosition.x, &screenPosition.y);
		newPosition = screenPosition.y;
	}

	if (!holding && HasCapture()){
		ReleaseMouse();
	}

	if (leftUp && holding){
		holding = false;
		if (HasCapture()){
			ReleaseMouse();
		}
		if (splitLine){
			splitLine->Destroy();
			splitLine = nullptr;
		}
		if (canResize(newPosition)){
			doResize(newPosition, evt.ShiftDown());
		}
		return;
	}

	if (leftUp){
		return;
	}

	if (click && !holding){
		holding = true;
		CaptureMouse();
		oldy = newPosition;
#ifdef _WIN32
		int px = 2, py = evt.GetY();
		ClientToScreen(&px, &py);
		splitLine = new wxDialog(this, -1, emptyString, wxPoint(px, py), wxSize(GetSize().GetWidth(), 2), wxSTAY_ON_TOP | wxBORDER_NONE);
		splitLine->SetBackgroundColour(Options.GetColour(WINDOW_TEXT));
		splitLine->Show();
#endif
	}

	if (holding){
		const bool canBeResized = canResize(newPosition);
#ifdef _WIN32
		if (canBeResized && splitLine && newPosition != oldy){
			int px = 2, py = newPosition;
			resizerParent->ClientToScreen(&px, &py);
			splitLine->SetPosition(wxPoint(px, py));
		}
#else
		if (canBeResized && newPosition != oldy){
			doResize(newPosition, evt.ShiftDown());
		}
#endif
		oldy = newPosition;
	}
}

void KaiWindowResizer::OnPaint(wxPaintEvent& evt)
{
	const wxColour & pointColor = Options.GetColour(WINDOW_RESIZER_DOTS);
	const wxColour & backgroundColor = Options.GetColour(WINDOW_BACKGROUND);

	wxSize size = GetClientSize();
	if (size.x < 1 || size.y < 1){ return; }
	if (bmp && (bmp->GetWidth() < size.x || bmp->GetHeight() < size.y)) {
		delete bmp;
		bmp = nullptr;
	}
	if (!bmp) { bmp = new wxBitmap(size.x, size.y); }
	wxMemoryDC mdc(*bmp);
	mdc.SetBrush(backgroundColor);
	mdc.SetPen(backgroundColor);
	mdc.DrawRectangle(0, 0, size.x, size.y);
	mdc.SetPen(pointColor);

	int xpoint = 0;
	bool drawTwoPoints = true;

	while (xpoint <= size.x){
		if (drawTwoPoints){
			mdc.DrawPoint(xpoint, 0);
			mdc.DrawPoint(xpoint, size.y - 1);
			drawTwoPoints = false;
		}
		else{
			mdc.DrawPoint(xpoint, size.y / 2);
			drawTwoPoints = true;
		}
		xpoint += 2;
	}

	wxPaintDC dc(this);
	dc.Blit(0, 0, size.x, size.y, &mdc, 0, 0);
}
