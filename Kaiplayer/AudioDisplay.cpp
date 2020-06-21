// Copyright (c) 2005, Rodrigo Braz Monteiro
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of the Aegisub Group nor the names of its contributors
//     may be used to endorse or promote products derived from this software
//     without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// -----------------------------------------------------------------------------
//
// AEGISUB
//
// Website: http://aegisub.cellosoft.com
// Contact: mailto:zeratul@cellosoft.com
//


///////////
// Headers

#include "Config.h"

#include "KaiMessageBox.h"
#include <wx/filename.h>
#include <math.h>
#include <vector>
#include "AudioDisplay.h"
#include "EditBox.h"

#include "Config.h"
#include "AudioBox.h"

#include "ColorSpace.h"
#include "Hotkeys.h"
#include "SubsGrid.h"
#include "kainoteApp.h"
#include "RendererVideo.h"
#include <math.h>
//#undef DrawText

inline D3DCOLOR D3DCOLOR_FROM_WX(const wxColour &col){
	return (D3DCOLOR)((((col.Alpha()) & 0xff) << 24) | (((col.Red()) & 0xff) << 16) | (((col.Green()) & 0xff) << 8) | ((col.Blue()) & 0xff));
}
//inline D3DCOLOR D3DCOLOR_FROM_WXA(col, alpha){
//	((D3DCOLOR)((((alpha)& 0xff) << 24) | (((col.Red()) & 0xff) << 16) | (((col.Green()) & 0xff) << 8) | ((col.Blue()) & 0xff)))
//}

int64_t abs64(int64_t input) {
	if (input < 0) return -input;
	return input;
}




///////////////
// Constructor
AudioDisplay::AudioDisplay(wxWindow *parent)
	: wxWindow(parent, -1, wxDefaultPosition, wxSize(100, 100), wxWANTS_CHARS, _T("Audio Display"))
	, spectrumSurface(NULL)
	, d3dDevice(NULL)
	, d3dObject(NULL)
	, d3dLine(NULL)
	, d3dFontTahoma13(NULL)
	, d3dFontTahoma8(NULL)
	, d3dFontVerdana11(NULL)
	, backBuffer(NULL)
{
	// Set variables
	deviceLost = false;
	spectrumRenderer = NULL;
	ScrollBar = NULL;
	karaoke = NULL;
	peak = NULL;
	min = NULL;
	dialogue = NULL;
	cursorPaint = false;
	defCursor = true;
	karaAuto = Options.GetBool(AUDIO_KARAOKE_SPLIT_MODE);
	hasKara = Options.GetBool(AUDIO_KARAOKE);
	if (hasKara){ karaoke = new Karaoke(this); }
	hasSel = hasMark = false;
	diagUpdated = false;
	NeedCommit = false;
	loaded = false;
	blockUpdate = false;
	holding = false;
	draggingScale = false;
	inside = ownProvider = false;
	whichsyl = 0;
	letter = -1;
	Grabbed = -1;
	Position = 0;
	PositionSample = 0;
	oldCurPos = 0;
	scale = 1.0f;
	provider = NULL;
	player = NULL;
	hold = 0;
	samples = 0;
	samplesPercent = 100;
	selMark = 0;
	curMarkMS = 0;
	hasFocus = (wxWindow::FindFocus() == this);
	needImageUpdate = false;
	needImageUpdateWeak = true;
	playingToEnd = false;
	LastSize = wxSize(-1, -1);
	int fontSize = Options.GetInt(PROGRAM_FONT_SIZE);
	verdana11 = wxFont(fontSize + 1, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, L"Verdana");
	tahoma13 = wxFont(fontSize + 3, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, L"Tahoma");
	tahoma8 = *Options.GetFont(-1);
	int fh;
	GetTextExtent(L"#TWFfGH", NULL, &fh, NULL, NULL, &tahoma8);
	timelineHeight = fh + 8;
	UpdateTimer.SetOwner(this, Audio_Update_Timer);
	GetClientSize(&w, &h);
	h -= timelineHeight;
	ProgressTimer.SetOwner(this, 7654);
	Bind(wxEVT_TIMER, [=](wxTimerEvent &evt){
		if (!provider->audioNotInitialized){
			UpdateImage(); ProgressTimer.Stop();
		}
		else if (provider->audioProgress != lastProgress){
			Refresh(false);
		}
	}, 7654);
	ChangeOptions();
	// Set cursor
	//wxCursor cursor(wxCURSOR_BLANK);
	//SetCursor(cursor);

}


//////////////
// Destructor
AudioDisplay::~AudioDisplay() {
	if (player) { player->CloseStream(); delete player; }
	if (ownProvider && provider) { delete provider; provider = NULL; }
	ClearDX();
	if (karaoke){ delete karaoke; }
	if (spectrumRenderer){ delete spectrumRenderer; };
	if (peak){
		delete[] peak;
		delete[] min;
	}


	player = NULL;
	karaoke = NULL;
	spectrumRenderer = NULL;
	peak = NULL;
	min = NULL;
}

/////////
// Reset
void AudioDisplay::Reset() {
	hasSel = false;
	diagUpdated = false;
	NeedCommit = false;
}


////////////////
// Update image
void AudioDisplay::UpdateImage(bool weak, bool updateImmediately) {
	// Update samples
	UpdateSamples();

	// Set image as needing to be redrawn
	needImageUpdate = true;
	if (weak == false) {// && needImageUpdateWeak == true
		needImageUpdateWeak = false;
	}
	if (updateImmediately){
		DoUpdateImage();
	}
	else{
		Refresh(false);
	}
}

void AudioDisplay::DrawDashedLine(D3DXVECTOR2 *vector, size_t vectorSize, D3DCOLOR fill, int dashLen)
{

	D3DXVECTOR2 actualPoint[2];
	for (size_t i = 0; i < vectorSize - 1; i++){
		size_t iPlus1 = (i < (vectorSize - 1)) ? i + 1 : 0;
		D3DXVECTOR2 pdiff = vector[i] - vector[iPlus1];
		float len = sqrt((pdiff.x * pdiff.x) + (pdiff.y * pdiff.y));
		if (len == 0){ return; }
		D3DXVECTOR2 diffUnits = pdiff / len;
		float singleMovement = 1 / (len / (dashLen * 2));
		actualPoint[0] = vector[i];
		actualPoint[1] = actualPoint[0];
		for (float j = 0; j <= 1; j += singleMovement){
			actualPoint[1] -= diffUnits * dashLen;
			if (j + singleMovement >= 1){ actualPoint[1] = vector[iPlus1]; }
			d3dLine->Draw(actualPoint, 2, fill);
			actualPoint[1] -= diffUnits * dashLen;
			actualPoint[0] -= (diffUnits * dashLen) * 2;
		}
	}
}

void AudioDisplay::ClearDX()
{
	SAFE_RELEASE(spectrumSurface);
	SAFE_RELEASE(backBuffer);
	SAFE_RELEASE(d3dDevice);
	SAFE_RELEASE(d3dObject);
	SAFE_RELEASE(d3dLine);
	SAFE_RELEASE(d3dFontTahoma13);
	SAFE_RELEASE(d3dFontTahoma8);
	SAFE_RELEASE(d3dFontVerdana11);
}

bool AudioDisplay::InitDX(const wxSize &size)
{

	if (!d3dObject){
		d3dObject = Direct3DCreate9(D3D_SDK_VERSION);
		PTR(d3dObject, _("Nie można utworzyć obiektu Direct3D"));
	}
	else{
		SAFE_RELEASE(spectrumSurface);
		SAFE_RELEASE(backBuffer);
		SAFE_RELEASE(d3dLine);
		SAFE_RELEASE(d3dFontTahoma13);
		SAFE_RELEASE(d3dFontTahoma8);
		SAFE_RELEASE(d3dFontVerdana11);
	}

	HRESULT hr;
	HWND hwnd = GetHWND();
	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.Windowed = TRUE;
	d3dpp.hDeviceWindow = hwnd;
	d3dpp.BackBufferWidth = size.x;
	d3dpp.BackBufferHeight = size.y;
	d3dpp.BackBufferCount = 1;
	d3dpp.SwapEffect = D3DSWAPEFFECT_COPY;//D3DSWAPEFFECT_DISCARD;//D3DSWAPEFFECT_COPY;//
	d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
	//d3dpp.Flags					 = D3DPRESENTFLAG_VIDEO;
	d3dpp.Flags = 0;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;//D3DPRESENT_INTERVAL_DEFAULT;

	if (d3dDevice){
		hr = d3dDevice->Reset(&d3dpp);
		if (FAILED(hr)){ return false; }
	}
	else{
		hr = d3dObject->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd,
			D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED, &d3dpp, &d3dDevice);//| D3DCREATE_FPU_PRESERVE
		if (FAILED(hr)){
			HR(d3dObject->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd,
				D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED, &d3dpp, &d3dDevice), _("Nie można utworzyć urządzenia D3D9"));
		}
	}
	hr = d3dDevice->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, TRUE);
	hr = d3dDevice->SetRenderState(D3DRS_ANTIALIASEDLINEENABLE, TRUE);

	hr = d3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	hr = d3dDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
	hr = d3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	hr = d3dDevice->SetRenderState(D3DRS_DITHERENABLE, TRUE);

	hr = d3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	hr = d3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	hr = d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	hr = d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	hr = d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	hr = d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_SPECULAR);

	hr = d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	hr = d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	hr = d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	HR(hr, _("Zawiodło któreś z ustawień DirectX"));

	D3DXMATRIX matOrtho;
	D3DXMATRIX matIdentity;

	D3DXMatrixOrthoOffCenterLH(&matOrtho, 0, size.x, size.y, 0, 0.0f, 1.0f);
	D3DXMatrixIdentity(&matIdentity);

	HR(d3dDevice->SetTransform(D3DTS_PROJECTION, &matOrtho), _("Nie można ustawić macierzy porojekcji"));
	HR(d3dDevice->SetTransform(D3DTS_WORLD, &matIdentity), _("Nie można ustawić macierzy świata"));
	HR(d3dDevice->SetTransform(D3DTS_VIEW, &matIdentity), _("Nie można ustawić macierzy widoku"));
	HR(d3dDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &backBuffer), _("Nie można stworzyć powierzchni"));


	HR(D3DXCreateLine(d3dDevice, &d3dLine), _("Nie można stworzyć linii D3DX"));
	wxSize sizeTahoma13 = tahoma13.GetPixelSize();
	wxSize sizeTahoma8 = tahoma8.GetPixelSize();
	wxSize sizeVerdana11 = verdana11.GetPixelSize();
	HR(D3DXCreateFontW(d3dDevice, sizeTahoma13.y, sizeTahoma13.x, FW_BOLD, 0, FALSE, DEFAULT_CHARSET, OUT_TT_ONLY_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, TEXT("Tahoma"), &d3dFontTahoma13), _("Nie można stworzyć czcionki D3DX"));
	HR(D3DXCreateFontW(d3dDevice, sizeTahoma8.y, sizeTahoma8.x, FW_NORMAL, 0, FALSE, DEFAULT_CHARSET, OUT_TT_ONLY_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, TEXT("Tahoma"), &d3dFontTahoma8), _("Nie można stworzyć czcionki D3DX"));
	HR(D3DXCreateFontW(d3dDevice, sizeVerdana11.y, sizeVerdana11.x, FW_BOLD, 0, FALSE, DEFAULT_CHARSET, OUT_TT_ONLY_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, TEXT("Verdana"), &d3dFontVerdana11), _("Nie można stworzyć czcionki D3DX"));
	//HR(d3dLine->SetAntialias(TRUE), _("Linia nie ustawi AA"));
	HR(d3dDevice->CreateOffscreenPlainSurface(size.x, size.y, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &spectrumSurface, 0), _("Nie można stworzyć plain surface"));
	//HR(d3dDevice->CreateTexture(size.x, size.y, 1, D3DUSAGE_RENDERTARGET,
	//D3DFMT_R8G8B8,D3DPOOL_DEFAULT,&texture, NULL), "Nie można utworzyć tekstury" );
	HR(d3dDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE), L"FVF failed");

	return true;
}

