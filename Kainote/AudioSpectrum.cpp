//  Copyright (c) 2016-2022, Marcin Drob

//  Kainote is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.

//  Kainote is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.

//  You should have received a copy of the GNU General Public License
//  along with Kainote.  If not, see <http://www.gnu.org/licenses/>.

//this code piervously was taken from Aegisub 2 it's rewritten by me almost all.



//#include "Config.h"
#include "AudioSpectrum.h"

//#include "ColorSpace.h"
#include <wx/log.h>
#include <process.h>
#include <assert.h>
#include <vector>
#include <list>
#include <utility>
#include <algorithm>
//#include "UtilsWindows.h"


const unsigned int orgsubcachelen = 16;//original subcache length when overlaps = 1
static unsigned int subcachelen = orgsubcachelen;//orgsubcachelen * overlaps
static unsigned int overlaps = 1;// number of overlaps used in FFT

class SpectrumCache{
private:
	std::vector<CacheLine> data;

public:
	CacheLine& GetLine(unsigned long i)
	{

		// This check ought to be redundant
		if (i >= start && i-start < subcachelen)
			return data[(i-start)];
		else{
			return null_line;}
	}

	SpectrumCache(){};

	void CreateCache(FFT *fft, unsigned long _start)
	{
		// Add an upper limit to number of overlaps or trust user to do sane things?
		// Any limit should probably be a function of subcachelen

		// First fill the data vector with blanks
		// Both start and end are included in the range stored, so we have end-start+1 elements
		if (data.size() < subcachelen){
			data.resize(subcachelen, null_line); 
		}

		start = _start;
		//test it
		unsigned int overlap_offset = doublelen / overlaps;

		//long long sample=start;
		long long fftStart = start;
		fftStart = (fftStart * doublelen) / (overlaps);
		long long sample = fftStart;
		//float scale_factor = 10 / sqrt(2 * (float)(doublelen));

		for (unsigned long i = 0; i < subcachelen; ++i) {
			// Start sample number of the next line calculated
			// line_length is half of the number of samples used to calculate a line, since half of the output from
			// a Fourier transform of real data is redundant, and not interesting for the purpose of creating
			// a frequenmcy/power spectrum.
			
			//sample = fftStart + (i / orgsubcachelen * doublelen) + (i % overlaps * overlap_offset);
			fft->Transform(sample);

			CacheLine &line = data[i];

			for (size_t j = 0; j < line_length; ++j) {
				int g = (j * 2) + 1;
				line[j] = /*log10( */sqrt(fft->output[j * 2] * fft->output[j * 2] + fft->output[g] * fft->output[g])/* * scale_factor + 1)*/;
			}
						
			sample = fftStart += overlap_offset;
		}
		
	}
	
	static void SetLineLength()
	{
		null_line.resize(line_length, 0);
	}

	~SpectrumCache()
	{
	}
	unsigned long start = -1;
	static CacheLine null_line;
};

CacheLine SpectrumCache::null_line;


// AudioSpectrum
AudioSpectrum::AudioSpectrum(Provider *_provider)
{
	provider = _provider;
	power_scale = 1;
	AudioThreads = new AudioSpectrumMultiThreading(provider, &sub_caches);
	SpectrumCache::SetLineLength();
	//SetupSpectrum();
	ChangeColours();
	minband = 0;//Options.GetInt(_T("Audio Spectrum Cutoff"));
	maxband = line_length/* - minband * 2/3*/; // TODO: make this customisable?
	nonlinear = Options.GetBool(AUDIO_SPECTRUM_NON_LINEAR_ON);
}


AudioSpectrum::~AudioSpectrum()
{
	delete AudioThreads;
	for (size_t i = 0; i < sub_caches.size(); ++i){
		delete sub_caches[i];
	}
}

void AudioSpectrum::SetupSpectrum(int _overlaps)
{
	overlaps = _overlaps;
	subcachelen = orgsubcachelen * overlaps;
}

