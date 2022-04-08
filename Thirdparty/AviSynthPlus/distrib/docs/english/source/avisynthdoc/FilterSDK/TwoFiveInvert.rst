
`TwoFiveInvert`_
================

::

    #include "avisynth.h"

    class Invert : public GenericVideoFilter {

        public:
            Invert(PClip _child) : GenericVideoFilter(_child) {}
            PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);
    };

    PVideoFrame __stdcall Invert::GetFrame(int n, IScriptEnvironment* env) {

    PVideoFrame src = child->GetFrame(n, env);

    PVideoFrame dst = env->NewVideoFrame(vi);

    const unsigned char* srcpY = src->GetReadPtr(PLANAR_Y);

    const unsigned char* srcpV = src->GetReadPtr(PLANAR_V);

    const unsigned char* srcpU = src->GetReadPtr(PLANAR_U);

    unsigned char* dstpY = dst->GetWritePtr(PLANAR_Y);

    unsigned char* dstpV = dst->GetWritePtr(PLANAR_V);

    unsigned char* dstpU = dst->GetWritePtr(PLANAR_U);

    const int src_pitchY = src->GetPitch(PLANAR_Y);

    const int src_pitchUV = src->GetPitch(PLANAR_V);

    const int dst_pitchY = dst->GetPitch(PLANAR_Y);

    const int dst_pitchUV = dst->GetPitch(PLANAR_U);

    const int row_sizeY = dst->GetRowSize(PLANAR_Y); // Could also be PLANAR_Y_ALIGNED which would return a mod16 rowsize

    const int row_sizeUV = dst->GetRowSize(PLANAR_U); // Could also be PLANAR_U_ALIGNED which would return a mod8 rowsize

    const int heightY = dst->GetHeight(PLANAR_Y);

    const int heightUV = dst->GetHeight(PLANAR_U);

    for (int y = 0; y < heightY; y++) {
        for (int x = 0; x < row_sizeY; x++)
            dstpY[x] = srcpY[x] ^ 255;
            srcpY += src_pitchY;
            dstpY += dst_pitchY;
    }

    for (int y = 0; y < heightUV; y++) {
        for (int x = 0; x < row_sizeUV; x++) {
            dstpU[x] = srcpU[x] ^ 255;
            dstpV[x] = srcpV[x] ^ 255;
        }

        srcpU += src_pitchUV;
        dstpU += dst_pitchUV;
        srcpV += src_pitchUV;
        dstpV += dst_pitchUV;
    }

    return dst;

    }

    AVSValue __cdecl Create_Invert(AVSValue args, void* user_data, IScriptEnvironment* env) {
        return new Invert(args[0].AsClip());
    }

    extern "C" __declspec(dllexport) const char* __stdcall AvisynthPluginInit2(IScriptEnvironment* env) {
        env->AddFunction("Invert", "c", Create_Invert, 0);
        return "`Invert' sample plugin";
    }

Back to :doc:`AviSynthTwoFiveSDK <AviSynthTwoFiveSDK>`

$Date: 2006/11/24 18:21:26 $

.. _TwoFiveInvert: http://www.avisynth.org/TwoFiveInvert
