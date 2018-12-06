//  Copyright (c) 2018, Marcin Drob

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


#include "VideoPlayer.h"
#include "Videobox.h"
#include "CsriMod.h"
#include "DshowRenderer.h"
#include "kainoteMain.h"

bool VideoPlayer::PlayLine(int start, int eend)
{
	int duration = GetDuration();
	if (vstate == None || start >= eend || start >= duration){ return false; }
	if (duration < eend){ eend = duration; }
	SetPosition(start);
	Play(eend);
	return true;
}

void VideoPlayer::DrawLines(wxPoint point)
{
	wxMutexLocker lock(mutexLines);
	int w, h;
	videoWindow->GetClientSize(&w, &h);
	w /= 2; h /= 2;
	crossRect.top = (h > point.y) ? point.y - 12 : point.y - 40;
	crossRect.bottom = (h > point.y) ? point.y + 23 : point.y - 5;
	crossRect.left = (w < point.x) ? point.x - 100 : point.x + 5;
	crossRect.right = (w < point.x) ? point.x - 5 : point.x + 100;

	vectors[0].x = point.x;
	vectors[0].y = 0;
	vectors[1].x = point.x;
	vectors[1].y = backBufferRect.bottom;
	vectors[2].x = 0;
	vectors[2].y = point.y;
	vectors[3].x = backBufferRect.right;
	vectors[3].y = point.y;
	cross = true;
	if (vstate == Paused && !block){
		Render(resized);
	}
}

void VideoPlayer::DrawProgressBar()
{
	//progress text position
	wxMutexLocker lock(mutexProgBar);
	int w, h;
	videoWindow->TD->GetClientSize(&w, &h);
	progressBarRect.top = 16;
	progressBarRect.bottom = 60;
	progressBarRect.left = w - 167;
	progressBarRect.right = w - 3;
	//black frame coordinates
	vectors[4].x = w - 170;
	vectors[4].y = 5;
	vectors[5].x = w - 5;
	vectors[5].y = 5;
	vectors[6].x = w - 5;
	vectors[6].y = 15;
	vectors[7].x = w - 170;
	vectors[7].y = 15;
	vectors[8].x = w - 170;
	vectors[8].y = 5;
	//white frame coordinates
	vectors[9].x = w - 169;
	vectors[9].y = 6;
	vectors[10].x = w - 6;
	vectors[10].y = 6;
	vectors[11].x = w - 6;
	vectors[11].y = 14;
	vectors[12].x = w - 169;
	vectors[12].y = 14;
	vectors[13].x = w - 169;
	vectors[13].y = 6;
	//progress bar coordinates
	int rw = w - 168;
	vectors[14].x = rw;
	vectors[14].y = 10.5;
	vectors[15].x = (GetDuration() > 0) ? (((float)time / (float)GetDuration()) * 161) + rw : 161 + rw;
	vectors[15].y = 10.5;
}

bool VideoPlayer::DrawTexture(byte *nframe, bool copy)
{

	wxCriticalSectionLocker lock(mutexRender);
	byte *fdata = NULL;
	byte *texbuf;
	byte bytes = (vformat == RGB32) ? 4 : (vformat == YUY2) ? 2 : 1;
	//DWORD black = (vformat == RGB32) ? 0 : (vformat == YUY2) ? 0x80108010 : 0x10101010;
	//DWORD blackuv = (vformat == RGB32) ? 0 : (vformat == YUY2) ? 0x80108010 : 0x8080;

	D3DLOCKED_RECT d3dlr;

	if (nframe){
		fdata = nframe;
		if (copy){
			byte *cpy = (byte*)datas;
			memcpy(cpy, fdata, vheight * pitch);
		}
	}
	else{
		KaiLog(_("Brak bufora klatki")); return false;
	}


	if (instance){
		framee->strides[0] = vwidth * bytes;
		framee->planes[0] = fdata;
		csri_render(instance, framee, (time / 1000.0));
	}


#ifdef byvertices
	HR(MainStream->LockRect(&d3dlr, 0, 0), _("Nie mo¿na zablokowaæ bufora tekstury"));//D3DLOCK_NOSYSLOCK
#else
	HR(MainStream->LockRect(&d3dlr, 0, D3DLOCK_NOSYSLOCK), _("Nie mo¿na zablokowaæ bufora tekstury"));
#endif
	texbuf = static_cast<byte *>(d3dlr.pBits);

	int diff = d3dlr.Pitch - (vwidth*bytes);
	if (!diff){
		memcpy(texbuf, fdata, (vheight*pitch));
	}
	else if (diff > 0){

		if (vformat >= YV12){
			for (int i = 0; i < vheight; i++){
				memcpy(texbuf, fdata, vwidth);
				texbuf += vwidth;
				fdata += vwidth;
				//memset(texbuf-4, black, diff + 4);
				texbuf += diff;
			}
			int hheight = vheight / 2;
			int fwidth = (vformat == NV12) ? vwidth : vwidth / 2;
			int fdiff = (vformat == NV12) ? diff : diff / 2;

			for (int i = 0; i < hheight; i++){
				memcpy(texbuf, fdata, fwidth);
				texbuf += fwidth;
				fdata += fwidth;
				//memset(texbuf-2, blackuv, fdiff + 2);
				texbuf += fdiff;
			}
			if (vformat < NV12){
				for (int i = 0; i < hheight; i++){
					memcpy(texbuf, fdata, fwidth);
					texbuf += fwidth;
					fdata += fwidth;
					//memset(texbuf-2, blackuv, fdiff + 2);
					texbuf += fdiff;
				}
			}
		}
		else
		{
			int fwidth = vwidth * bytes;
			for (int i = 0; i < vheight; i++){
				memcpy(texbuf, fdata, fwidth);
				texbuf += (fwidth + diff);
				fdata += fwidth;
			}
		}

	}
	else{
		KaiLog(wxString::Format("bad pitch diff %i pitch %i dxpitch %i", diff, pitch, d3dlr.Pitch));
	}

	MainStream->UnlockRect();

	return true;
}

