/******************************************************
SuperEQ written by Naoki Shibata  shibatch@users.sourceforge.net

Shibatch Super Equalizer is a graphic and parametric equalizer plugin
for winamp. This plugin uses 16383th order FIR filter with FFT algorithm.
It's equalization is very precise. Equalization setting can be done
for each channel separately.


Homepage : http://shibatch.sourceforge.net/
e-mail   : shibatch@users.sourceforge.net

Some changes are from foobar2000 (www.foobar2000.org):

Copyright (c) 2001-2003, Peter Pawlowski
All rights reserved.

Other changes are:

Copyright (c) 2003, Klaus Post

*******************************************************/

#include <math.h>
#include "supereq.h"
#include "paramlist.h"
#include <vector>
#include <avs/minmax.h>
#include <avisynth.h>



enum {N_BANDS=18};

struct eq_config
{
	int bands[N_BANDS];
};

static const eq_config def_bands = {20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20};


static void setup_bands(const eq_config & src,double dst[N_BANDS])
{
	int n;
	for(n=0;n<N_BANDS;n++)
	{
		dst[n] = (double)pow(10.0,(src.bands[n]-20)/-20.0);
	}
}


class AVSsupereq : public GenericVideoFilter
{
private:
	std::vector<supereq_base*> eqs;
	paramlist paramroot;
	eq_config my_eq;

  int dstbuffer_size;
  int dst_samples_filled;

