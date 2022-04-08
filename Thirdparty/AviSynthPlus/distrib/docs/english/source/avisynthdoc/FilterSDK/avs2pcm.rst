
avs2pcm
=======

avs2pcm reads a script and outputs raw audio (`lpcm`_, that is lineair
pcm). The byte order will be little endian, the sign (signed or
unsiged) will depend on the bith depth and the channels will be
interleaved.

Here's avs2pcm.cpp:
::

    #include <stdio.h>
    #include <Windows.h>
    #include "avisynth.h"

    #define MY_VERSION "Avs2PCM 0.01"

    const AVS_Linkage *AVS_linkage = 0;

    int __cdecl main(int argc, const char* argv[])
    {
        const char* infile = NULL;
        const char* outfile = NULL;
        FILE* out_fh;

        if (!strcmp(argv[1], "-h")) {
            fprintf(stderr, MY_VERSION "\n"
            "Usage: avs2pcm in.avs out.pcm\n");
            return 2;
        } else {
            infile = argv[1];
            outfile = argv[2];
        }

        try {
            char* sample_type;
            typedef IScriptEnvironment* (__stdcall *DLLFUNC)(int);
            IScriptEnvironment* env;
            HMODULE avsdll = LoadLibrary("avisynth.dll");
            if (!avsdll) {
                fprintf(stderr, "failed to load avisynth.dll\n");
                return 2;
            }

            DLLFUNC CreateEnv = (DLLFUNC)GetProcAddress(avsdll, "CreateScriptEnvironment");
            if (!CreateEnv) {
                fprintf(stderr, "failed to load CreateScriptEnvironment()\n");
                FreeLibrary(avsdll);
                return 1;
            }

            env = CreateEnv(AVISYNTH_INTERFACE_VERSION);
            AVS_linkage = env->GetAVSLinkage();
            AVSValue arg(infile);
            AVSValue res = env->Invoke("Import", AVSValue(&arg, 1));
            if (!res.IsClip()) {
                fprintf(stderr, "Error: '%s' didn't return a video clip.\n", infile);
                FreeLibrary(avsdll);
                return 1;
            }

            PClip clip = res.AsClip();

            if (clip->GetVersion() < 5) {
                fprintf(stderr, "Error: too old version ('%d') of avisynth.dll loaded.\nplease install v2.60 or later.\n",
                    clip->GetVersion());
                return 1;
            }

            VideoInfo vi = clip->GetVideoInfo();

            if (!vi.HasAudio()) {
                fprintf(stderr, "Error: '%s' video only clip.\n", infile);
                FreeLibrary(avsdll);
                return 1;
            }

            fprintf(stderr, " %s:\n", infile);
            fprintf(stderr, " %d Herz,\n", vi.audio_samples_per_second);
            fprintf(stderr, " %d channels,\n", vi.nchannels);
            fprintf(stderr, " %I64d audio samples,\n", vi.num_audio_samples);

            switch(vi.SampleType()) {
            case SAMPLE_INT8 : sample_type = "8 bit";
                break;
            case SAMPLE_INT16 : sample_type = "16 bit";
                break;
            case SAMPLE_INT24 : sample_type = "24 bit";
                break;
            case SAMPLE_INT32 :
            case SAMPLE_FLOAT : sample_type = "32 bit";
                break;
            default: sample_type = "unknown sample type";
                break;
            }

            fprintf(stderr, " %s", sample_type);

            out_fh = fopen(outfile, "wb");
            if (!out_fh) {
                fprintf(stderr, "fopen(\"%s\") failed", outfile);
                FreeLibrary(avsdll);
                return 1;
            }

            const __int64 start = 0;
            const __int64 count = vi.num_audio_samples;
            const int channels = vi.AudioChannels();
            __int64 bytes = vi.BytesFromAudioSamples(count);
            int BlockAlign = vi.AudioChannels() * vi.BytesPerAudioSample();

            unsigned char* samples = new unsigned char[BlockAlign*count];
            clip->GetAudio(samples, start, count, env);
            fwrite(samples, bytes, 1, out_fh);

            delete[] samples;
            env->DeleteScriptEnvironment();
            FreeLibrary(avsdll);
            AVS_linkage = 0;

        } catch(AvisynthError err) {
            fprintf(stderr, "\nAvisynth error:\n%s\n", err.msg);
            return 1;
        }

        fclose(out_fh);
        return 0;
    }


