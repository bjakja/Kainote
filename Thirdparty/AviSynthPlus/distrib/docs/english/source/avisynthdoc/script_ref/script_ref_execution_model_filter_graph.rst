
The script execution model - The filter graph
=============================================

The final purpose of every AviSynth script is to create a filter graph that
can be used by the top-level AVI stream object (see previous section) to call
the filters needed for the creation of each specific frame that is requested
by the host video application.

This filter graph is **implicit** in the sense that it is not directly
available to script writers for querying or modification. However it is
created and built-up on each filter call that the script source code makes,
using the following rules:

-   Each filter -actually its resulting clip- links to *all* its source
    clips (those that it uses directly or indirectly for input).
-   Source filters (such as :doc:`AviSource <../corefilters/avisource>`) do not link to other clips.
    Their resulting clip is a leaf-node in the filter graph.
-   Only filters -ie their resulting clips- that are linked by the final
    clip returned by the script are part of the filter graph. That is a clip
    is part of the filter graph only if there is a path that connects it with
    the final clip returned by the script.


An example of a filter graph
----------------------------

Consider the following script, which loads a clip, makes a simple levels
processing and muxes it with a custom sound track; then it overlays on top of
it another clip, along with its inverted image, as separate windows.

::

    AviSource("clip1.avi")
    Levels(10, 1, 248, 0, 255)
    aud = WavSource("mysoundtrack.wav")
    AudioDub(last, aud)
    ov = AviSource("clip2.avi")
    ov1 = Lanczos4Resize(ov, 280, 210)
    ov2 = ov1.Invert()
    w = last.Width
    h = last.Height
    Overlay(ov1, x=w-280-40, y=40) # first clip is last
    Overlay(ov2, x=w-280-40, y=h-210-40)

The filter graph that results from the script code above is the following.

::

    AviSource(clip1) <-- Levels <--+
                                   |
    WavSource <--------------------+-- AudioDub <--+
                         |
    AviSource(clip2) <-- Lanczos4Resize <--+-------+-- Overlay <--+
         |                      |
         +-- Invert
                                           <-----------+-- Overlay (filter graph's root)

The root of the filter graph, that is the filter from which the host video
application requests video frames is the second :doc:`Overlay <../corefilters/overlay>` (the last line of
the script).

--------

Back to the :doc:`script execution model <script_ref_execution_model>`.

$Date: 2010/11/28 18:47:26 $
