
InvertNeg
=========

I'll start off with a complete, working Avisynth plugin. It's called
"InvertNeg", and it produces a photo-negative of the input clip.

Here's InvertNeg.cpp:
::

    #include <windows.h>
    #include "avisynth.h"

    class InvertNeg : public GenericVideoFilter {
    public:
        InvertNeg(PClip _child, IScriptEnvironment* env);
        PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);
    };

    InvertNeg::InvertNeg(PClip _child, IScriptEnvironment* env) :
        GenericVideoFilter(_child) {
        if (!vi.IsPlanar() || !vi.IsYUV()) {
            env->ThrowError("InvertNeg: planar YUV data only!");
        }
    }

    PVideoFrame __stdcall InvertNeg::GetFrame(int n, IScriptEnvironment* env) {

        PVideoFrame src = child->GetFrame(n, env);
        PVideoFrame dst = env->NewVideoFrame(vi);

        const unsigned char* srcp;
        unsigned char* dstp;
        int src_pitch, dst_pitch, row_size, height;
        int p, x, y;

        int planes[] = {PLANAR_Y, PLANAR_V, PLANAR_U};

        for (p=0; p<3; p++) {
            srcp = src->GetReadPtr(planes[p]);
            dstp = dst->GetWritePtr(planes[p]);

            src_pitch = src->GetPitch(planes[p]);
            dst_pitch = dst->GetPitch(planes[p]);
            row_size = dst->GetRowSize(planes[p]);
            height = dst->GetHeight(planes[p]);

            for (y = 0; y < height; y++) {
                for (x = 0; x < row_size; x++) {
                    dstp[x] = srcp[x] ^ 255;
                }
                srcp += src_pitch; // or srcp = srcp + src_pitch;
                dstp += dst_pitch; // or dstp = dstp + dst_pitch;
            }
        }
        return dst;
    }

    AVSValue __cdecl Create_InvertNeg(AVSValue args, void* user_data, IScriptEnvironment* env) {
        return new InvertNeg(args[0].AsClip(), env);
    }

    const AVS_Linkage *AVS_linkage = 0;

    extern "C" __declspec(dllexport) const char* __stdcall AvisynthPluginInit3(IScriptEnvironment* env, const AVS_Linkage* const vectors) {
        AVS_linkage = vectors;
        env->AddFunction("InvertNeg", "c", Create_InvertNeg, 0);
        return "InvertNeg sample plugin";
    }


Compile this file into a DLL named InvertNeg.dll. See
:doc:`compiling instructions <CompilingAvisynthPlugins>`. Now create an
Avisynth script which looks something like this:
::

    LoadPlugin("d:\path\InvertNeg.dll")
    clip = BlankClip().ConvertToYV12()
    return clip.InvertNeg()

Line by line breakdown
----------------------

Here's a line-by-line breakdown of InvertNeg.cpp.
::

    #include "avisynth.h"


This header declares all the classes and miscellaneous constants that
you might need when writing a plugin. All external plugins should
#include it.

