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

#include "SubsTime.h"
#include "Config.h"
#include "SubsDialogue.h"
#include <wx/log.h>

SubsTime::SubsTime(){
	mstime = 0;
	form = ASS;
	orgframe = 0;
}

SubsTime::SubsTime(int ms, int orgFrame){
	mstime = ms;
	form = ASS;
	orgframe = orgFrame;
}

SubsTime::~SubsTime(){
}

void SubsTime::SetRaw(wxString rawtime, char format)
{
	form = format;
	ParseMS(rawtime);
}

void SubsTime::ParseMS(wxString raw)
{

	int csec1 = 0, sec1, min1, godz1;
	if (raw.Trim() == emptyString){ mstime = 0; orgframe = 0; }
	else if (form < MDVD){
		wxString csec, sec, min, godz;
		size_t godz11 = raw.find(L":", 0);
		godz = raw.SubString(0, godz11 - 1);
		godz1 = wxAtoi(godz);
		min = raw.SubString(godz11 + 1, godz11 + 2);
		min1 = wxAtoi(min);
		sec = raw.SubString(godz11 + 4, godz11 + 5);
		sec1 = wxAtoi(sec);
		if (form < SRT){
			csec = raw.SubString(godz11 + 7, godz11 + 8);
			csec1 = wxAtoi(csec) * 10;
		}
		else if (form == SRT){
			csec = raw.SubString(godz11 + 7, godz11 + 9);
			csec1 = wxAtoi(csec);
		}

		mstime = (godz1 * 3600000) + (min1 * 60000) + (sec1 * 1000) + csec1;
	}
	else{
		int result = wxAtoi(raw);
		if (form == FRAME){
			orgframe = result;
		}
		else if (form == MDVD){
			orgframe = result;
			mstime = (result / 25.f)*(1000.f);
			if (orgframe < 0){ orgframe = 0; }
		}
		else{ mstime = result * 100; }
	}


}

wxString SubsTime::raw(char ft)//,float custfps
{
	wxString rawtxt;
	if (ft == 0){ ft = form; }
	if (ft < SRT){
		int csec = mstime / 10;
		int sec = mstime / 1000;
		int min = mstime / 60000;
		int godz = mstime / 3600000;
		rawtxt = wxString::Format(L"%01i:%02i:%02i.%02i", godz, (min % 60), (sec % 60), (csec % 100));
	}
	else if (ft == TMP){
		int sec = mstime / 1000;
		int min = mstime / 60000;
		int godz = mstime / 3600000;
		rawtxt = wxString::Format(L"%02i:%02i:%02i", godz, (min % 60), (sec % 60));
	}
	else if (ft == SRT){
		int sec = mstime / 1000;
		int min = mstime / 60000;
		int godz = mstime / 3600000;
		rawtxt = wxString::Format(L"%02i:%02i:%02i,%03i", godz, (min % 60), (sec % 60), (mstime % 1000));
	}
	else{
		if (ft == MDVD && !orgframe && mstime){
			orgframe = ceil(mstime * (25.f / 1000.f));
		}
		rawtxt = wxString::Format(L"%i", (ft != MPL2) ? orgframe : (int)ceil(mstime * (10.0f / 1000.0f)));
	}
	//form=ft;
	return rawtxt;
}

void SubsTime::Change(int ms)
{
	mstime += ms;
	if (mstime < 0){ mstime = 0; }
	if (form == MDVD){
		//MDVD format normally will take 25fps same as in Vobsub
		orgframe = ceil(mstime * (25.f / 1000.f));
	}
}
void SubsTime::ChangeFrame(int frame)
{
	orgframe += frame;
	if (orgframe < 0){ orgframe = 0; }
}
void SubsTime::NewTime(int ms)
{
	mstime = ms; if (mstime < 0){ mstime = 0; }
	if (form == MDVD){
		//MDVD format normally will take 25fps same as in Vobsub
		orgframe = ceil(mstime * (25.f / 1000.f));
	}

}

void SubsTime::NewFrame(int frame)
{
	orgframe = frame;
	if (orgframe < 0){ orgframe = 0; }
}

char SubsTime::GetFormat()
{
	return form;
}
void SubsTime::ChangeFormat(char format, float fps)
{
	if (format == form)
		return;
	if (format == ASS){ mstime = ZEROIT(mstime); }
	if (form == MDVD && format != FRAME){
		float fpsa = (fps) ? fps : Options.GetFloat(CONVERT_FPS);
		if (fpsa < 1){ fpsa = 23.976f; }
		mstime = (orgframe / fpsa) * (1000);
	}
	else if (format == MDVD && form != FRAME){
		float fpsa = (fps) ? fps : Options.GetFloat(CONVERT_FPS);
		if (fpsa<1){ fpsa = 23.976f; }
		orgframe = ceil(mstime * (fpsa / 1000));
	}
	form = format;
}

wxString SubsTime::GetFormatted(char format)
{
	return raw(format);
}

bool SubsTime::operator> (const SubsTime &comp)
{
	return mstime > comp.mstime;
}

bool SubsTime::operator< (const SubsTime &comp)
{
	return mstime < comp.mstime;
}

bool SubsTime::operator>= (const SubsTime &comp)
{
	return mstime > comp.mstime;
}

bool SubsTime::operator<= (const SubsTime &comp)
{
	return mstime < comp.mstime;
}

bool SubsTime::operator== (const SubsTime &comp)
{
	return mstime == comp.mstime;
}

bool SubsTime::operator!= (const SubsTime &comp)
{
	return mstime != comp.mstime;
}

SubsTime SubsTime::operator- (const SubsTime &comp)
{
	SubsTime tmp = SubsTime(comp);
	tmp.mstime = mstime - comp.mstime;
	tmp.orgframe = orgframe - comp.orgframe;
	if (tmp.mstime < 0){ tmp.mstime = 0; }
	if (tmp.orgframe < 0){ tmp.orgframe = 0; }
	return tmp;
}

SubsTime SubsTime::operator+ (const SubsTime &comp)
{
	SubsTime tmp = SubsTime(comp);
	tmp.mstime = mstime + comp.mstime;
	tmp.orgframe = orgframe + comp.orgframe;
	return tmp;
}