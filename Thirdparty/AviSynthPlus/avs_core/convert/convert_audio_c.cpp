// Avisynth+
// https://avs-plus.net
//
// This file is part of Avisynth+ which is released under GPL2+ with exception.

// Convert Audio helper functions (Pure C)
// Copyright (c) 2020 Xinyue Lu, (c) 2021 pinterf

#include <avs/types.h>

/* Supported fast route conversions:
 *
 * |    From: | U 8 | S16 | S24 | S32 | FLT |
 * | To:      |     |     |     |     |     |
 * |  U 8     |  -  | CS  | CS  | CS  | CSA |
 * |  S16     | CS  |  -  | CS  | CSA | CSA |
 * |  S24     | CS  | CS  |  -  | CS  |     |
 * |  S32     | CS  | CSA | CS  |  -  | CSA |
 * |  FLT     | CSA | CSA |     | CSA |  -  |
 * 
 * * C = C, S = SSE2+, A = AVX2
 */

/*
 8 bit: unsigned (middle point 128)
 16,24,32 bit: signed
 32 bit float: -1.0 .. 1.0

 Assymetric range considerations.

 Android: It is implementation dependent whether the positive maximum of 1.0 is included in the interval
 when converting to integer representation
 
 Method 1 (e.g. Android): smallest number is full scale, 1.0 is clamped to 1.0 minus one LSB
   -0x8000 - 0x7FFF is valid, nominally +1.0 (top of range) is not part of the range [-1.0..1.0)
 Method 2: largest number is full scale
   -0x7FFF - 0x7FFF, while -8000 exceeds lower limit. [-1.0..1.0] + smallest value is theoretically invalid

*/

// until 3.6.1: S16 = (S32 + 0x8000) >> 16   (plain round-before shift)
// Actual: S16 = S32 >> 16
void convert32To16(void *inbuf, void *outbuf, int count) {
  auto in16 = reinterpret_cast<int16_t *>(inbuf);
  auto out = reinterpret_cast<int16_t *>(outbuf);

  for (int i = 0; i < count; i++)
    out[i] = in16[i * 2 + 1];
}

// until 3.6.1: S32 = (S16 << 16) + (unsigned short)(S16 + 32768)
//              0x7fff -> 0x7fffffff, 0x8000 -> 0x80000000
// Actual: S32 = S16 << 16
void convert16To32(void *inbuf, void *outbuf, int count) {
  auto in = reinterpret_cast<int16_t *>(inbuf);
  auto out16 = reinterpret_cast<int16_t *>(outbuf);

  for (int i = 0; i < count; i++) {
    out16[i * 2] = 0;
    out16[i * 2 + 1] = in[i];
  }
}

void convert32To8(void* inbuf, void* outbuf, int count) {
  auto in8 = reinterpret_cast<int8_t*>(inbuf);
  auto out = reinterpret_cast<uint8_t*>(outbuf);

  for (int i = 0; i < count; i++)
    out[i] = in8[i * 4 + 3] + 128;
}

void convert8To32(void *inbuf, void *outbuf, int count) {
  auto in = reinterpret_cast<uint8_t *>(inbuf);
  auto out8 = reinterpret_cast<int8_t *>(outbuf);

  for (int i = 0; i < count; i++) {
    out8[i * 4] = 0;
    out8[i * 4 + 1] = 0;
    out8[i * 4 + 2] = 0;
    out8[i * 4 + 3] = in[i] - 128;
  }
}

void convert16To8(void *inbuf, void *outbuf, int count) {
  auto in8 = reinterpret_cast<int8_t *>(inbuf);
  auto out = reinterpret_cast<uint8_t *>(outbuf);

  for (int i = 0; i < count; i++)
    out[i] = in8[i * 2 + 1] + 128;
}

// until 3.6.1: S16 = (S8 << 8) + (unsigned short)(S8 + 128)
//              This make 0x7f(255-128) -> 0x7fff & 0x80(0-128) -> 0x8000
// Actual: S16 = (U8-128) << 8
void convert8To16(void *inbuf, void *outbuf, int count) {
  auto in = reinterpret_cast<uint8_t *>(inbuf);
  auto out8 = reinterpret_cast<int8_t *>(outbuf);

  for (int i = 0; i < count; i++) {
    out8[i * 2] = 0;
    out8[i * 2 + 1] = in[i] - 128;
  }
}

void convert32To24(void *inbuf, void *outbuf, int count) {
  auto in8 = reinterpret_cast<int8_t *>(inbuf);
  auto out8 = reinterpret_cast<int8_t *>(outbuf);

  for (int i = 0; i < count; i++) {
    out8[i * 3 + 0] = in8[i * 4 + 1];
    out8[i * 3 + 1] = in8[i * 4 + 2];
    out8[i * 3 + 2] = in8[i * 4 + 3];
  }
}

void convert24To32(void *inbuf, void *outbuf, int count) {
  auto in8 = reinterpret_cast<int8_t *>(inbuf);
  auto out8 = reinterpret_cast<int8_t *>(outbuf);

  for (int i = 0; i < count; i++) {
    out8[i * 4] = 0;
    out8[i * 4 + 1] = in8[i * 3 + 0];
    out8[i * 4 + 2] = in8[i * 3 + 1];
    out8[i * 4 + 3] = in8[i * 3 + 2];
  }
}

void convert24To16(void *inbuf, void *outbuf, int count) {
  auto in8 = reinterpret_cast<int8_t *>(inbuf);
  auto out8 = reinterpret_cast<int8_t *>(outbuf);

  for (int i = 0; i < count; i++) {
    out8[i * 2 + 0] = in8[i * 3 + 1];
    out8[i * 2 + 1] = in8[i * 3 + 2];
  }
}

