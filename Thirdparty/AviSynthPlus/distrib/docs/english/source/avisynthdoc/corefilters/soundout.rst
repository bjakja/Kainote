
SoundOut
========

SoundOut is a GUI driven sound output module for AviSynth.


Installation and Usage
----------------------

Copy "SoundOut.dll" and "libsndfile-1.dll" to your AviSynth plugin directory,
usually ``"c:\program files\avisynth 2.5\plugins"``. If you want to have
"SoundOut.dll" at another location, you should move "libsndfile-1.dll" to
your system32 folder, usually ``"c:\windows\system32"``.

Add ``SoundOut()`` to your script, where you would like to export audio. If
you have your video stored in a variable, use ``SoundOut(variable)`` to add
SoundOut. A GUI should pop up, when you open your script. Here is a simple
example of how to use it:

::

    AviSource("myvideo.avi")
    SoundOut()

If you need some sample processing, to change samplerate or otherwise edit
your video, you must do it before calling the SoundOut module. Like this:

::

    AviSource("myvideo.avi")
    Amplifydb(3)
    SSRC(44100)
    SoundOut()

Output Modules
--------------


WAV/AIF/CAF
~~~~~~~~~~~

This will allow you to export uncompressed audio to the following formats:

-   Microsoft WAV format
-   Apple/SGI AIFF format
-   Sun/NeXT AU format
-   RAW PCM data
-   Sonic Foundry's 64 bit RIFF/WAV (WAVE64)
-   Apple Core Audio File format
-   Microsoft WAV format with Broadcast Wave Format chunk.

Note, that 8 bit samples are NOT supported in the Core Audio File and
Sun/NeXT AU format.


FLAC
~~~~

This will allow you to export lossless compressed audio FLAC format.

FLAC supports 8,16 or 24 bit audio. Any other format is internally converted
to 24 bit.


APE
~~~

This will allow you to export lossless compressed audio to the Monkey Audio
Codec (APE) format.

APE does not support input sample sizes that are larger than 2GB. Use only
for smaller files.

APE supports 8, 16 or 24 bit audio. Any other format is internally converted
to 24 bit.


MP2
~~~

This will allow you to compress your audio to MPEG 1 Layer 2 (MP2).

TwoLame only supports 16 mono or stereo audio. If you attempt to compress
more than two channels, an error will be shown. Any other format than 16 bit
integer samples are internally converted to 16 bit.


MP3
~~~

This will allow you to compress your audio to MPEG 1 Layer 3 (MP3) using LAME
v3.97 encoder.

| LAME Supports up to two channels of audio and the following samplerates:
| 48000, 44100, 32000, 24000, 22050, 16000, 12000, 11025 and 8000 Hz.


AC3
~~~

This will allow you to compress your audio to A/52 (AC3). The encoding is
done via libaften.

Aften supports 1 to 6 channel audio. Supported samplerates are 48000, 44100
or 32000 samples per second.

Channel mapping is:

+--------------------+---------------------------------------------------------+
| Number of channels | Channel order                                           |
+====================+=========================================================+
| 1                  | Center                                                  |
+--------------------+---------------------------------------------------------+
| 2                  | Left, Right                                             |
+--------------------+---------------------------------------------------------+
| 3                  | Left, Center, Right                                     |
+--------------------+---------------------------------------------------------+
| 4                  | Left, Right, Surround Left, Surround Right              |
+--------------------+---------------------------------------------------------+
| 5                  | Left, Center, Right, Surround Left, Surround Right      |
+--------------------+---------------------------------------------------------+
| 6                  | Left, Center, Right, Surround Left, Surround Right, LFE |
+--------------------+---------------------------------------------------------+

OGG
~~~

This will allow you to compress your audio to an Vorbis encoded OGG file. It
is possible to give an average bitrate, or do the encode as CBR.


WavPack
~~~~~~~

WavPack is a completely open audio compression format providing lossless,
high-quality lossy compression mode. Compatible with virtually all PCM audio
formats including 8, 16, 24, and 32-bit ints; 32-bit floats; mono, stereo,
and multichannel; sampling rates from 6 to 192 kHz (and non-standard rates)


