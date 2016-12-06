//  Copyright (c) 2016, Marcin Drob

//  Kainote is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.

//  Kainote is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.

//  You should have received a copy of the GNU General Public License
//  along with Kainote.  If not, see <http://www.gnu.org/licenses/>.

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
	void SetAlphaString(wxString alpha);
    AssColor();
	AssColor(wxString kol);
	AssColor(wxColour kol, int alpha = -1);
    ~AssColor();
	bool operator == (const AssColor &col){return (a==col.a && r==col.r && g==col.g && b==col.b);}
	bool operator != (const AssColor &col){return (a!=col.a || r!=col.r || g!=col.g || b!=col.b);}

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
