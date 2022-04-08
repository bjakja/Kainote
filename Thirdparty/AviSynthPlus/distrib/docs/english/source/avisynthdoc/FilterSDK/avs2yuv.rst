
avs2yuv
=======

avs2yuv reads a script and outputs raw video (YUV or RGB). It's a
stripped down version of the famous avs2yuv.

Here's avs2yuv.cpp:
::

    #include <stdio.h>
    #include <Windows.h>
    #include "avisynth.h"

    #define MY_VERSION "Avs2YUV 0.24"

    const AVS_Linkage *AVS_linkage = 0;

    int __cdecl main(int argc, const char* argv[])
    {
    const char* infile = NULL;
    const char* outfile = NULL;
    FILE* out_fh;

    if (!strcmp(argv[1], "-h")) {
        fprintf(stderr, MY_VERSION "\n"
            "Usage: avs2yuv.exe in.avs out.raw\n");
        return 2;
    } else {
        infile = argv[1];
        outfile = argv[2];
    }

    try {
        char* colorformat;
        typedef IScriptEnvironment* (__stdcall *DLLFUNC)(int);
        IScriptEnvironment* env;
        HMODULE avsdll = LoadLibrary("avisynth.dll");
        if (!avsdll) {
            fprintf(stderr, "failed to load avisynth.dll\n");
            return 2;
        }

        DLLFUNC CreateEnv = (DLLFUNC)GetProcAddress(avsdll, "CreateScriptEnvironment");
        if (!CreateEnv) {
            fprintf(stderr, "failed to load CreateScriptEnvironment()\n");
            FreeLibrary(avsdll);
            return 1;
        }

        env = CreateEnv(AVISYNTH_INTERFACE_VERSION);

        AVS_linkage = env->GetAVSLinkage();
        AVSValue arg(infile);
        AVSValue res = env->Invoke("Import", AVSValue(&arg, 1));
        if (!res.IsClip()) {
            fprintf(stderr, "Error: '%s' didn't return a video clip.\n", infile);
            FreeLibrary(avsdll);
            return 1;
        }

        PClip clip = res.AsClip();

        if (clip->GetVersion() < 5) {
            fprintf(stderr, "Error: too old version ('%d') of avisynth.dll loaded.\nplease install v2.60 or later.\n",
                clip->GetVersion());
            return 1;
        }

        VideoInfo vi = clip->GetVideoInfo();

        if (!vi.HasVideo()) {
            fprintf(stderr, "Error: '%s' audio only clip.\n", infile);
            FreeLibrary(avsdll);
            return 1;
        }

        fprintf(stderr, " %s:\n", infile);
        fprintf(stderr, " %dx%d,\n", vi.width, vi.height);
        fprintf(stderr, " %d/%d fps,\n", vi.fps_numerator, vi.fps_denominator);
        fprintf(stderr, " %d frames,\n", vi.num_frames);
        if (vi.IsYUV()) {
            colorformat = "YUV";
        } else {
            colorformat = "RGB";
        }
        fprintf(stderr, " %s color format", colorformat);

        out_fh = fopen(outfile, "wb");
        if (!out_fh) {
            fprintf(stderr, "fopen(\"%s\") failed", outfile);
            FreeLibrary(avsdll);
            return 1;
        }

        static const int planes[] = {PLANAR_Y, PLANAR_U, PLANAR_V};

        for (int frm = 0; frm < vi.num_frames; ++frm) {
            PVideoFrame f = clip->GetFrame(frm, env);

            for (int p=0; p<3; p++) { // for interleaved formats only the first plane (being the whole frame) is written
                int height = f->GetHeight(planes[p]);
                int rowsize = f->GetRowSize(planes[p]);
                int pitch = f->GetPitch(planes[p]);
                const BYTE* data = f->GetReadPtr(planes[p]);
                for (int y=0; y<height; y++) {
                    fwrite(data, 1, rowsize, out_fh);
                    data += pitch;
                }
            }
        }

        env->DeleteScriptEnvironment();
        FreeLibrary(avsdll);
        AVS_linkage = 0;

    } catch(AvisynthError err) {
        fprintf(stderr, "\nAvisynth error:\n%s\n", err.msg);
        return 1;
    }

    fclose(out_fh);
    return 0;
    }


