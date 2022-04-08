
AviSynth Syntax - Script functions
==================================

They provide AviSynth script information.

-   ScriptName   |   v2.60   |   ScriptName()

Returns the as entered path and filename of the loaded script as a string.

*Examples:*
::

    name = ScriptName() # name = "F:\Directory\Tmp\..\video.avs"

-   ScriptFile   |   v2.60   |   ScriptFile()

Returns the resolved filename of the loaded script as a string.

*Examples:*
::

    file = ScriptFile() # name = "video.avs"

-   ScriptDir   |   v2.60   |   ScriptDir()

Returns the resolved path of the loaded script as a string.

*Examples:*
::

    folder = ScriptDir() # name = "F:\Directory"

--------

Back to :doc:Internal functions <syntax_internal_functions>`.

$Date: 2011/04/17 03:58:18 $