void AudioDisplay::DoUpdateImage() {
	// Prepare bitmap
	int displayH = h + timelineHeight;
	// Invalid dimensions
	if (w < 1 || displayH < 1 || isHidden) return;
	// Loaded?
	if (!loaded || !provider) return;
	wxMutexLocker lock(mutex);
	// Needs updating?
	//if (!needImageUpdate) return;
	bool weak = needImageUpdateWeak;

	if (LastSize.x != w || LastSize.y != h || !d3dDevice || needToReset) {
		LastSize = wxSize(w, h);
		if (!InitDX(wxSize(w, displayH))){
			ClearDX();
			if (!InitDX(wxSize(w, displayH))){
				KaiLog(_("Nie można zresetować Direct3D"));
				needToReset = true;
				return;
			}
		}
	}


	// Is spectrum?
	bool spectrum = false;
	if (provider && box->SpectrumMode->GetValue()) {
		spectrum = true;
	}
	HRESULT hr;
	if (deviceLost)
	{
		if (FAILED(hr = d3dDevice->TestCooperativeLevel()))
		{
			if (D3DERR_DEVICELOST == hr ||
				D3DERR_DRIVERINTERNALERROR == hr)
				return;

			if (D3DERR_DEVICENOTRESET == hr)
			{
				ClearDX();
				if (!InitDX(wxSize(w, displayH)))
					return;

				UpdateImage(false, true);
			}
			return;
		}

		deviceLost = false;
	}

	// Background
	hr = d3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, background, 1.0f, 0);


	hr = d3dDevice->BeginScene();
	// Draw image to be displayed

	if (provider->audioNotInitialized){
		DrawProgress();
	}
	else{
		// Option
		selStart = 0;
		selEnd = 0;
		lineStart = 0;
		lineEnd = 0;
		selStartCap = 0;
		selEndCap = 0;
		int64_t drawSelStart = 0;
		int64_t drawSelEnd = 0;

		GetDialoguePos(lineStart, lineEnd, false);
		hasSel = true;

		GetDialoguePos(selStartCap, selEndCap, true);
		selStart = lineStart;
		selEnd = lineEnd;
		drawSelStart = lineStart;
		drawSelEnd = lineEnd;

		if (spectrum) {
			DrawSpectrum(weak);
		}

		// Draw selection bg
		if (hasSel && drawSelStart < drawSelEnd && drawSelectionBackground) {
			D3DCOLOR fill;
			if (NeedCommit) fill = selectionBackgroundModified;
			else fill = selectionBackground;
			VERTEX v9[4];
			CreateVERTEX(&v9[0], drawSelStart, 0, fill);
			CreateVERTEX(&v9[1], drawSelEnd + 1, 0, fill);
			CreateVERTEX(&v9[2], drawSelStart, h, fill);
			CreateVERTEX(&v9[3], drawSelEnd + 1, h, fill);
			HRN(hr = d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, v9, sizeof(VERTEX)), L"primitive failed");

		}

		HRN(d3dLine->SetWidth(1.0f), L"line set width failed");

		// Draw spectrum


		if (!spectrum){
			//// Waveform
			if (provider) {
				DrawWaveform(weak);
			}

			// Nothing
			else {
				D3DXVECTOR2 v2[2] = { D3DXVECTOR2(0, h / 2), D3DXVECTOR2(w, h / 2) };
				d3dLine->Begin();
				d3dLine->Draw(v2, 2, waveform);
				d3dLine->End();
			}
		}

		//// Draw previous line
		DrawInactiveLines();

		// Draw seconds boundaries
		if (drawBoundaryLines) {
			d3dLine->Begin();
			int64_t start = Position*samples;
			int rate = provider->GetSampleRate();
			int pixBounds = rate / samples;
			D3DXVECTOR2 v2[2] = { D3DXVECTOR2(0, 0), D3DXVECTOR2(0, h) };
			if (pixBounds >= 8) {
				for (int x = 0; x < w; x++) {
					if (((x*samples) + start) % rate < samples) {
						v2[0].x = x;
						v2[1].x = x;
						DrawDashedLine(v2, 2, secondBondariesColor);
					}
				}
			}
			d3dLine->End();
		}




		if (hasSel) {
			// Draw boundaries
			// Draw start boundary
			int startDraw = lineStart + (selWidth / 2);
			//if(selWidth % 2 == 0){startDraw--;}
			D3DXVECTOR2 v2[2] = { D3DXVECTOR2(startDraw, 0), D3DXVECTOR2(startDraw, h) };
			d3dLine->SetWidth(selWidth);
			d3dLine->Begin();
			d3dLine->Draw(v2, 2, lineStartBondaryColor);
			d3dLine->End();
			VERTEX v6[6];
			CreateVERTEX(&v6[0], startDraw, 0, lineStartBondaryColor);
			CreateVERTEX(&v6[1], startDraw + 10, 0, lineStartBondaryColor);
			CreateVERTEX(&v6[2], startDraw, 10, lineStartBondaryColor);
			CreateVERTEX(&v6[3], startDraw, h - 10, lineStartBondaryColor);
			CreateVERTEX(&v6[4], startDraw + 10, h, lineStartBondaryColor);
			CreateVERTEX(&v6[5], startDraw, h, lineStartBondaryColor);

			HRN(d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 1, v6, sizeof(VERTEX)), L"primitive failed");
			HRN(d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 1, &v6[3], sizeof(VERTEX)), L"primitive failed");
			HRN(d3dDevice->DrawPrimitiveUP(D3DPT_LINESTRIP, 1, v6, sizeof(VERTEX)), L"primitive failed");
			HRN(d3dDevice->DrawPrimitiveUP(D3DPT_LINESTRIP, 1, &v6[3], sizeof(VERTEX)), L"primitive failed");

			// Draw end boundary

			startDraw = lineEnd + (selWidth / 2);
			v2[0] = D3DXVECTOR2(startDraw, 0);
			v2[1] = D3DXVECTOR2(startDraw, h);
			d3dLine->Begin();
			d3dLine->Draw(v2, 2, lineEndBondaryColor);
			d3dLine->End();
			d3dLine->SetWidth(1.f);
			CreateVERTEX(&v6[0], startDraw, 0, lineEndBondaryColor);
			CreateVERTEX(&v6[1], startDraw - 10, 0, lineEndBondaryColor);
			CreateVERTEX(&v6[2], startDraw, 10, lineEndBondaryColor);
			CreateVERTEX(&v6[3], startDraw, h - 10, lineEndBondaryColor);
			CreateVERTEX(&v6[4], startDraw - 10, h, lineEndBondaryColor);
			CreateVERTEX(&v6[5], startDraw, h, lineEndBondaryColor);

			HRN(d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 1, v6, sizeof(VERTEX)), L"primitive failed");
			HRN(d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 1, &v6[3], sizeof(VERTEX)), L"primitive failed");
			HRN(d3dDevice->DrawPrimitiveUP(D3DPT_LINESTRIP, 1, v6, sizeof(VERTEX)), L"primitive failed");
			HRN(d3dDevice->DrawPrimitiveUP(D3DPT_LINESTRIP, 1, &v6[3], sizeof(VERTEX)), L"primitive failed");

			// Draw karaoke
			if (hasKara) {
				int karstart = selStart;
				wxString acsyl;
				D3DXVECTOR2 v2[2] = { D3DXVECTOR2(0, 0), D3DXVECTOR2(0, h) };
				for (size_t j = 0; j < karaoke->syls.size(); j++)
				{
					karaoke->GetTextStripped(j, acsyl);

					int fw = 0, fh = 0;
					if (!acsyl.empty())
						GetTextExtentPixel(acsyl, &fw, &fh);

					float XX = GetXAtMS(karaoke->syltimes[j]);
					if (XX >= 0){
						v2[0].x = XX;
						v2[1].x = XX;
						d3dLine->Begin();
						d3dLine->Draw(v2, 2, syllableBondaresColor);
						d3dLine->End();
					}
					if (fh != 0){
						int center = ((XX - karstart) - fw) / 2;
						D3DXVECTOR2 v5[2] = { D3DXVECTOR2(center + karstart - 1, (fh / 2) + 1), D3DXVECTOR2(center + karstart + fw + 2, (fh / 2 + 1)) };
						d3dLine->SetWidth(fh);
						d3dLine->Begin();
						d3dLine->Draw(v5, 2, syllableBondaresColor);
						d3dLine->End();
						d3dLine->SetWidth(1);
						RECT rect = { center + karstart, 0, center + karstart + fw, fh };
						d3dFontVerdana11->DrawTextW(NULL, acsyl.wchar_str(), -1, &rect, DT_LEFT, syllableTextColor);
						//obramowanie aktywynej sylaby
						if (letter >= 0 && syll >= 0 && syll == j){
							int start, end;
							int fwl, fhl;
							if (letter == 0){ fwl = 0; }
							else{
								GetTextExtentPixel(acsyl.Mid(0, letter), &fwl, &fhl);
							}

							karaoke->GetSylTimes(j, start, end);

							start = GetXAtMS(start);
							end = GetXAtMS(end);

							int center = start + ((end - start - fw) / 2);
							D3DXVECTOR2 v3[2] = { D3DXVECTOR2(center + fwl, 1), D3DXVECTOR2(center + fwl, fh) };
							d3dLine->Begin();
							d3dLine->Draw(v3, 2, syllableTextColor);
							d3dLine->End();
						}
					}
					if (j == whichsyl){
						D3DXVECTOR2 v5[5] = { D3DXVECTOR2(karstart + 2, 1), D3DXVECTOR2(karstart + 2, h - 2), D3DXVECTOR2(XX - 2, h - 2), D3DXVECTOR2(XX - 2, 1), D3DXVECTOR2(karstart + 2, 1) };
						d3dLine->Begin();
						d3dLine->Draw(v5, 5, syllableTextColor);
						d3dLine->End();
					}

					karstart = XX;
				}
			}
		}
		// Draw keyframes
		if (drawKeyframes && provider->KeyFrames.size() > 0) {
			DrawKeyframes();
		}

		// Modified text
		if (NeedCommit || selStart > selEnd) {
			RECT rect;
			rect.left = 4;
			rect.top = 4;
			rect.right = rect.left + 300;
			rect.bottom = rect.top + 100;
			wxString text;
			if (selStart <= selEnd) {
				text = _("Zmodyfikowano");
				DRAWOUTTEXT(d3dFontVerdana11, text, rect, DT_LEFT | DT_TOP, 0xFFFF0000);
			}
			else {
				text = _("Czas ujemny");
				DRAWOUTTEXT(d3dFontVerdana11, text, rect, DT_LEFT | DT_TOP, 0xFFFF0000);
			}
		}

		DrawTimescale();


		if (hasMark){
			selMark = GetXAtMS(curMarkMS);
			if (selMark >= 0 && selMark < w){
				d3dLine->SetWidth(2.f);
				d3dLine->Begin();
				D3DXVECTOR2 v2[2] = { D3DXVECTOR2(selMark + 1, 0), D3DXVECTOR2(selMark + 1, h) };
				d3dLine->Draw(v2, 2, lineBondaryMark);
				d3dLine->End();
				d3dLine->SetWidth(1.f);
				//dc.DrawRectangle(selMark,0,2,h);
				STime time(curMarkMS);
				wxString text = time.raw();
				int dx, dy;
				GetTextExtent(text, &dx, &dy, 0, 0, &verdana11);
				//dx=selMark-(dx/2);
				dy = h - dy - 2;
				RECT rect;
				rect.left = selMark - 150;
				rect.top = dy;
				rect.right = rect.left + 300;
				rect.bottom = rect.top + 100;
				DRAWOUTTEXT(d3dFontVerdana11, text, rect, DT_CENTER, 0xFFFFFFFF);
			}
		}
		// Draw current frame
		if (drawVideoPos) {
			VideoCtrl *Video = Notebook::GetTab()->Video;
			if (Video->GetState() == Paused) {
				d3dLine->SetWidth(2);

				float x = GetXAtMS(Video->Tell());
				d3dLine->Begin();
				D3DXVECTOR2 v2[2] = { D3DXVECTOR2(x, 0), D3DXVECTOR2(x, h) };
				DrawDashedLine(v2, 2, AudioCursor);
				d3dLine->End();
				d3dLine->SetWidth(1.f);
			}
		}



		if (cursorPaint){
			D3DXVECTOR2 v2[2] = { D3DXVECTOR2(curpos, 0), D3DXVECTOR2(curpos, h) };
			d3dLine->SetWidth(2);
			d3dLine->SetAntialias(TRUE);
			d3dLine->Begin();
			d3dLine->Draw(v2, 2, AudioCursor);
			d3dLine->End();
			d3dLine->SetAntialias(FALSE);
			d3dLine->SetWidth(1);
			if (!player->IsPlaying()){
				STime time;
				time.NewTime(GetMSAtX(curpos));
				wxString text = time.GetFormatted(ASS);
				RECT rect;
				rect.left = curpos - 150;
				rect.top = (hasKara) ? 20 : 5;
				rect.right = rect.left + 300;
				rect.bottom = rect.top + 100;
				DRAWOUTTEXT(d3dFontTahoma13, text, rect, DT_CENTER, 0xFFFFFFFF);
			}
		}


		// Draw focus border
		if (hasFocus) {
			D3DXVECTOR2 v5[5] = { D3DXVECTOR2(0, 0), D3DXVECTOR2(w - 1, 0), D3DXVECTOR2(w - 1, h - 1), D3DXVECTOR2(0, h - 1), D3DXVECTOR2(0, 0) };
			d3dLine->Begin();
			d3dLine->Draw(v5, 5, waveform);
			d3dLine->End();
		}

	}
	hr = d3dDevice->EndScene();

	hr = d3dDevice->Present(NULL, NULL, NULL, NULL);

	if (D3DERR_DEVICELOST == hr ||
		D3DERR_DRIVERINTERNALERROR == hr){
		deviceLost = true;
		UpdateImage(false, true);
	}
	// Done
	needImageUpdate = false;
	needImageUpdateWeak = true;
}


