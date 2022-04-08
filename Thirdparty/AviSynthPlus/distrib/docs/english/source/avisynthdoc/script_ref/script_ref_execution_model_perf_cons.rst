
The script execution model - Performance considerations
=======================================================

This section presents some performance-related issues that originate from the
way AviSynth scripts are executed; it also provides advice on how to optimise
your scripts and AviSynth configuration so that your scripts are parsed
and/or encoded faster.


Plugin auto-loading
-------------------

An important thing to note is that auto-loading although is a convenient
method to have all your favorite filters on hands, it *does* incur a speed
penalty. The penalty is twofold:

1.  The loading, registering and unloading of plugins takes some time.
    The parsing of .avsi scripts also takes some time. This time, although
    small, is payed *in every* AviSynth script invocation.
2.  The registering of many functions and globals increases the size of
    internal AviSynth data structures, which on turn increases the seek time
    to locate a filter / variable during the parsing phase as well as during
    runtime script parsing.

For small scripts and / or small number of auto-loading plugins the ease of
use outweights the above speed penalty (since there is also a speed penalty
in writing a lot of :doc:`LoadPlugin <../syntax/syntax_plugins>` calls in every script that needs them).
However if you regularly write large and complex scripts and have a large
number of plugins / include scripts in your AviSynth plugin folder, you
should consider a more granular approach to increase overall script parsing /
encoding performance.

For example, you could group :doc:`LoadPlugin <../syntax/syntax_plugins>` calls for related plugins in
separate .avsi scripts and have a central .avsi script with a config function
that loads different .avsi scripts depending on its arguments. Then place in
the plugin folder only the central .avsi script and the bare-essential
plugins that you use almost every time.


Frame caching and the effect on spliting filter graph's paths
-------------------------------------------------------------

In order to improve performance AviSynth places, transparently to script
writers, a specialised Cache filter just after each filter. The purpose of
the cache is to avoid the computationally expensive generation of a video
frame that has recently been created; if the frame is in the cache then it is
returned immediately, avoiding a possibly long chain of filter calls.

The presence of the cache gives a speed and memory advantage to filter graphs
that split processing paths *as late as possible*. In our filter graph
example above, if instead of:

::

    ov = AviSource("clip2.avi")
    ov1 = Lanczos4Resize(ov, 280, 210)
    ov2 = ov1.Invert()

we have used the following code:

::

    ov = AviSource("clip2.avi")
    ov2 = ov
    ov1 = Lanczos4Resize(ov, 280, 210)
    ov2 = ov2.Lanczos4Resize(280, 210).Invert()

then the respective part of the filter graph would have been:

::

    ...
               |
    AviSource(clip2) <--+-- Lanczos4Resize <--+-- Overlay <--+
                        |
                        |
                        +-- Lanczos4Resize <-- Invert
                        <------+-- Overlay (filter graph's root)

In the later case we would have one more filter (and cache) in the filter
chain and -more importantly- we would have to generate two resized frames for
each call by the host application to get a frame instead of one.

Therefore, always try to split processing paths as late as possible; it will
make your scripts faster.


What *not* to include in runtime scripts
----------------------------------------

Although as said above, runtime scripts are parsed as regular scripts do and
thus every statement allowed to a regular script is allowed in a runtime
script, some statements are not advisable from a performance point of view.

The principal reason is that runtime script parsing occurs in *every* frame
requested. Therefore, as a rule of thumb, computationally expensive actions
should in general be placed outside the runtime environment (at the main
script) in order to be executed only once. This practice trades some start-up
overhead with savings during frame serving, which in general dominates the
overall clip rendering / encoding time; thus it is justified as an
optimisation. This is of course to be taken with a grain of salt because
there are circmustances where the application needs force the (balanced) use
of such statements.

Having said all that, let's see our not-to-do-in-runtime-scripts list (and
some interesting counter-examples):

-   The following actions should most of the time be avoided:

    -   Importing a script.
    -   Loading a plugin.
    -   Defining a user function.

Issuing them on every frame will slow down (maybe significantly) encoding
speed and (subject to implementation details) eat valuable memory. Moreover,
this overhead will be beared without returning significant gains. It is in
general much better to place them at the main script. See however an example
of acceptable use: `Subtitles from a changing text-file`_.

-   Calling a lot of filters / functions inside the runtime script will
    slow down your encoding speed. Those filters will be created and
    destroyed on every frame; thus you pay initialisation/cleanup costs at
    every frame.

If you can, put not-essential for the runtime processing filter calls outside
the runtime environment; break the runtime script in more scripts if you have
to. For example, instead of doing this:
::

    AviSource("myclip.avi")
    total_frames = Framecount()
    ScriptClip("""
        Levels(0, 0.9, 255, 5, 250)
        total_frames % current_frame < 2 ? FlipHorizontal : last
        Tweak(hue=18)
        Subtitle("frame: " + String(current_frame), y=320)
        """)

do this:

::

    AviSource("myclip.avi")
    total_frames = Framecount()
    Levels(0, 0.9, 255, 5, 250)
    ScriptClip("""total_frames % current_frame < 2 ? FlipHorizontal : last""")
    Tweak(hue=18)
    ScriptClip("""Subtitle("frame: " + String(current_frame), y=320)""")

-   :doc:`Arrays <script_ref_arrays>`, due to their recursive, script-based implementation can be
    expensive to parse, especially if they host a large number of elements.
    Using them without paying attention to minimize operations will slow down
    your encoding speed.

See however an example of acceptable use: `Per frame filtering, exporting
specific frame(s)`_ (note that FrameFilter is a wrapper around
:doc:`ScriptClip <../corefilters/conditionalfilter>`).

--------

Back to the :doc:`script execution model <script_ref_execution_model>`.

$Date: 2011/04/29 20:11:14 $

.. _Subtitles from a changing text-file:
    http://forum.doom9.org/showthread.php?t=129191
.. _Per frame filtering, exporting specific frame(s):
    http://avslib.sourceforge.net/examples/example-016.html
