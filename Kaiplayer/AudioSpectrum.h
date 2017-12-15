// Copyright (c) 2005, 2006, Rodrigo Braz Monteiro
// Copyright (c) 2006, 2007, Niels Martin Hansen
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of the Aegisub Group nor the names of its contributors
//     may be used to endorse or promote products derived from this software
//     without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// -----------------------------------------------------------------------------
//
// AEGISUB
//
// Website: http://aegisub.cellosoft.com
// Contact: mailto:zeratul@cellosoft.com
//

#pragma once

#include <wx/wxprec.h>
#include <stdint.h>
#include "GFFT/GFFT.h"
#include "VideoFfmpeg.h"


class FinalSpectrumCache;
class AudioSpectrumMultiThreading;

typedef std::vector<float> CacheLine;

class AudioSpectrum {
	friend class SpectrumThread;
private:

	// Colour pallettes
	unsigned char colours_normal[256*3];

	VideoFfmpeg *provider;
	
	unsigned long num_lines; // number of lines needed for the audio
	unsigned int fft_overlaps; // number of overlaps used in FFT
	float power_scale; // amplification of displayed power
	int minband; // smallest frequency band displayed
	int maxband; // largest frequency band displayed
	int subcachelen;
	size_t numsubcaches;
	wxCriticalSection CritSec;
	void SetupSpectrum(int overlaps = 1);
	std::vector<FinalSpectrumCache*> sub_caches;
	AudioSpectrumMultiThreading *AudioThreads;
public:
	AudioSpectrum(VideoFfmpeg *_provider);
	~AudioSpectrum();
	

	void RenderRange(int64_t range_start, int64_t range_end, bool selected, unsigned char *img, int imgleft, int imgwidth, int imgpitch, int imgheight, int percent);

	void SetScaling(float _power_scale);
	void ChangeColours();
};

class AudioSpectrumMultiThreading
{
public:
	AudioSpectrumMultiThreading(unsigned long _subcachelen, VideoFfmpeg *provider);
	~AudioSpectrumMultiThreading();
	void SetCache(std::vector<FinalSpectrumCache*> *_sub_caches, unsigned int _overlaps){ sub_caches = _sub_caches; overlaps = _overlaps; }
	void CreateCache(unsigned long _start, unsigned long _end);
private:
	static unsigned int __stdcall AudioProc(void* cls);
	void AudioPorocessing(int numOfTread);
	void SetAudio(unsigned long start, int len, FFT *fft);
	std::vector<FinalSpectrumCache*> *sub_caches;
	unsigned long start, end, len, subcachelen;
	unsigned int overlaps;
	int numThreads;
	FFT *ffttable = NULL;
	HANDLE *threads=NULL;
	HANDLE *eventCacheCopleted = NULL;
	HANDLE *eventMakeCache = NULL;
	HANDLE eventKillSelf;
	static AudioSpectrumMultiThreading *sthread;
};

