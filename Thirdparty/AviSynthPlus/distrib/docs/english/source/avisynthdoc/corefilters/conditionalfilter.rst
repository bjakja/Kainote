
ConditionalFilter
=================

.. toctree::
    :maxdepth: 3

.. contents:: Table of contents



ConditionalFilter
-----------------

``ConditionalFilter`` (clip testclip, clip source1, clip source2, string
expression1, string operator, string expression2, bool "show")

``ConditionalFilter`` returns source1 when the condition formed by
"expression1+operator+expression2" is met for current frame, otherwise it
returns source2. If any function in expression1 or expression2 is not
explicitly applied to a clip, it will be applied on testclip. The audio is
taken from source1.

An example. This will choose frames from vid_blur when the average luma value
of a frame is less than 20. Otherwise frames from vid will be returned.
::

    vid = AviSource("file")
    vid_blur = vid.Blur(1.5)
    ConditionalFilter(vid, vid_blur, vid, "AverageLuma()", "lessthan", "20")

Adding show="true" will display the actual values on the screen.

The strings expression1 and expression2 can be any numeric or boolean
expressions, and may include internal or user functions, as well as some
additional functions which are predefined (:ref:`the Runtime Functions <conditional-runtime-functions>`) and the
special runtime variable current_frame (the framenumber of the requested
frame).

The string operator can be "equals", "greaterthan" or "lessthan". Or "=", ">"
or "<" respectively.


ConditionalSelect
-----------------

``ConditionalSelect`` (clip testclip, string expression, clip source0, clip
source1, clip source2, ... , bool "show")

``ConditionalSelect`` returns each one frame from several source clips based
on an integer evaluator. If the expression evaluates to the integer j
(starting at zero), the frame from the j-th source clip is returned.

This will return a frame from vid_blur2 when the average luma value of a
frame (of vid) is less than 15, will return a frame from vid_blur when the
average luma value of a frame (of vid) is higher than 15 but smaller than 25.
Otherwise a frame from vid will be returned.

