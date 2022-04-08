
AviSynth Runtime environment
============================


.. toctree::
    :maxdepth: 3

.. contents:: Table of contents



Definition
----------

The runtime environment is an extension to the normal AviSynth script
execution environment that is available to scripts executed by
:ref:`runtime filters <Runtime filters>`. Its basic characteristic is
that the runtime filters' scripts are evaluated (compiled) **at every frame**.
This allows for complex video processing that it would be difficult or
impossible to perform in a normal AviSynth script.

Let's now look at the above definition in more detail. The runtime
environment is:

1.  An environment, that is a set of available local and global variables
    and filters / functions to be used by the script.
2.  An extension to the normal AviSynth script execution environment;
    that is there are additional variables and functions available to runtime
    scripts.
3.  Available to scripts executed by runtime filters **only**, that is
    scripts inside it are executed only during the AviSynth "runtime" (the
    frame serving phase).

The last is the biggest difference. Normal script code is parsed and
evaluated (compiled) at the start of the script's :doc:`execution <../script_ref/script_ref_execution_model_sequence_events>` (the parsing
phase) in a linear fashion, from top to bottom; the result is the creation of
a :doc:`filter graph <../script_ref/script_ref_execution_model_filter_graph>` that is used by AviSynth to serve frames to the host video
application. Runtime script code is executed *after* the parsing phase, when
frames are served. Moreover it is compiled on every frame requested and only
for that specific frame, in an event-driven fashion.

**Note:** Audio is *not* handled by the runtime environment; it is passed
through untouched. This also means that you cannot modify clip audio with
runtime filters.


.. _Runtime filters:

Runtime filters
---------------

The following :doc:`filters: <../corefilters>` are the basic set of the so-called "runtime
filters":

-   :doc:`ConditionalFilter <../corefilters/conditionalfilter>`: Selects, on every frame, from one of two
    (provided) filters based on the evaluation of a conditional expression.
-   :doc:`ScriptClip <../corefilters/conditionalfilter>`: Compiles arbitrary script code on every frame and
    returns a clip.
-   :doc:`FrameEvaluate <../corefilters/conditionalfilter>`: Compiles arbitrary script code on every frame but
    the filter's output is ignored.
-   :doc:`ConditionalReader <../corefilters/conditionalreader>`: Loads input from an external file to a
    selectable :doc:`variable <syntax_script_variables>` on every frame.

In addition, the :doc:`WriteFile <../corefilters/write>` filter can also be considered a runtime filter,
because it sets the special variables set by all runtime filters before
evaluating the expressions passed to it, which can use the special :doc:`runtime functions <syntax_internal_functions_runtime>`.


.. _Special runtime variables and functions:

Special runtime variables and functions
---------------------------------------

All runtime filters set and make available to runtime scripts the following
special variables.

-   last: The clip passed as argument to the filter
-   current_frame: The frame number (ranging from zero to input clip's
    :doc:`Framecount <syntax_clip_properties>` minus one) of the requested frame.

All the above variables are defined at the :doc:`top-level script local scope <../script_ref/script_ref_execution_model_lifetime_variables>`.
That is you read and write to them in a runtime script as if they where
variables that you declare at the script level.

Runtime scripts can also call a rich set of :doc:`special functions <syntax_internal_functions_runtime>` that provide
various pieces of information for the current frame of their input clip.


How to script inside the runtime environment
--------------------------------------------

Using the runtime environment is simple: you use one of the :ref:`runtime
filters <Runtime filters>` and supply it with the needed arguments. Among them, two are the
most important:

-   The input clip
-   The runtime script

The latter is supplied as a string argument which contains the AviSynth
script commands. Scripts can contain many statements (you can use a multiline
string), local and global variables and function calls, as well as special
:doc:`runtime functions <syntax_internal_functions_runtime>` and :ref:`variables <Special runtime variables and functions>`. In general all statements of the
AviSynth :doc:`syntax <syntax>` are permitted, but attention must be paid to avoid overly
complex scripts because this has a penalty on speed that is paid at **every
frame**. See the runtime section of :doc:`scripting reference's <../script_ref/script_ref>`
:doc:`performance considerations <../script_ref/script_ref_execution_model_perf_cons>` for details.

Let's see a few examples:

::

    TODO...EXAMPLES

--------

Other points:

-   :doc:`Runtime functions <syntax_internal_functions_runtime>` and variables, such as current_frame,
    AverageLuma(), etc., are available only at the runtime script's scope. To
    use them in a function you must pass them as arguments.
-   You can include an explicit ``return`` statement in your runtime
    script to return a clip that is not contained in the special ``last``
    variable.

--------

Back to :doc:`AviSynth Syntax <syntax>`.

$Date: 2008/12/07 15:46:17 $
