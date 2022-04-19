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

#pragma once

#include "RendererVideo.h"
#include <vector>
#include <thread>
#include "include\ffms.h"

class chapter;


class Provider
{
	friend class RendererFFMS2;
public:
	static Provider* Get(const wxString& filename, RendererVideo* renderer, 
		wxWindow* progressSinkWindow, bool* success);
	virtual ~Provider();
	virtual void GetFrameBuffer(byte** buffer) {};
	virtual void GetFrame(int frame, byte* buff) {};
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
	void SetPosition(int time, bool starttime);
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
	int m_height;
	int m_arwidth;
	int m_arheight;
	int m_numFrames;
	int m_sampleRate = -1;
	int m_bytesPerSample;
	int m_channels;
	int m_lastTime;
	int m_lastFrame = -1;
	int m_framePlane = 0;
	int m_changedTime = 0;
	bool m_isStartTime = false;
	double m_duration = 0;
	float m_FPS;
	long long m_numSamples;
	HANDLE m_thread = nullptr;
	HANDLE m_eventStartPlayback,
		m_eventSetPosition,
		m_eventKillSelf,
		m_eventComplete;
	wxString m_filename;
	wxArrayInt m_keyFrames;
	std::vector<int> m_timecodes;
};