Compile this file into an EXE named avs2yuv.exe. See
:doc:`compiling instructions <CompilingAvisynthPlugins>`. Now open the
command line and go to the folder where avs2yuv.exe and your script (called
example.avs here) are located. Our script:
::

    ColorBars()
    ConvertToYV12()
    Trim(0,4)
    Showframenumber()


Type the following on the command line (the name of the output clip can
be arbitrary in our application):
::

    avs2yuv.exe example.avs output.raw


So the output file will contain five frames of YV12 data (640x480). The
raw stream can be played with `YUVtoolkit`_ for example. You can also
import it in AviSynth using the plugin RawSource.

Line by line breakdown
----------------------

Here's a line-by-line breakdown of avs2yuv.cpp.
::

    #include <stdio.h>


The header stdio.h contains objects like `stderr`_ (a pointer to a FILE
object) and functions like `fprintf`_, `fopen`_ and `fwrite`_. Those
will be used later on.

The standard error stream (*stderr*) is the default destination for error
messages and other diagnostic warnings. Like stdout, it is usually also
directed by default to the text console (generally, on the screen).

*fprintf* writes formatted data to stream.

*fopen* opens the file whose name is specified in the parameter filename
and associates it with a stream that can be identified in future
operations by the FILE pointer returned.

*fwrite* writes data to a file which is opened bij *fopen*.
::

    #include <Windows.h>


::

    #include "avisynth.h"


This header declares all the classes and miscellaneous constants that
you might need when accessing avisynth.dll.
::

    #define MY_VERSION "Avs2YUV 0.24"


Defines the version number which will be printed (using the "-h"
option) later on.
::

    const AVS_Linkage *AVS_linkage = 0;


This declares and initializes the server pointers static storage
:doc:`AVS_Linkage <AVSLinkage>`.
::

    int __cdecl main(int argc, const char* argv[])


argv and argc are how command line arguments are passed to main() in C
and C++ (you can name them the way you want to). argc will be the
number of strings pointed to by the array argv. This will be one plus
the number of arguments, with the first one being the name of the
application. Thus when using the command line "avs2yuv.exe in.avs
out.raw" we have argv[0]="avs2yuv.exe", argv[1]="in.avs",
argv[2]="out.raw" and argc=2.
::

    const char* infile = NULL;
    const char* outfile = NULL;


initialize infile and outfile as null pointers by setting them to
`NULL`_. We could have set them to 0 too since that's the same in
C/C++.
::

    FILE* out_fh;


out_fh is declared as a pointer to a `FILE`_ object.
::

    if (!strcmp(argv[1], "-h")) {
        fprintf(stderr, MY_VERSION "\n"
            "Usage: avs2yuv.exe in.avs out.raw\n");
        return 2;


When using the command line "avs2yuv.exe -h" it will print to the
console how the application should be used ('h' from help). The
`return`_ terminates the function main() (and thus the application).
returning 0 means that your program executed without errors and
returning a different int means it executed with errors.

"Avs2YUV 0.24" (followed by an enter) "Usage: avs2yuv.exe in.avs
out.raw" (followed by an enter)
::

    } else {
        infile = argv[1];
        outfile = argv[2];
    }