  SFLOAT* dstbuffer;
  SFLOAT* passbuffer;
  int64_t next_sample;
  int64_t inputReadOffset;

public:
AVSsupereq(PClip _child, const char* filename, IScriptEnvironment* env)
: GenericVideoFilter(_child)
{
  const unsigned last_nch   = (unsigned)vi.AudioChannels();
  const unsigned last_srate = (unsigned)vi.audio_samples_per_second;

  FILE *settingsfile;
  settingsfile = fopen(filename, "r");

  if (settingsfile != NULL) {
    int n;
    for(n=0;n<N_BANDS;n++) {
      int readval;
      if (fscanf(settingsfile, "%d", &readval) == 1) {
        my_eq.bands[n] = ((-readval)+20);
      }
    }
    fclose(settingsfile);
  } else {
    env->ThrowError("SuperEQ: Could not open file");
  }

  unsigned n;
  for(n=0;n<last_nch;n++)
    eqs.push_back(new supereq<float>);
  double bands[N_BANDS];
  //    my_eq = cfg_eq;
  setup_bands(my_eq,bands);
  for(n=0;n<last_nch;n++)
    eqs[n]->equ_makeTable(bands,&paramroot,(double)last_srate);

  dstbuffer = new SFLOAT[last_srate * last_nch];  // Our buffer can minimum contain one second.
  passbuffer = new SFLOAT[last_srate];  // Our buffer can minimum contain one second.
  dstbuffer_size = last_srate;

  next_sample = 0;  // Next output sample
  inputReadOffset = 0;  // Next input sample
  dst_samples_filled = 0;
}

AVSsupereq(PClip _child, int* values, IScriptEnvironment* env)
: GenericVideoFilter(_child)
{
  const unsigned last_nch   = (unsigned)vi.AudioChannels();
  const unsigned last_srate = (unsigned)vi.audio_samples_per_second;

  unsigned n;
  for(n=0; n<N_BANDS; n++) {
      my_eq.bands[n] = (-values[n]+20);
  }

  for(n=0; n<last_nch; n++)
    eqs.push_back(new supereq<float>);

  double bands[N_BANDS];
  //    my_eq = cfg_eq;
  setup_bands(my_eq, bands);

  for(n=0; n<last_nch; n++)
    eqs[n]->equ_makeTable(bands, &paramroot, (double)last_srate);

  dstbuffer = new SFLOAT[last_srate * last_nch];  // Our buffer can minimum contain one second.
  passbuffer = new SFLOAT[last_srate];  // Our buffer can minimum contain one second.
  dstbuffer_size = last_srate;

  next_sample = 0;  // Next output sample
  inputReadOffset = 0;  // Next input sample
  dst_samples_filled = 0;
}
private:

void __stdcall GetAudio(void* buf, int64_t start, int64_t count, IScriptEnvironment* env)
{
  const unsigned last_nch   = (unsigned)vi.AudioChannels();
  const unsigned last_srate = (unsigned)vi.audio_samples_per_second;

  if (start != next_sample) {  // Reset on seek
    inputReadOffset = start;  // Reset at new read position.
    dst_samples_filled=0;

    for (size_t i = 0; i < eqs.size(); ++i)
      delete eqs[i];
    eqs.clear();

    unsigned n;
    for(n=0;n<last_nch;n++)
      eqs.push_back(new supereq<float>);
    double bands[N_BANDS];
    setup_bands(my_eq,bands);
    for(n=0;n<last_nch;n++)
      eqs[n]->equ_makeTable(bands,&paramroot,(double)last_srate);
  }

  bool buffer_full = false;
  int samples_filled = 0;

  do {
    // Empty buffer if something is still left.
    if (dst_samples_filled) {
      int copysamples = min((int)count-samples_filled, dst_samples_filled);
      // Copy finished samples
      int pitch = copysamples*last_nch*sizeof(SFLOAT);
      env->BitBlt((BYTE*)buf+samples_filled*last_nch*sizeof(SFLOAT), pitch,
        (BYTE*)dstbuffer, pitch,
        pitch, 1
      );
      dst_samples_filled -= copysamples;

      if (dst_samples_filled) { // Move non-used samples
        memcpy(dstbuffer, &dstbuffer[copysamples*last_nch], dst_samples_filled*sizeof(SFLOAT)*last_nch);
      }
      samples_filled += copysamples;
      if (samples_filled >= count)
        buffer_full = true;
    }
    // If buffer empty - refill
    if (dst_samples_filled<=0) {
      // Feed new samples to filter
      child->GetAudio(dstbuffer, inputReadOffset, last_srate, env);
      inputReadOffset += last_srate;

      {for (unsigned n=0; n<last_nch; n++) { // Copies n channels to separate buffers to individual filters
        SFLOAT *db = dstbuffer + n;
        for (unsigned s=0, r=0; s<last_srate; s++, r+=last_nch)
          passbuffer[s] = db[r];

        eqs[n]->write_samples(passbuffer, last_srate);
      }}

      // Read back samples from filter
      int samples_out = 0;
      {for (unsigned n=0; n<last_nch; n++) { // Copies back samples from individual filters
        SFLOAT *data_out = eqs[n]->get_output(&samples_out);
        // Check temp buffer size
        if (dstbuffer_size < samples_out) {
          if (n) { // Extremely unlikely but if it happens we need to transcribe the old buf.
            SFLOAT *db = new SFLOAT[samples_out*last_nch];
            if (dstbuffer_size) {
              memcpy(db, dstbuffer, dstbuffer_size*sizeof(SFLOAT)*last_nch);
              delete[] dstbuffer;
			}
            dstbuffer = db;
            dstbuffer_size = samples_out;
          }
          else {
            if (dstbuffer_size)
              delete[] dstbuffer;
            dstbuffer = new SFLOAT[samples_out*last_nch];
            dstbuffer_size = samples_out;
          }
        }
        SFLOAT *db = dstbuffer + n;
        for (int s=0, r=0; s<samples_out; s++, r+=last_nch)
          db[r] = data_out[s];
      }}
      dst_samples_filled = samples_out;
    }
  } while (!buffer_full);
  next_sample += count;
}
	~AVSsupereq()
	{
    delete[] dstbuffer;
    delete[] passbuffer;
    for (size_t i = 0; i < eqs.size(); ++i)
      delete eqs[i];
	}


};

AVSValue __cdecl Create_SuperEq(AVSValue args, void*, IScriptEnvironment* env) {
  return new AVSsupereq(args[0].AsClip(), args[1].AsString(), env);
}

AVSValue __cdecl Create_SuperEqCustom(AVSValue args, void*, IScriptEnvironment* env) {
  int eq[N_BANDS];
  AVSValue args_c = args[1];
  const int num_args = args_c.ArraySize();
  for (int i = 0; i<N_BANDS; i++) {
    eq[i] = i<num_args ? args_c[i].AsInt() : 0;
  }
  return new AVSsupereq(args[0].AsClip(), eq, env);
}
