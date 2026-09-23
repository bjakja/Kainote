//  Copyright (c) 2021 - 2026, Marcin Drob

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



#include "Provider.h"
#include "ProviderDummy.h"
#include "ProviderFFMS2.h"
#include "KaiMessageBox.h"
#include "AudioBox.h"
#include "VisualDrawingShapes.h"
#include "Notebook.h"


Provider::Provider(const wxString& filename, RendererVideo* renderer)
	: m_renderer(renderer)
	, m_filename(filename)
	, m_eventStartPlayback(CreateEvent(0, FALSE, FALSE, 0))
	, m_eventSetPosition(CreateEvent(0, FALSE, FALSE, 0))
	, m_eventKillSelf(CreateEvent(0, FALSE, FALSE, 0))
	, m_eventComplete(CreateEvent(0, FALSE, FALSE, 0))
{
}

Provider* Provider::Get(const wxString& filename, RendererVideo* renderer, wxWindow* progressSinkWindow, bool* success)
{
	if (filename.StartsWith(L"?dummy") || filename.StartsWith(L"dummy")) {
		return new ProviderDummy(filename, renderer, progressSinkWindow, success);
	}
	else {
		return new ProviderFFMS2(filename, renderer, progressSinkWindow, success);
	}
	return nullptr;
}

Provider::~Provider()
{
	auto closeHandle = [](HANDLE& handle) {
		if (handle) {
			CloseHandle(handle);
			handle = nullptr;
		}
	};

	closeHandle(m_eventStartPlayback);
	closeHandle(m_eventSetPosition);
	closeHandle(m_eventKillSelf);
	closeHandle(m_eventComplete);
}

void Provider::Play()
{
	SetEvent(m_eventStartPlayback);
}

int Provider::GetSampleRate()
{
	return m_sampleRate;
}

int Provider::GetBytesPerSample()
{
	return m_bytesPerSample;
}

int Provider::GetChannels()
{
	return m_channels;
}

long long Provider::GetNumSamples()
{
	return m_numSamples;
}

void Provider::GetWaveForm(int* min, int* peak, long long start, int w, int h, int samples, float scale) {
	if (audioNotInitialized) { return; }
	int n = w * samples;
	for (int i = 0; i < w; i++) {
		peak[i] = 0;
		min[i] = h;
	}

	// Prepare waveform
	int cur;
	int curvalue;

	// Prepare buffers
	int needLen = n * m_bytesPerSample;
	if (needLen <= 0)
		return;

	char* raw = new char[needLen];
	short* raw_short = reinterpret_cast<short*>(raw);
	GetBuffer(raw, start, n);
	int half_h = h / 2;
	int half_amplitude = int(half_h * scale);
	// Calculate waveform
	for (int i = 0; i < n; i++) {
		cur = i / samples;
		curvalue = half_h - (int(raw_short[i]) * half_amplitude) / 0x8000;
		if (curvalue > h) curvalue = h;
		if (curvalue < 0) curvalue = 0;
		if (curvalue < min[cur]) min[cur] = curvalue;
		if (curvalue > peak[cur]) peak[cur] = curvalue;
	}

	delete[] raw;

}

void Provider::RunPlaybackThread()
{
	HANDLE events_to_wait[] = {
		m_eventStartPlayback,
		m_eventSetPosition,
		m_eventKillSelf
	};
	int tdiff = 0;

	while (1) {
		DWORD wait_result = WaitForMultipleObjects(sizeof(events_to_wait) / sizeof(HANDLE), events_to_wait, FALSE, INFINITE);

		if (wait_result == WAIT_OBJECT_0 + 0)
		{
			unsigned char* buff = m_renderer->m_FrameBuffer;
			int acttime;
			while (1) {
				if (WaitForSingleObject(m_eventKillSelf, 0) == WAIT_OBJECT_0) { return; }

				if (m_renderer->m_Frame != m_lastFrame) {
					m_renderer->m_Time = m_timebase.MsAt(m_renderer->m_Frame);
					m_lastFrame = m_renderer->m_Frame;
				}
				if (!FetchPlaybackFrame(buff)) {
					// Retrying a failing fetch would spin the thread without ever
					// looking at the stop or kill event again, so end playback.
					wxCommandEvent* evt = new wxCommandEvent(wxEVT_COMMAND_BUTTON_CLICKED, ID_END_OF_STREAM);
					wxQueueEvent(m_renderer->videoControl, evt);
					break;
				}

				m_renderer->DrawTexture(buff);
				m_renderer->Render(false);

				if (m_renderer->m_Time >= m_renderer->m_PlayEndTime || 
					m_renderer->m_Frame >= m_numFrames - 1) {
					wxCommandEvent* evt = new wxCommandEvent(wxEVT_COMMAND_BUTTON_CLICKED, ID_END_OF_STREAM);
					wxQueueEvent(m_renderer->videoControl, evt);
					break;
				}
				else if (m_renderer->m_State != Playing) {
					break;
				}
				acttime = timeGetTime() - m_renderer->m_LastTime;

				m_renderer->m_Frame++;
				m_renderer->m_Time = m_timebase.MsAt(m_renderer->m_Frame);

				tdiff = m_renderer->m_Time - acttime;

				if (tdiff > 0) { Sleep(tdiff); }
				else if (tdiff < -20) {
					while (1) {
						if (m_renderer->m_Frame >= m_numFrames) {
							m_renderer->m_Frame = m_numFrames - 1;
							m_renderer->m_Time = m_renderer->m_PlayEndTime;
							break;
						}
						int frameTime = m_timebase.MsAt(m_renderer->m_Frame);
						if (frameTime >= acttime || frameTime >= m_renderer->m_PlayEndTime) {
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
			m_renderer->SetFFMS2Position(m_changedTime, m_isStartTime, m_refreshAudio);
		}
		else {
			break;
		}

	}
}

void Provider::SetPosition(int time, bool starttime, bool refteshAudio/* = true*/)
{
	m_changedTime = time;
	m_isStartTime = starttime;
	m_refreshAudio = refteshAudio;
	SetEvent(m_eventSetPosition);
}

