
AviSynth Syntax - Control functions
===================================

They facilitate flow of control (loading of scripts, arguments checks, global
settings adjustment, etc.).

-   Apply   |     |   Apply(string func_string [, arg1 [, arg2 [, ... [,
    argn]]]] )

Apply calls the function or filter func_string with arguments arg1, arg2,
..., argn (as many as supplied). Thus, it provides a way to call a function
or filter **by name** providing arguments in the usual way as in a typical
function call. Consequently, ``Apply("f", x)`` is equivalent to ``f(x)``
which in turn is equivalent to ``Eval("f(" + String(x) + ")")``.

*Examples:*
::

    # here the same call to BicubicResize as in the Eval() example is shown
    Apply("BicubicResize", last, 352, 288)
    # Note that the clip argument must be supplied - 'last' is not implicitly assumed

-   Eval   |     |   Eval(expression [, string "name"])

Eval evaluates an arbitrary *expression* as if it was placed inside the
script at the point of the call to Eval and returns the result of evaluation
(either to the :doc:`variable <syntax_script_variables>` that is explicitly assigned to or to the last
special variable. You can use Eval to construct and evaluate expressions
dynamically inside your scripts, based on variable input data. Below some
specific examples are shown but you get the general idea.
Argument *name* is filename specified in the error message instead of script
name.

*Examples:*
::

    # this calls BicubicResize(last, 352, 288)
    settings = "352, 288"
    Eval( "BicubicResize(" + settings + ")" )
    ...
    # this will result in Defined(u) == false
    u = Eval("#")
    ...
    # this increments a global based on a variable's value
    dummy = Eval("global my_counter = my_counter + " + String(increment))

-   Import   |     |   Import(filename)

Import evaluates the contents of another AviSynth script and returns the
imported script's return value. Typically it is used to make available to the
calling script library functions and the return value is not used. However
this is simply a convention; it is not enforced by the :doc:`AviSynth Syntax <syntax>`.
See also the dedicated :doc:`Import <../corefilters/import>` page in :doc:`Internal filters <../corefilters>` for other
possible uses. Possible scenarios (an indicative list) where the return value
could be of use is for the library script to:

-   indicate whether it succesfully initialised itself (a bool return
    value),
-   inform for the number of presets found on disk (an int return value);

the value then could be tested by the calling script to decide what action to
take next.

*Examples:*
::

    Import("mylib.avsi")  # here we do not care about the value (mylib.avsi contains only functions)
    ...
    okflag = Import("mysources.avsi")  # mysources loads predetermined
    filenames from a folder into globals
    source = okflag ? global1 + global2 + global3 : BlankClip()

-   Select   |     |   Select(index, item0 [, item1 [, ... [, itemn]]])

Returns the item selected by the index argument, which must be of int type (0
returns item0, 1 returns item1, ..., etc). Items can be any :doc:`variable <syntax_script_variables>`
or expression of any type and can even be mixed.

*Examples:*
::

    # select a clip-brush from a set of presets
    idx = 2
    brush = Select(idx, \
         AviSource("round.avi"), \
         rectangle, \
         diagonal, \
         diagonal.FlipHorizontal)

-   Default   |     |   Default(x, d)

Returns *x* if Defined(x) is true, *d* otherwise. *x* must either be a
function's argument or an already declared script variable (ie a variable
which has been assigned a value) else an error will occur.

*Examples:*
::

    function myfunc(clip c, ..., int "strength") {
        ...
        strength = Default(strength, 4) # if not supplied make it 4
        ...
    }

-   Assert   |     |   Assert(condition [, err_msg])

Does nothing if condition is true; throws an error, immediately terminating
script execution, if *condition* is false. In the later case err_msg, if
supplied, is presented to the user; else the standard message "Assert:
assertion failed". shows up.

*Examples:*
::

    function myfunc(clip c, ..., int "strength") {
        ...
        strength = Default(strength, 4) # if not supplied make it 4
        Assert(strength > 0, "'strength' must be positive")
        ...
    }

-   NOP   |     |   NOP()

This is a no-operation function provided mainly for conditional execution
with non-return value items such as :doc:`Import <../corefilters/import>`, when no "else" condition is
desired. That is, use it whenever the :doc:`AviSynth Syntax <syntax>` requires an
operation (such as with the ?: operator) but your script does not need one.
Return value: 0 (int type).

*Examples:*
::

    preset = want_presets ? AviSource("c:\presets\any.avi") : NOP
    ...
    loadlib ? Import("my_useful_functions.avs") : NOP

-   UnDefined   |   v2.60   |   UnDefined() Returns the undefined state.
    It's the state for which Defined() returns false.

*Examples:*
::

    x = Undefined()
        Defined(x) # = true

-   SetMemoryMax   |   v2   |   SetMemoryMax(amount)