When the second argument (argv[1]) is not '-h' it will set infile to
the name of the input file (being argv[1]) and outfile to the name of
the output file (being argv[2]).
::

    try {
        char* colorformat;
        IScriptEnvironment* env;


env returns a pointer to the :ref:`IScriptEnvironment <cplusplus_iscriptenvironment>` interface.
::

    HMODULE avsdll = LoadLibrary("avisynth.dll");


`LoadLibrary`_ loads the specified module (which is avisynth.dll here)
into the address space of the process (the process being avs2yuv.exe
here). When successful avsdll will be the handle to the module, else it
will be NULL.
::

    if (!avsdll) {
        fprintf(stderr, "failed to load avisynth.dll\n");
        return 2;
    }


When avsdll is NULL (thus 0), !avsdll evaluates to one, and the error
"failed to load avisynth.dll" is printed to the console.
::

    typedef IScriptEnvironment* (__stdcall *DLLFUNC)(int);
    DLLFUNC CreateEnv = (DLLFUNC)GetProcAddress(avsdll, "CreateScriptEnvironment");


:ref:`CreateScriptEnvironment <cplusplus_createscriptenvironment>` is exported by avisynth.dll and it is a
pointer to the :ref:`IScriptEnvironment <cplusplus_iscriptenvironment>` interface. `GetProcAddress`_
will retrieve the address of the exported function (when failing it
will return NULL).

In order to do so you must declare a function pointer (called 'DLLFUNC'
here) with *exactly* the same prototype as the function it is supposed
to represent. This is done in the first line (note that
:ref:`CreateScriptEnvironment <cplusplus_createscriptenvironment>` has one parameter of type 'int')
::

    typedef IScriptEnvironment* (__stdcall *DLLFUNC)(int);

The `typedef`_ declaration is used to construct shorter or more
meaningful names (like 'DLLFUNC' here) for types that are already
defined (like 'IScriptEnvironment*' here).

In the second line the value of GetProcAddress is cast to the correct
function pointer type.
::

    ... = (DLLFUNC)GetProcAddress(...);


We could also have used
::

    IScriptEnvironment* (__stdcall *CreateEnv)(int) = NULL;
    CreateEnv = (IScriptEnvironment* (__stdcall *)(int))GetProcAddress(avsdll, "CreateScriptEnvironment");


or shorter and less readable
::

    IScriptEnvironment* (__stdcall *CreateEnv)(int) = (IScriptEnvironment* (__stdcall *)(int))GetProcAddress(avsdll, "CreateScriptEnvironment");


::

    if (!CreateEnv) {
        fprintf(stderr, "failed to load CreateScriptEnvironment()\n");
        FreeLibrary(avsdll);
        return 1;
    }


When CreateEnv is NULL (so GetProcAddress failed to retrieve the
exported function) an error is written to the console. `FreeLibrary`_
frees the module from your memory.
::

    env = CreateEnv(AVISYNTH_INTERFACE_VERSION);


This creates the script environment. Its members can be accessed by
:ref:`env->... <cplusplus_iscriptenvironment>`.
::

    AVS_linkage = env->GetAVSLinkage();


This gets the server pointers static storage :doc:`AVS_Linkage <AVSLinkage>`.
::

    AVSValue arg(infile);
    AVSValue res = env->Invoke("Import", AVSValue(&arg, 1));


This calls the `Import`_ function on the input file infile. So the
script is loaded.
::

    if (!res.IsClip()) {
        fprintf(stderr, "Error: '%s' didn't return a video clip.\n", infile);
        FreeLibrary(avsdll);
        return 1;
    }


f the return value of the script is not a clip an error is written to
the console.
::

    PClip clip = res.AsClip();

    if (clip->GetVersion() < 5) {
        fprintf(stderr, "Error: too old version ('%d') of avisynth.dll loaded.\nplease install v2.60 or later.\n",
            clip->GetVersion());
        return 1;
    }


If the loaded avisynth.dll has an api version earlier than 5 an error
is thrown. This is needed if you used functionality which doesn't exist
in older versions. Like using DeleteScriptEnvironment down the road to
delete the script environment (yes it is easy to make it compatible
with older api versions, but this is just for illustration). So it can
be used to force a specific version.
::

    VideoInfo vi = clip->GetVideoInfo();


:ref:`GetVideoInfo <cplusplus_getvideoinfo>` returns a :doc:`VideoInfo <VideoInfo>` structure of the clip.
::

    if (!vi.HasVideo()) {
        fprintf(stderr, "Error: '%s' audio only clip.\n", infile);
        FreeLibrary(avsdll);
        return 1;
    }


Returns an error if the clip doesn't contain video (in case it contains
only audio for example).
::

    fprintf(stderr, " %s:\n", infile);
    fprintf(stderr, " %dx%d,\n", vi.width, vi.height);
    fprintf(stderr, " %d/%d fps,\n", vi.fps_numerator, vi.fps_denominator);
    fprintf(stderr, " %d frames,\n", vi.num_frames);
    if (vi.IsYUV()) {
        colorformat = "YUV";
    } else {
        colorformat = "RGB";
    }
    fprintf(stderr, " %s color format", colorformat);


