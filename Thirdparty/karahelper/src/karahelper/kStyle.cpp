// Style functions

#include "stdafx.h"

// constructor
kStyle::kStyle()
{
    defvalue();
}

// copy constructor
kStyle::kStyle(kStyle& s2)
{
    CopyStyle(s2);
}

void kStyle::CopyStyle(kStyle& s2)
{
    n_margin.bottom = s2.n_margin.bottom;
    n_margin.top = s2.n_margin.top;
    n_margin.left = s2.n_margin.left;
    n_margin.right = s2.n_margin.right;
    n_alignment = s2.n_alignment;
    n_border = s2.n_border;
    n_outlineX = s2.n_outlineX;
    n_outlineY = s2.n_outlineY;
    n_shadowX = s2.n_shadowX;
    n_shadowY = s2.n_shadowY;
    n_colors[0] = s2.n_colors[0];
    n_colors[1] = s2.n_colors[1];
    n_colors[2] = s2.n_colors[2];
    n_colors[3] = s2.n_colors[3];
    n_charset = s2.n_charset;
    n_fontname = s2.n_fontname;
    n_fontsize = s2.n_fontsize;
    n_fontscaleX = s2.n_fontscaleX;
    n_fontscaleY = s2.n_fontscaleY;
    n_fontspacing = s2.n_fontspacing;
    n_fontweight = s2.n_fontweight;
    n_f_italic = s2.n_f_italic;
    n_f_underline = s2.n_f_underline;
    n_f_strikeout = s2.n_f_strikeout;
    n_angleZ = s2.n_angleZ;
    n_angleX = s2.n_angleX;
    n_angleY = s2.n_angleY;
    n_blur = s2.n_blur;
    n_be = s2.n_be;
}

// default style params
void kStyle::defvalue()
{
    n_stylename = L"Default";
    n_margin.bottom = 20;
    n_margin.top = 20;
    n_margin.left = 20;
    n_margin.right = 20;
    n_alignment = 2;
    n_border = 0;
    n_outlineX = n_outlineY = 2;
    n_shadowX = n_shadowY = 3;
    n_colors[0] = 0x00ffffff;
    n_colors[1] = 0x0000ffff;
    n_colors[2] = 0x00000000;
    n_colors[3] = 0x00000000;
    n_charset = DEFAULT_CHARSET;
    n_fontname = L"Arial";
    n_fontsize = 18;
    n_fontscaleX = n_fontscaleY = 100;
    n_fontspacing = 0;
    n_fontweight = FW_BOLD;
    n_f_italic = false;
    n_f_underline = false;
    n_f_strikeout = false;
    n_angleZ = n_angleX = n_angleY = 0;
    n_blur = 0;
    n_be = 0;
}

void kStyle::operator = (kStyle& s2)
{
    CopyStyle(s2);
}

bool kStyle::operator == (kStyle& s2)
{
    return(n_alignment == s2.n_alignment
           && n_margin.bottom == s2.n_margin.bottom
           && n_margin.top == s2.n_margin.top
           && n_margin.left == s2.n_margin.left
           && n_margin.right == s2.n_margin.right
           && n_border == s2.n_border
           && n_outlineX == s2.n_outlineX
           && n_outlineY == s2.n_outlineY
           && n_shadowX == s2.n_shadowX
           && n_shadowY == s2.n_shadowY
           // colors
           && n_colors[0] == s2.n_colors[0]
           && n_colors[1] == s2.n_colors[1]
           && n_colors[2] == s2.n_colors[2]
           && n_colors[3] == s2.n_colors[3]
           // blurs
           && n_be == s2.n_be
           && n_blur == s2.n_blur
           // fonts
           && isFontEqual(s2));
}

bool kStyle::isFontEqual(kStyle& s2)
{
    return(n_charset == s2.n_charset
           && n_fontname == s2.n_fontname
           && n_fontsize == s2.n_fontsize
           && n_fontscaleX == s2.n_fontscaleX
           && n_fontscaleY == s2.n_fontscaleY
           && n_fontspacing == s2.n_fontspacing
           && n_fontweight == s2.n_fontweight
           && n_f_italic == s2.n_f_italic
           && n_f_underline == s2.n_f_underline
           && n_f_strikeout == s2.n_f_strikeout
          );
}

