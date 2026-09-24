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



#include "SubtitlesProvider.h"
#include "RendererVideo.h"
#include "OpennWrite.h"
#include "KainoteFrame.h"
#include "DshowRenderer.h"
#include "WinUndef.h"
#include <wx/thread.h>
#include <wx/app.h>
#include <process.h>
#include "config.h"
#include "UtilsWindows.h"
#include "AssBlend.h"
#include "Notebook.h"

std::atomic<bool> SubtitlesLibass::m_IsReady{ false };
SubtitlesLibass* SubtitlesLibass::m_LastRenderer = nullptr;
wxMutex SubtitlesLibass::openMutex;

void MessageCallback(int level, const char *fmt, va_list args, void *) {
	if (level >= 4) return;
	char buf[1024];

	vsprintf_s(buf, sizeof(buf), fmt, args);

	if (level < 2) // warning/error
#if _DEBUG
		KaiLogSilent(L"Libass: " + wxString(buf, wxConvUTF8));
#else
		KaiLogSilent(L"Libass: " + wxString(buf, wxConvUTF8));
#endif
#if _DEBUG
	else // verbose
		KaiLogSilent(L"Libass: " + wxString(buf, wxConvUTF8));
#endif
}

unsigned int __stdcall  ProcessLibassCache(void *data)
{
	SubtitlesLibass * libass = (SubtitlesLibass*)data;
	
	libass->m_Libass = ass_renderer_init(libass->m_Library);
	if (libass->m_Libass) {
		ass_set_font_scale(libass->m_Libass, 1.);
		ass_set_fonts(libass->m_Libass, "Arial", "Arial", 1, nullptr, true);
	}
	libass->m_IsReady.store(libass->m_Libass != nullptr);
	//reload all tabs to shows subtitles
	wxTheApp->CallAfter([]() { Notebook::RefreshVideo(); });

	return 0;
}

SubtitlesLibass::SubtitlesLibass()
{
	ReloadLibraries();
}
	
SubtitlesLibass::~SubtitlesLibass()
{
	//close it by force can make memory leaks
	if (thread){
		CloseHandle(thread);
	}

	m_AssTrack = nullptr;
	m_Tracks.Clear();
}

void SubtitlesLibass::BlendImages(ASS_Image* img, unsigned char* buffer)
{
	int videoPitch = m_VideoSize.GetWidth() * m_BytesPerColor;
	// libass returns alpha-masked monochrome images, each blended in its colour
	for (; img; img = img->next) {
		if (img->h == 0 || img->w == 0)
			continue;
		BlendAssBitmap(buffer + (img->dst_y * videoPitch) + (img->dst_x * 4), videoPitch,
			img->bitmap, img->stride, img->w, img->h, img->color);
	}
}

// call with openMutex locked
ASS_Image* SubtitlesLibass::RenderFrame(int time, int* change)
{
	*change = 1;
	if (!(m_IsReady.load() && m_AssTrack))
		return nullptr;
	ass_set_frame_size(m_Libass, m_VideoSize.GetWidth(), m_VideoSize.GetHeight());
	ASS_Image* img = ass_render_frame(m_Libass, m_AssTrack, time, change);
	if (m_LastRenderer != this)
		*change = 1;
	m_LastRenderer = this;
	return img;
}

void SubtitlesLibass::Draw(unsigned char* buffer, int time)
{
	wxMutexLocker lock(openMutex);
	int change;
	BlendImages(RenderFrame(time, &change), buffer);
	// libass now compares with this render, not with what the overlay shows
	m_HasRendered = false;
}

