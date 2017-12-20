// Renderer

#include "stdafx.h"

kRenderer::kRenderer(HDC DC)
{
    n_DC = DC;
    n_path.Init(DC);
}

std::wstring kRenderer::Render(kStyle& style, std::wstring text)
{
    n_style = &style;
    n_text = text;

    //init font
    initFont();

    // create path
    if(!Path()) return L"";

    return n_path._Convert();
}

bool kRenderer::Path()
{
    int n_width = 0;
    if(n_style->n_fontspacing || (long)GetVersion() < 0)
    {
        n_path._BeginPath();
        for(LPCWSTR s = n_text.c_str(); *s; s++) // chars
        {
            SIZE extent;
            if(!GetTextExtentPoint32W(n_DC, s, 1, &extent))
            {
                return false;
            }
            TextOutW(n_DC, n_width, 0, s, 1);

            n_width += extent.cx + (int)n_style->n_fontspacing;
        }
        n_path._EndPath();
    }
    else
    {
        // full
        SIZE extent;
        if(!GetTextExtentPoint32W(n_DC, n_text.c_str(), (int)n_text.length(), &extent))
        {
            return false;
        }

        n_path._BeginPath();
        TextOutW(n_DC, 0, 0, n_text.c_str(), (int)n_text.length());
        n_path._EndPath();
    }
    // ok, now we have path
    return true;
}

void kRenderer::initFont()
{
    LOGFONTW lf;
    memset(&lf, 0, sizeof(lf));
    lf <<= *n_style;
    lf.lfHeight = (LONG)(n_style->n_fontsize + 0.5);
    lf.lfOutPrecision = OUT_TT_PRECIS;
    lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
    lf.lfQuality = ANTIALIASED_QUALITY;
    lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;

    HFONT nFont = CreateFontIndirect(&lf);
    if(!nFont)
    {
        wcsncpy_s(lf.lfFaceName, LF_FACESIZE, L"Arial", _TRUNCATE);
        nFont = CreateFontIndirect(&lf);
    }

    HFONT hOldFont = SelectFont(n_DC, nFont);
}