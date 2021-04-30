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

#include "ProviderDummy.h"
#include "ColorSpace.h"
#include <wx/tokenzr.h>
#include "utils.h"
#include "Videobox.h"

ProviderDummy::~ProviderDummy()
{
}

ProviderDummy::ProviderDummy(const wxString& filename, RendererVideo* renderer, wxWindow* progressSinkWindow, bool* success)
	:Provider(filename, renderer)
{
	if (ParseDummyData(filename)) {
		if (filename.StartsWith(L"?")) {
			GenerateFrame();
			GenerateTimecodes();
			if (renderer) {
				unsigned int threadid = 0;
				m_thread = (HANDLE)_beginthreadex(0, 0, DummyProc, this, 0, &threadid);
				//CreateThread( NULL, 0,  (LPTHREAD_START_ROUTINE)FFMS2Proc, this, 0, 0);
				SetThreadPriority(m_thread, THREAD_PRIORITY_TIME_CRITICAL);
				SetThreadName(threadid, "DummyThread");
			}
		}
		*success = true;
		return;
	}
	*success = false;
}

void ProviderDummy::GetFrameBuffer(byte** buffer)
{
	if (!m_FrameBuffer)
		GenerateFrame();

	memcpy(*buffer, m_FrameBuffer, m_framePlane);
}

void ProviderDummy::GetFrame(int frame, byte* buff)
{
	GetFrameBuffer(&buff);
}

void ProviderDummy::GetBuffer(void* buf, int64_t start, int64_t count, double vol)
{
	memset(buf, 0, count);
}

void ProviderDummy::GetChapters(std::vector<chapter>* _chapters)
{
}

void ProviderDummy::DeleteOldAudioCache()
{
}

void ProviderDummy::SetColorSpace(const wxString& matrix)
{
}

void ProviderDummy::GenerateTimecodes()
{

}

void ProviderDummy::GenerateFrame()
{
	byte r = m_frameColor.Red();
	byte g = m_frameColor.Green();
	byte b = m_frameColor.Blue();
	byte* buff = m_FrameBuffer;
	if (m_pattern) {
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
		for (int i = 0; i < m_height; i++)
		{
			if ((i % 10) == 0) { ch1 = !ch1; }
			ch = ch1;
			for (int j = 0; j < m_width; j++)
			{
				int k = ((i * m_width) + j) * 4;
				if ((j % 10) == 0 && j > 0) { ch = !ch; }
				buff[k] = (ch) ? b : b1;
				buff[k + 1] = (ch) ? g : g1;
				buff[k + 2] = (ch) ? r : r1;
				buff[k + 3] = 0xFF;
			}

		}
	}
	else {
		for (int i = 0; i < m_height; i++) {
			for (int k = 0; k < m_width; k++) {
				*buff++ = b;
				*buff++ = g;
				*buff++ = r;
				*buff++ = 0xFF;
			}
		}
	}
}

bool ProviderDummy::ParseDummyData(const wxString& data)
{
	if (data.StartsWith(L"dummy")) {
		m_channels = 1;
		m_sampleRate = 44100;
		m_bytesPerSample = 2;
		m_numSamples = (int64_t)5 * 30 * 60 * 1000 * m_sampleRate / 1000;
		return true;
	}
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
	m_FPS = dfps;
	int dur = wxAtoi(strduration);
	if (dur < 1)
		return false;

	m_duration = dur;
	int width = wxAtoi(strwidth);
	int height = wxAtoi(strheight);
	if (!width || !height)
		return false;

	m_width = width;
	m_height = height;

	m_frameColor = wxColour(wxAtoi(strcolorr), wxAtoi(strcolorg), wxAtoi(strcolorb));
	if (strpattern == L"c")
		m_pattern = true;

	m_framePlane = m_height * m_width * 4;
}

unsigned int __stdcall ProviderDummy::DummyProc(void* cls)
{
	((ProviderDummy*)cls)->Processing();
	return 0;
}

void ProviderDummy::Processing()
{
	HANDLE events_to_wait[] = {
		m_eventStartPlayback,
		m_eventSetPosition,
		m_eventKillSelf
	};

	int tdiff = 0;

	if (m_width < 0) { return; }

	while (1) {
		DWORD wait_result = WaitForMultipleObjects(sizeof(events_to_wait) / sizeof(HANDLE), events_to_wait, FALSE, INFINITE);

		if (wait_result == WAIT_OBJECT_0 + 0)
		{
			byte* buff = (byte*)m_renderer->m_FrameBuffer;
			int acttime;
			while (1) {

				if (m_renderer->m_Frame != m_lastFrame) {
					m_renderer->m_Time = m_timecodes[m_renderer->m_Frame];
					m_lastFrame = m_renderer->m_Frame;
				}
				
				memcpy(&buff[0], m_FrameBuffer, m_framePlane);

				m_renderer->DrawTexture(buff);
				m_renderer->Render(false);

				if (m_renderer->m_Time >= m_renderer->m_PlayEndTime || m_renderer->m_Frame >= m_numFrames - 1) {
					wxCommandEvent* evt = new wxCommandEvent(wxEVT_COMMAND_BUTTON_CLICKED, ID_END_OF_STREAM);
					wxQueueEvent(m_renderer->videoControl, evt);
					break;
				}
				else if (m_renderer->m_State != Playing) {
					break;
				}
				acttime = timeGetTime() - m_renderer->m_LastTime;

				m_renderer->m_Frame++;
				m_renderer->m_Time = m_timecodes[m_renderer->m_Frame];

				tdiff = m_renderer->m_Time - acttime;

				if (tdiff > 0) { Sleep(tdiff); }
				else if (tdiff < -20) {
					while (1) {
						int frameTime = m_timecodes[m_renderer->m_Frame];
						if (frameTime >= acttime || frameTime >= m_renderer->m_PlayEndTime || m_renderer->m_Frame >= m_numFrames) {
							if (m_renderer->m_Frame >= m_numFrames) {
								m_renderer->m_Frame = m_numFrames - 1;
								m_renderer->m_Time = m_renderer->m_PlayEndTime;
							}
							break;
						}
						else {
							m_renderer->m_Frame++;
						}
					}

				}

			}
		}
		else if (wait_result == WAIT_OBJECT_0 + 1) {
			//entire seeking have to be in this thread or subtitles will out of sync
			m_renderer->SetFFMS2Position(m_changedTime, m_isStartTime);
		}
		else {
			break;
		}

	}
}
