#ifndef __Merge_AVX2_H__
#define __Merge_AVX2_H__

#include <avisynth.h>
template<typename pixel_t>
void average_plane_avx2(BYTE *p1, const BYTE *p2, int p1_pitch, int p2_pitch, int rowsize, int height);

template<bool lessthan16bit>
void weighted_merge_planar_uint16_avx2(BYTE *p1, const BYTE *p2, int p1_pitch, int p2_pitch, int width, int height, float weight_f, int weight_i, int invweight_i);
void weighted_merge_planar_avx2(BYTE *p1,const BYTE *p2, int p1_pitch, int p2_pitch,int rowsize, int height, float weight_f, int weight_i, int invweight_i);

#endif  // __MergeAVX2_H__