::

    vid = AviSource("file")
    vid_blur = vid.Blur(1.0)
    vid_blur2 = vid.Blur(1.5)
    ConditionalSelect(vid, "luma_av = AverageLuma()"+chr(13)+"luma_av <
    25 ? (luma_av < 15 ? 2 : 1) : 0", vid, vid_blur, vid_blur2)

    # Adding show="true" will display the actual values on the screen.

If a frame is requested from a non-existing source clip (say the expression
evaluates to -1 or 3 in the example above, where 3 source clips are
supplied), the frame of the testclip will be returned.

Audio from "testclip" is passed through untouched.


.. _ScriptClip:

ScriptClip
----------

``ScriptClip`` (clip, string filter, bool "show", bool "after_frame")

``ScriptClip`` returns the clip returned by the filter evaluated on every
frame. The string filter can be any expression returning a clip, including
internal or user clip functions, and may include line breaks (allowing a
sequence of statements to be evaluated). Also, also some functions which are
predefined (:ref:`the Runtime Functions <conditional-runtime-functions>`) and the special runtime variable
current_frame (the framenumber of the requested frame) can be used in the
filter expression. Adding show="true" will display the actual values on the
screen.

Some examples:
::

    # This will print the difference from the previous frame onto the current one:
    clip = AviSource("c:\file.avi")
    ScriptClip(clip, "Subtitle(String(YDifferenceFromPrevious))")

    # This will apply blur on each frame based on the difference from the previous.
    # This will also show how errors are reported on some frames :)
    clip = AviSource("c:\file.avi")
    ScriptClip(clip, "Blur(YDifferenceFromPrevious/20.0)")

    # This will apply temporalsoften to very static scenes, and apply a _variable_ blur on moving scenes.
    # Blur is now capped properly. We also assign a variable - and this is why a line break is inserted:
    function fmin(float f1, float f2) {
      return (f1<f2) ? f1 : f2
    }
    clip = AviSource("c:\file.avi")
    T = clip.TemporalSoften(2, 7, 7, 3, 2)
    ScriptClip(clip, "diff = YDifferenceToNext()"+chr(13)+"diff>2.5 ?
    Blur(fmin(diff/20, 1.5)) : T")

    # Shows the frame-number in a clip:
    ScriptClip("subtitle(string(current_frame))")

    # Shows 'frame = the frame-number' in a clip:
    ScriptClip("""subtitle("frame = " + string(current_frame))""")

In v2.55 an *after_frame=true/false* option to is added. This determines if
the script should be evaluated before (default operation) or after the frame
has been fetched from the filters above.

"Restrictions": the output of the script MUST be exactly like the clip
delivered to ``ScriptClip`` (same colorspace, width and height). Your
returned clip is allowed to have different length - but the length from
"clip" is always used. Audio from "clip" is passed through untouched. For two
very different sources (MPEG2DEC3 and AviSource) - you might run into
colorspace mismatches. This is known quirk.

FrameEvaluate
-------------

``FrameEvaluate`` (clip clip, script filter, bool "after_frame")

Similar to ``ScriptClip``, except the output of the filter is ignored. This
can be used for assigning variables, etc. Frames are passed directly through
from the supplied clip.

In v2.53 an ``after_frame=true/false`` option to is added. This determines if
the script should be evaluated before (default operation) or after the frame
has been fetched from the filters above.

ConditionalReader
-----------------

This filter allows you to import arbitrary information into a selectable
variable.

See the dedicated :doc:`ConditionalReader <conditionalreader>` page.


.. _conditional-runtime-functions:

Runtime Functions
-----------------

These are the internal functions which are evaluated every frame.

| These will return the average pixel value of a plane (require YV12, ISSE):
| ``AverageLuma`` (clip)
| ``AverageChromaU`` (clip)
| ``AverageChromaV`` (clip)

| These return a float value between 0 and 255 of the absolute difference
  between two planes (require YV12, ISSE):
| ``RGBDifference`` (clip1, clip2)
| ``LumaDifference`` (clip1, clip2)
| ``ChromaUDifference`` (clip1, clip2)
| ``ChromaVDifference`` (clip1, clip2)

When using these functions there is an "implicit last" clip (first parameter
doesn't have to be specified), so the first parameter is replaced by the
testclip.

| These should be quite handy for detecting scene change transitions:
| ``RGBDifferenceFromPrevious`` (clip)
| ``YDifferenceFromPrevious`` (clip)
| ``UDifferenceFromPrevious`` (clip)
| ``VDifferenceFromPrevious`` (clip)
| ``RGBDifferenceToNext`` (clip)
| ``YDifferenceToNext`` (clip)
| ``UDifferenceToNext`` (clip)
| ``VDifferenceToNext`` (clip)

::

    # This will replace the last frame before a scenechange
    # with the first frame after the scenechange:
    ConditionalFilter(last, last, last.trim(1,0), "YDifferenceToNext()", ">", "10", true)

Other internal functions
~~~~~~~~~~~~~~~~~~~~~~~~

| ``YPlaneMax`` (clip, float threshold)
| ``UPlaneMax`` (clip, float threshold)
| ``VPlaneMax`` (clip, float threshold)
| ``YPlaneMin`` (clip, float threshold)
| ``UPlaneMin`` (clip, float threshold)
| ``VPlaneMin`` (clip, float threshold)
| ``YPlaneMedian`` (clip)
| ``UPlaneMedian`` (clip)
| ``VPlaneMedian`` (clip)
| ``YPlaneMinMaxDifference`` (clip, float threshold)
| ``UPlaneMinMaxDifference`` (clip, float threshold)
| ``VPlaneMinMaxDifference`` (clip, float threshold)

Threshold is a percentage, on how many percent of the pixels are allowed
above or below minimum. The threshold is optional and defaults to 0.

If you understand the stuff above, you can proceed with "advanced conditional
filtering", which tells you a little bit more about conditional filtering.

Advanced conditional filtering: part I
--------------------------------------

You will have to know a few things about the functionality of AviSynth to
understand this section:
Scripts are parsed from top to bottom, but when a frame is requested the last
filter is actually being invoked first, requesting frames upwards in the
filter chain. For example:

::

    AviSource("myfile.avi")
    ColorYUV(analyze=true)
    Histogram() When opening the script in Vdub the following happens

-   When Vdub requests a frame, AviSynth requests the frame from
    Histogram.
-   Histogram requests a frame from ColorYUV,
-   ColorYUV requests a frame from AviSource, which produces the frame,
    and delivers it to ColorYUV.
-   ColorYUV processes the image and sends it on to Histogram, which
    returns it to Vdub.

So the filter chain basically works backwards (the output is 'pulled' from
below rather than 'pushed' from above), which gives each filter the
possibility to request several frames from the source above. Conditional
filters however, need to evaluate scripts before they request frames from the
filter above, because they need to know which filter to call. Another
important issue is that run-time scripts are evaluated in the same context as
the main script. Hence only global defined variables in the conditional
filter 'environment' can be used inside a function (and vice versa). Have a
look at the following script:

::

    v = AviSource("E:\Temp\Test3\atomic_kitten.avi").ConvertToYV12

    function g(clip c)
    {
      global w = c
      c2 = ScriptClip(c, "subtitle(t)")
      c3 = FrameEvaluate(c2, "t = String(text)")
      c4 = FrameEvaluate(c3, "text = YDifferenceFromPrevious(w)")
      return c4
    }

    g(v)

This filter chain works like this:

-   When Vdub requests a frame, AviSynth requests a frame from the second
    FrameEvaluate, the last filter in the chain generated by g().
-   The second FrameEvaluate evaluates YDifferenceFromPrevious(w), which
    leads to the following actions:

    -   YDifferenceFromPrevious requests a frame from ConvertToYV12;
    -   ConvertToYV12 requests a frame from AviSource, which produces the
        frame, and delivers it to ConvertToYV12;
    -   ConvertToYV12 processes the image and returns it to
        YDifferenceFromPrevious;
    -   YDifferenceFromPrevious requests a second frame from
        ConvertToYV12, which is obtained in a similar way to the first;
    -   It then compares the two frames to calculate its result which it
        delivers to FrameEvaluate.

-   FrameEvaluate assigns this value to the variable text.
-   After this a frame is requested from the first FrameEvaluate.
-   The first FrameEvaluate, after evaluating String(text) and assigning
    this value to the variable *t*, requests a frame from ScriptClip.
-   ScriptClip sets *last* to the result of ConvertToYV12(), evaluates
    Subtitle(t) (creating a new, temporary, filter chain), and requests a
    frame from it.

    -   Subtitle requests a frame from ConvertToYV12;
    -   ConvertToYV12 requests a frame from AviSource, which produces the
        frame, and delivers it to ConvertToYV12;
    -   ConvertToYV12 processes the image and returns it to Subtitle;
    -   Subtitle adds the specified text to the frame and delivers the
        result to ScriptClip.

-   ScriptClip returns the subtitled frame to the first FrameEvaluate.
-   In turn this frame is returned to the second FrameEvaluate, and hence
    to Avisynth which returns it to VDub.

Notice how the addition of run-time filters and run-time functions makes the
interactions between different parts of the filter chain more complex. This
added complexity is managed internally by Avisynth, so you needn't worry
about it. However, care is required when setting and using variables, as the
order of events can be less obvious to the script writer (you!).

As can be seen, *w* is defined as a global variable. This way we can use it
later in the script in the conditional environment. If we want to use the
variables *t* and *text* in a different function (inside or outside the
conditional environment), they must also be defined as global variables. Thus
for example:

::

    v = AviSource("E:\Temp\Test3\atomic_kitten.avi").ConvertToYV12

    function g(clip c)
    {
      global w = c
      c2 = ScriptClip(c, "subtitle(t)")
      c3 = FrameEvaluate(c2, "me()")
      c4 = FrameEvaluate(c3, "global text = YDifferenceFromPrevious(w)")
      return c4
    }

    function me()
    {
      global t = String(text)
    }

g(v) This is just an illustration to demonstrate the various
features. Much of the script above is redundant, and can be removed. The
following two scripts give the same output

::

    v = AviSource("c:\clip.avi")
    # ScriptClip accepts multi-line scripts:
    Scriptclip(v,"
        text = YDifferenceFromPrevious()
        t = string(text)
        subtitle(t)
    ")

    v = AviSource("c:\clip.avi")
    ScriptClip(v, "Subtitle(String(YDifferenceFromPrevious))")

In the following section some frame dependent info will be written to a text-file.

Advanced conditional filtering: part II
---------------------------------------

In the following example, some frame dependent info will be written to a
text-file. The first variable "a" indicates whether the frame is combed (for
a certain threshold). Note that IsCombed is a filter from the Decomb plugin.
The second variable "b" indicates whether there is "much" movement in the
frame.

::

    global sep="."
    global combedthreshold=25

    function IsMoving()
    {
    global b = (diff < 1.0) ? false : true
    }

    function CombingInfo(clip c)
    {
    file = "F:\interlace.log"
    global clip = c
    c = WriteFile(c, file, "a", "sep", "b")
    c = FrameEvaluate(c, "global a = IsCombed(clip, combedthreshold)")
    c = FrameEvaluate(c, "IsMoving")
    c = FrameEvaluate(c, "global diff =
    0.50*YDifferenceFromPrevious(clip) + 0.25*UDifferenceFromPrevious(clip) +
    0.25*VDifferenceFromPrevious(clip)")
    return c
    }

    v = mpeg2source("F:\From_hell\from_hell.d2v").trim(100,124)
    CombingInfo(v)

We can tidy up the two functions, and remove global variables, by writing
them as follows:

::

    function IsMoving(float diff)
    {
     return (diff >= 1.0)
    }

    function CombingInfo(clip c)
    {
     file = "F:\interlace.log"

     c = WriteFile(c, file, "a", "sep", "b")
     c = FrameEvaluate(c,"
           diff = 0.50*YDifferenceFromPrevious() +
           0.25*UDifferenceFromPrevious() + 0.25*VDifferenceFromPrevious()
           b = IsMoving(diff)
           a = IsCombed(combedthreshold)
         ")

     return c
    }

In the following section an example of "adaptive motion/resizing filter" will
be considered.


Advanced conditional filtering: part III
----------------------------------------

Some adaptive motion/resizing filters appeared on the forums. These filters
discriminate between low, medium and high motion in a clip (on frame basis).
By doing that, different filters can be used for different kind of motion in
the clip. In general, one should use temporal smoothing in low motion scenes,
spatial smoothing in high motion scenes and use spatio-temporal smoothing in
medium motion scenes.

Below, a simplified version of QUANTIFIED MOTION FILTER v1.5 b1 (10/07/2003)
by HomiE FR, is given:

::

    ----------------------------------------------------
    # QUANTIFIED MOTION FILTER v1.3
    # LOADING AVISYNTH PLUGINS
    LoadPlugin("C:\PROGRA~1\GORDIA~1\mpeg2dec3.dll")
    LoadPlugin("C:\PROGRA~1\GORDIA~1\TemporalCleaner.dll")
    LoadPlugin("C:\PROGRA~1\GORDIA~1\FluxSmooth.dll")
    LoadPlugin("C:\PROGRA~1\GORDIA~1\UnFilter.dll")

    # LOADING QUANTIFIED MOTION FILTER SCRIPT

    Import("E:\temp\QMF\qmf.avs")

    # LOW MOTION FILTER FUNCTION
    # -> SHARP RESIZING + TEMPORAL ONLY
    function Low_Motion_Filter(clip c)
    {
      c = TemporalCleaner(c, 5, 10)
      c = LanczosResize(c, 512, 272)
      return c
    }

    # MEDIUM MOTION FILTER FUNCTION
    # -> NEUTRAL BICUBIC RESIZING + TEMPORAL & SPATIAL
    function Medium_Motion_Filter(clip c)
    {
      c = FluxSmooth(c, 7, 7)
      c = BicubicResize(c, 512, 272, 0.00, 0.50)
      return c
    }

    # HIGH MOTION FILTER FUNCTION
    # -> SOFT RESIZING + SPATIAL ONLY
    function High_Motion_Filter(clip c)
    {
      c = FluxSmooth(c, -1, 14)
      c = UnFilter(c, -30, -30)
      c = BilinearResize(c, 512, 272)
      return c
    }

    # OPENING VIDEO SOURCE
    AviSource("E:\temp\QMF\britney-I_love_rock_'n_roll.avi")
    ConvertToYV12(interlaced=true)
    Telecide(0)

    # APPLYING ADAPTATIVE RESIZING FILTER (USING QMF)
    QMF()
    ----------------------------------------------------

    # QUANTIFIED MOTION FILTER (17/08/2003) by HomiE FR
    (homie.fr@wanadoo.fr)
    # MOTION ESTIMATION FUNCTION
    function ME()
    {
      # SETTING MOTION LEVEL ACCORDING TO AVERAGE DIFFERENCE [1]
      **global motion_level** = (**diff** < threshold_lm) ? 0 :
      motion_level
      **global motion_level** = (**diff** >= threshold_lm && **diff**
      <= threshold_hm) ? 1 : motion_level
      **global motion_level** = (**diff** > threshold_hm) ? 2 :
      motion_level
    }

    # QUANTIFIED MOTION FILTER FUNCTION
    function QMF(clip c, float "threshold_lm", float "threshold_hm", bool
    "debug")
    {
      # SETTING MOTION LEVELS THRESHOLDS [2]
      threshold_lm = default(threshold_lm, 4.0)
      threshold_hm = default(threshold_hm, 12.0)
      global threshold_lm = threshold_lm
      global threshold_hm = threshold_hm

      # ENABLING/DISABLING DEBUG INFORMATION [3]
      debug = default(debug, false)

      # INITIALIZING MOTION LEVEL
      global motion_level = 0

      # SETTING PRESENT CLIP [4]
      global clip = c

      # GETTING OUTPUT RESOLUTION [5]
      width = Width(Low_Motion_Filter(c))
      height = Height(Low_Motion_Filter(c))
      global c_resized = PointResize(c, width, height)

      # APPLYING MOTION FILTER ACCORDING TO MOTION LEVEL [6]
      c = ConditionalFilter(c, Low_Motion_Filter(c), c_resized,
      "**motion_level**", "=", "0")  # [6a]
      c = ConditionalFilter(c, Medium_Motion_Filter(c), c,
      "**motion_level**", "=", "1")       # [6b]
      c = ConditionalFilter(c, High_Motion_Filter(c), c,
      "**motion_level**", "=", "2")         # [6c]

      # PRINTING DEBUG INFORMATION [7]
      c = (debug == true) ? ScriptClip(c, "Debug()") : c

      # GETTING MOTION LEVEL THROUGH MOTION ESTIMATION [8]
      c = FrameEvaluate(c, "ME()")

      # GETTING DIFFERENCES BETWEEN PAST/PRESENT FRAMES [9]
      c = FrameEvaluate(c, "**global diff** =
      0.50*YDifferenceFromPrevious(clip) + 0.25*UDifferenceFromPrevious(clip)
      + 0.25*VDifferenceFromPrevious(clip)")
      return c
    }

    # DEBUG INFORMATION FUNCTION
    function Debug(clip c)
    {
      # PRINTING VERSION INFORMATION [10]
      c = Subtitle(c, "Quantified Motion Filter", x=20, y=30,
      font="lucida console", size=18, text_color=$FFFFFF)
      c = Subtitle(c, "by HomiE FR (homie.fr@wanadoo.fr)", x=20, y=45,
      font="lucida console", size=14, text_color=$FFFFFF)

      # PRINTING MOTION ESTIMATION INFORMATION [11]
      c = Subtitle(c, "motion estimation", x=20, y=85, font="lucida
      console", size=18, text_color=$FFFFFF)
      c = Subtitle(c, "diff = "+string(**diff**), x=20,y=110,
      font="lucida console", size=16, text_color=$FFCCCC)

      # PRINTING QUANTIFIED MOTION FILTER INFORMATION [12]
      c = Subtitle(c, "quantified motion filter", x=20, y=135,
      font="lucida console", size=18, text_color=$FFFFFF)
      c = (**motion_level** == 0) ? Subtitle(c, "scene type = low
      motion", x=20, y=160, font="lucida console", size=16,
      text_color=$66FF66) : c
      c = (**motion_level** == 1) ? Subtitle(c, "scene type = medium
      motion", x=20, y=160, font="lucida console", size=16,
      text_color=$66FF66) : c
      c = (**motion_level** == 2) ? Subtitle(c, "scene type = high
      motion", x=20, y=160, font="lucida console", size=16,
      text_color=$66FF66) : c
      return c
    }
    ----------------------------------------------------

This filter chain works like this:

-   When Vdub requests a frame, AviSynth requests a frame from QMF.

-   QMF request a frame from FrameEvaluate [9].
-   After doing this the script [9] is evaluated, and the global variable
    *diff* is assigned after requesting a frame from AviSource. FrameEvaluate
    [9] requests a frame from FrameEvaluate [8].
-   Once again the script [8] is evaluated:

-   when evaluating me(), the global variable *motion_level* is assigned
    for that frame [1]

-   If debug=true, a frame is requested from ScriptClip [7], and thus
    from Debug().
-   After that (and also when debug was set to false) a frame is
    requested from the last ConditionalFilter [6c], which requests a frame
    from [6b], which in turn requests a frame from [6a].

-   Note that in the end, a frame of High_Motion_filter,
    Medium_Motion_filter, or Low_Motion_filter is requested depending on the
    value of *motion_level*.

-   QMF request a frame from Telecide, Telecide from ConvertToYV12 and
    finally ConvertToYV12 from AviSource.
-   AviSource produces the frame and sends it to ConvertToYV12, etc.

A few details were omitted, but this is how the script basically works.

$Date: 2012/04/09 08:19:32 $