Commandline Output
~~~~~~~~~~~~~~~~~~

This output module will allow you to output to any program that supports
input from stdin. This gives you complete control of your encoding, if you
have commandline tools for the job.

You can select the format SoundOut should deliver to the application you use.
There are three WAV formats and RAW PCM data. This is sent to stdin of the
application. The program builds the command line from 4 parts, *the
executable*, *command line options* before the output file, the *output file*
that is selected, and *command line options* after the output file name.

There are two ways of specifying the executable. Either give complete path to
the executable, or simply enter the executable's filename, and place it in a
subdirectory called SoundOut in your plugin directory.


Exporting from script
---------------------

It is possible to use SoundOut as an ordinary filter, running inside the
script and giving parameters for each output mode. The parameters consists of
two things: General Parameters, which can be used for all filters, and filter
specific parameters, which gives parameters to the active output module.

The **out** parameter determines whether the GUI will be shown, if it is
properly set, the filter will begin exporting audio as soon as it is started.

If the **out** parameter is **not** set, it is still possible to set
additional parameters. The defaults will however be retrieved from the
registry, but specific parameters override


General Parameters
~~~~~~~~~~~~~~~~~~

+----------------+---------+----------------------------------------------------------------------------------------+
| Parameter name | Type    | Values                                                                                 |
+================+=========+========================================================================================+
| output         | string  || Select output module to use. Possible values are:                                     |
|                |         || "WAV", "AC3", "MP2", "MP3", "OGG", "FLAC", "MAC", "WV" and "CMD".                     |
|                |         || If none, or an invalid value is given, the ordinary GUI will be shown.                |
+----------------+---------+----------------------------------------------------------------------------------------+
| filename       | string  || Full path to the output filename, including extension.                                |
|                |         || No extra quotes are required.                                                         |
|                |         || If no filename is given a file selector will pop up.                                  |
+----------------+---------+----------------------------------------------------------------------------------------+
| showprogress   | bool    || Show the progress window? Default: true                                               |
+----------------+---------+----------------------------------------------------------------------------------------+
| overwritefile  | string  || "Yes": Always overwrite file.                                                         |
|                |         || "No": Never Overwrite file                                                            |
|                |         || "Ask": Ask if file should be overwritten.                                             |
+----------------+---------+----------------------------------------------------------------------------------------+
| autoclose      | bool    || Should the progress window close automatically 5 seconds after                        |
|                |         |  encoding has finished?                                                                |
|                |         || This will also code the window, even though an error occurred                         |
|                |         || Default: false                                                                        |
+----------------+---------+----------------------------------------------------------------------------------------+
| silentblock    | bool    || When processing, enabling this option will return silent                              |
|                |         |  samples instead of blocking the requesting application. If disabled, any              |
|                |         |  application requesting audio will be blocking, while sound is being exported          |
|                |         || Default: true                                                                         |
+----------------+---------+----------------------------------------------------------------------------------------+
| addvideo       | bool    || When enabled, this will add video to the current output, if none                      |
|                |         |  is present. The video is a black 32x32 pixels at 25fps, with the length of the audio. |
|                |         || Default: true                                                                         |
+----------------+---------+----------------------------------------------------------------------------------------+
| wait           | integer || How many seconds should the output window be shown, if autoclose is on.               |
|                |         || Default: 5.                                                                           |
+----------------+---------+----------------------------------------------------------------------------------------+

WAV/AIF/CAF Script Parameters
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

