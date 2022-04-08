
WorkingWithImages
=================

All images are given a pitch. The pitch is basically what can be described as
"length of a line". What's funny is that the pitch does not have to be equal
to the width of the image.

For instance, if you crop something off your image, the only thing that
changes is the width of your image; the pitch and the actual byte-count of a
line remains the same.

The image is then laid out like this:

``rrrrrrrrrrrrrrrrpppp``

``rrrrrrrrrrrrrrrrpppp``

``rrrrrrrrrrrrrrrrpppp``

Where 'r' are the pixels inside the image used, and 'p' is the padding
inserted after each line.


Fast Information
----------------

*Width* = width in pixels.

*Rowsize* = number of bytes in one line.

*Pitch* = distance from start of one line to the next in bytes.

*vi->BytesFromPixels(n)* = size of n pixels in bytes - does NOT take pitch into
consideration, so can only be used within a line.

For the :doc:`PlanarImageFormat <PlanarImageFormat>`:

Aligned rowsize = width in bytes, always divisible with 8.


VideoInfo vs. PVideoFrame
-------------------------

VideoInfo is considered the "constant" video info. This cannot change in any
way. The information such as width, height and colorspace cannot change. So
the information you get from this can be trusted to be the same from all
frames you recieve. If you change the VideoInfo you have received it will not
change the frames you receive, but a modified 'VideoInfo vi' can be sent to
env->NewVideoFrame(vi) and a new frame with the changed parameters will be
created. Only your own filter can modify the VideoInfo given to your filter.

PVideoFrame contains information about a particular frame you requested.
Height and Rowsize should not change (this can be considered a bug). Pitch
can change, so **you cannot rely on pitch being the same for all frames you
recieve**.


More Information
----------------

See more about :doc:`DataStorageInAviSynth <DataStorageInAviSynth>`.

See more about :doc:`ColorSpaces <ColorSpaces>`.

----

Back to :doc:`FilterSDK`

$Date: 2014/10/27 22:04:54 $
