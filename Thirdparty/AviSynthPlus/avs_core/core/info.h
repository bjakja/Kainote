// original IT0051 by thejam79
// add YV12 mode by minamina 2003/05/01
//
// Borrowed from the author of IT.dll, whose name I
// could not determine. Modified for YV12 by Donald Graft.
// RGB32 Added by Klaus Post
// Converted to generic planar, and now using exact coordinates - NOT character coordinates by Klaus Post
// Refactor, DrawString...() is the primative, Oct 2010 Ian Brabham
// TO DO: Clean up - and move functions to a .c file.

// pinterf:
// planar high bit depth, planar RGB, RGB48/64
// utf8 option, internally unicode, Latin-1 Supplement 00A0-00FF
// Original hexadecimal bitmap definitions changed to binary literals
// Add some new characters from Latin Extended A
// Configurable color
// Configurable halocolor (text outline)
// Configurable background fading
// Alignment
// multiline

#ifndef __INFO_H__
#define __INFO_H__

#ifdef AVS_LINUX
#include <uchar.h>
#endif
#include <sstream>
#include "internal.h"
#include <unordered_map>
#include <array>
#include <iomanip>
#include <vector>
#include <cstring>

class BitmapFont {

  int number_of_chars;
  std::string font_name;
  std::string font_filename;

public:
  const int width;
  const int height;
  const bool bold;
  std::vector<uint16_t> font_bitmaps;

  std::unordered_map<uint16_t, int> charReMap; // unicode code point vs. font image index

  void SaveAsC(const uint16_t* _codepoints);

  BitmapFont(int _number_of_chars, const uint16_t* _src_font_bitmaps, const uint16_t* _codepoints, int _w, int _h, std::string _font_name, std::string _font_filename, bool _bold, bool debugSave) :
    number_of_chars(_number_of_chars),
    font_name(_font_name),
    font_filename(_font_filename),
    width(_w), height(_h),
    bold(_bold)
    //font_bitmaps(_font_bitmaps),
  {
    //fixme: do not copy data
    font_bitmaps.resize(height * number_of_chars);
    std::memcpy(font_bitmaps.data(), _src_font_bitmaps, font_bitmaps.size() * sizeof(uint16_t));

    for (int i = 0; i < _number_of_chars; i++) {
      charReMap[_codepoints[i]] = i;
    }

    if (debugSave)
      SaveAsC(_codepoints);
  }

  // helper function for remapping a wchar_t string to font index entry list
  std::vector<int> remap(const std::wstring& s16);

  // generate outline on-the-fly
  void generateOutline(uint16_t* outlined, int fontindex) const;
};

std::unique_ptr<BitmapFont> GetBitmapFont(int size, const char* name, bool bold, bool debugSave);

void SimpleTextOutW(BitmapFont* current_font, const VideoInfo& vi, PVideoFrame& frame, int real_x, int real_y, std::wstring& text, bool fadeBackground, int textcolor, int halocolor, bool useHaloColor, int align);
void SimpleTextOutW_multi(BitmapFont* current_font, const VideoInfo& vi, PVideoFrame& frame, int real_x, int real_y, std::wstring& text, bool fadeBackground, int textcolor, int halocolor, bool useHaloColor, int align, int lsp);

// legacy function w/o outline, originally with ASCII input, background fading
void DrawStringPlanar(VideoInfo& vi, PVideoFrame& dst, int x, int y, const char* s);
void DrawStringYUY2(VideoInfo& vi, PVideoFrame& dst, int x, int y, const char* s);
void DrawStringRGB32(VideoInfo& vi, PVideoFrame& dst, int x, int y, const char* s);
void DrawStringRGB24(VideoInfo& vi, PVideoFrame& dst, int x, int y, const char* s);

#endif  // __INFO_H__
