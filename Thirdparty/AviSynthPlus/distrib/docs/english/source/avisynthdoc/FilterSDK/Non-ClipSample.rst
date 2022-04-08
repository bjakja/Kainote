
Non-clip Sample
===============


Non-clip functions in plugins
-----------------------------

Unlike usual clip functions, non-clip functions return not frames of clip but
single (scalar) value as AVSValue (variant of float, integer, boolean,
string).

Numeric function example
------------------------

Here's a sample from script.cpp, edited for external use (by foxyshadis, see `discussion <http://forum.doom9.org/showthread.php?t=132026>`_):
::

     AVSValue Sin(AVSValue args, void* user_data, IScriptEnvironment*
     env) { return sin(args[0].AsFloat()); }

     extern "C" __declspec(dllexport) const char* __stdcall
     AvisynthPluginInit2(IScriptEnvironment* env) {
        env->AddFunction("sin", "f", Sin, 0);
        return "sin";
     }


You don't even need a class/constructor, come to think of it.
Include avisynth.h and that could be a whole plugin right there. (If not
a terribly useful one.)

Clip to float function example
------------------------------

There are Avisynth :ref:`Runtime functions <conditional-runtime-functions>` that evaluated at every frame. They
can be used inside the scripts passed to runtime filters (:doc:`ConditionalFilter, ScriptClip, FrameEvaluate <../corefilters/conditionalfilter>`) to return information for a
frame. Here is sample code of plugin with non-clip function:
::

     /*
        NonClipSample plugin for Avisynth -- a non-clip sample function for Conditional Filter

        Copyright (C) 2007 Alexander Balakhnin 'Fizick' http://avisynth.org.ru

        This program is free software; you can redistribute it and/or modify
        it under the terms of the GNU General Public License as published by
        the Free Software Foundation.

        This program is distributed in the hope that it will be useful,
        but WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
        GNU General Public License for more details.

        You should have received a copy of the GNU General Public License
        along with this program; if not, write to the Free Software
        Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

        V1.0. Based on AverageLuma internal filter and SimpleSample

        Usage:
        ScriptClip(" Subtitle(String(NonClipSample())) ")
     */

     //following 2 includes needed
     #include "windows.h"
     #include "avisynth.h"

     // The function is called for every frame in conditional environment.
     // Unlike usual clip functions, it returns not frames of clip but single (scalar) value
     // as AVSValue (variant of float, integer, boolean, string)

     AVSValue __cdecl NonClipSample(AVSValue args, void* user_data, IScriptEnvironment* env)
     {
        // This sample function calculate mean luma value for currect frame plane

        // Get function paramenters from input args array

                if (!args[0].IsClip()) // check validity
                        env->ThrowError("NonClipSample: No clip supplied!");

                PClip child = args[0].AsClip(); // very first parameter is source clip

        // Get clip video information
                VideoInfo vi = child->GetVideoInfo();

                if (!vi.IsPlanar())
                        env->ThrowError("NonClipSample: Only planar images (as YV12) supported!");

        // get current frame number

                AVSValue cn = env->GetVar("current_frame");
                if (!cn.IsInt())
                        env->ThrowError("NonClipSample: This filter can only be used within Conditional Filter");

                int n = cn.AsInt(); // frame number

        // Get source frame and its properties

                PVideoFrame src = child->GetFrame(n,env); // source frame (smart pointer)
                int plane = PLANAR_Y; // set plane to PLANAR_Y
                const BYTE* srcp = src->GetReadPtr(plane); // pointer to plane data (framebuffer)
                int height = src->GetHeight(plane);
                int width = src->GetRowSize(plane);
                int pitch = src->GetPitch(plane);

        // calculate result of our function
                unsigned int sum = 0; // init sum

                for (int h=0; h < height; h++) {
                        for (int w = 0; w < width; w++) {
                                sum += srcp[w];          // sum each byte from source
                        }
                        srcp = srcp + pitch; // to next line
                }
                float average = (float)sum / (float)(height * width); // normalize sum to get average
        // return float result as AVSValue
                return (AVSValue)average;
     }


     // The following function is the function that actually registers the filter in AviSynth
     // It is called automatically, when the plugin is loaded to see which functions this filter contains.

     extern "C" __declspec(dllexport) const char* __stdcall AvisynthPluginInit2(IScriptEnvironment* env) {
         env->AddFunction("NonClipSample", "c", NonClipSample, 0);
        // The AddFunction has the following paramters:
        // AddFunction(Filtername , Arguments, Function to call,0);

        // Arguments is a string that defines the types and optional names of the arguments for you filter.
        // c - Video Clip
        // i - Integer number
        // f - Float number
        // s - String
        // b - boolean

         // The word inside the [ ] lets you used named parameters in your script

        return "NonClipSample plugin";
        // A freeform name of the plugin.
     }


Compile it as DLL like other AviSynth plugins

Not processing frames plugin with Avisynth AtExit function
----------------------------------------------------------

This plugin does not process frames. It loads a font into Windows and
automatically unloads it after avisynth is done running. To run code at the
end of the script, it registers an ``env->AtExit`` procedure. See
`this discussion <http://forum.doom9.org/showthread.php?t=130383>`_.
::

     /* LoadFont for avisynth

       Created by Shin-san of Ishin Digital Anime Fansubbing
       Special thanks to stickboy, Leak, and IanB of the doom9.org forums for the help
       Special thanks to sh0dan for his simple sample script, which this file is based on

       This code, if made public, is protected by the GPLv3, which can be found at
       www.gnu.org. I'm too lazy to copy/paste it in here

       Purpose: to make it so I can load a font into Windows and automatically unload it
       after avisynth is done running
     */

     #include
     #include "avisynth.h"

     void __cdecl UnLoadFont(void* user_data, IScriptEnvironment* env);

     AVSValue __cdecl Create_LoadFont(AVSValue args, void* user_data, IScriptEnvironment* env) {

       char *file = args[1].AsString("");

       // and now load the font
       if ( (AddFontResource( file )) > 0 )
       {
          SendMessage(HWND_BROADCAST, WM_FONTCHANGE, 0, 0);

          env->AtExit(UnLoadFont, strdup(file) ); // register the procedure
       }
       else
       {
          env->ThrowError("LoadFont: Font load '%s' failed.", file);
       }
       return args[0];
     }

     extern "C" __declspec(dllexport) const char* __stdcall AvisynthPluginInit2(IScriptEnvironment* env)
     {
        env->AddFunction("LoadFont", "c[FONT]s", Create_LoadFont, 0);

        return "'LoadFont' LoadFont plugin";
        // A freeform name of the plugin.
     }

     void __cdecl UnLoadFont(void* user_data, IScriptEnvironment* env)
     {
       char *loadedFont = (char*)user_data;

       if (loadedFont && *loadedFont)
       {
          if ( RemoveFontResource(loadedFont) > 0 )
          {
             SendMessage(HWND_BROADCAST, WM_FONTCHANGE, 0, 0);
          }
          free(loadedFont);
       }
       return;
     }


----

Back to :doc:`FilterSDK <FilterSDK>`

$Date: 2014/11/12 06:57:07 $
