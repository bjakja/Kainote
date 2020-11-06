// Path


#include "stdafx.h"

kPath::kPath()
{
    // some
}

void kPath::Init(HDC dc)
{
    n_DC = dc;
    n_pCount = 0;
}

void kPath::ClearPath()
{
    if(n_pCount == 0) return;
    delete [] n_pFlag;
    delete [] n_pPoint;
    n_pFlag = NULL;
    n_pPoint = NULL;
    n_pCount = 0;
}

void kPath::_BeginPath()
{
    ClearPath();
    BeginPath(n_DC);
//		ReportInfo(format("BeginPath: error %d", GetLastError()));
}

bool kPath::_EndPath()
{
    CloseFigure(n_DC);
    if(EndPath(n_DC) != 0)
    {
        n_pCount = GetPath(n_DC, NULL, NULL, 0); // number of points enumerated
        if(!n_pCount) return true;

        // allocate some nya memory~
        n_pFlag = (BYTE*)malloc(sizeof(BYTE) * n_pCount);
        n_pPoint = (POINT*)malloc(sizeof(POINT) * n_pCount);

        if(n_pCount == GetPath(n_DC, n_pPoint, n_pFlag, (int)n_pCount)) return true;
    }
    else
    {
//		ReportInfo(format("EndPath: error %d", GetLastError()));
        AbortPath(n_DC);
    }
    return true;
}

std::wstring kPath::_Convert()
{
    // modify coordinate system
    ChangeCS();
    CreateDrawing();

    return n_temptext.substr(1);
}

void kPath::ChangeCS()
{
    // calculate bounds
    ptrdiff_t minx = INT_MAX;
    ptrdiff_t miny = INT_MAX;
    ptrdiff_t maxx = INT_MIN;
    ptrdiff_t maxy = INT_MIN;

    for(size_t i = 0; i < n_pCount; ++i)
    {
        ptrdiff_t ix = n_pPoint[i].x;
        ptrdiff_t iy = n_pPoint[i].y;

        if(ix < minx) minx = ix;
        if(ix > maxx) maxx = ix;
        if(iy < miny) miny = iy;
        if(iy > maxy) maxy = iy;
    }

    // calc size
    // <.<

    // move CS
    for(size_t i = 0; i < n_pCount; ++i)
    {
        n_pPoint[i].x -= (LONG)minx;
        n_pPoint[i].y -= (LONG)miny;
    }
}

void kPath::CreateDrawing()
{
    n_temptext = L"";
    n_lasttype = 0;

    // all points
    for(size_t i = 0; i < n_pCount; ++i)
    {
        switch(n_pFlag[i]) // select type of point
        {
        case PT_MOVETO:
            n_temptext = n_temptext + format(" m %d %d", n_pPoint[i].x, n_pPoint[i].y);
            n_lasttype = 1; // move
            break;
        case PT_LINETO:
            if(n_pCount - (i - 1) >= 2)
            {
                if(n_lasttype != 2) n_temptext = n_temptext + L" l";
                n_temptext = n_temptext + format(" %d %d", n_pPoint[i].x, n_pPoint[i].y);
                n_lasttype = 2; // line
            }
            break;
        case PT_BEZIERTO:
            if(n_pCount - (i - 1) >= 4)
            {
                n_temptext = n_temptext + format(" b %d %d %d %d %d %d", n_pPoint[i].x, n_pPoint[i].y,
                                                 n_pPoint[i+1].x, n_pPoint[i+1].y, n_pPoint[i+2].x, n_pPoint[i+2].y);
                n_lasttype = 3; // bezier
            }
            i += 2;
            break;
        case PT_BSPLINETO:
            if(n_pCount - (i - 1) >= 4)
            {
                if(n_lasttype != 4) n_temptext = n_temptext + L" s";
                n_temptext = n_temptext + format(" %d %d %d %d %d %d", n_pPoint[i].x, n_pPoint[i].y,
                                                 n_pPoint[i+1].x, n_pPoint[i+1].y, n_pPoint[i+2].x, n_pPoint[i+2].y);
                n_lasttype = 4; // b-spline
            }
            i += 2;
            break;
        case PT_CLOSEFIGURE:
            n_temptext = n_temptext + L" c";
            n_lasttype = 5; // close
            break;
        }
    }
    // free path data
    ClearPath();
}
