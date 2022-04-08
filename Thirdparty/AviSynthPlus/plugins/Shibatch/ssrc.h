/******************************************************
  A fast and high quality sampling rate converter SSRC
                                           written by Naoki Shibata


Homepage : http://shibatch.sourceforge.net/
e-mail   : shibatch@users.sourceforge.net

Some changes are:

Copyright (c) 2001-2003, Peter Pawlowski
All rights reserved.

Other changes are:

Copyright (c) 2003, Klaus Post

*******************************************************/


#include <avisynth.h>
#include <avs/config.h>
#include <cstring>
#include "mem_block.h"

typedef SFLOAT audio_sample;
typedef audio_sample REAL_inout;

#include "fft.h"

#include <stdlib.h>
#ifndef AVS_WINDOWS
#define _aligned_malloc(size, alignment) aligned_alloc(alignment, size)
#define _aligned_free(ptr) free(ptr)
#endif

class Buffer
{
private:
	mem_block_t<REAL_inout> buffer;
	int buf_data;
public:
	AVS_FORCEINLINE Buffer() {buf_data=0;buffer.set_mem_logic(mem_block::ALLOC_FAST_DONTGODOWN);}
	AVS_FORCEINLINE REAL_inout * GetBuffer(int * siz) {*siz=buf_data;return buffer;}
	AVS_FORCEINLINE int Size() {return buf_data;}
	void Read(int size);
	void Write(const REAL_inout * ptr,int size);
};


class Resampler_base
{
public:
	class CONFIG
	{
	public:
		int sfrq,dfrq,nch,dither,pdf,fast;
		AVS_FORCEINLINE CONFIG(int _sfrq,int _dfrq,int _nch,int _dither,int _pdf,int _fast=1)
		{
			sfrq=_sfrq;
			dfrq=_dfrq;
			nch=_nch;
			dither=_dither;
			pdf=_pdf;
			fast=_fast;
		}
	};

private:
	Buffer in,out;
	void bufloop(int finish);
protected:

	Resampler_base(const Resampler_base::CONFIG & c);

	void AVS_FORCEINLINE __output(REAL_inout value, int& delay2)
	{
		if(delay2 == 0) {
			out.Write(&value,1);
		} else {
			delay2--;
		}
	};

	virtual unsigned int Resample(REAL_inout * input,unsigned int size,int ending)=0;

	double peak;
	int nch,sfrq,dfrq;
	double gain;


	double AA,DF;
	int FFTFIRLEN;

public:
	double GetPeak() {return peak;}//havent tested if this actually still works

void Write(const REAL_inout* input,int size);
	AVS_FORCEINLINE void Finish() {bufloop(1);}

	AVS_FORCEINLINE REAL_inout* GetBuffer(int * s) {return out.GetBuffer(s);}
	AVS_FORCEINLINE void Read(unsigned int s) {out.Read(s);}

	unsigned int GetLatency();//returns amount of audio data in in/out buffers in milliseconds

	AVS_FORCEINLINE unsigned int GetDataInInbuf() {return in.Size();}

	virtual ~Resampler_base() {}

	static Resampler_base * Create(Resampler_base::CONFIG & c);
};

/*
#define SSRC_create(sfrq,dfrq,nch,dither,pdf,fast) \
	Resampler_base::Create(Resampler_base::CONFIG(sfrq,dfrq,nch,dither,pdf,fast))

moved outside
*/
Resampler_base *SSRC_create(int sfrq, int dfrq, int nch, int dither, int pdf, int fast);

/*
USAGE:
Resampler_base::Create() / SSRC_create with your resampling params (see source for exact info what they do)

 loop:
Write() your PCM data to be resampled
GetBuffer() to get pointer to buffer with resampled data and amount of resampled data available (in bytes)
(note: SSRC operates on blocks, don't expect it to return any data right after your first Write() )

Read() to tell the resampler how much data you took from the buffer ( <= size returned by GetBuffer )

you can use GetLatency() to get amount of audio data (in ms) currently being kept in resampler's buffers
(quick hack for determining current output time without extra stupid counters)

Finish() to force-convert remaining PCM data after last Write() (warning: never Write() again after Flush() )

delete when done

*/