///////////////////////
// Draw Inactive Lines
void AudioDisplay::DrawInactiveLines() {
	// Check if there is anything to do
	if (shadeType == 0) return;

	// Spectrum?
	bool spectrum = false;
	if (provider && spectrumOn) {
		spectrum = true;
	}

	// Set options
	Dialogue *shade;
	int shadeX1, shadeX2;
	int shadeFrom, shadeTo;

	// Only previous
	if (shadeType == 1) {
		shadeFrom = grid->GetKeyFromPosition(line_n, -1);
		shadeTo = grid->GetKeyFromPosition(line_n, 1);
	}

	// All
	else {
		shadeFrom = 0;
		shadeTo = grid->GetCount() - 1;
	}
	D3DXVECTOR2 v2[2];
	Dialogue *ADial = grid->GetDialogue(line_n);
	if (!ADial){ return; }
	int aS = GetXAtMS(ADial->Start.mstime);
	int aE = GetXAtMS(ADial->End.mstime);

	for (int j = shadeFrom; j <= shadeTo; j++) {
		if (j == line_n) continue;
		if (j < 0 || j >= grid->GetCount()) continue;
		shade = grid->GetDialogue(j);
		if (!shade->isVisible)
			continue;

		// Get coordinates
		shadeX1 = GetXAtMS(shade->Start.mstime);
		shadeX2 = GetXAtMS(shade->End.mstime);
		if (shadeX2 < 0 || shadeX1 > w) continue;


		// Draw over waveform

		// Selection
		int selX1 = MAX(0, GetXAtMS(curStartMS));
		int selX2 = MIN(w, GetXAtMS(curEndMS));

		// Get ranges (x1->x2, x3->x4).
		int x1 = MAX(0, shadeX1);
		int x2 = MIN(w, shadeX2);
		int x3 = MAX(x1, selX2);
		int x4 = MAX(x2, selX2);

		// Clip first range
		x1 = MIN(x1, selX1);
		x2 = MIN(x2, selX1);

		VERTEX v9[4];
		CreateVERTEX(&v9[0], x1, 0, inactiveLinesBackground);
		CreateVERTEX(&v9[1], x2 + 1, 0, inactiveLinesBackground);
		CreateVERTEX(&v9[2], x1, h, inactiveLinesBackground);
		CreateVERTEX(&v9[3], x2 + 1, h, inactiveLinesBackground);
		HRN(d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, v9, sizeof(VERTEX)), L"inactive lines primitive failed");
		CreateVERTEX(&v9[0], x3, 0, inactiveLinesBackground);
		CreateVERTEX(&v9[1], x4 + 1, 0, inactiveLinesBackground);
		CreateVERTEX(&v9[2], x3, h, inactiveLinesBackground);
		CreateVERTEX(&v9[3], x4 + 1, h, inactiveLinesBackground);
		HRN(d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, v9, sizeof(VERTEX)), L"inactive lines primitive failed");

		if (!spectrum) {
			d3dLine->Begin();
			// draw lines of inactive waveform
			for (int i = x1; i < x2; i++){
				v2[0] = D3DXVECTOR2(i, peak[i]);
				v2[1] = D3DXVECTOR2(i, min[i] - 1);
				d3dLine->Draw(v2, 2, waveformInactive);
			}
			for (int i = x3; i < x4; i++){
				v2[0] = D3DXVECTOR2(i, peak[i]);
				v2[1] = D3DXVECTOR2(i, min[i] - 1);
				d3dLine->Draw(v2, 2, waveformInactive);
			}


			d3dLine->End();
		}

		// Draw boundaries
		d3dLine->SetWidth(selWidth);
		d3dLine->Begin();
		v2[0] = D3DXVECTOR2(shadeX1 + (selWidth / 2), 0);
		v2[1] = D3DXVECTOR2(shadeX1 + (selWidth / 2), h);
		d3dLine->Draw(v2, 2, boundaryInactiveLine);
		v2[0] = D3DXVECTOR2(shadeX2 + (selWidth / 2), 0);
		v2[1] = D3DXVECTOR2(shadeX2 + (selWidth / 2), h);
		d3dLine->Draw(v2, 2, boundaryInactiveLine);
		d3dLine->End();
		d3dLine->SetWidth(1.f);
	}

}



