
Subtract
========

``Subtract`` (clip1, clip2)

``Subtract`` produces an output clip in which every pixel is set according to
the difference between the corresponding pixels in *clip1* and *clip2*. More
specifically, it sets each pixel to (50% gray) + (*clip1* pixel) - (*clip2*
pixel). You can use :doc:`Levels <levels>` afterwards if you want to increase the
contrast.

Also see :ref:`here <multiclip>` for the resulting clip properties.

**Examples:**

::

    # Make the differences between clip1 and clip2 blatantly obvious
    Subtract(clip1, clip2).Levels(127, 1, 129, 0, 255)

If you want to see the deltas between adjacent frames in a single clip, you
can do it like this:

::

    Subtract(clip.Trim(1,0), clip)

**About offset of luma range:**

For YUV formats the valid Y range is from 16 to 235 inclusive and subtract
takes this into account. This means that the following script

::

    Subtract(any_clip, any_clip)

will result in a grey clip with luma = 126. For those that require a subtract
function for pc_range YUV data use :doc:`Overlay <overlay>`:

::

    #Overlay(any_clip, any_clip, mode="Difference", pc_range=true)
    # grey clip with luma = 128
    Overlay(clip1, clip2, mode="Difference", pc_range=true)

$Date: 2006/09/27 18:41:25 $
