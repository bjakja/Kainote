#include "check.h"
#include "../Kainote/AssBlend.h"

#include <cstdint>
#include <algorithm>
#include <cstdlib>
#include <vector>

namespace {

// The blend libass bitmaps used before, per channel with exact division.
void ReferenceBlend(unsigned char *dst, int dstPitch, const unsigned char *src, int srcStride,
	int w, int h, uint32_t color)
{
	unsigned a = 255 - (color & 0xFF);
	unsigned r = color >> 24, g = (color >> 16) & 0xFF, b = (color >> 8) & 0xFF;
	for (int y = 0; y < h; y++) {
		uint32_t *row = (uint32_t *)(dst + y * dstPitch);
		for (int x = 0; x < w; x++) {
			unsigned v = src[y * srcStride + x];
			unsigned aa = a * v;
			uint32_t p = row[x];
			unsigned db = p & 0xFF, dg = (p >> 8) & 0xFF, dr = (p >> 16) & 0xFF, da = p >> 24;
			db = (b * aa + db * (65025 - aa)) / 65025;
			dg = (g * aa + dg * (65025 - aa)) / 65025;
			dr = (r * aa + dr * (65025 - aa)) / 65025;
			da = (aa * 255 + da * (65025 - aa)) / 65025;
			row[x] = db | (dg << 8) | (dr << 16) | (da << 24);
		}
	}
}

struct Case
{
	int w, h;
	std::vector<unsigned char> dst, src;
	Case(int width, int height, unsigned seed) : w(width), h(height), dst(width * height * 4), src(width * height)
	{
		std::srand(seed);
		for (auto &c : dst) c = (unsigned char)std::rand();
		for (auto &c : src) c = (unsigned char)std::rand();
	}
};

int MaxDifference(const std::vector<unsigned char> &a, const std::vector<unsigned char> &b)
{
	int worst = 0;
	for (size_t i = 0; i < a.size(); i++)
		worst = std::max(worst, std::abs(a[i] - b[i]));
	return worst;
}

} // namespace

TEST(blend_matches_the_exact_formula_within_rounding)
{
	const uint32_t colors[] = { 0xFFFFFF00, 0x10204080, 0x80FF00FE, 0x00000000, 0xC0C0C0FF };
	for (int w : { 1, 3, 4, 7, 16, 33 }) {
		for (uint32_t color : colors) {
			Case c(w, 5, w * 31 + color);
			std::vector<unsigned char> expected = c.dst;
			ReferenceBlend(expected.data(), w * 4, c.src.data(), w, w, 5, color);
			BlendAssBitmap(c.dst.data(), w * 4, c.src.data(), w, w, 5, color);
			CHECK(MaxDifference(c.dst, expected) <= 2);
		}
	}
}

TEST(zero_coverage_leaves_the_pixels_alone)
{
	Case c(9, 2, 7);
	std::fill(c.src.begin(), c.src.end(), 0);
	std::vector<unsigned char> before = c.dst;
	BlendAssBitmap(c.dst.data(), 9 * 4, c.src.data(), 9, 9, 2, 0x11223300);
	CHECK(c.dst == before);
}

TEST(full_opaque_coverage_writes_the_colour)
{
	Case c(6, 1, 3);
	std::fill(c.src.begin(), c.src.end(), 255);
	BlendAssBitmap(c.dst.data(), 6 * 4, c.src.data(), 6, 6, 1, 0x11223300);
	for (int x = 0; x < 6; x++) {
		CHECK_EQ((int)c.dst[x * 4 + 0], 0x33);
		CHECK_EQ((int)c.dst[x * 4 + 1], 0x22);
		CHECK_EQ((int)c.dst[x * 4 + 2], 0x11);
		CHECK_EQ((int)c.dst[x * 4 + 3], 0xFF);
	}
}

TEST(strides_wider_than_the_bitmap_are_respected)
{
	std::vector<unsigned char> dst(8 * 2 * 4, 0), src(10 * 2, 255);
	BlendAssBitmap(dst.data(), 8 * 4, src.data(), 10, 5, 2, 0xFFFFFF00);
	CHECK_EQ((int)dst[4 * 4 + 3], 0xFF);
	CHECK_EQ((int)dst[5 * 4 + 3], 0);
	CHECK_EQ((int)dst[8 * 4 + 4 * 4 + 3], 0xFF);
}
