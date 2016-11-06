// Copyright (c) 2005, Rodrigo Braz Monteiro
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


//
// Most of this code was taken from http://www.codeproject.com/audio/waveInFFT.asp
// And rewriten by Rodrigo Braz Monteiro
//


///////////
// Headers
//#include "config.h"
#include "VideoFfmpeg.h"
#include "FFT.h"
#include <math.h>
#include <utility>


FFT::FFT(size_t nsamples, VideoFfmpeg *_prov)
	:n_samples(nsamples)
	,allsamples(0)
	,diff(-1)
	,prov(_prov)
{
	if (!IsPowerOfTwo(n_samples)) {
		throw L"FFT requires power of two input.";
	}
	angle_num = 2.0f * 3.1415926535897932384626433832795f;
	NumBits = NumberOfBitsNeeded(n_samples);
	input=new short[n_samples];//0
	output_i=new float[n_samples*4];
	output_r=new float[n_samples*4];
}
FFT::~FFT()
{
	if(input){delete[] input;}
	delete[] output_i;
	delete[] output_r;
}

//void FFT::RecreateTable(size_t asamples)
//{
//	if(allsamples<asamples){
//		if(input){delete[] input;}
//		input = new short[asamples];
//	}
//	allsamples=asamples;
//	diff=-1;
//}
//
//void FFT::SetDiff(size_t whre)
//{
//	diff=whre;
//	prov->GetBuffer(input,whre,allsamples);
//}
/////////////
// Transform
void FFT::Transform (size_t whre, size_t wthread, bool inverse) {
	
	//whre -= diff;
	prov->GetBuffer(input,whre,n_samples);
		
	
	int th=wthread * n_samples;
	//if(whre>=allsamples-n_samples){wxLogStatus("przekroczenie %i %i",whre, allsamples-n_samples);return;}
	//assert(whre<allsamples-n_samples);

	// Variables
	unsigned int i, j, k, n;
	float tr, ti;

	// Calculate needed bits
	// Copy samples to output buffers
	for (i=0; i<n_samples; i++) {
		j = ReverseBits (i,NumBits)+th;
		output_r[j] = (float)input[i/*+whre*/];
		output_i[j] = 0.0f;
	}

	unsigned int BlockEnd = 1;
	unsigned int BlockSize;
	for (BlockSize = 2;BlockSize<=n_samples;BlockSize<<=1) {
		// Calculate variables for this iteration
		float delta_angle = angle_num / (float)BlockSize;
		float sm2 = sin (-2 * delta_angle);
		float sm1 = sin (-delta_angle);
		float cm2 = cos (-2 * delta_angle);
		float cm1 = cos (-delta_angle);
		float w = 2 * cm1;
		float ar0, ar1, ar2, ai0, ai1, ai2;

		// Apply for every sample
		for(i=0;i<n_samples;i+=BlockSize) {
			ar1 = cm1;
			ar2 = cm2;
			ai1 = sm1;
			ai2 = sm2;

			for (j=i,n=0;n<BlockEnd;j++,n++) {
				k = j + BlockEnd;

				ar0 = w*ar1 - ar2;
				ai0 = w*ai1 - ai2;
				ar2 = ar1;
				ai2 = ai1;
				ar1 = ar0;
				ai1 = ai0;

				tr = ar0*output_r[k+th] - ai0*output_i[k+th];
				ti = ar0*output_i[k+th] + ai0*output_r[k+th];

				output_r[k+th] = output_r[j+th] - tr;
				output_i[k+th] = output_i[j+th] - ti;

				output_r[j+th] += tr;
				output_i[j+th] += ti;
			}
		}

		// Set next block end to current size
		BlockEnd = BlockSize;
	}

}



//////////////////////////////////////
// Checks if number is a power of two
bool FFT::IsPowerOfTwo (unsigned int x) {
	if (x < 2) return false;
	if (x & (x-1)) return false;
    return true;
}


//////////////////////////
// Bits needed by the FFT
unsigned int FFT::NumberOfBitsNeeded (unsigned int n_samples) {
	int i;

	if (n_samples < 2) {
		return 0;
	}

	for (i=0; ;i++) {
		if(n_samples & (1 << i)) return i;
    }
}


/////////////////////////////
// Get reversed bit position
unsigned int FFT::ReverseBits (unsigned int index, unsigned int bits) {
	unsigned int i, rev;

	for(i=rev=0;i<bits;i++) {
		rev = (rev << 1) | (index & 1);
		index >>= 1;
	}

	return rev;
}


//////////////////////////
// Get frequency at index
float FFT::FrequencyAtIndex (unsigned int baseFreq, unsigned int n_samples, unsigned int index) {
	if (index >= n_samples) return 0.0;
	else if (index <= n_samples/2) {
		return ((float)index / (float)n_samples * baseFreq);
	}
	else {
		return (-(float)(n_samples-index) / (float)n_samples * baseFreq);
	}
}
