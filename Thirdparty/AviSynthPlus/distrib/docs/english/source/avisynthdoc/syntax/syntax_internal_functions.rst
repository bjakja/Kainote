
AviSynth Syntax - Internal functions
====================================

In addition to :doc:`internal filters <../corefilters>` AviSynth has a fairly large number of
other (non-clip) internal functions. The input or/and output of these
functions are not clips, but some other variables which can be used in a
script. They are roughly classified as follows:

-   :doc:`Boolean functions <syntax_internal_functions_boolean>`

They return true or false, if the condition that they test holds or not,
respectively.

-   :doc:`Control functions <syntax_internal_functions_control>`

They facilitate flow of control (loading of scripts, arguments checks, global
settings adjustment, etc.).

-   :doc:`Conversion functions <syntax_internal_functions_conversion>`

They convert between different types.

-   :doc:`Numeric functions <syntax_internal_functions_numeric>`

They provide common mathematical operations on numeric variables.

-   :doc:`Runtime functions <syntax_internal_functions_runtime>`

These are internal functions which are evaluated at every frame. They can be
used inside the scripts passed to runtime filters (:doc:`ConditionalFilter <../corefilters/conditionalfilter>`,
:doc:`ScriptClip <../corefilters/conditionalfilter>`, :doc:`FrameEvaluate <../corefilters/conditionalfilter>`) to return information for a frame.

-   :doc:`Script functions <syntax_internal_functions_script>`

They provide AviSynth script information.

-   :doc:`String functions <syntax_internal_functions_string>`

They provide common operations on string variables.

-   :doc:`Version functions <syntax_internal_functions_version>`

They provide AviSynth version information.

$Date: 2011/01/16 12:24:09 $
