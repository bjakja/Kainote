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

// GStreamer-based general-playback renderer for the Linux build.  playbin
// decodes and plays the file (video + audio in one pipeline, so A/V sync is
// handled by the GStreamer clock); the video sink is an appsink that delivers
// BGRA frames to the app, which composites the live libass subtitles + the
// fullscreen progress bar into them and presents through the shared wx software
// paint path (RendererVideo::RenderToDc) — the same path the frame-accurate
// FFMS2 renderer uses.  Drawing app-side lets the visual-editing handles, zoom
// and progress bar render on top of the video, interactively and while paused.
#ifndef _WIN32

#include "RendererVideo.h"

#include <atomic>
#include <mutex>
#include <thread>
#include <vector>

// Keep GStreamer/GLib types out of this header so the (cross-platform)
// VideoBox.cpp translation unit does not need the GStreamer include path.
typedef struct _GstElement GstElement;
typedef struct _GstSample GstSample;

class RendererGStreamer : public RendererVideo
{
	friend class RendererVideo;
	friend class VideoBox;
public:
	RendererGStreamer(VideoBox *control, bool visualDisabled);
	virtual ~RendererGStreamer();

	bool OpenFile(const wxString &fname, int subsFlag, bool vobsub, bool changeAudio = true) override;
	bool OpenSubs(int flag, bool redraw = true, wxString *text = nullptr, bool resetParameters = false) override;
	bool Play(int end = -1) override;
	bool Pause() override;
	bool Stop() override;
	void SetPosition(int _time, bool starttime = true, bool corect = true, bool async = true, bool refreshAudio = true) override;
	int GetFrameTime(bool start = true) override;
	void GetStartEndDelay(int startTime, int endTime, int *retStart, int *retEnd) override;
	int GetFrameTimeFromTime(int time, bool start = true) override;
	int GetFrameTimeFromFrame(int frame, bool start = true) override;
	int GetPlayEndTime(int time) override;
	int GetDuration() override;
	int GetVolume() override;
	void GetVideoSize(int *width, int *height) override;
	void GetFpsnRatio(float *fps, long *arx, long *ary) override;
	void SetVolume(int vol) override;
	void Render(bool RecreateFrame = true, bool wait = true) override;
	void RecreateSurface() override;
	void EnableStream(long index) override;
	void ChangePositionByFrame(int cpos) override;
	void ChangeVobsub(bool vobsub = false) override;
	wxArrayString GetStreams() override;
	// Current frame as a BGRA buffer (caller deletes when *del is set), with the
	// libass overlay optionally baked in.  Matches the Windows DirectShow path's
	// screenshot/copy-frame support (SaveFrame).
	unsigned char *GetFrameWithSubs(bool subs, bool *del) override;
	// Fullscreen progress bar: pre-rendered (main thread) into a premultiplied
	// BGRA buffer that the appsink frame handler blends into the video frame.
	void DrawProgressBar(const wxString &timesString) override;
	bool EnumFilters(Menu *menu) override { return false; }
	bool FilterConfig(wxString name, int idx, wxPoint pos) override { return false; }
	void OpenKeyframes(const wxString &filename) override {}

	// appsink callback target (new-sample / new-preroll).  Public only so the
	// C-linkage trampolines in the .cpp can reach it; do not call directly.
	void HandleSample(GstSample *sample);

private:
	void TearDown();
	bool QueryVideoInfo();
	void SetVolumeInternal();
	void BusLoop();
	bool WaitPreroll();

	GstElement *m_Pipeline = nullptr;   // playbin
	GstElement *m_AppSink = nullptr;    // video sink (borrowed; owned by playbin)

	std::thread m_BusThread;
	std::atomic_bool m_BusStop{ false };

	std::mutex m_SubsMutex;             // guards m_FrameBuffer/dims + libass Draw vs Open
	bool m_SubsOpened = false;

	// Fullscreen progress bar, pre-rendered on the main thread by DrawProgressBar
	// and blended into the frame (video-frame pixel coords) by the frame handler.
	std::mutex m_PbMutex;
	std::vector<unsigned char> m_PbBuffer; // premultiplied BGRA
	int m_PbX = 0, m_PbY = 0, m_PbW = 0, m_PbH = 0;
	std::atomic_bool m_PbValid{ false };

	int m_VolumeMb = 0;                 // cached volume, millibels (0 = full, <0 attenuation)
	int m_DurationMs = 0;               // cached duration
	long m_ArX = 0;
	long m_ArY = 0;
	std::atomic_bool m_ReachedPlayEnd{ false };
};

#endif // _WIN32
