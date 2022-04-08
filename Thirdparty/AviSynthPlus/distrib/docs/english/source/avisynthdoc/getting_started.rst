
Getting started
===============

Basically, AviSynth works like this: First, you create a simple text document
with special commands, called a script. These commands make references to one
or more videos and the filters you wish to run on them. Then, you run a video
application, such as VirtualDub, and open the script file. This is when
AviSynth takes action. It opens the videos you referenced in the script, runs
the specified filters, and feeds the output to video application. The
application, however, is not aware that AviSynth is working in the
background. Instead, the application thinks that it is directly opening a
filtered AVI file that resides on your hard drive.

Linear Editing
--------------

The simplest thing you can do with AviSynth is the sort of editing you can do
in VirtualDub. The scripts for this are easy to write because you don't have
to worry about variables and complicated expressions if you don't want.

For testing create a file called test.avs and put the following single line
of text in it:
::

    Version

Now open this file with e.g. Windows Media Player and you should see a ten-
second video clip showing AviSynth's version number and a copyright notice.

``Version`` is what's called a "source filter", meaning that it generates a
clip instead of modifying one. The first command in an AviSynth script will
always be a source filter.

Now add a second line to the script file, so that it reads like this:
::

    Version
    ReduceBy2

Reopen the file in Media Player. You should see the copyright notice again,
but now half as large as before.
:doc:`ReduceBy2 <corefilters/reduceby2>` is a "transformation filter," meaning that it takes the
previous clip and modifies it in some way. You can chain together lots of
transformation filters, just as in VirtualDub.
Let's add another one to make the video fade to black at the end. Add another
line to the script file so that it reads:
::

    Version
    ReduceBy2
    FadeOut(10)

Now reopen the file. The clip should be the same for the first 9 seconds, and
then in the last second it should fade smoothly to black.
The :doc:`FadeOut <corefilters/fade>` filter takes a numerical argument, which indicates the number
of frames to fade.

It takes a long time before the fade starts, so let's trim the beginning of
the clip to reduce the wait, and fade out after that.
Let's discard the first 120 of them, and keep the frames 120-150:
::

    Version
    ReduceBy2
    # Chop off the first 119 frames, and keep the frames 120-150
    # (AviSynth starts numbering frames from 0)
    Trim(120,150)
    FadeOut(10)

In this example we used a comment for the first time.
Comments start with the # character and continue to the end of the line, and
are ignored completely by AviSynth.
The :doc:`Trim <corefilters/trim>` filter takes two arguments, separated by a comma: the first and
the last frame to keep from the clip. If you put 0 for the last frame, it's
the same as "end of clip," so if you only want to remove the first 119 frames
you should use Trim(120,0).

Keeping track of frame numbers this way is a chore. It's much easier to open
a partially-completed script in an application like VirtualDub which will
display the frame numbers for you. You can also use the :doc:`ShowFrameNumber <corefilters/showframes>`
filter, which prints each frame's number onto the frame itself.

In practice a much more useful source filter than :doc:`Version <corefilters/version>` is :doc:`AVISource <corefilters/avisource>`,
which reads in an AVI file (or one of several other types of files) from
disk. If you have an AVI file handy, you can try applying these same filters
to your file:
::

    AVISource("d:\capture.avi")  # or whatever the actual pathname is
    ReduceBy2
    FadeOut(15)
    Trim(120,0)

Even a single-line script containing only the AVISource command can be useful
for adding support for >2GB AVI files to applications which only support <2GB
ones.


--------


Non-Linear Editing
------------------

Now we're getting to the fun part. Make an AVS file with the following script
in it:
::

    StackVertical(Version, Version)

Now open it. Result: An output video with two identical lines of version
information, one on top of the other.
Instead of taking numbers or strings as arguments, :doc:`StackVertical <corefilters/stack>` takes
video clips as arguments. In this script, the Version filter is being called
twice. Each time, it returns a copy of the version clip. These two clips are
then given to :doc:`StackVertical <corefilters/stack>`, which joins them together (without knowing
where they came from).

One of the most useful filters of this type is :doc:`UnalignedSplice <corefilters/splice>`, which
joins video clips end-to-end. Here's a script which loads three AVI files
(such as might be produced by AVI_IO) and concatenates them together.
::

    UnalignedSplice(AVISource("d:\capture.00.avi"), \
    AVISource("d:\capture.01.avi"), \
    AVISource("d:\capture.02.avi"))

Both :doc:`StackVertical <corefilters/stack>` and :doc:`UnalignedSplice <corefilters/splice>` can take as few as two arguments
or as many as sixty.
You can use the ``+`` operator as a shorthand for :doc:`UnalignedSplice <corefilters/splice>`.

For example, this script does the same thing as the previous example:
::

    AVISource("d:\capture.00.avi") + \
    AVISource("d:\capture.01.avi") + \
    AVISource("d:\capture.02.avi")

Now let's suppose you're capturing with an application that also saves the
video in multiple AVI segments, but puts the audio in a separate WAV file.
Can we recombine everything? You bet:
::

    AudioDub(AVISource("d:\capture.00.avi") + \
    AVISource("d:\capture.01.avi") + \
    AVISource("d:\capture.02.avi"), \
    WAVSource("d:\audio.wav"))

$Date: 2008/07/18 17:38:49 $
