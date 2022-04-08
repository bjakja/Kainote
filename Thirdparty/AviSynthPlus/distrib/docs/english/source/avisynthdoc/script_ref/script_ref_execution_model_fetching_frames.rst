
The script execution model - The fetching of frames
===================================================

Despite that the script writer writes the script from top to bottom, AviSynth
requests frames from the filters included in the script in the reverse order
(**from bottom to top**). This is a consequence of the fact that AviSynth
fakes the existence of an AVI file to the host video application through its
AVI handler code.

The host video application does not know that AviSynth is behind the scenes;
it just requests a finished and ready to render video frame, that is the
final result of the script's processing from the AVI handler. The AVI handler
in turn requests the frame from the root of the filter graph, ie from the
final clip returned by the script.

In order to create the frame the final clip requests video frame(s) from its
input clip(s). This process continues until a source filter, such as
:doc:`AviSource <../corefilters/avisource>`, is reached which directly produces a video frame without
requesting input from other filters. Therefore, the request process traverses
the filter chain from bottom to top. That frame is then subsequently
processed by all filters in the request chain (now from top to bottom, as in
the script source code), resulting in the finished video frame that is served
to the host video application.

This backward searching implementation has been chosen for effectiveness
reasons and it doesn't need to bother you except for one important case: when
you use runtime filters (cf. next sections).

--------

Back to the :doc:`script execution model <script_ref_execution_model>`.

$Date: 2008/04/20 19:07:33 $
