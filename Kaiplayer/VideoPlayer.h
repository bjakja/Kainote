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

#pragma once
#include <wx/wx.h>
#include <d3d9.h>
#include <d3dx9.h>


#undef DrawText

#include "VideoFfmpeg.h"
#include "Menu.h"
#include "Visuals.h"

//#define byvertices 5
#include <dxva2api.h>

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
	FloatRect(float _x, float _y, float _width, float _height){ x = _x; y = _y; width = _width; height = _height; };
	FloatRect(){ x = 0; y = 0; width = 0; height = 0; }
	float GetBottom() const { return y - height - 1; }
	float GetRight()  const { return x - width - 1; }
	float x;
	float y;
	float width;
	float height;
};

void CreateVERTEX(VERTEX *v, float X, float Y, D3DCOLOR Color, float Z = 0.0f);


class AudioDisplay;
class DShowPlayer;
class VideoCtrl;
struct csri_fmt;
struct csri_frame;

class VideoPlayer{
public:
	//constructor / destructor
	VideoPlayer(VideoCtrl *window);
	~VideoPlayer();

	//common functions
	void DrawLines(wxPoint point);
	void DrawProgressBar();
	bool DrawTexture(byte *nframe = NULL, bool copy = false);
	void DrawZoom();
	int GetCurrentPosition();
	int GetCurrentFrame();
	bool PlayLine(int start, int end);
	void ResetVisual();
	void ResetZoom();
	void SetVisual(bool remove = false, bool settext = false, bool noRefresh = false);
	void SetVisualZoom();
	void SetZoom();
	bool UpdateRects(bool changeZoom = true);
	void Zoom(const wxSize &size);
	void ZoomMouseHandle(wxMouseEvent &evt);

	//virtual functions for both FFMS2 and Direct Show
	virtual bool OpenFile(const wxString &fname, wxString *textsubs, bool vobsub, bool changeAudio = true){ return true; };
	virtual bool OpenSubs(wxString *textsubs, bool redraw = true, bool fromFile = false){ return true; };
	virtual bool Play(int end = -1){ return true; };
	virtual bool Pause(){ return true; };
	virtual bool Stop(){ return true; };
	virtual void SetPosition(int time, bool starttime = true, bool corect = true){};
	virtual int GetPlayEndTime(int time){ return 0; };
	virtual int GetDuration(){ return 0; };
	virtual int GetVolume(){ return 0; };
	virtual void GetVideoSize(int *width, int *height){};
	virtual wxSize GetVideoSize(){ return wxSize(); };
	virtual void GetFpsnRatio(float *fps, long *arx, long *ary){};
	virtual void SetVolume(int vol){};
	virtual void ChangePositionByFrame(int step){};
	virtual int GetFrameTime(bool start = true){ return 0; };
	virtual void GetStartEndDelay(int startTime, int endTime, int *retStart, int *retEnd){};
	virtual int GetFrameTimeFromTime(int time, bool start = true){ return 0; };
	virtual int GetFrameTimeFromFrame(int frame, bool start = true){ return 0; };
	virtual byte *GetFramewithSubs(bool subs, bool *del){ return NULL; };

	//virtual functions for FFMS2
	virtual void GoToNextKeyframe(){};
	virtual void GoToPrevKeyframe(){};
	virtual void DeleteAudioCache(){}
	virtual void SetColorSpace(const wxString& matrix){}
	virtual void OpenKeyframes(const wxString &filename){};

	//virtual functions for Direct Show
	void EnableStream(long index);
	void ChangeVobsub(bool vobsub = false);
	bool EnumFilters(Menu *menu);
	bool FilterConfig(wxString name, int idx, wxPoint pos);
	wxArrayString GetStreams();
	//rest
	void UpdateVideoWindow();
	

	void Render(bool RecreateFrame = true);
	
	void RecreateSurface();
	
	
	
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
	int numframe;
	int panelHeight;
	long ax, ay;
	float AR, fps;
	byte *datas;
	byte vformat;
	float frameDuration;
	float zoomParcent;
	wxString coords;
	wxString pbtime;
	ID3DXLine *lines;
	LPD3DXFONT overlayFont;
	wxCriticalSection mutexRender;
	wxMutex mutexLines;
	wxMutex mutexProgBar;
	wxMutex mutexOpenFile;
	PlaybackState vstate;
	Visuals *Visual;
	int playend;
	size_t lasttime;
	std::vector<chapter> chapters;
	FloatRect zoomRect;
	wxString keyframesFileName;
	
//protected:
	//virtual void SetScaleAndZoom(){}
protected:
	bool InitDX(bool reset = false);

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

	csri_frame *framee;
	csri_fmt *format;
	VideoCtrl *videoWindow;
};