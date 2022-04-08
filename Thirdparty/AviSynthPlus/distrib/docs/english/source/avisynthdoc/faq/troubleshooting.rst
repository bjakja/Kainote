
Troubleshooting
===============


.. toctree::
    :maxdepth: 3

.. contents:: Table of contents



Installation problems
---------------------

If you got problems getting AviSynth to work at all, try the following
script:
::

    Version()

and open it in Windows Media Player 6.4 (it is a file ``mplayer2.exe`` located
in ``C:\Program Files\Windows Media Player``, other versions of WMP will not
work). If you see a video with a message with Avisynth version and Copyright,
then it is installed properly.

If that doesn't work, you can try the following:

-   Empty the plugin folder of AviSynth: autoloaded scripts (``*.avsi``) or
    some filters can cause this (:ref:`see here <Check your autoloading plugin directory>`).
-   Install codecs, in particular `Huffyuv`_: it can be that there is no
    decoder present which can decode your video.
-   If you use an encoding package (like DVD2SVCD, GKnot, DVX, ...) make
    sure that you use the version of AviSynth that came with that package: it
    might be that new versions of AviSynth are not compatible with the
    package. Try to get support from the package developers.
-   Reinstall AviSynth: it might be that something went wrong with the
    installation. If you tried playing with new beta version, reinstall a
    stable release.
-   If all of the above doesn't help drop a post in the `Doom9 Forums`_.


Other problems
--------------

Creating scripts with AviSynth is not always easy, and sometimes AviSynth
produces very strange results. This is a little guide to help you figure out
the most common errors.


Write Simple
------------