//////////////////
// Draw timescale
void AudioDisplay::DrawTimescale() {

	// Set colours
	VERTEX v9[4];
	D3DXVECTOR2 v2[2];
	CreateVERTEX(&v9[0], 0, h, timescaleBackground);
	CreateVERTEX(&v9[1], w, h, timescaleBackground);
	CreateVERTEX(&v9[2], 0, h + timelineHeight, timescaleBackground);
	CreateVERTEX(&v9[3], w, h + timelineHeight, timescaleBackground);

	HRN(d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, v9, sizeof(VERTEX)), L"primitive failed");
	d3dLine->Begin();
	v2[0] = D3DXVECTOR2(0, h);
	v2[1] = D3DXVECTOR2(w, h);
	d3dLine->Draw(v2, 2, timescaleText);
	
	// Timescale ticks
	int64_t start = Position*samples;
	int rate = provider->GetSampleRate();
	int lastTextPos = -1000;
	int lastLinePos = -20;
	auto drawTime = [=](int x, int64_t pos, int *lastTextPos, bool drawMS){
		wxCoord textW;
		int s = pos / rate;
		int hr = s / 3600;
		int m = s / 60;
		m = m % 60;
		s = s % 60;
		wxString text;
		if (hr) text = wxString::Format(_T("%i:%02i:%02i"), hr, m, s);
		else if (m) text = wxString::Format(_T("%i:%02i"), m, s);
		else text = wxString::Format(_T("%i"), s);
		if (drawMS){
			int ms = (pos / (rate / 10)) % 10;
			if (ms)
				text << wxString::Format(_T(".%i"), ms);
		}
		GetTextExtent(text, &textW, NULL, NULL, NULL, &tahoma8);
		//if (drawMS)
		//textW += 20;
		if (x > (*lastTextPos) + textW){
			RECT rect;
			rect.left = x - 50;//MAX(0,x-textW/2)+1;
			rect.top = h + 8;
			rect.right = rect.left + 100;
			rect.bottom = rect.top + 40;
			d3dFontTahoma8->DrawTextW(NULL, text.wchar_str(), -1, &rect, DT_CENTER, timescaleText);
			(*lastTextPos) = x;
		}
	};
	for (int i = 1; i < 32; i *= 2) {
		int pixBounds = rate / (samples * 10 / i);
		if (pixBounds <= 1)
			pixBounds = 1;
		else if (pixBounds > 10)
			pixBounds = 10;
		else{
			pixBounds = (pixBounds / 2) * 2;
		}
		for (int x = 0; x < w; x++) {
			int64_t pos = (x * samples) + start;
			// Second boundary
			if (pos % rate < samples) {
				v2[0] = D3DXVECTOR2(x, h + 2);
				v2[1] = D3DXVECTOR2(x, h + 8);
				d3dLine->Draw(v2, 2, timescaleText);
				lastLinePos = x;
				// Draw text
				drawTime(x, pos, &lastTextPos, false);
			}

			// Other
			else if (pos % (rate / pixBounds * i) < samples) {
				v2[0] = D3DXVECTOR2(x, h + 2);
				v2[1] = D3DXVECTOR2(x, h + 5);
				d3dLine->Draw(v2, 2, timescaleText);
				if (lastLinePos + 20 <= x)
					drawTime(x, pos, &lastTextPos, true);
				lastLinePos = x;
			}
		}
		break;
	}
	d3dLine->End();
}


////////////
// Waveform
void AudioDisplay::DrawWaveform(bool weak) {
	// Prepare Waveform
	if (!weak || peak == NULL || min == NULL) {
		if (peak) delete[] peak;
		if (min) delete[] min;
		peak = new int[w];
		min = new int[w];
	}

	// Get waveform
	if (!weak) {
		provider->GetWaveForm(min, peak, Position*samples, w, h, samples, scale);
	}
	d3dLine->Begin();
	// Draw pre-selection
	if (!hasSel) selStartCap = w;
	D3DXVECTOR2 v2[2];
	HRESULT hr;
	for (int64_t i = 0; i < selStartCap; i++) {
		v2[0] = D3DXVECTOR2(i, peak[i]);
		v2[1] = D3DXVECTOR2(i, min[i] - 1);
		hr = d3dLine->Draw(v2, 2, waveform);
	}

	if (hasSel) {
		// Draw selection
		D3DCOLOR waveformSel = waveform;
		if (drawSelectionBackground) {
			if (NeedCommit) waveformSel = waveformModified;
			else waveformSel = waveformSelected;
		}
		for (int64_t i = selStartCap; i < selEndCap; i++) {
			v2[0] = D3DXVECTOR2(i, peak[i]);
			v2[1] = D3DXVECTOR2(i, min[i] - 1);
			d3dLine->Draw(v2, 2, waveformSel);
		}

		// Draw post-selection
		for (int64_t i = selEndCap; i < w; i++) {
			v2[0] = D3DXVECTOR2(i, peak[i]);
			v2[1] = D3DXVECTOR2(i, min[i] - 1);
			d3dLine->Draw(v2, 2, waveform);
		}
	}
	d3dLine->End();
}


//////////////////////////
// Draw spectrum analyzer
void AudioDisplay::DrawSpectrum(bool weak) {

	if (!weak) {
		if (!spectrumRenderer)
			spectrumRenderer = new AudioSpectrum(provider);
		spectrumRenderer->SetScaling(scale);
		D3DLOCKED_RECT d3dlr;
		try{
			HRN(spectrumSurface->LockRect(&d3dlr, 0, D3DLOCK_NOSYSLOCK), _("Nie można zablokować bufora tekstury"));
		}
		catch (...){}
		byte *img = static_cast<byte *>(d3dlr.pBits);
		int dxw = d3dlr.Pitch / 4;

		// Use a slightly slower, but simple way
		// Always draw the spectrum for the entire width
		// Hack: without those divs by 2 the display is horizontally compressed
		spectrumRenderer->RenderRange(Position*samples, (Position + w)*samples, img, w, dxw, h, samplesPercent);
		spectrumSurface->UnlockRect();

	}
	wxRect crc = GetClientRect();
	RECT rc = { crc.x, crc.y, crc.width - crc.x, crc.height - crc.y };
	if (FAILED(d3dDevice->StretchRect(spectrumSurface, &rc, backBuffer, &rc, D3DTEXF_LINEAR))){
		KaiLog(_("Nie można nałożyć powierzchni spectrum na siebie"));
	}

}

void AudioDisplay::DrawProgress()
{
	//koordynaty czarnej ramki
	D3DXVECTOR2 vectors[16];
	float halfY = (h + 20) / 2;
	vectors[4].x = 20;
	vectors[4].y = halfY - 20;
	vectors[5].x = w - 20;
	vectors[5].y = halfY - 20;
	vectors[6].x = w - 20;
	vectors[6].y = halfY + 20;
	vectors[7].x = 20;
	vectors[7].y = halfY + 20;
	vectors[8].x = 20;
	vectors[8].y = halfY - 20;
	//koordynaty białej ramki
	vectors[9].x = 21;
	vectors[9].y = halfY - 19;
	vectors[10].x = w - 21;
	vectors[10].y = halfY - 19;
	vectors[11].x = w - 21;
	vectors[11].y = halfY + 19;
	vectors[12].x = 21;
	vectors[12].y = halfY + 19;
	vectors[13].x = 21;
	vectors[13].y = halfY - 19;
	//koordynaty paska postępu
	int rw = 22;
	vectors[14].x = rw;
	vectors[14].y = halfY;
	vectors[15].x = ((provider->audioProgress / 1.f) * (w - 44)) + rw;
	vectors[15].y = halfY;

	RECT textParcent;
	textParcent.left = 20;
	textParcent.right = w - 20;
	textParcent.top = halfY - 20;
	textParcent.bottom = halfY + 20;
	wxString txt = std::to_string((int)(provider->audioProgress * 100.f)) + L"%";
	//try{
	d3dLine->SetWidth(1);
	d3dLine->Begin();
	d3dLine->Draw(&vectors[4], 5, 0xFF00FFFF);
	d3dLine->Draw(&vectors[9], 5, 0xFFFFFFFF);
	d3dLine->End();
	d3dLine->SetWidth(37);
	d3dLine->Begin();
	d3dLine->Draw(&vectors[14], 2, 0xFFFFFFFF);
	d3dLine->End();

	DRAWOUTTEXT(d3dFontTahoma13, txt, textParcent, DT_CENTER | DT_VCENTER, 0xFFFFFFFF)
		//} catch (...){}
}

//////////////////////////
// Get selection position
void AudioDisplay::GetDialoguePos(int64_t &selStart, int64_t &selEnd, bool cap) {
	selStart = GetXAtMS(curStartMS);
	selEnd = GetXAtMS(curEndMS);

	if (cap) {
		if (selStart < 0) selStart = 0;
		if (selEnd < 0) selEnd = 0;
		if (selStart >= w) selStart = w - 1;
		if (selEnd >= w) selEnd = w - 1;
	}
}




//////////
// Update
void AudioDisplay::Update(bool moveToEnd) {
	if (blockUpdate) return;
	if (loaded) {
		if (Options.GetBool(AUDIO_AUTO_SCROLL))
			MakeDialogueVisible(false, moveToEnd);
		else//it is possible to change position before without refresh and refresh it here without redrawing spectrum
			UpdateImage(/*true*/);
	}
}


//////////////////////
// Recreate the image
void AudioDisplay::RecreateImage() {
	LastSize = wxSize(w, h);
	GetClientSize(&w, &h);
	h -= timelineHeight;
	//delete origImage;
	//origImage = NULL;
	UpdateImage(false);
}


/////////////////////////
// Make dialogue visible
void AudioDisplay::MakeDialogueVisible(bool force, bool moveToEnd) {
	// Variables
	int startShow = 0, endShow = 0;
	// In karaoke mode the syllable and as much as possible towards the end of the line should be shown

	GetTimesSelection(startShow, endShow, true);

	int startPos = GetSampleAtMS(startShow);
	int endPos = GetSampleAtMS(endShow);
	int startX = GetXAtMS(startShow);
	int endX = GetXAtMS(endShow);
	if (hasKara){
		if (startX < 50 || endX >(w - 100)) {
			UpdatePosition((startPos + endPos - w * samples) / 2, true);
		}
	}
	else if (force || (startX < 50 && endX < w) || (endX > w - 50 && startX > 0)) {
		if ((startX < 50) || (endX >= w - 50)) {

			if (moveToEnd && (endX >= w - 50 || endX < 50)){
				// Make sure the right edge of the selection is at least 50 pixels from the edge of the display
				UpdatePosition(endPos - ((w - 50) * samples), true);
			}
			else if (!moveToEnd){
				// Make sure the left edge of the selection is at least 50 pixels from the edge of the display
				UpdatePosition(startPos - 50 * samples, true);
			}
		}
		else {
			// Otherwise center the selection in display
			UpdatePosition((startPos + endPos - w * samples) / 2, true);
		}
	}

	// Update
	UpdateImage();
}


////////////////
// Set position
void AudioDisplay::SetPosition(int pos) {
	Position = pos;
	PositionSample = pos * samples;
	UpdateImage();
}


///////////////////
// Update position
void AudioDisplay::UpdatePosition(int pos, bool IsSample) {
	// Safeguards
	if (!provider) return;
	if (IsSample) pos /= samples;
	int len = provider->GetNumSamples() / samples;
	if (pos < 0) pos = 0;
	if (pos >= len) pos = len - 1;
	// Set
	Position = pos;
	PositionSample = pos*samples;
	UpdateScrollbar();
}


