#ifndef AVSAUDIO
#define AVSAUDIO

#include <wx/wxprec.h>
#include <wx/filename.h>
#include <stdint.h>
#include <windows.h>
#include "avisynth.h"


typedef IScriptEnvironment* __stdcall FUNC(int);

class ffmpeg2{
	private:
	char **Cache;
	HINSTANCE hLib;
	wxString filename;
	PClip clip;
	
	int blnum;
	int SampleRate;
	int BytesPerSample;
	int Channels;
	int64_t NumSamples;

	bool LoadFromClip(AVSValue clip);
	bool OpenAVSAudio();
	void Clearcache();
	
	wxMutex AviSynthMutex;

protected:
	IScriptEnvironment *env;
public:

	int GetSampleRate();
	int GetBytesPerSample();
	int GetChannels();
	int64_t GetNumSamples();

	bool CacheIt();

	void GetBuffer(void *buf, int64_t start, int64_t count);
	void GetAudio(void *buf, int64_t start, int64_t count);
	void GetWaveForm(int *min,int *peak,int64_t start,int w,int h,int samples,float scale);

	ffmpeg2(wxString filename, int tab, bool *success);
	~ffmpeg2();
	};

#endif