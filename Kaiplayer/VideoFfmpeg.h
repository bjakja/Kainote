//  Copyright (c) 2016, Marcin Drob

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

#ifndef VIDEOFFMPEG
#define VIDEOFFMPEG


#pragma once
#include <wx/string.h>
#include <wx/file.h>
#include <vector>
//#include <stdint.h>
#include "include\ffms.h"
#include "ProgressDialog.h"

class VideoRend;

class VideoFfmpeg
{
public:
	//VideoFfmpeg(const wxString &filename, bool *success);
	//test
	VideoFfmpeg(const wxString &filename, VideoRend *renderer, bool *success);
	static unsigned int __stdcall FFMS2Proc(void* cls);
	void Processing();
	void Refresh(bool wait=true);
	void Play(){SetEvent(eventStartPlayback);};
	volatile bool success;
	wxString fname;
	VideoRend *rend;
	HANDLE thread;
	HANDLE eventStartPlayback,
		eventRefresh,
		eventKillSelf,
		eventComplete;
	//endtest
	~VideoFfmpeg();
	void GetFrame(int frame, byte* buff);
	void GetBuffer(void *buf, int64_t start, int64_t count, double vol=1.0);
	void GetWaveForm(int *min,int *peak,int64_t start,int w,int h,int samples,float scale);
	
	
	int width;
	int height;
	int arwidth;
	int arheight;
	int NumFrames;
	double Duration;
	double Delay;
	float fps;
	
	int GetSampleRate();
	int GetBytesPerSample();
	int GetChannels();
	int64_t GetNumSamples();
	bool CacheIt();
	int TimefromFrame(int nframe);
	int FramefromTime(int time);
	int lasttime;
	int lastframe;
	wxArrayInt KeyFrames;
	std::vector<int> Timecodes;
	int GetMSfromFrame(int frame);
	int GetFramefromMS(int MS, int seekfrom=0);
	int Init();

	//bool com_inited;
	ProgressSink *progress;
	static int __stdcall UpdateProgress(int64_t Current, int64_t Total, void *ICPrivate);
	void Clearcache();
	FFMS_VideoSource *videosource;
	FFMS_AudioSource *audiosource;
	FFMS_ErrorInfo errinfo;
	const FFMS_Frame *fframe;
	int SampleRate;
	int BytesPerSample;
	int Channels;

	
	int64_t NumSamples;
	
	
	
	wxString diskCacheFilename;
	wxFile file_cache;
	bool DiskCache();
	void Cleardiskc();
	bool disccache;
	wxMutex blockaudio;
	wxMutex blockvideo;
	
private:
	
	char **Cache;
	int blnum;
	void GetAudio(void *buf, int64_t start, int64_t count);
	void DeleteOldAudioCache();
	
};

#endif