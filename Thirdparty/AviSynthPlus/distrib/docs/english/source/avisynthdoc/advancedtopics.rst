
Advanced Topics
===============


.. toctree::
    :maxdepth: 3

.. contents:: Table of contents


.. _Interlaced and field-based video:

Interlaced and field-based video
--------------------------------

Currently (v2.5x and older versions), AviSynth has no interlaced flag which
can be used for interlaced video. There is a field-based flag, but contrary
to what you might expect, *this flag is not related to interlaced video*. In
fact, all video (progressive or interlaced) is frame-based, unless you use
AviSynth filters to change that. There are two filters who turn frame-based
video into field-based video: :doc:`SeparateFields <corefilters/separatefields>`
and :doc:`AssumeFieldBased <corefilters/parity>`.

More information about this can be found :doc:`here <advancedtopics/interlaced_fieldbased>`


.. _Video Sampling:

Color format conversions, the Chroma Upsampling Error and the 4:2:0 Interlaced Chroma Problem
---------------------------------------------------------------------------------------------

The *Chroma Upsampling Error* is the result of your video is upsampled
incorrectly (interlaced YV12 upsampled as progressive or vice versa).
Visually, it means that you will often see gaps on the top and bottom of
colored objects and "ghost" lines floating above or below the objects. The
*4:2:0 Interlaced Chroma Problem* is the problem that 4:2:0 Interlaced itself
is flawed. The cause is that frames which show both moving parts and static
parts are upsampled using interlaced upsampling. This result in chroma
problems which are visible on bright-colored diagonal edges (in the static
parts of the frame). More about these issues can be found :doc:`here <advancedtopics/sampling>`.


.. _ColorSpace Conversions:

Colorspace Conversions
----------------------

About the different RGB <-> YUV :doc:`color conversions <advancedtopics/color_conversions>`.


.. _Wrong levels:

Wrong levels and colors upon playback
-------------------------------------

When playing back video content, several issues might go wrong. The levels
could be wrong, resulting in washed out colors (black is displayed as dark
gray and white is displayed as light gray). This is described in more detail
:doc:`here <advancedtopics/luminance_levels>`. The other issue is a slight
distortion in color (which often looks like a small change in brightness) and
this is described :doc:`here <advancedtopics/colorimetry>`.


.. _Hybrid Video:

AviSynth, variable framerate (vfr) video and Hybrid video
---------------------------------------------------------

There are two kinds of video when considering framerate. Constant framerate
(cfr) video and variable framerate (vfr) video. For cfr video the frames have
a constant duration, and for vfr video the frames have a non-constant
duration. Many editing programs (including VirtualDub and AviSynth) assume
that the video has cfr. One of the reasons is that avi doesn't support vfr.
This won't change in the near future for `various reasons`_. Although the avi
container doesn't support vfr, there are several contains (mkv, mp4 and wmv
for example) which do support vfr.

It's important to realize that in general video is intrinsically cfr (at
least in the capping video or ripping dvds arena). There is one exception
where converting to vfr is very useful, which is hybrid video. Hybrid video
consists of parts which are interlaced/progressive NTSC (29.97 fps) and FILM
(which is telecined to 29.97 fps). When playing hybrid video the NTSC part
(also called video part) is played back at 29.97 fps and the telecined part
at 23.976 fps.  Examples of hybrid video include some of the anime and Star
Trek stuff.

More info about creating vfr video and opening it in AviSynth can be found
:doc:`here <advancedtopics/hybrid_video>`.


.. _Importing Media into AviSynth:

Importing your media in AviSynth
--------------------------------

A lot of media formats (video, audio and images) can be imported into
AviSynth by using one of AviSynth's internal filters, specific plugins or
DirectShowSource in combination with the appropriate DirectShow filters. It
is not always trivial to import your media into AviSynth, because there are
often many ways to do so, and for each way you need to have some specific
codecs installed. :doc:`This document <advancedtopics/importing_media>`
describes which formats can be imported in AviSynth and how they should be
imported. Also a short summary is included about how to make graphs (graphs
of approriate DirectShow filters which can be used to play you media file)
in Graphedit and how to open the graphs in AviSynth.


.. _Resizing:

Resizing
--------

Resampling is the process of converting a signal from one sampling rate to
another, while changing the information carried by the signal as little as
possible. When applied to an image, this process is sometimes called image
scaling. More about image scaling, various resampling kernels and the
implementation in AviSynth can be found `on avisynth.nl`_ (...).

$Date: 2010/02/28 14:31:47 $

.. _various reasons:
    http://forum.doom9.org/showthread.php?s=&threadid=69132
.. _on avisynth.nl: http://avisynth.nl/index.php/Resampling
