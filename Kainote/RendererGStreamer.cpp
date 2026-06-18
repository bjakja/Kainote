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

#ifndef _WIN32

#include "RendererGStreamer.h"
#include "VideoBox.h"
#include "VideoFullscreen.h"
#include "KainoteFrame.h"
#include "TabPanel.h"
#include "Visuals.h"
#include "DshowRenderer.h"   // colour-format enum (ARGB32)
#include "SubtitlesProviderManager.h"
#include "LogHandler.h"
#include "config.h"
#include "Hotkeys.h"

#include <gst/gst.h>
#include <gst/video/video.h>
#include <gst/audio/streamvolume.h>
#include <gst/app/gstappsink.h>

#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/dcmemory.h>
#include <wx/dcgraph.h>

#include <algorithm>
#include <cmath>
#include <cstring>

namespace {
	// GstPlayFlags (the playbin "flags" GFlags) — not exported in a public header.
	enum {
		KAI_PLAY_FLAG_VIDEO       = (1 << 0),
		KAI_PLAY_FLAG_AUDIO       = (1 << 1),
		KAI_PLAY_FLAG_TEXT        = (1 << 2),
		KAI_PLAY_FLAG_SOFT_VOLUME = (1 << 4),
	};

	void EnsureGstInit()
	{
		static std::once_flag once;
		std::call_once(once, [] {
#ifdef KAI_GST_PLUGIN_DIR
			g_setenv("GST_PLUGIN_PATH_1_0", KAI_GST_PLUGIN_DIR, FALSE);
#endif
			GError *err = nullptr;
			if (!gst_init_check(nullptr, nullptr, &err)) {
				KaiLog(wxString::Format(L"GStreamer init failed: %s",
					wxString::FromUTF8(err ? err->message : "unknown")));
			}
			if (err) g_error_free(err);
		});
	}

	// appsink delivery trampolines -> RendererGStreamer::HandleSample.
	GstFlowReturn kai_new_sample(GstElement *sink, gpointer user)
	{
		GstSample *s = gst_app_sink_pull_sample(GST_APP_SINK(sink));
		if (s) {
			static_cast<RendererGStreamer *>(user)->HandleSample(s);
			gst_sample_unref(s);
		}
		return GST_FLOW_OK;
	}
	GstFlowReturn kai_new_preroll(GstElement *sink, gpointer user)
	{
		GstSample *s = gst_app_sink_pull_preroll(GST_APP_SINK(sink));
		if (s) {
			static_cast<RendererGStreamer *>(user)->HandleSample(s);
			gst_sample_unref(s);
		}
		return GST_FLOW_OK;
	}

	// Flatten a GstToc entry tree into chapters (editions hold chapters as
	// sub-entries), mirroring the Windows DirectShow marker list.
	void ParseTocEntries(GList *entries, std::vector<chapter> &out)
	{
		for (GList *e = entries; e; e = e->next) {
			GstTocEntry *entry = static_cast<GstTocEntry *>(e->data);
			if (gst_toc_entry_get_entry_type(entry) == GST_TOC_ENTRY_TYPE_CHAPTER) {
				gint64 start = 0, stop = 0;
				gst_toc_entry_get_start_stop_times(entry, &start, &stop);
				chapter ch;
				ch.time = (start > 0) ? (int)(start / GST_MSECOND) : 0;
				GstTagList *tags = gst_toc_entry_get_tags(entry);
				gchar *title = nullptr;
				if (tags && gst_tag_list_get_string(tags, GST_TAG_TITLE, &title) && title) {
					ch.name = wxString::FromUTF8(title);
					g_free(title);
				}
				else {
					ch.name = wxString::Format(_("Rozdział %i"), (int)out.size() + 1);
				}
				out.push_back(ch);
			}
			ParseTocEntries(gst_toc_entry_get_sub_entries(entry), out);
		}
	}

	// Blend a premultiplied-BGRA overlay (sw x sh) onto a BGRA frame at (dx, dy).
	void BlendPremultBgra(unsigned char *dst, int dstPitch, int dstW, int dstH,
		const unsigned char *src, int dx, int dy, int sw, int sh)
	{
		for (int y = 0; y < sh; ++y) {
			const int fy = dy + y;
			if (fy < 0 || fy >= dstH)
				continue;
			const unsigned char *s = src + (size_t)y * sw * 4;
			unsigned char *d = dst + (size_t)fy * dstPitch + (size_t)dx * 4;
			for (int x = 0; x < sw; ++x) {
				const int fx = dx + x;
				if (fx < 0 || fx >= dstW) { continue; }
				const unsigned char *sp = s + (size_t)x * 4;
				unsigned char *dp = d + (size_t)x * 4;
				const unsigned int ia = 255 - sp[3];
				dp[0] = (unsigned char)(sp[0] + dp[0] * ia / 255);
				dp[1] = (unsigned char)(sp[1] + dp[1] * ia / 255);
				dp[2] = (unsigned char)(sp[2] + dp[2] * ia / 255);
				dp[3] = 255;
			}
		}
	}
}

