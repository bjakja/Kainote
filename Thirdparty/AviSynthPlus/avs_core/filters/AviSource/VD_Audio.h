//	VirtualDub - Video processing and capture application
//	Copyright (C) 1998-2001 Avery Lee
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#ifndef f_AUDIO_H
#define f_AUDIO_H

#include <avs/win.h>
#include <vfw.h>
//#include "FrameSubset.h"      // no need for this in avisynth

typedef void (*AudioFormatConverter)(void *, void *, long);
typedef long (*AudioPointSampler)(void *, void *, long, long, long);
typedef long (*AudioUpSampler)(void *, void *, long, long, long);
typedef long (*AudioDownSampler)(void *, void *, long *, int, long, long, long);

class AudioSource;

///////

class AudioStream {
protected:
	WAVEFORMATEX *format;
	long format_len;

	AudioStream *source;
	long samples_read;
	long stream_len;
	long stream_limit;

	AudioStream();

	WAVEFORMATEX *AllocFormat(long len);
public:
	virtual ~AudioStream();

	virtual WAVEFORMATEX *GetFormat();
	virtual long GetFormatLen();
	virtual long GetSampleCount();
	virtual long GetLength();

	virtual long _Read(void *buffer, long max_samples, long *lplBytes);
	virtual long Read(void *buffer, long max_samples, long *lplBytes);
	virtual bool Skip(long samples);
	virtual void SetSource(AudioStream *source);
	virtual void SetLimit(long limit);
	virtual BOOL isEnd();
	virtual BOOL _isEnd();
};

class AudioStreamSource : public AudioStream {
private:
	AudioSource *aSrc;
	WAVEFORMATEX *pwfexTempInput;
	long cur_samp;
	long end_samp;
	HACMSTREAM hACStream;
	ACMSTREAMHEADER ashBuffer;
	void *inputBuffer;
	void *outputBuffer;
	char *outputBufferPtr;
	long lPreskip;
	bool fZeroRead;
	bool fStart;

	enum { INPUT_BUFFER_SIZE = 16384 };

public:
	AudioStreamSource(AudioSource *src, long first_sample, long max_sample, BOOL allow_decompression);
	~AudioStreamSource();

	long _Read(void *buffer, long max_samples, long *lplBytes);
	bool Skip(long samples);
	bool Seek(long samples);
	BOOL _isEnd();
};

class AudioStreamConverter : public AudioStream {
private:
	AudioFormatConverter convRout;
	void *cbuffer;
	int bytesPerInputSample, bytesPerOutputSample;
	int offset;

	enum { BUFFER_SIZE=4096 };

public:
	AudioStreamConverter(AudioStream *src, bool to_16bit, bool to_stereo_or_right, bool single_only);
	~AudioStreamConverter();

	long _Read(void *buffer, long max_samples, long *lplBytes);
	BOOL _isEnd();

	bool Skip(long);
};

class AudioStreamResampler : public AudioStream {
private:
	AudioPointSampler ptsampleRout;
	AudioUpSampler upsampleRout;
	AudioDownSampler dnsampleRout;
	void *cbuffer;
	int bytesPerSample;
	long samp_frac;
	long accum;
	int holdover;
	long *filter_bank;
	int filter_width;
	bool fHighQuality;

	enum { BUFFER_SIZE=512 };

	long Upsample(void *buffer, long samples, long *lplBytes);
	long Downsample(void *buffer, long samples, long *lplBytes);

public:
	AudioStreamResampler(AudioStream *source, long new_rate, bool integral_rate, bool high_quality);
	~AudioStreamResampler();

	long _Read(void *buffer, long max_samples, long *lplBytes);
	BOOL _isEnd();
};

class AudioCompressor : public AudioStream {
private:
	HACMSTREAM hACStream;
	HACMDRIVER hADriver;
	ACMSTREAMHEADER ashBuffer;
	WAVEFORMATEX *pwfexTempOutput;
	void *inputBuffer;
	void *outputBuffer;
	//char *outputBufferPtr;
	void *holdBuffer;
	long holdBufferSize;
	long holdBufferOffset;
	BOOL fStreamEnded;
	LONG bytesPerInputSample;
	LONG bytesPerOutputSample;

	enum { INPUT_BUFFER_SIZE = 16384 };

	void	ResizeHoldBuffer(long lNewSize);
	void	WriteToHoldBuffer(void *data, long lBytes);

public:
	AudioCompressor(AudioStream *src, WAVEFORMATEX *dst_format, long dst_format_len);
	~AudioCompressor();
	void CompensateForMP3();
	void *	Compress(long lInputSamples, long *lplSrcInputSamples, long *lplOutputBytes, long *lplOutputSamples);
	BOOL	isEnd();
};

class AudioL3Corrector {
private:
	long samples, frame_bytes, read_left;
	bool header_mode;
	char hdr_buffer[4];

public:
	AudioL3Corrector();
	long ComputeByterate(long sample_rate);
	void Process(void *buffer, long bytes);
};
/*
class AudioSubset : public AudioStream {
private:
	FrameSubset subset;
	FrameSubsetNode *pfsnCur;
	int iOffset;
	long lSrcPos;
	long lSkipSize;

	char skipBuffer[512];

public:
	AudioSubset(AudioStream *, FrameSubset *, long, long);
	~AudioSubset();
	long _Read(void *, long, long *);
	BOOL _isEnd();
};
*/
class AudioStreamAmplifier : public AudioStream {
private:
	long lFactor;

public:
	AudioStreamAmplifier(AudioStream *src, long lFactor);
	~AudioStreamAmplifier();

	long _Read(void *buffer, long max_samples, long *lplBytes);
	BOOL _isEnd();
	bool Skip(long);
};

//long AudioTranslateVideoSubset(FrameSubset& dst, FrameSubset& src, long usPerFrame, WAVEFORMATEX *pwfex);

#endif
