#ifndef VIDEOFFMPEG
#define VIDEOFFMPEG


#pragma once
#include <wx/string.h>
#include <wx/file.h>
#include <vector>
//#include <stdint.h>
#include "include\ffms.h"
#include "ProgressDialog.h"


class VideoFfmpeg
{
public:
	VideoFfmpeg(const wxString &filename, int tab, bool *success);
	~VideoFfmpeg();
	void GetFrame(int frame, byte* buff);
	void GetBuffer(void *buf, int64_t start, int64_t count, double vol=1.0);
	void GetWaveForm(int *min,int *peak,int64_t start,int w,int h,int samples,float scale);
	//void GetFFT(CacheLine &cache, int start, int count);
	//FFT *fft;
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
	int GetFramefromMS(int MS, int seekfrom=1);
	int Init(const wxString &filename, int tab);

	//bool com_inited;
	ProgresDialog *progress;
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
	
};

#endif