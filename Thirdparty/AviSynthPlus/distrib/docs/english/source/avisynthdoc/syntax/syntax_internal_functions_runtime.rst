
AviSynth Syntax - Runtime functions
===================================

These are the internal functions which are evaluated at every frame. They can
be used inside the scripts passed to runtime filters (:doc:`ConditionalFilter <../corefilters/conditionalfilter>`,
:doc:`ScriptClip <../corefilters/conditionalfilter>`, :doc:`FrameEvaluate <../corefilters/conditionalfilter>`) to return information for a frame (usually
the current one). When using these functions there is an implicit
**``last``** clip (the one that is passed to the runtime filter). Thus, first
parameter doesn't have to be specified; it is replaced by the ``last`` clip.

-   AverageLuma   |   v2.x   |   AverageLuma(clip)
-   AverageChromaU   |   v2.x   |   AverageChromaU(clip)
-   AverageChromaV   |   v2.x   |   AverageChromaV(clip)

This group of functions return a float value with the average pixel value of
a plane (Luma, U-chroma and V-chroma, respectively). They require that
``clip`` is in `YV12`_ `colorspace`_ and `ISSE`_.

*Examples:*
::

    threshold = 55
    luma = AverageLuma()
    luma < threshold ? Levels(0, 1.0 + 0.5*(threshold -
    luma)/threshold, 255, 10, 255) : last

-   RGBDifference   |   v2.x   |   RGBDifference(clip1, clip2)
-   LumaDifference   |   v2.x   |   LumaDifference(clip1, clip2)
-   ChromaUDifference   |   v2.x   |   ChromaUDifference(clip1, clip2)
-   ChromaVDifference   |   v2.x   |   ChromaVDifference(clip1, clip2)

This group of functions return a float value between 0 and 255 of the
absolute difference between two planes (that is two frames from two different
clips). Either the combined RGB difference or the Luma, U-chroma or V-chroma
differences, respectively. They require that ``clip`` is in `YV12`_
`colorspace`_ and `ISSE`_.

*Examples:*
::

    ovl = Overlay(last, mov_star, x=some_xvalue, y=some_yvalue, mask=mov_mask)
    ldif = LumaDifference(ovl) # implicit last for clip1
    udif = ChromaUDifference(Tweak(hue=24), ovl)
    ...


The next two groups of functions should be quite handy for detecting scene
change transitions:

-   RGBDifferenceFromPrevious   |   v2.x   |
    RGBDifferenceFromPrevious(clip)
-   YDifferenceFromPrevious   |   v2.x   |
    YDifferenceFromPrevious(clip)
-   UDifferenceFromPrevious   |   v2.x   |
    UDifferenceFromPrevious(clip)
-   VDifferenceFromPrevious   |   v2.x   |
    VDifferenceFromPrevious(clip)

This group of functions return the absolute difference of pixel value between
the current and previous frame of ``clip``. Either the combined RGB
difference or the Luma, U-chroma or V-chroma differences, respectively.

*Examples:*
::

    scene_change = YDifferenceFromPrevious() > threshold ? true : false
    scene_change ? some_filter(...) : another_filter(...)

-   RGBDifferenceToNext   |   v2.x   |   RGBDifferenceToNext(clip)
-   YDifferenceToNext   |   v2.x   |   YDifferenceToNext(clip)
-   UDifferenceToNext   |   v2.x   |   UDifferenceToNext(clip)
-   VDifferenceToNext   |   v2.x   |   VDifferenceToNext(clip)

This group of functions return the absolute difference of pixel value between
the current and next frame of ``clip``. Either the combined RGB difference or
the Luma, U-chroma or V-chroma differences, respectively.

*Examples:*
::

    # both th1, th2 are positive thresholds; th1 is larger enough than th2
    scene_change = YDifferenceFromPrevious() > th1 && YDifferenceToNext()
    < th2
    scene_change ? some_filter(...) : another_filter(...)

-   YPlaneMax   |   v2.x   |   YPlaneMax(clip, float threshold)
-   UPlaneMax   |   v2.x   |   UPlaneMax(clip, float threshold)
-   VPlaneMax   |   v2.x   |   VPlaneMax(clip, float threshold)
-   YPlaneMin   |   v2.x   |   YPlaneMin(clip, float threshold)
-   UPlaneMin   |   v2.x   |   UPlaneMin(clip, float threshold)
-   VPlaneMin   |   v2.x   |   VPlaneMin(clip, float threshold)
-   YPlaneMedian   |   v2.x   |   YPlaneMedian(clip)
-   UPlaneMedian   |   v2.x   |   UPlaneMedian(clip)
-   VPlaneMedian   |   v2.x   |   VPlaneMedian(clip)
-   YPlaneMinMaxDifference   |   v2.x   |   YPlaneMinMaxDifference(clip,
    float threshold)
-   UPlaneMinMaxDifference   |   v2.x   |   UPlaneMinMaxDifference(clip,
    float threshold)
-   VPlaneMinMaxDifference   |   v2.x   |   VPlaneMinMaxDifference(clip,
    float threshold)

This group of functions return statistics about the distribution of pixel
values on a plane (Luma, U-chroma and V-chroma, respectively). The statistics
are, in order of presentation: maximum, minimum, median and range (maximum -
minimum difference). ``threshold`` is a percentage, stating how many percent
of the pixels are allowed above or below minimum. The threshold is optional
and defaults to 0.

*Examples:*
::

    # median and average are close only on even distributions; this can be a useful diagnostic
    have_intense_brights = YPlaneMedian() - AverageLuma() < threshold
    ...
    # a simple per-frame normalizer to [16..235], CCIR, range
    Levels(YPlaneMin(), 1.0, YPlaneMax(), 16, 235)

--------

Back to :doc:`Internal functions <syntax_internal_functions>`.

$Date: 2008/04/20 19:07:34 $

.. _YV12: http://avisynth.org/mediawiki/YV12
.. _colorspace: http://avisynth.org/mediawiki/Color_spaces
.. _ISSE: http://avisynth.org/mediawiki/ISSE
