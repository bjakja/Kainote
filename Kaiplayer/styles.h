#ifndef STYLES_H_INCLUDED
#define STYLES_H_INCLUDED

#include <wx/colour.h>



class AssColor
{
    public:
    long r,g,b,a;
    wxString GetAss(bool alpha, bool style=false);
    wxColour GetWX();
    void SetAss(wxString kolor);
    void SetWX(wxColour kolor, int alpha);
	wxString GetHex(bool alpha) const;

    AssColor();
	AssColor(wxString kol);
	AssColor(wxColour kol);
    ~AssColor();

};

class Styles
{
public:
	wxString Name;
	wxString Fontname, Fontsize, ScaleX, ScaleY, Spacing, Angle, Outline, Shadow, Alignment, MarginL, MarginR, MarginV, Encoding;
	//wxString Scomment;
	AssColor PrimaryColour, SecondaryColour, OutlineColour, BackColour;
	bool Bold, Italic, Underline, StrikeOut, BorderStyle;
	wxString styletext();
    Styles();
    Styles(wxString styledata,char format=1);
    ~Styles();
    bool stylesplit(wxString styledata, char form);
	Styles *Copy();
};

class SInfo
{
public:
	wxString Name;
	wxString Val;
	//wxString Scomment;
	SInfo(wxString _Name, wxString _Val);
	SInfo();
	SInfo *Copy();
};

#endif // STYLES_H_INCLUDED
