//  Copyright (c) 2016 - 2020, Marcin Drob

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

//#include "Utils.h"
#include "styles.h"
#include <wx/tokenzr.h>
#include <wx/log.h>
#include "config.h"

AssColor::AssColor()
{
	r = g = b = a = 0;

}
AssColor::AssColor(const wxColour &col, int alpha)
{
	a = (alpha != -1) ? alpha : 0xFF - col.Alpha();
	b = col.Blue();
	g = col.Green();
	r = col.Red();
}
AssColor::~AssColor()
{
}

AssColor::AssColor(const wxString &col)
{
	a = b = g = r = 0;
	SetAss(col);
}

void AssColor::SetAss(wxString color)
{
	if (color.IsNumber()){
		long ssakol = 0;
		color.ToLong(&ssakol, 10);
		r = ssakol & 0xFF;
		g = (ssakol >> 8) & 0xFF;
		b = (ssakol >> 16) & 0xFF;
		a = (ssakol >> 24) & 0xFF;

	}
	else{
		color.Upper();
		bool ishtml = color.StartsWith(L"#");
		wxString astr, rstr, gstr, bstr;
		color.Replace(L"&", L"");
		color.Replace(L"H", L"");
		color.Replace(L"#", L"");
		if (color.length() > 7){ astr = color.SubString(0, 1); astr.ToLong(&a, 16); color = color.Mid(2); }
		rstr = color.SubString(4, 5), gstr = color.SubString(2, 3), bstr = color.SubString(0, 1);
		if (ishtml){ wxString tmp = rstr; rstr = bstr; bstr = tmp; }
		rstr.ToLong(&r, 16);
		gstr.ToLong(&g, 16);
		bstr.ToLong(&b, 16);

	}
}

void AssColor::SetAlphaString(wxString alpha)
{
	alpha.Replace(L"&", L"");
	alpha.Replace(L"H", L"");
	alpha.ToLong(&a, 16);
}
void AssColor::SetWX(const wxColour &kolor, int alpha)
{
	a = alpha;
	b = kolor.Blue();
	g = kolor.Green();
	r = kolor.Red();
}


wxString AssColor::GetAss(bool alpha, bool style) const {

	wxString k1 = _T("&H");
	if (alpha) k1 << wxString::Format(_T("%02X"), a);
	k1 << wxString::Format(_T("%02X%02X%02X"), b, g, r);
	if (!style) k1 << _T("&");
	return k1;
}

wxColour AssColor::GetWX() const
{
	wxColour kol;
	kol.Set(r, g, b, 0xFF - a);
	return kol;
}

wxString AssColor::GetHex(bool alpha) const
{
	if (alpha && a)
		return wxString::Format(L"#%02X%02X%02X%02X", a, r, g, b);
	return wxString::Format(L"#%02X%02X%02X", r, g, b);
}

void AssColor::Copy(const AssColor& color, bool alpha)
{
	r = color.r;
	g = color.g;
	b = color.b;
	if (alpha)
		a = color.a;
}

bool AssColor::NotEqual(const AssColor& color, bool alpha /*= false*/)
{
	if (alpha)
		return a != color.a || r != color.r || g != color.g || b != color.b;

	return r != color.r || g != color.g || b != color.b;
}

double Styles::GetOtlineDouble()
{
	double outline = 0.;
	if (!Outline.ToDouble(&outline))
		outline = wxAtoi(Outline);

	return outline;
}

double Styles::GetShadowDouble()
{
	double shadow = 0.;
	if (!Shadow.ToDouble(&shadow))
		shadow = wxAtoi(Shadow);

	return shadow;
}
double Styles::GetSpacingDouble()
{
	double spacing = 0.;
	if (!Spacing.ToDouble(&spacing))
		spacing = wxAtoi(Spacing);

	return spacing;
}
double Styles::GetAngleDouble()
{
	double angle = 0.;
	if (!Angle.ToDouble(&angle))
		angle = wxAtoi(Angle);

	return angle;
}
int Styles::GetAlignment()
{
	double align = 0.;
	if (!Alignment.ToDouble(&align))
		align = wxAtoi(Alignment);

	return align;
}

double Styles::GetScaleXDouble()
{
	double scalex = 0.;
	if (!ScaleX.ToDouble(&scalex))
		scalex = wxAtoi(ScaleX);

	return scalex;
}

double Styles::GetScaleYDouble()
{
	double scaley = 0.;
	if (!ScaleY.ToDouble(&scaley))
		scaley = wxAtoi(ScaleY);

	return scaley;
}

