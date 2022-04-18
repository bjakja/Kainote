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

#include "config.h"
#include "Visuals.h"
#include "KainoteFrame.h"
#include "RendererVideo.h"
#include "TabPanel.h"
#include "VideoCtrl.h"
#include "SubsGrid.h"
#include "EditBox.h"


Cross::Cross()
{

}

void Cross::OnMouseEvent(wxMouseEvent &event)
{
	if ((tab->video->IsFullScreen() && tab->video->GetFullScreenWindow() &&
		!tab->video->GetFullScreenWindow()->showToolbar->GetValue()) || event.RightUp() || tab->video->IsMenuShown()){
		if (cross){
			tab->video->SetCursor(wxCURSOR_ARROW);
			cross = false;
			tab->video->Render(false);
		}
		return;
	}
	
	int x = event.GetX();
	int y = event.GetY();

	if (event.Leaving()){
		if (cross){
			cross = false;
			RendererVideo *renderer = tab->video->GetRenderer();
			tab->video->SetCursor(wxCURSOR_ARROW); 
				
			if (tab->video->GetState() <= Paused && !renderer->m_BlockResize){ 
				tab->video->Render(false); 
			}
		}
		return;
	}

	if (event.Entering()){
		//tab->video->SetCursor(wxCURSOR_BLANK);
		//KaiLog(L"Cross blank");
		cross = true;
		int nx = 0, ny = 0;
		int w = 0, h = 0;
		int diffW = 1, diffH = 1;
		RendererVideo *renderer = tab->video->GetRenderer();
		if (renderer){
			diffX = renderer->m_BackBufferRect.left;
			diffY = renderer->m_BackBufferRect.top;
			w = renderer->m_BackBufferRect.right - diffX;
			h = renderer->m_BackBufferRect.bottom - diffY;
			if (diffX)
				diffW = 0;
			if (diffY)
				diffH = 0;
		}
		else{
			tab->video->GetClientSize(&w, &h);
			diffX = diffY = 0;
			h -= tab->video->GetPanelHeight();
		}
		tab->grid->GetASSRes(&nx, &ny);
		coeffX = (float)nx / (float)(w - diffW);
		coeffY = (float)ny / (float)(h - diffH);
	}
	float zx = (x / zoomScale.x) + zoomMove.x;
	float zy = (y / zoomScale.y) + zoomMove.y;
	int posx = (float)zx * coeffX;
	int posy = (float)zy * coeffY;
	coords = emptyString;
	coords << posx << L", " << posy;
	DrawLines(wxPoint(x, y));

	if (event.MiddleDown() || (event.LeftDown() && event.ControlDown())){
		Dialogue *aline = tab->edit->line;
		bool istl = (tab->grid->hasTLMode && aline->TextTl != emptyString);
		wxString ltext = (istl) ? aline->TextTl : aline->Text;
		wxRegEx posmov(L"\\\\(pos|move)([^\\\\}]+)", wxRE_ADVANCED);
		posmov.ReplaceAll(&ltext, emptyString);

		wxString postxt;
		float zx = (x / zoomScale.x) + zoomMove.x;
		float zy = (y / zoomScale.y) + zoomMove.y;
		float posx = (float)zx * coeffX;
		float posy = (float)zy * coeffY;
		postxt = L"\\pos(" + getfloat(posx) + L"," + getfloat(posy) + L")";
		if (ltext.StartsWith(L"{")){
			ltext.insert(1, postxt);
		}
		else{
			ltext = L"{" + postxt + L"}" + ltext;
		}
		if (istl){ aline->TextTl = ltext; }
		else{ aline->Text = ltext; }
		tab->grid->ChangeCell((istl) ? TXTTL : TXT, tab->grid->currentLine, aline);
		tab->grid->Refresh(false);
		tab->grid->SetModified(VISUAL_POSITION);
	}
}

void Cross::Draw(int time)
{
	if (cross && isOnVideo){
		HRESULT hr;
		if (font) {
			DRAWOUTTEXT(font, coords, crossRect, (crossRect.left < vectors[0].x) ? 10 : 8, 0xFFFFFFFF);
		}
		if (line) {
			hr = line->SetWidth(3);
			hr = line->Begin();
			hr = line->Draw(&vectors[0], 2, 0xFF000000);
			hr = line->Draw(&vectors[2], 2, 0xFF000000);
			hr = line->End();
			hr = line->SetWidth(1);
		}
		D3DXVECTOR2 v1[4];
		v1[0] = vectors[0];
		v1[0].x += 0.5f;
		v1[1] = vectors[1];
		v1[1].x += 0.5f;
		v1[2] = vectors[2];
		v1[2].y += 0.5f;
		v1[3] = vectors[3];
		v1[3].y += 0.5f;
		if (line) {
			hr = line->Begin();
			hr = line->Draw(&v1[0], 2, 0xFFFFFFFF);
			hr = line->Draw(&v1[2], 2, 0xFFFFFFFF);
			hr = line->End();
		}
	}
}

