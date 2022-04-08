// Avisynth+
// https://avs-plus.net
//
// This file is part of Avisynth+ which is released under GPL2+ with exception.

// Convert Audio helper functions (SSE2/SSSE3)
// Copyright (c) 2020 Xinyue Lu, (c) 2021 pinterf

#include <avs/types.h>
#include <avs/config.h>
#include <smmintrin.h> // SSE4.1 at most

#if defined(GCC) || defined(CLANG)
  #define SSE2 __attribute__((__target__("sse2")))
  #define SSSE3 __attribute__((__target__("ssse3")))
  #define SSE41 __attribute__((__target__("sse4.1")))
#else
  #define SSE2
  #define SSSE3
  #define SSE41
#endif

// Easy: 32-16, 16-32, 32-8, 8-32, 16-8, 8-16
// Hard: 32-24, 24-32, 24-16, 16-24, 24-8, 8-24
// Float: 32-FLT, FLT-32

SSE2 void convert32To16_SSE2(void *inbuf, void *outbuf, int count) {
  auto in = reinterpret_cast<int32_t *>(inbuf);
  auto in16 = reinterpret_cast<int16_t *>(inbuf);
  auto out = reinterpret_cast<int16_t *>(outbuf);

  const int c_loop = count & ~7;

  for (int i = c_loop; i < count; i++)
    out[i] = in16[i * 2 + 1];

  for (int i = 0; i < c_loop; i += 8) {
    __m128i in32a = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in)); in += 4;
    __m128i in32b = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in)); in += 4;
    __m128i in16a = _mm_srai_epi32(in32a, 16);
    __m128i in16b = _mm_srai_epi32(in32b, 16);
    __m128i out16 = _mm_packs_epi32(in16a, in16b);
    _mm_storeu_si128(reinterpret_cast<__m128i *>(out), out16); out += 8;
  }
}

SSE2 void convert16To32_SSE2(void *inbuf, void *outbuf, int count) {
  auto in = reinterpret_cast<int16_t *>(inbuf);
  auto out = reinterpret_cast<int32_t *>(outbuf);
  auto out16 = reinterpret_cast<int16_t *>(outbuf);

  const int c_loop = count & ~7;

  for (int i = c_loop; i < count; i++) {
    out16[i * 2] = 0;
    out16[i * 2 + 1] = in[i];
  }

  __m128i zero = _mm_set1_epi16(0);
  for (int i = 0; i < c_loop; i += 8) {
    __m128i in16 = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in)); in += 8;
    __m128i out32a = _mm_unpacklo_epi16(zero, in16);
    __m128i out32b = _mm_unpackhi_epi16(zero, in16);
    _mm_storeu_si128(reinterpret_cast<__m128i *>(out), out32a); out += 4;
    _mm_storeu_si128(reinterpret_cast<__m128i *>(out), out32b); out += 4;
  }
}

SSE2 void convert32To8_SSE2(void *inbuf, void *outbuf, int count) {
  auto in = reinterpret_cast<int32_t *>(inbuf);
  auto in8 = reinterpret_cast<int8_t *>(inbuf);
  auto out = reinterpret_cast<uint8_t *>(outbuf);

  const int c_loop = count & ~15;

  for (int i = c_loop; i < count; i++)
    out[i] = in8[i * 4 + 3] + 128;

  for (int i = 0; i < c_loop; i += 16) {
    __m128i in32a = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in)); in += 4;
    __m128i in32b = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in)); in += 4;
    __m128i in32c = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in)); in += 4;
    __m128i in32d = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in)); in += 4;

    __m128i in8a = _mm_srai_epi32(in32a, 24);
    __m128i in8b = _mm_srai_epi32(in32b, 24);
    __m128i in8c = _mm_srai_epi32(in32c, 24);
    __m128i in8d = _mm_srai_epi32(in32d, 24);

    __m128i out8a = _mm_packs_epi32(in8a, in8b);
    __m128i out8b = _mm_packs_epi32(in8c, in8d);

    __m128i out8 = _mm_packs_epi16(out8a, out8b);

    out8 = _mm_add_epi8(out8, _mm_set1_epi8(-128));
    _mm_storeu_si128(reinterpret_cast<__m128i *>(out), out8); out += 16;
  }
}

