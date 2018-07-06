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
#include <d3d9.h>
#include <d3dx9.h>


#undef DrawText

#include "VideoFfmpeg.h"
#include "Menu.h"
#include "Visuals.h"

//#define byvertices 5
//#define DXVA 77

//#if DXVA
#include <dxva2api.h>
//#endif
#include <chrono>
#include "LogHandler.h"


#ifndef SAFE_DELETE
#define SAFE_DELETE(x) if (x !=NULL) { delete x; x = NULL; }
#endif

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(x) if (x != NULL) { x->Release(); x = NULL; } 
#endif



#ifndef PTR
#define PTR(what,err) if(!what) {KaiLog(err); return false;}
#endif

#ifndef PTR1
#define PTR1(what,err) if(!what) {KaiLog(err); return;}
#endif

#ifndef HR
#define HR(what,err) if(FAILED(what)) {KaiLog(err); return false;}
#endif

#ifndef HRN
#define HRN(what,err) if(FAILED(what)) {KaiLog(err); return;}
#endif



typedef void csri_inst;
typedef void csri_rend;

enum PlaybackState
{
	Playing,
	Paused,
	Stopped,
	None
};
struct chapter
{
	wxString name;
	int time;
};

struct VERTEX
{	
	float fX;	
	float fY;	
	float fZ;
	D3DCOLOR Color;	
};

class FloatRect
{
public:
	FloatRect(float _x, float _y, float _width, float _height){x=_x; y=_y; width = _width; height = _height;};
	FloatRect(){x=0; y=0; width = 0; height = 0;}
	float GetBottom() const { return y - height - 1; }
    float GetRight()  const { return x - width - 1; }
	float x;
	float y;
	float width;
	float height;
};

void CreateVERTEX (VERTEX *v, float X, float Y, D3DCOLOR Color, float Z=0.0f);


class AudioDisplay;
class DShowPlayer;
struct csri_fmt;
struct csri_frame;

class VideoRenderer : public wxWindow
{
	
	public:
		VideoRenderer(wxWindow *parent, const wxSize &size=wxDefaultSize);
		virtual ~VideoRenderer();

		bool OpenFile(const wxString &fname, wxString *textsubs, bool Dshow, bool vobsub, bool changeAudio = true);
		bool OpenSubs(wxString *textsubs, bool redraw=true, bool fromFile = false);
		bool Play(int end=-1);
		bool PlayLine(int start, int end);
		bool Pause();
		bool Stop();
        void SetPosition(int time, bool starttime=true, bool corect=true, bool reloadSubs=true);
		void GoToNextKeyframe();
        void GoToPrevKeyframe();
		int GetCurrentPosition();
		int GetCurrentFrame();
		int GetFrameTime(bool start = true);
		void GetStartEndDelay(int startTime, int endTime, int *retStart, int *retEnd);
		int GetFrameTimeFromTime(int time, bool start = true);
		int GetFrameTimeFromFrame(int frame, bool start = true);
		int GetPlayEndTime(int time);
		int GetDuration();
		int GetVolume();
		void GetVideoSize(int *width, int *height);
		wxSize GetVideoSize();
		void GetFpsnRatio(float *fps, long *arx, long *ary);
		void UpdateVideoWindow();
		void SetVolume(int vol);
		
