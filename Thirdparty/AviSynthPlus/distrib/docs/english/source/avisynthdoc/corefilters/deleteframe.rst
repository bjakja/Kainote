
DeleteFrame
===========

``DeleteFrame`` (clip, int frame [, ...])

``DeleteFrame`` deletes a set of frames, given as a number of arguments. The
sound track is not modified, so if you use this filter to delete many frames
you may get noticeable desynchronization.

**Examples:**

::

    DeleteFrame(3, 9, 21, 42) # delete frames 3, 9, 21 and 42

If you want to delete a range of frames (*a* to *b*, say) along with the
corresponding portion of the soundtrack, you can do it with :doc:`Trim <trim>` like
this:

::

    Trim(0,a-1) ++ Trim(b+1,0)

Or like this:

::

    Loop(0,a,b)

+-----------+--------------------------------------+
| Changelog |                                      |
+===========+======================================+
| v2.58     | added support for multiple arguments |
+-----------+--------------------------------------+

$Date: 2008/06/16 19:42:53 $