int VideoPlayer::GetCurrentPosition()
{
	return time;
}

int VideoPlayer::GetCurrentFrame()
{
	return numframe;
}

bool VideoPlayer::UpdateRects(bool changeZoom)
{
	wxRect rt;
	TabPanel* tab = (TabPanel*)videoWindow->GetParent();
	if (isFullscreen){
		hwnd = videoWindow->TD->GetHWND();
		rt = videoWindow->TD->GetClientRect();
		if (panelOnFullscreen){ rt.height -= panelHeight; }
		pbar = Options.GetBool(VideoProgressBar);
		cross = false;
	}
	else{
		hwnd = videoWindow->GetHWND();
		rt = videoWindow->GetClientRect();
		rt.height -= panelHeight;
		pbar = false;
	}
	if (!rt.height || !rt.width){ return false; }

	windowRect.bottom = rt.height;
	windowRect.right = rt.width;
	windowRect.left = rt.x;
	windowRect.top = rt.y;

	int arwidth = rt.height / AR;
	int arheight = rt.width * AR;

	if (arwidth > rt.width)
	{
		int onebar = (rt.height - arheight) / 2;
		//KaiLog(wxString::Format("onebar w %i, h %i, %i", onebar, rt.height, arheight));
		/*if(zoomParcent>1){
		int zoomARHeight = ((zoomRect.width - zoomRect.x)) * AR;
		onebar = (zoomRect.width - zoomRect.x > rt.width)? (rt.height - zoomARHeight)/2 : 0;
		wLogStatus("height %i %i %i, %i", zoomARHeight,arheight,rt.height,onebar);
		}*/
		backBufferRect.bottom = arheight + onebar;
		//if(backBufferRect.bottom % 2 != 0){backBufferRect.bottom++;}
		backBufferRect.right = rt.width;//zostaje bez zmian
		backBufferRect.left = 0;
		backBufferRect.top = onebar;
	}
	else if (arheight > rt.height)
	{
		int onebar = (rt.width - arwidth) / 2;
		//KaiLog(wxString::Format("onebar w %i, h %i, %i", onebar, rt.width, arwidth));
		/*if(zoomParcent>1){
		int zoomARWidth = ((zoomRect.height - zoomRect.y)) / AR;
		onebar = (zoomRect.height - zoomRect.y > rt.height)? (rt.width - zoomARWidth)/2 : 0;
		wLogStatus("width %i %i %i, %i", zoomARWidth,arwidth,rt.width,onebar);
		}*/
		backBufferRect.bottom = rt.height;//zostaje bez zmian
		backBufferRect.right = arwidth + onebar;
		//if(backBufferRect.right % 2 != 0){backBufferRect.right++;}
		backBufferRect.left = onebar;
		backBufferRect.top = 0;
	}
	else
	{
		//KaiLog(wxString::Format("equal %i %i", windowRect.right, windowRect.bottom));
		backBufferRect = windowRect;
	}
	//}
	if (changeZoom){
		wxSize s(backBufferRect.right, backBufferRect.bottom);
		float videoToScreenX = (float)s.x / (float)vwidth;
		float videoToScreenY = (float)s.y / (float)vheight;
		zoomRect.x = (mainStreamRect.left * videoToScreenX) + backBufferRect.left;
		zoomRect.y = (mainStreamRect.top * videoToScreenY) + backBufferRect.top;
		zoomRect.height = (mainStreamRect.bottom * videoToScreenY);
		zoomRect.width = (mainStreamRect.right * videoToScreenX);
		if (Visual){
			SetVisualZoom();
		}
	}
	return true;
}