Some information about the clip is written to the console.
::

    out_fh = fopen(outfile, "wb");


Creates an empty binary file and opens it for writing. It returns a
file pointer called 'out_fh' here. Nb, 'wb' means write mode and
binary.
::

    if (!out_fh) {
        fprintf(stderr, "fopen(\"%s\") failed", outfile);
        FreeLibrary(avsdll);
        return 1;
    }


When failing (thus when out_fh is NULL) an error is written to the
console.
::

    static const int planes[] = {PLANAR_Y, PLANAR_U, PLANAR_V};


All three planes will be processed. For interleaved formats,
automatically, only the first plane (being the whole frame) will be
written to the output file.
::

    for (int frm = 0; frm < vi.num_frames; ++frm) {


Run to all frames in the input file.
::

    PVideoFrame f = clip->GetFrame(frm, env);


Gets frame 'frm'.
::

    for (int p=0; p<3; p++) {
        int height = f->GetHeight(planes[p]);
        int rowsize = f->GetRowSize(planes[p]);
        int pitch = f->GetPitch(planes[p]);
        const BYTE* data = f->GetReadPtr(planes[p]);


Gets the height, rowsize, pitch and a read pointer 'data' to the plane.
See :doc:`InvertNeg <InvertNeg>` for more information.
::

    for (int y=0; y<height; y++) {
        fwrite(data, 1, rowsize, out_fh);


Writes 'rowsize' bytes from the block of memory pointed by 'data' to
the current position in the file pointer 'out_fh'.
::

        data += pitch;

Move the read pointer to the next line.
::

    env->DeleteScriptEnvironment();


When all frames are processed the script environment is deleted.
::

    FreeLibrary(avsdll);


Frees the library (avisynth.dll) from memory.
::

    AVS_linkage = 0;


::

    } catch(AvisynthError err) {
        fprintf(stderr, "\nAvisynth error:\n%s\n", err.msg);
        return 1;


If a runtime error occurs, the `try-catch statement`_ catches the
error, and it is written to the console.
::

    fclose(out_fh);


Closes the file.
::

    return 0;


The application executed succesfully, so we return zero.

| todo - static and dynamic linking (see above) -
| http://msdn.microsoft.com/en-us/library/windows/desktop/ms685090%28v=vs.85%29.aspx
| http://msdn.microsoft.com/en-us/library/d14wsce5.aspx

____

Back to :doc:`FilterSDK`

$Date: 2014/10/27 22:04:54 $

.. _YUVtoolkit: http://www.yuvtoolkit.com/
.. _stderr: http://www.cplusplus.com/reference/cstdio/stderr/
.. _fprintf: http://www.cplusplus.com/reference/cstdio/fprintf/
.. _fopen: http://www.cplusplus.com/reference/cstdio/fopen/
.. _fwrite: http://www.cplusplus.com/reference/cstdio/fwrite/
.. _NULL: http://www.cplusplus.com/reference/cstddef/NULL/
.. _FILE: http://www.cplusplus.com/reference/cstdio/FILE/
.. _return: http://www.cplusplus.com/doc/tutorial/functions/
.. _LoadLibrary: http://msdn.microsoft.com/en-us/library/windows/desktop/ms684175(v=vs.85).aspx
.. _GetProcAddress: http://msdn.microsoft.com/en-us/library/windows/desktop/ms683212(v=vs.85).aspx
.. _typedef: http://www.cplusplus.com/doc/tutorial/other_data_types/
.. _FreeLibrary: http://msdn.microsoft.com/en-us/library/windows/desktop/ms683152(v=vs.85).aspx
.. _Import: http://avisynth.nl/index.php/Import
.. _try-catch statement: http://avisynth.nl/index.php/Control_structures
