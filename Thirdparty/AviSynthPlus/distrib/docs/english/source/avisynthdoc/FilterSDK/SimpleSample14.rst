
`SimpleSample 1.4`_
===================

`SimpleSample 1.4: <http://www.geocities.com/siwalters_uk/simplesample14.zip>`_

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

        The author can be contacted at:
        Simon Walters
        siwalters@hotmail.com

        V1.0 - 1st release.
        V1.0a - Revised version to put colourspace checking in the right
    place.
      V1.0b - Added detailed comments.  // sh0dan
        V1.1 - Added sh0dan's planar (YV12) code
        V1.2 - Revert to RGB24 colourspace only to show simple pixel
    manipulation;
        V1.3 - add in a parameter to vary the size of the square.
        V1.4 - Add in RGB32 colourspace code

    */
    //following 2 includes needed
    #include "windows.h"
    #include "avisynth.h"

    /****************************
     * The following is the header definitions.
     * For larger projects, move this into a .h file
     * that can be included.
     ****************************/


    class SimpleSample : public GenericVideoFilter {
      // SimpleSample defines the name of your filter class.
      // This name is only used internally, and does not affect the name of your filter or similar.
      // This filter extends GenericVideoFilter, which incorporates basic functionality.
      // All functions present in the filter must also be present here.
        int SquareSize;

    public:
      // This defines that these functions are present in your class.
      // These functions must be that same as those actually implemented.
      // Since the functions are "public" they are accessible to other classes.
      // Otherwise they can only be called from functions within the class itself.

        SimpleSample(PClip _child, int _SquareSize, IScriptEnvironment* env);
      // This is the constructor. It does not return any value, and is always used,
      //  when an instance of the class is created.
      // Since there is no code in this, this is the definition.

      ~SimpleSample();
      // The is the destructor definition. This is called when the filter is destroyed.


        PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);
      // This is the function that AviSynth calls to get a given frame.
      // So when this functions gets called, the filter is supposed to return frame n.
    };

    /***************************
     * The following is the implementation
     * of the defined functions.
     ***************************/

    //Here is the acutal constructor code used
    SimpleSample::SimpleSample(PClip _child, int _SquareSize,
    IScriptEnvironment* env) :
        GenericVideoFilter(_child), SquareSize(_SquareSize) {
      // This is the implementation of the constructor.
      // The child clip (source clip) is inherited by the GenericVideoFilter,
      //  where the following variables gets defined:
      //   PClip child;   // Contains the source clip.
      //   VideoInfo vi;  // Contains videoinfo on the source clip.
        if (!vi.IsRGB()) // is input not RGB (24 or 32)
                env->ThrowError("SimpleSample: input to filter must be in an RGB colourspace");
    }

    // This is where any actual destructor code used goes
    SimpleSample::~SimpleSample() {
      // This is where you can deallocate any memory you might have used.
    }




    PVideoFrame __stdcall SimpleSample::GetFrame(int n,
    IScriptEnvironment* env) {
    // This is the implementation of the GetFrame function.
    // See the header definition for further info.

        PVideoFrame src = child->GetFrame(n, env);
       // Request frame 'n' from the child (source) clip.
        PVideoFrame dst = env->NewVideoFrame(vi);
       // Construct a frame based on the information of the current frame
       // contained in the "vi" struct.

        const unsigned char* srcp = src->GetReadPtr();
      // Request a Read pointer from the source frame.
      // This will return the position of the upperleft pixel in YUY2 images,
      // and return the lower-left pixel in RGB.
      // RGB images are stored upside-down in memory.
      // You should still process images from line 0 to height.

        unsigned char* dstp = dst->GetWritePtr();
        // Request a Write pointer from the newly created destination image.
      // You can request a writepointer to images that have just been
      // created by NewVideoFrame. If you recieve a frame from
      PClip->GetFrame(...)
      // you must call env->MakeWritable(&frame) be recieve a valid write pointer.

        const int dst_pitch = dst->GetPitch();
      // Requests pitch (length of a line) of the destination image.
      // For more information on pitch see:
      // http://www.avisynth.org/index.php?page=WorkingWithImages
        // (short version - pitch is always equal to or greater than width to allow for seriously fast assembly code)

        const int dst_width = dst->GetRowSize();
      // Requests rowsize (number of used bytes in a line.
      // See the link above for more information.

        const int dst_height = dst->GetHeight();
      // Requests the height of the destination image.

        const int src_pitch = src->GetPitch();
        const int src_width = src->GetRowSize();
        const int src_height = src->GetHeight();

        int w, h;

        // This version of SimpleSample is intended to show some simple pixel manipulation.
        // It is designed purely for clarity and not as good or clever code :-)



        if (vi.IsRGB24()) {
                // The code just deals with RGB24 colourspace where each pixel is represented by
                // 3 bytes, Blue, Green and Red.
                // Although this colourspace is the easiest to understand, it is very rarely used because
                // a 3 byte sequence (24bits) cannot be processed easily using normal 32 bit registers.

                for (h=0; h < src_height;h++) {                       // Loop from bottom line to top line.
                        for (w = 0; w < src_width; w+=3) {            // Loop from left side of the image to the right side 1 pixel (3 bytes) at a time
                                                                      // stepping 3 bytes (a pixel width in RGB24 space)

                                *(dstp + w) = *(srcp + w);            // Copy each Blue byte from source to destination.
                                *(dstp + w + 1) = *(srcp + w + 1);    // Copy Green.
                                *(dstp + w + 2) = *(srcp + w + 2);    // Copy Red
                        }

                        srcp = srcp + src_pitch; // Add the pitch (note use of pitch and not width) of one line (in bytes) to the source pointer
                        dstp = dstp + dst_pitch; // Add the pitch to the destination pointer.
                }
                // end copy src to dst

                //Now draw a white square in the middle of the frame
                // Normally you'd do this code within the loop above but here it is in a separate loop for clarity;

                dstp = dst->GetWritePtr();                                                               // reset the destination pointer to the bottom, left pixel. (RGB colourspaces only)
                dstp = dstp + (dst_height/2 - SquareSize/2)*dst_pitch;                                   // move pointer to SquareSize/2 lines from the middle of the frame;
                for (h=0; h < SquareSize;h++) {                                                          // only scan 100 lines
                        for (w = dst_width/2 - SquareSize*3/2; w < dst_width/2 + SquareSize*3/2; w+=3) { // only scans the middle SquareSize pixels of a line
                                *(dstp + w) = 255;                                                       // Set Blue to maximum value.
                                *(dstp + w + 1) = 255;                                                   // and Green.
                                *(dstp + w + 2) = 255;                                                   // and Red - therefore the whole pixel is now white.
                        }
                        dstp = dstp + dst_pitch;
                }
        }

        if (vi.IsRGB32()) {
                // This code deals with RGB32 colourspace where each pixel is represented by
                // 4 bytes, Blue, Green and Red and "spare" byte that could/should be used for alpha
                // keying but usually isn't.

                // Although this colourspace isn't memory efficient, code end ups running much
                // quicker than RGB24 as you can deal with whole 32bit variables at a time
                // and easily work directly and quickly in assembler (if you know how to that is :-)

                for (h=0; h < src_height;h++) {                                                 // Loop from bottom line to top line.
                        for (w = 0; w < src_width/4; w+=1) {                                    // and from leftmost pixel to rightmost one.
                                *((unsigned int *)dstp + w) = *((unsigned int *)srcp + w);      // Copy each whole pixel from source to destination.
                        }                                                                       // by temporarily treating the src and dst pointers as
                                                                                                // pixel pointers intead of byte pointers
                        srcp = srcp + src_pitch; // Add the pitch (note use of pitch and not width) of one line (in bytes) to the source pointer
                        dstp = dstp + dst_pitch; // Add the pitch to the destination pointer.
                }
                // end copy src to dst

                //Now draw a white square in the middle of the frame
                // Normally you'd do this code within the loop above but here it is in a separate loop for clarity;

                dstp = dst->GetWritePtr();                                // reset the destination pointer to the bottom, left pixel. (RGB colourspaces only)
                dstp = dstp + (dst_height/2 - SquareSize/2)*dst_pitch;    // move pointer to SquareSize/2 lines from the middle of the frame;

                int woffset = dst_width/8 - SquareSize/2;  // lets precalulate the width offset like we do for the lines.

                for (h=0; h < SquareSize;h++) {                                     // only scan SquareSize number of lines
                        for (w = 0; w < SquareSize; w+=1) {                         // only scans the middle SquareSize pixels of a line
                                *((unsigned int *)dstp + woffset + w) = 0x00FFFFFF; // Set Red,Green and Blue to maximum value in 1 instruction.
                        }
                        dstp = dstp + dst_pitch;
                }
      }

      // As we now are finished processing the image, we return the destination image.
        return dst;
    }


    // This is the function that created the filter, when the filter has been called.
    // This can be used for simple parameter checking, so it is possible to create different filters,
    // based on the arguments recieved.

    AVSValue __cdecl Create_SimpleSample(AVSValue args, void* user_data,
    IScriptEnvironment* env) {
        return new SimpleSample(args[0].AsClip(),
                 args[1].AsInt(0),      // Corresponds to our 1st parameter - the size of the square in pixels.
                 env);                  // Calls the constructor with the arguments provied.
    }


    // The following function is the function that actually registers the filter in AviSynth
    // It is called automatically, when the plugin is loaded to see which functions this filter contains.

    extern "C" __declspec(dllexport) const char* __stdcall
    AvisynthPluginInit2(IScriptEnvironment* env) {
        env->AddFunction("SimpleSample", "c[SIZE]i",
        Create_SimpleSample, 0);
        // The AddFunction has the following paramters:
        // AddFunction(Filtername , Arguments, Function to call,0);

        // Arguments is a string that defines the types and optional names of the arguments for you filter.
        // c - Video Clip
        // i - Integer number
        // f - Float number
        // s - String
        // b - boolean

         // The word inside the [ ] lets you used named parameters in your script
         // e.g last=SimpleSample(last,size=100).
         // but last=SimpleSample(last,100) will also work automagically

        return "`SimpleSample' SimpleSample plugin";
        // A freeform name of the plugin.
    }

Back to :doc:`SimpleSample <SimpleSample>`

$Date: 2006/10/28 20:18:15 $

.. _SimpleSample 1.4: http://www.avisynth.org/SimpleSample+1.4
