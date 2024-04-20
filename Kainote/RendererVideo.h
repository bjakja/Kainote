//  Copyright (c) 2020, Marcin Drob

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


//#include "Visuals.h"
//#include "Menu.h"
//#include "SubtitlesProviderManager.h"
#include "Provider.h"
#include "KainoteFrame.h"
//#include "VisualDrawingShapes.h"
#include <d3d9.h>
#include <d3dx9.h>
#include <dxva2api.h>

class SubtitlesProviderManager;

enum PlaybackState
{
	Playing,
	Paused,
	Stopped,
	None
};

class chapter
{
public:
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
	FloatRect(float _x, float _y, float _width, float _height){ x = _x; y = _y; width = _width; height = _height; };
	FloatRect(){ x = 0; y = 0; width = 0; height = 0; }
	float GetBottom() const { return y - height - 1; }
	float GetRight()  const { return x - width - 1; }
	float x;
	float y;
	float width;
	float height;
};

void CreateVERTEX(VERTEX * v, float X, float Y, D3DCOLOR Color, float Z = 0.0f);


class AudioDisplay;
class DShowPlayer;
class Menu;
class Provider;
class VideoBox;
class Visuals;

class RendererVideo
{
	friend class RendererDirectShow;
	friend class RendererFFMS2;
	friend class VideoBox;
public:
	RendererVideo(VideoBox *control, bool visualDisabled);
	virtual ~RendererVideo();

	virtual bool OpenFile(const wxString &fname, int subsFlag, bool vobsub, bool changeAudio = true){
		return false; 
	};
	virtual bool OpenSubs(int flag, bool redraw = true, wxString *text = nullptr, bool resetParameters = false){ return false; };
	virtual bool Play(int end = -1){ return false; };
	virtual bool Pause(){ return false; };
	virtual bool Stop(){ return false; };
	virtual void SetPosition(int _time, bool starttime = true, bool corect = true, bool async = true){};
	virtual void SetFFMS2Position(int time, bool starttime){};
	virtual void GoToNextKeyframe(){};
	virtual void GoToPrevKeyframe(){};
	virtual int GetFrameTime(bool start = true){ return 0; };
	virtual void GetStartEndDelay(int startTime, int endTime, int *retStart, int *retEnd){};
	virtual int GetFrameTimeFromTime(int time, bool start = true){ return 0; };
	virtual int GetFrameTimeFromFrame(int frame, bool start = true){ return 0; };
	//if nothing loaded or loaded via Direct Show VFF is nullptr
	//return true if VFF is present
	//bool GetStartEndDurationFromMS(Dialogue *dial, SubsTime &duration);
	virtual int GetPlayEndTime(int time){ return 0; };
	virtual int GetDuration(){ return 0; };
	virtual int GetVolume(){ return 0; };
	virtual void GetVideoSize(int *width, int *height){};
	virtual void GetFpsnRatio(float *fps, long *arx, long *ary){};
	virtual void SetVolume(int vol){};
	virtual bool DrawTexture(unsigned char * nframe = nullptr, bool copy = false) { return false; };
	virtual void Render(bool RecreateFrame = true, bool wait = true){};
	virtual void RecreateSurface(){};
	virtual void EnableStream(long index){};
	virtual void ChangePositionByFrame(int cpos){};
	virtual void ChangeVobsub(bool vobsub = false){};
	virtual wxArrayString GetStreams(){ wxArrayString empty; return empty; };
	virtual unsigned char *GetFrameWithSubs(bool subs, bool *del){ return nullptr; };
	// buffer must be released via delete
	virtual unsigned char* GetFrame(int frame, bool subs) { return nullptr; };
	//int GetPreciseTime(bool start = true){};
	virtual void DeleteAudioCache(){}
	virtual void SetColorSpace(const wxString& matrix, bool render = true){}
	virtual void OpenKeyframes(const wxString &filename){};
	
