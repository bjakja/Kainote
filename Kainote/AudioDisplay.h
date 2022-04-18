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


#pragma once

///////////
// Headers
#include "AudioSpectrum.h"
#include "AudioPlayerDSound.h"
#include "SubsDialogue.h"
#include "KaraokeSplitting.h"
#include "KaiScrollbar.h"
#include <wx/window.h>
#include <wx/bitmap.h>
#include <stdint.h>


#include <d3d9.h>
#include <d3dx9.h>
#include <atomic>

//////////////
// Prototypes

class SubsGrid;
class AudioBox;
class EditBox;
class TabPanel;


/////////////////
// Display class
class AudioDisplay : public wxWindow {
	friend class Karaoke;
	friend class AudioBox;
private:
	int line_n = 0;
	Dialogue *dialogue = nullptr;


	AudioSpectrum *spectrumRenderer = nullptr;
	wxSize LastSize;
	volatile float curpos;

	int64_t PositionSample;
	float scale;
	int samples;
	int64_t Position;
	int samplesPercent;
	int oldCurPos;
	bool ownProvider;
	bool hasFocus;
	bool blockUpdate;
	bool inside;
	bool playingToEnd;
	bool defCursor;
	//bool needImageUpdate;
	bool needImageUpdateWeak;
	bool hasSel;
	bool diagUpdated;
	bool holding;
	bool draggingScale;
	int64_t selStart;
	int64_t selEnd;
	int64_t lineStart;
	int64_t lineEnd;
	int64_t selStartCap;
	int64_t selEndCap;
	int64_t selMark;

	int lastX;
	int lastDragX;
	int curStartMS;
	int curEndMS;

	int holdSyl;

	int *peak = nullptr;
	int *min = nullptr;

	wxCriticalSection mutex;
	wxCriticalSection mutexUpdate;
	int currentSyllable = 0;
	int currentCharacter = 0;
	int syllableHover = 0;
	LPDIRECT3D9 d3dObject = nullptr;
	LPDIRECT3DDEVICE9 d3dDevice = nullptr;
	LPDIRECT3DSURFACE9 backBuffer = nullptr;
	LPDIRECT3DSURFACE9 spectrumSurface = nullptr;

	ID3DXLine *d3dLine = nullptr;
	LPD3DXFONT d3dFontTahoma13 = nullptr;
	LPD3DXFONT d3dFontTahoma8 = nullptr;
	LPD3DXFONT d3dFontVerdana11 = nullptr;
	bool deviceLost = false;
	bool needToReset = false;
	//config
	int selWidth = 0;
	int shadeType = 0;
	int timelineHeight = 0;
	bool drawVideoPos = false;
	bool spectrumOn = false;
	bool drawSelectionBackground;
	bool drawKeyframes;
	bool drawBoundaryLines;
	wxFont verdana11;
	wxFont tahoma13;
	wxFont tahoma8;

	D3DCOLOR keyframe;
	D3DCOLOR background;
	D3DCOLOR selectionBackgroundModified;
	D3DCOLOR selectionBackground;
	D3DCOLOR waveform;
	D3DCOLOR secondBondariesColor;
	D3DCOLOR lineStartBondaryColor;
	D3DCOLOR lineEndBondaryColor;
	D3DCOLOR syllableBondaresColor;
	D3DCOLOR syllableTextColor;
	D3DCOLOR lineBondaryMark;
	D3DCOLOR AudioCursor;
	D3DCOLOR waveformInactive;
	D3DCOLOR boundaryInactiveLine;
	D3DCOLOR inactiveLinesBackground;
	D3DCOLOR timescaleBackground;
	//D3DCOLOR timescale3dLight;
	D3DCOLOR timescaleText;
	D3DCOLOR waveformModified;
	D3DCOLOR waveformSelected;

	void OnPaint(wxPaintEvent &event);
	void OnMouseEvent(wxMouseEvent &event);
	void OnSize(wxSizeEvent &event);
	//void OnUpdateTimer(wxTimerEvent &event);
	static unsigned int _stdcall OnUpdateTimer(PVOID pointer);
	void UpdateTimer();
	void OnGetFocus(wxFocusEvent &event);
	void OnLoseFocus(wxFocusEvent &event);
	void OnEraseBackground(wxEraseEvent &event){};
	void OnLostCapture(wxMouseCaptureLostEvent &evt){ if (HasCapture()){ ReleaseMouse(); Grabbed = -1; hold = 0; holding = false; } };