+----------------+---------+----------------------------------------------------+
| Parameter name | Type    | Values                                             |
+================+=========+====================================================+
| type           | integer || Select WAVE format                                |
|                |         || 0: Microsoft WAV (default),                       |
|                |         || 1: WAV with WAVEFORMATEX,                         |
|                |         || 2: Apple/SGI AIFF,                                |
|                |         || 3: Sun/NeXT AU,                                   |
|                |         || 4: RAW PCM,                                       |
|                |         || 5: S.F. WAVE64,                                   |
|                |         || 6: Core Audio File,                               |
|                |         || 7: Broadcast Wave.                                |
+----------------+---------+----------------------------------------------------+
| format         | integer || Sets the sample format number of bits per sample. |
|                |         || 0: 16bit per sample,                              |
|                |         || 1: 24bit per sample,                              |
|                |         || 2: 32bit per sample,                              |
|                |         || 3: 32bit float per sample,                        |
|                |         || Default: Same as input.                           |
+----------------+---------+----------------------------------------------------+
| peakchunck     | bool    || Add Peak chunk to WAV file?                       |
|                |         || Default: false                                    |
+----------------+---------+----------------------------------------------------+

Audio will be written in the format delivered to the SoundOut plugin. All
internal sound formats are supported.


FLAC Script Parameters
~~~~~~~~~~~~~~~~~~~~~~

+------------------+---------+-------------------------------------------------------+
| Parameter name   | Type    | Values                                                |
+==================+=========+=======================================================+
| compressionlevel | integer || Sets the compression level. 1(fastest) to 8(slowest) |
|                  |         || Default: 6                                           |
+------------------+---------+-------------------------------------------------------+

APE Script Parameters
~~~~~~~~~~~~~~~~~~~~~

+------------------+---------+-------------------------------------------------------+
| Parameter name   | Type    | Values                                                |
+==================+=========+=======================================================+
| compressionlevel | integer || Sets the compression level. 1(fastest) to 6(slowest) |
|                  |         || Default: 3                                           |
+------------------+---------+-------------------------------------------------------+

MP2 Script Parameters
~~~~~~~~~~~~~~~~~~~~~

+----------------+---------+-------------------------------------------------------+
| Parameter name | Type    | Values                                                |
+================+=========+=======================================================+
| bitrate        | integer || Sets Bitrate for CBR or maximum bitrate for VBR.     |
|                |         || Default: 192                                         |
+----------------+---------+-------------------------------------------------------+
| stereomode     | integer || -1: Automatic (default)                              |
|                |         || 0: Separate Stereo                                   |
|                |         || 1: Separate Stereo                                   |
|                |         || 2: Joint Stereo                                      |
|                |         || 3: Dual Channel                                      |
|                |         || 4: Mono                                              |
+----------------+---------+-------------------------------------------------------+
| psymodel       | integer || -1: Fast & Dumb                                      |
|                |         || 0: Low complexity                                    |
|                |         || 1: ISO PAM 1                                         |
|                |         || 2: ISO PAM 2                                         |
|                |         || 3: PAM 1 Rewrite (default)                           |
|                |         || 4: PAM 2 Rewrite                                     |
+----------------+---------+-------------------------------------------------------+
| vbrquality     | float   || Sets VBR Quality. Useful range is about -10 to 10.   |
|                |         || Default is 0                                         |
+----------------+---------+-------------------------------------------------------+
| vbr            | bool    || Encode as VBR?                                       |
|                |         || Default: false.                                      |
+----------------+---------+-------------------------------------------------------+
| quick          | bool    || Quick Encode?                                        |
|                |         || Default: false.                                      |
+----------------+---------+-------------------------------------------------------+
| dab            | bool    || Add DAB Extensions?                                  |
|                |         || Default: false.                                      |
|                |         || *According to TwoLame documentation this might not   |
|                |         |  be reliable.*                                        |
+----------------+---------+-------------------------------------------------------+
| crc            | bool    || Add CRC Error checks?                                |
|                |         || Default: false.                                      |
+----------------+---------+-------------------------------------------------------+
| original       | bool    || Set Original Flag?                                   |
|                |         || Default: false.                                      |
+----------------+---------+-------------------------------------------------------+
| copyright      | bool    || Set Copyright flag?                                  |
|                |         || Default: false.                                      |
+----------------+---------+-------------------------------------------------------+
| emphasis       | integer || Set Emphasis flag.                                   |
|                |         || 0: No Emphasis (default)                             |
|                |         || 1: 50/15 ms                                          |
|                |         || 3: CCIT J.17                                         |
+----------------+---------+-------------------------------------------------------+


