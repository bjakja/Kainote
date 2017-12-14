// Copyright (c) 2005, 2006, Rodrigo Braz Monteiro
// Copyright (c) 2006, 2007, Niels Martin Hansen
// Copyright(c) 2014, 2017, Marcin Drob
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


#include "Config.h"

#include <assert.h>
#include <vector>
#include <list>
#include <utility>
#include <algorithm>
#include "AudioSpectrum.h"

#include "ColorSpace.h"
#include <wx/log.h>
#include <process.h>



class FinalSpectrumCache{
private:
	std::vector<CacheLine> data;
	unsigned long start; 

public:
	CacheLine& GetLine(unsigned long i, unsigned int overlap)
	{

		// This check ought to be redundant
		if (i >= start && i-start < length)
			return data[(i-start) + overlap*length];
		else{
			return null_line;}
	}


	FinalSpectrumCache(FFT *fft, unsigned long _start)
	{
		start = _start;

		// Add an upper limit to number of overlaps or trust user to do sane things?
		// Any limit should probably be a function of length

		// First fill the data vector with blanks
		// Both start and end are included in the range stored, so we have end-start+1 elements
		data.resize(length*overlaps, null_line);

		unsigned int overlap_offset = line_length / overlaps * 2; // FIXME: the result seems weird/wrong without this factor 2, but why?

		int doublelen=line_length*2;
		int64_t sample=start;

		for (unsigned long i = 0; i < length; ++i) {
			// Start sample number of the next line calculated
			// line_length is half of the number of samples used to calculate a line, since half of the output from
			// a Fourier transform of real data is redundant, and not interesting for the purpose of creating
			// a frequenmcy/power spectrum.
			
			for (unsigned int overlap = 0; overlap < overlaps; ++overlap) {
				// Initialize
				sample = (start * doublelen) + (overlap*overlap_offset) + (i*doublelen);
				
				fft->Transform(sample);
				
				CacheLine &line = data[i + length*overlap];
				
				for (size_t j = 0; j < line_length; ++j) {
					//line[j] = sqrt(fft->output_r[j]*fft->output_r[j] +
						//fft->output_i[j]*fft->output_i[j]);
					int g = (j *2) + 1;//(line_length);
					//float ns = (fft->output[j] * fft->output[j]) + (fft->output[g] * fft->output[g]);
					line[j] = sqrt(fft->output[j*2] * fft->output[j*2] + fft->output[g] * fft->output[g]);
				}
					
			}
				
				
			
		}
		
	}

	static void SetLineLength(unsigned long _length, unsigned int _overlaps)
	{
		length = _length;
		overlaps = _overlaps;
		null_line.resize(line_length, 0);
	}

	~FinalSpectrumCache()
	{
	}

	static unsigned long length;
	static unsigned int overlaps;
	static CacheLine null_line;
};

CacheLine FinalSpectrumCache::null_line;
unsigned long FinalSpectrumCache::length;
unsigned int FinalSpectrumCache::overlaps;

// AudioSpectrum
AudioSpectrum::AudioSpectrum(VideoFfmpeg *_provider)
{
	provider = _provider;
	subcachelen = 16;
	power_scale = 1;
	int64_t _num_lines = provider->GetNumSamples()+doublelen / doublelen;
	num_lines = (unsigned long)_num_lines;
	numsubcaches = (num_lines + subcachelen)/subcachelen;
	AudioThreads = new AudioSpectrumMultiThreading(line_length, subcachelen, provider);
	SetupSpectrum();
	ChangeColours();
	minband = 0;//Options.GetInt(_T("Audio Spectrum Cutoff"));
	maxband = line_length - minband * 2/3; // TODO: make this customisable?
	sub_caches.resize(numsubcaches, 0);
}


AudioSpectrum::~AudioSpectrum()
{
	delete AudioThreads;
	for (size_t i = 0; i < numsubcaches; ++i){
		if (sub_caches[i]) delete sub_caches[i];
	}
}

void AudioSpectrum::SetupSpectrum(int overlaps)
{

	fft_overlaps = overlaps;

	FinalSpectrumCache::SetLineLength(subcachelen, fft_overlaps);
	AudioThreads->SetCache(&sub_caches, fft_overlaps);
}

