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

#include "KaiStaticText.h"
#include <wx/dcmemory.h>
#include <wx/dcclient.h>
#include <wx/sizer.h>
//#include "GraphicsD2D.h"

KaiStaticText::KaiStaticText(wxWindow *parent, int id, const wxString& _text, const wxPoint &pos, const wxSize &size, int style)
	: wxWindow(parent, id, pos, size, style)
	, text(_text)
	, textColour(WINDOW_TEXT)
	, textHeight(0)
	, textScroll(nullptr)
	, scPos(0)
{
	int fullw = size.x;
	int windowHeight = size.y;
	wxSize newSize = originalSize = size;
	CalculateSize(&fullw, &windowHeight);

	if (size.x < 1){
		newSize.x = fullw;
	}
	if (size.y < 1){
		newSize.y = windowHeight + 2;
		if (windowHeight < 17){ newSize.y = 17; }
	}
	SetMinSize(newSize);
	Bind(wxEVT_ERASE_BACKGROUND, [=](wxEraseEvent &evt){});
	//Bind(wxEVT_LEFT_DOWN, [=](wxMouseEvent &evt){});
	Bind(wxEVT_PAINT, &KaiStaticText::OnPaint, this);
}

void KaiStaticText::CalculateSize(int *w, int *h)
{
	int fullw = 0;
	textHeight = 0;
	int textLineHeight = 0;
	if (text.empty()) {
		wxFont KaiStaticTextFont = GetFont();
		GetTextExtent(L"|", &fullw, &textHeight, 0, 0, &KaiStaticTextFont);
		textLineHeight = textHeight;
	}
	else{
		int fw, fh;
		int newWrap = -1;
		//int allwrap = -1;
		int currentPosition = 0;
		bool seekSpace = true;
		int mesureSize = (*w > 10) ? *w : 1000;
		size_t i = 0;
		//size_t len = text.length();
		while (i < text.length()){
			size_t nfound = text.find(wxUniChar(L'\n'), i);
			i = (nfound != -1) ? nfound : text.length() - 1;
			wxString stringToMesure = text.Mid(currentPosition, i - currentPosition + 1);
			GetTextExtent((stringToMesure.length()) ? stringToMesure : wxString(L"|"), &fw, &fh);
			if (!textLineHeight)
				textLineHeight = fh;
			if (fw > mesureSize){
				size_t j = currentPosition + 1;
				bool foundWrap = false;
				//fullw = mesureSize;
				size_t textPosition = currentPosition;
				int currentFW = 0;
				while (currentPosition < i)
				{
					size_t spacePos = text.find(wxUniChar(L' '), j);
					if (spacePos == -1 || i < spacePos)
						spacePos = i;

					j = spacePos + 1;
					GetTextExtent(text.Mid(textPosition, spacePos - textPosition + 1), &fw, &fh);
					textPosition = spacePos + 1;
					currentFW += fw;
					if (currentFW <= mesureSize){
						newWrap = j;
						foundWrap = true;
						if (j<i)
							continue;
					}
					else if (currentFW > mesureSize && !foundWrap){
						j = (currentPosition + 30 < i) ? currentPosition + 30 : currentPosition + 1;
						fw = 0;
						while (fw <= mesureSize && j < text.length()){
							GetTextExtent(text.Mid(currentPosition, j - currentPosition + 1), &fw, &fh);
							j++;
						}
						j--;
						newWrap = j;
					}
					if (newWrap > i){
						newWrap = i;
					}
					GetTextExtent(text.Mid(currentPosition, newWrap - currentPosition + 1), &fw, &fh);
					currentPosition = textPosition = newWrap;
					currentFW = 0;
					if (newWrap < i){
						text.insert(newWrap, 1, L'\n');
						currentPosition++;
						i++;
					}
					j = currentPosition + 1;
					foundWrap = false;
					if (fullw < fw){
						fullw = fw;
					}
					textHeight += fh;
				}

			}
			else{
				if (fullw < fw){
					fullw = fw;
				}
				textHeight += fh;
				currentPosition = i + 1;
			}
			i++;
		}

	}
	int heightMesure = (*h > 0) ? *h : 600;
	*h = textHeight;
	if (textHeight > heightMesure){ *h = heightMesure; fullw += 20; }
	if (textLineHeight < textHeight && fullw < 200) {
		fullw = 200;
	}
	*w = fullw;
}