Styles::Styles()
{
	Name = _T("Default");
	Fontname = _T("Garamond");
	Fontsize = L"40";
	PrimaryColour.SetAss(_T("&H00FFFFFF&"));
	SecondaryColour.SetAss(_T("&H00000000&"));
	OutlineColour.SetAss(_T("&H00FF0000&"));
	BackColour.SetAss(_T("&H00000000&"));
	Bold = false;
	Italic = false;
	Underline = false;
	StrikeOut = false;
	ScaleX = _T("100");
	ScaleY = _T("100");
	Spacing = _T("0");
	Angle = _T("0");
	BorderStyle = false;
	Outline = _T("2");
	Shadow = _T("2");
	Alignment = _T("2");
	MarginL = _T("20");
	MarginR = _T("20");
	MarginV = _T("20");
	Encoding = _T("1");
	//iterator++;
}


Styles::Styles(wxString styledata, char form)
{
	bool isstyle = parseStyle(styledata, form);
}

Styles::~Styles()
{
}


bool Styles::parseStyle(const wxString &styledata, char form)
{
	wxStringTokenizer assstyle(styledata, _T(","));

	if (!assstyle.HasMoreTokens()) return false;
	wxString token2 = assstyle.GetNextToken();
	Name = token2.AfterFirst(L' ');


	if (!assstyle.HasMoreTokens()) return false;
	Fontname = assstyle.GetNextToken();


	if (!assstyle.HasMoreTokens()) return false;
	Fontsize = assstyle.GetNextToken();
	


	if (!assstyle.HasMoreTokens()) return false;
	PrimaryColour.SetAss(assstyle.GetNextToken());


	if (!assstyle.HasMoreTokens()) return false;
	SecondaryColour.SetAss(assstyle.GetNextToken());

	if (form < 2){
		if (!assstyle.HasMoreTokens()) return false;
		OutlineColour.SetAss(assstyle.GetNextToken());

		if (!assstyle.HasMoreTokens()) return false;
		BackColour.SetAss(assstyle.GetNextToken());
	}
	else
	{
		if (!assstyle.HasMoreTokens()) return false;
		assstyle.GetNextToken();

		if (!assstyle.HasMoreTokens()) return false;
		OutlineColour.SetAss(assstyle.GetNextToken());

		BackColour = OutlineColour;
	}



	if (!assstyle.HasMoreTokens()) return false;
	if (assstyle.GetNextToken() == _T("0")){ Bold = false; }
	else{ Bold = true; };



	if (!assstyle.HasMoreTokens()) return false;
	if (assstyle.GetNextToken() == _T("0")){ Italic = false; }
	else{ Italic = true; };

	if (form < 2){
		if (!assstyle.HasMoreTokens()) return false;
		if (assstyle.GetNextToken() == _T("0"))
		{
			Underline = false;
		}
		else{ Underline = true; };


		if (!assstyle.HasMoreTokens()) return false;
		if (assstyle.GetNextToken() == _T("0"))
		{
			StrikeOut = false;
		}
		else{ StrikeOut = true; };


		if (!assstyle.HasMoreTokens()) return false;
		ScaleX = assstyle.GetNextToken();


		if (!assstyle.HasMoreTokens()) return false;
		ScaleY = assstyle.GetNextToken();


		if (!assstyle.HasMoreTokens()) return false;
		Spacing = assstyle.GetNextToken();


		if (!assstyle.HasMoreTokens()) return false;
		Angle = assstyle.GetNextToken();

	}
	else{
		Underline = false;
		StrikeOut = false;
		ScaleX = L"100";
		ScaleY = L"100";
		Spacing = L"0";
		Angle = L"0";
	}

	if (!assstyle.HasMoreTokens()) return false;
	if (assstyle.GetNextToken() == _T("3")){ BorderStyle = true; }
	else{ BorderStyle = false; };


	if (!assstyle.HasMoreTokens()) return false;
	Outline = assstyle.GetNextToken();


	if (!assstyle.HasMoreTokens()) return false;
	Shadow = assstyle.GetNextToken();


	if (!assstyle.HasMoreTokens()) return false;
	Alignment = assstyle.GetNextToken();

	if (form == 2)
	{
		if (Alignment == L"9"){ Alignment = L"4"; }
		else if (Alignment == L"10"){ Alignment = L"5"; }
		else if (Alignment == L"11"){ Alignment = L"6"; }
		else if (Alignment == L"5"){ Alignment = L"7"; }
		else if (Alignment == L"6"){ Alignment = L"8"; }
		else if (Alignment == L"7"){ Alignment = L"9"; }
	}


	if (!assstyle.HasMoreTokens()) return false;
	MarginL = assstyle.GetNextToken();


	if (!assstyle.HasMoreTokens()) return false;
	MarginR = assstyle.GetNextToken();


	if (!assstyle.HasMoreTokens()) return false;
	MarginV = assstyle.GetNextToken();

	if (form == 2){
		if (!assstyle.HasMoreTokens()) return false;
		assstyle.GetNextToken();
	}


	if (!assstyle.HasMoreTokens()) return false;
	Encoding = assstyle.GetNextToken();
	Encoding.Trim(true);

	return true;
}

