//  Copyright (c) 2020 - 2026, Marcin Drob

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
#include "Timebase.h"
#include "Playback.h"
#include <atomic>
#include "KainoteFrame.h"
//#include "VisualDrawingShapes.h"
#include <d3d9.h>
#include <d3dx9.h>
#include "UndoD3DXMacros.h"
#include <dxva2api.h>
#ifndef _WIN32
#include <atomic>
#include <memory>
#include <mutex>
#include <vector>
class wxDC;
#endif

class SubtitlesProviderManager;

enum PlaybackState : int
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
class AudioPosition;
class DShowPlayer;
class Menu;
class Provider;
class VideoBox;
class Visuals;

class RendererVideo
{
	friend class RendererDirectShow;
	friend class RendererFFMS2;
	friend class RendererGStreamer;
	friend class VideoBox;
public:
	RendererVideo(VideoBox *control, bool visualDisabled);
	virtual ~RendererVideo();

	virtual bool OpenFile(const wxString &fname, int subsFlag, bool vobsub, bool changeAudio = true){
		return false; 
	};
	virtual bool OpenSubs(int flag, bool redraw = true, wxString *text = nullptr, bool resetParameters = false){ return false; };
	// plays to end, or to the end of the video when end is not above 0
	bool Play(int end = -1);
	// pauses playback, or plays when it is paused or stopped
	bool Pause();
	bool Stop();
	// flags are SeekFlags
	virtual void SetPosition(int time, bool startTime = true, int flags = 0){};
	// the tab's timebase, owned by the video box
	const Timebase &GetTimebase();
	virtual int GetDuration(){ return 0; };
	virtual int GetVolume(){ return 0; };
	virtual void GetVideoSize(int *width, int *height){};
	virtual void GetFpsnRatio(float *fps, long *arx, long *ary){};
	virtual void SetVolume(int vol){};
	virtual bool DrawTexture(unsigned char * nframe = nullptr, bool copy = false) { return false; };
	virtual void Render(bool RecreateFrame = true, bool wait = true){};
	virtual void RecreateSurface(){};
	virtual void EnableStream(long index){};
	virtual void ChangePositionByFrame(int step);
	virtual void ChangeVobsub(bool vobsub = false){};
	virtual wxArrayString GetStreams(){ wxArrayString empty; return empty; };
	virtual unsigned char *GetFrameWithSubs(bool subs, bool *del){ return nullptr; };
	// buffer must be released via delete
	virtual unsigned char* GetFrame(int frame, bool subs) { return nullptr; };
	//int GetPreciseTime(bool start = true){};
	virtual void DeleteAudioCache(){}
	virtual void SetColorSpace(const wxString& matrix, bool render = true){}
	
	IDirect3DSurface9 * m_MainSurface = nullptr;
	IDirect3DDevice9 *m_D3DDevice = nullptr;
	D3DFORMAT m_D3DFormat;
	volatile bool m_BlockResize = false;
	// also read by the playback thread's seeks
	std::atomic<bool> m_HasVisualEdition{ false };
	// frames are NV12 that DXVA2 converts on the GPU, instead of BGRA
	bool m_Nv12 = false;
	size_t FrameBytes() const
	{
		return m_Nv12 ? (size_t)m_Width * m_Height * 3 / 2 : (size_t)m_Height * m_Pitch;
	}
	bool m_VideoResized = false;
	bool m_HasZoom = false;
	bool m_SwapFrame = false;
	int m_Width = 0;
	int m_Height = 0;
	int m_Pitch = 0;
	unsigned char *m_FrameBuffer = nullptr;
	RECT m_BackBufferRect;
	unsigned char m_Format;
	float m_FrameDuration = 0.f;
	float m_ZoomPercent = 1.f;
	wxString m_ProgressBarTime;
	ID3DXLine *m_D3DLine = nullptr;
	ID3DXFont *m_D3DFont = nullptr;
	ID3DXFont * m_D3DCalcFont = nullptr;
	wxCriticalSection m_MutexRendering;
	wxMutex m_MutexProgressBar;
	wxMutex m_MutexOpen;
	wxMutex m_MutexVisualChange;
	FloatRect m_ZoomRect;
	std::vector<chapter> m_Chapters;
	IDirectXVideoProcessorService *m_DXVAService = nullptr;
	IDirectXVideoProcessor *m_DXVAProcessor = nullptr;
	IDirect3D9 *m_D3DObject = nullptr;
	IDirect3DSurface9 *m_BlackBarsSurface = nullptr;
	VideoBox *videoControl = nullptr;
	Visuals *m_Visual = nullptr;
#ifndef _WIN32
	// Shared Linux software present: subclasses composite into m_FrameBuffer
	// (BGRA), double-buffer it here, and RenderToDc blits it to the wx video
	// window (and draws the visual-editing overlay on top).  Used by both the
	// FFMS2 (frame-accurate) and GStreamer (app-sink) renderers.
	std::atomic_bool m_LinuxRenderQueued{ false };
	std::mutex m_LinuxPendingFrameMutex;
	std::vector<unsigned char> m_LinuxPendingFrame;
	std::vector<unsigned char> m_LinuxPresentFrame;
	// Timestamp paired with m_LinuxPendingFrame under the same mutex.
	int m_LinuxPendingTime = -1;
	std::atomic_uint m_LinuxPresentedFrames{ 0 };
	// Outlives the renderer (copied into the QueueLinuxRender CallAfter lambda),
	// so a queued present can't touch a renderer that was deleted meanwhile
	// (e.g. switching renderer type via the FFMS2/GStreamer toggle).  Cleared at
	// the top of each derived destructor.
	std::shared_ptr<std::atomic_bool> m_LinuxAlive = std::make_shared<std::atomic_bool>(true);
	void RenderToDc(wxDC& dc);
	void QueueLinuxRender();
	void PresentLinuxFrame(const unsigned char* frame);
#endif
#

