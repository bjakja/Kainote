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
#include "Provider.h"


class SpectrumCache;
class AudioSpectrumMultiThreading;

typedef std::vector<float> CacheLine;


class AudioSpectrum {
	friend class SpectrumThread;
private:

	// Colour pallettes
	unsigned char palette[256 * 3];

	Provider *provider;
	//unsigned int fft_overlaps; // number of overlaps used in FFT
	float power_scale; // amplification of displayed power
	int minband; // smallest frequency band displayed
	int maxband; // largest frequency band displayed
	bool nonlinear = true;
	wxCriticalSection CritSec;
	void SetupSpectrum(int overlaps = 1);
	std::vector<SpectrumCache*> sub_caches;
	AudioSpectrumMultiThreading *AudioThreads;
public:
	AudioSpectrum(Provider *_provider);
	~AudioSpectrum();
	
	void RenderRange(int64_t range_start, int64_t range_end, unsigned char *img, int imgwidth, int imgpitch, int imgheight, int percent);
	void CreateRange(std::vector<int> &output, std::vector<int> &intensities, int64_t timeStart, int64_t timeEnd, wxPoint frequency, int peek);
	void SetScaling(float _power_scale);
	void ChangeColours();
	void SetNonLinear(bool _nonlinear){ nonlinear = _nonlinear; }
};

class AudioSpectrumMultiThreading
{
public:
	AudioSpectrumMultiThreading(Provider *provider, std::vector<SpectrumCache*> *_sub_caches);
	~AudioSpectrumMultiThreading();
	//void SetCache(unsigned int _overlaps){overlaps = _overlaps; }
	void CreateCache(unsigned long _start, unsigned long _end);
	unsigned long start=0, end=0, lastCachePosition = 0;
	void FindCache(unsigned long _start, unsigned long _end);
	int numThreads;
private:
	static unsigned int __stdcall AudioProc(void* cls);
	void AudioPorocessing(int numOfTread);
	void SetAudio(unsigned long start, int len, FFT *fft);
	std::vector<SpectrumCache*> *sub_caches;
	unsigned long len;
	//unsigned int overlaps;
	FFT *ffttable = NULL;
	HANDLE *threads=NULL;
	HANDLE *eventCacheCopleted = NULL;
	HANDLE *eventMakeCache = NULL;
	HANDLE eventKillSelf;
	static AudioSpectrumMultiThreading *sthread;
};

