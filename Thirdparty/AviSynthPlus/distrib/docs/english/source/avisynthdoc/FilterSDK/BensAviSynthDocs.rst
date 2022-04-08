
BensAviSynthDocs
================

This is a text of the original documentation written for AviSynth 1.0 by Ben
Rudiak-Gould - copied from `<http://math.berkeley.edu/~benrg/avisynth-extensions.html>`_.
Now (November 2006) it is available online at the `mirror.`_

.. toctree::
    :maxdepth: 3

.. contents:: Table of contents

Writing Avisynth plugins
------------------------


An example
~~~~~~~~~~

I'll start off with a complete, working Avisynth plugin. It's called
"Invert," and it produces a photo-negative of the input clip.

**Here's Invert.cpp**::

    #include "avisynth.h"


    class Invert : public GenericVideoFilter {
    public:
        Invert(PClip _child) : GenericVideoFilter(_child) {}
        PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment*
        env);
    };


    PVideoFrame __stdcall Invert::GetFrame(int n, IScriptEnvironment*
    env) {

        PVideoFrame src = child->GetFrame(n, env);
        PVideoFrame dst = env->NewVideoFrame(vi);

        const unsigned char* srcp = src->GetReadPtr();
        unsigned char* dstp = dst->GetWritePtr();

        const int src_pitch = src->GetPitch();
        const int dst_pitch = dst->GetPitch();
        const int row_size = dst->GetRowSize();
        const int height = dst->GetHeight();

        for (int y = 0; y < height; y++) {
            for (int x = 0; x < row_size; x++)
                dstp[x] = srcp[x] ^ 255;
            srcp += src_pitch;
            dstp += dst_pitch;
        }

        return dst;
    }


    AVSValue __cdecl Create_Invert(AVSValue args, void* user_data,
    IScriptEnvironment* env) {
        return new Invert(args[0].AsClip());
    }


    extern "C" __declspec(dllexport) const char* __stdcall
    AvisynthPluginInit(IScriptEnvironment* env) {
        env->AddFunction("Invert", "c", Create_Invert, 0);
        return "`Invert' sample plugin";
    }


Compile this file into a DLL named Invert.dll. Now create an
Avisynth script which looks something like this::

    LoadPlugin("d:\path\Invert.dll")
    clip = AVISource("d:\path2\video.avi")
    return clip.Invert()

If all is well, you should see a photo negative of your video clip
when you open this script.

How it works
~~~~~~~~~~~~

Here's a line-by-line breakdown of Invert.cpp.
::

    #include "avisynth.h"

This header declares all the classes and miscellaneous constants that you
might need when writing a plugin. All external plugins should #include it.

