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


#include "styles.h"
#include <wx/tokenzr.h>
#include <wx/log.h>

AssColor::AssColor()
{
    r=g=b=a=0;

}
AssColor::AssColor(wxColour kol, int alpha)
{
    a= (alpha != -1)? alpha : 0xFF - kol.Alpha();
	b=kol.Blue();
    g=kol.Green();
    r=kol.Red();
}
AssColor::~AssColor()
{
}

AssColor::AssColor(wxString kol)
{
	a=b=g=r=0;
	SetAss(kol);
}

void AssColor::SetAss(wxString kol)
{
	if(kol.IsNumber()){
		long ssakol=0;
		kol.ToLong(&ssakol,10);
		r=ssakol & 0xFF;
		g=(ssakol>>8) & 0xFF;
		b=(ssakol>>16) & 0xFF;
		a=(ssakol>>24) & 0xFF;

	}
	else{
		kol.Upper();
		bool ishtml = kol.StartsWith("#");
		wxString astr, rstr, gstr, bstr;
		kol.Replace("&","");kol.Replace("H","");kol.Replace("#","");
		if(kol.Len()>7){astr=kol.SubString(0,1);astr.ToLong(&a, 16);kol=kol.Mid(2);}
		rstr=kol.SubString(4,5), gstr=kol.SubString(2,3), bstr=kol.SubString(0,1);
		if(ishtml){wxString tmp = rstr; rstr=bstr; bstr= tmp;}
		rstr.ToLong(&r, 16);
		gstr.ToLong(&g, 16);
		bstr.ToLong(&b, 16);
	
	}
}

void AssColor::SetAlphaString(wxString alpha)
{
	alpha.Replace("&","");
	alpha.Replace("H","");
	alpha.ToLong(&a, 16);
}
void AssColor::SetWX(wxColour kol1, int alpha)
{
    a=alpha;
    b=kol1.Blue();
    g=kol1.Green();
    r=kol1.Red();
}


wxString AssColor::GetAss(bool alpha,bool style) {
	
	wxString k1=_T("&H");
	if (alpha) k1 << wxString::Format(_T("%02X"),a);
	k1 << wxString::Format(_T("%02X%02X%02X"),b,g,r);
	if (!style) k1<< _T("&");
	return k1;
}

wxColour AssColor::GetWX()
{
    wxColour kol;
    kol.Set(r,g,b,0xFF-a);
    return kol;
}

wxString AssColor::GetHex(bool alpha) const
{
	if (alpha && a)
		return wxString::Format("#%02X%02X%02X%02X", a, r, g, b);
	return wxString::Format("#%02X%02X%02X", r, g, b);
}


Styles::Styles()
{
    Name=_T("Default");
    Fontname=_T("Garamond");
    Fontsize=_T("30");
    PrimaryColour.SetAss(_T("&H00FFFFFF&"));
    SecondaryColour.SetAss(_T("&H000000FF&"));
    OutlineColour.SetAss(_T("&H00FF0000&"));
    BackColour.SetAss(_T("&H00000000&"));
    Bold=false;
    Italic=false;
    Underline=false;
    StrikeOut=false;
    ScaleX=_T("100");
    ScaleY=_T("100");
    Spacing=_T("0");
    Angle=_T("0");
    BorderStyle=false;
    Outline=_T("2");
    Shadow=_T("2");
    Alignment=_T("2");
    MarginL=_T("20");
    MarginR=_T("20");
    MarginV=_T("20");
    Encoding=_T("1");
	//iterator++;
}


Styles::Styles(wxString styledata,char form)
{
  bool isstyle=stylesplit(styledata, form);
}

Styles::~Styles()
{
}


