// nStyle.h
// style definition
// style classes
// modificators

#pragma once
#include "stdafx.h"
#include "kUtils.h"

class kStyle
{
public:
    // global style parameters
    std::wstring n_stylename;
    RECT	n_margin;		// measured from the sides
    int		n_alignment;	// 1 - 9: as on the numpad, 0: default
    int		n_border;	// 0: outline, 1: opaque box
    double	n_outlineX, n_outlineY;
    double	n_shadowX, n_shadowY;
    DWORD	n_colors[4];		// usually: {primary, secondary, outline/background, shadow} ABGR
    int		n_charset;
    std::wstring n_fontname;
    double	n_fontsize;		// height
    double	n_fontscaleX, n_fontscaleY; // percent
    double	n_fontspacing;	// +/- pixels
    int		n_fontweight;
    bool	n_f_italic;
    bool	n_f_underline;
    bool	n_f_strikeout;
    double	n_angleZ, n_angleX, n_angleY;
    int		n_rel;		// relative, 0: window, 1: video
    double	n_blur;
    int		n_be;

    kStyle();
    kStyle(kStyle& s2);

    bool isFontEqual(kStyle& s2);
    void defvalue();
    void CopyStyle(kStyle& s2);

    // for override tags
    void mod_be(nStrArray& params);		// be
    void mod_blur(nStrArray& params);	// blur
    void mod_fs(nStrArray& params);		// fs
    void mod_fscx(nStrArray& params);	// fscx
    void mod_fscy(nStrArray& params);	// fscy
    void mod_fsp(nStrArray& params);	// fsp

    bool operator == (kStyle& s2);
    void operator = (kStyle& s2);

    std::wstring getAssFile(std::wstring text, int w, int h);
};

struct kFontStruct
{
    double k_width;
    double k_height;
    double k_ascent;
    double k_descent;
    double k_extlead;
};

LOGFONTW& operator <<= (LOGFONTW& lfw, kStyle& s);