/////////////////////////////
// Set samples in percentage
// Note: aka Horizontal Zoom
void AudioDisplay::SetSamplesPercent(int percent, bool update, float pivot) {
	// Calculate
	if (percent < 1) percent = 1;
	if (percent > 100) percent = 100;
	if (samplesPercent == percent) return;
	samplesPercent = percent;

	// Update
	if (update) {
		// Center scroll
		int oldSamples = samples;
		UpdateSamples();
		PositionSample += int64_t((oldSamples - samples)*w*pivot);
		if (PositionSample < 0) PositionSample = 0;

		// Update
		//UpdateSamples();
		UpdateImage();
		UpdateScrollbar();

		//Refresh(false);
	}
}


//////////////////
// Update samples
void AudioDisplay::UpdateSamples() {
	// Set samples
	if (!provider) return;
	if (w) {
		int64_t totalSamples = provider->GetNumSamples();
		int total = totalSamples / w;
		int max = 5760000 / w;	// 2 minutes at 48 kHz maximum
		if (total > max) total = max;
		int min = 8;
		if (total < min) total = min;
		int range = total - min;
		samples = int(range*pow(samplesPercent / 100.0, 3) + min);

		// Set position
		int length = w * samples;
		if (PositionSample + length > totalSamples) {
			PositionSample = totalSamples - length;
			if (PositionSample < 0) PositionSample = 0;
			if (samples) Position = PositionSample / samples;
		}
	}
}


/////////////
// Set scale
void AudioDisplay::SetScale(float _scale) {
	if (scale == _scale) return;
	scale = _scale;
	UpdateImage();
}


//////////////////
// Load from file
void AudioDisplay::SetFile(wxString file, bool fromvideo) {
	// Unload
	if (player) {
		try {
			if (player->IsPlaying()){
				Stop();
			}
			try {
				player->CloseStream();
			}
			catch (const wxChar *e) {
				wxLogError(e);
			}
			if (ownProvider && provider){ delete provider; provider = NULL; }
			delete player;
			if (spectrumRenderer){ delete spectrumRenderer; spectrumRenderer = NULL; }

			player = NULL;
			Reset();
			loaded = false;
		}
		catch (wxString e) {
			wxLogError(e);
		}
		catch (const wxChar *e) {
			wxLogError(e);
		}
		catch (...) {
			wxLogError(_T("Unknown error unloading audio"));
		}
	}
	// Load
	if (!file.IsEmpty()) {
		try {
			// Get provider
			TabPanel *pan = ((TabPanel*)box->GetGrandParent());
			VideoCtrl *vb = pan->Video;
			VideoFfmpeg *FFMS2 = vb->GetFFMS2();
			kainoteApp *Kaia = (kainoteApp*)wxTheApp;
			bool success = true;
			if (FFMS2 && fromvideo){
				provider = FFMS2;
				ownProvider = (provider->videosource == NULL);
				if (ownProvider){ FFMS2 = NULL; }
			}
			else{
				provider = new VideoFfmpeg(file, 0, Kaia->Frame, &success);
				if (!success || provider->SampleRate < 0) {
					delete provider; provider = 0;
					loaded = false; return;
				}
				//copy keyframes to not check if video is loaded

				if (FFMS2){
					provider->KeyFrames = FFMS2->KeyFrames;
					provider->Timecodes = FFMS2->Timecodes;
					provider->NumFrames = FFMS2->NumFrames;
					provider->fps = FFMS2->fps;
				}
				RendererVideo* renderer = vb->GetRenderer();
				ownProvider = true;
				renderer->SetAudioPlayer(this);
			}


			// Get player
			player = new DirectSoundPlayer2();
			player->SetProvider(provider);
			player->OpenStream();
			loaded = true;

			UpdateImage();
		}
		catch (const wxChar *e) {
			if (player) { delete player; player = 0; }
			if (provider) { delete provider; provider = 0; }
			wxLogError(e);
		}
		catch (wxString &err) {
			if (player) { delete player; player = 0; }
			if (provider) { delete provider; provider = 0; }
			KaiMessageBox(err, _T("Error loading audio"), wxICON_ERROR | wxOK);
		}
		catch (...) {
			if (player) { delete player; player = 0; }
			if (provider) { delete provider; provider = 0; }
			wxLogError(_T("Unknown error loading audio"));
		}
	}

	if (!loaded) return;

	assert(loaded == (provider != NULL));
	if (provider->audioNotInitialized){
		ProgressTimer.Start(50);
	}
	// Set default selection
	int n = grid->currentLine;
	SetDialogue(grid->GetDialogue(n), n);
}



////////////////////
// Update scrollbar
void AudioDisplay::UpdateScrollbar() {
	if (!provider) return;
	int page = w / 12;
	int len = provider->GetNumSamples() / samples / 12;
	Position = (PositionSample / samples);
	ScrollBar->SetScrollbar(Position / 12, page, len, int(page * 0.7), true);
}


//////////////////////////////////////////////
// Gets the sample number at the x coordinate
int64_t AudioDisplay::GetSampleAtX(int x) {
	return (x + Position)*samples;
}


/////////////////////////////////////////////////
// Gets the x coordinate corresponding to sample
float AudioDisplay::GetXAtSample(int64_t n) {
	return samples ? ((double)n / (double)samples) - Position : 0;
}


/////////////////
// Get MS from X
int AudioDisplay::GetMSAtX(int64_t x) {
	return (PositionSample + (x*samples)) * 1000 / provider->GetSampleRate();
}


/////////////////
// Get X from MS
float AudioDisplay::GetXAtMS(int64_t ms) {
	return ((ms * provider->GetSampleRate() / 1000.0) - PositionSample) / (double)samples;
}


////////////////////
// Get MS At sample
int AudioDisplay::GetMSAtSample(int64_t x) {
	return x * 1000 / provider->GetSampleRate();
}


////////////////////
// Get Sample at MS
int64_t AudioDisplay::GetSampleAtMS(int64_t ms) {
	return ms * provider->GetSampleRate() / 1000;
}


void AudioDisplay::ChangeOptions()
{
	ChangeColours();
	selWidth = Options.GetInt(AUDIO_LINE_BOUNDARIES_THICKNESS);
	shadeType = Options.GetInt(AUDIO_INACTIVE_LINES_DISPLAY_MODE);
	drawVideoPos = Options.GetBool(AUDIO_DRAW_VIDEO_POSITION);
	drawSelectionBackground = Options.GetBool(AUDIO_DRAW_SELECTION_BACKGROUND);
	spectrumOn = Options.GetBool(AUDIO_SPECTRUM_ON);
	drawBoundaryLines = Options.GetBool(AUDIO_DRAW_SECONDARY_LINES);
	drawKeyframes = Options.GetBool(AUDIO_DRAW_KEYFRAMES);

	keyframe = D3DCOLOR_FROM_WX(Options.GetColour(AUDIO_KEYFRAMES));
	background = D3DCOLOR_FROM_WX(Options.GetColour(AUDIO_BACKGROUND));
	selectionBackgroundModified = D3DCOLOR_FROM_WX(Options.GetColour(AUDIO_SELECTION_BACKGROUND_MODIFIED));
	selectionBackground = D3DCOLOR_FROM_WX(Options.GetColour(AUDIO_SELECTION_BACKGROUND));
	secondBondariesColor = D3DCOLOR_FROM_WX(Options.GetColour(AUDIO_SECONDS_BOUNDARIES));
	lineStartBondaryColor = D3DCOLOR_FROM_WX(Options.GetColour(AUDIO_LINE_BOUNDARY_START));
	lineEndBondaryColor = D3DCOLOR_FROM_WX(Options.GetColour(AUDIO_LINE_BOUNDARY_END));
	syllableBondaresColor = D3DCOLOR_FROM_WX(Options.GetColour(AUDIO_SYLLABLE_BOUNDARIES));
	syllableTextColor = D3DCOLOR_FROM_WX(Options.GetColour(AUDIO_SYLLABLE_TEXT));
	lineBondaryMark = D3DCOLOR_FROM_WX(Options.GetColour(AUDIO_LINE_BOUNDARY_MARK));
	AudioCursor = D3DCOLOR_FROM_WX(Options.GetColour(AUDIO_PLAY_CURSOR));
	waveformInactive = D3DCOLOR_FROM_WX(Options.GetColour(AUDIO_WAVEFORM_INACTIVE));
	boundaryInactiveLine = D3DCOLOR_FROM_WX(Options.GetColour(AUDIO_LINE_BOUNDARY_INACTIVE_LINE));
	inactiveLinesBackground = D3DCOLOR_FROM_WX(Options.GetColour(AUDIO_INACTIVE_LINES_BACKGROUND));
	timescaleBackground = D3DCOLOR_FROM_WX(Options.GetColour(WINDOW_BACKGROUND));
	//timescale3dLight = D3DCOLOR_FROM_WX(wxSystemSettings::GetColour(wxSYS_COLOUR_3DLIGHT));
	timescaleText = D3DCOLOR_FROM_WX(Options.GetColour(WINDOW_TEXT));
	waveform = D3DCOLOR_FROM_WX(Options.GetColour(AUDIO_WAVEFORM));
	waveformModified = D3DCOLOR_FROM_WX(Options.GetColour(AUDIO_WAVEFORM_MODIFIED));
	waveformSelected = D3DCOLOR_FROM_WX(Options.GetColour(AUDIO_WAVEFORM_SELECTED));
}

////////
// Play
void AudioDisplay::Play(int start, int end, bool pause) {
	//Stop();
	if (pause && Notebook::GetTab()->Video->GetState() == Playing){ Notebook::GetTab()->Video->Pause(); }

	// Check provider
	if (!provider) {
		return;
	}

	// Set defaults
	playingToEnd = end < 0;
	int64_t num_samples = provider->GetNumSamples();
	start = GetSampleAtMS(start);
	if (end != -1) end = GetSampleAtMS(end);
	else end = num_samples - 1;

	// Sanity checking
	if (start < 0) start = 0;
	if (start >= num_samples) start = num_samples - 1;
	if (end >= num_samples) end = num_samples - 1;
	if (end < start) end = start;

	// Redraw the image to avoid any junk left over from mouse movements etc
	// See issue #598
	UpdateImage(true);

	// Call play
	player->Play(start, end - start);
	if (!UpdateTimer.IsRunning()) UpdateTimer.Start(10);
	//CreateTimerQueueTimer(&UpdateTimerHandle, 0, WAITORTIMERCALLBACK(OnUpdateTimer), this, 1, 16, WT_EXECUTELONGFUNCTION);
}


