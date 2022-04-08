
AviSynth FAQ - Using VirtualDub plugins in AviSynth
===================================================


.. toctree::
    :maxdepth: 3

.. contents:: Table of contents



Where can I download the latest version of scripts which import filters from VirtualDub?
----------------------------------------------------------------------------------------

The AviSynth scripts are on the `Shared_functions`_ page, or you can download
a package called vdub_filtersv15.zip from `[1]`_.


Which filters can be imported?
------------------------------

Most filters. Read the corresponding documentation.


Do these scripts work in RGB-space or in YUV-space?
---------------------------------------------------

You need to convert your clip to RGB32 before applying these scripts.


How do I make such a script?
----------------------------

Take a look at the following example script (this VirtualDub filter can be
downloaded from `Donald's homepage`_):

Smart Bob by Donald Graft:

::

    function VD_SmartBob(clip clip, bool show_motion, int threshold, bool
                         motion_map_denoising)
    {
      LoadVirtualdubPlugin("d:\bob.vdf", "_VD_SmartBob", 1)
      return clip.SeparateFields._VD_SmartBob(clip.GetParity?1:0,
        \  default(show_motion,false)?1:0, default(threshold,10),
        \  default(motion_map_denoising,true)?1:0)
    }

The VirtualDub plugin is imported with the command "LoadVirtualdubPlugin".
The first argument gives the path of the plugin, the second argument the name
for the plugin that will be used in the script and the third argument is
called the preroll.

The preroll should be set to at least the number of frames the filter needs
to pre-process to fill its buffers and/or updates its internal variables.
This last argument is used in some filters like: SmartBob, SmartDeinterlace,
TemporalCleaner and others. The reason is that due to filtering architecture
of VirtualDub the future frames can't be accessed by a filter. Dividee
reports: "In the "Add filter" dialog of VirtualDub, some filters have a
"Lag:" value in their description. I think this is the value that must be
used as preroll. Unfortunately, this indication is not always present. In
those cases you have to guess." Of course you can always ask the creator of
the filter.

The first step is to find out the sequence of the arguments in the last line
where the clip is returned. Configure the script in VirtualDub and select
"Save processing Settings" in the File Menu or press Ctrl+S. Open the created
.vcf file with a text editor and you should see lines like this:

::

    VirtualDub.video.filters.Add("smart bob (1.1 beta 2)");
    VirtualDub.video.filters.instance[0].Config(1, 0, 10, 1);

The order of the arguments is the one that has to be used in AviSynth. To
find the role of the arguments, play with them in VirtualDub and examine the
resulting lines.

The second step is to test the filter and to compare it with the VirtualDub
filter itself. For the programming itself you can learn a lot by looking at
the script which are already contained in vdub_filters.avs.

Example script which uses the function VD_SmartBob:

::

    Import("d:\vdub_filters.avs")
    AviSource("d:\filename.avi")
    ConvertToRGB32()  # only when necessary (but doesn't hurt)
    VD_SmartBob(false, 10, true)
    ConvertBackToYUY2()  # only when necessary

The package vdub_filtersv15.zip is a bit outdated since many new VirtualDub
filters are not in it. If that's the case for your VirtualDub filter and you
don't want to create a function yourself (such as VD_SmartBob), could also
use the following script:

::

    LoadVirtualdubplugin("d:\bob.vdf", "VD_SmartBob", 1)
    VD_SmartBob(1, 0, 10, 1) # parameters taken from the .vcf file

| :doc:`Main Page <faq_sections>` | :doc:`General Info <faq_general_info>` | :doc:`Loading Clips <faq_loading_clips>` | :doc:`Loading Scripts <faq_frameserving>` | :doc:`Common Error Messages <faq_common_errors>` | :doc:`Processing Different Content <faq_different_types_content>` | :doc:`Dealing with YV12 <faq_yv12>` | **Processing with Virtualdub Plugins** |

$Date: 2009/09/12 20:57:20 $

.. _Shared_functions: http://avisynth.org/mediawiki/Shared_functions
.. _[1]: http://neuron2.net/hosted.html (http://neuron2.net/hosted.html)
.. _Donald's homepage: http://neuron2.net/bob.html
