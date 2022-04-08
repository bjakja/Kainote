
Interlaced and Field-based video
================================

Currently (v2.5x and older versions), AviSynth has no interlaced flag which
can be used for interlaced video. There is a field-based flag, but contrary
to what you might expect, this flag is not related to interlaced video. In
fact, all video (progressive or interlaced) is frame-based, unless you use
AviSynth filters to change that. There are two filter who turn frame-based
video into field-based video: :doc:`SeparateFields <../corefilters/separatefields>` and :doc:`AssumeFieldBased <../corefilters/parity>`.

-   SeparateFields / Weave

    -   SeparateFields: If needed (for example when denoisers require
        progressive input), you can split up interlaced streams in their fields
        by using SeparateFields. The output clip will have twice the framerate of
        the original clip.
    -   Weave: After applying a spatial denoiser (for temporal denoisers
        the situation is a bit more [`involved`_]), the filter Weave can be used
        to combine the fields again to produce interlaced frames.

-   AssumeFieldBased / AssumeFrameBased

    -   AssumeFieldBased: Applying this filter results in a field-based
        clip. This can be useful when making "artificial" field-based clips.
    -   AssumeFrameBased: Applying this filter results in a frame-based
        clip. This can be useful when making "artificial" frame-based clips.


Color conversions and interlaced / field-based video
----------------------------------------------------

Let's assume you have **interlaced video**, you want to work in field-based
mode (to apply some filtering for example) and you also need to do some color
conversion. Do you need to do the conversion on the frame-based clip or can
you do it on the field-based clip? Well, that depends on the
:doc:`color conversion <../corefilters/convert>` you want to apply:

* YUY2<->RGB conversions can be done on either of them. (Note, that in this
  case, the setting interlaced=true/false doesn't do anything. It's simply
  ignored.)
* YV12<->YUY2/RGB conversions should be done on the frame-based clip (with
  the **interlaced=true** setting). Doing them on the field-based clip will
  yield incorrect results. The exact reason of this is outside the scope of
  this page, but it is a consequence of how the color format YV12 is defined.
  The main issue is that chroma is shared between pixels on two different lines
  in a frame. More information can be found here :doc:`Sampling <sampling>`.

The more experienced users should consider the following. In general,
interlaced video has parts where there is no or little movement. Thus, you
won't hardly see any interlacing effects (also called combing) in these
parts. They can be considered progressive, and when doing a YV12<->YUY2/RGB
conversion on a progressive video you should use the **interlaced=false**
setting to get better results. It is possible to the YV12<->YUY2/RGB
conversion on frame basis while switching between interlaced=true and
interlaced=false. Here's how to do it (you will need to have decomb installed
in order to be able to use the function IsCombed)

::

    function ConvertHybridToYUY2(clip a, int "threshold", bool "debug")
    {
    debug = default(debug, false)
    global threshold = default(threshold, 20)

    b = ConvertToYUY2(a, interlaced=false)
    c = ConvertToYUY2(a, interlaced=true)
    ConditionalFilter(a, b, c, "IsCombed(threshold)", "equals", "true",
    show=debug)
    }

    function ConvertHybridToRGB(clip a, int "threshold", bool "debug")
    {
    debug = default(debug, false)
    global threshold = default(threshold, 20)

    b = ConvertToYUY2(a, interlaced=false)
    c = ConvertToYUY2(a, interlaced=true)
    ConditionalFilter(a, b, c, "IsCombed(threshold)", "equals", "true",
    show=debug)
    }

    AviSource("D:\captures\interlaced-clip.avi") # interlaced YV12
    #ConvertHybridToYUY2(debug=true)
    ConvertHybridToYUY2()

However, the downside of this is that it may lead to [`chroma shimmering`_]
in the combed-progressive frame transitions. So, it's not a perfect solution.


Color conversions, interlaced / field-based video and the interlaced flag of dvd2avi
------------------------------------------------------------------------------------

