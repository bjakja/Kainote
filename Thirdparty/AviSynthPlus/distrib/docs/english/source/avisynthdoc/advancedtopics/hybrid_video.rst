
AviSynth, variable framerate (vfr) video and hybrid video
=========================================================

There are two kinds of video when considering framerate, constant framerate
(cfr) video and variable framerate (vfr) video. For cfr video the frames have
a constant duration, and for vfr video the frames have a non-constant
duration. Many editing programs (including VirtualDub and AviSynth) assume
that the video is cfr, partly because avi doesn't support vfr. This won't
change in the near future for `various reasons`_. Although the avi container
doesn't support vfr, there are several containers (mkv, mp4 and wmv/asf for
example) which do support vfr.

Hybrid video is commonly defined as being a mix of pulled-down material and
non-pulled-down material (where the pulldown can be of fields, as in standard
3:2 pulldown, or full frames). It's not relevant whether the pulldown is hard
(the fields/frames are duplicated before the encoding) or soft (adding the
appriopriate flags in the stream which indicate which fields/frames should be
duplicated during playback). So, it can be either cfr or vfr. Thus hybrid
video is simply video with different base framerates (for example 8, 12, and
16 fps at which anime is often drawn). The base framerate is the rate before
any pulldown. What makes hybrids challenging is the need to decide what final
framerate to use.


.. toctree::
    :maxdepth: 3

.. contents:: Table of contents



Variable framerate and hybrid video
-----------------------------------

It's important to understand that usually video is cfr. There is one example
where converting to vfr can be very useful, which is hybrid video. Hybrid
video is video with different base framerates (for example 8, 12, and 16 fps
at which anime is often drawn). The most common example of hybrid video
consists of parts that are interlaced/progressive NTSC (29.97 fps) and other
parts which are FILM (telecined from 23.976 fps to 29.97 fps). For soft
pulldown, the NTSC part (also called video part) is played back at 29.97 fps
and the telecined part also by duplicating fields (to go from 23.976 fps to
29.97 fps). For hard pulldown, it is played back at 29.97 fps without adding
any fields. Other examples of hybrid video include many of the modern anime
TV Series, many of the Sci-Fi TV Series, such as Stargate: SG1, Star Trek:
TNG, and Babylon 5), and many of the "Making Of" documentaries included on
DVD.

Examples of hybrid video include many of the modern anime TV Series, many of
the Sci-Fi TV Series (such as Stargate: SG1, Star Trek: TNG, and Babylon 5),
and many of the "Making Of" documentaries included on DVD.

The TIVTC package is designed to work with hybrid video losslessly, while the
Decomb package has routines to convert to cfr via blending.


How to recognize vfr content (mkv/mp4)
--------------------------------------

Here are some ways to determine if the mkv/mp4 is vfr:

*mpeg-2:* DGIndex will report a Film/Video percentage, which can tell you
much hybrid content a soft-pulldowned file has. It will not work with hard
pulldown, and isn't always accurate if hard/soft are mixed.

*mkv:* get timecodes file using `mkv2vfr`_ to check this.

*mp4:* this can be found out by using mp4dump (from the `MPEG4 tools by
MPEG4ip package`_). Open a dos prompt and type (using appropriate paths)
::

    mp4dump -verbose=2 holly_xvid.mp4 > log.txt

Open the log file, and look for output like this (look up the stts atom to
figure out the length of each frame):

::

    type stts
           version = 0 (0x00)
           flags = 0 (0x000000)
           entryCount = 41 (0x00000029)
            sampleCount = 3 (0x00000003)
            sampleDelta = 1000 (0x000003e8)
            sampleCount[1] = 1 (0x00000001)
            sampleDelta[1] = 2000 (0x000007d0)
            sampleCount[2] = 3 (0x00000003)
            sampleDelta[2] = 1000 (0x000003e8)
            sampleCount[3] = 1 (0x00000001)
            sampleDelta[3] = 2000 (0x000007d0)
            etc ...

| *sampleDelta* indicates how long the frames get displayed and *sampleCount*
  tells how many frames. Thus on the example above:
| 3 frames are displayed with length 1000
| 1 frame are displayed with length 2000
| 3 frames with length 1000
| 1 frame with length 2000
| etc ...