SSE2 void convert8To32_SSE2(void *inbuf, void *outbuf, int count) {
  auto in = reinterpret_cast<uint8_t *>(inbuf);
  auto out = reinterpret_cast<int32_t *>(outbuf);
  auto out8 = reinterpret_cast<int8_t *>(outbuf);

  const int c_loop = count & ~15;

  for (int i = c_loop; i < count; i++) {
    out8[i * 4] = 0;
    out8[i * 4 + 1] = 0;
    out8[i * 4 + 2] = 0;
    out8[i * 4 + 3] = in[i] - 128;
  }

  __m128i n128 = _mm_set1_epi8(-128);
  __m128i zero = _mm_set1_epi16(0);
  for (int i = 0; i < c_loop; i += 16) {
    __m128i in8 = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in)); in += 16;
    in8 = _mm_add_epi8(in8, n128);
    __m128i v16a = _mm_unpacklo_epi8(zero, in8);
    __m128i v16b = _mm_unpackhi_epi8(zero, in8);
    __m128i out32a = _mm_unpacklo_epi16(zero, v16a);
    __m128i out32b = _mm_unpackhi_epi16(zero, v16a);
    __m128i out32c = _mm_unpacklo_epi16(zero, v16b);
    __m128i out32d = _mm_unpackhi_epi16(zero, v16b);
    _mm_storeu_si128(reinterpret_cast<__m128i *>(out), out32a); out += 4;
    _mm_storeu_si128(reinterpret_cast<__m128i *>(out), out32b); out += 4;
    _mm_storeu_si128(reinterpret_cast<__m128i *>(out), out32c); out += 4;
    _mm_storeu_si128(reinterpret_cast<__m128i *>(out), out32d); out += 4;
  }
}

SSE2 void convert16To8_SSE2(void *inbuf, void *outbuf, int count) {
  auto in = reinterpret_cast<int16_t *>(inbuf);
  auto in8 = reinterpret_cast<int8_t *>(inbuf);
  auto out = reinterpret_cast<uint8_t *>(outbuf);

  const int c_loop = count & ~15;

  for (int i = c_loop; i < count; i++)
    out[i] = in8[i * 2 + 1] + 128;

  for (int i = 0; i < c_loop; i += 16) {
    __m128i in16a = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in)); in += 8;
    __m128i in16b = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in)); in += 8;
    __m128i in8a = _mm_srai_epi16(in16a, 8);
    __m128i in8b = _mm_srai_epi16(in16b, 8);
    __m128i out8 = _mm_packs_epi16(in8a, in8b);
    out8 = _mm_add_epi8(out8, _mm_set1_epi8(-128));
    _mm_storeu_si128(reinterpret_cast<__m128i *>(out), out8); out += 16;
  }
}

SSE2 void convert8To16_SSE2(void *inbuf, void *outbuf, int count) {
  auto in = reinterpret_cast<uint8_t *>(inbuf);
  auto out = reinterpret_cast<int16_t *>(outbuf);
  auto out8 = reinterpret_cast<int8_t *>(outbuf);

  const int c_loop = count & ~15;

  for (int i = c_loop; i < count; i++) {
    out8[i * 2] = 0;
    out8[i * 2 + 1] = in[i] - 128;
  }

  __m128i n128 = _mm_set1_epi8(-128);
  __m128i zero = _mm_set1_epi16(0);
  for (int i = 0; i < c_loop; i += 16) {
    __m128i in8 = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in)); in += 16;
    in8 = _mm_add_epi8(in8, n128);
    __m128i out16a = _mm_unpacklo_epi8(zero, in8);
    __m128i out16b = _mm_unpackhi_epi8(zero, in8);
    _mm_storeu_si128(reinterpret_cast<__m128i *>(out), out16a); out += 8;
    _mm_storeu_si128(reinterpret_cast<__m128i *>(out), out16b); out += 8;
  }
}