LOGFONTW& operator <<= (LOGFONTW& lfw, kStyle& s)
{
    lfw.lfCharSet = s.n_charset;
    wcsncpy_s(lfw.lfFaceName, LF_FACESIZE, s.n_fontname.c_str(), _TRUNCATE);
    HDC hDC = GetDC(0);
    lfw.lfHeight = -MulDiv((int)(s.n_fontsize + 0.5), GetDeviceCaps(hDC, LOGPIXELSY), 72);
    ReleaseDC(0, hDC);
    lfw.lfWeight = s.n_fontweight;
    lfw.lfItalic = s.n_f_italic ? -1 : 0;
    lfw.lfUnderline = s.n_f_underline ? -1 : 0;
    lfw.lfStrikeOut = s.n_f_strikeout ? -1 : 0;
    return(lfw);
}

std::wstring kStyle::getAssFile(std::wstring text, int w, int h)
{
    // header
    int bluradjust = 0;
    if(n_blur > 0)
        bluradjust += (int)(n_blur * 3) | 1;
    if(n_be)
        bluradjust += 1;
    if(n_blur == 0)
        bluradjust += (int)n_outlineY;

    std::wstring temp = L"[Script Info]\r\n;ASS temp file\r\nScriptType: v4.00+\r\nCollisions: Normal\r\nScaledBorderAndShadow: Yes\r\n";
    temp = temp + format("PlayResX: %d\r\nPlayResY: %d\r\nTimer: 100.0000\r\n\r\n", w, h);
    // styles
    temp = temp + L"[V4+ Styles]\r\nFormat: Name, Fontname, Fontsize, PrimaryColour, SecondaryColour, OutlineColour, BackColour, Bold, Italic, Underline, StrikeOut, ScaleX, ScaleY, Spacing, Angle, BorderStyle, Outline, Shadow, Alignment, MarginL, MarginR, MarginV, Encoding\r\n";
    temp = temp + format("Style: Default,%s,%d,&H%08x,&H%08x,&H%08x,&H%08x,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n\r\n", \
                         _WS(n_fontname).c_str(), (int)n_fontsize, n_colors[0], n_colors[1], n_colors[2], n_colors[3], (n_fontweight > FW_NORMAL) ? -1 : 0, \
                         (n_f_italic) ? -1 : 0, (n_f_underline) ? -1 : 0, (n_f_strikeout) ? -1 : 0, (int)n_fontscaleX, (int)n_fontscaleY, (int)n_fontspacing, \
                         (int)n_angleZ, (n_border == 0) ? 1 : ((n_border == 1) ? 3 : 0), (int)n_outlineY, (int)n_shadowY, 7 /*align*/, bluradjust, 0, 0, n_charset);
    // events
    temp = temp + L"[Events]\r\nFormat: Layer, Start, End, Style, Actor, MarginL, MarginR, MarginV, Effect, Text\r\n";
    temp = temp + L"Dialogue: 0,0:00:00.00,0:00:05.00,Default,,0000,0000,0000,,";
    if(n_blur > 0)
        temp = temp + format("{\\blur%f.2}", n_blur);
    if(n_be > 0)
        temp = temp + format("{\\be%d}", n_be);
    temp = temp + text;
    temp = temp + L"\r\n";

    return temp;
}

// Override tags
void kStyle::mod_be(nStrArray& params)		// be
{
    if(params.size() == 1) return;
    n_be = get_i(params[1]);
}

void kStyle::mod_blur(nStrArray& params)	// blur
{
    if(params.size() == 1) return;
    n_blur = get_f(params[1]); // destination
}

void kStyle::mod_fs(nStrArray& params)		// fs
{
    if(params.size() == 1) return;
    n_fontsize = get_f(params[1]); // destination
}

void kStyle::mod_fscx(nStrArray& params)	// fscx
{
    if(params.size() == 1) return;
    n_fontscaleX = get_f(params[1]); // destination
}

void kStyle::mod_fscy(nStrArray& params)	// fscy
{
    if(params.size() == 1) return;
    n_fontscaleY = get_f(params[1]); // destination
}

void kStyle::mod_fsp(nStrArray& params)		// fsp
{
    if(params.size() == 1) return;
    n_fontspacing = get_f(params[1]); // destination
}
