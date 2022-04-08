
AviSynth Syntax - Plugins
=========================

With these functions you can add external functions to AviSynth.

``LoadPlugin`` ("filename" [, ...])

Loads one or more external avisynth plugins (DLLs).


--------

``LoadVirtualDubPlugin`` ("filename", "filtername", preroll)

This loads a plugin written for VirtualDub. "filename" is the name of the
.vdf file. After calling this function, the filter will be known as
"filtername" in avisynth. VirtualDub filters only supports RGB32. If the
video happens to be in RGB24-format, then you must use ``ConvertToRGB32``
(``ConvertToRGB`` won't suffice).

Some filters output depends on previous frames; for those preroll should be
set to at least the number of frames the filter needs to pre-process to fill
its buffers and/or updates its internal variables.


--------

``LoadVFAPIPlugin`` ("filename", "filtername")

This allows you to use VFAPI plugins (TMPGEnc import plugins).


--------

``LoadCPlugin`` ("filename" [, ...])
``Load_Stdcall_Plugin`` ("filename" [, ...])

Loads so called Avisynth C-plugins (DLLs).
Load_Stdcall_Plugin() is an alias for LoadCPlugin().
C-plugins are created on pure C language and use special "AviSynth C API"
(unlike ordinary Avisynt plugins which are created with MS C++). C-plugins
must be loaded with LoadCPlugin() or Load_Stdcall_Plugin().

Kevin provides a LoadCPlugin.dll that overloads the LoadCPlugin() verb to
support plugins compiled using the C subroutine calling sequence, use
Load_Stdcall_Plugin() to load stdcall calling sequence plugins when using
Kevins version. Advice: keep these plugins outside your auto plugin loading
directory to prevent crashes. `[discussion]`_ `[AVISynth C API (by
kevina20723)]`_

--------


Plugin autoload and name precedence (v2)
----------------------------------------

It is possible to put all plugins and script files with user-defined
functions or (global) variables in a directory from where all files with the
extension .AVSI (**v2.08, v2.5**, the type was .AVS in **v2.05-2.07**) and
.DLL are loaded at startup, unloaded and then loaded dynamically as the
script needs them.

.AVSI scripts in this directory should only contain function definitions and
global variables, no main processing section (else strange errors may occur),
it also is not recommended to put other files in that directory.

The directory is stored in the registry (the registry key has changed for
**v2.5**). You can use double-clicking a .REG-file with the following lines
to set the path (of course inserting your actual path):
::

    REGEDIT4

    [HKEY_LOCAL_MACHINE\SOFTWARE\Avisynth]
    "plugindir2_5"="c:\\program files\\avisynth 2.5\\plugins"

The order in which function names take precedence is as follows:
::

    user-defined function (always have the highest priority)
       plugin-function (have higher priority than built-in functions, they will override a built-in function)
          built-in function

Inside those groups the function loaded at last takes precedence, there is no
error in a namespace conflict.


Plugin autoload and conflicting function names (v2.55)
------------------------------------------------------

Starting from v2.55 there is DLLName_function() support. The problem is that
two plugins can have different functions which are named the same. To call
the needed one, DLLName_function() support is added. It auto-generates the
additional names both for auto-loaded plugins and for plugins loaded with
LoadPlugin.

**Some examples:**

::

    # using fielddeinterlace from decomb510.dll
    AviSource("D:\captures\jewel.avi")
    decomb510_fielddeinterlace(blend=false)

Suppose you have  the plugins mpeg2dec.dll and mpeg2dec3.dll in your auto
plugin dir, and you want to load a d2v file with mpeg2dec.dll (which outputs
YUY2):

::

    # using mpeg2source from mpeg2dec.dll
    mpeg2dec_mpeg2source("F:\From_hell\from_hell.d2v")

or with mpeg2dec3.dll (which outputs YV12):

::

    # using mpeg2source from mpeg2dec3.dll
    mpeg2dec3_mpeg2source("F:\From_hell\from_hell.d2v")

$Date: 2008/04/20 19:07:34 $

.. _[discussion]: http://forum.doom9.org/showthread.php?s=&threadid=58840
.. _[AVISynth C API (by kevina20723)]:
    http://kevin.atkinson.dhs.org/avisynth_c/
