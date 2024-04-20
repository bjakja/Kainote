//  Copyright (c) 2016 - 2021, Marcin Drob

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

#pragma once

#include <wx/colour.h>



class AssColor
{
	public:
	long r, g, b, a;
	wxString GetAss(bool alpha, bool style = false) const;
	wxString GetAlpha() const;
	wxColour GetWX() const;
	//no const cause it replaces this string
	void SetAss(wxString color);
	void SetWX(const wxColour &color, int alpha);
	wxString GetHex(bool alpha) const;
	void Copy(const AssColor& color, bool alpha = false);
	bool NotEqual(const AssColor& color, bool alpha = false);
	//no const cause it replaces this string
	void SetAlphaString(wxString alpha);
	AssColor();
	AssColor(const wxString &col);
	AssColor(const wxColour &col, int alpha = -1);
	AssColor(int red, int green, int blue, int alpha = -1) {
		r = red;
		g = green;
		b = blue;
		if (alpha > -1)
			a = alpha;
	}
	~AssColor();
	bool operator == (const AssColor &col){ return (a == col.a && r == col.r && g == col.g && b == col.b); }
	bool operator != (const AssColor &col){ return (a != col.a || r != col.r || g != col.g || b != col.b); }

};


class Styles
{
public:
	wxString Name;
	wxString Fontname, ScaleX, ScaleY, Spacing, Angle, Outline, Shadow, Alignment, MarginL, MarginR, MarginV, Encoding;
	//wxString Scomment;
	wxString Fontsize;
	AssColor PrimaryColour, SecondaryColour, OutlineColour, BackColour;
	bool Bold, Italic, Underline, StrikeOut, BorderStyle;
	wxString GetRaw();
	wxString GetFontSizeString(){ return Fontsize; };
	double GetFontSizeDouble();
	void SetFontSizeString(const wxString &size){ Fontsize = size; };
	void SetFontSizeDouble(double size);
	double GetOtlineDouble();
	double GetShadowDouble();
	double GetSpacingDouble();
	double GetAngleDouble();
	int GetAlignment();
	double GetScaleXDouble();
	double GetScaleYDouble();
	Styles();
	Styles(wxString styledata, char format = 1);
	~Styles();
	bool parseStyle(const wxString &styledata, char form);
	//no releasing here changes of style is copied to multiple styles
	//int list of changes needed
	//also enum with style element names 
	void CopyChanges(Styles *changedStyle, int whatToChange);
	int Compare(Styles *changedStyle);
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

enum{
	STYLE_FONT_NAME = 1,
	STYLE_FONT_SIZE,
	STYLE_FONT_BOLD = 4,
	STYLE_FONT_ITALIC = 8,
	STYLE_FONT_UNDERLINE = 16,
	STYLE_FONT_STRIKEOUT = 32,
	STYLE_COLOR_PRIMARY = 64,
	STYLE_COLOR_SECONDARY = 128,
	STYLE_COLOR_OUTLINE = 1 << 8,
	STYLE_COLOR_SHADOW = 1 << 9,
	STYLE_OUTLINE = 1 << 10,
	STYLE_SHADOW = 1 << 11,
	STYLE_SCALE_X = 1 << 12,
	STYLE_SCALE_Y = 1 << 13,
	STYLE_ANGLE = 1 << 14,
	STYLE_SPACING = 1 << 15,
	STYLE_BORDER_STYLE = 1 << 16,
	STYLE_ALIGNMENT = 1 << 17,
	STYLE_MARGIN_LEFT = 1 << 18,
	STYLE_MARGIN_RIGHT = 1 << 19,
	STYLE_MARGIN_VERTICAL = 1 << 20,
	STYLE_ENCODING = 1 << 21,
};

enum {
	ASS = 1,
	SRT,
	TMP,
	MDVD,
	MPL2,
	FRAME = 10
};