MP3 Script Parameters
~~~~~~~~~~~~~~~~~~~~~

+----------------+---------+-------------------------------------------------------+
| Parameter name | Type    | Values                                                |
+================+=========+=======================================================+
| mode           | integer || Sets Encoding mode:                                  |
|                |         || 0: VBR (default)                                     |
|                |         || 1: ABR                                               |
|                |         || 2: CBR                                               |
+----------------+---------+-------------------------------------------------------+
| vbrpreset      | integer || Sets quality preset, when using VBR mode.            |
|                |         || Standard = 1001 (default),                           |
|                |         || extreme = 1002,                                      |
|                |         || insane = 1003,                                       |
|                |         || standard_fast = 1004,                                |
|                |         || extreme_fast = 1005,                                 |
|                |         || medium = 1006,                                       |
|                |         || medium_fast = 1007                                   |
+----------------+---------+-------------------------------------------------------+
| abrrate        | integer || Sets Average bitrate for ABR encoding.               |
|                |         || Default: 128                                         |
+----------------+---------+-------------------------------------------------------+
| cbrrate        | integer || Sets Bitrate for CBR encoding.                       |
|                |         || Default: 128                                         |
+----------------+---------+-------------------------------------------------------+

AC3 Script Parameters
~~~~~~~~~~~~~~~~~~~~~

+---------------------+---------+--------------------------------------------------------------+
| Parameter name      | Type    | Values                                                       |
+=====================+=========+==============================================================+
| iscbr               | bool    || Encode at Constant Bitrate?                                 |
|                     |         || Default: true.                                              |
+---------------------+---------+--------------------------------------------------------------+
| cbrrate             | integer || Sets Bitrate for CBR or maximum bitrate for VBR.            |
|                     |         || Default: 384                                                |
+---------------------+---------+--------------------------------------------------------------+
| vbrquality          | integer || VBR Bitrate quality. Values between 1 and 1023 are accepted |
|                     |         || Default: 220.                                               |
+---------------------+---------+--------------------------------------------------------------+
| drc                 | integer || Dynamic Range Compression                                   |
|                     |         || 0: Film Light                                               |
|                     |         || 1: Film Standard                                            |
|                     |         || 2: Music Light                                              |
|                     |         || 3: Music Standard                                           |
|                     |         || 4: Speech                                                   |
|                     |         || 5: None (default)                                           |
+---------------------+---------+--------------------------------------------------------------+
| acmod               | integer || Set channel mapping                                         |
|                     |         || 0 = 1+1 (Ch1,Ch2)                                           |
|                     |         || 1 = 1/0 (C)                                                 |
|                     |         || 2 = 2/0 (L,R)                                               |
|                     |         || 3 = 3/0 (L,R,C)                                             |
|                     |         || 4 = 2/1 (L,R,S)                                             |
|                     |         || 5 = 3/1 (L,R,C,S)                                           |
|                     |         || 6 = 2/2 (L,R,SL,SR)                                         |
|                     |         || 7 = 3/2 (L,R,C,SL,SR)                                       |
+---------------------+---------+--------------------------------------------------------------+
| dialognormalization | integer || Dialog normalization. Values from 0 to 31 are               |
|                     |         |  accepted                                                    |
|                     |         || Default: 31.                                                |
+---------------------+---------+--------------------------------------------------------------+
| islfe               | bool    || Is there LFE channel present?                               |
|                     |         || Default: false if less than 4 channels, true otherwise.     |
+---------------------+---------+--------------------------------------------------------------+
| bandwidthfilter     | bool    || Use the bandwidth low-pass filter?                          |
|                     |         || Default: false.                                             |
+---------------------+---------+--------------------------------------------------------------+
| lfelowpass          | bool    || Use the LFE low-pass filter                                 |
|                     |         || Default: false.                                             |
+---------------------+---------+--------------------------------------------------------------+
| dchighpass          | bool    || Use the DC high-pass filter                                 |
|                     |         || Default: false.                                             |
+---------------------+---------+--------------------------------------------------------------+
| dolbysurround       | bool    || Is the material Dolby Surround encoded? (only applies to    |
|                     |         |  stereo sound, otherwise ignored)                            |
|                     |         || Default: false.                                             |
+---------------------+---------+--------------------------------------------------------------+
| blockswitch         | bool    || Selectively use 256-point MDCT?                             |
|                     |         || Default: false (Use only 512-point MDCT).                   |
+---------------------+---------+--------------------------------------------------------------+
| accuratealloc       | bool    || Do more accurate encoding?                                  |
|                     |         || Default: true.                                              |
+---------------------+---------+--------------------------------------------------------------+