SSSE3 void convert32To24_SSSE3(void *inbuf, void *outbuf, int count) {
  auto in = reinterpret_cast<int32_t *>(inbuf);
  auto in8 = reinterpret_cast<int8_t *>(inbuf);
  auto out8 = reinterpret_cast<int8_t *>(outbuf);

  const int c_loop = count & ~15;

  for (int i = c_loop; i < count; i++) {
    out8[i * 3 + 0] = in8[i * 4 + 1];
    out8[i * 3 + 1] = in8[i * 4 + 2];
    out8[i * 3 + 2] = in8[i * 4 + 3];
  }

  __m128i inv[4], outv[3], mask[6];
  // clang-format off
  mask[0] = _mm_set_epi8(
    -1, -1, -1, -1,
    15, 14, 13, 11,
    10,  9,  7,  6,
     5,  3,  2,  1);
  mask[1] = _mm_set_epi8(
     5,  3,  2,  1,
    -1, -1, -1, -1,
    -1, -1, -1, -1,
    -1, -1, -1, -1);
  mask[2] = _mm_set_epi8(
    -1, -1, -1, -1,
    -1, -1, -1, -1,
    15, 14, 13, 11,
    10,  9,  7,  6);
  mask[3] = _mm_set_epi8(
    10,  9,  7,  6,
     5,  3,  2,  1,
    -1, -1, -1, -1,
    -1, -1, -1, -1);
  mask[4] = _mm_set_epi8(
    -1, -1, -1, -1,
    -1, -1, -1, -1,
    -1, -1, -1, -1,
    15, 14, 13, 11);
  mask[5] = _mm_set_epi8(
    15, 14, 13, 11,
    10,  9,  7,  6,
     5,  3,  2,  1,
    -1, -1, -1, -1);

  for (int i = 0; i < c_loop; i += 16) {
    inv[0] = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in)); in += 4;
    inv[1] = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in)); in += 4;
    inv[2] = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in)); in += 4;
    inv[3] = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in)); in += 4;

    outv[0] = _mm_or_si128(
      _mm_shuffle_epi8(inv[0], mask[0]),
      _mm_shuffle_epi8(inv[1], mask[1])
    );
    outv[1] = _mm_or_si128(
      _mm_shuffle_epi8(inv[1], mask[2]),
      _mm_shuffle_epi8(inv[2], mask[3])
    );
    outv[2] = _mm_or_si128(
      _mm_shuffle_epi8(inv[2], mask[4]),
      _mm_shuffle_epi8(inv[3], mask[5])
    );

    _mm_storeu_si128(reinterpret_cast<__m128i *>(out8), outv[0]); out8 += 16;
    _mm_storeu_si128(reinterpret_cast<__m128i *>(out8), outv[1]); out8 += 16;
    _mm_storeu_si128(reinterpret_cast<__m128i *>(out8), outv[2]); out8 += 16;
  }
  // clang-format on
}

SSSE3 void convert24To32_SSSE3(void *inbuf, void *outbuf, int count) {
  auto in8 = reinterpret_cast<int8_t *>(inbuf);
  auto out8 = reinterpret_cast<int8_t *>(outbuf);
  auto out = reinterpret_cast<int32_t *>(outbuf);

  const int c_loop = count & ~15;

  for (int i = c_loop; i < count; i++) {
    out8[i * 4] = 0;
    out8[i * 4 + 1] = in8[i * 3 + 0];
    out8[i * 4 + 2] = in8[i * 3 + 1];
    out8[i * 4 + 3] = in8[i * 3 + 2];
  }

  __m128i inv[3], outv[4], mask[6];
  // clang-format off
  mask[0] = _mm_set_epi8(
    11, 10,  9, -1,
     8,  7,  6, -1,
     5,  4,  3, -1,
     2,  1,  0, -1);
  mask[1] = _mm_set_epi8(
    -1, -1, -1, -1,
    -1, -1, -1, -1,
    -1, -1, 15, -1,
    14, 13, 12, -1);
  mask[2] = _mm_set_epi8(
     7,  6,  5, -1,
     4,  3,  2, -1,
     1,  0, -1, -1,
    -1, -1, -1, -1);
  mask[3] = _mm_set_epi8(
    -1, -1, -1, -1,
    -1, 15, 14, -1,
    13, 12, 11, -1,
    10,  9,  8, -1);
  mask[4] = _mm_set_epi8(
     3,  2,  1, -1,
     0, -1, -1, -1,
    -1, -1, -1, -1,
    -1, -1, -1, -1);
  mask[5] = _mm_set_epi8(
    15, 14, 13, -1,
    12, 11, 10, -1,
     9,  8,  7, -1,
     6,  5,  4, -1);

  for (int i = 0; i < c_loop; i += 16) {
    inv[0] = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in8)); in8 += 16;
    inv[1] = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in8)); in8 += 16;
    inv[2] = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in8)); in8 += 16;

    outv[0] = _mm_shuffle_epi8(inv[0], mask[0]);
    outv[1] = _mm_or_si128(
      _mm_shuffle_epi8(inv[0], mask[1]),
      _mm_shuffle_epi8(inv[1], mask[2])
    );
    outv[2] = _mm_or_si128(
      _mm_shuffle_epi8(inv[1], mask[3]),
      _mm_shuffle_epi8(inv[2], mask[4])
    );
    outv[3] = _mm_shuffle_epi8(inv[2], mask[5]);

    _mm_storeu_si128(reinterpret_cast<__m128i *>(out), outv[0]); out += 4;
    _mm_storeu_si128(reinterpret_cast<__m128i *>(out), outv[1]); out += 4;
    _mm_storeu_si128(reinterpret_cast<__m128i *>(out), outv[2]); out += 4;
    _mm_storeu_si128(reinterpret_cast<__m128i *>(out), outv[3]); out += 4;
  }
  // clang-format on
}