RendererGStreamer::RendererGStreamer(VideoBox *control, bool visualDisabled)
	: RendererVideo(control, visualDisabled)
{
	EnsureGstInit();
}

RendererGStreamer::~RendererGStreamer()
{
	if (m_LinuxAlive) m_LinuxAlive->store(false); // stop any queued present touching us
	Stop();
	TearDown();
	m_State = None;
}

// ---------------------------------------------------------------------------
// Pipeline lifecycle
// ---------------------------------------------------------------------------

void RendererGStreamer::TearDown()
{
	m_BusStop.store(true);
	if (m_BusThread.joinable())
		m_BusThread.join();
	m_BusStop.store(false);

	if (m_Pipeline) {
		// Disconnect the appsink callbacks first, then NULL the pipeline (which
		// stops the streaming thread + blocks here until it has), so no
		// HandleSample can run after this returns.
		if (m_AppSink)
			g_signal_handlers_disconnect_by_data(m_AppSink, this);
		gst_element_set_state(m_Pipeline, GST_STATE_NULL);
		gst_element_get_state(m_Pipeline, nullptr, nullptr, GST_CLOCK_TIME_NONE);
		gst_object_unref(m_Pipeline);
	}
	m_Pipeline = nullptr;
	m_AppSink = nullptr; // owned by playbin

	std::lock_guard<std::mutex> lock(m_LinuxPendingFrameMutex);
	m_LinuxPendingFrame.clear();
	m_LinuxPresentFrame.clear();
}

bool RendererGStreamer::WaitPreroll()
{
	GstState state = GST_STATE_VOID_PENDING;
	GstStateChangeReturn ret = gst_element_get_state(m_Pipeline, &state, nullptr, 10 * GST_SECOND);
	return ret == GST_STATE_CHANGE_SUCCESS || ret == GST_STATE_CHANGE_NO_PREROLL;
}

bool RendererGStreamer::QueryVideoInfo()
{
	GstPad *pad = nullptr;
	g_signal_emit_by_name(m_Pipeline, "get-video-pad", 0, &pad);
	if (!pad) {
		KaiLog(_("Plik wideo nie zawiera strumienia obrazu"));
		return false;
	}
	GstCaps *caps = gst_pad_get_current_caps(pad);
	gst_object_unref(pad);
	if (!caps)
		return false;

	GstVideoInfo vinfo;
	gboolean ok = gst_video_info_from_caps(&vinfo, caps);
	gst_caps_unref(caps);
	if (!ok)
		return false;

	const int width = GST_VIDEO_INFO_WIDTH(&vinfo);
	const int height = GST_VIDEO_INFO_HEIGHT(&vinfo);
	if (width <= 0 || height <= 0)
		return false;

	int fpsN = GST_VIDEO_INFO_FPS_N(&vinfo);
	int fpsD = GST_VIDEO_INFO_FPS_D(&vinfo);
	float fps = (fpsN > 0 && fpsD > 0) ? (float)fpsN / (float)fpsD : 23.976f;

	int parN = GST_VIDEO_INFO_PAR_N(&vinfo);
	int parD = GST_VIDEO_INFO_PAR_D(&vinfo);
	if (parN <= 0) parN = 1;
	if (parD <= 0) parD = 1;
	// Display aspect ratio numerator/denominator (matches the DirectShow
	// renderer's ARatioX/ARatioY semantics: m_AspectRatio = ARatioY / ARatioX).
	m_ArX = (long)width * parN;
	m_ArY = (long)height * parD;

	{
		// Serialise dims + frame-buffer alloc with the appsink frame handler.
		std::lock_guard<std::mutex> lock(m_SubsMutex);
		if (m_Width != width || m_Height != height || !m_FrameBuffer) {
			m_Width = width;
			m_Height = height;
			m_Pitch = m_Width * 4;
			m_Format = ARGB32;
			if (m_FrameBuffer) { delete[] m_FrameBuffer; m_FrameBuffer = nullptr; }
			m_FrameBuffer = new unsigned char[(size_t)m_Height * m_Pitch]();
		}
	}

	// Seed the present buffer with a black frame so RenderToDc always has a
	// correctly-sized buffer to blit and never falls back to m_FrameBuffer
	// (which the streaming thread may be writing).
	{
		std::lock_guard<std::mutex> pendingLock(m_LinuxPendingFrameMutex);
		m_LinuxPresentFrame.assign((size_t)m_Height * m_Pitch, 0);
	}

	videoControl->m_FPS = fps;
	videoControl->m_AspectRatioX = m_ArX;
	videoControl->m_AspectRatioY = m_ArY;
	videoControl->m_AspectRatio = (m_ArX == 0) ? 0.0f : (float)m_ArY / (float)m_ArX;
	m_FrameDuration = 1000.0f / fps;

	gint64 dur = 0;
	if (gst_element_query_duration(m_Pipeline, GST_FORMAT_TIME, &dur) && dur > 0)
		m_DurationMs = (int)(dur / GST_MSECOND);
	return true;
}

