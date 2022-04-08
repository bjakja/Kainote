
Getting started with audio
==========================

The best filters to take a look at if you are searching for a way to get
started with an audio filter is the `internal audio filters`_ of AviSynth.
Mainly `audio.cpp`_ is interesting.

Basically you override GetAudio(...) instead of GetFrame, and fill the buffer
with data. A simple filter could look like this:


Filter creation - skip if no audio:
:::::::::::::::::::::::::::::::::::

::

    AVSValue __cdecl HalfVolume::Create(AVSValue args, void*, IScriptEnvironment* env) {
        if (!args[0].AsClip()->GetVideoInfo().HasAudio())
            return args[0];

        // Auto convert audio to a compatible format.
        AVSValue CA_args[3] = { args[0], SAMPLE_INT16 | SAMPLE_FLOAT, SAMPLE_FLOAT };
        PClip clip = env->Invoke("ConvertAudio", AVSValue(CA_args, 3)).AsClip();

        return new HalfVolume(clip);
    }


What ConvertAudio() does is, that you tell it that your filter supports
SAMPLE_INT16 and SAMPLE_FLOAT, and that it prefers SAMPLE_FLOAT. If the
input isn't 16 bit or float, it'll be converted to float, otherwise the
original PClip is returned.


Constructor
-----------

::

    HalfVolume::HalfVolume(PClip _child)
        : GenericVideoFilter(_child) { // Provide null GetFrame, GetParity, etc
    }


GetAudio override
-----------------

::

    void __stdcall HalfVolume::GetAudio(void* buf, __int64 start, __int64
    count, IScriptEnvironment* env) {
      child->GetAudio(buf, start, count, env);
      int channels = vi.AudioChannels();

      if (vi.SampleType() == SAMPLE_INT16) {
        short* samples = (short*)buf;
        for (int i=0; i< count; i++) {
          for(int j=0;j< channels;j++) {
            samples[i*channels+j] += 1; // Round
            samples[i*channels+j] /= 2; // Halve
          }
        }
      } else if (vi.SampleType() == SAMPLE_FLOAT) {
        SFLOAT* samples = (SFLOAT*)buf;
        for (int i=0; i< count; i++) {
          for(int j=0;j< channels;j++) {
             samples[i*channels+j] /= 2.0f; // Halve, rounding not needed
          }
        }
      }
    }


Implementation of a half volume filter. Very explicit, so it isn't going to
be the fastest possible, but it should serve the purpose. Furthermore have a
look `discussion here`_ and look also at `audio.cpp`_ for a bunch of more
advanced stuff. A lot of technical details are also to be found in
:doc:`AviSynth Two-Five Audio <AviSynthTwoFiveAudio>`.

----

Since we are invoking ConvertAudio() you might wonder whether it is
possible to call it in a script. That is indeed possible, but not
documented since it is not very practical to do so. You need to know
that:

::

    SAMPLE_INT8  = 1b     = 1,
    SAMPLE_INT16 = 10b    = 2,
    SAMPLE_INT24 = 100b   = 4,
    SAMPLE_INT32 = 1000b  = 8,
    SAMPLE_FLOAT = 10000b = 16

(although these values might change some day).

In the example above we call ConvertAudio() with

::

    SAMPLE_INT16
    SAMPLE_FLOAT = 10b
    10000b = 10010b = 18,

so you need to call it as ConvertAudio(clip, 18, 16).

You should realise that float audio will be converted to 16 bit when
feeding it to the encoder. At least unless you have set :doc:`global
OPT_AllowFloatAudio = True <../syntax/syntax_internal_functions_control>` in your script.
__________________________________________________________________

Back to :doc:`FilterSDK`

$Date: 2015/09/14 20:23:59 $

.. _internal audio filters:
    http://avisynth2.cvs.sourceforge.net/avisynth2/avisynth/src/audio/
.. _audio.cpp: http://avisynth2.cvs.sourceforge.net/avisynth2/avisynth/src/audio/audio.cpp?view=markup
.. _ConvertAudio.cpp: http://avisynth2.cvs.sourceforge.net/avisynth2/avisynth/src/audio/convertaudio.cpp?view=markup
.. _discussion here: http://forum.doom9.org/showthread.php?s=&threadid=72760&highlight=ConvertAudiohere
