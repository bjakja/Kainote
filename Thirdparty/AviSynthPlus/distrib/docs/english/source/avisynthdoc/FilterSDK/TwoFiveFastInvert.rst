
`TwoFiveFastInvert`_
====================

Please not that the code below may not actually be faster than the
:doc:`TwoFiveInvert <TwoFiveInvert>` sample, because the compiler doesn't handle 64 bit integers
very well. However this is not the case when using :doc:`MMX <MMX>` or :doc:`IntegerSSE <IntegerSSE>`.
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

    const __int64* srcpY = (__int64*)src->GetReadPtr(PLANAR_Y);

    const __int64* srcpV = (__int64*)src->GetReadPtr(PLANAR_V);

    const __int64* srcpU = (__int64*)src->GetReadPtr(PLANAR_U);

    __int64* dstpY = (__int64*)dst->GetWritePtr(PLANAR_Y);

    __int64* dstpV = (__int64*)dst->GetWritePtr(PLANAR_V);

    __int64* dstpU = (__int64*)dst->GetWritePtr(PLANAR_U);

    const int src_pitchY = src->GetPitch(PLANAR_Y)/8; // 8 bytes per loop

    const int src_pitchUV = src->GetPitch(PLANAR_V)/8;

    const int dst_pitchY = dst->GetPitch(PLANAR_Y)/8;

    const int dst_pitchUV = dst->GetPitch(PLANAR_U)/8;

    const int row_sizeY = dst->GetRowSize(PLANAR_Y_ALIGNED)/8; // Mod 16 size

    const int row_sizeUV = dst->GetRowSize(PLANAR_U_ALIGNED)/8; // Mod 8 size

    const int heightY = dst->GetHeight(PLANAR_Y);

    const int heightUV = dst->GetHeight(PLANAR_U);

    __int64 inv = 0xffffffffffffffff;

    for (int y = 0; y < heightY; y++) {

        for (int x = 0; x < row_sizeY; x++)
            dstpY[x] = srcpY[x] ^ inv;
            srcpY += src_pitchY;
            dstpY += dst_pitchY;
    }

    for (int y = 0; y < heightUV; y++) {

        for (int x = 0; x < row_sizeUV; x++) {
                dstpU[x] = srcpU[x] ^ inv;
                dstpV[x] = srcpV[x] ^ inv;
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
        env->AddFunction("Invert", "c", Create_Invert, 0);  return "`Invert' sample plugin";
    }

Back to :doc:`AviSynthTwoFiveSDK <AviSynthTwoFiveSDK>`

$Date: 2013/04/22 07:18:44 $

.. _TwoFiveFastInvert: http://www.avisynth.org/TwoFiveFastInvert
