
`SimpleSampleClass`_
=====================


:doc:`SimpleSample <SimpleSample>` with an explanation of the used classes and function calls:

::

    /*
        SimpleSample plugin for Avisynth -- a simple sample

        Copyright (C) 2002-2003 Simon Walters, All Rights Reserved

        This program is free software; you can redistribute it and/or
    modify
        it under the terms of the GNU General Public License as published
    by
        the Free Software Foundation.

        This program is distributed in the hope that it will be useful,
        but WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
        GNU General Public License for more details.

        You should have received a copy of the GNU General Public License
        along with this program; if not, write to the Free Software
        Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.


        2005 - trevlac added comments.
    */

    //=============== SECTION #1: Includes and Prototypes =================

    //following two includes are needed
    #include "windows.h"
    #include "avisynth.h"



    //------------ Prototype the functions, ect. So we can call/use them later.

    //--------------------------------------------------------------
    // Prototype AvisynthPluginInit2()
    //
    // Avisynth calls this when you do LoadPlugin("c:\myplugin.dll").
    // All the fancy stuff on the front is for the external call to your dll.
    // Basically you use this to add your function to Avisynth.
    //
    // Parms:
    //          env - This is a pointer back to Avisynth so you can setup your filter.
    //                          It's type is a class defined in avisynth.h.  Basically it lets
    //                          you call functions to set things up.
    //
    // Returns - Looks like it returns a string.  Did you know?
    //
    //---------------------------------------------------------------

    extern "C" __declspec(dllexport) const char* __stdcall
    AvisynthPluginInit2(IScriptEnvironment* env);




    //---------------------------------------------
    // Prototype Create_SimpleSample()
    //
    // Avisynth calls this when a script runs your function.
    //
    // Parms:
    //          args - contains all the arguments passed to the function by the script
    //
    //          user_data - can be used to pass in a chunk of filter info.  This data pointer
    //                                  comes from you when you setup your function in AvisynthPluginInit2()
    //
    //          env - Pointer back to Avisynth so you can call it's functions.
    //
    // Return - AVSValue can take on many types.  It's not quite a void pointer.  It's a class
    //                  defined in the avisynth.h file, but it lets you return anything a script might need.
    //                  After all, this is where you get called and return to a script.
    //
    //---------------------------------------------------------------

    AVSValue __cdecl Create_SimpleSample(AVSValue args, void* user_data,
    IScriptEnvironment* env);




    //---------------------------------------------
    // Prototype SimpleSample Class
    //
    // This is your class.  If you don't know classes, think of it as
    // a structure with function pointers.  There are only two functions
    // here; SimpleSample() (the constructor) and GetFrame().  This is the
    // Prototype so we can use it later.  The details of the functions are not
    // here.  Just enough info to create the class structure.
    //
    // SimpleSample is based on the GenericVideoFilter class which is defined in
    // the avisynth.h file.  GenericVideoFilter is based on another class.  This just
    // means your class 'structure' already comes with a few function pointers.
    // you can then replace the ones you want with your own functionality.  If you
    // don't care about the other ones, you get the built in functionality ... which
    // probably does things like return zero.
    //
    // At the risk of repeating ... we really only care about two functions.
    // SimpleSample() and GetFrame()
    //
    //---------------------------------------------------------------

    class SimpleSample : public GenericVideoFilter {
    public:
        SimpleSample(PClip _child);
        PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);
    };





    //=============== SECTION #2:  The functions Avisynth Calls =================

    //--------------------------------------------------------------
    // AvisynthPluginInit2()
    //
    // Avisynth calls this when you do LoadPlugin("c:\myplugin.dll").
    // All the fancy stuff on the front is for the external call to your dll.
    // Basically you use this to add your function to Avisynth.
    //
    // Parms:
    //          env - This is a pointer back to Avisynth so you can setup your filter.
    //                          It's type is a class defined in avisynth.h.  Basically it lets
    //                          you call functions to set things up.
    //
    // Returns - Looks like it returns a string.  Did you know?
    //
    //---------------------------------------------------------------
    extern "C" __declspec(dllexport) const char* __stdcall
    AvisynthPluginInit2(IScriptEnvironment* env) {

        //--- We use the pointer back to Avisynth to call its 'AddFunction' function.
        //--- Add just one function which a script will call as "SimpleSample",

        //--- takes a single argument which is a video [c]lip
        //--- There are more types: along with 'i'nt and 's'tring you can specify 'b'ool, 'f'loat, and 'c'lip
        //--- Avisynth checks these types and complains to the user if they are not using the right kind.

        //--- Here, Avisynth will call the function in this dll called Create_SimpleSample
        //--- when it 1st encounters SimpleSample in a script.

        //--- That last 0 is where you could pass a pointer to a bunch of data.  The pointer
        //--- would be passed back to you when Avisynth calls Create_SimpleSample.

        env->AddFunction("SimpleSample", "c", Create_SimpleSample, 0);


        //--- Did you know LoadPlugin() returns a string?  Do you think you favorite plugins
        //--- pass back secret messages?  Who checks ?


        return "`SimpleSample' SimpleSample plugin";
    }



    //---------------------------------------------
    // Create_SimpleSample()
    //
    // Avisynth calls this when a script runs your function.
    //
    // Parms:
    //          args - contains all the arguments passed to the function by the script
    //
    //          user_data - can be used to pass in a chunk of filter info.  This data pointer
    //                                  comes from you when you setup your function in AvisynthPluginInit2()
    //
    //          env - Pointer back to avisynth so you can call it's functions.
    //
    // Return - AVSValue can take on many types.  It's not quite a void pointer.  It's a class
    //                  defined in the avisynth.h file, but it lets you return anything a script might need.
    //                  After all, this is where you get called and return to a script.
    //
    //---------------------------------------------------------------
    AVSValue __cdecl Create_SimpleSample(AVSValue args, void* user_data,
    IScriptEnvironment* env) {

        //--- As stated ... this is where we get to if someone puts our function in a script.
        //--- Here we call our class constructor and pass it the first arg as a clip.
        //--- That effectively allocates space in memory for our class structure ... which is a
        //--- glorified structure with a bunch of function pointers.  :)

        //--- Note that a constructor function does not return a type ... but we are returning a pointer
        //--- to our newly allocated class structure.

        return new SimpleSample(args[0].AsClip());
    }






    //---------------------------------------------
    // SimpleSample::SimpleSample()
    //
    // This the class constructor and what gets called when we do 'new SimpleSample()' just above.
    //
    // It's sorta interesting that our function does nothing.  {}  That is because we are based
    // on the GenericVideoFilter class structure, which is based on another class structure.
    // Those guys allocate some space for a few important variables like vi and child.  So if
    // you wonder where the heck these came from ... we got them for free from GenericVideoFilter.
    //
    // Parms:
    //          _child - That's that video clip we got passed from above. We effectively call
    //                          GenericVideoFilter and pass it that same clip.
    //
    // Return - not supposed to. We are a new class structure.
    //
    //---------------------------------------------------------------

    SimpleSample::SimpleSample(PClip _child) : GenericVideoFilter(_child)
    {}



    //---------------------------------------------
    // SimpleSample::GetFrame()
    //
    // What we've all been waiting for!  All the frames come here.  The other junk just sets things up.
    // Avisynth has a reference to our class structure (aka we are in the filter chain)
    // Our class is based on a GenericVideoFilter, which has a dummy GetFrame() function.
    // We defined our own GetFrame() so Avisynth calls here.
    //
    // Parms:
    //          n - The frame we are supposed to return.
    //
    //          env - Pointer back to avisynth so you can call it's functions.
    //
    // Return - PVideoFrame - This is a pointer to a class structure that effectively holds the
    //                  frame you were messing with.
    //
    //---------------------------------------------------------------

    PVideoFrame __stdcall SimpleSample::GetFrame(int n,
    IScriptEnvironment* env) {


        //--- That mysterious vi is there from GenericVideoFilter.
        //--- It is a structure of info about the frame which is filled by Avisynth.
        //--- If you use VC++, add avisynth.h to your project and look in classView for the details.
        //--- Below ... we report an error back to Avisynth ... we only do YUY2 data.

        if (vi.IsYV12()) // is input not YUY2
                env->ThrowError("SimpleSample: input to filter must be in YUY2");


        //--- That mysterious child is there from GenericVideoFilter.
        //--- It is a clip variable which is filled by Avisynth.
        //--- We need to produce frame n, so we get that from the child clip.
        //--- That's our input frame src.
        //--- We need a place to build our output frame before we pass it back.
        //--- Avisynth has a handy built in method.  Just call it and tell it what type of frame
        //--- we want ... and we get one we can muck with.  vi defines the input type.  If we want another
        //--- type like RGB at 1/2 the size, we make a different videoInfo structure and
        //--- use that instead to create our output.

        PVideoFrame src = child->GetFrame(n, env);
        PVideoFrame dst = env->NewVideoFrame(vi);


        //--- Gotta do this to read and write from/to the source and destination.  src and dst are classes.
        //--- We can't just change their data.  We need to call their functions to change them.  The ones
        //--- below give us pointers to their main pixel data.

        const unsigned char* srcp = src->GetReadPtr();
        unsigned char* dstp = dst->GetWritePtr();


        //--- We want to know the width (in bytes) and height (in pixels) of the source and dest frames so
        //--- we can loop thru their pixels.  Pitch is the number of bytes from the start of one
        //--- image row to the start of the next.  This may be different than width because there
        //--- may be space at the end of each row that pads for better memory alignment.

        const int dst_pitch = dst->GetPitch();
        const int dst_width = dst->GetRowSize();
        const int dst_height = dst->GetHeight();
        const int src_pitch = src->GetPitch();
        const int src_width = src->GetRowSize();
        const int src_height = src->GetHeight();



        //--- FINALLY !!!! --------
        //--- We loop through the bytes and copy from source to destination.

        for (int h=0; h < src_height;h++) {
                for (int w = 0; w < src_width; w++) {

                        //--- Put your code here to muck with the frame
                        *(dstp + w) = *(srcp + w);
                }

                //--- This moves us to the next line
                srcp = srcp + src_pitch;
                dstp = dstp + dst_pitch;
        }

        return dst; //-- return the frame


Back to :doc:`SimpleSample <SimpleSample>`

$Date: 2010/03/13 14:52:05 $

.. _SimpleSampleClass : http://www.avisynth.org/SimpleSample
