
AviSynth FAQ - General information
==================================


.. toctree::
    :maxdepth: 3

.. contents:: Table of contents



.. _What is AviSynth:

What is AviSynth?
-----------------

AviSynth (AVI SYNTHesizer) is a frameserver. An excellent description is
given on `Lukes homepage`_:

"AviSynth is a very useful utility created by Ben Rudiak-Gould. It provides
many options for joining and filtering videos. What makes AviSynth unique is
the fact that it is not a stand-alone program that produces output files.
Instead, AviSynth acts as the "middle man" between your videos and video
applications.

Basically, AviSynth works like this: First, you create a simple text document
with special commands, called a script. These commands make references to one
or more videos and the filters you wish to run on them. Then, you run a video
application, such as Virtualdub, and open the script file. This is when
AviSynth takes action. It opens the videos you referenced in the script, runs
the specified filters, and feeds the output to video application. The
application, however, is not aware that AviSynth is working in the
background. Instead, the application thinks that it is directly opening a
filtered AVI file that resides on your hard drive.

There are five main reasons why you would want to use AviSynth:

1.  Join Videos: AviSynth lets you join together any number of videos,
    including segmented AVIs. You can even selectively join certain portions
    of a video or dub soundtracks.
2.  Filter Videos: Many video processing filters are built in to
    AviSynth. For example, filters for resizing, cropping, and sharpening
    your videos.
3.  Break the 2 GB Barrier: AviSynth feeds a video to a program rather
    than letting the program directly open the video itself. Because of this,
    you can usually use AviSynth to open files larger than 2 GB in programs
    that don't natively support files of that size.
4.  Open Unsupported Formats: AviSynth can open almost any type of video,
    including MPEGs and certain Quicktime MOVs. However, when AviSynth feeds
    video to a program, it looks just like a standard AVI to that program.
    This allows you to open certain video formats in programs that normally
    wouldn't support them.
5.  Save Disk Space: AviSynth generates the video that it feeds to a
    program on the fly. Therefore, no temporary or intermediate videos are
    created. Because of this, you save disk space."


.. _Who is developing AviSynth:

Who is developing AviSynth?
---------------------------

Originally AviSynth (up to v1.0b) was developed by Ben Rudiak-Gould. See
`mirror of his homepage`_. Currently it is developed by Sh0dan, IanB,
d'Oursse (AviSynth v3), Bidoche (AviSynth v3) and `others`_.


.. _Where can I download the latest versions of AviSynth:

Where can I download the latest versions of AviSynth?
-----------------------------------------------------

The most recent stable version is v2.57, which can be found `on Sourceforge`_ (just as
more recent builds).


.. _What are the main bugs in these versions:

What are the main bugs in these versions?
-----------------------------------------

Current bugs can be found in the documentation on the `AviSynth project
page`_. Fixed bugs can be found in the :doc:`Changelist <../changelist>`.


.. _Where can I find documentation about AviSynth:

Where can I find documentation about AviSynth?
----------------------------------------------

Documentation about the filters of AviSynth can be found on the main
page of `AviSynth's website`_, and in particular here: :doc:`Internal filters <../corefilters>`. You should read these
documents before posting to the forum, but it's OK to post if you have
trouble understanding them.


.. _How do I install/uninstall AviSynth:

How do I install/uninstall AviSynth?
------------------------------------

Starting from v2.06 AviSynth comes with an auto installer. Also make sure you
have no other versions of AviSynth floating around on your harddisk, because
there is a chance that one of those versions will be registered. Remove them
if necessary. For uninstalling AviSynth go to "program", "AviSynth 2.5" and
select "Uninstall AviSynth".

Installing AviSynth v2.05 or older versions: move avisynth.dll to your
system/system32 directory and run install.reg. For uninstalling run
uninstall.reg and delete avisynth.dll.


.. _What is the main difference between v1.0x, v2.0x, v2.5x, v2.6x and v3.x:

What is the main difference between v1.0x, v2.0x, v2.5x, v2.6x and v3.x?
------------------------------------------------------------------------

The versions v1.0x and v2.0x are compatible and outdated. The main difference
with v2.5x is that the internal structure of AviSynth has changed (YV12 and
multichannel support) with the consequence that external plugins compiled for
v1.0x/v2.0x will not work for v2.5x/v2.6x and vice versa. In v2.6x other
planar formats like YV24 and Y8 are added. v2.5x plugins will work in v2.6x
but not vice-versa. All versions are incompatible with v3.x, which will also
work under Linux/MacOSX (see `AviSynth v3`_) and rely on the GStreamer API.