SSSE3 void convert24To16_SSSE3(void *inbuf, void *outbuf, int count) {
  auto in8 = reinterpret_cast<int8_t *>(inbuf);
  auto out8 = reinterpret_cast<int8_t *>(outbuf);
  auto out = reinterpret_cast<int16_t *>(outbuf);

  const int c_loop = count & ~15;

  for (int i = c_loop; i < count; i++) {
    out8[i * 2 + 0] = in8[i * 3 + 1];
    out8[i * 2 + 1] = in8[i * 3 + 2];
  }

  __m128i inv[3], outv[2], mask[4];
  // clang-format off
  mask[0] = _mm_set_epi8(
    -1, -1, -1, -1,
    -1, -1, 14, 13,
    11, 10,  8,  7,
     5,  4,  2,  1);
  mask[1] = _mm_set_epi8(
     7,  6,  4,  3,
     1,  0, -1, -1,
    -1, -1, -1, -1,
    -1, -1, -1, -1);
  mask[2] = _mm_set_epi8(
    -1, -1, -1, -1,
    -1, -1, -1, -1,
    -1, -1, -1, 15,
    13, 12, 10,  9);
  mask[3] = _mm_set_epi8(
    15, 14, 12, 11,
     9,  8,  6,  5,
     3,  2,  0, -1,
    -1, -1, -1, -1);

  for (int i = 0; i < c_loop; i += 16) {
    inv[0] = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in8)); in8 += 16;
    inv[1] = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in8)); in8 += 16;
    inv[2] = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in8)); in8 += 16;

    outv[0] = _mm_or_si128(
      _mm_shuffle_epi8(inv[0], mask[0]),
      _mm_shuffle_epi8(inv[1], mask[1])
    );
    outv[1] = _mm_or_si128(
      _mm_shuffle_epi8(inv[1], mask[2]),
      _mm_shuffle_epi8(inv[2], mask[3])
    );

    _mm_storeu_si128(reinterpret_cast<__m128i *>(out), outv[0]); out += 8;
    _mm_storeu_si128(reinterpret_cast<__m128i *>(out), outv[1]); out += 8;
  }
  // clang-format on
}