bool RendererGStreamer::OpenFile(const wxString &fname, int subsFlag, bool vobsub, bool changeAudio)
{
	wxMutexLocker lock(m_MutexOpen);
	if (m_State == Playing) { videoControl->Stop(); }
	TearDown();
	EnsureGstInit();

	m_Time = 0;
	m_Frame = 0;
	m_ReachedPlayEnd.store(false);

	m_Pipeline = gst_element_factory_make("playbin", "kainote-playbin");
	if (!m_Pipeline) {
		KaiLog(_("Nie można utworzyć potoku GStreamer (playbin)"));
		return false;
	}

	// file path -> URI
	gchar *uri = nullptr;
	wxString path = fname;
	if (path.StartsWith("?")) {
		// dummy/special source not supported by the playback path.
		TearDown();
		return false;
	}
	if (path.Contains("://"))
		uri = g_strdup(path.utf8_str().data());
	else
		uri = gst_filename_to_uri(path.utf8_str().data(), nullptr);
	if (!uri) { TearDown(); return false; }
	g_object_set(m_Pipeline, "uri", uri, nullptr);
	g_free(uri);

	// Video sink: a bin "videoconvert ! appsink(BGRA)" so the app receives packed
	// BGRA frames it composites + presents itself (sync=TRUE keeps the video on
	// the pipeline clock, i.e. A/V synced with the audio sink).
	GstElement *vbin = gst_bin_new("kainote-vbin");
	GstElement *conv = gst_element_factory_make("videoconvert", nullptr);
	GstElement *sink = gst_element_factory_make("appsink", "kainote-appsink");
	if (!vbin || !conv || !sink) {
		if (sink) gst_object_unref(sink);
		if (conv) gst_object_unref(conv);
		if (vbin) gst_object_unref(vbin);
		KaiLog(_("Nie można utworzyć modułu wyświetlania obrazu GStreamer"));
		TearDown();
		return false;
	}
	GstCaps *sinkCaps = gst_caps_new_simple("video/x-raw", "format", G_TYPE_STRING, "BGRA", nullptr);
	gst_app_sink_set_caps(GST_APP_SINK(sink), sinkCaps);
	gst_caps_unref(sinkCaps);
	gst_app_sink_set_max_buffers(GST_APP_SINK(sink), 2);
	gst_app_sink_set_drop(GST_APP_SINK(sink), TRUE);
	g_object_set(sink, "sync", TRUE, "emit-signals", TRUE, nullptr);
	g_signal_connect(sink, "new-sample", G_CALLBACK(kai_new_sample), this);
	g_signal_connect(sink, "new-preroll", G_CALLBACK(kai_new_preroll), this);
	gst_bin_add_many(GST_BIN(vbin), conv, sink, nullptr);
	gst_element_link(conv, sink);
	{
		GstPad *cpad = gst_element_get_static_pad(conv, "sink");
		gst_element_add_pad(vbin, gst_ghost_pad_new("sink", cpad));
		gst_object_unref(cpad);
	}
	g_object_set(m_Pipeline, "video-sink", vbin, nullptr);
	m_AppSink = sink; // borrowed; owned by vbin -> playbin

	// We composite subtitles ourselves; disable playbin's own text rendering.
	gint flags = 0;
	g_object_get(m_Pipeline, "flags", &flags, nullptr);
	flags |= KAI_PLAY_FLAG_VIDEO | KAI_PLAY_FLAG_AUDIO | KAI_PLAY_FLAG_SOFT_VOLUME;
	flags &= ~KAI_PLAY_FLAG_TEXT;
	g_object_set(m_Pipeline, "flags", flags, nullptr);

	SetVolumeInternal();

	if (gst_element_set_state(m_Pipeline, GST_STATE_PAUSED) == GST_STATE_CHANGE_FAILURE || !WaitPreroll()) {
		KaiLog(wxString::Format(_("Nie można otworzyć pliku wideo: %s"), fname));
		TearDown();
		return false;
	}

	if (!QueryVideoInfo()) { TearDown(); return false; }

	UpdateRects();

	if (changeAudio)
		KainoteFrame::Get()->OpenAudioInTab(tab, GLOBAL_CLOSE_AUDIO, emptyString);

	OpenSubs(subsFlag, false, nullptr, true);

	// Chapters: drain any TOC posted during preroll (the analogue of the Windows
	// DirectShow IAMExtendedSeeking markers).  Done on this (main) thread before
	// the bus thread starts, so VideoBox's m_Chapters reads have no writer race.
	m_Chapters.clear();
	{
		GstBus *tocBus = gst_element_get_bus(m_Pipeline);
		GstMessage *tocMsg = nullptr;
		while ((tocMsg = gst_bus_pop_filtered(tocBus, GST_MESSAGE_TOC)) != nullptr) {
			GstToc *toc = nullptr;
			gboolean updated = FALSE;
			gst_message_parse_toc(tocMsg, &toc, &updated);
			if (toc) {
				ParseTocEntries(gst_toc_get_entries(toc), m_Chapters);
				gst_toc_unref(toc);
			}
			gst_message_unref(tocMsg);
		}
		gst_object_unref(tocBus);
	}

	m_BusStop.store(false);
	m_BusThread = std::thread(&RendererGStreamer::BusLoop, this);

	m_State = Stopped;

	if (m_Visual) {
		m_Visual->SizeChanged(wxRect(m_BackBufferRect.left, m_BackBufferRect.top,
			m_BackBufferRect.right, m_BackBufferRect.bottom), nullptr, nullptr, nullptr);
	}
	// Present the prerolled frame (it may have arrived before QueryVideoInfo set
	// the frame buffer up; pull it explicitly to be sure something is on screen).
	if (m_AppSink) {
		GstSample *preroll = gst_app_sink_try_pull_preroll(GST_APP_SINK(m_AppSink), 0);
		if (preroll) {
			HandleSample(preroll);
			gst_sample_unref(preroll);
		}
	}
	return true;
}

