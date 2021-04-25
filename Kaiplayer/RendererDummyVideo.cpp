//  Copyright (c) 2021, Marcin Drob

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

#include "RendererDummyVideo.h"
#include "DshowRenderer.h"
#include "Videobox.h"
#include "AudioBox.h"
#include "TabPanel.h"
#include "ColorSpace.h"
#include <wx/tokenzr.h>

RendererDummyVideo::RendererDummyVideo(VideoCtrl* control, bool visualDisabled)
	: RendererVideo(control, visualDisabled)
{
}

RendererDummyVideo::~RendererDummyVideo()
{
}

bool RendererDummyVideo::OpenFile(const wxString& fname, int subsFlag, bool vobsub, bool changeAudio)
{
	wxMutexLocker lock(m_MutexOpen);
	if (m_State == Playing) { videoControl->Stop(); }

	if (m_State != None) {
		m_VideoResized = videoControl->m_FullScreenProgressBar = false;
		m_State = None;
		Clear();
	}

	if (!ParseDummyData(fname))
		return false;
	if (m_Width < 1 || m_Height < 1)
		return false;

	m_Time = 0;
	m_Frame = 0;

	m_D3DFormat = D3DFMT_X8R8G8B8;
	m_Format = RGB32;
	int arwidth = m_Width;
	int arheight = m_Height;
	while (1) {
		bool divided = false;
		for (int i = 10; i > 1; i--) {
			if ((arwidth % i) == 0 && (arheight % i) == 0) {
				arwidth /= i; arheight /= i;
				divided = true;
				break;
			}
		}
		if (!divided) { break; }
	}
	videoControl->m_AspectRatioX = arwidth;
	videoControl->m_AspectRatioY = arheight;
	if (m_Width % 2 != 0) { m_Width++; }
	m_Pitch = m_Width * 4;

	diff = 0;
	m_FrameDuration = (1000.0f / videoControl->m_FPS);
	if (videoControl->m_AspectRatioY == 0 || videoControl->m_AspectRatioX == 0) { videoControl->m_AspectRatio = 0.0f; }
	else { videoControl->m_AspectRatio = (float)videoControl->m_AspectRatioY / (float)videoControl->m_AspectRatioX; }

	m_MainStreamRect.bottom = m_Height;
	m_MainStreamRect.right = m_Width;
	m_MainStreamRect.left = 0;
	m_MainStreamRect.top = 0;
	if (m_FrameBuffer) { delete[] m_FrameBuffer; m_FrameBuffer = NULL; }
	m_FrameBuffer = new byte[m_Height * m_Pitch];
	byte r = frameColor.Red();
	byte g = frameColor.Green();
	byte b = frameColor.Blue();
	byte* buff = m_FrameBuffer;
	if (pattern) {
		byte h = 0, s = 0, l = 0;
		byte r1 = 0, g1 = 0, b1 = 0;
		//I don't know if it's "b" swapped with "g" specially 
		//but I leave it like that to looks exactly like in Aegisub
		rgb_to_hsl(r, b, g, &h, &s, &l);
		l += 24;
		if (l < 24) l -= 48;
		hsl_to_rgb(h, s, l, &r1, &b1, &g1);
		bool ch = false;
		bool ch1 = false;
		for (int i = 0; i < m_Height; i++)
		{
			if ((i % 10) == 0) { ch1 = !ch1; }
			ch = ch1;
			for (int j = 0; j < m_Width; j++)
			{
				int k = ((i * m_Width) + j) * 4;
				if ((j % 10) == 0 && j > 0) { ch = !ch; }
				buff[k] = (ch) ? b : b1;
				buff[k + 1] = (ch) ? g : g1;
				buff[k + 2] = (ch) ? r : r1;
				buff[k + 3] = 0xFF;
			}

		}
	}
	else {
		for (int i = 0; i < m_Height; i++) {
			for (int k = 0; k < m_Width; k++) {
				*buff++ = b;
				*buff++ = g;
				*buff++ = r;
				*buff++ = 0xFF;
			}
		}
	}

	UpdateRects();

	if (!InitDX()) {
		return false;
	}

	m_SubsProvider->SetVideoParameters(wxSize(m_Width, m_Height), RGB32, false);

	OpenSubs(subsFlag, false);

	m_State = Stopped;

	if (m_Visual) {
		m_Visual->SizeChanged(wxRect(m_BackBufferRect.left, m_BackBufferRect.top,
			m_BackBufferRect.right, m_BackBufferRect.bottom), m_D3DLine, m_D3DFont, m_D3DDevice);
	}
	return true;
}

bool RendererDummyVideo::OpenSubs(int flag, bool redraw, wxString* text, bool resetParameters)
{
	wxCriticalSectionLocker lock(m_MutexRendering);
	if (resetParameters)
		m_SubsProvider->SetVideoParameters(wxSize(m_Width, m_Height), m_Format, m_SwapFrame);

	return m_SubsProvider->Open(tab, flag, text);
}

bool RendererDummyVideo::Play(int end)
{
	return false;
}

bool RendererDummyVideo::Pause()
{
	return false;
}

bool RendererDummyVideo::Stop()
{
	return false;
}