void Styles::CopyChanges(Styles *changedStyle, int whatToChange)
{
	if (whatToChange & STYLE_FONT_NAME){
		Fontname = changedStyle->Fontname;
	}
	if (whatToChange & STYLE_FONT_SIZE){
		Fontsize = changedStyle->Fontsize;
	}
	if (whatToChange & STYLE_FONT_BOLD){
		Bold = changedStyle->Bold;
	}
	if (whatToChange & STYLE_FONT_ITALIC){
		Italic = changedStyle->Italic;
	}
	if (whatToChange & STYLE_FONT_UNDERLINE){
		Underline = changedStyle->Underline;
	}
	if (whatToChange & STYLE_FONT_STRIKEOUT){
		StrikeOut = changedStyle->StrikeOut;
	}
	if (whatToChange & STYLE_COLOR_PRIMARY){
		PrimaryColour = changedStyle->PrimaryColour;
	}
	if (whatToChange & STYLE_COLOR_SECONDARY){
		SecondaryColour = changedStyle->SecondaryColour;
	}
	if (whatToChange & STYLE_COLOR_OUTLINE){
		OutlineColour = changedStyle->OutlineColour;
	}
	if (whatToChange & STYLE_COLOR_SHADOW){
		BackColour = changedStyle->BackColour;
	}
	if (whatToChange & STYLE_OUTLINE){
		Outline = changedStyle->Outline;
	}
	if (whatToChange & STYLE_SHADOW){
		Shadow = changedStyle->Shadow;
	}
	if (whatToChange & STYLE_SCALE_X){
		ScaleX = changedStyle->ScaleX;
	}
	if (whatToChange & STYLE_SCALE_Y){
		ScaleY = changedStyle->ScaleY;
	}
	if (whatToChange & STYLE_ANGLE){
		Angle = changedStyle->Angle;
	}
	if (whatToChange & STYLE_SPACING){
		Spacing = changedStyle->Spacing;
	}
	if (whatToChange & STYLE_BORDER_STYLE){
		BorderStyle = changedStyle->BorderStyle;
	}
	if (whatToChange & STYLE_ALIGNMENT){
		Alignment = changedStyle->Alignment;
	}
	if (whatToChange & STYLE_MARGIN_LEFT){
		MarginL = changedStyle->MarginL;
	}
	if (whatToChange & STYLE_MARGIN_RIGHT){
		MarginR = changedStyle->MarginR;
	}
	if (whatToChange & STYLE_MARGIN_VERTICAL){
		MarginV = changedStyle->MarginV;
	}
	if (whatToChange & STYLE_ENCODING){
		Encoding = changedStyle->Encoding;
	}
}