OGG Script Parameters
~~~~~~~~~~~~~~~~~~~~~

+----------------+---------+-------------------------------------------+
| Parameter name | Type    | Values                                    |
+================+=========+===========================================+
| vbrbitrate     | integer || Selects the average bitrate to encode at |
|                |         || Default: 128.                            |
+----------------+---------+-------------------------------------------+
| cbr            | bool    || Encode as CBR?                           |
|                |         || Default: false.                          |
+----------------+---------+-------------------------------------------+

Wavpack Script Parameters
~~~~~~~~~~~~~~~~~~~~~~~~~

+------------------+---------+----------------------------------------------------+
| Parameter name   | Type    | Values                                             |
+==================+=========+====================================================+
| compressionlevel | integer || Sets the compression level. 0(Very Fast) to       |
|                  |         |  5(Extremely Slow)                                 |
|                  |         || Default: 2 (Normal)                               |
+------------------+---------+----------------------------------------------------+
| format           | integer || Sets the sample format number of bits per sample. |
|                  |         || 0: 8bit per sample,                               |
|                  |         || 1: 16bit per sample,                              |
|                  |         || 2: 24bit per sample,                              |
|                  |         || 3: 32bit per sample,                              |
|                  |         || 4: 32bit float per sample,                        |
|                  |         || Default: Same as input.                           |
+------------------+---------+----------------------------------------------------+

Commandline Output Script Parameters
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

+----------------+---------+--------------------------------------------------------+
| Parameter name | Type    | Values                                                 |
+================+=========+========================================================+
| type           | integer || Select WAVE format                                    |
|                |         || 0: Microsoft WAV (default),                           |
|                |         || 1: WAV with WAVEFORMATEX,                             |
|                |         || 2: RAW PCM,                                           |
|                |         || 3: S.F. WAVE64.                                       |
+----------------+---------+--------------------------------------------------------+
| format         | integer || Select Output Bits per sample                         |
|                |         || 0: 16 Bit                                             |
|                |         || 1: 24 Bit                                             |
|                |         || 2: 32 Bit                                             |
|                |         || 3: 32 bit float                                       |
|                |         || Default is same as input.                             |
+----------------+---------+--------------------------------------------------------+
| executable     | string  || Executable to use                                     |
|                |         || Default: "aften.exe" (without quotes).                |
+----------------+---------+--------------------------------------------------------+
| prefilename    | string  || Parameters that are placed before the output filename |
|                |         || Default: "-b 384 -" (without quotes).                 |
+----------------+---------+--------------------------------------------------------+
| postfilename   | string  || Parameters that are placed after the output filename  |
|                |         || Default: "" (without quotes).                         |
+----------------+---------+--------------------------------------------------------+
| showoutput     | bool    || Show the output window?                               |
|                |         || Default: true.                                        |
+----------------+---------+--------------------------------------------------------+
| nofilename     | bool    || Encode without output filename, and don't use         |
|                |         |  postfilename?                                         |
|                |         || Default: false.                                       |
+----------------+---------+--------------------------------------------------------+

Examples
--------

::

    SoundOut(output = "mp3", filename="c:\outputFile.mp3", autoclose = true, \
             showprogress=true, mode = 2, cbrrate = 192)
    # Engages mp3 output module with CBR at 192kbit/sec.

