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


#pragma once

#include <wx/string.h>
#include <vector>
#include <thread>
#include "include\ffms.h"
#include "ProgressDialog.h"

class VideoRend;

class VideoFfmpeg
{
public:
	VideoFfmpeg(const wxString &filename, VideoRend *renderer, bool *success);
	static unsigned int __stdcall FFMS2Proc(void* cls);
	void Processing();
	void Refresh(bool wait=true);
	void Play(){SetEvent(eventStartPlayback);};
	~VideoFfmpeg();
	void GetFrame(int frame, byte* buff);
	void GetBuffer(void *buf, int64_t start, int64_t count, double vol=1.0);
	void GetWaveForm(int *min,int *peak,int64_t start,int w,int h,int samples,float scale);
	
	int GetSampleRate();
	int GetBytesPerSample();
	int GetChannels();
	int64_t GetNumSamples();
	bool RAMCache();
	int TimefromFrame(int nframe);
	int FramefromTime(int time);
	int GetMSfromFrame(int frame);
	int GetFramefromMS(int MS, int seekfrom=0);
	int Init();

	//bool com_inited;
	ProgressSink *progress;
	static int __stdcall UpdateProgress(int64_t Current, int64_t Total, void *ICPrivate);
	bool CreateAudioSource(int audioTrack);
	static void AudioLoad(VideoFfmpeg *parent, bool newIndex, int audiotrack);
	void Clearcache();
	FFMS_VideoSource *videosource;
	FFMS_AudioSource *audiosource;
	FFMS_ErrorInfo errinfo;
	FFMS_Index *index;
	FFMS_Index *audioIndex = NULL;
	const FFMS_Frame *fframe;
	
	bool DiskCache(bool audioExist=false);
	void Cleardiskc();
	void DeleteOldAudioCache();
	wxString ColorCatrixDescription(int cs, int cr);
	void SetColorSpace(const wxString& matrix);
	bool disccache;
	volatile bool success;
	volatile bool isBusy;
	volatile bool audioNotInitialized = true;
	volatile float audioProgress = 0;
	int width;
	int height;
	int arwidth;
	int arheight;
	int NumFrames;
	int CR;
	int CS;
	int SampleRate=-1;
	int BytesPerSample;
	int Channels;
	int lasttime;
	int lastframe;
	double Duration;
	double Delay;
	float fps;
	int64_t NumSamples;
	wxMutex blockaudio;
	wxMutex blockvideo;
	HANDLE thread;
	HANDLE eventStartPlayback,
		eventRefresh,
		eventKillSelf,
		eventComplete,
		eventAudioComplete;
	wxString diskCacheFilename;
	wxString ColorSpace;
	wxString RealColorSpace;
	wxString fname;
	wxString indexPath;
	VideoRend *rend;
	//wxFile file_cache;
	FILE *fp;
	wxArrayInt KeyFrames;
	std::vector<int> Timecodes;
	std::thread *audioLoadThread = NULL;
private:
	
	char **Cache;
	int blnum;
	void GetAudio(void *buf, int64_t start, int64_t count);
	
	
};