void AudioSpectrum::RenderRange(long long range_start, long long range_end, unsigned char *img, int imgwidth, int imgpitch, int imgheight, int percent)
{
	wxCriticalSectionLocker locker(CritSec);
	int newOverlaps = ceil(((float)imgwidth / ((float)(range_end - range_start) / (float)doublelen)) * ((100 - percent) / 100.f)) + 1;
	if (newOverlaps < 1){ newOverlaps = 1; }
	if (newOverlaps > 24){
		newOverlaps = 24;
	}
	
	if(newOverlaps != overlaps){ 
		for (size_t i = 0; i < sub_caches.size(); ++i){
			sub_caches[i]->start = -1;
		}
		SetupSpectrum(newOverlaps);
	}
	unsigned long first_line = (unsigned long)(overlaps * range_start / doublelen);
	unsigned long last_line = (unsigned long)(overlaps * range_end / doublelen);
	unsigned long startcache = first_line / subcachelen;
	unsigned long endcache = last_line / subcachelen;

	size_t size = sub_caches.size();
	size_t neededsize = (endcache - startcache) + AudioThreads->numThreads;
	if (size < neededsize){
		sub_caches.resize((neededsize * 3), 0);
		for (int i = size; i < neededsize * 3; i++)
			sub_caches[i] = new SpectrumCache();
	}

	int last_imgcol_rendered = -1;
	float factor = pow(line_length, 1.f / (imgheight - 1));
	// Some scaling constants
	const int maxpower = (1 << (16 - 1)) * 256;

	const double upscale = power_scale * 16384 / line_length;
	AudioThreads->CreateCache(startcache, endcache);

	// Note that here "lines" are actually bands of power data
	unsigned long sampleRange = (last_line - first_line + 1);
	unsigned long subcache = AudioThreads->lastCachePosition;
	SpectrumCache *cache = sub_caches[subcache];
	for (unsigned long i = first_line, k = 0; i <= last_line; ++i, ++k) {
		// Handle horizontal compression and don't unneededly re-render columns
		int imgcol = imgwidth * k / sampleRange;
		if (imgcol <= last_imgcol_rendered)
			continue;
		//size_t subcache = i / subcachelen;
		if (i % subcachelen == 0 && k>0){
			subcache++; 
			if (subcache >= sub_caches.size()){ subcache = 0; }
			cache = sub_caches[subcache]; 
		}
		CacheLine &line=cache->GetLine(i);

#define WRITE_PIXEL \
	if (intensity < 0) intensity = 0; \
	if (intensity > 255) intensity = 255; \
	img[((imgheight - y - 1) * imgpitch + x) * 4 + 2] = palette[intensity * 3 + 0]; \
	img[((imgheight - y - 1) * imgpitch + x) * 4 + 1] = palette[intensity * 3 + 1]; \
	img[((imgheight - y - 1) * imgpitch + x) * 4 + 0] = palette[intensity * 3 + 2];

		// Handle horizontal expansion
		int next_line_imgcol = imgwidth * (k + 1) / sampleRange;
		if (next_line_imgcol >= imgpitch)
			next_line_imgcol = imgpitch - 1;

		//last_imgcol_rendered = next_line_imgcol;

		for (int x = imgcol; x <= next_line_imgcol; ++x) {

			// Decide which rendering algo to use
			if (maxband - minband > imgheight) {
				// more than one frequency sample per pixel (vertically compress data)
				// pick the largest value per pixel for display
				// Iterate over pixels, picking a range of samples for each
				int sample2 = 0;
				int sample1 = 0;
				float samplecounter = 1.f;
				for (int y = 0; y < imgheight; ++y) {
					if (nonlinear){
						if (sample2 >= (int)samplecounter){
							sample2++;
						}
						else{ sample2 = (int)samplecounter; }
						samplecounter *= factor;
						if (sample1 >= line_length)
							sample1 = line_length - 1;
						if (sample2 >= line_length)
							sample2 = line_length - 1;
					}
					else{
						sample1 = MAX(0, maxband * y / imgheight + minband);
						sample2 = MIN(signed(line_length - 1), maxband * (y + 1) / imgheight + minband);
					}
					float maxval = 0;
					for (int samp = sample1; samp <= sample2; samp++) {
						if (line[samp] > maxval) maxval = line[samp];
					}
					//float maxval = *std::max_element(&line[sample1], &line[sample2]);
					sample1 = sample2+1;
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
					float ideal = (float)(y + 1.)/imgheight * maxband;
					float sample1 = line[(int)floor(ideal) + minband]/* * upscale*/;
					float sample2 = line[(int)ceil(ideal) + minband]/* * upscale*/;
					float frac = ideal - floor(ideal);
					int intensity = int(((1 - frac) * sample1 + frac * sample2) * upscale / maxpower * 256);
					WRITE_PIXEL
				}
			}
		}

#undef WRITE_PIXEL

	}
	
}

