#pragma once
#include <wx/string.h>
//#include <Windows.h>
typedef unsigned char BYTE;
//#include "streams.h"
//#include "Dvdmedia.h"
//#include "wmsdkidl.h"

static long int crv_tab[256];
static long int cbu_tab[256];
static long int cgu_tab[256];
static long int cgv_tab[256];
static long int tab_76309[256];
static unsigned char clp[1024];	

class CColorSpaceConverter
{
public:
	CColorSpaceConverter(int fmt, int width, int height);
	virtual ~CColorSpaceConverter(void);

	BYTE* convert_to_rgb24(BYTE* frameBuffer);
	void SavePNG(wxString path, BYTE* frameBuffer);
	void SavetoClipboard(BYTE* frameBuffer);

private:
	BYTE* m_pRgbaBuffer;

	int fmt;
	int m_width; 
	int m_height;

	int m_uPlanePos;
	int m_vPlanePos;
};

