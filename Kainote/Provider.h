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

#pragma once

#include "RendererVideo.h"
#include "KainoteFrame.h"
#include "MisspellReplacer.h"
#include "SubsGrid.h"
#include "Provider.h"
#include "TabPanel.h"
#include <vector>
#include <thread>
#include "../Thirdparty/ffms2/include/ffms.h"
#ifndef FFMS_CS_UNSPECIFIED
#define FFMS_CS_UNSPECIFIED 0
#define FFMS_CS_RGB 0
#define FFMS_CS_BT709 1
#define FFMS_CS_FCC 4
#define FFMS_CS_BT470BG 5
#define FFMS_CS_SMPTE170M 6
#define FFMS_CS_SMPTE240M 7
#endif
class chapter;
class RendererVideo;

class Provider
{
	friend class RendererFFMS2;
public:
	static Provider* Get(const wxString& filename, RendererVideo* renderer, 
		wxWindow* progressSinkWindow, bool* success);
	virtual ~Provider();
	virtual void GetFrameBuffer(unsigned char** buffer) {};
	virtual void GetFrame(int frame, unsigned char* buff) {};
	virtual void GetBuffer(void* buf, long long start, long long count, double vol = 1.0) {};
	virtual void GetChapters(std::vector<chapter>* _chapters) {}
	virtual void DeleteOldAudioCache() {};
	virtual void SetColorSpace(const wxString& matrix) {};
	virtual bool HasVideo() { return false; };

	void Play();
	int GetSampleRate();
	int GetBytesPerSample();
	int GetChannels();
	long long GetNumSamples();
	void GetWaveForm(int* min, int* peak, long long start, int w, int h, int samples, float scale);
	int TimefromFrame(int nframe);
	int FramefromTime(int time);
	int GetMSfromFrame(int frame);
	int GetFramefromMS(int MS, int seekfrom = 0, bool safe = true);
	const wxArrayInt& GetKeyframes() { return m_keyFrames; };
	const std::vector<int> GetTimecodes() { return m_timecodes; };
	void SetKeyframes(const wxArrayInt& keyframes) {
		m_keyFrames = keyframes;
	}
	void SetTimecodes(const std::vector<int>& timecodes) {
		m_timecodes = timecodes;
	}
	float GetFPS() { return m_FPS; }
	void SetFPS(float FPS) { m_FPS = FPS; }
	long long GetNumFrames() { return m_numFrames; }
	void SetNumFrames(long long numFrames) { m_numFrames = numFrames; }
	void OpenKeyframes(const wxString& filename);
	void SetPosition(int time, bool starttime, bool refteshAudio = true);
	bool AudioNotInitialized() {
		return audioNotInitialized;
	}
	float GetAudioProgress() {
		return m_audioProgress;
	}
protected:
	Provider(const wxString& filename, RendererVideo* renderer);
	volatile bool audioNotInitialized = true;
	volatile float m_audioProgress = 0;
	RendererVideo* m_renderer = nullptr;
	int m_width = -1;
	int m_height = -1;
	int m_arwidth = -1;
	int m_arheight = -1;
	int m_numFrames = 0;
	int m_sampleRate = -1;
	int m_bytesPerSample = 0;
	int m_channels = 0;
	int m_lastTime = 0;
	int m_lastFrame = -1;
	int m_framePlane = 0;
	int m_changedTime = 0;
	bool m_isStartTime = false;
	bool m_refreshAudio = true;
	double m_duration = 0;
	float m_FPS = 0;
	long long m_numSamples = 0;
	HANDLE m_thread = nullptr;
	HANDLE m_eventStartPlayback = nullptr;
	HANDLE m_eventSetPosition = nullptr;
	HANDLE m_eventKillSelf = nullptr;
	HANDLE m_eventComplete = nullptr;
	wxString m_filename;
	wxArrayInt m_keyFrames;
	std::vector<int> m_timecodes;
};