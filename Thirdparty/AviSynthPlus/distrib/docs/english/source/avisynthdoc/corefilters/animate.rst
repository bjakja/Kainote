
Animate / ApplyRange
====================

``Animate`` (clip, int start_frame, int end_frame, string filtername,
start_args, end_args)
``ApplyRange`` (clip, int start_frame, int end_frame, string filtername,
args)

``Animate`` is a meta-filter which evaluates its parameter filter with
continuously varying arguments. At frame start_frame and earlier, the filter
is evaluated with the arguments given by start_args. At frame end_frame and
later, the filter is evaluated with the arguments given by end_args. In
between, the arguments are linearly interpolated for a smooth transition.

``ApplyRange`` is a special case of ``Animate`` where start_args = end_args,
and present in *v2.51*. It can be used when you want to apply a certain
filter only on a certain range of frames of a clip - unlike Animate, frames
outside the range start_frame to end_frame are passed through untouched.
Another difference with ``Animate`` is that args can't contain a clip.
Starting from *v2.53* it supports audio, and start_frame can be equal to
end_frame (such that only one frame is processed).

In cases where a large number of ranges need similar processing using many
``ApplyRange`` calls may cause resource issues. An alternative can be to use
:ref:`ConditionalReader <complicated-applyrange>` with :doc:`ConditionalFilter <conditionalfilter>` to select between the
unprocessed and processed version of a source.

The filter name must be enclosed in quotation marks (it's a string), and the
two nested argument lists are not parenthesized. Strings and video clips
can't be interpolated, and therefore must be identical in the two argument
lists. An important warning though: If you use a clip as first argument, that
same clip shouldn't be included in start_args and  end_args. Thus for example
::

    v = Version()
    Animate(v,0,149,"Crop", v,0,0,64,32, v,316,0,64,32)

results in an error, since the first frame is the same as the invalid syntax
Crop(v, v, 0, 0, 64, 32).

This filter will not correctly handle a changing sound track, so I don't
recommend its use with filters which modify the sound track. And for heaven's
sake don't make the initial and final parameters yield a different output
frame size.

The filtername argument can even be ``Animate`` if you want quadratic rather
than linear interpolation.

**Several examples:**

::

    # Make a scrolling version of the "Version" video:
    ver = Version()
    Animate(0,149,"Crop", ver,0,0,64,32, ver,316,0,64,32)

    # or what is the same:
    ver = Version()
    Animate(ver,0,149,"Crop", 0,0,64,32, 316,0,64,32)

    # Fade to white
    AviSource("E:\pdwork\DO-Heaven.AVI")
    Animate(100,200,"Levels", 0,1,255,0,255, 0,1,255,255,255)

    # Do a gradual zoom into the center of a 320x240 video, starting at
    # 1:1 magnification in frame 100 and ending with 4:1 magnification
    # in frame 200:
    clip = AviSource("E:\pdwork\DO-Heaven.avi")
    Animate(100,200,"BicubicResize", clip,320,240,0,0,320,240,
    clip,320,240,120,90,80,60)
    # Animate(clip, 100,200,"BicubicResize", 320,240,0,0,320,240,
    320,240,120,90,80,60) # also works

    # Make the text "Hello, World!" zoom out from the center of a 320x240
    video:
    BlankClip(width=320, height=240)
    Animate(0,48,"Subtitle", "Hello, World!",160,120,0,99999,"Arial",0,
      \  "Hello, World!",25,130,0,99999,"Arial",48)

**Zooming clip c2 while overlaying it on c1:**

::

    Function myfunc(clip c1, clip c2, int x, int y, int w, int h)
    {
      w = w - w%2
      h = h - h%2
      my_c2 = BicubicResize(c2,w,h)
      Overlay(c1,my_c2,x,y)
    }

    c1 = AviSource("D:\Captures\jewel.avi") # c1 is larger than c2
    c2 = AviSource("D:\Captures\atomic.avi").BicubicResize(320,240)
    Animate(0,1000,"myfunc",c1,c2,10,10,10,10,c1,c2,300,300,360,288)
    # or
    # Animate(c1,0,1000,"myfunc", c2,10,10,10,10, c2,300,300,360,288)

    # but the following doesn't work, since three clips are passed to myfunc (c1, c1 and c2), while only two are allowed:
    # Animate(c1,0,1000,"myfunc",c1,c2,10,10,10,10,c1,c2,300,300,360,288)

**A small picture enlarges on a black clip until replace the main clip:**

::

    function res(clip clip, clip "LClip", int "width", int "height", int "centerX", int "centerY") {
    LClip = BicubicResize(LClip, width, height)
    Overlay(clip, LClip, centerX-LClip.width/2, centerY-LClip.height/2)
    }

    function resize(clip clip, clip "LClip", int "start_frame", int
    "start_width", int "start_height",
      \ int "end_frame", int "end_width", int "end_height", int
      "centerX", int "centerY") {
      return Animate(start_frame, end_frame, "res", clip, LClip,
      start_width, start_height, centerX, centerY,
        \ clip, LClip, end_width, end_height, centerX, centerY)
    }

    clip = AviSource("D:\captures\jewel.avi")
    clip = clip.BicubicResize(640,480)
    clip = clip.ConvertToRGB()
    black = BlankClip(clip)

    resize(black, clip, 0, 120, 120*clip.height/clip.width, 500, 640, 480, clip.width/2, clip.height/2)

**Examples using ApplyRange:**

::

    ver = Version()
    return ver.ApplyRange(0,149,"Crop", 158,0,64,32)
    # gives an error since cannot have different frame sizes within a
    clip ::Version()
    ApplyRange(100,149,"Blur", 1.0) ::AviSource("E:\pdwork\DO-
    Heaven.avi").BicubicResize(320,240)
    ApplyRange(0,48,"Subtitle", "Hello,
    World!",25,130,0,99999,"Arial",48)

    # or what is the same:
    clip = AviSource("E:\pdwork\DO-Heaven.avi").BicubicResize(320,240)
    ApplyRange(clip, 0,48,"Subtitle", "Hello,
    World!",25,130,0,99999,"Arial",48)

    # but since the frame range can be provided to Subtitle itself, this
    is the same as:
    AviSource("E:\pdwork\DO-Heaven.avi").BicubicResize(320,240)
    Subtitle("Hello, World!",25,130,0,48,"Arial",48)

$Date: 2009/09/12 15:10:22 $