void VideoPlayer::SetVisual(bool remove/*=false*/, bool settext/*=false*/, bool noRefresh /*= false*/)
{
	TabPanel* tab = (TabPanel*)videoWindow->GetParent();

	if (remove){
		SAFE_DELETE(Visual); tab->Edit->Visual = 0;
		hasVisualEdition = false;
		if (!noRefresh){
			OpenSubs(tab->Grid->GetVisible());
			Render();
		}
	}
	else{

		int vis = tab->Edit->Visual;
		if (!Visual){
			Visual = Visuals::Get(vis, videoWindow);
		}
		else if (Visual->Visual != vis){
			bool vectorclip = Visual->Visual == VECTORCLIP;
			delete Visual;
			Visual = Visuals::Get(vis, videoWindow);
			if (vectorclip && !settext){ OpenSubs(tab->Grid->GetVisible()); }
		}
		else{ SAFE_DELETE(Visual->dummytext); }
		if (settext){ OpenSubs(tab->Grid->GetVisible()); }
		Visual->SizeChanged(wxRect(backBufferRect.left, backBufferRect.top, backBufferRect.right, backBufferRect.bottom), lines, overlayFont, d3device);
		SetVisualZoom();
		Visual->SetVisual(tab->Edit->line->Start.mstime, tab->Edit->line->End.mstime, tab->Edit->line->IsComment, noRefresh);
		hasVisualEdition = true;
	}
}

void VideoPlayer::ResetVisual()
{
	SAFE_DELETE(Visual->dummytext);
	Visual->SetCurVisual();
	hasVisualEdition = true;
	Render();
}

void VideoPlayer::DrawZoom()
{
	D3DXVECTOR2 v2[5];
	wxSize s(backBufferRect.right, backBufferRect.bottom);
	v2[0].x = zoomRect.x;
	v2[0].y = zoomRect.y;
	v2[1].x = v2[0].x;
	v2[1].y = zoomRect.height - 1;
	v2[2].x = zoomRect.width - 1;
	v2[2].y = v2[1].y;
	v2[3].x = v2[2].x;
	v2[3].y = v2[0].y;
	v2[4].x = v2[0].x;
	v2[4].y = v2[0].y;


	VERTEX v24[12];
	CreateVERTEX(&v24[0], 0, 0, 0x88000000);
	CreateVERTEX(&v24[1], s.x, 0, 0x88000000);
	CreateVERTEX(&v24[2], v2[2].x, v2[0].y, 0x88000000);
	CreateVERTEX(&v24[3], v2[0].x, v2[0].y, 0x88000000);
	CreateVERTEX(&v24[4], v2[0].x, v2[2].y, 0x88000000);
	CreateVERTEX(&v24[5], 0, s.y, 0x88000000);
	CreateVERTEX(&v24[6], s.x, s.y, 0x88000000);
	CreateVERTEX(&v24[7], 0, s.y, 0x88000000);
	CreateVERTEX(&v24[8], v2[0].x, v2[2].y, 0x88000000);
	CreateVERTEX(&v24[9], v2[2].x, v2[2].y, 0x88000000);
	CreateVERTEX(&v24[10], v2[2].x, v2[0].y, 0x88000000);
	CreateVERTEX(&v24[11], s.x, 0, 0x88000000);

	HRN(d3device->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE), "fvf failed");
	HRN(d3device->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 4, v24, sizeof(VERTEX)), "primitive failed");
	HRN(d3device->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 4, &v24[6], sizeof(VERTEX)), "primitive failed");
	lines->SetWidth(1);
	lines->Begin();
	lines->Draw(v2, 5, 0xFFBB0000);
	lines->End();

}