// ---------------------------------------------------------------------------
// Frame delivery (appsink) + compositing
// ---------------------------------------------------------------------------

void RendererGStreamer::HandleSample(GstSample *sample)
{
	GstBuffer *buf = gst_sample_get_buffer(sample);
	GstCaps *caps = gst_sample_get_caps(sample);
	if (!buf || !caps)
		return;

	GstVideoInfo vinfo;
	if (!gst_video_info_from_caps(&vinfo, caps))
		return;
	const int w = GST_VIDEO_INFO_WIDTH(&vinfo);
	const int h = GST_VIDEO_INFO_HEIGHT(&vinfo);
	if (w <= 0 || h <= 0)
		return;

	if (GST_BUFFER_PTS_IS_VALID(buf)) {
		const int ms = (int)(GST_BUFFER_PTS(buf) / GST_MSECOND);
		m_Time = ms;
		if (m_PlayEndTime && ms >= m_PlayEndTime && !m_ReachedPlayEnd.exchange(true)) {
			wxQueueEvent(videoControl, new wxCommandEvent(wxEVT_COMMAND_MENU_SELECTED, VIDEO_PLAY_PAUSE));
			wxQueueEvent(videoControl, new wxCommandEvent(wxEVT_COMMAND_MENU_SELECTED, ID_REFRESH_TIME));
		}
	}

	GstVideoFrame vframe;
	if (!gst_video_frame_map(&vframe, &vinfo, buf, GST_MAP_READ))
		return;
	const unsigned char *src = static_cast<const unsigned char *>(GST_VIDEO_FRAME_PLANE_DATA(&vframe, 0));
	const int srcStride = GST_VIDEO_FRAME_PLANE_STRIDE(&vframe, 0);

	{
		std::lock_guard<std::mutex> lock(m_SubsMutex);
		if (m_Width != w || m_Height != h || !m_FrameBuffer) {
			m_Width = w;
			m_Height = h;
			m_Pitch = m_Width * 4;
			m_Format = ARGB32;
			if (m_FrameBuffer) { delete[] m_FrameBuffer; m_FrameBuffer = nullptr; }
			m_FrameBuffer = new unsigned char[(size_t)m_Height * m_Pitch]();
			m_SubsProvider->SetVideoParameters(wxSize(m_Width, m_Height), ARGB32, false);
		}
		const int rowBytes = std::min(srcStride, m_Pitch);
		for (int y = 0; y < m_Height; ++y)
			memcpy(m_FrameBuffer + (size_t)y * m_Pitch, src + (size_t)y * srcStride, rowBytes);

		// Subtitles, then (fullscreen) progress bar, composited into the frame.
		if (m_SubsOpened)
			m_SubsProvider->Draw(m_FrameBuffer, m_Time);

		if (m_PbValid.load() && videoControl->m_FullScreenProgressBar) {
			std::lock_guard<std::mutex> pbLock(m_PbMutex);
			if (!m_PbBuffer.empty() && m_PbW > 0 && m_PbH > 0)
				BlendPremultBgra(m_FrameBuffer, m_Pitch, m_Width, m_Height,
					m_PbBuffer.data(), m_PbX, m_PbY, m_PbW, m_PbH);
		}

		// Copy the composited frame into the pending buffer while still holding
		// m_SubsMutex, so a resize can't swap m_FrameBuffer between compositing
		// and this copy.  Lock order: m_SubsMutex -> m_PbMutex -> pending.
		std::lock_guard<std::mutex> pendingLock(m_LinuxPendingFrameMutex);
		const size_t frameBytes = (size_t)m_Height * m_Pitch;
		if (m_LinuxPendingFrame.size() != frameBytes)
			m_LinuxPendingFrame.resize(frameBytes);
		memcpy(m_LinuxPendingFrame.data(), m_FrameBuffer, frameBytes);
	}
	gst_video_frame_unmap(&vframe);
	QueueLinuxRender();
}