Sets the maximum memory (in MB) that AviSynth uses for its internal Video
Frame cache to the value of *amount*. From v2.5.8, setting to zero just
returns the current Memory Max value. In the 2.5 series the default Memory
Max value is 25% of the free physical memory, with a minimum of 16MB. From
rev 2.5.8 RC4, the default Memory Max is also limited to 512MB.

+-----------------------------+-----+-----+-----+-----+------+------+------+
| Free                        | <64 | 128 | 256 | 512 | 1024 | 2048 | 3072 |
+=============================+=====+=====+=====+=====+======+======+======+
| Default Max v2.57 and older | 16  | 32  | 64  | 128 | 256  | 512  | 768  |
+-----------------------------+-----+-----+-----+-----+------+------+------+
| Default Max since v2.58 RC4 | 16  | 32  | 64  | 128 | 256  | 512  | 512  |
+-----------------------------+-----+-----+-----+-----+------+------+------+

In some versions there is a default setting of 5MB, which is quite low. If
you encounter problems (e.g. low speed) try to set this values to at least
32MB. Too high values can result in crashes because of 2GB address space
limit.  Return value: Actual MemoryMax value set.

*Examples:*
::

    SetMemoryMax(128)

-   SetWorkingDir   |   v2   |   SetWorkingDir(path)

Sets the default directory for AviSynth to the *path* argument. This is
primarily for easy loading of source clips, :doc:`importing <../corefilters/import>` scripts, etc. It
does not affect plugins' autoloading. Return value is 0 if successful, -1
otherwise.

*Examples:*
::

    SetWorkingDir("c:\my_presets")
    AviSource("border_mask.avi")  # this loads c:\my_presets\border_mask.avi

-   SetPlanarLegacyAlignment   |   v2.56   |
    SetPlanarLegacyAlignment(mode)

Set alignment mode for `planar`_ frames. *mode* can either be true or false.
Some older :doc:`plugins <../externalplugins>` illegally assume the layout of video frames in memory.
This special filter forces the memory layout of planar frames to be
compatible with prior versions of AviSynth. The filter works on the
GetFrame() call stack, so it effects filters **before** it in the script.

*Examples:*
::

    Example - Using an older version of Mpeg2Source() (1.10 or older):

    LoadPlugin("...\Mpeg2Decode.dll")
    Mpeg2Source("test.d2v")         # A plugin that illegally assumes the layout of memory
    SetPlanarLegacyAlignment(true)  # Set legacy memory alignment for prior statements
    ConvertToYUY2()     # Statements through to the end of the script have
    ...                             # advanced memory alignment.

-   OPT_AllowFloatAudio   |   v2.57   |   global OPT_AllowFloatAudio = True

This option enables WAVE_FORMAT_IEEE_FLOAT audio output. The default is to
autoconvert Float audio to 16 bit.

-   OPT_UseWaveExtensible   |   v2.58   |   global OPT_UseWaveExtensible = True

This option enables WAVE_FORMAT_EXTENSIBLE audio output. The default is
WAVE_FORMAT_EX.

**Note:** The default DirectShow component for .AVS files,
"AVI/WAV File Source", does not correctly implement WAVE_FORMAT_EXTENSIBLE
processing, so many application may not be able to detect the audio track.
There are third party DirectShow readers that do work correctly. Intermediate
work files written using the AVIFile interface for later DirectShow
processing will work correctly if they use the DirectShow "File Source
(async)" component or equivalent.

-   OPT_VDubPlanarHack   |   v2.60   |   global OPT_VDubPlanarHack = True

This option enables flipped YV24 and YV16 chroma planes. This is an hack for
early versions of Virtualdub with YV24/YV16 support.

-   OPT_dwChannelMask   |   v2.60   |   global OPT_dwChannelMask(int v)

This option enables you to set ChannelMask. It overrides WAVEFORMATEXTENSIBLE.dwChannelMask which is set according to this table

| ``0x00004, // 1 -- -- Cf``
| ``0x00003, // 2 Lf Rf``
| ``0x00007, // 3 Lf Rf Cf``
| ``0x00033, // 4 Lf Rf -- -- Lr Rr``
| ``0x00037, // 5 Lf Rf Cf -- Lr Rr``
| ``0x0003F, // 5.1 Lf Rf Cf Sw Lr Rr``
| ``0x0013F, // 6.1 Lf Rf Cf Sw Lr Rr -- -- Cr``
| ``0x0063F, // 7.1 Lf Rf Cf Sw Lr Rr -- -- -- Ls Rs``

-   OPT_AVIPadScanlines   |   v2.60   |   global OPT_AVIPadScanlines =
    True

This option enables DWORD aligned planar padding. Default is packed aligned
planar padding. See `memory alignment used in the AVIFile output emulation (not yet written)`_.

--------

Back to :doc:`Internal functions <syntax_internal_functions>`.

$Date: 2012/04/15 14:13:14 $

.. _planar: http://avisynth.org/mediawiki/Planar
.. _memory alignment used in the AVIFile output emulation (not yet written):
    http://avisynth.org/mediawiki/index.php?title=AVIFile_output_emulation