For the more experienced users. Dvd2avi keeps track of whether a frame is
interlaced or progressive (by using the interlaced flag). In principle,
dvd2avi can be modified to store this in a text-file and AviSynth can read
and use it on frame-basis. However, it's useless. The problem is that
sometimes progressive video is encoded as interlaced, and thus is detected as
interlaced by dvd2avi. In the previous section, it is explained, that in that
case you should use interlaced=false during the YV12<->YUY2/RGB conversion
(since there's no movement) to get more accurate results. **So, it's the
presence of combing which is important for the YV12<->YUY2/RGB conversion,
and not whether a frame is interlaced.**


Changing the order of the fields of a clip
------------------------------------------

There is a filter which swaps the even/odd fields :doc:`SwapFields <../corefilters/swapfields>`, and a plugin
which reverses the field dominance [`ReverseFieldDominance`_]. The former
changes the spatial order and the latter the temporal order.


Swapping fields
^^^^^^^^^^^^^^^

before using SwapFields:

+------+---------+
| line | frame 0 |
+======+=========+
| 0    | t0      |
+------+---------+
| 1    | b1      |
+------+---------+
| 2    | t2      |
+------+---------+
| 3    | b3      |
+------+---------+
| 4    | t4      |
+------+---------+
| 5    | b5      |
+------+---------+

field order (top field first then bottom field):

+------+---------+---------+
| line | field 0 | field 1 |
+======+=========+=========+
| 0    | t0      |         |
+------+---------+---------+
| 1    |         | b1      |
+------+---------+---------+
| 2    | t2      |         |
+------+---------+---------+
| 3    |         | b3      |
+------+---------+---------+
| 4    | t4      |         |
+------+---------+---------+
| 5    |         | b5      |
+------+---------+---------+

after using SwapFields:

+------+---------+
| line | frame 0 |
+======+=========+
| 0    | b1      |
+------+---------+
| 1    | t0      |
+------+---------+
| 2    | b3      |
+------+---------+
| 3    | t2      |
+------+---------+
| 4    | b5      |
+------+---------+
| 5    | t4      |
+------+---------+

field order (top field first then bottom field):

+------+---------+---------+
| line | field 0 | field 1 |
+======+=========+=========+
| 0    |         | b1      |
+------+---------+---------+
| 1    | t0      |         |
+------+---------+---------+
| 2    |         | b3      |
+------+---------+---------+
| 3    | t2      |         |
+------+---------+---------+
| 4    |         | b5      |
+------+---------+---------+
| 5    | t4      |         |
+------+---------+---------+

Note that the even and odd lines are swapped, so you can call the Top Field
as Bottom Field, and vice versa.


Reversing field dominance
^^^^^^^^^^^^^^^^^^^^^^^^^

before reversing the field dominance:

+------+---------+
| line | frame 0 |
+======+=========+
| 0    | t0      |
+------+---------+
| 1    | b1      |
+------+---------+
| 2    | t2      |
+------+---------+
| 3    | b3      |
+------+---------+
| 4    | t4      |
+------+---------+
| 5    | b5      |
+------+---------+

field order (top field first then bottom field):

+------+---------+---------+
| line | field 0 | field 1 |
+======+=========+=========+
| 0    | t0      |         |
+------+---------+---------+
| 1    |         | b1      |
+------+---------+---------+
| 2    | t2      |         |
+------+---------+---------+
| 3    |         | b3      |
+------+---------+---------+
| 4    | t4      |         |
+------+---------+---------+
| 5    |         | b5      |
+------+---------+---------+

after reversing the field dominance (assuming the lines will be shifted up,
and the last one will be duplicated):

+------+---------+
| line | frame 0 |
+======+=========+
| 0    | b1      |
+------+---------+
| 1    | t2      |
+------+---------+
| 2    | b3      |
+------+---------+
| 3    | t4      |
+------+---------+
| 4    | b5      |
+------+---------+
| 5    | b5      |
+------+---------+

field order (bottom field first then top field):

+------+---------+---------+
| line | field 0 | field 1 |
+======+=========+=========+
| 0    | b1      |         |
+------+---------+---------+
| 1    |         | t2      |
+------+---------+---------+
| 2    | b3      |         |
+------+---------+---------+
| 3    |         | t4      |
+------+---------+---------+
| 4    | b5      |         |
+------+---------+---------+
| 5    |         | b5      |
+------+---------+---------+

Note that the top and bottom fields are swapped, but the even and odd lines
are not swapped.


The parity (= order) of the fields in AviSynth
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

If a clip is field-based AviSynth keeps track of the parity of each field
(that is, whether it's the top or the bottom field of a frame). If the clip
is frame-based it keeps track of the dominant field in each frame (that is,
which field in the frame comes first when they're separated).

However, this information isn't necessarily correct, because field
information usually isn't stored in video files and AviSynth's source filters
just normally default to assuming bottom field first (with the exception of
the MPEG2Source plugin which gets it right!).


About DV / DVD in relation to field dominance
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The field dominance is not the same for every source. DV (with interlaced
content) has bottom field first, while DVD (or CVD/SVCD) has top field first.
Thus when convert between those two, you need to change the field dominance.
This can be done in AviSynth (see above), but also in the encoder itself (for
bff material like DV footage, you need to set the Upper field first flag).
Some comments on other [`methods`_].


References
^^^^^^^^^^

| [`DV / DVD and field dominance`_]
| About [`field dominance`_].
| [`Doom thread`_] about swapped fields and field dominance.
| [`ReverseFieldDominance`_ plugin]

$Date: 2006/12/15 19:29:25 $

.. _involved:
    http://www.doom9.org/index.html?/capture/postprocessing_avisynth.html
.. _chroma shimmering:
    http://forum.doom9.org/showthread.php?s=&postid=476199#post476199
.. _ReverseFieldDominance:
    http://www.geocities.com/siwalters_uk/reversefielddominance.html
.. _methods:
    http://forum.doom9.org/showthread.php?s=&postid=410692#post410692
.. _DV / DVD and field dominance:
    http://forum.doom9.org/showthread.php?s=&threadid=47393
.. _field dominance: http://www.lurkertech.com/lg/dominance.html
.. _Doom thread: http://forum.doom9.org/showthread.php?s=&postid=268353