// ---------------------------------------------------------------------------
// Subtitles
// ---------------------------------------------------------------------------

bool RendererGStreamer::OpenSubs(int flag, bool redraw, wxString *text, bool resetParameters)
{
	bool result;
	{
		std::lock_guard<std::mutex> lock(m_SubsMutex);
		if (resetParameters)
			m_SubsProvider->SetVideoParameters(wxSize(m_Width, m_Height), ARGB32, false);
		result = m_SubsProvider->Open(tab, flag, text);
		m_SubsOpened = result && flag != CLOSE_SUBTITLES;
	}

	if (redraw && m_State != None && m_State != Playing && m_Pipeline) {
		// Paused: re-seek to the current position so a fresh frame prerolls and
		// gets recomposited with the new subtitles (new-preroll -> HandleSample).
		gst_element_seek_simple(m_Pipeline, GST_FORMAT_TIME,
			(GstSeekFlags)(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_ACCURATE),
			(gint64)m_Time * GST_MSECOND);
	}
	return result;
}

// ---------------------------------------------------------------------------
// Bus (EOS / errors / duration)
// ---------------------------------------------------------------------------

void RendererGStreamer::BusLoop()
{
	GstBus *bus = gst_element_get_bus(m_Pipeline);
	if (!bus)
		return;
	const GstMessageType want = (GstMessageType)(GST_MESSAGE_EOS | GST_MESSAGE_ERROR |
		GST_MESSAGE_WARNING | GST_MESSAGE_DURATION_CHANGED);
	while (!m_BusStop.load()) {
		GstMessage *msg = gst_bus_timed_pop_filtered(bus, 100 * GST_MSECOND, want);
		if (!msg)
			continue;
		switch (GST_MESSAGE_TYPE(msg)) {
		case GST_MESSAGE_EOS:
			wxQueueEvent(videoControl, new wxCommandEvent(wxEVT_COMMAND_BUTTON_CLICKED, ID_END_OF_STREAM));
			break;
		case GST_MESSAGE_ERROR: {
			GError *err = nullptr; gchar *dbg = nullptr;
			gst_message_parse_error(msg, &err, &dbg);
			KaiLog(wxString::Format(L"GStreamer: %s", wxString::FromUTF8(err ? err->message : "error")));
			if (err) g_error_free(err);
			g_free(dbg);
			// Don't leave the UI stuck "playing" on a fatal pipeline error.
			if (m_State == Playing)
				wxQueueEvent(videoControl, new wxCommandEvent(wxEVT_COMMAND_MENU_SELECTED, VIDEO_PLAY_PAUSE));
			break;
		}
		case GST_MESSAGE_DURATION_CHANGED: {
			gint64 dur = 0;
			if (gst_element_query_duration(m_Pipeline, GST_FORMAT_TIME, &dur) && dur > 0)
				m_DurationMs = (int)(dur / GST_MSECOND);
			break;
		}
		default:
			break;
		}
		gst_message_unref(msg);
	}
	gst_object_unref(bus);
}

