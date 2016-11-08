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

#ifndef AUDIO_SPECTRUM_H
#define AUDIO_SPECTRUM_H

#include <wx/wxprec.h>
#include <stdint.h>
#include "VideoFfmpeg.h"

class FFT;
// Specified and implemented in cpp file, interface is private to spectrum code
//class AudioSpectrumCacheManager;
//class SpectrumThread;
class FinalSpectrumCache;

class AudioSpectrum {
	friend class SpectrumThread;
private:
	// Data provider
	//AudioSpectrumCacheManager *cache;
	SpectrumThread *cache;

	// Colour pallettes
	unsigned char colours_normal[256*3];
	unsigned char colours_selected[256*3];

	

	VideoFfmpeg *provider;
	

	unsigned long line_length; // number of frequency components per line (half of number of samples)
	unsigned long num_lines; // number of lines needed for the audio
	unsigned int fft_overlaps; // number of overlaps used in FFT
	float power_scale; // amplification of displayed power
	int minband; // smallest frequency band displayed
	int maxband; // largest frequency band displayed
	int subcachelen;
	FFT *fft;
	wxCriticalSection CritSec;
	void SetupSpectrun(int overlaps = 1, int length = (1<<7));
public:
	AudioSpectrum(VideoFfmpeg *_provider);
	~AudioSpectrum();
	

	void RenderRange(int64_t range_start, int64_t range_end, bool selected, unsigned char *img, int imgwidth, int imgheight, int percent);

	void SetScaling(float _power_scale);
};

typedef std::vector<float> CacheLine;
class SpectrumThread
{
	friend class AudioSpectrum;
public:
	SpectrumThread(AudioSpectrum *spc, size_t numsubcaches, size_t overlaps);
	~SpectrumThread();
	void MakeSubCaches(size_t start, size_t bufstart, size_t len, size_t buflen, unsigned char *img, int imgwidth, int imgheight, unsigned char *palette);
	CacheLine &GetLine(unsigned long i, unsigned int overlap);
	//void Age();
	void Wait();
private:
	void procincls(int numthread);
	AudioSpectrum *spc;
	size_t start;
	size_t first_line;
	size_t last_line;
	size_t length;
	size_t numsubcaches;
	size_t overlaps;
	int imgwidth;
	int imgheight;
	unsigned char *palette;
	unsigned char *img;
	std::vector<FinalSpectrumCache*> sub_caches;
	
};

#endif