		void Render(bool RecreateFrame=true, bool wait = true);
		void DrawLines(wxPoint point);
		void DrawProgBar();
		bool DrawTexture(byte *nframe=NULL, bool copy=false);
		void RecreateSurface();
		void EnableStream(long index);
		void ChangePositionByFrame(int cpos);
		void ChangeVobsub(bool vobsub=false);
		wxArrayString GetStreams();
		void SetVisual(bool remove=false, bool settext=false, bool noRefreshAfterRemove = false);
		void ResetVisual();
		byte *GetFramewithSubs(bool subs, bool *del);
		bool UpdateRects(bool changeZoom=true);
		void Zoom(const wxSize &size);
		void DrawZoom();
		void ZoomMouseHandle(wxMouseEvent &evt);
		void SetZoom();
		void ResetZoom();
		void SetVisualZoom();
		int GetPreciseTime(bool start = true);
		void DeleteAudioCache(){if(VFF){VFF->DeleteOldAudioCache();}}
		void SetColorSpace(const wxString& matrix, bool render=true){
			if(VFF){
				VFF->SetColorSpace(matrix);
				if(vstate==Paused)
					Render();
			}
		}
		void OpenKeyframes(const wxString &filename);
		virtual void CaptureMouse(){};
		virtual void ReleaseMouse(){};
		virtual bool HasCapture(){return true;};
		virtual bool SetCursor(const wxCursor &cursor){return true;};
		LPDIRECT3DSURFACE9 MainStream;
		LPDIRECT3DDEVICE9 d3device;
		D3DFORMAT d3dformat;
		volatile bool block;
		bool IsDshow;
		bool seek;
		bool hasVisualEdition;
		bool hasDummySubs = true;
		bool cross;
		bool pbar;
		bool resized;
		bool isFullscreen;
		bool panelOnFullscreen;
		bool hasZoom;
		int vwidth;
		int vheight;
		int pitch;
		int time;
		int lastframe;
		int panelHeight;
		long ax,ay;
		float AR, fps;
		char *datas;
		byte vformat;
		float frameDuration;
		float zoomParcent;
		wxString coords;
		wxString pbtime;
		ID3DXLine *lines;
		LPD3DXFONT m_font;
		VideoFfmpeg *VFF;
		AudioDisplay *player;
		wxCriticalSection mutexRender;
		wxMutex mutexLines;
		wxMutex mutexProgBar;
		wxMutex mutexOpenFile;
		PlaybackState vstate;
		Visuals *Visual;
		int playend;
		size_t lasttime;
		std::vector<chapter> chaps;
		FloatRect zoomRect;
		wxString keyframesFileName;
		//std::chrono::system_clock::time_point startTime;
		bool EnumFilters(Menu *menu);
		bool FilterConfig(wxString name, int idx, wxPoint pos);
	protected:
		virtual void SetScaleAndZoom(){}
	private:
		bool InitDX(bool reset=false);
		
		void Clear();
		
		
		LPDIRECT3D9 d3dobject;
		LPDIRECT3DSURFACE9 bars;


#if byvertices
		LPDIRECT3DVERTEXBUFFER9 vertex;
		LPDIRECT3DTEXTURE9 texture;
#endif
		IDirectXVideoProcessorService *dxvaService;
		IDirectXVideoProcessor *dxvaProcessor;
		
		HWND hwnd;
		bool devicelost;
		//bool playblock;
		
		int diff;
		char grabbed;
		
		csri_inst *instance;
		csri_rend *vobsub;
		RECT crossRect;
		RECT progressBarRect;
		RECT windowRect;
		RECT backBufferRect;
		RECT mainStreamRect;
		wxPoint zoomDiff;
		
		int avframetime;
		
		D3DXVECTOR2 vectors[16];
		DShowPlayer *vplayer;
		
		csri_frame *framee;
		csri_fmt *format;
		
};

#ifndef DRAWOUTTEXT
#define DRAWOUTTEXT(font,text,rect,align,color)\
	RECT tmpr=rect;\
	tmpr.top--;tmpr.bottom--;\
	tmpr.left--;tmpr.right--;\
	for(int i=0; i<9; i++)\
	{\
		if(i%3==0 && i>0){tmpr.left=rect.left-1; tmpr.right=rect.right-1; tmpr.top++;tmpr.bottom++;}\
		if(i!=4){font->DrawTextW(NULL, text.wchar_str(), -1, &tmpr, align, 0xFF000000 );}\
		tmpr.left++;tmpr.right++;\
	}\
	font->DrawTextW(NULL, text.wchar_str(), -1, &rect, align, color );
#endif

