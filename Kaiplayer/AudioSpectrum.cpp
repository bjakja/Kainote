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

#include "Config.h"

#include <assert.h>
#include <vector>
#include <list>
#include <utility>
#include <algorithm>
#include "FFT.h"
#include "AudioSpectrum.h"

#include "ColorSpace.h"
#include <wx/log.h>


//typedef std::vector<float> CacheLine;
// Bottom level FFT cache, holds actual power data itself


class FinalSpectrumCache{
private:
	std::vector<CacheLine> data;
	unsigned long start, length; // start and end of range
	unsigned int overlaps;
	
	

public:
	CacheLine& GetLine(unsigned long i, unsigned int overlap)
	{

		// This check ought to be redundant
		if (i >= start && i-start < length)
			return data[(i-start) + overlap*length];
		else{
			//wxLogStatus("Getline null %i %i %i",i, start, (i-start));
			return null_line;}
	}


	FinalSpectrumCache(FFT *fft, unsigned long _start, unsigned long _length, unsigned int _overlaps, size_t numthread)
	{
		start = _start;
		length = _length;
		overlaps = _overlaps;

		// Add an upper limit to number of overlaps or trust user to do sane things?
		// Any limit should probably be a function of length

		// First fill the data vector with blanks
		// Both start and end are included in the range stored, so we have end-start+1 elements
		data.resize(length*overlaps, null_line);

		unsigned int overlap_offset = line_length / overlaps * 2; // FIXME: the result seems weird/wrong without this factor 2, but why?

		int doublelen=line_length*2;
		int th=numthread * doublelen;
		int64_t sample=start;

		for (unsigned long i = 0; i < length; ++i) {
			// Start sample number of the next line calculated
			// line_length is half of the number of samples used to calculate a line, since half of the output from
			// a Fourier transform of real data is redundant, and not interesting for the purpose of creating
			// a frequenmcy/power spectrum.
			//wxLogStatus("bufor : %i  %i",start, start * doublelen);

			//int64_t sample = start * doublelen + overlap*overlap_offset;

				
			for (unsigned int overlap = 0; overlap < overlaps; ++overlap) {
				// Initialize
				sample = (start * doublelen) + (overlap*overlap_offset) + (i*doublelen);
				
				fft->Transform(sample,numthread);
				
				CacheLine &line = data[i + length*overlap];
				
				for (size_t j = 0; j < line_length; ++j) {
					line[j] = sqrt(fft->output_r[j+th]*fft->output_r[j+th] +
						fft->output_i[j+th]*fft->output_i[j+th]);
				}
					
			}
				
				
			
		}
		
	}

	static void SetLineLength(unsigned long new_length)
	{
		line_length = new_length;
		null_line.resize(new_length, 0);
	}

	~FinalSpectrumCache()
	{
	}

	static unsigned long line_length;
	static CacheLine null_line;
};

CacheLine FinalSpectrumCache::null_line;
unsigned long FinalSpectrumCache::line_length;

// AudioSpectrum
AudioSpectrum::AudioSpectrum(VideoFfmpeg *_provider)
{
	provider = _provider;
	subcachelen=16;
	power_scale = 1;
	line_length = 1<<7;
	size_t doublelen=line_length*2;
	int64_t _num_lines = provider->GetNumSamples()+doublelen / doublelen;
	num_lines = (unsigned long)_num_lines;
	SetupSpectrun();
	

	// Generate colour maps
	unsigned char *palptr = colours_normal;
	for (int i = 0; i < 256; i++) {
		//hsl_to_rgb(170 + i * 2/3, 128 + i/2, i, palptr+0, palptr+1, palptr+2);	// Previous
		hsl_to_rgb((255+128-i)/2, 128 + i/2, MIN(255,2*i), palptr+0, palptr+1, palptr+2);	// Icy blue
		//hsl_to_rgb(174, 255-i, i, palptr+0, palptr+1, palptr+2);
		palptr += 3;
	}
	palptr = colours_selected;
	for (int i = 0; i < 256; i++) {
		//hsl_to_rgb(170 + i * 2/3, 128 + i/2, i*3/4+64, palptr+0, palptr+1, palptr+2);
		hsl_to_rgb((255+128-i)/2, 128 + i/2, MIN(255,3*i/2+64), palptr+0, palptr+1, palptr+2);	// Icy blue
		//hsl_to_rgb(174, 255-i, (i*0.875f)+32, palptr+0, palptr+1, palptr+2);
		palptr += 3;
	}

	minband = 0;//Options.GetInt(_T("Audio Spectrum Cutoff"));
	maxband = line_length - minband * 2/3; // TODO: make this customisable?
	fft = new FFT(doublelen, provider);
	
}


