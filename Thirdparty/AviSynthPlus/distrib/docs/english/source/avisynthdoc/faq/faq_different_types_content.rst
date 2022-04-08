
AviSynth FAQ - Recognizing and processing different types of content
====================================================================


.. toctree::
    :maxdepth: 3

.. contents:: Table of contents



The video and audio in my final encoding is out of sync, what should I do?
--------------------------------------------------------------------------

Assuming that you processed your video and or audio with AviSynth, there can
be several reasons why your final encoding is not in sync (synchronization).
The most common ones are:

1) Your source is already out of sync (thus before any AviSynth
   processing or any encoding). It's a pain to correct this, but that's not
   the scope of this FAQ.

2) The audio has a constant delay, and you forgot to add the delay
   (either in AviSynth if you imported the audio in AviSynth or in an
   encoder if you imported the audio directly in your encoder). As an
   example, the demuxed audio stream from a VOB has often a delay. When
   demuxing this audio stream with DGIndex, the delay (actually how the
   delay should be corrected) is written into the name of the demuxed audio
   stream. You can use :doc:`DelayAudio <../corefilters/delayaudio>` to add the delay in AviSynth.

::

    vid = MPEG2Source("D:\movie.d2v")
    aud = NicAC3Source("D:\movie T01 2_0ch 448Kbps DELAY -218ms.ac3")
    AudioDub(vid, aud)
    DelayAudio(-0.218)

3) The audio has a variable delay (with a zero delay at the beginning and
   a maximal delay at the end). This can be caused when you load a clip into
   AviSynth which has a variable framerate. Pretty much anything except
   video contained in an AVI or MPEG-2/VOB file can be variable framerate.
   If you used `DirectShowSource`_ the load your clip, you can use the
   following to ensure sync. What happens is that frames are added or
   removed to ensure sync, thus converting it to a constant framerate video.

::

    # a mkv-file is used here as an example:
    DirectShowSource("D:\movie.mkv", fps=xxx, convertfps=true) # fps = average framerate

If you are not using DirectShowSource or you don't want to add or remove
frames, you need to create a timecodes file first and use it later on in your
final encoding. Have a look at :doc:`this article <../advancedtopics/hybrid_video>` for more information on this
subject.


How do I recognize progressive, interlaced, telecined, hybrid and blended content?
----------------------------------------------------------------------------------

It is important to know your content if you want to process it. The most
important ones are: progressive, interlaced, telecined, hybrid and blended
content, and they should be processed differently.

-   **Progressive and interlaced content:**

|     Most filters assume that your content is progressive (which means that every
      frame is taken at a different time-instant), unless the filter has an option
      interlaced=true/false. When the option is present you can use interlaced=true
      for interlaced content. For interlaced content, every field (a frame consists
      of two fields) is taken at a different time-instant. This is explained in the
      `Analog Capture Guide`_ and the `Force Film, IVTC, and Deinterlacing`_
      tutorial.

-   **Telecined content:**

|     Usually movies are shot at 24 fps (frames per second). When putting this on a
      dvd, fields are added to get the required frame rate of 30 fps (well, it's
      actually 29.97 fps, but that's not important here). When doing this, the
      content is called "telecined content" (this holds for the conversions 25 fps
      -> 30 fps and 24 fps -> 25 fps as well, provided fields are added). More
      about this can be found in the `Force Film, IVTC, and Deinterlacing`_
      tutorial.

-   **Hybrid content:**

|     Hybrid content is content with different base frame rates (for example 8, 12,
      and 16 fps at which anime is often drawn). Start Trek is a different example
      consisting of telecined (at 30 fps) and interlaced content (at 30 fps).  Have
      a look at :doc:`this article <../advancedtopics/hybrid_video>` for more information on this subject.

-   **Blended content:**

|     Blended content is content which consists of blended fields (in some fields
      there is content from different time-instants visible). It's usually the
      result of bad NTSC to PAL conversions (and vice-versa), or messed-up
      deinterlacing. Some examples can be found `in these guides`_ `on Doom9`_.


How do I process interlaced content?
------------------------------------

There are two ways to process your interlaced content (assuming that you use
a filter which has no interlaced=true option). The first one is the most
accurate, but also the slowest: bobbing, processing and reinterlacing. The
second one is the fastest, but also less accurate one: processing the fields
separately.

1) bobbing:

::

    AssumeTFF() # or AssumeBFF (set the video's field order correctly)
    TDeint(mode=1, type=3) # or any other smart Bob
    Filter(...)
    AssumeTFF() # or AssumeBFF (set the video's field order correctly)
    Separatefields()
    Selectevery(4,0,3)
    Weave()