void KaiStaticText::SetLabelText(const wxString &_text){
	text = _text;
	int fullw = originalSize.x;
	int windowHeight = originalSize.y;
	wxSize size = GetClientSize();
	CalculateSize(&fullw, &windowHeight);
	if (size.x != fullw || size.y != windowHeight){
		size.x = fullw;
		size.y = windowHeight;
		SetMinSize(size);
		//SetSize(size);
		//SetMaxSize(size);
		wxSizer *sizer = GetSizer();
		if (sizer){
			sizer->Layout();
		}
		//GetParent()->Layout();
		Refresh(false);
		Update();
		return;
	}

	Refresh(false);
}

void KaiStaticText::OnPaint(wxPaintEvent &evt)
{
	int w = 0;
	int h = 0;
	GetClientSize(&w, &h);
	if (w == 0 || h == 0){ return; }
	if (textHeight > 400){
		if (!textScroll){
			textScroll = new KaiScrollbar(this, 9999, wxDefaultPosition, wxDefaultSize, wxVERTICAL);
			textScroll->SetScrollRate(10);
			int sw = 0, sh = 0;
			textScroll->GetSize(&sw, &sh);
			textScroll->SetSize(w - sw, 0, sw, h);
			w -= sw;
		}
		int pageSize = h;
		if (scPos<0){ scPos = 0; }
		else if (scPos > (textHeight + 20) - pageSize){ scPos = (textHeight + 20) - pageSize; }
		textScroll->SetScrollbar(scPos, pageSize, textHeight + 20, pageSize - 1);
	}
	else if (textScroll){
		textScroll->Destroy();
		textScroll = nullptr;
	}


	wxMemoryDC tdc;
	wxBitmap KaiStaticTextBitmap(w, h);
	tdc.SelectObject(KaiStaticTextBitmap);
	/*GraphicsRenderer *renderer = GraphicsRenderer::GetDirect2DRenderer();
	GraphicsContext *gc = renderer->CreateContext(tdc);
	if (!gc){*/
	tdc.SetFont(GetFont());
	tdc.SetBrush(Options.GetColour(WINDOW_BACKGROUND));
	tdc.SetPen(*wxTRANSPARENT_PEN);
	tdc.DrawRectangle(0, 0, w, h);
	tdc.SetTextForeground(Options.GetColour(textColour));
	int center = (textHeight < h) ? (h - textHeight) / 2 : 0;
	tdc.DrawText(text, 0, -scPos + center);
	//}
	//	else{
	//		gc->SetFont(GetFont(), Options.GetColour(textColour));
	//		gc->SetBrush(Options.GetColour(WINDOW_BACKGROUND));
	//		gc->SetPen(*wxTRANSPARENT_PEN);
	//		gc->DrawRectangle(0, 0, w, h);
	//		int center = (textHeight < h) ? (h - textHeight) / 2 : 0;
	//		gc->DrawTextU(text, 0, -scPos + center);
	//		delete gc;
	//	}
	wxPaintDC dc(this);
	dc.Blit(0, 0, w, h, &tdc, 0, 0);
}

void KaiStaticText::OnScroll(wxScrollEvent& event)
{
	int newPos = event.GetPosition();

	if (scPos != newPos) {
		scPos = newPos;
		Refresh(false);
	}
}

void KaiStaticText::OnMouseScroll(wxMouseEvent &evt)
{
	if (evt.GetWheelRotation() != 0 && textScroll) {
		int step = 10 * evt.GetWheelRotation() / evt.GetWheelDelta();
		scPos -= step;
		Refresh(false);
		return;
	}
	evt.Skip();
}

bool KaiStaticText::SetFont(const wxFont &font)
{
	wxWindow::SetFont(font);
	int fullw, windowHeight;
	CalculateSize(&fullw, &windowHeight);
	if (windowHeight < 17){ windowHeight = 17; }
	SetMinSize(wxSize(fullw, (windowHeight > 400)? 400 : windowHeight));
	return true;
}

void KaiStaticText::OnSize(wxSizeEvent& event)
{
	/*if (lastSize != event.GetSize()){
		int fullw = originalSize.x;
		int windowHeight = originalSize.y;
		CalculateSize(&fullw, &windowHeight);

		Refresh(false);
		lastSize = event.GetSize();
		}*/
	Refresh(false);
}

BEGIN_EVENT_TABLE(KaiStaticText, wxWindow)
EVT_COMMAND_SCROLL(9999, KaiStaticText::OnScroll)
EVT_MOUSEWHEEL(KaiStaticText::OnMouseScroll)
EVT_SIZE(KaiStaticText::OnSize)
END_EVENT_TABLE()