AudioSpectrum::~AudioSpectrum()
{
	delete cache;
	delete fft;
}

void AudioSpectrum::SetupSpectrun(int overlaps, int length)
{
	
	fft_overlaps = overlaps;

	FinalSpectrumCache::SetLineLength(line_length);
	cache = new SpectrumThread(this,(num_lines + subcachelen)/subcachelen, fft_overlaps);

}

void AudioSpectrum::RenderRange(int64_t range_start, int64_t range_end, bool selected, unsigned char *img, int imgwidth, int imgheight, int percent)
{
    wxCriticalSectionLocker locker(CritSec);
    int parc = pow((150-percent)/100.0f, 8);
	if(parc<2){parc=2;}
	//int newlen = 7.f/pow((150-percent)/100.0f, 8);
	//newlen = MID(7,newlen,12);
    //wxLogStatus("line len %i",parc);
	if(parc!=fft_overlaps){ 
		delete cache;
		//delete fft;
		//cache=new SpectrumThread(this,(num_lines + subcachelen)/subcachelen, fft_overlaps);//new AudioSpectrumCacheManager(provider, line_length, num_lines, fft_overlaps);
		SetupSpectrun(parc);
	}
	
	unsigned long first_line = (unsigned long)(fft_overlaps * range_start / line_length / 2);
	unsigned long last_line = (unsigned long)(fft_overlaps * range_end / line_length / 2);
	//wxLogStatus("line len %i",(int)(last_line-first_line));
	//unsigned int overlap_offset = (line_length * 2);
	size_t copysamples;// = ((range_end - range_start))*fft_overlaps;
	
	
	//fft->RecreateTable(copysamples + (overlap_offset*36));//724672 -> 752640
	//wxLogStatus("line len %i %i", copysamples + (overlap_offset*36), 752640);
	
	copysamples = (last_line - first_line);
	unsigned int cpsl = (copysamples+ subcachelen) / subcachelen;
	
	
	//int last_imgcol_rendered = -1;

	unsigned char *palette;
	if (selected)
		palette = colours_selected;
	else
		palette = colours_normal;

	// Note that here "lines" are actually bands of power data
	unsigned long baseline = first_line / fft_overlaps;
	//wxLogStatus("len %i %i ",(int)last_line, (int)((cpsl+fft_overlaps / fft_overlaps)*subcachelen + first_line));
	cache->MakeSubCaches(baseline / subcachelen, first_line, (cpsl+fft_overlaps / fft_overlaps), last_line, img, imgwidth, imgheight, palette);
	

	cache->Wait();
	//cache->Age();
	
}


void AudioSpectrum::SetScaling(float _power_scale)
{
	power_scale = _power_scale;
}
//SpectrumThread *SpectrumThread::st=NULL;

SpectrumThread::SpectrumThread(AudioSpectrum *_spc, size_t _numsubcaches, size_t _overlaps)
	:spc(_spc)
	,overlaps(_overlaps)
	,numsubcaches(_numsubcaches)
{
	sub_caches.resize(numsubcaches,0);
}

 SpectrumThread::~SpectrumThread(){
	for (size_t i = 0; i < numsubcaches; ++i){
		if (sub_caches[i]) delete sub_caches[i];
	}
}

CacheLine &SpectrumThread::GetLine(unsigned long i, unsigned int overlap)
{
	// Determine which sub-cache this line resides in
	size_t subcache = i / spc->subcachelen;
			
	if(!sub_caches[subcache]){
		//wxLogStatus("nullsubcache %i", subcache);
		return FinalSpectrumCache::null_line;
	}
	return sub_caches[subcache]->GetLine(i, overlap);
}



void SpectrumThread::MakeSubCaches(size_t _start, size_t _bufstart, size_t _len, size_t lastbuf, unsigned char *_img, int _imgwidth, int _imgheight, unsigned char *_palette)
{
	start=_start;
	length=_len;
	first_line=_bufstart;
	last_line=lastbuf;
	img=_img;
	imgwidth=_imgwidth;
	imgheight=_imgheight;
	palette=_palette;
	//size_t sample = (start * spc->subcachelen) * (spc->line_length*2);
	//spc->fft->SetDiff(sample);
	for(int t=0; t<4; t++){
		thread[t] = CreateThread( NULL, 0,  (LPTHREAD_START_ROUTINE)proc, (void*)new std::pair<int,SpectrumThread*>(t,this), 0, 0);
	}
}