.. _How do I know which version number of AviSynth I have:

How do I know which version number of AviSynth I have?
------------------------------------------------------

Open a text-editor, for example notepad. Add the following line

::

    Version()

and save the file with the extension "avs". Save for example as "version.avs"
(make sure that the extension is "avs" and not "txt"). Open the file in an
application which can read AVI-files, for example WMP 6.4 or Media Player
Classic. The version number will be displayed.


.. _How do I make an AVS-file:

How do I make an AVS-file?
--------------------------

Use your preferred text editor (e.g. Notepad). See also :ref:`this <How do I know which version number of AviSynth I have>`.

Although AviSynth doesn't need them, there are several GUIs (graphical user
interface) which may help you writing your AVS files. You can read a
description for each one of them :ref:`under the GUI entry <Does AviSynth have a GUI (graphical user interface)>`.


.. _Where do I save my AVS-file:

Where do I save my AVS-file?
----------------------------

Anywhere on your hard-disk.


.. _Are plugins compiled for v2.5x/v2.6x compatible with v1.0x/v2.0x and vice versa:

Are plugins compiled for v2.5x/v2.6x compatible with v1.0x/v2.0x and vice versa?
--------------------------------------------------------------------------------

As explained :ref:`earlier <What is the main difference between v1.0x, v2.0x, v2.5x, v2.6x and v3.x>` that is not the case. However it is possible to use a
v1.0x/v2.0x plugin in v2.5x/v2.6x, as explained :ref:`next <How do I use a plugin compiled for v2.0x in v2.5x>`.


.. _How do I use a plugin compiled for v2.0x in v2.5x:

How do I use a plugin compiled for v2.0x in v2.5x?
--------------------------------------------------

See http://avisynth.nl/index.php/LoadOldPlugins. (When using an older version of LoadPluginEx.dll, don't
move this plugin to your plugin dir. But move it to a separate folder,
otherwise VirtualdubMod and WMP6.4 will crash on exit.) This will enable you
using v2.0x plugins in v2.5x. An example script (using the v2.0x plugin Dust
by Steady):

::

    LoadPlugin("C:\Program Files\avisynth2_temp\plugins\LoadPluginEx.dll")
    LoadPlugin("C:\Program Files\avisynth2_temp\plugins\dustv5.dll")

    AviSource("D:\clip.avi")
    ConvertToYUY2()
    PixieDust(5)

If you want to automate this process, have a look at `LoadOldPlugins`_.


.. _How do I switch between different AviSynth versions without re-install:

How do I switch between different AviSynth versions without re-install?
-----------------------------------------------------------------------

-   You can use AvisynthSwitcher available `on lalternative`_. Versions v2.08 and
    v2.50 are provided, but you can easily add a new one under
    AvisynthSwitcher\versions\Avisynth 2.x.x.

-   Some other ways are described `in this thread`_.


.. _VirtualdubMod, WMP6.4, CCE and other programs crash every time on exit (when previewing an avs file):

VirtualdubMod, WMP6.4, CCE and other programs crash every time on exit (when previewing an avs file)?
-----------------------------------------------------------------------------------------------------

This problem can be caused by certain plugins in your (autoloading) plugin
folder. The solution is to move the problematic plugins outside your plugin
folder and load them manually.


.. _My computer seems to crash at random during a second pass in any encoder:

My computer seems to crash at random during a second pass in any encoder?
-------------------------------------------------------------------------

AviSynth is highly optimized. As a consequence it is possible that your
computer seems to crash at random during a second pass. Try running the
`Prime95`_ stress test for an hour, to check if your system is stable. If
this test fails (or your computer locks up) make sure that your computer is
not overclocked and lower your bus speed of your processor in steps of (say)
five MHz till the crashes are gone.


.. _Is there a command line utility for encoding to DivX/XviD using AviSynth:

Is there a command line utility for encoding to DivX/XviD using AviSynth?
-------------------------------------------------------------------------

-   There is a command line utility called `AVS2AVI`_ (and AVS2AVI GUI)
    for encoding to DivX / XviD using AviSynth. [`discussion thread`_]
-   `xvid_encraw`_ for encoding to XviD in M4V. Use `mp4box`_ or `YAMB`_
    to mux it into MP4.


.. _Does AviSynth have a GUI (graphical user interface):

Does AviSynth have a GUI (graphical user interface)?
----------------------------------------------------