Compile this file into an EXE named avs2pcm.exe. See
:doc:`compiling instructions <CompilingAvisynthPlugins>`. Now open the
command line and go to the folder where avs2pcm.exe and your script
(called example.avs here) are located. Our script:
::

    Tone(length=1, frequency=2, samplerate=48000, channels=1, type="square", level=1.0) # float
    ConvertAudioTo16Bit()


Type the following on the command line (the name of the output clip can
be arbitrary in our application):
::

    avs2pcm.exe example.avs output.pcm

So the output file will contain 48000 samples of 16-bit data (at 48
kHz, one channel). You can import it in AviSynth using the plugin
NicAudio:
::

    v = Blankclip(1000)
    a = RaWavSource("D:\AviSynth\Plugins\avs2pcm\output.pcm", 48000, 16, 1) # little-endian
    Audiodub(v,a).ConvertAudioTo16Bit().GetChannels(1) # Audiograph doesn't support 24/32bit nor multichannel
    Audiograph(20)


Line by line breakdown
----------------------

Here's a line-by-line breakdown of avs2pcm.cpp:
::

    #include <stdio.h>
    #include <Windows.h>
    #include "avisynth.h"

    #define MY_VERSION "Avs2PCM 0.01"

    const AVS_Linkage *AVS_linkage = 0;

    int __cdecl main(int argc, const char* argv[])
    {
        const char* infile = NULL;
        const char* outfile = NULL;
        FILE* out_fh;

        if (!strcmp(argv[1], "-h")) {
            fprintf(stderr, MY_VERSION "\n"
            "Usage: avs2pcm in.avs out.pcm\n");
            return 2;
        } else {
            infile = argv[1];
            outfile = argv[2];
        }

        try {
            char* sample_type;
            typedef IScriptEnvironment* (__stdcall *DLLFUNC)(int);
            IScriptEnvironment* env;
            HMODULE avsdll = LoadLibrary("avisynth.dll");
            if (!avsdll) {
                fprintf(stderr, "failed to load avisynth.dll\n");
                return 2;
            }

            DLLFUNC CreateEnv = (DLLFUNC)GetProcAddress(avsdll, "CreateScriptEnvironment");
            if (!CreateEnv) {
                fprintf(stderr, "failed to load CreateScriptEnvironment()\n");
                FreeLibrary(avsdll);
                return 1;
            }

            env = CreateEnv(AVISYNTH_INTERFACE_VERSION);
            AVS_linkage = env->GetAVSLinkage();
            AVSValue arg(infile);
            AVSValue res = env->Invoke("Import", AVSValue(&arg, 1));
            if (!res.IsClip()) {
                fprintf(stderr, "Error: '%s' didn't return a clip.\n", infile);
                FreeLibrary(avsdll);
                return 1;
            }

            PClip clip = res.AsClip();

            if (clip->GetVersion() < 5) {
                fprintf(stderr, "Error: too old version ('%d') of avisynth.dll loaded.\nplease install v2.60 or later.\n",
                    clip->GetVersion());
                return 1;
            }

            VideoInfo vi = clip->GetVideoInfo();


The lines above are explained in :doc:`avs2yuv <avs2yuv>`, so they won't be repeated
here.
::

        if (!vi.HasAudio()) {
            fprintf(stderr, "Error: '%s' video only clip.\n", infile);
            FreeLibrary(avsdll);
            return 1;
        }


Returns an error if the clip doesn't contain audio.
::

        fprintf(stderr, " %s:\n", infile);
        fprintf(stderr, " %d Herz,\n", vi.audio_samples_per_second);
        fprintf(stderr, " %d channels,\n", vi.nchannels);
        fprintf(stderr, " %I64d audio samples,\n", vi.num_audio_samples);

        switch(vi.SampleType()) {
        case SAMPLE_INT8 : sample_type = "8 bit";
            break;
        case SAMPLE_INT16 : sample_type = "16 bit";
            break;
        case SAMPLE_INT24 : sample_type = "24 bit";
            break;
        case SAMPLE_INT32 :
        case SAMPLE_FLOAT : sample_type = "32 bit";
            break;
        default: sample_type = "unknown sample type";
            break;
        }

        fprintf(stderr, " %s", sample_type);


Some information about the clip is written to the console.
::

        out_fh = fopen(outfile, "wb");


Creates an empty binary file and opens it for writing. It returns a
file pointer called 'out_fh' here. Nb, 'wb' means write mode and
binary.
::

        if (!out_fh) {
            fprintf(stderr, "fopen(\"%s\") failed", outfile);
            FreeLibrary(avsdll);
            return 1;
        }


When failing (thus when out_fh is NULL) an error is written to the
console.
::

        const __int64 start = 0;
        const __int64 count = vi.num_audio_samples;


This gives the number of audio samples in our stream.
::

        const int channels = vi.AudioChannels();


This gives the number of audio channels of our stream.
::

        __int64 bytes = vi.BytesFromAudioSamples(count);


We will use `fwrite`_ to write 'count' audio samples to a file. So we
will need to know the corresponding number of bytes which needs to be
written. :doc:`BytesFromAudioSamples <VideoInfo>` gives the number of bytes and it is
calculated internally as follows:

+-------------------------+-------------------------------------------+--------------------------------------+
| function                | value                                     | size                                 |
+=========================+===========================================+======================================+
| BytesPerChannelSample() || = sizeof(unsigned char)                  || = 1 byte [for 8 bitaudio],          |
|                         || = sizeof(signed short)                   || = 2 bytes [for 16 bit audio],       |
|                         |                                           || = 3 bytes [for 24 bit audio],       |
|                         || = sizeof(signed int)                     || = 4 bytes [for 32 bit audio],       |
|                         || = sizeof(SFLOAT)                         || = 4 bytes [for float audio;         |
|                         |                                           |  this is also 32 bit audio]          |
+-------------------------+-------------------------------------------+--------------------------------------+
| BytesPerAudioSample()   | AudioChannels() * BytesPerChannelSample() |                                      |
+-------------------------+-------------------------------------------+--------------------------------------+
| BytesFromAudioSamples() | num_audio_samples * BytesPerAudioSample() |                                      |
+-------------------------+-------------------------------------------+--------------------------------------+


::

        int BlockAlign = vi.BytesPerAudioSample();
        unsigned char* samples = new unsigned char[BlockAlign*count];
        clip->GetAudio(samples, start, count, env);
        fwrite(samples, bytes, 1, out_fh);
        delete[] samples;


There are a few ways to write audio to a file. The simpliest one is the
one above. Let's look at what happens with our data with an example:
::

    Tone(length=1, frequency=2, samplerate=48000, channels=1, type="square", level=1.0) # float
    ConvertAudioTox() // x = 8Bit, 16Bit, 24Bit, 32Bit and Float


The samples are always written to the pcm file as `little endian`_. So
this means the bytes are written in reversed order (thus the least
significant byte first and the most significant byte last).

+--------------------------------------------------+-------------------------------------------------------------------------------------------------------+-------------------------------+
| type (x)                                         | value of samples                                                                                      | bytes written in file         |
+==================================================+=======================================================================================================+===============================+
| 8Bit (samples between 0 and 2^8-1)               || s[0] = 255;                                                                                          | FF .. 00 ..                   |
|                                                  || s[count-1] = 0                                                                                       |                               |
+--------------------------------------------------+-------------------------------------------------------------------------------------------------------+-------------------------------+
| 16Bit (samples between -2^15 and 2^15-1)         || s[1]*256+s[0] = 127*256+255 = 32767;                                                                 | FF 7F .. 00 80 ..             |
|                                                  || s[count-1]*256+s[count-2] = 128*256+0 = 32768 (= -32768)                                             |                               |
+--------------------------------------------------+-------------------------------------------------------------------------------------------------------+-------------------------------+
| 24Bit (samples between -2^23 and 2^23-1)         || s[2]*16^4+s[1]*16^2+s[0] = 8388607;                                                                  | FF FF 7F .. 00 00 80 ..       |
|                                                  || s[count-1]*16^4+s[count-2]*16^2+s[count-3] = 8388608 (= -8388608)                                    |                               |
+--------------------------------------------------+-------------------------------------------------------------------------------------------------------+-------------------------------+
| 32Bit (samples between -2^31 and 2^31-1)         || s[3]*16^6+s[2]*16^4+s[1]*16^2+s[0] = 2147483647;                                                     | FF FF FF 7F .. 00 00 00 80 .. |
|                                                  || s[count-1]*16^6+s[count-2]*16^4+s[count-3]*16^2+s[count-4] = 2147483648 (= -2147483648)              |                               |
+--------------------------------------------------+-------------------------------------------------------------------------------------------------------+-------------------------------+
| `Float`_ (samples between -1.00000 and 1.000000) || s[3]*16^6+s[2]*16^4+s[1]*16^2+s[0] = 63*16^6+128*16^4+0*16^2+0 = 106535321;                          | 00 00 80 3F .. 00 00 80 BF .. |
|                                                  || s[count-1]*16^6+s[count-2]*16^4+s[count-3]*16^2+s[count-4] = 191*16^6+128*16^4+0*16^2+0 = 3212836864 |                               |
+--------------------------------------------------+-------------------------------------------------------------------------------------------------------+-------------------------------+


Above the samples are declared as unsigned char (regardless of the
number of bits in a sample), but they are filled by GetAudio as
explained above. Thus an audio sample is stored in multiple samples
(each sample having the size of a byte). This is the simpliest and
cleanest way to do, but you could also have done the following instead:
::

    int BlockAlign = vi.BytesPerAudioSample();

    switch (vi.SampleType()) {
    case SAMPLE_INT8 : {
        unsigned char* samples = new unsigned char[BlockAlign*count];
        clip->GetAudio(samples, start, count, env);
        fwrite(samples, bytes, 1, out_fh);
        delete[] samples;
        break;
        }
    case SAMPLE_INT16 : {
        signed short* samples = new signed short[channels*count];
        clip->GetAudio(samples, start, count, env);
        fwrite(samples, bytes, 1, out_fh);
        delete[] samples;
        break;
        }
    case SAMPLE_INT24 : {
        unsigned char* samples = new unsigned char[3*channels*count];
        clip->GetAudio(samples, start, count, env);
        fwrite(samples, bytes, 1, out_fh);
        delete[] samples;
        break;
        }
    case SAMPLE_INT32 : {
        signed int* samples = new signed int[channels*count];
        clip->GetAudio(samples, start, count, env);
        fwrite(samples, bytes, 1, out_fh);
        delete[] samples;
        break;
        }
    case SAMPLE_FLOAT : {
        SFLOAT* samples = new SFLOAT[channels*count];
        clip->GetAudio(samples, start, count, env);
        fwrite(samples, bytes, 1, out_fh);
        delete[] samples;
        break;
        }
    }


Here an audio sample is stored in one sample (having the size of a
multiple bytes), but fwrite will write the same bytes to the file as
earlier.

At first glance it seems possible to simplify the code above by moving
the lines "clip .. delete[] samples" outside the switch statement, but
that's not possible. The reason is that each code-block corresponding
to a label has its own scope. So the variable samples doesn't exist
outside the switch statement and can't be used there for further
processing. See `here`_ for more information.
::

        env->DeleteScriptEnvironment();
        FreeLibrary(avsdll);
        AVS_linkage = 0;

        } catch(AvisynthError err) {
            fprintf(stderr, "\nAvisynth error:\n%s\n", err.msg);
            return 1;
        }

        fclose(out_fh);
        return 0;
    }


The remaining lines above are explained in :doc:`avs2yuv <avs2yuv>`, so they won't be
repeated here.

____

Back to :doc:`FilterSDK`

$Date: 2014/10/27 22:04:54 $

.. _lpcm: http://wiki.multimedia.cx/index.php?title=PCM
.. _fwrite: http://www.cplusplus.com/reference/cstdio/fwrite/
.. _little endian: http://en.wikipedia.org/wiki/Endianness
.. _Float: http://avisynth.nl/index.php/Float
.. _here: http://stackoverflow.com/questions/92396/why-cant-variables-be-declared-in-a-switch-statement