If AviSynth produces strange results, try simplifying your script. Try
splitting up your script into as many lines as possible. This will help you
identify your problem. For example:
::

    video =
    AviSource("file23.avi").ConvertToYUY2().Trim(539,8534)
    return AudioDub(Blur(video,1.5).Reduceby2().Bilinearrresize(512,384),
    Wavsource("file23.wav").AmplifyDB(4.5)

is not as readable as
::

    AviSource("file23.avi")
    ConvertToYUY2()
    Trim(539, 8534)
    Blur(1.5)
    Reduceby2()
    Bilinearrresize(512, 384)
    AudioDub(Wavsource("file23.wav"))
    AmplifyDB(4.5)

Furthermore it has the advantage, that you more easily:

-   Comment out a single command (line). This is good for testing the
    effect of a filter.
-   Get the proper position (line) of problem command, if there is a
    syntax error.
-   Put in a "return last" at some position in the script. This will
    output the video at that place in the filterchain. So by varying the
    place of the "return last" you can check up to which line the video is
    correct.
-   Get an overview of the "flow" of the script. (Is it a good thing that
    the :doc:`Trim <../corefilters/trim>` command only affects the video in the clip above?)


Always check parameters
-----------------------

If you have a filter that gives you unexpected results, try using it with the
simplest parameters. Always check the internal filters either on AviSynth
homepage or in the documentation that came along with your AviSynth.

Be sure you use the same type of parameters as the ones described in the
documentation. The most common error in this case is related to the first
parameter in all filters, "clip". Be sure you understand how "implicit last"
works. If you do not have a "last clip", most filters will fail with an
"Invalid parameter" error.

"Filter does not return a clip" is reported if the output of your last filter
is put into a variable, and there isn't any "last clip". For instance:
::

    video = AviSource("file.avi")
    audio = WavSource("file.wav")
    combined = AudioDub(video, audio)

will fail. This can be solved by:
::

    video = AviSource("file.avi")
    audio = WavSource("file.wav")
    AudioDub(video, audio)

where 'last' now contains a clip, or:
::

    video = AviSource("file.avi")
    audio = WavSource("file.wav")
    combined = AudioDub(video, audio)
    return combined

where the variable is returned, or even:
::

    video = AviSource("file.avi")
    audio = WavSource("file.wav")
    return AudioDub(video, audio)


Test scripts using Virtualdub
-----------------------------

Always use `Virtualdub`_ or even better `VirtualDubMod`_ to test your
scripts. This is what all AviSynth functionality is tested against (by its
developers). AviSynth does of course work with other programs, but if you get
errors in other applications it's most likely not an AviSynth problem, but a
limitation within the software you are using.

These limitations are mostly linked to:

-   Color format problems. The application you are using does not support
    the color format you are using as script output.
-   Size problems. Some programs does not accept all sizes of images.


Go through the script step by step
----------------------------------

As mentioned in "Write Simple" it is always a good thing to test every step
of your script, if there are problems.

You can comment out a filter (filters) by placing a '#' in front of the line
(or before filter). That way it (and all rest of the line) will be ignored by
AviSynth.

You can put in a "return last" or "return myvariable" any place in the
script.

At any place in the script you can add the :doc:`Info() <../corefilters/info>` filter to get
information about the image and sound at the current stage of the filtering.


.. _Check your autoloading plugin directory:

Check your autoloading plugin directory
---------------------------------------

Plugins autoloading usually works fine, but you must NOT put here:

-   any plugins for incompatible AviSynth versions (e.g. old 2.0.x).
-   special LoadPluginEx.DLL plugin (from WarpSharp package) used for
    loading of old 2.0 plugins.
-   AviSynth C-plugins which use AviSynth C API instead of regular
    interface.
-   too many AviSynth plugins (this 50 plugins auto prescan load limit is
    removed in v2.57 though).
-   any other DLL files (usually it is safe, but is not recommended).

You must also remember, that all AVSI files in your plugin-directory are
automatically included in your script. This is a feature, to allow you to
include your own (or borrowed) functions, without have to copy/paste them
into every script.

*Notes. In old AviSynth versions (up to 2.0.7) all AVS files in your plugin-
directory were automatically included in your script. This also means that if
you copy any sample scripts into your plugin directory they will always be
included, and may generate errors (in old versions!).*

In general, any AVSI (early AVS) file whose commands are not wrapped into
functions will be problematic.

All other file formats besides AVSI and DLL files are ignored, so you can
safely leave your documentation there.

How to empty plugin dir? Simply create some subfolder (e.g. "hide") and move
all (or some) files there.

Remember some files (DirectShowSource.dll, TCPDeliver.dll plugins,
ColorRGB.avsi) are part of AviSynth (since v2.56).


Use conservative image sizes
----------------------------

If you have problems with distorted images, try using conservative frame
sizes. That means, use sizes, where height and width are always divisible by
16. Using image sizes that are not divisible by 2 is in many cases
problematic, and should always be avoided.

If you do however find that there is a problem with certain sizes of images,
please submit a bug-report. See below how to do that.


Finally check the AviSynth Q&A
------------------------------

If you still got problems (loading scripts in certain encoders, or colorspace
errors) have a look at the AviSynth Q&A, especially :ref:`Problems when
Encoder X reads AVS-files`. Be also sure to check What are the main
bugs in these versions in the FAQ.


Reporting bugs / Asking for help
--------------------------------

We will need many informations to be able to help you. If you don't supply us
with that, there is a good chance that we won't be able to help you or locate
the error.

Be sure to ** always** include:

-   AviSynth version. (and date of beta, if not a SourceForge final
    release)
-   The simplest possible script for recreating the error.
-   The EXACT error message you get.
-   VirtualDub (Mod) version.
-   All file information from VirtualDub / File / File Information.
-   Used plugin versions.
-   Codecs and image sizes of input material.

Bug reports should be submitted at the `SourceForge Project page`_. Be sure
to check if there is already a bug summitted similar to yours - there might
just be. Errors in external plugins shouldn't be reported here, but to the
author of the filter.

A very good place to get help is the `Doom9 Forums`_. Be sure to search the
forum before asking questions. Many topics have been covered there! - Then
enter into the discussion.

$Date: 2009/09/12 20:57:20 $

.. _Huffyuv: http://www.avisynth.org/mediawiki/wiki/huffyuv.htm
.. _Doom9 Forums: http://forum.doom9.org/forumdisplay.php?s=&forumid=33
.. _Virtualdub: http://www.avisynth.org/mediawiki/wiki/virtualdub.htm
.. _VirtualDubMod: http://www.avisynth.org/mediawiki/wiki/virtualdubmod.htm
.. _SourceForge Project page: http://sourceforge.net/projects/avisynth2