void VideoPlayer::ZoomMouseHandle(wxMouseEvent &evt)
{
	int x = evt.GetX();
	int y = evt.GetY();
	//wxWindow *win = this;
	VideoCtrl *vb = videoWindow;
	//if(isFullscreen){win = vb->TD; wxGetMousePosition(&x,&y);}

	wxSize s(backBufferRect.right, backBufferRect.bottom);
	wxSize s1(backBufferRect.right - backBufferRect.left, backBufferRect.bottom - backBufferRect.top);
	float ar = (float)s1.x / (float)s1.y;

	FloatRect tmp = zoomRect;
	//wxWindow *window = (isFullscreen)? (wxWindow*)((VideoCtrl*)this)->TD : this; 

	bool rotation = evt.GetWheelRotation() != 0;

	if (evt.ButtonUp()){
		if (vb->HasCapture()){ vb->ReleaseMouse(); }
		if (!vb->hasArrow){ 
			vb->SetCursor(wxCURSOR_ARROW); 
			vb->hasArrow = true; 
		}
	}


	if (!(evt.LeftDown() || evt.LeftIsDown())){
		bool setarrow = false;

		if (abs(x - zoomRect.x) < 5){
			setarrow = true;
			vb->SetCursor(wxCURSOR_SIZEWE);
			vb->hasArrow = false;
		}
		if (abs(y - zoomRect.y) < 5){
			setarrow = true;
			vb->SetCursor(wxCURSOR_SIZENS);
			vb->hasArrow = false;
		}
		if (abs(x - zoomRect.width) < 5){
			setarrow = true;
			vb->SetCursor(wxCURSOR_SIZEWE);
			vb->hasArrow = false;
		}
		if (abs(y - zoomRect.height) < 5){
			setarrow = true;
			vb->SetCursor(wxCURSOR_SIZENS);
			vb->hasArrow = false;
		}

		if (!setarrow && !vb->hasArrow){ vb->SetCursor(wxCURSOR_ARROW); vb->hasArrow = true; }
	}
	if (evt.LeftDown()){
		if (!vb->HasCapture()){ vb->CaptureMouse(); }
		grabbed = -1;
		if (abs(x - zoomRect.x) < 5){
			zoomDiff.x = zoomRect.x - x;
			grabbed = 0;
		}
		else if (abs(y - zoomRect.y) < 5){
			zoomDiff.y = zoomRect.y - y;
			grabbed = 1;
		}
		else if (abs(x - zoomRect.width) < 5){
			zoomDiff.x = zoomRect.width - x;
			grabbed = 2;
		}
		else if (abs(y - zoomRect.height) < 5){
			zoomDiff.y = zoomRect.height - y;
			grabbed = 3;
		}
		else{
			zoomDiff.x = x - zoomRect.x;
			zoomDiff.y = y - zoomRect.y;
		}

	}
	else if (evt.LeftIsDown() || rotation){
		int minx = backBufferRect.left;
		int miny = backBufferRect.top;
		if (rotation){
			int step = 5 * evt.GetWheelRotation() / evt.GetWheelDelta();
			zoomRect.x -= step;
			zoomRect.y -= step / ar;
			zoomRect.width += step;
			zoomRect.height += step / ar;
		}
		else if (grabbed < 0){
			float oldzx = zoomRect.x;
			float oldzy = zoomRect.y;
			if (zoomRect.x >= minx && zoomRect.width < s.x || (zoomRect.width == s.x && zoomRect.x > x - zoomDiff.x)){
				zoomRect.x = x - zoomDiff.x;
			}
			if (zoomRect.y >= miny && zoomRect.height < s.y || (zoomRect.height == s.y && zoomRect.y > y - zoomDiff.y)){
				zoomRect.y = y - zoomDiff.y;
			}
			if (zoomRect.x >= minx && zoomRect.width <= s.x){
				zoomRect.width += (zoomRect.x - oldzx);
			}
			if (zoomRect.y >= miny && zoomRect.height <= s.y){
				zoomRect.height += (zoomRect.y - oldzy);
			}
			zoomRect.x = MID(minx, zoomRect.x, s.x);
			zoomRect.y = MID(miny, zoomRect.y, s.y);
			zoomRect.width = MIN(zoomRect.width, s.x);
			zoomRect.height = MIN(zoomRect.height, s.y);
			Zoom(s1);
			return;
		}
		else if (grabbed < 2){
			if (grabbed == 0){
				float oldzx = zoomRect.x;
				zoomRect.x = x - zoomDiff.x;
				//if(zoomRect.x<minx){zoomRect = FloatRect(minx, miny, s.x, s.y);Zoom(s);return;}
				zoomRect.y += (zoomRect.x - oldzx) / ar;
				zoomRect.width -= (zoomRect.x - oldzx);
				zoomRect.height -= (zoomRect.x - oldzx) / ar;
			}
			else{
				float oldzy = zoomRect.y;
				zoomRect.y = y - zoomDiff.y;
				//if(zoomRect.y<miny){zoomRect = FloatRect(minx, miny, s.x, s.y);Zoom(s);return;}
				zoomRect.x += (zoomRect.y - oldzy) * ar;
				zoomRect.height -= (zoomRect.y - oldzy);
				zoomRect.width -= (zoomRect.y - oldzy) * ar;
			}
		}
		else{
			//wLogStatus("zoom1 %f %f %f %f", zoomRect.x, zoomRect.y, zoomRect.width, zoomRect.height);
			if (grabbed == 2){
				//if(zoomRect.width - zoomRect.x < 100 || (zoomRect.width - zoomRect.x == 100 && zoomRect.x < x - zoomDiff.x) ){return;}
				float oldzw = zoomRect.width;
				zoomRect.width = (x - zoomDiff.x);
				zoomRect.height += (zoomRect.width - oldzw) / ar;
			}
			else{
				//if(zoomRect.width - zoomRect.x < 100 || (zoomRect.width - zoomRect.x == 100 && zoomRect.y < y - zoomDiff.y) ){return;}
				float oldzh = zoomRect.height;
				zoomRect.height = (y - zoomDiff.y);
				zoomRect.width += (zoomRect.height - oldzh) * ar;
			}

		}
		////wLogStatus("zoom1 %f %f %f %f", zoomRect.x, zoomRect.y, zoomRect.width, zoomRect.height);
		if (zoomRect.width > s.x){
			zoomRect.x -= zoomRect.width - s.x;
			zoomRect.width = s.x;
		}
		if (zoomRect.height > s.y){
			zoomRect.y -= zoomRect.height - s.y;
			zoomRect.height = s.y;
		}
		if (zoomRect.x < minx){
			zoomRect.width -= (zoomRect.x - minx);
			zoomRect.x = minx;
		}
		if (zoomRect.y < miny){
			zoomRect.height -= (zoomRect.y - miny);
			zoomRect.y = miny;
		}

		zoomRect.width = MIN(zoomRect.width, s.x);
		zoomRect.height = MIN(zoomRect.height, s.y);
		zoomRect.x = MID(minx, zoomRect.x, s.x);
		zoomRect.y = MID(miny, zoomRect.y, s.y);
		if (zoomRect.width - zoomRect.x < 100){
			zoomRect = tmp;
		}
		Zoom(s1);
	}

}