void Cross::DrawLines(wxPoint point)
{
	RendererVideo *renderer = tab->video->GetRenderer();
	if (!renderer)
		return;

	//without this mutex it crash maybe only slowing something else and blocking crash in that way
	wxMutexLocker lock(m_MutexCrossLines);
	
	if (point.y < renderer->m_BackBufferRect.top || point.x < renderer->m_BackBufferRect.left ||
		point.y > renderer->m_BackBufferRect.bottom || point.x > renderer->m_BackBufferRect.right) {
		isOnVideo = false;
		goto done;
	}
	else
		isOnVideo = true;

	{
		int w, h, fw, fh;
		tab->video->GetWindowSize(&w, &h);
		RECT rcRect = { 0, 0, 0, 0 };
		if (calcfont && calcfont->DrawTextW(nullptr, coords.wc_str(), -1, &rcRect, DT_CALCRECT, 0xFFFFFFFF)) {
			fw = rcRect.right - rcRect.left;
			fh = rcRect.bottom - rcRect.top;
		}
		else {
			tab->video->GetTextExtent(coords, &fw, &fh, nullptr, nullptr, Options.GetFont(4));
		}
		int margin = fh * 0.25f;
		w /= 2; h /= 2;
		crossRect.top = (h > point.y) ? point.y - (margin * 2) - 2 : point.y - (margin * 2) - 2 - fh;
		crossRect.bottom = (h > point.y) ? point.y + fh : point.y - margin;
		crossRect.left = (w < point.x) ? point.x - fw - margin : point.x + margin;
		crossRect.right = (w < point.x) ? point.x - margin : point.x + fw + margin;

		vectors[0].x = point.x;
		vectors[0].y = renderer->m_BackBufferRect.top;
		vectors[1].x = point.x;
		vectors[1].y = renderer->m_BackBufferRect.bottom;
		vectors[2].x = renderer->m_BackBufferRect.left;
		vectors[2].y = point.y;
		vectors[3].x = renderer->m_BackBufferRect.right;
		vectors[3].y = point.y;
		cross = true;
	}
	done:
	//play and pause
	if (tab->video->GetState() <= Paused && !renderer->m_BlockResize){
		tab->video->Render(renderer->m_VideoResized);
	}
}

void Cross::SetCurVisual()
{
	if ((tab->video->IsFullScreen() && tab->video->GetFullScreenWindow() && 
		!tab->video->GetFullScreenWindow()->showToolbar->GetValue()) || tab->video->IsMenuShown()){
		if (cross){
			tab->video->SetCursor(wxCURSOR_ARROW);
			cross = false;
			tab->video->Render(false);
		}
		return;
	}
	else {
		cross = false;
	}

	int nx = 0, ny = 0;
	int w = 0, h = 0;
	int diffW = 1, diffH = 1;
	RendererVideo *renderer = tab->video->GetRenderer();
	
	if (renderer){
		
		diffX = renderer->m_BackBufferRect.left;
		diffY = renderer->m_BackBufferRect.top;
		w = renderer->m_BackBufferRect.right - diffX;
		h = renderer->m_BackBufferRect.bottom - diffY;
		if (diffX)
			diffW = 0;
		if (diffY)
			diffH = 0;
	}
	else{
		tab->video->GetClientSize(&w, &h);
		h -= tab->video->GetPanelHeight();
		diffX = diffY = 0;
	}
	tab->grid->GetASSRes(&nx, &ny);
	coeffX = (float)nx / (float)(w - diffW);
	coeffY = (float)ny / (float)(h - diffH);
}

void Cross::SizeChanged(wxRect wsize, LPD3DXLINE _line, LPD3DXFONT _font, LPDIRECT3DDEVICE9 _device)
{
	Visuals::SizeChanged(wsize, _line, _font, _device);
	wxFont* font12 = Options.GetFont(4);
	wxSize pixelSize = font12->GetPixelSize();
	if (pixelSize.x == 0 || pixelSize.y == 0) { return; }
	HRN(D3DXCreateFontW(device, pixelSize.y, 0, FW_BOLD, 0, FALSE, 
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE, L"Tahoma", &calcfont), 
		_("Nie można stworzyć czcionki D3DX"));
	
}