////////
// Stop
void AudioDisplay::Stop(bool stopVideo) {
	if (stopVideo && Notebook::GetTab()->Video->GetState() == Playing){ Notebook::GetTab()->Video->Pause(); }
	else if (player) {
		player->Stop();
		if (UpdateTimer.IsRunning()) UpdateTimer.Stop();
		//DeleteTimerQueueTimer(NULL, UpdateTimerHandle, INVALID_HANDLE_VALUE);
		cursorPaint = false;
		Refresh(false);
	}

}


void AudioDisplay::ChangePosition(int time, bool center /*= true*/)
{
	int64_t samplepos = GetSampleAtMS(time);
	if (center)
		samplepos = (samplepos / samples) - (w / 2);

	UpdatePosition(samplepos, !center);
	UpdateImage();
}

///////////////////////////
// Get samples of dialogue
void AudioDisplay::GetTimesDialogue(int &start, int &end) {
	start = dialogue->Start.mstime;
	end = dialogue->End.mstime;
}


////////////////////////////
// Get samples of selection
void AudioDisplay::GetTimesSelection(int &start, int &end, bool rangeEnd /*= false*/, bool ignoreKara /*= false*/) {
	if (hasKara && !ignoreKara){
		whichsyl = MID(0, whichsyl, (int)karaoke->syls.size() - 1);
		if (rangeEnd)
			karaoke->GetSylVisibleTimes(whichsyl, start, end);
		else
			karaoke->GetSylTimes(whichsyl, start, end);
	}
	else{
		start = curStartMS;
		end = curEndMS;
	}
}


/////////////////////////////
// Set the current selection
void AudioDisplay::SetSelection(int start, int end) {
	curStartMS = start;
	curEndMS = end;
	Update();
}


////////////////
// Set dialogue
void AudioDisplay::SetDialogue(Dialogue *diag, int n, bool moveToEnd) {
	// Actual parameters
	// Set variables
	bool isNextLine = (line_n + 1 == n);
	line_n = n;
	//dialog jest tylko odczytywany, jest on własnością editboxa, usuwać go nie można.
	dialogue = diag;
	NeedCommit = false;
	whichsyl = 0;
	// Set flags
	// Set times
	if (Options.GetBool(AUDIO_GRAB_TIMES_ON_SELECT)) {
		int s = dialogue->Start.mstime;
		int e = dialogue->End.mstime;

		// Never do it for 0:00:00.00->0:00:00.00 lines
		if (s != 0 || e != 0) {
			curStartMS = s;
			curEndMS = e;
		}
		else{
			size_t prevPos = grid->GetKeyFromPosition(line_n, -1);
			if (isNextLine && line_n != prevPos){
				Dialogue *pdial = grid->GetDialogue(prevPos);
				curStartMS = pdial->End.mstime;
			}
			else
				curStartMS = s;
			curEndMS = curStartMS + 5000;
		}
	}


	// Reset karaoke pos

	if (hasKara){
		//whichsyl=0;
		karaoke->Split();

	}

	// Update	
	Update(moveToEnd);
}


//////////////////
// Commit changes
void AudioDisplay::CommitChanges(bool nextLine, bool Save, bool moveToEnd) {
	// Loaded?
	if (!loaded) return;

	if (Save){ NeedCommit = false; }

	// Update dialogues
	blockUpdate = true;
	STime gtime = STime(Edit->line->Start);
	gtime.NewTime(curStartMS);
	Edit->StartEdit->SetTime(gtime, true, 1);
	gtime.NewTime(curEndMS);
	Edit->EndEdit->SetTime(gtime, true, 2);
	gtime.NewTime(curEndMS - curStartMS);
	Edit->DurEdit->SetTime(gtime, true, 1);
	if (Save){
		Edit->Send(AUDIO_CHANGE_TIME, nextLine);
		if (!nextLine){ Edit->UpdateChars(); }
		VideoCtrl *vb = ((TabPanel *)Edit->GetParent())->Video;
		if (vb && vb->GetState() != None)
			vb->RefreshTime();
	}
	blockUpdate = false;

	Update(moveToEnd);
}


////////////
// Add lead
void AudioDisplay::AddLead(bool in, bool out) {
	// Lead in
	if (in) {
		curStartMS -= Options.GetInt(AUDIO_LEAD_IN_VALUE);
		if (curStartMS < 0) curStartMS = 0;
	}

	// Lead out
	if (out) {
		curEndMS += Options.GetInt(AUDIO_LEAD_OUT_VALUE);
	}

	// Set changes
	UpdateTimeEditCtrls();
	NeedCommit = true;
	if (Options.GetBool(AUDIO_AUTO_COMMIT)) CommitChanges();
	Update();
}




/////////
// Paint
void AudioDisplay::OnPaint(wxPaintEvent& event) {
	//if (w == 0 || h == 0) return;
	DoUpdateImage();
}