bool Styles::stylesplit(wxString styledata, char form)
{
	wxStringTokenizer assstyle(styledata,_T(","));

	if (!assstyle.HasMoreTokens())return false;
	wxString token2 = assstyle.GetNextToken();
	Name=token2.AfterFirst(' ');


	if (!assstyle.HasMoreTokens())return false;
	Fontname = assstyle.GetNextToken();


	if (!assstyle.HasMoreTokens())return false;
	Fontsize = assstyle.GetNextToken();


	if (!assstyle.HasMoreTokens())return false;
	PrimaryColour.SetAss(assstyle.GetNextToken());


	if (!assstyle.HasMoreTokens())return false;
	SecondaryColour.SetAss(assstyle.GetNextToken());

	if(form<2){
		if (!assstyle.HasMoreTokens())return false;
		OutlineColour.SetAss(assstyle.GetNextToken());

		if (!assstyle.HasMoreTokens())return false;
		BackColour.SetAss(assstyle.GetNextToken());
	}
	else
	{
		if (!assstyle.HasMoreTokens())return false;
		assstyle.GetNextToken();

		if (!assstyle.HasMoreTokens())return false;
		OutlineColour.SetAss(assstyle.GetNextToken());

		BackColour=OutlineColour;
	}



	if (!assstyle.HasMoreTokens())return false;
	if(assstyle.GetNextToken()==_T("0")){Bold = false;}
	else{Bold = true;};



	if (!assstyle.HasMoreTokens())return false;
	if(assstyle.GetNextToken()==_T("0")){Italic = false;}
	else{Italic = true;};

	if(form<2){
		if (!assstyle.HasMoreTokens())return false;
		if(assstyle.GetNextToken()==_T("0"))
		{Underline = false;}else{Underline = true;};


		if (!assstyle.HasMoreTokens())return false;
		if(assstyle.GetNextToken()==_T("0"))
		{StrikeOut = false;}else{StrikeOut = true;};


		if (!assstyle.HasMoreTokens())return false;
		ScaleX = assstyle.GetNextToken();


		if (!assstyle.HasMoreTokens())return false;
		ScaleY = assstyle.GetNextToken();


		if (!assstyle.HasMoreTokens())return false;
		Spacing = assstyle.GetNextToken();


		if (!assstyle.HasMoreTokens())return false;
		Angle = assstyle.GetNextToken();

	}else{
		Underline = false;
		StrikeOut = false;
		ScaleX = "100";
		ScaleY = "100";
		Spacing= "0";
		Angle  = "0";
	}

	if (!assstyle.HasMoreTokens())return false;
	if(assstyle.GetNextToken()==_T("3")){BorderStyle = true;}else{BorderStyle = false;};


	if (!assstyle.HasMoreTokens())return false;
	Outline = assstyle.GetNextToken();


	if (!assstyle.HasMoreTokens())return false;
	Shadow = assstyle.GetNextToken();


	if (!assstyle.HasMoreTokens())return false;
	Alignment = assstyle.GetNextToken();

	if(form==2)
	{
		if(Alignment=="9"){Alignment="4";}
		else if(Alignment=="10"){Alignment="5";}
		else if(Alignment=="11"){Alignment="6";}
		else if(Alignment=="5"){Alignment="7";}
		else if(Alignment=="6"){Alignment="8";}
		else if(Alignment=="7"){Alignment="9";}
	}


	if (!assstyle.HasMoreTokens())return false;
	MarginL = assstyle.GetNextToken();


	if (!assstyle.HasMoreTokens())return false;
	MarginR = assstyle.GetNextToken();


	if (!assstyle.HasMoreTokens())return false;
	MarginV = assstyle.GetNextToken();

	if(form==2){
		if(!assstyle.HasMoreTokens())return false;
		assstyle.GetNextToken();
	}


	if (!assstyle.HasMoreTokens())return false;
	Encoding = assstyle.GetNextToken();
	Encoding.Trim(true);

	return true;
}

wxString Styles::styletext()
{
    wxString textfile;
    wxString bold=(Bold)?_T("-1"):_T("0"), italic=(Italic)?_T("-1"):_T("0"), underline=(Underline)?_T("-1"):_T("0"), strikeout=(StrikeOut)?_T("-1"):_T("0"),
		bordstyl=(BorderStyle)?_T("3"):_T("1");

    textfile<<_T("Style: ")<<Name<<_T(",")<<Fontname<<_T(",")<<Fontsize<<_T(",")<<PrimaryColour.GetAss(true,true)<<_T(",")
		<<SecondaryColour.GetAss(true,true)<<_T(",")<<OutlineColour.GetAss(true,true)<<_T(",")<<BackColour.GetAss(true,true)<<_T(",")<<bold<<_T(",")<<italic
		<<_T(",")<<underline<<_T(",")<<strikeout<<_T(",")<<ScaleX<<_T(",")<<ScaleY<<_T(",")<<Spacing<<_T(",")<<Angle
	    <<_T(",")<<bordstyl<<_T(",")<<Outline<<_T(",")<<Shadow<<_T(",")<<Alignment<<_T(",")<<MarginL<<_T(",")<<MarginR<<_T(",")<<MarginV<<_T(",")<<Encoding<<_T("\r\n");

    return textfile;
}

Styles *Styles::Copy()
{
	Styles *styl= new Styles();
	//if(!styl){return NULL;}
	styl->Alignment=Alignment;
	styl->Angle=Angle;
	styl->BackColour=BackColour;
	styl->Bold=Bold;
	styl->BorderStyle=BorderStyle;
	styl->Encoding=Encoding;
	styl->Fontname=Fontname;
	styl->Fontsize=Fontsize;
	styl->Italic=Italic;
	styl->MarginL=MarginL;
	styl->MarginR=MarginR;
	styl->MarginV=MarginV;
	styl->Name=Name;
	styl->Outline=Outline;
	styl->OutlineColour=OutlineColour;
	styl->PrimaryColour=PrimaryColour;
	styl->ScaleX=ScaleX;
	styl->ScaleY=ScaleY;
	styl->SecondaryColour=SecondaryColour;
	styl->Shadow=Shadow;
	styl->Spacing=Spacing;
	styl->StrikeOut=StrikeOut;
	styl->Underline=Underline;
	return styl;
}


SInfo::SInfo(wxString _Name, wxString _Val)
{
	Name=_Name;
	Val=_Val;
}
	
SInfo::SInfo()
{
}

SInfo *SInfo::Copy()
{
	SInfo *inf=new SInfo();
	//if(!inf){return NULL;}
	inf->Name=Name;
	inf->Val=Val;
	//inf->Scomment=Scomment;
	return inf;
}