The time values are not seconds, but "ticks", which you have to calculate
into seconds via the "timescale" value.  This "timescale" is stored in
timescale atom for the video track (make sure that you look at the right
timescale for your track, cause every track has its own timescale). Look for
output like this:

::

    type mdia
        type mdhd
    ...
         timeScale = 24976 (0x00006190)
         duration = 208000 (0x00032c80)
         language = 21956 (0x55c4)
         reserved = <2 bytes> 00 00

In this example the timeScale is 24976. Most of the frames have a length of
1000. 1000/24976 = 0.04 which means each frame of the first 3 gets displayed
with a length of 0.04 seconds, which is the equivalent to 25 fps (1/25 =
0.04). The next frame has a length of 2000. 2000/24976 = 0.08 which means
that it is displayed with a length of 0.08, which is the equivalent to 12.5
fps (1/12.5 = 0.08). etc ...

The log file above comes from a video which is in fact hybrid.


Opening MPEG-2 hybrid video in AviSynth and re-encoding
-------------------------------------------------------

Assuming you have hybrid video, there are several ways to encode it. They are
listed below. The first method is to convert it to cfr video (either 23.976
or 29.97 fps). The second one is to encode it at 120 fps using avi and
dropped frames (where duplicate frames are dropped upon playback). The third
one is to create true vfr using the mkv or mp4 container.


encoding to cfr (23.976 fps or 29.97 fps)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

If we choose the video rate, the video sequences will be OK, but the FILM
sequences will not be decimated, appearing slightly jumpy (due to the
duplicated frames). On the other hand, if we choose the FILM rate, the FILM
sequences will be OK, but the video sequences will be decimated, appearing
jumpy (due to the "missing" frames). Additionally, when encoding to 29.97
fps, you will get lower quality for the same file size, because of the 25%
greater number of frames. It's a tough decision which to choose. If the clip
is mostly FILM you might choose 23.976 fps, and if the clip is mostly video
you might choose 29.97 fps. The source also is a factor. If the majority of
the video portions are fairly static "talking heads", for example, you might
be able to decimate them to 23.976 fps without any obvious stutter on
playback.

When you create your d2v project file you will see whether the clip is mostly
video (NTSC) or FILM (in the information box). However, many of these hybrids
are encoded entirely as NTSC, with the film portions being "hard telecined"
(the already telecined extra fields having also been encoded) so you'll have
to examine the source carefully to determine what you have, and how you wish
to treat it.

The AviSynth plugins Decomb and TIVTC provide two special decimation modes to
better handle hybrid clips by blending. This will eat bitrate quickly, but it
appears very smooth. Here is a typical script to enable this mode of
operation:

::

    Telecide(order=0, guide=1)
    Decimate(mode=X) # tweak "threshold" for film/video detection

or

::

    TFM(mode=1)
    TDecimate(mode=0,hybrid=X) # tweak "vidThresh" for film/video detection

There are 2 factors that enable Decimate to treat the film and nonfilm
portions appropriately. First, when Telecide declares guide=1, it is able to
pass information to Decimate about which frames are derived from film and
which from video. For this mechanism to work, Decimate must immediately
follow Telecide. Clearly, the better job you do with pattern locking in
Telecide (by tweaking parameters as required), the better job Decimate can
do.

The second factor is the threshold. If a cycle of frames is seen that does
not have a duplicate, then the cycle is treated as video. The threshold
determines what percentage of frame difference is considered to be a
duplicate. Note that threshold=0 disables the second factor.

Make sure to get the field order correct - DVDs are generally order=1, and
captured video is generally order=0. The included DecombTutorial?.html
explains how to determine the field order.

*Mostly Film Clips (mode=3)*

When the clip is mostly film, we want to decimate the film portions normally
so they will be smooth. For the nonfilm portions, we want to reduce their
frame rate by blend decimating each cycle of frames from 5 frames to 4
frames. Video sequences so rendered appear smoother than when they are
decimated as film. Set Decimate to mode=3, or TDecimate to hybrid=1 for this
behavior.

Another IVTC was developed specifically to handle hybrid material without
blended frames: SmartDecimate. While you do get "clean" frames as a result,
it also may play with slightly more stutter than does Decomb's result. A
typical script might go:

::

    B = TDeint(mode=1) # or KernelBob(order=1)
    SmartDecimate(24, 60, B)

