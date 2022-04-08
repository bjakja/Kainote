
Filters with multiple input clips
=================================

There are some functions which combine two or more clips in different ways.
How the video content is calculated is described for each function, but here
is a summary which properties the result clip will have.

The input clips must always have the same color format and - with the
exception of :doc:`Layer <corefilters/layer>` and :doc:`Overlay <corefilters/overlay>` - the same dimensions.

+------------------------------------------------------------+----------------------+----------------------+------------------------+---------------------+
| filter                                                     | framerate            | framecount           | audio content          | audio sampling rate |
+============================================================+======================+======================+========================+=====================+
| :doc:`AlignedSplice, UnalignedSplice <corefilters/splice>` | first clip           | sum of all clips     | see filter description | first clip          |
+------------------------------------------------------------+                      +----------------------+                        |                     |
| :doc:`Dissolve <corefilters/dissolve>`                     |                      | sum of all clips     |                        |                     |
|                                                            |                      | minus the overlap    |                        |                     |
+------------------------------------------------------------+                      +----------------------+------------------------+                     |
| :doc:`Merge, MergeLuma, MergeChroma <corefilters/merge>`,  |                      | first clip `1`:sup:  | first clip             |                     |
| :doc:`Merge(A)RGB <corefilters/mergergb>`                  |                      |                      |                        |                     |
+------------------------------------------------------------+                      |                      |                        |                     |
| :doc:`Layer <corefilters/layer>`                           |                      |                      |                        |                     |
+------------------------------------------------------------+                      +----------------------+                        |                     |
| :doc:`Subtract <corefilters/subtract>`                     |                      | longer clip `2`:sup: |                        |                     |
+------------------------------------------------------------+                      |                      |                        |                     |
| :doc:`StackHorizontal, StackVertical <corefilters/stack>`  |                      |                      |                        |                     |
+------------------------------------------------------------+----------------------+----------------------+                        |                     |
| :doc:`Interleave <corefilters/interleave>`                 || (fps of first clip) || N x frame-count     |                        |                     |
|                                                            || x                   || of longer clip      |                        |                     |
|                                                            || (number of clips)   || `2`:sup:            |                        |                     |
+------------------------------------------------------------+----------------------+----------------------+------------------------+---------------------+

| `1`:sup: (the last frame of the shorter clip is repeated until the end of the clip)
| `2`:sup: (the last frame of the shorter clip is repeated until the end of the clip)

As you can see the functions are not completely symmetric but take some attributes from the FIRST clip.

$Date: 2008/07/19 15:17:14 $