///////////////
// Mouse event
void AudioDisplay::OnMouseEvent(wxMouseEvent& event) {
	// Get x,y
	int64_t x = event.GetX();
	int64_t y = event.GetY();

	bool shiftDown = event.m_shiftDown;
	if (box->arrows){ box->SetCursor(wxCURSOR_ARROW); box->arrows = false; }
	// Leaving event
	if (event.Leaving()) {
		if (!player->IsPlaying())
			cursorPaint = false;
		UpdateImage(true);
		return;
	}

	if (!player || !provider) {
		return;
	}

	// Is inside?

	bool onScale = false;
	if (x >= 0 && y >= 0 && x < w) {
		if (y < h) {
			inside = true;

			// Get focus
			if (Options.GetBool(AUDIO_AUTO_FOCUS) && wxWindow::FindFocus() != this) SetFocus();
		}
		else if (y < h + timelineHeight){
			onScale = true;
			if (!player->IsPlaying())
				cursorPaint = false;
		}
		if (inside && onScale){ UpdateImage(true); inside = false; }
	}
	else{ inside = false; }

	// All buttons click
	if (event.ButtonDown()) {
		SetFocus();
		if (!player->IsPlaying())
			cursorPaint = false;
	}

	// Buttons
	bool leftDown = event.LeftDown() || event.LeftDClick();
	bool rightDown = event.RightDown() || event.RightDClick();
	bool buttonDown = leftDown || rightDown;
	bool buttonUP = event.LeftUp() || event.RightUp();
	bool middleDown = event.MiddleDown();
	bool updated = false;
	syll = -1;
	// Click type

	if (buttonUP && holding) {
		holding = false;
		if (HasCapture()) ReleaseMouse();
	}


	if (((leftDown && event.GetModifiers() == wxMOD_CONTROL) || middleDown) && !onScale)
	{
		int pos = GetMSAtX(x);
		Notebook::GetTab()->Video->Seek(pos);
		UpdateImage(true);

	}

	if (buttonDown && !holding) {
		holding = true;
		CaptureMouse();
	}


	// Mouse wheel
	if (event.GetWheelRotation() != 0) {
		// Zoom or scroll?
		bool zoom = shiftDown;
		if (Options.GetBool(AUDIO_WHEEL_DEFAULT_TO_ZOOM)) zoom = !zoom;
		if (event.GetModifiers() == wxMOD_CONTROL){
			int step = event.GetWheelRotation() / event.GetWheelDelta();

			int pos = box->VerticalZoom->GetValue() + step;
			box->VerticalZoom->SetValue(pos);
			float value = pow(float(pos) / 50.0f, 3);
			SetScale(value);
			if (box->VerticalLink->GetValue()) {
				player->SetVolume(value);
				box->VolumeBar->SetThumbPosition(box->VerticalZoom->GetThumbPosition());
				Options.SetInt(AUDIO_VOLUME, pos);
			}
			Options.SetInt(AUDIO_VERTICAL_ZOOM, pos);
			Options.SaveAudioOpts();
		}
		// Zoom
		else if (zoom) {

			int step = event.GetWheelRotation() / event.GetWheelDelta();

			int value = box->HorizontalZoom->GetValue() - step;
			box->HorizontalZoom->SetValue(value);
			SetSamplesPercent(value, true, float(x) / float(w));
		}

		// Scroll
		else {
			int step = -event.GetWheelRotation() * w / 360;
			UpdatePosition(Position + step, false);
			UpdateImage();
		}
	}


	// Scale dragging
	if ((hold == 0 && onScale) || draggingScale) {
		SetCursor(wxNullCursor);
		if (rightDown)
		{
			SetMark(GetMSAtX(x));
			UpdateImage(true);
			return;
		}
		if (leftDown) {
			lastDragX = x;
			draggingScale = true;
		}
		else if (holding) {
			int delta = lastDragX - x;
			lastDragX = x;
			UpdatePosition(Position + delta);
			curpos = GetXAtSample(player->GetCurrentPosition());
			UpdateImage();
			//Refresh(false);
			return;
		}
		else draggingScale = false;
	}

	// Outside
	if (!inside && hold == 0) return;


	// Timing
	if (hasSel && !(event.ControlDown() && !event.AltDown() && hold == 0)) {

		letter = -1;
		// znacznik
		if (hold == 0) {
			if (hasMark && abs64(x - selMark) < 6) {
				wxCursor cursor(wxCURSOR_SIZEWE);
				SetCursor(cursor);
				defCursor = false;
				if (buttonDown) {
					hold = 4;
				}
			}





			//start
			else if (abs64(x - selStart) < 6) {
				wxCursor cursor(wxCURSOR_SIZEWE);
				SetCursor(cursor);
				defCursor = false;
				if (buttonDown) {
					hold = 1;
				}
			}


			//żółte linie karaoke
			else if (hasKara && y > 20)
			{
				if (!karaoke->CheckIfOver(x, &Grabbed)){
					int tmpsyl = -1;
					bool hasSyl = karaoke->GetSylAtX(x, &tmpsyl);
					if (Options.GetBool(AUDIO_KARAOKE_MOVE_ON_CLICK) && hasSyl &&
						!(tmpsyl<whichsyl - 1 || tmpsyl>whichsyl + 1) && (leftDown || rightDown)){
						Grabbed = (tmpsyl < whichsyl) ? whichsyl - 1 : whichsyl;
						hold = 5;
					}
					else if (leftDown && tmpsyl >= 0){
						whichsyl = tmpsyl;
						updated = true;
					}
					else if (abs64(x - selEnd) < 6){
						wxCursor cursor(wxCURSOR_SIZEWE);
						SetCursor(cursor);
						defCursor = false;
						if (leftDown){
							hold = 2;
						}
						else if (rightDown){
							Grabbed = whichsyl = karaoke->syltimes.size() - 1;
							hold = 5;
						}
						return;
					}
					if (!defCursor){ SetCursor(wxNullCursor); defCursor = true; }
				}
				else{

					wxCursor cursor(wxCURSOR_SIZEWE);
					SetCursor(cursor);
					defCursor = false;
					if (middleDown || (shiftDown && leftDown)){
						karaoke->Join(Grabbed);
						Commit();
						return;
					}

					if (buttonDown){ hold = 5; }
				}



			}
			// litery sylab
			else if (hasKara && karaoke->GetLetterAtX(x, &syll, &letter))
			{
				if (leftDown){
					if (karaoke->SplitSyl(syll, letter)){
						whichsyl = syll;
						Commit();
					}
				}


				if (!defCursor){ SetCursor(wxNullCursor); defCursor = true; }
			}

			// Grab end
			else if (abs64(x - selEnd) < 6) {
				wxCursor cursor(wxCURSOR_SIZEWE);
				SetCursor(cursor);
				defCursor = false;
				if (buttonDown) {
					hold = 2;
				}
			}

			// Dragging nothing, time from scratch
			else if (buttonDown && !hasKara) {
				if (leftDown) hold = 3;
				else hold = 2;
				lastX = x;

			}

			// restoring cursor
			else if (!defCursor){ SetCursor(wxNullCursor); defCursor = true; }

		}

		// Drag start/end
		if (hold != 0){

			// Dragging
			// Release
			if (buttonUP) {
				// Prevent negative times
				if (Grabbed == -1)
				{
					curStartMS = MAX(0, curStartMS);
					curEndMS = MAX(0, curEndMS);
					curStartMS = ZEROIT(curStartMS);
					curEndMS = ZEROIT(curEndMS);
					selStart = MAX(0, selStart);
					selEnd = MAX(0, selEnd);
					int nn = grid->currentLine;
					//automatic setting times of previous or next line(right alt + left click or drag)
					if (hold == 2 && /*event.ControlDown() && */event.AltDown()){
						Dialogue *dialc = grid->CopyDialogueWithOffset(nn, 1);
						if (dialc){
							dialc->Start.NewTime(curEndMS);
							if (dialc->End < dialc->Start){ dialc->End.NewTime(curEndMS + 5000); }
						}
					}
					else if (hold == 1 && /*event.ControlDown() && */event.AltDown()){
						Dialogue *dialc = grid->CopyDialogueWithOffset(nn, -1);
						if (dialc){
							dialc->End.NewTime(curStartMS);
							if (dialc->End < dialc->Start){ dialc->Start.NewTime(curStartMS - 5000); }
						}
					}
				}


				if (hasKara && Grabbed != -1)
				{
					int newpos = ZEROIT(GetMSAtX(x));
					int prev = (Grabbed == 0) ? curStartMS : karaoke->syltimes[Grabbed - 1];
					int next = (Grabbed == (int)karaoke->syls.size() - 1) ? curEndMS : karaoke->syltimes[Grabbed + 1];
					karaoke->syltimes[Grabbed] = MID(prev, newpos, next);
					whichsyl = Grabbed;
				}
				if (hold != 4){
					Commit(hold == 2);
				}

				// Update stuff

				hold = 0;

				return;
			}
			else {
				//drag timing change cursor
				if (hold == 4) {

					curMarkMS = GetMSAtX(x);
					updated = true;
				}
				// Drag from nothing or straight timing
				if (hold == 3) {

					if (leftDown)
						curStartMS = GetBoundarySnap(GetMSAtX(x), 16, event.ShiftDown(), true);
					else if (rightDown)
						curEndMS = GetMSAtX(x);

					updated = true;
					NeedCommit = true;

					if (leftDown && abs((long)(x - lastX)) > Options.GetInt(AUDIO_START_DRAG_SENSITIVITY)) {
						selStart = lastX;
						selEnd = x;
						curStartMS = GetBoundarySnap(GetMSAtX(lastX), 16, event.ShiftDown(), true);
						curEndMS = GetMSAtX(x);
						hold = 2;
					}

				}

				// Drag start
				if (hold == 1) {
					// Set new value
					if (x != selStart) {
						int snapped = GetBoundarySnap(GetMSAtX(x), 16, event.ShiftDown(), true);
						selStart = GetXAtMS(snapped);
						/*if (selStart > selEnd) {
						int temp = selStart;
						selStart = selEnd;
						selEnd = temp;
						hold = 2;
						curEndMS = snapped;
						snapped = GetMSAtX(selStart);
						}*/

						if (hasKara && (event.RightIsDown() || rightDown)){
							int sizes = karaoke->syls.size() - 1;
							int addtime = snapped - curStartMS;
							for (int i = 0; i < sizes; i++)
							{
								int time = karaoke->syltimes[i] + addtime;
								karaoke->syltimes[i] = ZEROIT(time);
								if (karaoke->syltimes[i] > karaoke->syltimes[i + 1]){
									karaoke->syltimes[i] = karaoke->syltimes[i + 1];
								}
							}
							//curEndMS=karaoke->syltimes[sizes-1];
						}
						curStartMS = snapped;
						updated = true;
						NeedCommit = true;
					}
				}

				// Drag end
				if (hold == 2) {
					// Set new value
					if (x != selEnd) {
						int snapped = GetBoundarySnap(GetMSAtX(x), 16, event.ShiftDown(), false);
						selEnd = GetXAtMS(snapped);

						curEndMS = snapped;

						updated = true;
						NeedCommit = true;
					}
				}
				//drag karaoke
				if (hold == 5 && Grabbed != -1){
					int newpos = ZEROIT(GetMSAtX(x));
					int sizes = karaoke->syls.size() - 1;
					int prev = (Grabbed == 0) ? curStartMS : karaoke->syltimes[Grabbed - 1];
					int next = (Grabbed == sizes) ? 2147483646 : karaoke->syltimes[Grabbed + 1];
					int prevpos = karaoke->syltimes[Grabbed];
					karaoke->syltimes[Grabbed] = MID(prev, newpos, next);
					if (Grabbed == sizes && (leftDown || event.LeftIsDown())){ curEndMS = karaoke->syltimes[Grabbed]; }
					//prawy przycisk myszy
					else if ((rightDown || event.RightIsDown()) && Grabbed != sizes){
						int addtime = karaoke->syltimes[Grabbed] - prevpos;

						for (int i = Grabbed + 1; i < (int)karaoke->syls.size() - 1; i++)
						{
							int time = karaoke->syltimes[i];
							time += addtime;
							time = ZEROIT(time);
							karaoke->syltimes[i] = time;
							if (karaoke->syltimes[i] > karaoke->syltimes[i + 1]){
								karaoke->syltimes[i] = karaoke->syltimes[i + 1];
							}
						}

						curEndMS = karaoke->syltimes[sizes];
					}
					updated = true;

				}


			}

		}
		// Update stuff
		if (updated) {

			if (!playingToEnd) {
				int64_t slend;
				if (hasKara && Grabbed >= 0){
					slend = GetSampleAtMS(karaoke->syltimes[Grabbed]);
				}
				else{
					slend = GetSampleAtX(selEnd);
				}
				player->SetEndPosition(slend);
			}

			UpdateImage(true);

			return;
		}

	}

	// Not holding
	else {
		hold = 0;
	}

	// Right click
	if (rightDown && hasKara) {
		SetFocus();
		int syl;
		if (karaoke->GetSylAtX(x, &syl)) {
			int start, end;
			karaoke->GetSylTimes(syl, start, end);
			Play(start, end);
			whichsyl = syl;
		}

	}

	// Middle click
	if (event.MiddleDClick()) {
		SetFocus();
		int start = 0, end = 0;
		GetTimesSelection(start, end);
		Play(start, end);
	}

	// Cursor drawing
	if (player && !player->IsPlaying() && event.Moving()) {

		if (inside){
			if (hasKara && letter != -1){
				cursorPaint = false;
				//needImageUpdateWeak = true;
				//needImageUpdate=true;
				//curpos = x;
				//Refresh(false);
				//return;
			}
			else{
				cursorPaint = true;
			}

			// Draw cursor
			//needImageUpdateWeak = true;
			//needImageUpdate=true;
			curpos = x;
			//Refresh(false);
			UpdateImage(true);
		}


	}
}


////////////////////////
// Get snap to boundary
int AudioDisplay::GetBoundarySnap(int ms, int rangeX, bool shiftHeld, bool start, bool keysnap) {
	// Range?
	if (rangeX <= 0) return ms;

	// Convert range into miliseconds
	int rangeMS = rangeX * samples * 1000 / provider->GetSampleRate();
	//int halfframe=Notebook::GetTab()->Video->avtpf/2;
	//VideoCtrl *vb = (VideoCtrl*)box->GetGrandParent();
	// Keyframe boundaries
	wxArrayInt boundaries;

	bool snapKey = Options.GetBool(AUDIO_SNAP_TO_KEYFRAMES);
	if (shiftHeld) snapKey = !snapKey;
	if (snapKey && provider->KeyFrames.size() > 0 && drawKeyframes) {
		int64_t keyMS;

		for (unsigned int i = 0; i < provider->KeyFrames.Count(); i++) {
			keyMS = provider->KeyFrames[i];
			int keyX = GetXAtMS(keyMS);
			if (keyX >= 0 && keyX < w) {
				int frameTime = 0;
				if (provider->Timecodes.size() < 1){
					//there is nothing to do when video is not loaded
					//put half of frame 23.976FPS
					frameTime = keyMS - 21;
				}
				else{
					int frame = provider->GetFramefromMS(keyMS);
					int prevFrameTime = provider->GetMSfromFrame(frame - 1);
					frameTime = keyMS + ((prevFrameTime - keyMS) / 2);
				}
				boundaries.Add(ZEROIT(frameTime/*-halfframe*/));
			}
		}
	}

	// Other subtitles' boundaries
	bool snapLines = Options.GetBool(AUDIO_SNAP_TO_OTHER_LINES);
	if (shiftHeld) snapLines = !snapLines;
	if (snapLines && (shadeType == 1 || shadeType == 2)) {
		Dialogue *shade;
		int shadeX1, shadeX2;
		int shadeFrom, shadeTo;

		// Get range
		if (shadeType == 1) {
			shadeFrom = grid->GetKeyFromPosition(line_n, -1);
			shadeTo = grid->GetKeyFromPosition(line_n, 1);
		}
		else {
			shadeFrom = 0;
			shadeTo = grid->GetCount() - 1;
		}

		for (int j = shadeFrom; j <= shadeTo; j++) {
			if (j == line_n) continue;
			shade = grid->GetDialogue(j);
			if (!shade->isVisible)
				continue;

			// Get coordinates
			shadeX1 = GetXAtMS(shade->Start.mstime);
			shadeX2 = GetXAtMS(shade->End.mstime);
			if (shadeX1 >= 0 && shadeX1 < w) boundaries.Add(shade->Start.mstime);
			if (shadeX2 >= 0 && shadeX2 < w) boundaries.Add(shade->End.mstime);

		}
	}

	// See if ms falls within range of any of them
	int minDist = rangeMS + 1;
	int adist = minDist;
	int bestMS = ms;
	for (unsigned int i = 0; i < boundaries.Count(); i++) {
		adist = abs(ms - boundaries[i]);
		if (adist < minDist) {
			if (keysnap && adist < 10){ continue; }
			bestMS = boundaries[i];
			minDist = adist;
		}
	}

	// Return best match
	return ZEROIT(bestMS);
}