In order to keep the result as smooth playing as possible, it will insert the
"Smart Bobbed" frames from time to time.

*Mostly Video Clips (mode=1)*

When the clip is mostly video, we want to avoid decimating the video portions
in order to keep playback as smooth as possible. For the film portions, we
want to leave them at the video rate but change the duplicated frames into
frame blends so it is not so obvious. Set Decimate to mode=1, or TDecimate to
hybrid=3 for this behavior.

In this case you may also consider leaving it interlaced and encoding as
such, especially if you'll be watching on a TV later.


encoding to cfr - 120 fps
~~~~~~~~~~~~~~~~~~~~~~~~~

For this you'll need `TIVTC and avi_tc`_. Start by creating a
`decimated avi with timecodes.txt <http://www.avisynth.org/VariableFrameRateVideo>`_,
but skip the muxing. Then open tc-gui's tc2cfr tab and add your files or
use this command line:

::

    tc2cfr 120000/1001 c:\video\video.avi c:\video\timecodes.txt c:\video\video-120.avi

Then mux with your audio. This works because tc2cfr creates an avi with drop
frames filling in the extra space with drop frames to create a smooth 120fps
avi.


.. _create-vfr-mkv:

encoding to vfr (mkv)
~~~~~~~~~~~~~~~~~~~~~

First download `mkvtoolnix`_. We will use this to mux our video into the MKV
container WITH a timecode adjustment file. Make sure that you have the latest
version (4.9.1 as of this writing), as older ones read timecodes incorrectly.

There are several AviSynth plugins that you can use to generate the VFR video
and required timecode file. An example is given below using the
`Decomb521VFR`_ plugin. Another alternative is the TDecimate plugin contained
in the `TIVTC`_ package. See their respective documentations to learn more
about tweaking them.

The `DeDup <http://avisynth.org/warpenterprises/>`_ plugin removes duplicate frames but does not change the
framerate (leaving jerky video if not decimated first), so it won't be
included. It can still be used after either method by using their timecodes
as input to DeDup.

*Decomb521VFR*

Add this to your script:

::

    Decomb521VFR_Decimate(mode=4, threshold=1.0, progress=true, timecodes="timecodes.txt", vfrstats="stats.txt")

Open this script in VirtualDub, it will create the timecodes and stats files,
then encode. It will seem to freeze at first, because it examines every frame
on the first load.

*TIVTC*

This is a 2-pass mode. Add this to your script:

::

    TFM(mode=1, output="tfm.txt")
    TDecimate(mode=4, output="stats.txt")

Open this and play through it in VirtualDub. Then close it, comment those
lines out (or start a second script) and add:

::

    TFM(mode=1, input="tfm.txt")
    TDecimate(mode=5, hybrid=2, dupthresh=1.0, input="stats.txt", tfmin="tfm.txt", mkvout="timecodes.txt")

Load and encode.

*framerate*

If you're encoding to a specific size using a bitrate calculator, vfr
decimation will mess up the calculations. To make them work again add these
to your script:

Before decimation:

::

    oldcount = framecount # this line must be before decimation
    oldfps = framerate

End of script:

::

    averagefps = (float(framecount)/float(oldcount))*oldfps
    AssumeFPS(averagefps)

Now mux to MKV:

1.  Open mmg.exe (mkvmerge gui)
2.  Add your video stream file
3.  Add your audio stream file
4.  Click on the imported video track
5.  Browse for the "timecodes.txt" timecode file
6.  Click on the audio track
7.  If your audio already needs a delay, set one
8.  Start muxing

To play it you need a Matroska splitter. For AVC you will need `Haali's
Splitter`_, but for ASP you can use it or `Gabest's Splitter`_.


encoding to vfr (mp4)
~~~~~~~~~~~~~~~~~~~~~

If you create a 120 fps avi with drop-frames, however, the mp4 muxed from it
will remove them along with any n-vops the encoder creates, leaving vfr. A
more laborous way is to encode multiple cfr avi files (some with 23.976 fps
film and some with 29.97 fps video) and join them directly into one vfr mp4
file with mp4box and the -cat option.

A third, much easier, method is to encode using the MKV method and then
processing the video with tc2mp4: more details on tc2mp4 can be found on the
[`Doom9 forums`_].