// ---------------------------------------------------------------------------
// Transport
// ---------------------------------------------------------------------------

bool RendererGStreamer::Play(int end)
{
	if (!m_Pipeline)
		return false;
	SetThreadExecutionState(ES_DISPLAY_REQUIRED | ES_CONTINUOUS);
	if (!(videoControl->IsShown() ||
		(videoControl->m_FullScreenWindow && videoControl->m_FullScreenWindow->IsShown())))
		return false;

	if (m_HasVisualEdition) {
		OpenSubs(OPEN_WHOLE_SUBTITLES, false);
		if (m_Visual) SAFE_DELETE(m_Visual->dummytext);
		m_HasVisualEdition = false;
	}
	else if (m_HasDummySubs && tab->editor) {
		OpenSubs(OPEN_WHOLE_SUBTITLES, false);
	}

	m_PlayEndTime = (end > 0) ? end : 0;
	m_ReachedPlayEnd.store(false);
	if (m_Time < GetDuration() - m_FrameDuration)
		gst_element_set_state(m_Pipeline, GST_STATE_PLAYING);

	m_State = Playing;
	return true;
}

bool RendererGStreamer::Pause()
{
	if (m_State == Playing) {
		SetThreadExecutionState(ES_CONTINUOUS);
		m_State = Paused;
		if (m_Pipeline) gst_element_set_state(m_Pipeline, GST_STATE_PAUSED);
	}
	else if (m_State != None) {
		Play();
	}
	else { return false; }
	return true;
}

bool RendererGStreamer::Stop()
{
	if (m_State == Playing) {
		SetThreadExecutionState(ES_CONTINUOUS);
		m_State = Stopped;
		if (m_Pipeline) {
			gst_element_set_state(m_Pipeline, GST_STATE_PAUSED);
			gst_element_seek_simple(m_Pipeline, GST_FORMAT_TIME,
				(GstSeekFlags)(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT), 0);
		}
		m_PlayEndTime = 0;
		m_Time = 0;
		return true;
	}
	return false;
}

void RendererGStreamer::SetPosition(int _time, bool starttime, bool corect, bool async, bool refreshAudio)
{
	if (!m_Pipeline)
		return;
	m_Time = MID(0, _time, GetDuration());
	if (corect) {
		m_Time /= m_FrameDuration;
		if (starttime) { m_Time++; }
		m_Time *= m_FrameDuration;
	}
	m_PlayEndTime = 0;
	m_ReachedPlayEnd.store(false);
	gst_element_seek_simple(m_Pipeline, GST_FORMAT_TIME,
		(GstSeekFlags)(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT),
		(gint64)m_Time * GST_MSECOND);
}

void RendererGStreamer::ChangePositionByFrame(int step)
{
	if (m_State == Playing || m_State == None)
		return;
	m_Time += (m_FrameDuration * step);
	SetPosition(m_Time, true, false);
	videoControl->RefreshTime();
}

void RendererGStreamer::Render(bool, bool)
{
	// App-side present: just request a repaint of the current frame so the
	// visual-editing overlay / progress bar are redrawn (RenderToDc).
	if (m_State == None)
		return;
	wxWindow *renderWindow = (videoControl->m_IsFullscreen && videoControl->m_FullScreenWindow) ?
		static_cast<wxWindow *>(videoControl->m_FullScreenWindow) : static_cast<wxWindow *>(videoControl);
	if (renderWindow)
		renderWindow->Refresh(false);
}

void RendererGStreamer::RecreateSurface()
{
	Render();
}

// ---------------------------------------------------------------------------
// Frame-timing helpers (identical to the DirectShow renderer — pure
// m_FrameDuration / FPS arithmetic, no backend state).
// ---------------------------------------------------------------------------

int RendererGStreamer::GetFrameTime(bool start)
{
	int halfFrame = (start) ? -(m_FrameDuration / 2.0f) : (m_FrameDuration / 2.0f) + 1;
	return m_Time + halfFrame;
}

void RendererGStreamer::GetStartEndDelay(int startTime, int endTime, int *retStart, int *retEnd)
{
	if (!retStart || !retEnd) { return; }
	int frameStartTime = (((float)startTime / 1000.f) * videoControl->m_FPS);
	int frameEndTime = (((float)endTime / 1000.f) * videoControl->m_FPS);
	frameStartTime++;
	frameEndTime++;
	*retStart = (((frameStartTime * 1000) / videoControl->m_FPS) + 0.5f) - startTime;
	*retEnd = (((frameEndTime * 1000) / videoControl->m_FPS) + 0.5f) - endTime;
}