void AudioDisplay::GetTextExtentPixel(const wxString &text, int *x, int *y)
{
	RECT rcRect = { 0, 0, 0, 0 };
	d3dFontVerdana11->DrawTextW(NULL, text.wchar_str(), -1, &rcRect, DT_CALCRECT, 0xFF000000);
	*x = rcRect.right - rcRect.left;
	*y = rcRect.bottom - rcRect.top;
	if (text.StartsWith(L" "))
		*x += 4;
	if (text.EndsWith(L" "))
		*x += 4;
}

//////////////
// Size event
void AudioDisplay::OnSize(wxSizeEvent &event) {
	// Set size
	LastSize = wxSize(w, h);
	GetClientSize(&w, &h);
	h -= timelineHeight;
	if (LastSize.x == w && LastSize.y == h)
		return;

	// Update image
	UpdateSamples();
	if (samples) {
		UpdatePosition(PositionSample / samples);
	}
	UpdateImage(false, true);

	// Update scrollbar
	UpdateScrollbar();
}


///////////////
// Timer event
void AudioDisplay::OnUpdateTimer(wxTimerEvent &event) {
	//VOID CALLBACK AudioDisplay::OnUpdateTimer(PVOID pointer, BOOLEAN timerOrWaitFaired){

	//wxMutexLocker lock(mutex);
	//AudioDisplay * ad = (AudioDisplay *)pointer;

	// Draw cursor
	curpos = -1;
	if (player->IsPlaying()) {
		cursorPaint = true;
		needImageUpdateWeak = true;
		needImageUpdate = true;
		int64_t curPos = player->GetCurrentPosition();
		if (curPos > player->GetStartPosition() && curPos < player->GetEndPosition()) {
			// Scroll if needed
			int posX = GetXAtSample(curPos);
			bool fullDraw = false;
			bool centerLock = false;
			bool scrollToCursor = Options.GetBool(AUDIO_LOCK_SCROLL_ON_CURSOR);
			if (centerLock) {
				int goTo = MAX(0, curPos - w * samples / 2);
				if (goTo >= 0) {
					UpdatePosition(goTo, true);
					UpdateImage(false, true);
					fullDraw = true;
				}
			}
			else {
				if (scrollToCursor) {
					if (posX < 80 || posX > w - 80) {
						int goTo = MAX(0, curPos - 80 * samples);
						if (goTo >= 0) {
							UpdatePosition(goTo, true);
							UpdateImage(false, true);
							fullDraw = true;
						}
					}
				}
			}

			// Draw cursor
			curpos = GetXAtSample(curPos);
			if (curpos >= 0.f && curpos < GetClientSize().GetWidth()) {

				//Refresh(false);
				//UpdateImage(false, true);
				DoUpdateImage();
			}
			else if (cursorPaint){
				cursorPaint = false;
				//Refresh(false);
				//UpdateImage(false, true);
				DoUpdateImage();
			}
		}
		else {

			cursorPaint = false;
			//Refresh(false);
			//UpdateImage(false, true);
			DoUpdateImage();
			if (curPos > player->GetEndPosition() + 8192) {
				player->Stop();
				if (UpdateTimer.IsRunning()) UpdateTimer.Stop();
				//DeleteTimerQueueTimer(NULL, UpdateTimerHandle, INVALID_HANDLE_VALUE);
				//DeleteTimerQueueTimer(NULL, UpdateTimerHandle, NULL);
			}
		}

	}

	// Restore background
	else {
		cursorPaint = false;
		//needImageUpdate=true;
		//cursorPaint=false;
		//Refresh(false);
		//KaiLog("Uuu update timer was not stopped");
		//if (UpdateTimer.IsRunning()) 
		//UpdateTimer.Stop();
	}
	oldCurPos = curpos;
	if (oldCurPos < 0) oldCurPos = 0;
}




///////////////
// Change line
void AudioDisplay::ChangeLine(int delta, bool block) {

	// Get next line number and make sure it's within bounds

	if (line_n == 0 && delta < 0 || line_n == grid->GetCount() - 1 && delta > 0) { return; }
	int next = grid->GetKeyFromPosition(line_n, delta);
	// Set stuff
	grid->SelectRow(next);
	grid->MakeVisible(next);
	Edit->SetLine(next);

}


void AudioDisplay::SetMark(int time)
{
	curMarkMS = time;
	if (!hasMark){
		hasMark = true;
		TabPanel *panel = (TabPanel*)grid->GetParent();
		panel->ShiftTimes->Contents();
	}
	else
		hasMark = true;
}

////////
// Next
void AudioDisplay::Next(bool play) {
	// Karaoke
	if (hasKara){
		whichsyl++;
		if (whichsyl >= (int)karaoke->syls.size()){ whichsyl = 0; ChangeLine(1); }
	}
	else{
		ChangeLine(1);
	}

	if (play){
		int start = 0, end = 0;
		GetTimesSelection(start, end);
		Play(start, end);
	}


}


////////////
// Previous
void AudioDisplay::Prev(bool play) {
	// Karaoke
	if (hasKara && play){
		whichsyl--;
		if (whichsyl < 0){ ChangeLine(-1); whichsyl = karaoke->syls.size() - 1; MakeDialogueVisible(); }

	}
	else{
		//if(Notebook::GetTab()->Video->GetState()==Playing){Notebook::GetTab()->Video->Pause();}
		ChangeLine(-1);
	}

	if (play) {
		int start = 0, end = 0;
		GetTimesSelection(start, end);
		Play(start, end);
	}
}



////////////////
// Focus events
void AudioDisplay::OnGetFocus(wxFocusEvent &event) {
	if (!hasFocus) {
		hasFocus = true;
		UpdateImage(true);
	}
}

void AudioDisplay::OnLoseFocus(wxFocusEvent &event) {
	//if(HasCapture()){ReleaseMouse();}
	if (hasFocus && loaded) {
		hasFocus = false;
		UpdateImage(true);
		//Refresh(false);
	}
}


//////////////////////////////
// Update time edit controls
bool AudioDisplay::UpdateTimeEditCtrls() {
	// Make sure this does NOT get short-circuit evaluation,
	// this is why binary OR instead of logical OR is used.
	// All three time edits must always be updated.

	Edit->StartEdit->SetTime(curStartMS, true, 1);
	Edit->EndEdit->SetTime(curEndMS, true, 2);
	return true;
}

void AudioDisplay::Commit(bool moveToEnd)
{
	bool autocommit = Options.GetBool(AUDIO_AUTO_COMMIT);
	if (hasKara){
		Edit->TextEdit->SetTextS(karaoke->GetText(), true, true, true);
	}

	if (autocommit) {
		CommitChanges(false, true, moveToEnd);
	}
	else{
		CommitChanges(false, false, moveToEnd);//UpdateImage(true);
		return;
	}
	if (!Options.GetBool(DISABLE_LIVE_VIDEO_EDITING)){ Edit->OnEdit(wxCommandEvent()); }
}

//////////////////
// Draw keyframes
void AudioDisplay::DrawKeyframes() {

	// Get min and max frames to care about
	int mintime = GetMSAtX(0);
	int maxtime = GetMSAtX(w);
	D3DXVECTOR2 v2[2];
	// Scan list
	d3dLine->Begin();
	for (size_t i = 0; i < provider->KeyFrames.size(); i++) {
		int cur = provider->KeyFrames[i];
		if (cur >= mintime && cur <= maxtime)
		{
			cur = ((cur - 20) / 10) * 10;
			//if(provider->Timecodes.size()<1){
			//	//cóż wiele zrobić nie możemy gdy nie mamy wideo;
			//	cur -= 21;
			//}else{
			//	int frame = provider->GetFramefromMS(cur);
			//	int prevFrameTime = provider->GetMSfromFrame(frame-1);
			//	cur = cur + ((prevFrameTime - cur) / 2);
			//}
			int x = GetXAtMS(cur);
			v2[0] = D3DXVECTOR2(x, 0);
			v2[1] = D3DXVECTOR2(x, h);
			d3dLine->Draw(v2, 2, keyframe);
		}
		if (cur > maxtime){ break; }

	}
	d3dLine->End();
}

bool AudioDisplay::SetFont(const wxFont &font)
{
	int fontSize = Options.GetInt(PROGRAM_FONT_SIZE);
	verdana11 = wxFont(fontSize + 1, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, L"Verdana");
	tahoma13 = wxFont(fontSize + 3, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, L"Tahoma");
	tahoma8 = *Options.GetFont(-1);
	int fh;
	GetTextExtent(L"#TWFfGH", NULL, &fh, NULL, NULL, &tahoma8);
	timelineHeight = fh + 8;
	UpdateTimer.SetOwner(this, Audio_Update_Timer);
	GetClientSize(&w, &h);
	h -= timelineHeight;
	UpdateImage(true);
	//test it!!!
	return true;
}
///////////////
// Event table
BEGIN_EVENT_TABLE(AudioDisplay, wxWindow)
EVT_MOUSE_EVENTS(AudioDisplay::OnMouseEvent)
EVT_PAINT(AudioDisplay::OnPaint)
EVT_SIZE(AudioDisplay::OnSize)
EVT_TIMER(Audio_Update_Timer, AudioDisplay::OnUpdateTimer)
EVT_SET_FOCUS(AudioDisplay::OnGetFocus)
EVT_KILL_FOCUS(AudioDisplay::OnLoseFocus)
EVT_MOUSE_CAPTURE_LOST(AudioDisplay::OnLostCapture)
EVT_ERASE_BACKGROUND(AudioDisplay::OnEraseBackground)
END_EVENT_TABLE()