SSSE3 void convert16To24_SSSE3(void *inbuf, void *outbuf, int count) {
  auto in = reinterpret_cast<int16_t *>(inbuf);
  auto in8 = reinterpret_cast<int8_t *>(inbuf);
  auto out8 = reinterpret_cast<int8_t *>(outbuf);

  const int c_loop = count & ~15;

  for (int i = c_loop; i < count; i++) {
    out8[i * 3] = 0;
    out8[i * 3 + 1] = in8[i * 2 + 0];
    out8[i * 3 + 2] = in8[i * 2 + 1];
  }

  __m128i inv[2], outv[3], mask[4];
  // clang-format off
  mask[0] = _mm_set_epi8(
    -1,  9,  8, -1,
     7,  6, -1,  5,
     4, -1,  3,  2,
    -1,  1,  0, -1);
  mask[1] = _mm_set_epi8(
    -1, -1, -1, -1,
    -1, -1, -1, -1,
    15, 14, -1, 13,
    12, -1, 11, 10);
  mask[2] = _mm_set_epi8(
     4, -1,  3,  2,
    -1,  1,  0, -1,
    -1, -1, -1, -1,
    -1, -1, -1, -1);
  mask[3] = _mm_set_epi8(
    15, 14, -1, 13,
    12, -1, 11, 10,
    -1,  9,  8, -1,
     7,  6, -1,  5);

  for (int i = 0; i < c_loop; i += 16) {
    inv[0] = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in)); in += 8;
    inv[1] = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in)); in += 8;

    outv[0] = _mm_shuffle_epi8(inv[0], mask[0]);
    outv[1] = _mm_or_si128(
      _mm_shuffle_epi8(inv[0], mask[1]),
      _mm_shuffle_epi8(inv[1], mask[2])
    );
    outv[2] = _mm_shuffle_epi8(inv[1], mask[3]);

    _mm_storeu_si128(reinterpret_cast<__m128i *>(out8), outv[0]); out8 += 16;
    _mm_storeu_si128(reinterpret_cast<__m128i *>(out8), outv[1]); out8 += 16;
    _mm_storeu_si128(reinterpret_cast<__m128i *>(out8), outv[2]); out8 += 16;
  }
  // clang-format on
}

SSSE3 void convert24To8_SSSE3(void *inbuf, void *outbuf, int count) {
  auto in8 = reinterpret_cast<int8_t *>(inbuf);
  auto out = reinterpret_cast<uint8_t *>(outbuf);

  const int c_loop = count & ~15;

  for (int i = c_loop; i < count; i++)
    out[i] = in8[i * 3 + 2] + 128;

  __m128i inv[3], outv, mask[3];
  __m128i n128 = _mm_set1_epi8(-128);
  // clang-format off
  mask[0] = _mm_set_epi8(
    -1, -1, -1, -1,
    -1, -1, -1, -1,
    -1, -1, -1, 14,
    11,  8,  5,  2);
  mask[1] = _mm_set_epi8(
    -1, -1, -1, -1,
    -1, -1, 13, 10,
     7,  4,  1, -1,
    -1, -1, -1, -1);
  mask[2] = _mm_set_epi8(
    15, 12,  9,  6,
     3,  0, -1, -1,
    -1, -1, -1, -1,
    -1, -1, -1, -1);

  for (int i = 0; i < c_loop; i += 16) {
    inv[0] = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in8)); in8 += 16;
    inv[1] = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in8)); in8 += 16;
    inv[2] = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in8)); in8 += 16;

    outv = _mm_or_si128(
      _mm_shuffle_epi8(inv[0], mask[0]),
      _mm_shuffle_epi8(inv[1], mask[1])
    );
    outv = _mm_or_si128(
      outv,
      _mm_shuffle_epi8(inv[2], mask[2])
    );
    outv = _mm_add_epi8(outv, n128);
    _mm_storeu_si128(reinterpret_cast<__m128i *>(out), outv); out += 16;
  }
  // clang-format on
}

SSSE3 void convert8To24_SSSE3(void *inbuf, void *outbuf, int count) {
  auto in = reinterpret_cast<uint8_t *>(inbuf);
  auto out8 = reinterpret_cast<int8_t *>(outbuf);

  const int c_loop = count & ~15;

  for (int i = c_loop; i < count; i++) {
    out8[i * 3] = 0;
    out8[i * 3 + 1] = 0;
    out8[i * 3 + 2] = in[i] - 128;
  }

  __m128i inv, outv[3], mask[3];
  __m128i n128 = _mm_set1_epi8(-128);
  // clang-format off
  mask[0] = _mm_set_epi8(
    -1,  4, -1, -1,
     3, -1, -1,  2,
    -1, -1,  1, -1,
    -1,  0, -1, -1);
  mask[1] = _mm_set_epi8(
    -1, -1,  9, -1,
    -1,  8, -1, -1,
     7, -1, -1,  6,
    -1, -1,  5, -1);
  mask[2] = _mm_set_epi8(
    15, -1, -1, 14,
    -1, -1, 13, -1,
    -1, 12, -1, -1,
    11, -1, -1, 10);

  for (int i = 0; i < c_loop; i += 16) {
    inv = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in)); in += 16;
    inv = _mm_add_epi8(inv, n128);

    outv[0] = _mm_shuffle_epi8(inv, mask[0]);
    outv[1] = _mm_shuffle_epi8(inv, mask[1]);
    outv[2] = _mm_shuffle_epi8(inv, mask[2]);
    _mm_storeu_si128(reinterpret_cast<__m128i *>(out8), outv[0]); out8 += 16;
    _mm_storeu_si128(reinterpret_cast<__m128i *>(out8), outv[1]); out8 += 16;
    _mm_storeu_si128(reinterpret_cast<__m128i *>(out8), outv[2]); out8 += 16;
  }
  // clang-format on
}

