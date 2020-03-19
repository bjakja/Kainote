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

#include "KaiWindowResizer.h"
#include "config.h"
#include "wx/dcmemory.h"

KaiWindowResizer::KaiWindowResizer(wxWindow *parent, std::function<bool(int)> _canResize, std::function<void(int, bool)> _doResize)
	:wxWindow(parent, -1, wxDefaultPosition, wxSize(-1, 5))
	, canResize(_canResize)
	, doResize(_doResize)
	, resizerParent(parent)
{
	SetMaxSize(wxSize(-1, 5));
	SetMinSize(wxSize(-1, 5));
	SetCursor(wxCURSOR_SIZENS);
	SetBackgroundColour(Options.GetColour(WindowBackground));
	Bind(wxEVT_MOUSE_CAPTURE_LOST, [=](wxMouseCaptureLostEvent &evt){
		holding = false;
		if (splitLine){
			splitLine->Destroy();
			splitLine = NULL;
		}
	});
	Bind(wxEVT_PAINT, &KaiWindowResizer::OnPaint, this);
	Bind(wxEVT_LEFT_DOWN, &KaiWindowResizer::OnMouseEvent, this);
	Bind(wxEVT_LEFT_UP, &KaiWindowResizer::OnMouseEvent, this);
	Bind(wxEVT_LEFT_DCLICK, &KaiWindowResizer::OnMouseEvent, this);
	Bind(wxEVT_MOTION, &KaiWindowResizer::OnMouseEvent, this);
	Bind(wxEVT_ERASE_BACKGROUND, [=](wxEraseEvent & evt){});
}

void KaiWindowResizer::OnMouseEvent(wxMouseEvent &evt)
{
	bool click = evt.LeftDown();
	bool leftUp = evt.LeftUp();

	int newPosition = evt.GetY();

	if (!holding && HasCapture())
		ReleaseMouse();

	if (leftUp && holding) {
		holding = false;
		ReleaseMouse();
		if (splitLine){
			int x;
			splitLine->GetPosition(&x, &newPosition);
			resizerParent->ScreenToClient(&x, &newPosition);
			splitLine->Destroy();
			splitLine = NULL;
			doResize(newPosition, evt.ShiftDown());
		}

	}

	if (leftUp && !holding) {
		return;
	}

	if (click && !holding) {
		holding = true;
		CaptureMouse();
		int px = 2, py = newPosition;
		ClientToScreen(&px, &py);
		splitLine = new wxDialog(this, -1, L"", wxPoint(px, py), wxSize(GetSize().GetWidth(), 2), wxSTAY_ON_TOP | wxBORDER_NONE);
		splitLine->SetBackgroundColour(Options.GetColour(WindowText));
		splitLine->Show();
	}

	if (holding){
		int px = 2, py = newPosition;
		ClientToScreen(&px, &py);
		int screenX = px, screenY = py;
		resizerParent->ScreenToClient(&screenX, &screenY);
		bool canBeResized = canResize(screenY);
		if (canBeResized && newPosition != oldy){
			splitLine->SetPosition(wxPoint(px, py));
		}
		oldy = newPosition;
	}
}

void KaiWindowResizer::OnPaint(wxPaintEvent& evt)
{
	//for now line will only horizontal
	const wxColour & pointColor = Options.GetColour(WINDOW_RESIZER_DOTS);
	const wxColour & backgroundColor = Options.GetColour(WindowBackground);

	wxSize size = GetClientSize();
	wxMemoryDC mdc(wxBitmap(size.x, size.y));
	mdc.SetBrush(backgroundColor);
	mdc.SetPen(backgroundColor);
	mdc.DrawRectangle(0, 0, size.x, size.y);
	mdc.SetPen(pointColor);

	int xpoint = 0;
	bool drawTwoPoints = true;

	while (xpoint <= size.x){
		if (drawTwoPoints){
			mdc.DrawPoint(xpoint, 0);
			mdc.DrawPoint(xpoint, 4);
			drawTwoPoints = false;
		}
		else{
			mdc.DrawPoint(xpoint, 2);
			drawTwoPoints = true;
		}
		xpoint += 2;
	}

	wxClientDC dc(this);
	dc.Blit(0, 0, size.x, size.y, &mdc, 0, 0);
}