int RendererGStreamer::GetFrameTimeFromTime(int _time, bool start)
{
	int halfFrame = (start) ? -(m_FrameDuration / 2.0f) : (m_FrameDuration / 2.0f) + 1;
	return _time + halfFrame;
}

int RendererGStreamer::GetFrameTimeFromFrame(int frame, bool start)
{
	int halfFrame = (start) ? -(m_FrameDuration / 2.0f) : (m_FrameDuration / 2.0f) + 1;
	return (frame * (1000.f / videoControl->m_FPS)) + halfFrame;
}

int RendererGStreamer::GetPlayEndTime(int _time)
{
	int newTime = _time;
	newTime /= m_FrameDuration;
	newTime = (newTime * m_FrameDuration) + 1.f;
	if (_time == newTime && newTime % 10 == 0) { newTime -= 5; }
	return newTime;
}

// ---------------------------------------------------------------------------
// Info / volume / streams
// ---------------------------------------------------------------------------

int RendererGStreamer::GetDuration()
{
	if (m_Pipeline) {
		gint64 dur = 0;
		if (gst_element_query_duration(m_Pipeline, GST_FORMAT_TIME, &dur) && dur > 0)
			m_DurationMs = (int)(dur / GST_MSECOND);
	}
	return m_DurationMs;
}

void RendererGStreamer::GetVideoSize(int *width, int *height)
{
	if (width) *width = m_Width;
	if (height) *height = m_Height;
}

void RendererGStreamer::GetFpsnRatio(float *fps, long *arx, long *ary)
{
	if (fps) *fps = videoControl->m_FPS;
	if (arx) *arx = m_ArX;
	if (ary) *ary = m_ArY;
}

void RendererGStreamer::SetVolumeInternal()
{
	if (!m_Pipeline)
		return;
	// VideoBox passes hundredths of a dB (IBasicAudio range: 0 = full, down to
	// -10000 = silence).  Convert to GStreamer's linear factor.
	double db = m_VolumeMb / 100.0;
	double linear = gst_stream_volume_convert_volume(GST_STREAM_VOLUME_FORMAT_DB,
		GST_STREAM_VOLUME_FORMAT_LINEAR, db);
	gst_stream_volume_set_volume(GST_STREAM_VOLUME(m_Pipeline), GST_STREAM_VOLUME_FORMAT_LINEAR, linear);
}

void RendererGStreamer::SetVolume(int vol)
{
	m_VolumeMb = vol;
	if (m_State == None)
		return;
	SetVolumeInternal();
}

int RendererGStreamer::GetVolume()
{
	return m_VolumeMb;
}

wxArrayString RendererGStreamer::GetStreams()
{
	wxArrayString out;
	if (!m_Pipeline)
		return out;
	gint nAudio = 0, current = -1;
	g_object_get(m_Pipeline, "n-audio", &nAudio, "current-audio", &current, nullptr);
	for (gint i = 0; i < nAudio; i++) {
		wxString name;
		GstTagList *tags = nullptr;
		g_signal_emit_by_name(m_Pipeline, "get-audio-tags", i, &tags);
		if (tags) {
			gchar *lang = nullptr;
			if (gst_tag_list_get_string(tags, GST_TAG_LANGUAGE_NAME, &lang) ||
				gst_tag_list_get_string(tags, GST_TAG_LANGUAGE_CODE, &lang)) {
				name = wxString::FromUTF8(lang);
				g_free(lang);
			}
			gst_tag_list_unref(tags);
		}
		if (name.empty())
			name = wxString::Format(_("Ścieżka %i"), (int)(i + 1));
		// Format consumed by VideoBox::ContextMenu: "<ident>:<name> <enabled>".
		out.Add(wxString::Format(L"A: %s %d", name, (i == current) ? 1 : 0));
	}
	return out;
}

void RendererGStreamer::EnableStream(long index)
{
	if (m_Pipeline)
		g_object_set(m_Pipeline, "current-audio", (gint)index, nullptr);
}