	IDirect3DSurface9 * m_MainSurface = nullptr;
	IDirect3DDevice9 *m_D3DDevice = nullptr;
	D3DFORMAT m_D3DFormat;
	bool m_DirectShowSeeking;
	volatile bool m_BlockResize = false;
	bool m_HasVisualEdition = false;
	bool m_HasDummySubs = true;
	bool m_VideoResized = false;
	bool m_HasZoom = false;
	bool m_SwapFrame = false;
	int m_Width = 0;
	int m_Height = 0;
	int m_Pitch = 0;
	int m_Time = 0;
	int m_Frame = 0;
	unsigned char *m_FrameBuffer = nullptr;
	RECT m_BackBufferRect;
	unsigned char m_Format;
	float m_FrameDuration = 0.f;
	float m_ZoomParcent = 1.f;
	wxString m_ProgressBarTime;
	ID3DXLine *m_D3DLine = nullptr;
	ID3DXFont *m_D3DFont = nullptr;
	ID3DXFont * m_D3DCalcFont = nullptr;
	wxCriticalSection m_MutexRendering;
	wxMutex m_MutexProgressBar;
	wxMutex m_MutexOpen;
	wxMutex m_MutexVisualChange;
	PlaybackState m_State = None;
	int m_PlayEndTime = 0;
	size_t m_LastTime = 0;
	FloatRect m_ZoomRect;
	std::vector<chapter> m_Chapters;
	IDirectXVideoProcessorService *m_DXVAService = nullptr;
	IDirectXVideoProcessor *m_DXVAProcessor = nullptr;
	IDirect3D9 *m_D3DObject = nullptr;
	IDirect3DSurface9 *m_BlackBarsSurface = nullptr;
	VideoBox *videoControl = nullptr;
	Visuals *m_Visual = nullptr;
#

	virtual bool EnumFilters(Menu *menu){ return false; };
	virtual bool FilterConfig(wxString name, int idx, wxPoint pos){ return false; };
	virtual bool HasFFMS2(){ return false; };
	virtual Provider * GetFFMS2(){ return nullptr; };
	virtual void ZoomChanged() {};
	// Non virtual functions
	void DrawProgressBar(const wxString &timesString);
	void Zoom(const wxSize &size);
	void DrawZoom();
	void ZoomMouseHandle(wxMouseEvent &evt);
	void SetZoom();
	void ResetZoom();
	void SetVisualZoom();
	void SetVisual(bool settext = false, bool noRefresh = false);
	void ResetVisual();
	//it's safe to not exist visual
	//returns true if removed
	bool RemoveVisual(bool noRefresh = false, bool disable = false);
	int GetCurrentPosition();
	int GetCurrentFrame();
	bool PlayLine(int start, int end);
	void UpdateVideoWindow();
	bool UpdateRects(bool changeZoom = true);
	void VisualChangeTool(int tool);
	bool HasVisual(bool hasDefault = false);
	Visuals *GetVisual();
	void SetAudioPlayer(AudioDisplay *player);
	void SaveFrame(int id);
	PlaybackState GetState();
private:

	bool InitDX();
	virtual bool InitRendererDX(){ return true; };
	void Clear(bool clearObject = true);
	virtual void ClearObject() {};
	virtual void DestroyFFMS2() {};

	HWND m_HWND;
	bool m_DeviceLost = false;

	int diff = 0;
	char m_Grabbed = -1;

	RECT m_ProgressBarRect;
	RECT m_WindowRect;
	RECT m_MainStreamRect;
	wxPoint m_ZoomDiff;

	int m_AverangeFrameTime = 42;
	D3DXVECTOR2 vectors[12];
	int m_ProgressBarLineWidth = 1;
	AudioDisplay *m_AudioPlayer = nullptr;
	TabPanel* tab = nullptr;
	SubtitlesProviderManager *m_SubsProvider = nullptr;

};