void AudioSpectrum::CreateRange(std::vector<int> &output, std::vector<int> &intensities, long long timeStart, long long timeEnd, wxPoint frequency, int peek)
{
	wxCriticalSectionLocker locker(CritSec);
	overlaps = 1;
	subcachelen = 16;
	int sampleRate = provider->GetSampleRate();
	long long range_start = timeStart * sampleRate / 1000;
	long long range_end = timeEnd * sampleRate / 1000;
	unsigned long first_line = (unsigned long)(range_start / doublelen);
	unsigned long last_line = (unsigned long)(range_end / doublelen);
	unsigned long startcache = first_line / subcachelen;
	unsigned long endcache = last_line / subcachelen;
	int indexStart = (int)(frequency.x / (sampleRate / doublelen));
	indexStart = MID(0, indexStart, line_length - 1);
	int indexEnd = (int)(frequency.y / (sampleRate / doublelen));
	indexEnd = MID(0, indexEnd, line_length - 1);
	if (indexEnd < indexStart)
		indexEnd = indexStart;
	
	size_t size = sub_caches.size();
	size_t neededsize = (endcache - startcache) + AudioThreads->numThreads;
	if (size < neededsize){
		sub_caches.resize((neededsize * 3), 0);
		for (int i = size; i < neededsize * 3; i++)
			sub_caches[i] = new SpectrumCache();
	}

	const int maxpower = (1 << (16 - 1)) * 100;
	const double upscale = 16384 / line_length;
	AudioThreads->CreateCache(startcache, endcache);

	// Note that here "lines" are actually bands of power data
	long long lasttime = -1;
	unsigned long subcache = AudioThreads->lastCachePosition;
	SpectrumCache *cache = sub_caches[subcache];
	//long long g = range_start;
	int lastintensity = 0;
	int lastintensitytime = 0;
	for (unsigned long i = first_line; i <= last_line; ++i) {
		if (i % subcachelen == 0 && i > first_line){
			subcache++;
			if (subcache >= sub_caches.size()){ subcache = 0; }
			cache = sub_caches[subcache];
		}
		CacheLine &line = cache->GetLine(i);
		long long lli = i;
		long long time = (lli * doublelen * 1000) / sampleRate;
		//g += doublelen;
		
		bool reached = false;
		if (lasttime < time){
			for (int i = indexStart; i <= indexEnd; i++){
				int intensity = int(100 * (line[i] * upscale) / maxpower);
				if (!peek){
					if (lastintensity < intensity)
						lastintensity = intensity;
					if (i == indexEnd){
						output.push_back(time);
						intensities.push_back(lastintensity);
						lastintensity = 0;
					}
				}
				else if (intensity >= peek){
					if (lastintensity < intensity){
						lastintensity = intensity;
						lastintensitytime = time;
					}
					//break;
					reached = true;
				}
			}
			if (lastintensity && !reached){
				output.push_back(lastintensitytime);
				lastintensity = 0;
			}
		}
		lasttime = time;
	}
}

void AudioSpectrum::SetScaling(float _power_scale)
{
	power_scale = _power_scale;
}

void AudioSpectrum::ChangeColours()
{
	const wxColour & firstcolor = Options.GetColour(AUDIO_SPECTRUM_BACKGROUND); 
	const wxColour & secondcolor = Options.GetColour(AUDIO_SPECTRUM_ECHO); 
	const wxColour & thirdcolor = Options.GetColour(AUDIO_SPECTRUM_INNER); 
	float r2 = thirdcolor.Red(), r1 = secondcolor.Red(), r = firstcolor.Red(), 
		g2 = thirdcolor.Green(), g1 = secondcolor.Green(), g = firstcolor.Green(), 
		b2 = thirdcolor.Blue(), b1 = secondcolor.Blue(), b = firstcolor.Blue();
	
	// Generate colour maps
	unsigned char *palptr = palette;
	float div = (1.f/128.f);
	float i = 0;
	int j = 0;
	while (j < 256) {
		int pointr = (i < 0.5f)? (r - (( r - r1) * (i * 2))) : (r1 - ((r1 - r2) * ((i * 2) - 1.f)));
		int pointg = (i < 0.5f)? (g - (( g - g1) * (i * 2))) : (g1 - ((g1 - g2) * ((i * 2) - 1.f)));
		int pointb = (i < 0.5f)? (b - (( b - b1) * (i * 2))) : (b1 - ((b1 - b2) * ((i * 2) - 1.f)));

		*palptr = (unsigned char)pointr;
		*(palptr + 1) = (unsigned char)pointg;
		*(palptr + 2) = (unsigned char)pointb;
		
		if(j < 128){i += div;}
		
		palptr += 3;
		j++;
	}

}