summary of the methods
~~~~~~~~~~~~~~~~~~~~~~

Summing up the advantages and disadvantages of the above mentioned methods.
When encoding to 23.976 or 29.97 fps the clip will be cfr (which editors like
AviSynth and Virtualdub need), but it may look jumpy on playback due to
duplicated or missing frames. That can be avoided with blending, but encoders
can't work as well with that. When encoding to 120 fps using drop frames, the
clip is cfr, not jumpy on playback, and very compatible. Encoding to mkv
using true vfr (using timecodes) neither loses nor duplicates frames, however
it is not nearly as broadly supported as AVI.


Opening non MPEG-2 hybrid video in AviSynth and re-encoding
-----------------------------------------------------------

It is possible to open vfr video in AviSynth without losing sync:
DirectShowSource. The most common formats that support hybrid video (vfr) are
**mkv**, **mp4**, **wmv**, and **rmvb**, and the methods below work for all
of them; however, if the source is mkv, you can also use  `mkv2vfr`_ and
AviSource.


opening non-avi vfr content in AviSynth
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The best way to get all frames while keeping sync and timing is to convert to
a common framerate, such as 120 fps for 24/30 (or rather 119.88). (Always use
convertfps=true, which adds frames like ChangeFPS, or your audio * will* go
out of sync.)

::

    DirectShowSource("F:\Hybrid\vfr.mp4", fps=119.88, convertfps=true)

You can also open it as 30p, which then has to be re-decimated but has less
frames to deal with, or 24p, breaking any 30p sections:

Re-encoding to 23.976 or 29.97 fps:

::

    DirectShowSource("F:\Hybrid\vfr.mkv", fps=29.97, convertfps=true) # or fps=23.976

or

::

    DirectShowSource("F:\Hybrid\vfr_startrek.mkv", fps=119.88, convertfps=true)
    FDecimate(29.97) # or FDecimate(23.976)

Another way is to find out the average framerate (by dividing the total
number of frames by the duration in seconds) and use this rate in
DirectShowSource. Depending on the duration of a frame, frames will be added
or dropped to keep sync, and it's almost guaranteed to stutter.
DirectShowSource will not telecine.


re-encoding 120 fps video
~~~~~~~~~~~~~~~~~~~~~~~~~

The easiest way to convert vfr sources back into vfr in AviSynth is by using
`DeDup. <http://akuvian.org/src/avisynth/dedup/>`_

1st pass:

::

    DupMC(log="stats.txt")

2nd pass:

::

    DeDup(threshold=.1,maxcopies=4,maxdrops=4,dec=true,log="stats.txt",times="timecodes.txt")

TIVTC can also do this:

1st pass:

::

    TFM(mode=0,pp=0)
    TDecimate(mode=4,output="stats.txt")

2nd pass:

::

    TFM(mode=0,pp=0)
    TDecimate(mode=6,hybrid=2,input="stats.txt",mkvout="timecodes.txt")

Once you've encoded your file, mux back to mkv or 120 fps avi.

This will chop out all the duplicate frames directshowsource inserts, while
keeping framecount and timing nearly identical. But do not use the timecode
file from the input video, use the new one. They may not be identical. (Of
course you can play with parameters if you want to use more of the
functionality of dedup.)


converting vfr to cfr avi for AviSynth
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

You can avoid analysing and decimating by using special tools to get a
minimal constant-rate avi to feed avisynth. After processing and re-encoding,
use tc2cfr or mmg on the output with the original timecodes to regain vfr and
full sync. (If you perform any kind of decimation or frame-rate change you'll
have to edit the timecode file yourself, although dedup does have a timesin
parameter.)

*avi*

`avi_tc`_ will create a timecode and normal video, if the avi uses drop
frames and not n-vops or fully encoded frames. It also requires that no audio
or secondary tracks are present. To use it, open tc-gui and add your file, or
use the following command line:

::

    cfr2tc c:\video\video-120.avi c:\video\video.avi c:\video\timecodes.txt 1

*mkv*

`mkv2vfr`_ extracts all video frames from Matroska to a normal AVI file and a
timecode file. This will only work if the mkv is in vfw-mode. The command-
line to use it is:

::

    mkv2vfr.exe input.mkv output.avi timecodes.txt