SSE41 void convert8ToFLT_SSE41(void* inbuf, void* outbuf, int count) {
  auto in = reinterpret_cast<uint8_t*>(inbuf);
  auto out = reinterpret_cast<SFLOAT*>(outbuf);
  constexpr float divisor = 1.0f / 128.f; // 1 << 7

  const int c_loop = count & ~3;

  for (int i = c_loop; i < count; i++)
    out[i] = (in[i] - 128) * divisor;

  __m128 divv = _mm_set1_ps(divisor);
  for (int i = 0; i < c_loop; i += 4) {
    __m128i in32 = _mm_cvtepu8_epi32(_mm_castps_si128(_mm_load_ss(reinterpret_cast<float *>(in)))); in += 4;
    in32 = _mm_sub_epi32(in32, _mm_set1_epi32(128));
    __m128 infl = _mm_cvtepi32_ps(in32);
    __m128 outfl = _mm_mul_ps(infl, divv);
    _mm_storeu_ps(out, outfl); out += 4;
  }
}

SSE2 void convertFLTTo8_SSE2(void* inbuf, void* outbuf, int count) {
  auto in = reinterpret_cast<SFLOAT*>(inbuf);
  auto out = reinterpret_cast<uint8_t*>(outbuf);
  constexpr float multiplier = 128.f;
  constexpr float max8 = 127.f;
  constexpr float min8 = -128.f;

  const int c_loop = count & ~3;

  for (int i = c_loop; i < count; i++) {
    float val = in[i] * multiplier;
    uint8_t result;
    if (val >= max8) result = 255;
    else if (val <= min8) result = 0;
    else result = static_cast<int8_t>(val) + 128;
    out[i] = result;
  }

  __m128 mulv = _mm_set1_ps(multiplier);
  __m128 maxv = _mm_set1_ps(max8);
  __m128 minv = _mm_set1_ps(min8);
  for (int i = 0; i < c_loop; i += 4) {
    __m128 infl = _mm_loadu_ps(in); in += 4;
    __m128 outfl = _mm_max_ps(minv, _mm_min_ps(maxv,_mm_mul_ps(infl, mulv)));
    __m128i out32 = _mm_cvttps_epi32(outfl);
    __m128i out16 = _mm_packs_epi32(out32, out32);
    __m128i out8 = _mm_packs_epi16(out16, out16);
    out8 = _mm_add_epi8(out8, _mm_set1_epi8(-128)); // 128
    *(uint32_t *)(out) = _mm_cvtsi128_si32(out8); out += 4;
  }
}

SSE41 void convert16ToFLT_SSE41(void* inbuf, void* outbuf, int count) {
  auto in = reinterpret_cast<int16_t*>(inbuf);
  auto out = reinterpret_cast<SFLOAT*>(outbuf);
  constexpr float divisor = 1.0f / 32768.f; // 1 << 15

  const int c_loop = count & ~3;

  for (int i = c_loop; i < count; i++)
    out[i] = in[i] * divisor;

  __m128 divv = _mm_set1_ps(divisor);
  for (int i = 0; i < c_loop; i += 4) {
    __m128i in32 = _mm_cvtepi16_epi32(_mm_loadl_epi64(reinterpret_cast<const __m128i*>(in))); in += 4;
    __m128 infl = _mm_cvtepi32_ps(in32);
    __m128 outfl = _mm_mul_ps(infl, divv);
    _mm_storeu_ps(out, outfl); out += 4;
  }
}

