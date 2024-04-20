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


#pragma once

//#include "VideoSlider.h"
#include "BitmapButton.h"
#include "RendererVideo.h"
//#include "VideoFullscreen.h"

#include "KaiTextCtrl.h"
#include "VideoBox.h"
#include "Provider.h"
#include "TabPanel.h"

class Provider;
class Fullscreen;
class VideoToolbar;
class VideoSlider;
class VolSlider;


enum PlaybackState;

class VideoBox : public wxWindow
{
	friend class RendererVideo;
	friend class RendererDirectShow;
	friend class RendererDummyVideo;
	friend class RendererFFMS2;
	friend class Fullscreen;
public:

	VideoBox(wxWindow *parent, const wxSize &size = wxDefaultSize);
	virtual ~VideoBox();
	bool Play();
	void PlayLine(int start, int end);
	bool Pause(bool burstbl = true);
	bool Stop();
	//custom FFMS2 -1 turn off, 0 Direct Show, 1 FFMS2
	bool LoadVideo(const wxString& fileName, int subsFlag, bool fullscreen = false, 
		bool changeAudio = true, int customFFMS2 = -1, bool dontPlayOnStart = false);
	

	bool Seek(int newPosition, bool starttime = true, bool refreshTime = true, bool reloadSubs = true, bool correct = true, bool asynchonize = true);
	int Tell();
	bool CalcSize(int *width, int *height, int wwidth = 0, int wheight = 0, bool setstatus = false, bool calcH = false);

	void NextFile(bool next = true);
	void SetFullscreen(int wmonitor = 0);
	void SetAspectRatio(float AR);
	void SetScaleAndZoom();
	void ChangeOnScreenResolution(TabPanel *tab);
	void OpenEditor(bool esc = true);
	void OnEndFile(wxCommandEvent& event);
	void OnPrew();
	void OnNext();
	void OnAccelerator(wxCommandEvent& event);
	//void OnVButton(wxCommandEvent& event);
	void OnVolume(wxScrollEvent& event);
	void OnSMinus();
	void OnSPlus();
	void ChangeStream();
	void RefreshTime();
	void NextChap();
	void PrevChap();
	void ConnectAcc(int id);
	//wxRect GetMonitorRect(int wmonitor);
	void ContextMenu(const wxPoint &pos);
	void OnMouseEvent(wxMouseEvent& event);
	void OnKeyPress(wxKeyEvent& event);
	void CaptureMouse();
	void ReleaseMouse();
	bool HasCapture();
	bool SetCursor(int cursorId);
	bool HasArrow();
	bool SetBackgroundColour(const wxColour &col);
	bool SetFont(const wxFont &font);
	void GetVideoSize(int *width, int *height);
	wxSize GetVideoSize();
	void GetFPSAndAspectRatio(float *FPS, float *AspectRatio, int *AspectRatioX, int *AspectRatioY);
	int GetDuration();

	bool OpenSubs(int flag, bool recreateFrame = true, bool refresh = false, bool resetParameters = false);
	void Render(bool recreateFrame = true);
	void ChangePositionByFrame(int cpos);
	bool RemoveVisual(bool noRefresh = false, bool disable = false);
	int GetFrameTime(bool start = true);
	int GetFrameTimeFromTime(int time, bool start = true);
	int GetFrameTimeFromFrame(int frame, bool start = true);
	void GetStartEndDelay(int startTime, int endTime, int *retStart, int *retEnd);
	void SetZoom();
	void GoToNextKeyframe();
	void GoToPrevKeyframe();
	void OpenKeyframes(const wxString &filename);
	void SetColorSpace(const wxString& matrix, bool render = true);
	int GetPlayEndTime(int time);
	void DisableVisuals(bool disable);
	void DeleteAudioCache();
	wxWindow *GetMessageWindowParent();
	bool IsFullScreen();
	bool IsDirectShow();
	void GetVideoListsOptions(int *videoPlayAfter, int *videoSeekAfter);
	void SetVisual(bool settext = false, bool noRefresh = false);
	void ResetVisual();
	bool HasFFMS2();
	//can return null
	bool HasVideo() {
		if (renderer)
			return true;

		return false;
	}
	Provider *GetFFMS2();
	void SetVisualEdition(bool value);
	//can return null
	RendererVideo *GetRenderer();
	//can return null
	Fullscreen *GetFullScreenWindow();
	VideoToolbar *GetVideoToolbar();
	void HideVideoToolbar();
	void ShowVideoToolbar();
	int GetPanelHeight();
	//void SetPanelHeight(int panelHeight);
	void UpdateVideoWindow();
	int GetCurrentFrame();
	void ChangeVobsub(bool vobsub = false);
	void SetPanelOnFullScreen(bool value);
	void SetVideoWindowLastSize(const wxSize & size);
	bool IsOnAnotherMonitor();
	void SaveVolume();
	bool IsMenuShown();
	const wxString &GetKeyFramesFileName();
	void SetKeyFramesFileName(const wxString &fileName);
	void GetWindowSize(int* x, int* y);
	PlaybackState GetState();
private:

	BitmapButton* m_ButtonPreviousFile;
	BitmapButton* m_ButtonPause;
	BitmapButton* m_ButtonStop;
	BitmapButton* m_ButtonNextFile;
	BitmapButton* m_ButtonPlayLine;

	TabPanel *tab;
	VideoSlider* m_SeekingSlider;
	wxWindow* m_VideoPanel;
	bool m_ArrowEater;
	bool m_blockRender;
	wxMutex vbmutex;
	wxMutex nextmutex;
	wxTimer m_VideoTimeTimer;
	KaiTextCtrl* m_TimesTextField;
	VolSlider* m_VolumeSlider;
	VideoToolbar *m_VideoToolbar;
	int actualFile;
	int id;
	int prevchap;
	int m_X;
	int m_Y;
	int m_ToolBarHeight = 22;
	wxArrayString files;
	bool m_IsMenuShown;
	RendererVideo *renderer = nullptr;
	wxSize m_VideoWindowLastSize;
	Fullscreen *m_FullScreenWindow;
	//bool m_HasArrow;
	int m_LastCursor = wxCURSOR_ARROW;
	int m_LastFullScreenCursor = wxCURSOR_ARROW;
	int m_LastActiveLineStartTime = -1;
	int m_LastActiveLineStartFrame = -1;
	bool m_ShownKeyframe;
	//wxString oldpath;
	wxString m_KeyframesFileName;
	std::vector<RECT> MonRects;
	bool m_IsOnAnotherMonitor = false;
	bool m_IsFullscreen = false;
	bool m_FullScreenProgressBar = false;
	bool m_PanelOnFullscreen = false;
	int m_PanelHeight = 44;
	long m_AspectRatioX, m_AspectRatioY;
	float m_AspectRatio = 0.f, m_FPS = 0.f;
	bool m_IsDirectShow = false;

	void OnSize(wxSizeEvent& event);
	void OnPlaytime(wxTimerEvent& event);
	void OnIdle(wxTimerEvent& event);
	void OnHidePB();
	void OnDeleteVideo();
	void OnOpVideo();
	void OnOpSubs();
	void OnPaint(wxPaintEvent& event);
	void OnCopyCoords(const wxPoint &pos);
	void OnErase(wxEraseEvent& event){};
	void OnChangeVisual(wxCommandEvent &evt);
	void OnLostCapture(wxMouseCaptureLostEvent &evt);
	void ChangeButtonBMP(bool play = false);
	
	wxTimer idletime;
	DECLARE_EVENT_TABLE()
};

enum
{
	ID_VIDEO_TIME = 2100,
	ID_BPREV,
	ID_BPAUSE,
	ID_BSTOP,
	ID_BNEXT,
	ID_BPLINE,
	ID_SLIDER,
	ID_VOL,
	ID_MRECSUBS,
	ID_MRECVIDEO,
	ID_IDLE,
	MENU_STREAMS = 3333,
	MENU_CHAPTERS = 12000,
	MENU_MONITORS = 15000,
	ID_END_OF_STREAM = 23333,
	ID_REFRESH_TIME
};

#ifndef DRAWOUTTEXT
#define DRAWOUTTEXT(font,text,rect,align,color)\
	RECT tmpr=rect;\
	tmpr.top--;tmpr.bottom--;\
	tmpr.left--;tmpr.right--;\
	for(int i=0; i<9; i++)\
			{\
		if(i%3==0 && i>0){tmpr.left=rect.left-1; tmpr.right=rect.right-1; tmpr.top++;tmpr.bottom++;}\
		if(i!=4){font->DrawTextW(nullptr, text.wchar_str(), -1, &tmpr, align, 0xFF000000 );}\
		tmpr.left++;tmpr.right++;\
			}\
	font->DrawTextW(nullptr, text.wchar_str(), -1, &rect, align, color );
#endif


