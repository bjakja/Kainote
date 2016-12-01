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
			wxLogStatus("Getline null %i %i %i",i, start, (i-start));
			return null_line;}
	}


	FinalSpectrumCache(FFT *fft, unsigned long _start, unsigned long _length, unsigned int _overlaps)
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
				
				fft->Transform(sample);
				
				CacheLine &line = data[i + length*overlap];
				
				for (size_t j = 0; j < line_length; ++j) {
					line[j] = sqrt(fft->output_r[j]*fft->output_r[j] +
						fft->output_i[j]*fft->output_i[j]);
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
	line_length = 1<<9;
	size_t doublelen=line_length*2;
	int64_t _num_lines = provider->GetNumSamples()+doublelen / doublelen;
	num_lines = (unsigned long)_num_lines;
	numsubcaches = (num_lines + subcachelen)/subcachelen;
	SetupSpectrum();
	ChangeColours();
	

	minband = 0;//Options.GetInt(_T("Audio Spectrum Cutoff"));
	maxband = line_length - minband * 2/3; // TODO: make this customisable?
	fft = new FFT(doublelen, provider);
	sub_caches.resize(numsubcaches,0);
}


AudioSpectrum::~AudioSpectrum()
{
	delete fft;
	for (size_t i = 0; i < numsubcaches; ++i){
		if (sub_caches[i]) delete sub_caches[i];
	}
}

void AudioSpectrum::SetupSpectrum(int overlaps, int length)
{
	
	fft_overlaps = overlaps;

	FinalSpectrumCache::SetLineLength(line_length);

}

void AudioSpectrum::RenderRange(int64_t range_start, int64_t range_end, bool selected, unsigned char *img, int imgleft, int imgwidth, int imgpitch, int imgheight, int parcent)
{
    wxCriticalSectionLocker locker(CritSec);
	
    float parcPow = pow((150-parcent)/100.0f, 8);
	int parc = parcPow * (imgwidth/500.f);
	if(parc<1){parc=1;}
	if(parc>24){parc=24;}
	//wxLogStatus("parc %i %i %f %f %i", parcent, imgwidth, parcPow, (imgwidth/500.f), parc); 
	//int newlen = 7.f/pow((150-percent)/100.0f, 8);
	//newlen = MID(7,newlen,12);
    //wxLogStatus("line len %i",parc);
	if(parc!=fft_overlaps){ 
		for (size_t i = 0; i < numsubcaches; ++i){
			if (sub_caches[i]) delete sub_caches[i]; sub_caches[i]=NULL;
		}
		//delete fft;
		//cache=new SpectrumThread(this,(num_lines + subcachelen)/subcachelen, fft_overlaps);//new AudioSpectrumCacheManager(provider, line_length, num_lines, fft_overlaps);
		SetupSpectrum(parc);
	}
	
	unsigned long first_line = (unsigned long)(fft_overlaps * range_start / line_length / 2);
	unsigned long last_line = (unsigned long)(fft_overlaps * range_end / line_length / 2);

	//float *power = new float[line_length];

	int last_imgcol_rendered = -1;

	unsigned char *palette=colours_normal;
	

	// Some scaling constants
	const int maxpower = (1 << (16 - 1))*256;

	const double upscale = power_scale * 16384 / line_length;
	//const double onethirdmaxpower = maxpower / 3, twothirdmaxpower = maxpower * 2/3;
	//const double logoverscale = log(maxpower*upscale - twothirdmaxpower);

	// Note that here "lines" are actually bands of power data
	unsigned long baseline = first_line / fft_overlaps;
	unsigned int overlap = first_line % fft_overlaps;
	unsigned int j = 0;
	unsigned int start = baseline / subcachelen;
	for (unsigned long i = first_line; i <= last_line; ++i) {
		// Handle horizontal compression and don't unneededly re-render columns
		int imgcol = imgleft + imgwidth * (i - first_line) / (last_line - first_line + 1);
		if (imgcol <= last_imgcol_rendered)
			continue;
		size_t subcache = i / subcachelen;
		baseline = i / fft_overlaps;
		overlap = i % fft_overlaps;
		
		if(!sub_caches[subcache])
		{
			//wxLogStatus("subcachemake %i %i %i", start, i, (start+i)*spc->subcachelen);
			sub_caches[subcache] = new FinalSpectrumCache(fft,(baseline/subcachelen) * subcachelen,subcachelen,fft_overlaps);
		}
			
		CacheLine &line=sub_caches[subcache]->GetLine(baseline, overlap);
		j++;
		//wxLogStatus("Befor cache");
		//wxLogStatus("bef cache i %i , last line %i", i, last_line);
		//AudioSpectrumCache::CacheLine &line = GetLine(baseline, overlap);
		//wxLogStatus("aft cache i %i , last line %i", i, last_line);
		/*++overlap;
		if (overlap >= fft_overlaps) {
			overlap = 0;
			++baseline;
		}*/

		// Apply a "compressed" scaling to the signal power
		//wxLogStatus("bef power i %i , last line %i", i, last_line);
		//for (unsigned int j = 0; j < line_length; j++) {
			// First do a simple linear scale power calculation -- 8 gives a reasonable default scaling
			//power[j] = line[j] * upscale;
			/*if (power[j] > maxpower * 2/3) {
				double p = power[j] - twothirdmaxpower;
				p = log(p) * onethirdmaxpower / logoverscale;
				power[j] = p + twothirdmaxpower;
			}*/
		//}
		//wxLogStatus("aft power i %i , last line %i", i, last_line);

#define WRITE_PIXEL \
	if (intensity < 0) intensity = 0; \
	if (intensity > 255) intensity = 255; \
	img[((imgheight-y-1)*imgpitch+x)*4 + 2] = palette[intensity*3+0]; \
	img[((imgheight-y-1)*imgpitch+x)*4 + 1] = palette[intensity*3+1]; \
	img[((imgheight-y-1)*imgpitch+x)*4 + 0] = palette[intensity*3+2];
	/*img[((imgheight-y-1)*imgpitch+x)*3 + 0] = palette[intensity*3+0]; \
	img[((imgheight-y-1)*imgpitch+x)*3 + 1] = palette[intensity*3+1]; \
	img[((imgheight-y-1)*imgpitch+x)*3 + 2] = palette[intensity*3+2];*/

		// Handle horizontal expansion
		int next_line_imgcol = imgleft + imgwidth * (i - first_line + 1) / (last_line - first_line + 1);
		if (next_line_imgcol >= imgpitch)
			next_line_imgcol = imgpitch-1;

		for (int x = imgcol; x <= next_line_imgcol; ++x) {

			// Decide which rendering algo to use
			if (maxband - minband > imgheight) {
				// more than one frequency sample per pixel (vertically compress data)
				// pick the largest value per pixel for display
			//wxLogStatus("bef last loop i %i , last line %i", x, next_line_imgcol);
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
	wxColour firstcolor = Options.GetColour("Audio Spectrum First Color"); 
	wxColour secondcolor = Options.GetColour("Audio Spectrum Second Color"); 
	wxColour thirdcolor = Options.GetColour("Audio Spectrum Third Color"); 
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