AviSynth doesn't have a full fledged gui, but several tools are available:

-   `VirtualDubMod`_: The following AviSynth related utilities are
    present:

    -   'Open via AVISynth' command: This allows you to open any AviSynth
        compatible video file by automatically generating a suitable script by a
        selectable template.
    -   AVS Editor (Hotkey Ctrl+E): Just open your AVS and under tools
        select "script editor". Change something and press F5 to preview the
        video.

-   AvisynthEditor: This is an advanced AviSynth script editor featuring
    syntax highlighting, auto-complete code and per version plugin definition
    files. `Here is a screenshot`_. It can be found `on lalternative`_. Discussion can
    be found on `Doom9.org forum`_.
-   `AVSGenie`_: AVSGenie allows the user to select a filter from a drop
    down list or from a popup menu. An editable page of parameters will then
    be brought into view, with a guide to the filter and it's parameters. A
    video preview window opens, showing "source" and "target" views. The
    source window, in simple cases, shows output of the first line of the
    script, generally an opened video file. The target window shows the
    output of the whole script. In this way, effects of filters can easily be
    seen. The line which represents the source window can be changed.
    Discussion can be found `on Doom9`_.
-   `SwiftAVS (by Snollygoster)`_: Another nice gui, formerly known as
    AviSynthesizer. [`discussion`_]
-   `AvsP`_: It's a tabbed script editor for Avisynth. It has many
    features common to programming editors, such as syntax highlighting,
    autocompletion, call tips. It also has an integrated video preview, which
    when coupled with tabs for each script make video comparisons a snap.
    What really makes AvsP unique is the ability to create graphical sliders
    and other elements for any filter's arguments, essentially giving
    Avisynth a gui without losing any of its powerful features. Discussion
    can be found `here`_.

| :doc:`Main Page <faq_sections>` | **General Info** | :doc:`Loading Clips <faq_loading_clips>` | :doc:`Loading Scripts <faq_frameserving>` | :doc:`Common Error Messages <faq_common_errors>` | :doc:`Processing Different Content <faq_different_types_content>` | :doc:`Dealing with YV12 <faq_yv12>` | :doc:`Processing with Virtualdub Plugins <faq_using_virtualdub_plugins>` |

$Date: 2008/10/26 14:18:53 $

.. _Lukes homepage: http://neuron2.net/LVG/avisynth.html
.. _mirror of his homepage:
    http://neuron2.net/www.math.berkeley.edu/benrg/index.html
.. _others: http://sourceforge.net/project/memberlist.php?group_id=57023
.. _on Sourceforge: http://sourceforge.net/project/showfiles.php?group_id=57023
.. _AviSynth project page:
    http://sourceforge.net/tracker/?atid=482673&group_id=57023
.. _AviSynth's website: http://avisynth.org/mediawiki/Main_Page
.. _AviSynth v3: http://avisynth.org/mediawiki/AviSynth_v3
.. _LoadOldPlugins: http://avisynth.org/mediawiki/LoadOldPlugins
.. _on lalternative: http://www.lalternative.org
.. _in this thread: http://forum.doom9.org/showthread.php?s=&threadid=45181
.. _Prime95: http://www.mersenne.org/freesoft.htm
.. _AVS2AVI: http://www.avs2avi.org/
.. _discussion thread: http://forum.doom9.org/showthread.php?t=71493
.. _xvid_encraw: http://forum.doom9.org/showthread.php?t=98469
.. _mp4box: http://kurtnoise.free.fr/index.php?dir=mp4tools/
.. _YAMB: http://forum.doom9.org/showthread.php?t=115459
.. _VirtualDubMod: http://avisynth.org/mediawiki/VirtualDubMod
.. _Here is a screenshot:
    http://www.lalternative.org/img/AvisynthEditor.gif
.. _Doom9.org forum:
    http://forum.doom9.org/showthread.php?s=&threadid=49487
.. _AVSGenie: http://www.yeomanfamily.demon.co.uk/avsgenie/avsgenie.htm
.. _on Doom9: http://forum.doom9.org/showthread.php?s=&threadid=54090
.. _SwiftAVS (by Snollygoster):
    http://sourceforge.net/project/showfiles.php?group_id=74272
.. _discussion: http://forum.doom9.org/showthread.php?s=&threadid=48326
.. _AvsP: http://avisynth.org/qwerpoi/Download.html
.. _here: http://forum.doom9.org/showthread.php?t=129385