void VideoPlayer::Zoom(const wxSize &size)
{
	//wxSize s1(backBufferRect.right - backBufferRect.left, backBufferRect.bottom - backBufferRect.top);
	hasZoom = true;
	float videoToScreenXX = size.x / (float)vwidth;
	float videoToScreenYY = size.y / (float)vheight;
	mainStreamRect.left = (zoomRect.x - backBufferRect.left) / videoToScreenXX;
	mainStreamRect.top = (zoomRect.y - backBufferRect.top) / videoToScreenYY;
	mainStreamRect.right = (zoomRect.width - backBufferRect.left) / videoToScreenXX;
	mainStreamRect.bottom = (zoomRect.height - backBufferRect.top) / videoToScreenYY;
	zoomParcent = size.x / (zoomRect.width - zoomRect.x/* + backBufferRect.left*/);
	if (isFullscreen){ UpdateRects(false); }
	if (Visual){
		SetVisualZoom();
		if (Visual && (Visual->Visual < CLIPRECT || Visual->Visual > VECTORDRAW)){
			SAFE_DELETE(Visual->dummytext);
			Visual->SetCurVisual();
			hasVisualEdition = true;
		}
	}
	Render(false);
	videoWindow->SetScaleAndZoom();
}

void VideoPlayer::SetVisualZoom()
{
	float videoToScreenX = (float)(backBufferRect.right - backBufferRect.left) / (float)(vwidth);
	float videoToScreenY = (float)(backBufferRect.bottom - backBufferRect.top) / (float)(vheight);
	float zoomX = mainStreamRect.left * videoToScreenX;
	float zoomY = mainStreamRect.top * videoToScreenY;
	D3DXVECTOR2 zoomScale((float)vwidth / (float)(mainStreamRect.right - mainStreamRect.left),
		(float)vheight / (float)(mainStreamRect.bottom - mainStreamRect.top));
	Visual->SetZoom(D3DXVECTOR2(zoomX - (backBufferRect.left / zoomScale.x),
		zoomY - (backBufferRect.top / zoomScale.y)), zoomScale);
}