	virtual bool EnumFilters(Menu *menu){ return false; };
	virtual bool FilterConfig(wxString name, int idx, wxPoint pos){ return false; };
	virtual Provider * GetFFMS2(){ return nullptr; };
	virtual void ZoomChanged() {};
	// Non virtual functions
	virtual void DrawProgressBar(const wxString &timesString);
	// visual editing and dummy subtitles show one line; playback needs them all
	void OpenSubsForPlayback();
	// while paused after edits, parses the whole script ahead of the next Play
	void PrepareWholeSubtitles();
	void ReopenSubsAfterSeek(bool playing);
	// Reopens the subtitles, and when paused redraws, on the UI thread: the
	// subtitle text comes from the grid and edit box. Seeks from the playback
	// thread queue one refresh at a time, which takes the latest state.
	void QueueSeekRefresh(bool playing, bool refreshAudio);
	// the video shows text that edits have since changed
	void MarkSubtitlesOutdated();
	// changes whenever subtitles are opened, on any thread
	unsigned SubtitlesGeneration() const { return m_SubsGeneration; }
	void Zoom(const wxSize &size);
	void DrawZoom();
	void ZoomMouseHandle(wxMouseEvent &evt);
	void SetZoom(float percent = -1, const wxPoint &mousePos = wxDefaultPosition);
	void ResetZoom();
	void SetVisualZoom();
	void SetVisual(bool settext = false, bool noRefresh = false);
	void ResetVisual();
	//it's safe to not exist visual
	//returns true if removed
	bool RemoveVisual(bool noRefresh = false, bool disable = false);
	int GetCurrentPosition();
	// Milliseconds of the video played so far: the sound card's position when
	// audio plays, else the system clock kept in step with it.
	int PlaybackClock();
	virtual int GetCurrentFrame();
	bool PlayLine(int start, int end);
	void UpdateVideoWindow();
	bool UpdateRects(bool changeZoom = true);
	void VisualChangeTool(int tool);
	bool HasVisual(bool hasDefault = false);
	Visuals *GetVisual();
	void SetAudioPlayer(AudioDisplay *player);
	void SaveFrame(int id);
	PlaybackState GetState();
protected:
	// the adapters start, pause and stop their stream; the state is set around them
	virtual void StartStream(){};
	virtual void PauseStream(){};
	virtual void StopStream(){};
	// the seek target for time: clamped to the video, and on a frame unless SEEK_NO_SNAP
	int SeekTarget(int time, bool startTime, int flags);
	// Sleep and timeGetTime step in 1 ms instead of ~15.6 ms while playing
	void SetFineTimer(bool fine);

	// written by the playback and stream threads as well as the UI thread
	std::atomic<PlaybackState> m_State{ None };
	std::atomic<int> m_Time{ 0 };
	std::atomic<int> m_Frame{ 0 };
	// 0 plays to the end of the video
	std::atomic<int> m_PlayEndTime{ 0 };
	std::atomic<size_t> m_LastTime{ 0 };
	// Taken from the audio player on the UI thread when playback starts, as
	// the playback thread must not reach the player itself.
	void SetAudioPosition(std::shared_ptr<AudioPosition> position);
	std::shared_ptr<AudioPosition> GetAudioPosition();
	std::atomic<unsigned> m_SubsGeneration{ 0 };
private:

	bool InitDX();
	// the back buffer covers the monitor, so a resize within it needs no Reset
	bool FitsBackBuffer() const;
	void SetProjection();
	HWND m_DeviceWindow = nullptr;
	UINT m_BackBufferWidth = 0;
	UINT m_BackBufferHeight = 0;
	virtual bool InitRendererDX(){ return true; };
	// the text an OpenSubs flag stands for, with the vector clip mask added
	wxString *SubtitlesText(int flag, wxString *text);
	void Clear(bool clearObject = true);
	virtual void ClearObject() {};

	HWND m_HWND;
	bool m_DeviceLost = false;

	int diff = 0;
	char m_Grabbed = -1;

	RECT m_ProgressBarRect;
	RECT m_WindowRect;
	RECT m_MainStreamRect;
	wxPoint m_ZoomDiff;

	wxString *WholeSubtitlesText();
	void RunQueuedSeekRefresh();
	void SeekRefresh(bool playing, bool refreshAudio);

	std::mutex m_AudioPositionMutex;
	std::shared_ptr<AudioPosition> m_AudioPosition;
	std::mutex m_SeekRefreshMutex;
	bool m_SeekRefreshQueued = false;
	bool m_SeekRefreshPlaying = false;
	bool m_SeekRefreshAudio = false;
	int m_AverangeFrameTime = 42;
	bool m_FineTimer = false;
	D3DXVECTOR2 vectors[12];
	int m_ProgressBarLineWidth = 1;
	AudioDisplay *m_AudioPlayer = nullptr;
	TabPanel* tab = nullptr;
	SubtitlesProviderManager *m_SubsProvider = nullptr;

};
