// Renderer
#pragma once
#include "stdafx.h"
#include "kStyle.h"	// Styles

#include "kPath.h" // path

class kRenderer
{
private:
    kStyle* n_style;
    std::wstring n_text;
    // some nya
    void initFont();	// 1
    bool Path();		// 2

public:
    HDC	n_DC;	// device context

    kPath n_path;

    kRenderer(HDC DC);
    std::wstring Render(kStyle& style, std::wstring text);
};