	bool InitDX(const wxSize &size);
	void ClearDX();
	void DrawDashedLine(D3DXVECTOR2 *vector, size_t vectorSize, D3DCOLOR fill, int dashLen = 3);
	void UpdateSamples();
	void Reset();
	void DrawTimescale();
	void DrawKeyframes();
	void DrawInactiveLines();
	void DrawWaveform(bool weak);
	void DrawSpectrum(bool weak);
	void DrawProgress();
	void GetDialoguePos(int64_t &start, int64_t &end, bool cap);
	//void GetKaraokePos(int64_t &start, int64_t &end, bool cap);
	void UpdatePosition(int pos, bool IsSample = false);

	void DoUpdateImage(bool weak);

public:
	SubsGrid *grid = nullptr;
	EditBox *Edit = nullptr;
	TabPanel *tab = nullptr;
	Provider *provider = nullptr;
	DirectSoundPlayer2 *player = nullptr;
	Karaoke *karaoke = nullptr;

	bool hasKara;
	bool karaAuto;
	bool NeedCommit;
	bool loaded;
	bool hasMark;
	bool isHidden = false;
	std::atomic<bool> stopPlayThread{ true };
	int curMarkMS;
	int Grabbed;
	int hold;

	int w, h;
	int w1 = 500;
	wxRect screenRect;
	AudioBox *box = nullptr;
	KaiScrollbar *ScrollBar = nullptr;
	wxTimer ProgressTimer;
	HANDLE UpdateTimerHandle = nullptr;
	HANDLE PlayEvent;
	HANDLE DestroyEvent;
	float lastProgress = -1.f;
	bool cursorPaint;

	AudioDisplay(wxWindow *parent);
	virtual ~AudioDisplay();

	void UpdateImage(bool weak = false, bool updateImmediately = false);
	void Update(bool moveToEnd = false);
	//void RecreateImage();
	void SetPosition(int pos);
	void SetSamplesPercent(int percent, bool update = true, float pivot = 0.5);
	void SetScale(float scale);
	void UpdateScrollbar();
	void SetDialogue(Dialogue *diag, int n = -1, bool moveToEnd = false);
	void MakeDialogueVisible(bool force = false, bool moveToEnd = false);
	void ChangeLine(int delta, bool block = false);
	void SetMark(int time);
	void Next(bool play = true);
	void Prev(bool play = true);

	bool UpdateTimeEditCtrls();
	void CommitChanges(bool nextLine = false, bool Save = true, bool moveToEnd = false);
	void Commit(bool moveToEnd = false);
	void AddLead(bool in, bool out);

	void SetFile(wxString file, bool fromvideo);
	//void Reload();

	void Play(int start, int end, bool pause = true);
	void Stop(bool stopVideo = true);

	int64_t GetSampleAtX(int x);
	float GetXAtSample(int64_t n);
	int GetMSAtX(int64_t x);
	float GetXAtMS(int64_t ms);
	int GetMSAtSample(int64_t x);
	int64_t GetSampleAtMS(int64_t ms);
	void ChangeOptions();
	void ChangeColours(){
		if (spectrumRenderer){
			spectrumRenderer->ChangeColours();
		}
		UpdateImage();
	}
	void ChangePosition(int time, bool center = true);
	void GetTimesDialogue(int &start, int &end);
	void GetTimesSelection(int &start, int &end, bool rangeEnd = false, bool ignoreKara = false);
	void SetSelection(int start, int end);
	int GetBoundarySnap(int x, int range, bool shiftHeld, bool start = true, bool keysnap = false, bool otherLines = true);
	void GetTextExtentPixel(const wxString &text, int *x, int *y);
	bool SetFont(const wxFont &font);
	DECLARE_EVENT_TABLE()
};


///////
// IDs
enum {
	Audio_Update_Timer = 1700
};