DWORD SpectrumThread::proc(void *cls)
{
	std::pair<int,SpectrumThread*> *stt = (std::pair<int,SpectrumThread*> *)cls;
	stt->second->procincls(stt->first);
	delete stt;
	return 0;
}

void SpectrumThread::procincls(int numthread)
{
	const int maxpower = (1 << (16 - 1))*256;

	const double upscale = spc->power_scale * 16384 / spc->line_length;
	int last_imgcol_rendered = -1;
	unsigned long baseline = first_line / spc->fft_overlaps;
	unsigned int overlap = first_line % spc->fft_overlaps;
	ULONG bufstart = (start * spc->subcachelen);

	for(size_t j = numthread; j < length; j += 4)
	{
		size_t subcache = (j+start);
		
		if(!sub_caches[subcache])
		{
			//wxLogStatus("subcachemake %i %i %i", start, i, (start+i)*spc->subcachelen);
			sub_caches[subcache] = new FinalSpectrumCache(spc->fft,(start+j)*spc->subcachelen,spc->subcachelen,spc->fft_overlaps,numthread);
		}
		ULONG len=(j+1==length)? last_line : ((subcache+1)  * spc->subcachelen)-1;
		ULONG strt=(j==0)? first_line : bufstart + (j * spc->subcachelen);
		for (unsigned long i = strt; i <= len; ++i) {
			// Handle horizontal compression and don't unneededly re-render columns
			int imgcol = imgwidth * (i - first_line) / (last_line - first_line + 1);
			if (imgcol <= last_imgcol_rendered)
				continue;

			baseline = i / spc->fft_overlaps;
			overlap = i % spc->fft_overlaps;
		
			CacheLine &power = GetLine(baseline, overlap);
		

	#define WRITE_PIXEL \
		if (intensity < 0) intensity = 0; \
		if (intensity > 255) intensity = 255; \
		img[((imgheight-y-1)*imgwidth+x)*3 + 0] = palette[intensity*3+0]; \
		img[((imgheight-y-1)*imgwidth+x)*3 + 1] = palette[intensity*3+1]; \
		img[((imgheight-y-1)*imgwidth+x)*3 + 2] = palette[intensity*3+2];

			// Handle horizontal expansion
			int next_line_imgcol = imgwidth * (i - first_line + 1) / (last_line - first_line + 1);
			if (next_line_imgcol >= imgwidth)
				next_line_imgcol = imgwidth-1;

			for (int x = imgcol; x <= next_line_imgcol; ++x) {

				// Decide which rendering algo to use
				if (spc->maxband - spc->minband > imgheight) {
					// more than one frequency sample per pixel (vertically compress data)
					// pick the largest value per pixel for display
					// Iterate over pixels, picking a range of samples for each
					for (int y = 0; y < imgheight; ++y) {
						int sample1 = MAX(0,spc->maxband * y/imgheight + spc->minband);
						int sample2 = MIN(signed(spc->line_length-1),spc->maxband * (y+1)/imgheight + spc->minband);
						float maxval = 0;
						for (int samp = sample1; samp <= sample2; samp++) {
							if (power[samp] > maxval) maxval = power[samp];
						}
						int intensity = int(256 * (maxval* upscale) / maxpower);
						WRITE_PIXEL
					}
				}
				else {
					// less than one frequency sample per pixel (vertically expand data)
					// interpolate between pixels
					// can also happen with exactly one sample per pixel, but how often is that?

					// Iterate over pixels, picking the nearest power values
					for (int y = 0; y < imgheight; ++y) {
						float ideal = (float)(y+1.)/imgheight * spc->maxband;
						float sample1 = power[(int)floor(ideal)+spc->minband]* upscale;
						float sample2 = power[(int)ceil(ideal)+spc->minband]* upscale;
						float frac = ideal - floor(ideal);
						int intensity = int(((1-frac)*sample1 + frac*sample2) / maxpower * 256);
						WRITE_PIXEL
					}
				}
			}

	#undef WRITE_PIXEL

		}

	}

}

void SpectrumThread::Wait()
{
	WaitForMultipleObjects(4, thread,true,INFINITE);
	for(int t=0; t<4; t++){CloseHandle(thread[t]);}
}