2) processing the fields separately:

::

    SeparateFields()
    even = SelectEven(last).Filter(...)
    odd = SelectOdd(last).Filter(...)
    Interleave(even, odd)
    Weave()


How do I process telecined content?
-----------------------------------

You need to inverse telecine (IVTC) before you do any processing. You can use
the plugin Decomb for example, `which can be downloaded here`_. See the
tutorials "`Force Film, IVTC, and Deinterlacing - what is DVD2AVI trying to
tell you and what can you do about it.`_" or "`the analog capture guide`_"
which explain how to do this.


How do I process hybrid content?
--------------------------------

You only run into troubles when your clip as openend in AviSynth shows
combing (being partly interlaced, telecined, etc ...). I'm not sure yet what
to do in that case.


What is variable framerate video?
---------------------------------

There are two kinds of video when considering framerate, constant framerate
(cfr) video and variable framerate (vfr) video. For cfr video the frames have
a constant duration, and for vfr video the frames have a non-constant
duration. Many editing programs (including VirtualDub and AviSynth) assume
that the video is cfr, partly because avi doesn't support vfr. Although the
avi container doesn't support vfr, there are several containers (mkv, mp4 and
wmv/asf for example) which do support vfr. More information can be found
:doc:`here <../advancedtopics/hybrid_video>`.


How do I import variable framerate video into AviSynth and how do I process it?
-------------------------------------------------------------------------------

There are two ways to import variable framerate video into AviSynth:

1.  Open the video in AviSynth using for example DirectShowSource(...,
    convertfps=false) or FFmpegSource. The problem is that in those cases no
    frames are added or removed to convert it to constant framerate video to
    ensure sync.

Generate a timecode file using some external program or using the AviSynth
plugin you use for importing the video into AviSynth (if possible). Many non-
AVI files contain video with a variable framerate, and in that case you need
to make sure of the following two things:

    1.  *Don't change the framerate and the number of frames in
    AviSynth.* If you don't this (and you don't change the timecodes file
    manually) your video and audio in your final encoding will be out of
    sync.

    2.  *Use the timecodes file again when muxing your encoded video and
    audio.* If you don't do this your video and audio in your final encoding
    will be out of sync.

2.  Open the video in AviSynth using for example DirectShowSource(...,
    convertfps=true). In this case frames are added or removed to convert it
    to constant framerate video to ensure sync. You can process the video the
    way you want. You can even create a new timecodes file and create a new
    variable framerate video using it. More information can be found :doc:`here <../advancedtopics/hybrid_video>`.

Regarding the first way. If you did change the framerate or the number of
frames, you can use DeDup to recreate a new timecode file:

::

    dedup_dedup(threshold=0.1, maxcopies=10, maxdrops=4, log="01.log", timesin="original.tmc", times="final.tmc")

The parameter "timesin" specifies the timecode file of the original video on
which the output file will be based on (rather than just using the input
stream's framerate). I never used it, so I'm not sure how good this is. Look
`here for a discussion`_.

| :doc:`Main Page <faq_sections>` | :doc:`General Info <faq_general_info>` | :doc:`Loading Clips <faq_loading_clips>` | :doc:`Loading Scripts <faq_frameserving>` | :doc:`Common Error Messages <faq_common_errors>` | **Processing Different Content** | :doc:`Dealing with YV12 <faq_yv12>` | :doc:`Processing with Virtualdub Plugins <faq_using_virtualdub_plugins>` |

$Date: 2008/07/02 18:57:42 $

.. _DirectShowSource: http://avisynth.org/mediawiki/DirectShowSource
.. _Analog Capture Guide: http://www.doom9.org/index.html?/capture/introduction.html
.. _Force Film, IVTC, and Deinterlacing: http://www.doom9.org/ivtc-tut.htm
.. _in these guides:
    http://www.doom9.org/index.html?/capture/postprocessing_avisynth.html
.. _which can be downloaded here: http://neuron2.net/decomb/decombnew.html
.. _Force Film, IVTC, and Deinterlacing - what is DVD2AVI trying to tell
    you and what can you do about it.: http://www.doom9.org/ivtc-tut.htm
.. _on Doom9: http://www.doom9.org/ivtc-tut.htm
.. _the analog capture guide:
    http://www.doom9.org/index.html?/capture/postprocessing_avisynth.html
.. _here for a discussion: http://forum.doom9.org/showthread.php?t=121593