void RendererDummyVideo::SetPosition(int _time, bool starttime, bool corect, bool async)
{
	bool playing = m_State == Playing;
	m_Time = MID(0, _time, GetDuration());
	if (corect) {
		m_Time /= m_FrameDuration;
		if (starttime) { m_Time++; }
		m_Time *= m_FrameDuration;
	}
	m_Frame = m_Time / m_FrameDuration;
	m_LastTime = timeGetTime() - m_Time;
	m_PlayEndTime = GetDuration();

	if (m_HasVisualEdition) {
		//block removing or changing visual from main thread
		wxMutexLocker lock(m_MutexVisualChange);
		SAFE_DELETE(m_Visual->dummytext);
		/*if (m_Visual->Visual == VECTORCLIP){
			m_Visual->SetClip(m_Visual->GetVisual(), true, false, false);
		}
		else{*/
		OpenSubs((playing) ? OPEN_WHOLE_SUBTITLES : OPEN_DUMMY, true);
		if (playing) { m_HasVisualEdition = false; }
		//}
	}
	else if (m_HasDummySubs) {
		OpenSubs((playing) ? OPEN_WHOLE_SUBTITLES : OPEN_DUMMY, true);
	}
	if (playing) {
		if (m_AudioPlayer) {
			m_AudioPlayer->player->SetCurrentPosition(m_AudioPlayer->GetSampleAtMS(m_Time));
		}
	}
	else {
		//rebuild spectrum cause position can be changed
		//and it causes random bugs
		if (m_AudioPlayer) { m_AudioPlayer->UpdateImage(false, true); }

		videoControl->RefreshTime();
		Render();
	}
}

void RendererDummyVideo::GoToNextKeyframe()
{
}

void RendererDummyVideo::GoToPrevKeyframe()
{
}

int RendererDummyVideo::GetFrameTime(bool start)
{
	return 0;
}

void RendererDummyVideo::GetStartEndDelay(int startTime, int endTime, int* retStart, int* retEnd)
{
}

int RendererDummyVideo::GetFrameTimeFromTime(int time, bool start)
{
	return 0;
}

int RendererDummyVideo::GetFrameTimeFromFrame(int frame, bool start)
{
	return 0;
}

int RendererDummyVideo::GetPlayEndTime(int time)
{
	return 0;
}

int RendererDummyVideo::GetDuration()
{
	return duration;
}

int RendererDummyVideo::GetVolume()
{
	if (m_State == None || !m_AudioPlayer) { return 0; }
	double dvol = m_AudioPlayer->player->GetVolume();
	dvol = sqrt(dvol);
	dvol *= 8100.0;
	dvol -= 8100.0;
	return dvol;
}

void RendererDummyVideo::GetVideoSize(int* width, int* height)
{
	*width = m_Width;
	*height = m_Height;
}

void RendererDummyVideo::GetFpsnRatio(float* fps, long* arx, long* ary)
{
	*fps = videoControl->m_FPS;
	*arx = videoControl->m_AspectRatioX;
	*ary = videoControl->m_AspectRatioY;
}

void RendererDummyVideo::SetVolume(int vol)
{
	if (m_State == None || !m_AudioPlayer) { return; }

	vol = 7600 + vol;
	double dvol = vol / 7600.0;
	int sliderValue = (dvol * 99) + 1;
	if (tab->Edit->ABox) {
		tab->Edit->ABox->SetVolume(sliderValue);
	}
}

bool RendererDummyVideo::DrawTexture(byte* nframe, bool copy)
{
	return false;
}

void RendererDummyVideo::Render(bool RecreateFrame, bool wait)
{
}

void RendererDummyVideo::ChangePositionByFrame(int cpos)
{
}

byte* RendererDummyVideo::GetFramewithSubs(bool subs, bool* del, void* converter)
{
	return nullptr;
}

void RendererDummyVideo::OpenKeyframes(const wxString& filename)
{
}

bool RendererDummyVideo::ParseDummyData(const wxString& data)
{
	wxStringTokenizer tokenzr(data, L":", wxTOKEN_RET_EMPTY_ALL);
	
	tokenzr.GetNextToken();
	if (!tokenzr.HasMoreTokens())
		return false;
	wxString strfps = tokenzr.GetNextToken();
	if (!tokenzr.HasMoreTokens())
		return false;
	wxString strduration = tokenzr.GetNextToken();
	if (!tokenzr.HasMoreTokens())
		return false;
	wxString strwidth = tokenzr.GetNextToken();
	if (!tokenzr.HasMoreTokens())
		return false;
	wxString strheight = tokenzr.GetNextToken();
	if (!tokenzr.HasMoreTokens())
		return false;
	wxString strcolorr = tokenzr.GetNextToken();
	if (!tokenzr.HasMoreTokens())
		return false;
	wxString strcolorg = tokenzr.GetNextToken();
	if (!tokenzr.HasMoreTokens())
		return false;
	wxString strcolorb = tokenzr.GetNextToken();
	if (!tokenzr.HasMoreTokens())
		return false;
	wxString strpattern = tokenzr.GetNextToken();

	double dfps = 0;
	if (!strfps.ToCDouble(&dfps))
		return false;
	videoControl->m_FPS = dfps;
	int dur = wxAtoi(strduration);
	if (dur < 1)
		return false;

	duration = dur;
	int width = wxAtoi(strwidth);
	int height = wxAtoi(strheight);
	if (!width || !height)
		return false;

	m_Width = width;
	m_Height = height;

	frameColor = wxColour(wxAtoi(strcolorr), wxAtoi(strcolorg), wxAtoi(strcolorb));
	if (strpattern == L"c")
		pattern = true;
}