AudioSpectrumMultiThreading::AudioSpectrumMultiThreading(Provider *provider, std::vector<SpectrumCache*> *_sub_caches)
{
	sub_caches = _sub_caches;
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
	if (_start == start && end == _end && (*sub_caches)[lastCachePosition]->start == _start){ return; }
	if (_start < end && start < _end){
		long newPosition = lastCachePosition - (start - _start);
		size_t subcachessize = sub_caches->size();
		if (newPosition < 0){ lastCachePosition = newPosition + subcachessize; }
		else if (newPosition >= subcachessize){ lastCachePosition = newPosition - subcachessize; }
		else(lastCachePosition = newPosition);
	}
	else{
		FindCache(_start * subcachelen, _end * subcachelen);
	}
	start = _start;
	end = _end;
	len = ((end - start) / numThreads) + 1;
	//if ((*sub_caches)[lastCachePosition]->start == _start * subcachelen &&)
	for (int i = 0; i < numThreads; i++)
		SetEvent(eventMakeCache[i]);

	WaitForMultipleObjects(numThreads, eventCacheCopleted, TRUE, INFINITE);
}

void AudioSpectrumMultiThreading::FindCache(unsigned long _start, unsigned long _end)
{
	for (size_t i = 0; i < sub_caches->size(); i++){
		SpectrumCache *cache = (*sub_caches)[i];
		if (cache->start == _start){
			lastCachePosition = i;
			break;
		}
		else if (_end == cache->start){
			long currentstart = i - ((len -1) * numThreads) - 1;
			lastCachePosition = (currentstart < 0) ? sub_caches->size() + currentstart : currentstart;
			break;
		}
	}
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
			unsigned long threadStart = start + len * numOfTread;
			unsigned long startcache = lastCachePosition + (len * numOfTread);
			bool audioSet = false;
			size_t sssize = sub_caches->size();
			for (unsigned long i = threadStart; i < threadStart + len; i++){
				unsigned long currentCache = i/* / overlaps*/ * subcachelen;
				if (startcache >= sssize){
					startcache -= sssize;
				}
				if ((*sub_caches)[startcache]->start != currentCache){
					if (!audioSet){ SetAudio(i, len - (i - threadStart), &cfft); audioSet = true; }
					(*sub_caches)[startcache]->CreateCache(&cfft, currentCache);
				}
				startcache++;
			}
			SetEvent(complete);
		}
		else if(wait_result == WAIT_OBJECT_0 + 1){
			break;
		}
	}
}

void AudioSpectrumMultiThreading::SetAudio(unsigned long _start, int _len, FFT *fft)
{
	if (_len < 1){
		assert(false);
	}
	//size_t samplestart = (_start * subcachelen) * doublelen;
	//size_t offset = (doublelen / overlaps) ;
	//size_t sampleend = ((_start + _len - 1) * subcachelen) * doublelen;
	//sampleend += ((subcachelen-1) * doublelen);
	long long samplestart = ((_start /*/ overlaps*/) * orgsubcachelen) * doublelen;
	long long offset = (doublelen / overlaps);
	long long sampleend = (((_start + _len - 1) /*/ overlaps*/) * orgsubcachelen) * doublelen;
	sampleend += /*((overlaps - 1) * offset) + */((subcachelen - 1) * offset/*doublelen*/);

	fft->SetAudio(samplestart, (sampleend - samplestart) + doublelen);
}

AudioSpectrumMultiThreading *AudioSpectrumMultiThreading::sthread = nullptr;