void convert16To24(void *inbuf, void *outbuf, int count) {
  auto in8 = reinterpret_cast<int8_t *>(inbuf);
  auto out8 = reinterpret_cast<int8_t *>(outbuf);

  for (int i = 0; i < count; i++) {
    out8[i * 3] = 0;
    out8[i * 3 + 1] = in8[i * 2 + 0];
    out8[i * 3 + 2] = in8[i * 2 + 1];
  }
}

void convert24To8(void *inbuf, void *outbuf, int count) {
  auto in8 = reinterpret_cast<int8_t *>(inbuf);
  auto out = reinterpret_cast<uint8_t *>(outbuf);

  for (int i = 0; i < count; i++)
    out[i] = in8[i * 3 + 2] + 128;
}

void convert8To24(void *inbuf, void *outbuf, int count) {
  auto in = reinterpret_cast<uint8_t *>(inbuf);
  auto out8 = reinterpret_cast<int8_t *>(outbuf);

  for (int i = 0; i < count; i++) {
    out8[i * 3] = 0;
    out8[i * 3 + 1] = 0;
    out8[i * 3 + 2] = in[i] - 128;
  }
}

void convert8ToFLT(void* inbuf, void* outbuf, int count) {
  auto in = reinterpret_cast<uint8_t*>(inbuf);
  auto out = reinterpret_cast<SFLOAT*>(outbuf);
  constexpr float divisor = 1.0f / 128.f; // 1 << 7

  for (int i = 0; i < count; i++)
    out[i] = (in[i] - 128) * divisor;
}

void convertFLTTo8(void* inbuf, void* outbuf, int count) {
  auto in = reinterpret_cast<SFLOAT*>(inbuf);
  auto out = reinterpret_cast<uint8_t*>(outbuf);
  constexpr float multiplier = 128.f;
  constexpr float max8 = 127.f;
  constexpr float min8 = -128.f;

  for (int i = 0; i < count; i++) {
    float val = in[i] * multiplier;
    uint8_t result;
    if (val >= max8) result = 255;
    else if (val <= min8) result = 0;
    else result = static_cast<int8_t>(val) + 128;
    out[i] = result;
  }
}

void convert16ToFLT(void* inbuf, void* outbuf, int count) {
  auto in = reinterpret_cast<int16_t*>(inbuf);
  auto out = reinterpret_cast<SFLOAT*>(outbuf);
  constexpr float divisor = 1.0f / 32768.f; // 1 << 15

  for (int i = 0; i < count; i++)
    out[i] = in[i] * divisor;
}

void convertFLTTo16(void* inbuf, void* outbuf, int count) {
  auto in = reinterpret_cast<SFLOAT*>(inbuf);
  auto out = reinterpret_cast<int16_t*>(outbuf);
  constexpr float multiplier = 32768.f;
  constexpr float max16 = 32767.f;
  constexpr float min16 = -32768.f;

  for (int i = 0; i < count; i++) {
    float val = in[i] * multiplier;
    int16_t result;
    if (val >= max16) result = 32767;
    else if (val <= min16) result = (int16_t)-32768;
    else result = static_cast<int16_t>(val);
    out[i] = result;
  }
}

// not yet used directly, 24 bit has 32 bit 2nd stage
void convert24ToFLT(void* inbuf, void* outbuf, int count) {
  auto in = reinterpret_cast<uint8_t*>(inbuf);
  auto out = reinterpret_cast<SFLOAT*>(outbuf);
  constexpr float divisor = 1.0f / 8388608.f; // 1 << 23

  for (int i = 0; i < count; i++)
    out[i] = (in[i * 3] | (in[i * 3 + 1] << 8) | (in[i * 3 + 2] << 16)) * divisor;
}

// not yet used directly, 24 bit has 32 bit 2nd stage
void convertFLTTo24(void* inbuf, void* outbuf, int count) {
  auto in = reinterpret_cast<SFLOAT*>(inbuf);
  auto out = reinterpret_cast<uint8_t*>(outbuf);
  constexpr float multiplier = 8388608.f;
  constexpr float max24 = 8388607.f;
  constexpr float min24 = -8388608.f;

  for (int i = 0; i < count; i++) {
    float val = in[i] * multiplier;
    int32_t result;
    if (val >= max24) result = 0x7FFFFF; // 8388607
    else if (val <= min24) result = 0x800000; // -8388608
    else result = static_cast<int32_t>(val);
    out[i * 3 + 0] = result & 0xFF;
    out[i * 3 + 1] = (result >> 8) & 0xFF;
    out[i * 3 + 2] = (result >> 16) & 0xFF;
  }
}

// note for 32 bit conversions: 32 bit integer cannot be represented in float
// 2147483647.0f is 2147483648.0f in reality

void convert32ToFLT(void *inbuf, void *outbuf, int count) {
  auto in = reinterpret_cast<int32_t *>(inbuf);
  auto out = reinterpret_cast<SFLOAT *>(outbuf);
  constexpr float divisor = 1.0f/2147483648.0f;

  for (int i = 0; i < count; i++)
    out[i] = in[i] * divisor;
}

void convertFLTTo32(void *inbuf, void *outbuf, int count) {
  auto in = reinterpret_cast<SFLOAT *>(inbuf);
  auto out = reinterpret_cast<int32_t *>(outbuf);
  constexpr float multiplier = 2147483648.0f;
  constexpr float max32 = 2147483647.0f;
  constexpr float min32 = -2147483648.0f;

  for (int i = 0; i < count; i++) {
    float val = in[i] * multiplier;
    int32_t result;
    if (val >= max32) result = 0x7FFFFFFF; // 2147483647
    else if (val <= min32) result = 0x80000000; // -2147483648
    else result = static_cast<int32_t>(val);
    out[i] = result;
  }
}