int Styles::Compare(Styles *changedStyle)
{
	int compareResult = 0;
	if (Fontname != changedStyle->Fontname){
		compareResult |= STYLE_FONT_NAME;
	}
	if (Fontsize != changedStyle->Fontsize){
		compareResult |= STYLE_FONT_SIZE;
	}
	if (Bold != changedStyle->Bold){
		compareResult |= STYLE_FONT_BOLD;
	}
	if (Italic != changedStyle->Italic){
		compareResult |= STYLE_FONT_ITALIC;
	}
	if (Underline != changedStyle->Underline){
		compareResult |= STYLE_FONT_UNDERLINE;
	}
	if (StrikeOut != changedStyle->StrikeOut){
		compareResult |= STYLE_FONT_STRIKEOUT;
	}
	if (PrimaryColour != changedStyle->PrimaryColour){
		compareResult |= STYLE_COLOR_PRIMARY;
	}
	if (SecondaryColour != changedStyle->SecondaryColour){
		compareResult |= STYLE_COLOR_SECONDARY;
	}
	if (OutlineColour != changedStyle->OutlineColour){
		compareResult |= STYLE_COLOR_OUTLINE;
	}
	if (BackColour != changedStyle->BackColour){
		compareResult |= STYLE_COLOR_SHADOW;
	}
	if (Outline != changedStyle->Outline){
		compareResult |= STYLE_OUTLINE;
	}
	if (Shadow != changedStyle->Shadow){
		compareResult |= STYLE_SHADOW;
	}
	if (ScaleX != changedStyle->ScaleX){
		compareResult |= STYLE_SCALE_X;
	}
	if (ScaleY != changedStyle->ScaleY){
		compareResult |= STYLE_SCALE_Y;
	}
	if (Angle != changedStyle->Angle){
		compareResult |= STYLE_ANGLE;
	}
	if (Spacing != changedStyle->Spacing){
		compareResult |= STYLE_SPACING;
	}
	if (BorderStyle != changedStyle->BorderStyle){
		compareResult |= STYLE_BORDER_STYLE;
	}
	if (Alignment != changedStyle->Alignment){
		compareResult |= STYLE_ALIGNMENT;
	}
	if (MarginL != changedStyle->MarginL){
		compareResult |= STYLE_MARGIN_LEFT;
	}
	if (MarginR != changedStyle->MarginR){
		compareResult |= STYLE_MARGIN_RIGHT;
	}
	if (MarginV != changedStyle->MarginV){
		compareResult |= STYLE_MARGIN_VERTICAL;
	}
	if (Encoding != changedStyle->Encoding){
		compareResult |= STYLE_ENCODING;
	}
	return compareResult;
}

wxString Styles::GetRaw()
{
	wxString textfile;
	wxString bold = (Bold) ? _T("-1") : _T("0"), italic = (Italic) ? _T("-1") : _T("0"), underline = (Underline) ? _T("-1") : _T("0"), strikeout = (StrikeOut) ? _T("-1") : _T("0"),
		bordstyl = (BorderStyle) ? _T("3") : _T("1");

	textfile << _T("Style: ") << Name << _T(",") << Fontname << _T(",") << Fontsize << _T(",") << PrimaryColour.GetAss(true, true) << _T(",")
		<< SecondaryColour.GetAss(true, true) << _T(",") << OutlineColour.GetAss(true, true) << _T(",") << BackColour.GetAss(true, true) << _T(",") << bold << _T(",") << italic
		<< _T(",") << underline << _T(",") << strikeout << _T(",") << ScaleX << _T(",") << ScaleY << _T(",") << Spacing << _T(",") << Angle
		<< _T(",") << bordstyl << _T(",") << Outline << _T(",") << Shadow << _T(",") << Alignment << _T(",") << MarginL << _T(",") << MarginR << _T(",") << MarginV << _T(",") << Encoding << _T("\r\n");

	return textfile;
}

Styles *Styles::Copy()
{
	Styles *styl = new Styles();
	//if(!styl){return NULL;}
	styl->Alignment = Alignment;
	styl->Angle = Angle;
	styl->BackColour = BackColour;
	styl->Bold = Bold;
	styl->BorderStyle = BorderStyle;
	styl->Encoding = Encoding;
	styl->Fontname = Fontname;
	styl->Fontsize = Fontsize;
	styl->Italic = Italic;
	styl->MarginL = MarginL;
	styl->MarginR = MarginR;
	styl->MarginV = MarginV;
	styl->Name = Name;
	styl->Outline = Outline;
	styl->OutlineColour = OutlineColour;
	styl->PrimaryColour = PrimaryColour;
	styl->ScaleX = ScaleX;
	styl->ScaleY = ScaleY;
	styl->SecondaryColour = SecondaryColour;
	styl->Shadow = Shadow;
	styl->Spacing = Spacing;
	styl->StrikeOut = StrikeOut;
	styl->Underline = Underline;
	return styl;
}


SInfo::SInfo(wxString _Name, wxString _Val)
{
	Name = _Name;
	Val = _Val;
}

SInfo::SInfo()
{
}

SInfo *SInfo::Copy()
{
	SInfo *inf = new SInfo();
	//if(!inf){return NULL;}
	inf->Name = Name;
	inf->Val = Val;
	//inf->Scomment=Scomment;
	return inf;
}

double Styles::GetFontSizeDouble()
{
	//try to convert to double if failed then to int
	//if there are plain text it returns 0
	double fontSize = 0.;
	if (!Fontsize.ToDouble(&fontSize))
		fontSize = wxAtoi(Fontsize);

	return fontSize;
}

void Styles::SetFontSizeDouble(double size)
{
	Fontsize = getfloat(size);
}