void AudioSpectrum::RenderRange(int64_t range_start, int64_t range_end, bool selected, unsigned char *img, int imgleft, int imgwidth, int imgpitch, int imgheight, int parcent)
{
    wxCriticalSectionLocker locker(CritSec);
	
    float parcPow = pow((150-parcent)/100.0f, 8);
	int parc = (parcPow * (imgwidth/500.f));
	if(parc<1){parc=1;}
	if(parc>10){parc=10;}
	unsigned long first_line = (unsigned long)(parc * range_start / line_length / 2);
	unsigned long last_line = (unsigned long)(parc * range_end / line_length / 2);
	unsigned long startcache = first_line / subcachelen;
	unsigned long endcache = last_line / subcachelen;
	if(parc!=fft_overlaps){ 
		for (size_t i = 0; i < numsubcaches; ++i){
			if (sub_caches[i]){
				delete sub_caches[i];
				sub_caches[i] = NULL;
			}
		}
		SetupSpectrum(parc);
	}
	else{
		for (size_t i = 0; i < numsubcaches; ++i){
			if (sub_caches[i] && (i < startcache || i> endcache)) {
				delete sub_caches[i]; 
				sub_caches[i] = NULL;
			}
		}
	}

	int last_imgcol_rendered = -1;

	unsigned char *palette=colours_normal;
	

	// Some scaling constants
	const int maxpower = (1 << (16 - 1))*256;

	const double upscale = power_scale * 16384 / line_length;
	AudioThreads->CreateCache(startcache, endcache);
	//const double onethirdmaxpower = maxpower / 3, twothirdmaxpower = maxpower * 2/3;
	//const double logoverscale = log(maxpower*upscale - twothirdmaxpower);

	// Note that here "lines" are actually bands of power data
	unsigned long baseline = first_line / fft_overlaps;
	unsigned int overlap = first_line % fft_overlaps;
	//unsigned int j = 0;
	unsigned int start = baseline / subcachelen;
	for (unsigned long i = first_line; i <= last_line; ++i) {
		// Handle horizontal compression and don't unneededly re-render columns
		int imgcol = imgleft + imgwidth * (i - first_line) / (last_line - first_line + 1);
		if (imgcol <= last_imgcol_rendered)
			continue;
		size_t subcache = i / subcachelen;
		baseline = i / fft_overlaps;
		overlap = i % fft_overlaps;
			
		CacheLine &line=sub_caches[subcache]->GetLine(baseline, overlap);

#define WRITE_PIXEL \
	if (intensity < 0) intensity = 0; \
	if (intensity > 255) intensity = 255; \
	img[((imgheight-y-1)*imgpitch+x)*4 + 2] = palette[intensity*3+0]; \
	img[((imgheight-y-1)*imgpitch+x)*4 + 1] = palette[intensity*3+1]; \
	img[((imgheight-y-1)*imgpitch+x)*4 + 0] = palette[intensity*3+2];

		// Handle horizontal expansion
		int next_line_imgcol = imgleft + imgwidth * (i - first_line + 1) / (last_line - first_line + 1);
		if (next_line_imgcol >= imgpitch)
			next_line_imgcol = imgpitch-1;

		for (int x = imgcol; x <= next_line_imgcol; ++x) {

			// Decide which rendering algo to use
			if (maxband - minband > imgheight) {
				// more than one frequency sample per pixel (vertically compress data)
				// pick the largest value per pixel for display
				// Iterate over pixels, picking a range of samples for each
				for (int y = 0; y < imgheight; ++y) {
					int sample1 = MAX(0,maxband * y/imgheight + minband);
					int sample2 = MIN(signed(line_length-1),maxband * (y+1)/imgheight + minband);
					float maxval = 0;
					for (int samp = sample1; samp <= sample2; samp++) {
						if (line[samp] > maxval) maxval = line[samp];
					}
					int intensity = int(256 * (maxval * upscale) / maxpower);
					WRITE_PIXEL
				}
			}
			else {
				// less than one frequency sample per pixel (vertically expand data)
				// interpolate between pixels
				// can also happen with exactly one sample per pixel, but how often is that?

				// Iterate over pixels, picking the nearest power values
				for (int y = 0; y < imgheight; ++y) {
					float ideal = (float)(y+1.)/imgheight * maxband;
					float sample1 = line[(int)floor(ideal)+minband] * upscale;
					float sample2 = line[(int)ceil(ideal)+minband] * upscale;
					float frac = ideal - floor(ideal);
					int intensity = int(((1-frac)*sample1 + frac*sample2) / maxpower * 256);
					WRITE_PIXEL
				}
			}
		}

#undef WRITE_PIXEL

	}
	
}


void AudioSpectrum::SetScaling(float _power_scale)
{
	power_scale = _power_scale;
}