External plugins do not link with avisynth.dll, so they can't directly
access functions that are defined in the main Avisynth source code.
Therefore, every important function in avisynth.h is either defined
inline or declared as virtual. The virtual functions act as callbacks
for external DLLs.
::

    class InvertNeg : public GenericVideoFilter {


An Avisynth filter is simply a C++ class implementing the :ref:`IClip <cplusplus_iclip>`
interface. More about classes can be found in this `tutorial`_. IClip
has four pure virtual methods: :ref:`GetVideoInfo <cplusplus_getvideoinfo>`,
:ref:`GetFrame <cplusplus_getframe>`, :ref:`GetParity <cplusplus_getparity>`,
and :ref:`GetAudio <cplusplus_getaudio>`.

The class GenericVideoFilter is a simple do-nothing filter defined in
avisynth.h. It derives from IClip and implements all four methods. Most
filters can inherit from GenericVideoFilter rather than directly from
IClip; this saves you from having to implement methods that you don't
care about, like GetAudio in this example. More about inheritance can
be found in this `tutorial <http://www.cplusplus.com/doc/tutorial/inheritance>`__. In our case two functions are defined in
the class: the constructor (InvertNeg()) and GetFrame.
::

    InvertNeg(PClip _child, IScriptEnvironment* env);


This is our constructor. It can be used to initialize variables, create
look up tables (LUTs) and to check whether the source clips (if there
is one) have the desired properties. In our example it is taking a
single clip, does some property checking and then simply passes through
to its output.

A PClip is a "smart pointer" to an IClip. It maintains a reference
count on the IClip object and automagically deletes it when the last
PClip referencing it goes away. For obvious reasons, you should always
use PClip rather than IClip* to refer to clips.

Like a genuine pointer, a PClip is only four bytes long, so you can
pass it around by value. Also like a pointer, a PClip can be assigned a
null value (0), which is often useful as a sentinel. Unlike a pointer,
PClip is initialized to 0 by default.

You'll need to make sure your class doesn't contain any circular PClip
references, or any PClips sitting in dynamically allocated memory that
you forget to delete. Other than that, you don't have to worry about
the reference-counting machinery.

Avisynth filters have a standardized output channel via IClip, but
(unlike for example `VirtualDub`_ filters) no standardized input
channel. Each filter is responsible for obtaining its own source
material -- usually (as in this case) from another clip, but sometimes
from several different clips, or from a file.

We will override the GetFrame method to do something more useful, while
leaving the other three methods as-is to pass through aspects of the
clip that we don't need to change.
::

    PVideoFrame InvertNeg::GetFrame(int n, IScriptEnvironment* env) {


This method is called to make our filter produce frame n of its output.
The second argument, env, is for our purposes simply a callback suite.
It is actually implemented in Avisynth by a class called ScriptEnvironment.
One :ref:`instance of this class <cplusplus_iscriptenvironment>` is created for each
opened AVS script, so there may sometimes be several instances active
at once. It is important that the callback functions be called through
the proper instance. A particular instance of your class will only be
used in one ScriptEnvironment, but different instances might be used in
different ScriptEnvironments.
::

    InvertNeg::InvertNeg(PClip _child, IScriptEnvironment* env) :
        GenericVideoFilter(_child) {


This is the class constructor and it is called when a filter instance
is being created (see Create_InvertNeg()).
::

    if (!vi.IsPlanar() || !vi.IsYUV()) {
        env->ThrowError("InvertNeg: planar YUV data only!");
    }


"vi" is a protected member of GenericVideoFilter. It is a structure of
type :doc:`VideoInfo <VideoInfo>`, and it contains information about the clip (like
frame size, frame rate, pixel format, audio sample rate, etc.). In our
example, when the image format of the clip is not planar or the
colorspace of the clip is not YUV, an error is returned using
:ref:`ThrowError <cplusplus_throwerror>`. As of writing this document there is no planar RGB, but
that might very well change in the future.
::

    PVideoFrame src = child->GetFrame(n, env);


This method returns a PVideoFrame, which is a smart pointer like PClip.

"child" is the other protected member of GenericVideoFilter, of type
PClip. It contains the clip that was passed to the constructor. For our
filter to produce frame n we need the corresponding frame of the input.
If you need a different frame from the input, all you have to do is
pass a different frame number to child->GetFrame.

GetFrame calls are usually intercepted by Avisynth's internal caching
code, so the frame request may never actually reach the child filter.
::

    PVideoFrame dst = env->NewVideoFrame(vi);


The NewVideoFrame callback allocates space for a video frame of the
supplied size. In this case it will hold our filter's output. The frame
buffer is uninitialized raw memory (except that in the debug build it
gets filled with the repeating byte pattern 0A 11 0C A7 ED, which is
easy to recognize because it looks like "ALLOCATED").

As already explained, "vi" contains information about the properties of
the clip. NewVideoFrame uses the information in this structure to
return a frame buffer of the appropriate size.

Frame buffers are reused once all the PVideoFrame references to them go
away. So usually NewVideoFrame won't actually need to allocate any
memory from the heap.
::

    const unsigned char* srcp;
    unsigned char* dstp;
    int src_pitch, dst_pitch, row_size, height;
    int p, x, y;

    int planes[] = {PLANAR_Y, PLANAR_V, PLANAR_U};


This is the declaration of the used variables. Note that we will modify
one color component (instead of an entire pixel) at the same time. Such
a component (for example an U sample) has a value between 0 and 255
(exactly one byte). Hence srcp and dstp needs to be declared as
unsigned char (or BYTE would have been possible too).

Note that in the constructor we excluded the formats which are not
planar or not YUV. For filter writers this means that they can write
one simple function that is called three times, one for each color
channel, assuming that the operations are channel-independent (which is
not always the case). This is what I have done here. Note it also works
for Y8 since these functions return zero if the plane (PLANAR_U,
PLANAR_V) doesn't exist. In our case the operations are
channel-independent so it doesn't matter which plane is processed
first.
::

    for (p=0; p<3; p++) {
        srcp = src->GetReadPtr(planes[p]);
        dstp = dst->GetWritePtr(planes[p]);


All frame buffers are readable, but not all are writable.

The rule about writability is this: A buffer is writable if and only if
there is exactly one PVideoFrame pointing to it. In other words, you
can only write to a buffer if no one else might be reading it. This
rule guarantees that as long as you hold on to a PVideoFrame and don't
write to it yourself, that frame will remain unchanged. The only
drawback is that you can't have two PVideoFrames pointing to a writable
buffer. This can sometimes be an inconvenience, as I'll explain later.

Any buffer you get from NewVideoFrame is guaranteed to be writable (as
long as you only assign it to one PVideoFrame!). Our filter's dst came
from NewVideoFrame, so we can safely call dst->GetWritePtr(). However,
frames you get from other clips via GetFrame may not be writable, in
which case GetWritePtr() will return a null pointer.

There is an :ref:`IsWritable() <cplusplus_iswritable>` method which you can call to find out if a
buffer is writable or not, and there's a :ref:`MakeWritable <cplusplus_makewritable>` callback to
ensure that it is.
::

    src_pitch = src->GetPitch(planes[p]);
    dst_pitch = dst->GetPitch(planes[p]);


Just as in VirtualDub, the "pitch" of a frame buffer is the offset (in
bytes) from the beginning of one scan line to the beginning of the
next. The source and destination buffers won't necessarily have the
same pitch.

Buffers created by NewVideoFrame are always quadword (8-byte) aligned
and always have a pitch that is a multiple of 8. (*i think it's 16 now,
need to check ???*)
::

    row_size = dst->GetRowSize(planes[p]);


The row size is the length of each row in bytes (not pixels) of plane
p. It's usually equal to the pitch or slightly less, but it may be
significantly less if the frame in question has been through Crop.

Since our source and destination frames have the same width and pixel
format, they will always have the same row size. Thus I only need one
row_size variable, and I could just as well have called
src->GetRowSize().
::

    height = dst->GetHeight(planes[p]);


The height is the height in samples of the plane p. Again, for our
filter this is the same for the source and the destination.
::

    for (y = 0; y < height; y++) {
        for (x = 0; x < row_size; x++) {
            dstp[x] = srcp[x] ^ 255;
        }
        srcp += src_pitch;
        dstp += dst_pitch;
    }


This is the code that does the actual work. The "srcp += src_pitch;
dstp += dst_pitch;" idiom is a useful way of dealing with potentially
differing pitches without too much grief.
::

    return dst;


GetFrame returns the newly-created frame. Our own references to this
frame and to the source frame will go away with the src and dst
variables. Our caller will become sole owner of the destination frame
(which therefore will still be writable), and the source frame will be
retained in the cache and eventually recycled. All through the magic of
C++ classes.
::

    AVSValue __cdecl Create_InvertNeg(AVSValue args, void* user_data, IScriptEnvironment* env) {


In order to use our new filter, we need a scripting-language function
which creates an instance of it. This is that function.

Script functions written in C++ take three arguments. args contains all
the arguments passed to the function by the script. user_data contains
the void pointer which you passed to AddFunction (see below). Usually
you won't need this. env contains the same IScriptEnvironment pointer
that will later be passed to GetFrame.

AVSValue is a variant type which can hold any one of the following: a
boolean value (true/false); an integer; a floating-point number; a
string; a video clip (PClip); an array of AVSValues; or nothing
("undefined"). You can test which one it is with the methods IsBool(),
IsInt(), IsFloat(), IsString(), IsClip(), IsArray(), and Defined()
(which returns true if the AVSValue is not "undefined"). You can get
the value with AsBool(), AsInt(), etc. For arrays, you can use the
ArraySize() method to get the number of elements, and [] indexing to
get the elements themselves. For convenience, IsFloat() and AsFloat()
will work with integers also. But boolean values are not treated as
numeric (unlike C).

The name "Create_InvertNeg" is arbitrary. This function will actually
be known as "InvertNeg" in scripts, because that's the name we pass to
:ref:`AddFunction <cplusplus_addfunction>` below.
::

    return new InvertNeg(args[0].AsClip(), env);


The args argument passed to a script function will always be an array.
The return value should be any one of the other types (never an array).

The types of the values in the args array are guaranteed to match one
of the function signatures that you pass to AddFunction, just as in
VirtualDub. Therefore, there's no need to worry about IsClip here.

Create_InvertNeg simply creates and returns a filter instance; it is
automatically converted to an AVSValue via the constructor
AVSValue(IClip*).
::

    const AVS_Linkage *AVS_linkage = 0;


This declares and initializes the server pointers static storage
:doc:`AVS_Linkage <AVSLinkage>`.
::

    extern "C" __declspec(dllexport) const char* __stdcall AvisynthPluginInit3(IScriptEnvironment* env, const AVS_Linkage* const vectors) {


This is the only function which gets exported from the DLL. It is
called by the script function LoadPlugin the first time this plugin in
loaded in a particular script. If several scripts are open at once and
more than one of them loads this plugin, AvisynthPluginInit3 may be
called more than once with different IScriptEnvironments. Therefore:

* You should not save the env parameter in a global variable.
* If you need to initialize any static data, you should do it in
  DLLMain, not in this function.

::

    AVS_linkage = vectors;


This saves the server pointers.

The main purpose of the AvisynthPluginInit3 function is to call
env->AddFunction.
::

    env->AddFunction("InvertNeg", "c", Create_InvertNeg, 0);


As promised, we now call AddFunction to let Avisynth know of the
existence of our filter. This function takes four arguments: the name
of the new script function; the parameter-type string; the C++ function
implementing the script function; and the user_data cookie.

The parameter-type string can be specified as follows:

* No return type is given. Function return values are not
  type-checked; you can return anything you like.
* There are more types: along with 'i'nt and 's'tring you can specify
  'b'ool, 'f'loat, and 'c'lip.
* The following types are available:
    + c - clip
    + i - integer
    + f - float
    + s - string
    + b - boolean
* You can follow any type with '*' or '+' to indicate "zero or more"
  or "one or more" respectively. In this case all the matching
  arguments will be gathered into a sub-array.

  * For example, if your type string is "is+f", then the
    integer argument will be args[0], the string arguments
    will be args[1][0], args[1][1], etc. (and there will be
    args[1].ArraySize() of them), and the float argument will
    be args[2].

* '.' matches a single argument of any type. To match multiple
  arguments of any type, use ".*" or ".+".
* You can have named arguments, by specifying the name in [brackets]
  before the type. Named arguments are also optional arguments; if
  the user omits them, they will be of the undefined type instead of
  the type you specify. For convenience, AVSValue offers a set of
  As...() functions which take default values. See avisynth.h.

::

    return "InvertNeg sample plugin";


The return value of AvisynthPluginInit3 is a string which can contain
any message you like, such as a notice identifying the version and
author of the plugin. This string becomes the return value of
LoadPlugin, and will almost always be ignored. You can also just return
0 if you prefer.


A variation as an in place filter
---------------------------------

The Invert filter could easily do its work in a single buffer, rather
than copying from one buffer to another. Here's a new implementation of
GetFrame that does this.
::

    #include <windows.h>
    #include "avisynth.h"

    class InvertNeg : public GenericVideoFilter {
    public:
        InvertNeg(PClip _child, IScriptEnvironment* env);
        PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);
    };

    InvertNeg::InvertNeg(PClip _child, IScriptEnvironment* env) :
        GenericVideoFilter(_child) {
        if (!vi.IsPlanar() || !vi.IsYUV()) {
            env->ThrowError("InvertNeg: planar YUV data only!");
        }
    }

    PVideoFrame __stdcall InvertNeg::GetFrame(int n, IScriptEnvironment* env) {

        PVideoFrame src = child->GetFrame(n, env);
        env->MakeWritable(&src);

        unsigned char* srcp;
        int src_pitch, row_size, height;
        int p, x, y;

        int planes[] = {PLANAR_Y, PLANAR_V, PLANAR_U};

        for (p=0; p<3; p++) {
            srcp = src->GetWritePtr(planes[p]);

            src_pitch = src->GetPitch(planes[p]);
            row_size = src->GetRowSize(planes[p]);
            height = src->GetHeight(planes[p]);

            for (y = 0; y < height; y++) {
                for (x = 0; x < row_size; x++) {
                    srcp[x] = srcp[x] ^ 255;
                    // even shorter would be srcp[x] ^= 255;
                }
                srcp += src_pitch;
            }
        }
        return src;
    }

    AVSValue __cdecl Create_InvertNeg(AVSValue args, void* user_data, IScriptEnvironment* env) {
        return new InvertNeg(args[0].AsClip(), env);
    }

    const AVS_Linkage *AVS_linkage = 0;

    extern "C" __declspec(dllexport) const char* __stdcall AvisynthPluginInit3(IScriptEnvironment* env, const AVS_Linkage* const vectors) {
        AVS_linkage = vectors;
        env->AddFunction("InvertNeg", "c", Create_InvertNeg, 0);
        return "InvertNeg sample plugin";
    }


The key difference between this version of the function and the
original version is the presence of the :ref:`MakeWritable <cplusplus_makewritable>` callback. This
is necessary because this time "we don't know where that source frame
has been." Someone else in the filter chain may be holding a reference
to it, in which case we won't be allowed to write to it.

Old versions

:doc:`Ben's AviSynth Docs <BensAviSynthDocs>` is the documentation written for AviSynth 1.0
by Ben Rudiak-Gould, in its original form.

See more about the modifications for AviSynth 2.5 in the :doc:`AviSynth
Two-Five SDK <AviSynthTwoFiveSDK>`. (need to adjust the two-five version using the code above
...)

____

Back to :doc:`FilterSDK`

$Date: 2015/09/14 20:23:59 $

.. _tutorial: http://www.cplusplus.com/doc/tutorial/classes
.. _VirtualDub: http://avisynth.nl/index.php/VirtualDub
