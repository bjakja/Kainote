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

#include "SubsTime.h"
#include "Config.h"
#include "KaraokeSplitting.h"//zeroit


STime::STime(){
	mstime=0;
	form=ASS;
	orgframe=0;
}

STime::STime(int ms){
	mstime=ms;
	form=ASS;
	orgframe=0;
}

STime::~STime(){
}

void STime::SetRaw(wxString rawtime, char format)
{
	form=format;
	ParseMS(rawtime);
}

void STime::ParseMS(wxString raw)
{   

	int csec1=0,sec1,min1,godz1;
	if(raw.Trim()==""){mstime=0;}
	else if (form<MDVD){
		wxString csec,sec,min,godz;
		size_t godz11=raw.find(_T(":"),0);
		godz=raw.SubString(0,godz11-1);
		//kkk<<godz;
		godz1=wxAtoi(godz);
		min=raw.SubString(godz11+1,godz11+2);
		//kkk<<min;
		min1=wxAtoi(min);
		sec=raw.SubString(godz11+4,godz11+5);
		//kkk<<sec;
		sec1=wxAtoi(sec);
		if(form<SRT){
			csec=raw.SubString(godz11+7,godz11+8);
			//kkk<<csec;
			csec1=wxAtoi(csec)*10;}
		else if(form==SRT){
			csec=raw.SubString(godz11+7,godz11+9);
			//kkk<<csec;
			csec1=wxAtoi(csec);}

		mstime=(godz1*3600000)+(min1*60000)+(sec1*1000)+csec1;
	}else if(form==MDVD||form==MPL2){   
		//float afps=(form==MPL2)?10 : Options.GetFloat("Default FPS");
		int ress=wxAtoi(raw);
		if(form==MDVD){orgframe=ress;mstime=(ress/Options.GetFloat("Default FPS"))*(1000);if(orgframe<0){orgframe=0;}}
		else{mstime=(ress/10)*(1000);}
	}


}

wxString STime::raw(char ft)//,float custfps
{
	wxString rawtxt;
	if(form==SRT){mstime=ZEROIT(mstime);}
	if(ft==0){ft=form;}
	if(ft<SRT)
	{
		int csec=mstime/10;
		int sec=mstime/1000;
		int min=mstime/60000;
		int godz=mstime/3600000;
		rawtxt = wxString::Format(_T("%01i:%02i:%02i.%02i"),godz,(min%60),(sec%60),(csec%100));
	}else if(ft==TMP)
	{
		int sec=mstime/1000;
		int min=mstime/60000;
		int godz=mstime/3600000;
		rawtxt = wxString::Format(_T("%02i:%02i:%02i"),godz,(min%60),(sec%60));
	}else if(ft==MDVD||ft==MPL2)
	{
		//float fps=(custfps>0)?custfps:Options.GetFloat(_T("Default FPS"));
		//if(fps<1){fps=23.976f;}
		//float afps=(ft==MPL2)?10 : fps;
		int czas=ceil(mstime*(10.0f/1000));
		rawtxt = wxString::Format(_T("%i"),(ft==MDVD)? orgframe : czas);
	}else if(ft==SRT)
	{
		int sec=mstime/1000;
		int min=mstime/60000;
		int godz=mstime/3600000;
		rawtxt = wxString::Format(_T("%02i:%02i:%02i,%03i"),godz,(min%60),(sec%60),(mstime%1000));
	}
	form=ft;
	return rawtxt;
}

void STime::Change(int ms)
{
	mstime+=ms;if(mstime<0){mstime=0;}
	//SetFormat(form);
}
void STime::NewTime(int ms)
{
	mstime=ms;if(mstime<0){mstime=0;}
	if(form==MDVD){
		float fpsa=Options.GetFloat(_T("Default FPS"));
		if(fpsa<1){fpsa=23.976f;}
		orgframe=ceil(mstime*(fpsa/1000));
	}
	//SetFormat(form);
}
char STime::GetFormat()
{
	return form;
}
void STime::ChangeFormat(char format,float fps)
{
	if(form==MDVD){
		float fpsa=(fps)?fps:Options.GetFloat("Default FPS");
		if(fpsa<1){fpsa=23.976f;}
		mstime=(orgframe/fpsa)*(1000);
	}else if(format==MDVD){
		float fpsa=(fps)?fps:Options.GetFloat(_T("Default FPS"));
		if(fpsa<1){fpsa=23.976f;}
		orgframe=ceil(mstime*(fpsa/1000));
	}
	form=format;
}

wxString STime::GetFormatted(char format)
{
	return raw(format);
}

bool STime::operator> (STime por)
{
	return mstime>por.mstime;
}

bool STime::operator< (STime por)
{
	return mstime<por.mstime;
}

bool STime::operator>= (STime por)
{
	return mstime>por.mstime;
}

bool STime::operator<= (STime por)
{
	return mstime<por.mstime;
}

bool STime::operator== (STime por)
{
	return mstime==por.mstime;
}

STime STime::operator- (STime por)
{
	STime tmp;
	tmp.mstime=mstime-por.mstime;
	return tmp;
}