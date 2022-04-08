// Avisynth v2.5.  Copyright 2002 Ben Rudiak-Gould et al.
// http://www.avisynth.org

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA, or visit
// http://www.gnu.org/copyleft/gpl.html .
//
// Linking Avisynth statically or dynamically with other modules is making a
// combined work based on Avisynth.  Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.
//
// As a special exception, the copyright holders of Avisynth give you
// permission to link Avisynth with independent modules that communicate with
// Avisynth solely through the interfaces defined in avisynth.h, regardless of the license
// terms of these independent modules, and to copy and distribute the
// resulting combined work under terms of your choice, provided that
// every copy of the combined work is accompanied by a complete copy of
// the source code of Avisynth (the version of Avisynth used to produce the
// combined work), being distributed under the terms of the GNU General
// Public License plus this exception.  An independent module is a module
// which is not derived from or based on Avisynth, such as 3rd-party filters,
// import and export plugins, or graphical user interfaces.

#include "ssrc-convert.h"
#include <avs/config.h>
#ifdef AVS_WINDOWS
#include <avs/win.h>
#endif


/******************************************
 *******   Resample Audio using SSRC******
 *****************************************/


SSRC::SSRC(PClip _child, int _target_rate, bool _fast, IScriptEnvironment* env)
  : GenericVideoFilter(_child), target_rate(_target_rate), fast(_fast)
{
  srcbuffer = nullptr;  // If constructor should return
  res = nullptr;

  if ((target_rate==vi.audio_samples_per_second)||(vi.audio_samples_per_second==0)) {
		skip_conversion=true;
		return;
	}

  skip_conversion = false;
  source_rate = vi.audio_samples_per_second;

  factor = double(target_rate) / vi.audio_samples_per_second;
  vi.num_audio_samples = (vi.num_audio_samples * target_rate + vi.audio_samples_per_second - 1) / vi.audio_samples_per_second; // Ceil

  res = SSRC_create(source_rate, target_rate, vi.AudioChannels(), 2,1, fast);

  if (!res)
    env->ThrowError("SSRC: could not resample between the two samplerates.");

  input_samples = source_rate;  // We convert one second of input per loop.
  vi.audio_samples_per_second = target_rate;
  srcbuffer = new SFLOAT[vi.AudioChannels() * input_samples];

  for(int i=0; i<vi.AudioChannels() * input_samples; i++)  // We stuff one second into the input (preroll)
    srcbuffer[i] = 0.0f;

  next_sample = -target_rate;
  inputReadOffset = 0;
  res->Write(srcbuffer, input_samples * vi.AudioChannels());

}

 /***************************************
 * Implementation notes:
 * - For some strange reason SSRC delivers a
 * loud click in the very beginning of the sample.
 *   Therefore we preroll a second of audio whenever
 * a new instance is created.
 ****************************************/


void __stdcall SSRC::GetAudio(void* buf, int64_t start, int64_t count, IScriptEnvironment* env)
{
  if (skip_conversion) {
		child->GetAudio(buf, start, count, env);
		return;
	}

  count *= vi.AudioChannels();   // This is how SSRC keeps count. We'll do the same

  if (start != next_sample) {  // Reset on seek
    bool skip_restart = false;

    if (start > next_sample) { // Don't seek if within 10 sek, but skip
      if ((next_sample + input_samples * 10) > start) {
        skip_restart = true;
      }
    }

    if (!skip_restart)  {
      inputReadOffset = (start * source_rate / target_rate) -  input_samples;  // Floor, Reset at new read position minus ONE second.
      res = SSRC_create(source_rate, target_rate, vi.AudioChannels(), 2, 1, fast);
      _RPT2(0, "SSRC: Resetting position. Next_sample: %d. Start:%d.!\n", (int)next_sample, (int)start);
      next_sample = start - target_rate;
    }
  }


  int ssrc_samples_tbr = int(count)*sizeof(SFLOAT);  // count in bytes
  bool buffer_full = false;


  if (next_sample < start) { // Skip
    int skip_count = int(start - next_sample) * vi.AudioChannels();
    _RPT1(0,"SSRC: Skipping %u samples", skip_count);
    int skip_nsamples = skip_count*sizeof(SFLOAT);  // count in bytes
    do {
      int ssrc_samples_av;
      res->GetBuffer(&ssrc_samples_av);

      if (ssrc_samples_av < skip_nsamples) {  // We don't have enough bytes - feed more.

        child->GetAudio(srcbuffer, inputReadOffset, input_samples, env);
        inputReadOffset += input_samples;

        res->Write(srcbuffer, input_samples * vi.AudioChannels());

      } else {  // Now we have enough data
        res->Read(skip_count);
        next_sample += start;
        buffer_full = true;
      }
    } while (!buffer_full);

    buffer_full = false;
  }


  do {
    int ssrc_samples_av;
    SFLOAT* ssrc_samples = res->GetBuffer(&ssrc_samples_av);

    if (ssrc_samples_av < ssrc_samples_tbr) {  // We don't have enough bytes - feed more.

      child->GetAudio(srcbuffer, inputReadOffset, input_samples, env);
      inputReadOffset += input_samples;
      res->Write(srcbuffer, input_samples * vi.AudioChannels());

    } else {  // Now we have enough data

      env->BitBlt((BYTE*)buf, ssrc_samples_tbr, (BYTE*)ssrc_samples, ssrc_samples_tbr, ssrc_samples_tbr , 1);
      res->Read((unsigned int)count);
      buffer_full = true;
    }

  } while (!buffer_full);
  next_sample = start + (count/vi.AudioChannels());

}

AVSValue __cdecl Create_SSRC(AVSValue args, void*, IScriptEnvironment* env) {

  PClip clip = args[0].AsClip();

  if (!clip->GetVideoInfo().HasAudio())
    env->ThrowError("Input clip does not have audio.");

  if (!(clip->GetVideoInfo().SampleType()&SAMPLE_FLOAT))
    env->ThrowError("Input audio sample format to SSRC must be float.");

  return new SSRC(args[0].AsClip(), args[1].AsInt(), args[2].AsBool(true), env);
}