External plugins do not link with ``avisynth.dll``, so they can't directly
access functions that are defined in the main Avisynth source code.
Therefore, every important function in ``avisynth.h`` is either defined
inline or declared as ``virtual``. The virtual functions act as callbacks for
external DLLs.
::

    class Invert : public GenericVideoFilter {

An Avisynth filter is simply a C++ class implementing the ``IClip``
interface. ``IClip`` has four pure virtual methods: ``GetVideoInfo``,
``GetFrame``, ``GetParity``, and ``GetAudio``.

The class ``GenericVideoFilter`` is a simple do-nothing filter defined in
``avisynth.h``. It derives from ``IClip`` and implements all four methods.
Most filters can inherit from ``GenericVideoFilter`` rather than directly
from ``IClip``; this saves you from having to implement methods that you
don't care about, like ``GetAudio``.
::

    Invert(PClip _child) : GenericVideoFilter(_child) {}

A ``PClip`` is a "smart pointer" to an ``IClip``. It maintains a reference
count on the IClip object and automagically deletes it when the last PClip
referencing it goes away. For obvious reasons, you should always use PClip
rather than IClip* to refer to clips.

Like a genuine pointer, a ``PClip`` is only four bytes long, so you can pass
it around by value. Also like a pointer, a ``PClip`` can be assigned a null
value (0), which is often useful as a sentinel. Unlike a pointer, ``PClip``
is initialized to 0 by default.

You'll need to make sure your class doesn't contain any circular ``PClip``
references, or any ``PClip``s sitting in dynamically allocated memory that
you forget to ``delete``. Other than that, you don't have to worry about the
reference-counting machinery.

Avisynth filters have a standardized output channel via ``IClip``, but
(unlike VirtualDub filters) no standardized input channel. Each filter is
responsible for obtaining its own source material -- usually (as in this
case) from another clip, but sometimes from several different clips, or from
a file.

``GenericVideoFilter`` has a single constructor taking a single clip, which
it then simply passes through to its output. We will override the
``GetFrame`` method to do something more useful, while leaving the other
three methods as-is to pass through aspects of the clip that we don't need to
change.
::

    PVideoFrame Invert::GetFrame(int n, IScriptEnvironment* env) {

This method is called to make our filter produce frame ``n`` of its output.
The second argument, ``env``, is for our purposes simply a callback suite. It
is actually implemented in Avisynth by a class called ``ScriptEnvironment``.
One instance of this class is created for each opened AVS script, so there
may sometimes be several instances active at once. It is important that the
callback functions be called through the proper instance. A particular
instance of your class will only be used in one ScriptEnvironment, but
different instances might be used in different ScriptEnvironments.

This method returns a PVideoFrame, which is a smart pointer like PClip.
::

    PVideoFrame src = child->GetFrame(n, env);

"``child``" is a protected member of ``GenericVideoFilter``, of type PClip.
It contains the clip that was passed to the constructor. For our filter to
produce frame ``n`` we need the corresponding frame of the input. If you need
a different frame from the input, all you have to do is pass a different
frame number to ``child->GetFrame``.

``GetFrame`` calls are usually intercepted by Avisynth's internal caching
code, so the frame request may never actually reach the child filter.
::

    PVideoFrame dst = env->NewVideoFrame(vi);

The ``NewVideoFrame`` callback allocates space for a video frame of the
supplied size. (In this case it will hold our filter's output.) The frame
buffer is uninitialized raw memory (except that in the debug build it gets
filled with the repeating byte pattern 0A 11 0C A7 ED, which is easy to
recognize because it looks like "ALLOCATED").

"``vi``" is another protected member of ``GenericVideoFilter`` (the only
other member, actually). It is a struct of type ``VideoInfo``, which contains
information about the clip (like frame size, frame rate, pixel format, audio
sample rate, etc.). ``NewVideoFrame`` uses the information in this struct to
return a frame buffer of the appropriate size.

Frame buffers are reused once all the PVideoFrame references to them go away.
So usually ``NewVideoFrame`` won't actually need to allocate any memory from
the heap.
::

    const unsigned char* srcp = src->GetReadPtr();
    unsigned char* dstp = dst->GetWritePtr();

All frame buffers are readable, but not all are writable.

The rule about writability is this: *A buffer is writable if and only if
there is exactly one PVideoFrame pointing to it.* In other words, you can
only write to a buffer if no one else might be reading it. This rule
guarantees that as long as you hold on to a PVideoFrame and don't write to it
yourself, that frame will remain unchanged. The only drawback is that you
can't have two PVideoFrames pointing to a writable buffer. This can sometimes
be an inconvenience, as I'll explain later.

Any buffer you get from ``NewVideoFrame`` is guaranteed to be writable (as
long as you only assign it to one PVideoFrame!). Our filter's ``dst`` came
from NewVideoFrame, so we can safely call dst->GetWritePtr(). However, frames
you get from other clips via ``GetFrame`` may not be writable, in which case
GetWritePtr() will return a null pointer.

There is an ``IsWritable()`` method which you can call to find out if a
buffer is writable or not, and there's a ``MakeWritable`` callback (described
below) to ensure that it is.
::

    const int src_pitch = src->GetPitch();
    const int dst_pitch = dst->GetPitch();

Just as in VirtualDub, the "pitch" of a frame buffer is the offset (in bytes)
from the beginning of one scan line to the beginning of the next. The source
and destination buffers won't necessarily have the same pitch.

Buffers created by ``NewVideoFrame`` are always quadword (8-byte) aligned and
always have a pitch that is a multiple of 8.
::

    const int row_size = dst->GetRowSize();

The row size is the length of each row in bytes (not pixels). It's usually
equal to the pitch or slightly less, but it may be significantly less if the
frame in question has been through ``Crop``.

Since our source and destination frames have the same width and pixel format,
they will always have the same row size. Thus I only need one row_size
variable, and I could just as well have called src->GetRowSize().
::

    const int height = dst->GetHeight();

The height is the height. (In pixels.) Again, for our filter this is the same
for the source and the destination.
::

        for (int y = 0; y < height; y++) {
            for (int x = 0; x < row_size; x++)
                dstp[x] = srcp[x] ^ 255;
            srcp += src_pitch;
            dstp += dst_pitch;
        }

This is the code that does the actual work. The "srcp += src_pitch; dstp +=
dst_pitch;" idiom is a useful way of dealing with potentially differing
pitches without too much grief.
::

    return dst;

``GetFrame`` returns the newly-created frame. Our own references to this
frame and to the source frame will go away with the ``src`` and ``dst``
variables. Our caller will become sole owner of the destination frame (which
therefore will still be writable), and the source frame will be retained in
the cache and eventually recycled. All through the magic of C++ classes.
::

    AVSValue __cdecl Create_Invert(AVSValue args, void* user_data, IScriptEnvironment* env) {

In order to use our new filter, we need a scripting-language function which
creates an instance of it. This is that function.

Script functions written in C++ take three arguments. ``args`` contains all
the arguments passed to the function by the script. ``user_data`` contains
the void pointer which you passed to ``AddFunction`` (see below). Usually you
won't need this. ``env`` contains the same IScriptEnvironment pointer that
will later be passed to ``GetFrame``.

``AVSValue`` is a variant type which can hold any one of the following: a
boolean value (true/false); an integer; a floating-point number; a string; a
video clip (PClip); an array of AVSValues; or nothing ("undefined"). You can
test which one it is with the methods ``IsBool()``, ``IsInt()``,
``IsFloat()``, ``IsString()``, ``IsClip()``, ``IsArray()``, and ``Defined()``
(which returns true if the AVSValue is not "undefined"). You can get the
value with ``AsBool()``, ``AsInt()``, etc. For arrays, you can use the
``ArraySize()`` method to get the number of elements, and ``[]`` indexing to
get the elements themselves. For convenience, ``IsFloat()`` and ``AsFloat()``
will work with integers also. But boolean values are not treated as numeric
(unlike C).

The name "Create_Invert" is arbitrary. This function will actually be known
as "Invert" in scripts, because that's the name we pass to ``AddFunction``
below.
::

    return new Invert(args[0].AsClip());

The ``args`` argument passed to a script function will always be an array.
The return value should be any one of the other types (never an array).

The types of the values in the ``args`` array are guaranteed to match one of
the function signatures that you pass to ``AddFunction``, just as in
VirtualDub. Therefore, there's no need to worry about ``IsClip`` here.

``Create_Invert`` simply creates and returns a filter instance; it is
automatically converted to an AVSValue via the constructor
``AVSValue(IClip*)``.
::

    extern "C" __declspec(dllexport) const char* __stdcall AvisynthPluginInit(IScriptEnvironment* env) {

This is the only function which gets exported from the DLL. It is called by
the script function ``LoadPlugin`` the first time this plugin in loaded *in a
particular script*. If several scripts are open at once and more than one of
them loads this plugin, ``AvisynthPluginInit`` may be called more than once
with different IScriptEnvironments. Therefore:

-   You should not save the ``env`` parameter in a global variable.
-   If you need to initialize any static data, you should do it in
    ``DLLMain``, not in this function.

The main purpose of the ``AvisynthPluginInit`` function is to call
``env->AddFunction``.
::

    env->AddFunction("Invert", "c", Create_Invert, 0);

As promised, we now call ``AddFunction`` to let Avisynth know of the
existence of our filter. This function takes four arguments: the name of the
new script function; the parameter-type string; the C++ function implementing
the script function; and the ``user_data`` cookie.

The parameter-type string is similar to the corresponding entity in
VirtualDub, except that:

-   No return type is given. Function return values are not type-checked;
    you can return anything you like.
-   There are more types: along with 'i'nt and 's'tring you can specify
    'b'ool, 'f'loat, and 'c'lip.
-   You can follow any type with '*' or '+' to indicate "zero or more" or
    "one or more" respectively. In this case all the matching arguments will
    be gathered into a sub-array. For example, if your type string is "is+f",
    then the integer argument will be args[0], the string arguments will be
    args[1][0], args[1][1], etc. (and there will be args[1].ArraySize() of
    them), and the float argument will be args[2].
-   '.' matches a *single* argument of any type. To match multiple
    arguments of any type, use ".*" or ".+".
-   You can have named arguments, by specifying the name in [brackets]
    before the type. Named arguments are also optional arguments; if the user
    omits them, they will be of the undefined type instead of the type you
    specify. For convenience, ``AVSValue`` offers a set of ``As...()``
    functions which take default values. See ``avisynth.h``.

::

    return "`Invert' sample plugin";

The return value of ``AvisynthPluginInit`` is a string which can contain any
message you like, such as a notice identifying the version and author of the
plugin. This string becomes the return value of ``LoadPlugin``, and will
almost always be ignored. You can also just return 0 if you prefer.


Variations
~~~~~~~~~~

An in-place filter
^^^^^^^^^^^^^^^^^^

The ``Invert`` filter could easily do its work in a single buffer, rather
than copying from one buffer to another. Here's a new implementation of
``GetFrame`` that does this.
::

    PVideoFrame Invert::GetFrame(int n, IScriptEnvironment* env) {

        PVideoFrame frame = child->GetFrame(n, env);

        env->MakeWritable(&frame);

        unsigned char* p = frame->GetWritePtr();
        const int pitch = frame->GetPitch();
        const int row_size = frame->GetRowSize();
        const int height = frame->GetHeight();

        for (int y = 0; y < height; y++) {
            for (int x = 0; x < row_size; x++)
                p[x] ^= 255;
            p += pitch;
        }

        return frame;
    }

The key difference between this version of the function and the original
version is the presence of the ``MakeWritable`` callback. This is necessary
because this time "we don't know where that frame's been." Someone else in
the filter chain may be holding a reference to it, in which case we won't be
allowed to write to it.

``MakeWritable`` is a simple solution to this problem. It is implemented as
follows (in avisynth.cpp):
::

    bool ScriptEnvironment::MakeWritable(PVideoFrame* pvf) {
      const PVideoFrame& vf = *pvf;

      // If the frame is already writable, do nothing.

      if (vf->IsWritable()) {
        return false;
      }

      // Otherwise, allocate a new frame (using NewVideoFrame) and
      // copy the data into it.  Then modify the passed PVideoFrame
      // to point to the new buffer.

      else {
        const int row_size = vf->GetRowSize();
        const int height = vf->GetHeight();
        PVideoFrame dst = NewVideoFrame(row_size, height);
        BitBlt(dst->GetWritePtr(), dst->GetPitch(), vf->GetReadPtr(),
        vf->GetPitch(), row_size, height);
        *pvf = dst;
        return true;
      }
    }


A filter that changes the frame size
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

In order to effect a change in frame size, two things must happen:

-   your ``GetVideoInfo`` method must return a ``VideoInfo`` struct with
    the appropriate size, and
-   your ``GetFrame`` method must return video frames of the appropriate
    size.

If you derive your filter class from ``GenericVideoFilter``, then a
convenient way to achieve both of these things is to assign the new values to
``vi.width`` and ``vi.height`` in your class constructor. This way you won't
have to override ``GetVideoInfo``, since ``GenericVideoFilter``'s
implementation just returns ``vi``. And if you allocate your output frames
using ``env->NewVideoFrame(vi)``, then they will be of the appropriate size
as well.

For an example of a simple filter which does this, see ``Crop`` or
``StackVertical``.


A filter which processes audio
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Audio processing is handled through the ``GetAudio`` method, which has the
following prototype:
::

    void GetAudio(void* buf, int start, int count, IScriptEnvironment* env);

You must fill in the ``buf``fer with ``count`` samples beginning with sample
number ``start``. A sample may vary from one to four bytes, depending on
whether the audio is 8- or 16-bit and mono or stereo. The flags ``vi.stereo``
and ``vi.sixteen_bit`` will tell you this.

If you cannot do your audio processing in-place, you must allocate your own
buffer for the source audio using ``new`` or ``malloc``.


A filter which rearranges frames
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Many of the built-in filters do nothing more than rearrange video frames (for
example ``Trim``, ``Splice``, ``SelectEvery``, ``Interleave``, ``Reverse``,
and ``ChangeFPS``). If you want to do this, you can write a ``GetFrame``
method like this:
::

    PVideoFrame GetFrame(int n, IScriptEnvironment* env) {
        return child->GetFrame(ConvertFrameNumber(n), env);
    }

But you must also do three other things:

-   Write a companion ``GetParity`` method so that field information is
    preserved; for example, **``bool GetParity(int n) { return
    child->GetParity(ConvertFrameNumber(n)); }``**;
-   Set ``vi.num_frames`` and/or call ``vi.SetFPS`` at instance
    construction time, if you change the frame count or frame rate;
-   Decide what you want to do with the audio track, and write a
    ``GetAudio`` method if necessary.


Other useful methods in IScriptEnvironment
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
::

    __declspec(noreturn) virtual void ThrowError(const char* fmt, ...);

This function throws an exception (of type ``AvisynthError``). Usually, your
error message will end up being displayed on the user's screen in lieu of the
video clip they were expecting.

You can safely call ``ThrowError`` from anywhere except inside ``GetParity``
and ``GetVideoInfo``.

I declared this function as ``__declspec(noreturn)`` to prevent "not all
control paths return a value" warnings. But it didn't work -- I still get the
warnings. Go figure.
::

    virtual long GetCPUFlags();

This is exactly the same as the corresponding VirtualDub function (to the
point of being implemented with code taken directly from VirtualDub -- sorry
Avery ;-) ).

To find out if you're running on a CPU that supports MMX, test
``env->GetCPUFlags() & CPUF_MMX``. There's a complete list of flags in
``avisynth.h``.
::

    virtual char* SaveString(const char* s, int length = -1);

This function copies its argument to a safe "permanent" location and returns
a pointer to the new location.

Each ``ScriptEnvironment`` instance has a buffer set aside for storing
strings, which is expanded as needed. The strings are not deleted until the
``ScriptEnvironment`` instance goes away (when the script file is closed,
usually). This is usually all the permanence that is needed, since all
related filter instances will already be gone by then.

The returned pointer is not const-qualified, and you're welcome to write to
it, as long as you don't stray beyond the bounds of the string.
::

    virtual char* Sprintf(const char* fmt, ...);

    virtual char* VSprintf(const char* fmt, char* val);

These store strings away in the same way as ``SaveString``, but they treat
their arguments like ``printf`` and ``vprintf``.

Currently there's a size limit of 4096 characters on strings created this
way. (The implementation uses ``_vsnprintf``, so you don't need to worry
about buffer overrun.)
::

    virtual AVSValue Invoke(const char* name, const AVSValue args, const char** arg_names=0);

You can use this to call a script function. There are many script functions
which can be useful from other filters; for example, the ``Bob`` filter uses
``SeparateFields``, and several source filters use ``UnalignedSplice``. Some
functions, like ``Weave``, are implemented entirely in terms of other
functions.

If you're calling a function taking exactly one argument, you can simply pass
it in the ``args`` parameter; ``Invoke`` will convert it into an array for
you. In order to call a function taking multiple arguments, you will need to
create the array yourself; it can be done like this:
::

        AVSValue args[5] = { clip, 0, true, 4.7, "my hovercraft is
        full of eels" };
        env->Invoke("Frob", AVSValue(args, 5));


In this case ``Frob`` would need to have a parameter-type string like "cibfs"
or "cfbfs" or "cf.*".

The ``arg_names`` parameter can be used to specify named arguments. Named
arguments can also be given positionally, if you prefer.

``Invoke`` throws ``IScriptEnvironment::NotFound`` if it can't find a
matching function prototype. You should be prepared to catch this unless you
know that the function exists and will accept the given arguments.
::

    virtual void BitBlt(unsigned char* dstp, int dst_pitch, const unsigned char* srcp,
                        int src_pitch, int row_size, int height);

This brilliantly-named function does a line-by-line copy from the source to
the destination. It's useful for quite a number of things; the built-in
filters ``DoubleWeave``, ``FlipVertical``, ``AddBorders``, ``PeculiarBlend``,
``StackVertical``, ``StackHorizontal``, and ``ShowFiveVersions`` all use it
to do their dirty work.
::

    typedef void (__cdecl *ShutdownFunc)(void* user_data, IScriptEnvironment* env);
    virtual void AtExit(ShutdownFunc function, void* user_data);

If you find yourself wanting an ``AvisynthPluginShutdown`` export, this is
the way to get that effect. Functions added through ``AtExit`` are called (in
the opposite order that they were added) when the corresponding
ScriptEnvironment goes away.

--------

.. image:: sig.gif
    :alt: Ben Rudiak-Gould


--------


Notes to Ben's docs
^^^^^^^^^^^^^^^^^^^

Plugin exported function name is replaced from ``AvisynthPluginInit`` for
AviSynth 1.0-2.0 to ``AvisynthPluginInit2`` for AviSynth 2.5. For other
changes see :doc:`AviSynthTwoFiveSDK. <AviSynthTwoFiveSDK>`

For AviSynth 2.5, the converted :doc:`TwoFiveInvert <TwoFiveInvert>` plugin filter source.


Back to :doc:`FilterSDK <FilterSDK>`

$Date: 2006/11/08 20:40:16 $

.. _mirror.: http://www.neuron2.net/www.math.berkeley.edu/benrg/avisynth-extensions.html