SSE2 void convertFLTTo16_SSE2(void* inbuf, void* outbuf, int count) {
  auto in = reinterpret_cast<SFLOAT*>(inbuf);
  auto out = reinterpret_cast<int16_t*>(outbuf);
  constexpr float multiplier = 32768.f;
  constexpr float max16 = 32767.f;
  constexpr float min16 = -32768.f;

  const int c_loop = count & ~3;

  for (int i = c_loop; i < count; i++) {
    float val = in[i] * multiplier;
    int16_t result;
    if (val >= max16) result = 32767;
    else if (val <= min16) result = (int16_t)-32768;
    else result = static_cast<int16_t>(val);
    out[i] = result;
  }

  __m128 mulv = _mm_set1_ps(multiplier);
  __m128 maxv = _mm_set1_ps(max16);
  __m128 minv = _mm_set1_ps(min16);
  for (int i = 0; i < c_loop; i += 4) {
    __m128 infl = _mm_loadu_ps(in); in += 4;
    __m128 outfl = _mm_max_ps(minv, _mm_min_ps(maxv, _mm_mul_ps(infl, mulv)));
    __m128i out32 = _mm_cvttps_epi32(outfl);
    __m128i out16 = _mm_packs_epi32(out32, out32);
    _mm_storel_epi64(reinterpret_cast<__m128i*>(out), out16); out += 4;
  }
}

SSE2 void convert32ToFLT_SSE2(void *inbuf, void *outbuf, int count) {
  auto in = reinterpret_cast<int32_t *>(inbuf);
  auto out = reinterpret_cast<SFLOAT *>(outbuf);
  const float divisor = 1.0f/2147483648.0f;

  const int c_loop = count & ~3;

  for (int i = c_loop; i < count; i++)
    out[i] = in[i] * divisor;

  __m128 divv = _mm_set1_ps(divisor);
  for (int i = 0; i < c_loop; i += 4) {
    __m128i in32 = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in)); in += 4;
    __m128 infl = _mm_cvtepi32_ps(in32);
    __m128 outfl = _mm_mul_ps(infl, divv);
    _mm_storeu_ps(out, outfl); out += 4;
  }
}

SSE41 void convertFLTTo32_SSE41(void *inbuf, void *outbuf, int count) {
  auto in = reinterpret_cast<SFLOAT *>(inbuf);
  auto out = reinterpret_cast<int32_t *>(outbuf);
  constexpr float multiplier = 2147483648.0f;
  constexpr float max32 = 2147483647.0f; // 2147483648.0f in reality
  constexpr float min32 = -2147483648.0f;

  const int c_loop = count & ~3;

  for (int i = c_loop; i < count; i++) {
    float val = in[i] * multiplier;
    int32_t result;
    if (val >= max32) result = 0x7FFFFFFF; // 2147483647
    else if (val <= min32) result = 0x80000000; // -2147483648
    else result = static_cast<int32_t>(val);
    out[i] = result;
  }

  __m128 mulv = _mm_set1_ps(multiplier);
  __m128 maxv = _mm_set1_ps(max32);
  __m128 minv = _mm_set1_ps(min32);
  __m128i maxv_i = _mm_set1_epi32(0x7FFFFFFF); // 2147483647
  __m128i minv_i = _mm_set1_epi32(0x80000000); // -2147483648
  for (int i = 0; i < c_loop; i += 4) {
    __m128 infl = _mm_loadu_ps(in); in += 4;
    __m128 outfl = _mm_mul_ps(infl, mulv);
    __m128i cmphigh = _mm_castps_si128(_mm_cmpge_ps(outfl, maxv));
    __m128i cmplow = _mm_castps_si128(_mm_cmpge_ps(minv, outfl));
    __m128i out32 = _mm_cvttps_epi32(outfl);
    out32 = _mm_blendv_epi8(out32, maxv_i, cmphigh);
    out32 = _mm_blendv_epi8(out32, minv_i, cmplow);
    _mm_storeu_si128(reinterpret_cast<__m128i *>(out), out32); out += 4;
  }
}
