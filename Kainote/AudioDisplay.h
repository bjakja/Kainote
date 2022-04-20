//  Copyright (c) 2016-2022, Marcin Drob

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

//this code piervously was taken from Aegisub 2 it's rewritten by me almost all.

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
//#include <stdint.h>
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

	long long PositionSample;
	float scale;
	int samples;
	long long Position;
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
	long long selStart;
	long long selEnd;
	long long lineStart;
	long long lineEnd;
	long long selStartCap;
	long long selEndCap;
	long long selMark;

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
	bool drawSelectionBackground = false;
	bool drawKeyframes = false;
	bool drawBoundaryLines = false;
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
	void GetDialoguePos(long long &start, long long &end, bool cap);
	//void GetKaraokePos(long long &start, long long &end, bool cap);
	void UpdatePosition(int pos, bool IsSample = false);

	void DoUpdateImage(bool weak);

public:
	SubsGrid *grid = nullptr;
	EditBox *edit = nullptr;
	TabPanel *tab = nullptr;
	Provider *provider = nullptr;
	DirectSoundPlayer2 *player = nullptr;
	Karaoke *karaoke = nullptr;

	bool hasKara = false;
	bool karaAuto = false;
	bool NeedCommit = false;
	bool loaded = false;
	bool hasMark = false;
	bool isHidden = false;
	std::atomic<bool> stopPlayThread{ true };
	int curMarkMS = false;
	int Grabbed = false;
	int hold = false;

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

	long long GetSampleAtX(int x);
	float GetXAtSample(long long n);
	int GetMSAtX(long long x);
	float GetXAtMS(long long ms);
	int GetMSAtSample(long long x);
	long long GetSampleAtMS(long long ms);
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


