
DuplicateFrame
==============

``DuplicateFrame`` (clip, int frame [, ...])

``DuplicateFrame`` is the opposite of :doc:`DeleteFrame <deleteframe>`. It duplicates a set
of frames given as a number of arguments. As with ``DeleteFrame``, the sound
track is not modified.

**Examples:**

::

    DuplicateFrame(3, 3, 21, 42) # Add 4 frames

+------------+--------------------------------------+
| Changelog: |                                      |
+============+======================================+
| v2.58      | added support for multiple arguments |
+------------+--------------------------------------+

$Date: 2008/06/16 19:42:53 $
