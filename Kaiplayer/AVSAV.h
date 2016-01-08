#ifndef AVSAUDIO
#define AVSAUDIO

#include <wx/wx.h>
#include <wx/filename.h>
#include <stdint.h>
#include <windows.h>
#include "avisynth.h"


typedef IScriptEnvironment* __stdcall FUNC(int);

class AVSProvider{
	private:
	char **Cache;
	HINSTANCE hLib;
	wxString filename;
	PClip clip;
	
	int width;
	int height;
	int arwidth;
	int arheight;
	int NumFrames;
	double Duration;
	double Delay;
	float fps;

	int blnum;
	int SampleRate;
	int BytesPerSample;
	int Channels;
	int64_t NumSamples;

	bool LoadFromClip(AVSValue clip);
	bool OpenAVSAudio();
	void Clearcache();
	void Cleardiskc();

	wxMutex AudioMutex;
	wxMutex VideoMutex;

protected:
	IScriptEnvironment *env;
public:

	int GetSampleRate();
	int GetBytesPerSample();
	int GetChannels();
	int64_t GetNumSamples();
	void GetFrame(int frame, byte* buff);
	int TimefromFrame(int nframe);
	int FramefromTime(int time);
	int GetMSfromFrame(int frame);
	int GetFramefromMS(int MS, int seekfrom=1);
	bool DiskCache();
	bool CacheIt();

	void GetBuffer(void *buf, int64_t start, int64_t count);
	void GetAudio(void *buf, int64_t start, int64_t count);
	void GetWaveForm(int *min,int *peak,int64_t start,int w,int h,int samples,float scale);

	AVSProvider(wxString filename, int tab, bool *success);
	~AVSProvider();
};

#endif