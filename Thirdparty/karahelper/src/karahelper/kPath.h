// Path class

#pragma once
#include "stdafx.h"
#include <math.h> // fabs, sqrt, sin, cos
#include "kStyle.h"
#include "kUtils.h"

// GetPath point types
#define PT_BSPLINETO 0xfc
#define PT_BSPLINEPATCHTO 0xfa

class kPath
{
private:
    HDC n_DC;
    BYTE* n_pFlag;
    POINT* n_pPoint;
    size_t n_pCount;

    std::wstring n_temptext;
    int	n_lasttype;

    void ClearPath();
    void ChangeCS();

    void CreateDrawing();

public:
    kPath();

    void Init(HDC dc);
    void _BeginPath();
    bool _EndPath();
    std::wstring _Convert();
};