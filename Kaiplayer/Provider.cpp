
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

#include "Provider.h"
#include "ProviderDummy.h"
#include "ProviderFFMS2.h"
#include "KeyframesLoader.h"
#include "KaiMessageBox.h"
#include "Tabs.h"
#include "TabPanel.h"

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

int64_t Provider::GetNumSamples()
{
	return m_numSamples;
}

void Provider::GetWaveForm(int* min, int* peak, int64_t start, int w, int h, int samples, float scale) {
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

	void* raw;
	raw = new char[needLen];


	short* raw_short = (short*)raw;
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

int Provider::TimefromFrame(int nframe)
{
	if (nframe < 0) { nframe = 0; }
	if (nframe >= m_numFrames) { nframe = m_numFrames - 1; }
	return m_timecodes[nframe];
}

int Provider::FramefromTime(int time)
{
	if (time <= 0) { return 0; }
	int start = m_lastFrame;
	if (m_lastTime > time)
	{
		start = 0;
	}
	int wframe = m_numFrames - 1;
	for (int i = start; i < m_numFrames - 1; i++)
	{
		if (m_timecodes[i] >= time && time < m_timecodes[i + 1])
		{
			wframe = i;
			break;
		}
	}
	m_lastFrame = wframe;
	m_lastTime = time;
	return m_lastFrame;
}

int Provider::GetMSfromFrame(int frame)
{
	if (frame >= m_numFrames) { return frame * (1000.f / m_FPS); }
	else if (frame < 0) { return 0; }
	return m_timecodes[frame];
}

int Provider::GetFramefromMS(int MS, int seekfrom, bool safe)
{
	if (MS <= 0) return 0;
	int result = (safe) ? m_numFrames - 1 : m_numFrames;
	for (int i = seekfrom; i < m_numFrames; i++)
	{
		if (m_timecodes[i] >= MS)
		{
			result = i;
			break;
		}
	}
	return result;
}

void Provider::OpenKeyframes(const wxString& filename)
{
	wxArrayInt keyframes;
	KeyframeLoader kfl(filename, &keyframes, this);
	if (keyframes.size()) {
		m_keyFrames = keyframes;
		TabPanel* tab = (m_renderer) ? (TabPanel*)m_renderer->videoControl->GetParent() : Notebook::GetTab();
		if (tab->Edit->ABox) {
			tab->Edit->ABox->SetKeyframes(keyframes);
		}
	}
	else {
		KaiMessageBox(_("Nieprawid³owy format klatek kluczowych"), _("B³¹d"), 4L, Notebook::GetTab());
	}
}

void Provider::SetPosition(int time, bool starttime)
{
	m_changedTime = time;
	m_isStartTime = starttime;
	SetEvent(m_eventSetPosition);
}
