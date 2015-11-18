#ifndef FFMPEGS2
#define FFMPEGS2

#include <wx/wx.h>
#include <wx/progdlg.h>
#include "include/ffms.h"

class ffmpeg2
	{
	public:
		ffmpeg2(wxString filename, int tab, bool *success);
		bool GetSource(wxString filename, int tab);
		~ffmpeg2();
		void GetBuffer(void *buf, int64_t Start, int64_t Count, bool player=true);
		int GetSampleRate();
		int GetBytesPerSample();
		int GetChannels();
		int64_t GetNumSamples();
		void DownMix(void *buff, int64_t Start, int64_t Count);
		void DownMixMono(void *buf, int64_t Start, int64_t Count);
		void GetWaveForm(int *min,int *peak,int64_t start,int w,int h,int samples,float scale);

	private:
		
		FFMS_AudioSource *audiosource;
		FFMS_ErrorInfo errinfo;
		bool com_inited;
		int SampleRate;
		int BytesPerSample;
		int Channels;
		int64_t NumSamples;
		wxCriticalSection CritSec;
	};

#endif