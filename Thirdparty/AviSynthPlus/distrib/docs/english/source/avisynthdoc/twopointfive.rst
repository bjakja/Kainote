
AviSynth 2.5
============


New Video Features
------------------

-   A new color format: native YV12 support. Supports YV12 planar input
    from DirectShowSource, AviSource and several MPEG decoders.

For a description of the color formats AviSynth can deal with see
:doc:`the documentation for Convert() <corefilters/convert>`.


New Audio Features
------------------

-   Internal float point samples support. AviSynth now has internal
    processing of float samples.
-   Native Int24 and Int32 sample support. Support input and output of
    higher precision samples.
-   Multiple channel support. AviSynth now supports an unlimited number
    of channels for both input and output.
-   Automatic sample conversion. If some filters doesn't support a
    specific sample type, they are converted to the format preserving most
    quality (most often floats). Float samples are also automatically
    converted back to 16bit before delivering them as output.


New Conditional Filters (v2.52)
-------------------------------

-   It is now possible to change variables and filter parameter every
    frame - very powerful and versatile function!

See :doc:`ConditionalFilter <corefilters/conditionalfilter>`, :doc:`ConditionalReader <corefilters/conditionalreader>` and :ref:`here <runtime-functions>`.


New Internal Features
---------------------

-   Optimizations. Many basic features has been optimized, and now
    performs much better than previous versions.
-   Better cache efficiency. Filters can give hints to the cache, if they
    don't want their input cached, so memory can be used more efficiently.


C-style plugin support (v2.53)
------------------------------

Added C-style plugin support (still in testing) to allow plugin writers to
use other compilers than MSVC.
See `this thread`_ for further info. The C-plugins must be loaded with
LoadCPlugin after loading avisynth_c.dll. Your script becomes for example
::

    LoadPlugin("...\avisynth_c.dll")
    LoadCPlugin("...\smartdecimate.dll") # notice the C in loadCplugin
    AviSource(...)
    SmartDecimate()

DirectShow audio support (v2.53)
--------------------------------

Using DirectShowSource it is possible to load audio from all sources that can
be opened through DirectShow.
For example, formats like: WAV/DTS/AC3/MP3 can be opened. Look
:doc:`at the documentation <corefilters/directshowsource>` for more information.

Plugins
-------

Filters for AviSynth v1.0x and v2.0x do no longer work in v2.5x. When you try
to load them, they will most likely even crash. You have to use a special
plugin "`LoadPluginEx.dll (this plugin is contained in the WarpSharp
package)`_" to load v2.0x plugins in v2.5x.
You first have to load the v2.5x plugins, then LoadPluginEx.dll and finally
the v2.0x plugins. You will also need to copy :ref:`msvcp71.dll and msvcr71.dll <JapanesePlugin>`
to your system dir. Your script becomes for example
::

    LoadPlugin("C:\Program Files\avisynth2_temp\LoadPluginEx.dll")
    LoadPlugin("C:\Program Files\avisynth2_temp\plugins\dustv5.dll")

    AviSource("D:\clip.avi")
    ConvertToYUY2  # v2.0x plugins require YUY2 (or RGB)
    PixieDust(5)

I think that you can use the same plugin to load v2.5x plugins in v2.0x,
however I never have tested this.

$Date: 2007/12/03 19:45:17 $

.. _this thread: http://forum.doom9.org/showthread.php?s=&threadid=58840
.. _LoadPluginEx.dll (this plugin is contained in the WarpSharp package):
    http://www.geocities.co.jp/SiliconValley-PaloAlto/2382/
