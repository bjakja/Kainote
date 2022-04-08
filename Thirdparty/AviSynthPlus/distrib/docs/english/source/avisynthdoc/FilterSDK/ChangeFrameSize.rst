
ChangeFrameSize
===============

The following example code shows how to double the width of the destination
frame.
::

    public:
        ShowPixelValues(PClip _child, IScriptEnvironment* env) :
        GenericVideoFilter(_child)
        {
        // constructor code

            vi.width = vi.width*2; // width is doubled here
            vi.height = vi.height * 2;  // height is also
            multiplied by 2
        }
        PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);
    };

    PVideoFrame __stdcall ShowPixelValues::GetFrame(int n, IScriptEnvironment* env)
    {
        PVideoFrame src = child->GetFrame(n, env);
        PVideoFrame dst = env->NewVideoFrame(vi); // new frame is double the size of src
    ...


A Similar approach has to be used for changing colorspace, framerate etc.
There is a bit more information at :doc:`WorkingWithImages <WorkingWithImages>`.

----

Back to :doc:`FilterSDK`

Date: 2014/10/27 22:04:54 $