void RendererGStreamer::DrawProgressBar(const wxString &timesString)
{
	wxMutexLocker openLock(m_MutexProgressBar);
	int videoW, videoH;
	{
		std::lock_guard<std::mutex> subsLock(m_SubsMutex);
		videoW = m_Width;
		videoH = m_Height;
	}
	if (videoW <= 0 || videoH <= 0)
		return;

	// Render the bar + time into a small premultiplied-BGRA bitmap on the main
	// thread (wx drawing is not thread-safe), to be blended into the video frame.
	wxFont *font = Options.GetFont(4);
	int fw = 0, fh = 0;
	videoControl->GetTextExtent(timesString, &fw, &fh, nullptr, nullptr, font);
	if (fw <= 0 || fh <= 0 || !font)
		return;

	const int barH = std::max(4, (int)(fh * 0.6f));
	const int pad = std::max(2, (int)(fh * 0.25f));
	const int W = fw + pad * 2;
	const int H = barH + pad * 3 + fh;

	wxBitmap bmp(W, H, 32);
	{
		wxMemoryDC mdc(bmp);
		mdc.SetBackground(*wxTRANSPARENT_BRUSH);
		mdc.Clear();
		wxGCDC dc(mdc);
		dc.SetPen(*wxTRANSPARENT_PEN);
		dc.SetBrush(wxBrush(wxColour(0, 0, 0, 150)));      // translucent backdrop
		dc.DrawRectangle(0, 0, W, H);
		dc.SetBrush(wxBrush(wxColour(255, 255, 255, 230))); // bar track
		dc.DrawRectangle(pad, pad, fw, barH);
		const int dur = GetDuration();
		float ratio = (dur > 0) ? std::min(1.0f, std::max(0.0f, (float)m_Time / (float)dur)) : 0.0f;
		dc.SetBrush(wxBrush(wxColour(46, 166, 226, 255)));  // Kainote blue fill
		dc.DrawRectangle(pad + 1, pad + 1, (int)((fw - 2) * ratio), barH - 2);
		dc.SetFont(*font);
		dc.SetTextForeground(wxColour(255, 255, 255, 255));
		dc.DrawText(timesString, pad, pad * 2 + barH);
	}

	wxImage img = bmp.ConvertToImage();
	const unsigned char *rgb = img.GetData();
	if (!rgb)
		return;
	const unsigned char *alpha = img.HasAlpha() ? img.GetAlpha() : nullptr;
	const size_t px = (size_t)W * (size_t)H;
	std::vector<unsigned char> bgra(px * 4);
	for (size_t i = 0; i < px; ++i) {
		const unsigned char a = alpha ? alpha[i] : 255;
		bgra[i * 4 + 0] = (unsigned char)(rgb[i * 3 + 2] * a / 255); // B (premultiplied)
		bgra[i * 4 + 1] = (unsigned char)(rgb[i * 3 + 1] * a / 255); // G
		bgra[i * 4 + 2] = (unsigned char)(rgb[i * 3 + 0] * a / 255); // R
		bgra[i * 4 + 3] = a;
	}

	std::lock_guard<std::mutex> lock(m_PbMutex);
	m_PbBuffer.swap(bgra);
	m_PbW = W;
	m_PbH = H;
	m_PbX = std::max(0, videoW - W - pad); // top-right of the video frame
	m_PbY = pad;
	m_PbValid.store(true);
}

unsigned char *RendererGStreamer::GetFrameWithSubs(bool subs, bool *del)
{
	if (del) *del = false;
	if (!m_Pipeline || m_Width <= 0 || m_Height <= 0)
		return nullptr;

	// Ask playbin (playsink) to convert its last displayed frame to packed BGRA.
	GstCaps *caps = gst_caps_new_simple("video/x-raw",
		"format", G_TYPE_STRING, "BGRA",
		"width", G_TYPE_INT, m_Width,
		"height", G_TYPE_INT, m_Height,
		nullptr);
	GstSample *sample = nullptr;
	g_signal_emit_by_name(m_Pipeline, "convert-sample", caps, &sample);
	gst_caps_unref(caps);
	if (!sample)
		return nullptr;

	const size_t need = (size_t)m_Width * (size_t)m_Height * 4;
	unsigned char *out = nullptr;
	GstBuffer *buf = gst_sample_get_buffer(sample);
	GstMapInfo map;
	if (buf && gst_buffer_map(buf, &map, GST_MAP_READ)) {
		if (map.size >= need) {
			out = new unsigned char[need];
			memcpy(out, map.data, need);
		}
		gst_buffer_unmap(buf, &map);
	}
	gst_sample_unref(sample);
	if (!out)
		return nullptr;

	if (subs) {
		std::lock_guard<std::mutex> lock(m_SubsMutex);
		if (m_SubsOpened)
			m_SubsProvider->Draw(out, m_Time); // bake the current libass overlay in
	}
	if (del) *del = true;
	return out;
}

void RendererGStreamer::ChangeVobsub(bool vobsub)
{
	// Embedded (muxed) subtitle handling is not wired through the GStreamer
	// playback path yet; libass overlay covers the editor's subtitles.
	// ponytail: muxed/vobsub track selection is a follow-up if requested.
}

#endif // _WIN32