encoding to MPEG-2 vfr video
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

`<http://forum.doom9.org/showthread.php?t=93691>`_

I didn't look at it yet, so i can't give any comments/hints.


Audio synchronization
---------------------

Several methods are discussed to encode your video (at 23.976, 29.97 or vfr
video). You might wonder why your audio stays in sync regardless of the
method you used to encode your video. Prior to encoding, the video and audio
have the same duration, so they start out in sync. The following two
situations might occur:

-   you change the framerate of the stream by speeding it up or slowing
    it down (as is often done by PAL-FILM conversions). This implies that the
    duration of the video stream will change, and hence the audio stream will
    become out of sync.
-   you change the framerate of the stream by adding or removing frames.
    This implies that the duration of the video stream will remain the same,
    and hence the audio stream will be in sync.

If you encode the video stream at 23.976 or 29.97 fps (both cfr) by using
Decimate(mode=3, threshold=1.0) or Decimate(mode=1, threshold=1.0), frames
will be removed or added, and thus your audio stream will be in sync. By a
similar reasoning the vfr encoding will be in sync.

Finally, suppose you open vfr video in AviSynth with DirectShowSource.
Compare the following

::

    DirectShowSource("F:\Hybrid\vfr_startrek.mkv", fps=29.97) # or fps=23.976

and

::

    DirectShowSource("F:\Hybrid\vfr_startrek.mkv", fps=29.97, convertfps=true) # or fps=23.976

The former will be out of sync since 24p sections are speeded up, and the
latter will be in sync since frames are added to convert it to cfr.

**To Do:**

-   tc2mp4, subs/Aegisub and ffmpegsource for timecode file:
    `<http://forums.animesuki.com/showthread.php?t=34738>`_
    `<http://forum.doom9.org/showthread.php?t=112199>`_
-   download `WMVTIMES.exe`_.
-   `subs also <http://forum.doom9.org/showthread.php?t=135889&page=2>`_
-   `how to determine whether a video (MP4) is vfr or not? <http://forum.doom9.org/showthread.php?t=137899>`_
-   Wilbert: I don't understand the comment about DeDup in "encoding to
    vfr (mkv)": need to investigate.


References
----------

Essential reading:

- `Force Film, IVTC, and Deinterlacing and more`_ (an article written by some people from at doom9).
- Creating `120 fps video`_.
- Documentation of `Decomb521VFR`_.
- About `Decomb521VFR1.0`_ mod for automated Matroska VFR.
- `Mkvextract GUI`_ by DarkDudae.

*Besides all people who contributed to the tools mentioned in this guide, the
author of this tutorial (Wilbert) would like to thank bond, manono, tritical
and foxyshadis for their useful suggestions and corrections of this
tutorial.*

$Date: 2011/12/04 15:28:20 $

.. _various reasons:
    http://forum.doom9.org/showthread.php?s=&threadid=69132
.. _mkv2vfr: http://haali.cs.msu.ru/mkv/mkv2vfr.exe
.. _MPEG4 tools by MPEG4ip package: http://www.rarewares.org/mp4.html
.. _TIVTC and avi_tc: http://bengal.missouri.edu/~kes25c/
.. _avi_tc: http://bengal.missouri.edu/~kes25c/
.. _mkvtoolnix: http://www.bunkus.org/videotools/mkvtoolnix/downloads.html
.. _Decomb521VFR: http://webpages.charter.net/falconx/decombvfrmod.html
.. _TIVTC: http://www.missouri.edu/~kes25c/
.. _Haali's Splitter: http://haali.cs.msu.ru/mkv/
.. _Gabest's Splitter: http://sourceforge.net/projects/guliverkli/
.. _Doom9 forums: http://forum.doom9.org/showthread.php?t=112199
.. _WMVTIMES.exe: http://fcchandler.home.comcast.net/WMVTIMES.exe
.. _Force Film, IVTC, and Deinterlacing and more:
    http://www.doom9.org/ivtc-tut.htm
.. _120 fps video: http://www.masteryoshidino.com/hentai/anime-encoding.htm
.. _Decomb521VFR1.0:
    http://forum.doom9.org/showthread.php?s=&threadid=80673
.. _Mkvextract GUI: http://forum.doom9.org/showthread.php?t=73819