void AudioSpectrum::ChangeColours()
{
	const wxColour & firstcolor = Options.GetColour(AudioSpectrumBackground); 
	const wxColour & secondcolor = Options.GetColour(AudioSpectrumEcho); 
	const wxColour & thirdcolor = Options.GetColour(AudioSpectrumInner); 
	float r2=thirdcolor.Red(), r1= secondcolor.Red(), r= firstcolor.Red(), 
		g2=thirdcolor.Green(), g1=secondcolor.Green(), g=firstcolor.Green(), 
		b2=thirdcolor.Blue(), b1=secondcolor.Blue(), b=firstcolor.Blue();
	
	// Generate colour maps
	unsigned char *palptr = colours_normal;
	float div = (1.f/128.f);
	float i = 0;
	int j = 0;
	while (j < 256) {
		int pointr = (i<0.5f)? (r - (( r - r1) * (i*2))) : (r1 - (( r1 - r2) * ((i*2)-1.f)));
		int pointg = (i<0.5f)? (g - (( g - g1) * (i*2))) : (g1 - (( g1 - g2) * ((i*2)-1.f)));
		int pointb = (i<0.5f)? (b - (( b - b1) * (i*2))) : (b1 - (( b1 - b2) * ((i*2)-1.f)));

		*palptr = (unsigned char)pointr;
		*(palptr+1) = (unsigned char)pointg;
		*(palptr+2) = (unsigned char)pointb;
		
		if(j<128){i += div;}
		
		palptr += 3;
		j++;
	}

}

AudioSpectrumMultiThreading::AudioSpectrumMultiThreading(unsigned long _line_length, unsigned long _subcachelen, VideoFfmpeg *provider)
{
	line_length = _line_length; 
	subcachelen = _subcachelen;
	sthread = this; 
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	numThreads = sysinfo.dwNumberOfProcessors;
	if (numThreads < 1){ numThreads = 2; }
	eventKillSelf = CreateEvent(0, TRUE, FALSE, 0);
	ffttable = new FFT[numThreads];
	threads = new HANDLE[numThreads];
	eventCacheCopleted = new HANDLE[numThreads];
	eventMakeCache = new HANDLE[numThreads];
	//int doublelen = line_length * 2;
	for (int i = 0; i < numThreads; i++){
		eventCacheCopleted[i] = CreateEvent(0, FALSE, FALSE, 0);
		eventMakeCache[i] = CreateEvent(0, FALSE, FALSE, 0);
		ffttable[i].Set(provider);
		threads[i] = (HANDLE)_beginthreadex(0, 0, AudioProc, new int(i), 0, 0);
	}
}

AudioSpectrumMultiThreading::~AudioSpectrumMultiThreading()
{
	if (ffttable) 
		delete[] ffttable;
	if (threads){
		SetEvent(eventKillSelf);
		WaitForMultipleObjects(numThreads, threads, TRUE, 20000);
		for (int i = 0; i < numThreads; i++){
			CloseHandle(threads[i]);
			CloseHandle(eventMakeCache[i]);
			CloseHandle(eventCacheCopleted[i]);
		}
		CloseHandle(eventKillSelf);
		delete[] threads;
		delete[] eventMakeCache;
		delete[] eventCacheCopleted;
	}
}

void AudioSpectrumMultiThreading::CreateCache(unsigned long _start, unsigned long _end)
{
	start = _start;
	end = _end;
	len = ((end - start) / numThreads) + 1;
	for (int i = 0; i < numThreads; i++)
		SetEvent(eventMakeCache[i]);

	WaitForMultipleObjects(numThreads, eventCacheCopleted, TRUE, INFINITE);
}

unsigned int __stdcall AudioSpectrumMultiThreading::AudioProc(void* num)
{
	int numOfThread = *((int*)num);
	sthread->AudioPorocessing(numOfThread);
	delete num;
	return 0;
}
void AudioSpectrumMultiThreading::AudioPorocessing(int numOfTread)
{
	HANDLE events_to_wait[] = {
		eventMakeCache[numOfTread],
		eventKillSelf
	};
	HANDLE &complete = eventCacheCopleted[numOfTread];
	FFT &cfft = ffttable[numOfTread];
	while (1){
		DWORD wait_result = WaitForMultipleObjects(sizeof(events_to_wait) / sizeof(HANDLE), events_to_wait, FALSE, INFINITE);
		if (wait_result == WAIT_OBJECT_0 + 0){
			unsigned long threadStart = start + len *numOfTread;
			for (unsigned long i = threadStart; i < threadStart + len; i++){
				//if (i > end){ continue; }
				if ((*sub_caches)[i] == NULL){
					(*sub_caches)[i] = new FinalSpectrumCache(&cfft, (i/overlaps) * subcachelen);
				}
			}
			SetEvent(complete);
		}
		else if(wait_result == WAIT_OBJECT_0 + 1){
			break;
		}
	}
}

AudioSpectrumMultiThreading *AudioSpectrumMultiThreading::sthread = NULL;