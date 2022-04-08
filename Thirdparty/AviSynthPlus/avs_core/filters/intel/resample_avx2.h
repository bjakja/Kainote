// Avisynth v2.5.  Copyright 2002 Ben Rudiak-Gould et al.
// http://www.avisynth.org

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA, or visit
// http://www.gnu.org/copyleft/gpl.html .
//
// Linking Avisynth statically or dynamically with other modules is making a
// combined work based on Avisynth.  Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.
//
// As a special exception, the copyright holders of Avisynth give you
// permission to link Avisynth with independent modules that communicate with
// Avisynth solely through the interfaces defined in avisynth.h, regardless of the license
// terms of these independent modules, and to copy and distribute the
// resulting combined work under terms of your choice, provided that
// every copy of the combined work is accompanied by a complete copy of
// the source code of Avisynth (the version of Avisynth used to produce the
// combined work), being distributed under the terms of the GNU General
// Public License plus this exception.  An independent module is a module
// which is not derived from or based on Avisynth, such as 3rd-party filters,
// import and export plugins, or graphical user interfaces.

#ifndef __Resample_AVX2_H__
#define __Resample_AVX2_H__

#include <avisynth.h>
#include "../resample_functions.h"

template<int filtersizealigned8, int filtersizemod8>
void resizer_h_avx2_generic_float(BYTE* dst8, const BYTE* src8, int dst_pitch, int src_pitch, ResamplingProgram* program, int width, int height, int bits_per_pixel);

template<bool lessthan16bit>
void resizer_h_avx2_generic_uint16_t(BYTE* dst8, const BYTE* src8, int dst_pitch, int src_pitch, ResamplingProgram* program, int width, int height, int bits_per_pixel);

void resizer_h_avx2_generic_uint8_t(BYTE* dst8, const BYTE* src8, int dst_pitch, int src_pitch, ResamplingProgram* program, int width, int height, int bits_per_pixel);

template<bool lessthan16bit>
void resize_v_avx2_planar_uint16_t(BYTE* dst0, const BYTE* src0, int dst_pitch, int src_pitch, ResamplingProgram* program, int width, int target_height, int bits_per_pixel, const int* pitch_table, const void* storage);

void resize_v_avx2_planar_float(BYTE* dst0, const BYTE* src0, int dst_pitch, int src_pitch, ResamplingProgram* program, int width, int target_height, int bits_per_pixel, const int* pitch_table, const void* storage);

void resize_v_avx2_planar_uint8_t(BYTE* dst, const BYTE* src, int dst_pitch, int src_pitch, ResamplingProgram* program, int width, int target_height, int bits_per_pixel, const int* pitch_table, const void* storage);

#endif // __Resample_AVX2_H__