bool SubtitlesLibass::DrawOverlay(unsigned char* overlay, int time, wxRect* dirty)
{
	wxMutexLocker lock(openMutex);
	int change;
	ASS_Image* img = RenderFrame(time, &change);
	if (m_HasRendered && (change == 0 || (!img && m_OverlayDrawn.IsEmpty())))
		return false;
	m_HasRendered = true;

	wxRect frame(0, 0, m_VideoSize.GetWidth(), m_VideoSize.GetHeight());
	wxRect drawn;
	for (ASS_Image* i = img; i; i = i->next) {
		if (i->w && i->h)
			drawn.Union(wxRect(i->dst_x, i->dst_y, i->w, i->h));
	}
	drawn.Intersect(frame);
	m_OverlayDrawn.Intersect(frame);

	int pitch = frame.width * 4;
	for (int y = m_OverlayDrawn.y; y < m_OverlayDrawn.GetBottom() + 1; y++)
		memset(overlay + y * pitch + m_OverlayDrawn.x * 4, 0, m_OverlayDrawn.width * 4);
	BlendImages(img, overlay);

	*dirty = m_OverlayDrawn;
	dirty->Union(drawn);
	m_OverlayDrawn = drawn;
	return true;
}

bool SubtitlesLibass::Open(wxString *text)
{
	wxMutexLocker lock(openMutex);
	if (!m_IsReady || !m_HasParameters) {
		SAFE_DELETE(text);
		if (!m_HasParameters)
			KaiLog(_("Libass works only with FFMS2"));//Libass only works with with FFMS2
	
		return false;
	}

	m_AssTrack = nullptr;
	if (!text) {
		return true;
	}

	if (!ReadTrack(text)){
		KaiLog(_("Libass only opens ASS and SSA subtitles"));//Libass only works with ASS and SSA subtiltes
		return false;
	}
	return true;
}

bool SubtitlesLibass::ReadTrack(wxString *text)
{
	m_HasRendered = false;
	size_t hash = ParsedScripts<ASS_Track>::Hash(*text);
	m_AssTrack = m_Tracks.Find(hash);
	if (m_AssTrack) {
		delete text;
		return true;
	}
	wxScopedCharBuffer buffer = text->mb_str(wxConvUTF8);
	m_AssTrack = ass_read_memory(m_Library, buffer.data(), strlen(buffer), nullptr);
	delete text;
	if (!m_AssTrack)
		return false;
	m_Tracks.Add(hash, m_AssTrack);
	return true;
}

bool SubtitlesLibass::OpenString(wxString *text)
{
	wxMutexLocker lock(openMutex);
	if (!m_IsReady) {
		SAFE_DELETE(text);
		return false;
	}

	m_AssTrack = nullptr;
	if (!ReadTrack(text)){
		KaiLog(_("Cannot open subtitles in Libass"));
		return false;
	}
	return true;
}

void SubtitlesLibass::SetVideoParameters(const wxSize & size, unsigned char format, bool isSwapped)
{
	m_VideoSize = size;
	m_IsSwapped = isSwapped;
	m_Format = format;
	m_HasParameters = format == RGB32 || format == ARGB32;
	// the caller may hand a new overlay of the new size, so clear all of it once
	m_HasRendered = false;
	m_OverlayDrawn = wxRect(0, 0, size.GetWidth(), size.GetHeight());
}

void SubtitlesLibass::ReloadLibraries(bool destroyExisted)
{
	wxMutexLocker lock(openMutex);
	if (destroyExisted) {
		//KaiLog("Libass release");
		m_IsReady.store(false);
		// tracks belong to the library
		m_AssTrack = nullptr;
		m_Tracks.Clear();
		if (m_Libass) {
			ass_renderer_done(m_Libass);
			m_Libass = nullptr;
		}
		if (m_Library) {
			ass_library_done(m_Library);
			m_Library = nullptr;
		}
	}
	if (!m_Library) {
		m_IsReady.store(false);
		m_Library = ass_library_init();
		ass_set_message_cb(m_Library, MessageCallback, nullptr);
		if (!m_Libass) {
			unsigned int threadid = 0;
			thread = (HANDLE)_beginthreadex(0, 0, ProcessLibassCache, this, 0, &threadid);
			SetThreadName(threadid, "LibassCache");
		}
	}
}

