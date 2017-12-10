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


#pragma once

#include <wx/wx.h>
#include "VideoSlider.h"
#include "BitmapButton.h"
#include "VideoRenderer.h"
#include "VideoFullscreen.h"
#include "VideoToolbar.h"
#include "KaiTextCtrl.h"

class kainoteFrame;

class VideoCtrl : public VideoRend
{
public:

    VideoCtrl(wxWindow *parent, kainoteFrame *kfparent, const wxSize &size=wxDefaultSize);
    virtual ~VideoCtrl();
    bool Play();
	void PlayLine(int start, int end);
    bool Pause(bool burstbl=true);
    bool Stop();

	bool LoadVideo(const wxString& fileName, wxString *subsName, bool fulls = false, bool changeAudio = true);
    PlaybackState GetState();

    bool Seek(int whre,bool starttime=true,bool disp=true, bool reloadSubs=true);
    int Tell();
	bool CalcSize(int *width, int *height,int wwidth=0,int wheight=0,bool setstatus=false,bool calcH=false);

	void NextFile(bool next=true);
	void SetFullscreen(int wmonitor=0);
	void SetAspectRatio(float AR);
	void SetScaleAndZoom();
	VideoSlider* vslider;
	wxWindow* panel;
	bool seekfiles;
	bool eater;
	bool fullarrow;
	bool blockpaint;
	wxMutex vbmutex;
	wxMutex nextmutex;
	wxTimer vtime;
	KaiTextCtrl* mstimes;
	VolSlider* volslider;
	VideoToolbar *vToolbar;
	void OpenEditor(bool esc=true);
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
	wxRect GetMonitorRect(int wmonitor);
	void ContextMenu(const wxPoint &pos, bool dummy=false);
	void OnMouseEvent(wxMouseEvent& event);
	void CaptureMouse(){if(isFullscreen && TD){TD->CaptureMouse();}else{wxWindow::CaptureMouse();}}
	void ReleaseMouse(){if(isFullscreen && TD){TD->ReleaseMouse();}else{wxWindow::ReleaseMouse();}}
	bool HasCapture(){if(isFullscreen && TD){return TD->HasCapture();}else{return wxWindow::HasCapture();}}
	bool SetCursor(const wxCursor &cursor){if(isFullscreen && TD){return TD->SetCursor(cursor);}else{return wxWindow::SetCursor(cursor);}};
	bool SetBackgroundColour(const wxColour &col);
	float wspx,wspy;
	wxSize lastSize;
	Fullscreen *TD;
	bool hasArrow;
	bool shownKeyframe;
	wxString oldpath;
	std::vector<RECT> MonRects;
	bool isOnAnotherMonitor;
private:
    BitmapButton* bprev;
	BitmapButton* bpause;
	BitmapButton* bstop;
	BitmapButton* bnext;
	BitmapButton* bpline;
	
    

    kainoteFrame *Kai;
	int actfile;
	int id;
	int prevchap;
	wxArrayString pliki;
	bool ismenu;
	
	void OnSize(wxSizeEvent& event);
	void OnKeyPress(wxKeyEvent& event);
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
	void OnLostCapture(wxMouseCaptureLostEvent &evt){if(HasCapture()){ReleaseMouse();}};
	void ChangeButtonBMP(bool play=false);
	wxTimer idletime;
	DECLARE_EVENT_TABLE()
};

enum
	{
	idvtime=2000,
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
	MENU_STREAMS=3333,
	MENU_CHAPTERS=12000,
	MENU_MONITORS=15000,
	};