Implementation notes
--------------------

SoundOut is Multithreaded, and uses one thread for requesting audio from the
previous filters, and another thread for encoding. The threads are given a
"below normal" priority.

Only attempt to run two exports at the same time at your own risk. It is most
likely slower and could potentially crash. You can safely export sound while
you encode, if your encode does not read audio from AviSynth.


+------------+----------------------------------+
| Changelist |                                  |
+============+==================================+
| v2.60      | Initial Release; based on v1.1.1 |
+------------+----------------------------------+

+-----------------------+---------------------------------------------------+
| Changelist (SoundOut) |                                                   |
+=======================+===================================================+
| v1.1.1                | - Downgraded FLAC to v1.2.0, to avoid backwards   |
|                       |   incompatible 24 bit files.                      |
|                       | - Conversion tune-up.                             |
|                       | - OverWriteFile set to "No" was not respected.    |
|                       | - Client sample requests shown in GUI.            |
+-----------------------+---------------------------------------------------+
| v1.1.0                | - Added WavPack output module.                    |
|                       | - Added Sample type selection to WAV Output.      |
|                       | - Updated FLAC to v 1.2.1 - 24 bit/sample seems   |
|                       |   broken, so only 8 & 16 bit are enabled.         |
|                       | - Fixed bug in FLAC to enable files larger than   |
|                       |   2GB.                                            |
|                       | - FLAC now uses the same GUI as other filters.    |
|                       | - Aften updated.                                  |
|                       | - Re-enabled Aften multithreading.                |
|                       | - Faster 3DNOW! float to 24 bit conversion.       |
+-----------------------+---------------------------------------------------+
| v1.0.3                | - Vorbis, AC3 and MP3 now checks if file can be   |
|                       |   created.                                        |
|                       | - Fixed hang in aften on multiprocessor machines. |
|                       | - Added wait parameter, how many seconds should   |
|                       |   SoundOut wait on autoclose.                     |
|                       | - Avoid lockup if encoder cannot be initialized   |
|                       |   and set for direct output.                      |
|                       | - Fixed OverwriteFile was not always being        |
|                       |   respected.                                      |
+-----------------------+---------------------------------------------------+
| v1.0.2                | - Updated libaften to rev534.                     |
|                       | - Fixed overwriteFile not being recognized in     |
|                       |   script.                                         |
|                       | - Fixed crash if mp2 file could not be opened for |
|                       |   writing.                                        |
|                       | - Exit blocked, even if filter is (almost)        |
|                       |   instantly destroyed, if script is set for       |
|                       |   output.                                         |
|                       | - AC3 is now reporting the actual samples encoded |
|                       |   (including padding).                            |
+-----------------------+---------------------------------------------------+
| v1.0.1                | - Updated libaften to rev. 512.                   |
|                       | - Added overwriteFile="yes"/"no"/"ask".           |
|                       |   Default is Ask.                                 |
+-----------------------+---------------------------------------------------+
| v1.0.0                | - The application will not exit, as long as an    |
|                       |   encode window is open.                          |
|                       | - Fixed "nofilename" not being recognized in      |
|                       |   script.                                         |
|                       | - LFE no longer overridden by registry, when      |
|                       |   using GUI.                                      |
+-----------------------+---------------------------------------------------+
| v0.9.9                | - Added ReplayGain calculation to Analyze.        |
|                       | - Parent filters are now blocked, or silent       |
|                       |   samples are returned, if the filter is          |
|                       |   currently exporting sound.                      |
|                       | - Video is automatically added, if none is        |
|                       |   present. (black 32x32 RGB32)                    |
|                       | - Buttons for export are disabled when output     |
|                       |   window is open.                                 |
|                       | - Main window is now minimized when export module |
|                       |   is selected.                                    |
|                       | - Fixed Analyze bug on 16 bit samples.            |
|                       | - Fixed WAVEFORMATEXTENSIBLE channel mapping in   |
|                       |   Commandline Output.                             |
|                       | - AC3 output: LFE option disabled when not        |
|                       |   relevant.                                       |
|                       | - AC3 output: LFE option named properly.          |
+-----------------------+---------------------------------------------------+
| v0.9.8                | - Added Analyze option to calculate average,      |
|                       |   maximum and RMS levels. Only available through  |
|                       |   GUI.                                            |
|                       | - WAVEFORMATEXTENSIBLE in commandline out         |
|                       |   attempts to set channel maps based on channel   |
|                       |   number.                                         |
|                       | - Fixed thread race issue on very fast encoders.  |
|                       | - Minor GUI tweaks.                               |
+-----------------------+---------------------------------------------------+
| v0.9.7                | - Added channelmapping to AC3 output.             |
|                       | - Added LFE channel indicator switch to AC3       |
|                       |   output.                                         |
|                       | - GUI now spawned in a new thread, fixing GUI     |
|                       |   lockup in foobar2000 and similar.               |
|                       | - Fixed general thread race issue, where a fast   |
|                       |   encoder might lead to incomplete output.        |
|                       | - Fixed WAVE_FORMAT_EXTENSIBLE header without     |
|                       |   info in CmdLine Output.                         |
|                       | - Fixed "Format" not working on Commandline       |
|                       |   output.                                         |
|                       | - Fixed Filename dialog not appearing.            |
|                       | - Forced final samplereading to be correct.       |
|                       | - Removed "private" option from MP2 GUI and       |
|                       |   script, as there is no way to set it via        |
|                       |   twolame.                                        |
|                       | - Removed DAB Extensions from MP2 GUI, as TwoLame |
|                       |   reports it as not functioning.                  |
+-----------------------+---------------------------------------------------+
| v0.9.6                | - Added complete script customization.            |
|                       | - Added possibility to set output file from       |
|                       |   script.                                         |
|                       | - Added window autoclose option to script.        |
|                       | - Added option to script to disable progress      |
|                       |   window.                                         |
|                       | - GUI creates message handle thread.              |
|                       | - Settings are now saved to registry if output    |
|                       |   filter initializes successfully.                |
|                       | - Updated documentation.                          |
+-----------------------+---------------------------------------------------+
| v0.9.5                | - Added Broadcast WAVE out.                       |
|                       | - Fixed OGG Vorbis support.                       |
|                       | - Fixed Text fields not being correctly read.     |
|                       | - Fixed AC3 settings not being restored properly. |
|                       | - Added: MP2 settings are now saved.              |
+-----------------------+---------------------------------------------------+
| v0.9.4                | - Added OGG Vorbis support.                       |
|                       | - Added: Parameters stored (on save) and read to  |
|                       |   registry.                                       |
|                       | - Added: "No filename needed" option in           |
|                       |   commandline output, to disable output filename  |
|                       |   prompt.                                         |
|                       | - Fixed collision between libaften and libvorbis. |
|                       | - Updated libaften to rev 257.                    |
|                       | - Enabled SSE optimizations in libaften.          |
|                       | - Hopefully fixed issue with commandline          |
|                       |   executable filename becoming garbled.           |
+-----------------------+---------------------------------------------------+
| v0.9.3                | - Added Commandline piping output.                |
|                       | - Added MP3 / LAME output.                        |
|                       | - Fixed AC3 VBR Error sometimes wrongly being     |
|                       |   displayed.                                      |
|                       | - Fixed AC3 DRC Setting not being respected.      |
|                       | - Various GUI bugfixes.                           |
+-----------------------+---------------------------------------------------+
| v0.9.2                | - Updated AC3 GUI.                                |
|                       | - Fixed crash in WAV output.                      |
|                       | - More stats during conversion.                   |
+-----------------------+---------------------------------------------------+
| v0.9.1                | - Added AC3 Output.                               |
|                       | - Added new parameter handling.                   |
|                       | - Fixed last block not being encoded.             |
+-----------------------+---------------------------------------------------+

$Date: 2011/04/29 20:09:50 $
