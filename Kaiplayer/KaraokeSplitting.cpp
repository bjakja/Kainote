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

#include "KaraokeSplitting.h"
#include "Config.h"
#include "AudioDisplay.h"
#include <math.h>
#include <wx/regex.h>

Karaoke::Karaoke(AudioDisplay *_AD)
{
	AD=_AD;
}

Karaoke::~Karaoke()
{
	Clearing();
}

void Karaoke::Clearing()
{
	syls.Clear();
	syltimes.Clear();
	kaas.Clear();
}

void Karaoke::Split()
{
	Clearing();
	bool Auto=AD->karaAuto;
	bool Everyn=Options.GetBool(AudioMergeEveryNWithSyllable);
	bool block = false;
	Dialogue *dial=AD->dialogue;
	wxString Text = (dial->TextTl != "")? dial->TextTl : dial->Text;
	int len=Text.Len();
	wxString aoi="aeioun ";
	wxString aoi1="aeiouy";
	wxString aoi2="aeiou";
	//wxString aoi3="aeiou ";
	wxString chars="!@#$%^&*()_+=-,./?'{}[]:;<> ";
	wxString achars=aoi+chars;
	int stime= dial->Start.mstime;

	wxString textlow=Text;
	textlow=textlow.Lower();
	if(textlow.Find("\\k")!=-1){
		Text<<"{";

		size_t strt=0;
		size_t lastStartBracket = 0;

		bool kpart=false;
		bool syll = false;
		
		wxString res;
		wxString kres;
		for(int i=0;i<len;i++)
		{
			wxUniChar ch=Text[i];
			wxUniChar nch = Text[i + 1];
			if (i == len - 1){
				res << ch;
				syls.Add(res);
				return;
			}
			if(ch=='\\' && (nch=='k'||nch=='K')){
				if (kaas.size()){	
					syls.Add(res.Mid(0, lastStartBracket));
					res = res.Mid(lastStartBracket);
					lastStartBracket = 0;
				}
				kpart=true;
				continue;
			}
			else if(kpart){
				if((nch=='o' || nch=='f')&&ch=='k'){kaas.Add("k"+wxString(nch));continue;}
				else if (ch == 'k' || ch == 'K'){ kaas.Add(ch); }
				if (nch == '}' || nch == '\\'){
					stime += (wxAtoi(kres) * 10);
					syltimes.Add(stime); 
					kres = "";
					kpart=false; 
					syll = true;
					continue;
				}
				kres << nch;
			}
			else{
				res<<ch;
				if (nch == '{'){ lastStartBracket = res.Len(); }
			}

		}

	}
	else
	{
		textlow += " ";
		len=textlow.Len();
		int start = 0;
		int i = 0;
		while( i<len)
		{
			wxUniChar ch = GetNextChar(&i, textlow);
			int newi = i;
			wxUniChar nch = GetNextChar(&newi, textlow);
			wxUniChar nnch = GetNextChar(&newi, textlow);

			if((Auto && achars.Find(ch)!=-1 && chars.Find(nch)==-1) || (!Auto && ch==' ')){
				if (Auto && (ch == 'n' && aoi1.Find(nch) != -1 || //linia odpowiedzialna za podzia³ n
					(aoi2.Find(ch)!=-1 && nch=='n' && 
					(nnch==' '||(Everyn && aoi1.Find(nnch)==-1))) ||
					(nch == '\"' && ch != ' ') || (nch=='\\' && nnch=='n'))){//#7 skip splitting " on end syllable
					continue;
				}
				syls.Add(Text.SubString(start,i-1));
				kaas.Add("k");
				start=i;
			}

		}
		int dur= dial->End.mstime-dial->Start.mstime;
		int times=(float)dur/(float)syls.size();


		for (size_t i=0; i<syls.size(); i++)
		{
			if(i==syls.size()-1){
				syltimes.Add(dial->End.mstime);break;
			}
			stime+=times;
			syltimes.Add(ZEROIT(stime));
		}
	}
	
}


wxString Karaoke::GetText()
{
	wxString text;
	for(size_t i=0; i<syls.size(); i++)
	{
		int time=(i==0)? syltimes[i]-AD->curStartMS : (syltimes[i]-syltimes[i-1]);
		time/=10;
		const wxString & sylText = syls[i];
		if (sylText.Len() && sylText[0] == L'{'){
			text << "{\\" << kaas[i] << time << sylText.After('{');
		}else
			text << "{\\" << kaas[i] << time << "}" << sylText;
	}
	return text;
}

bool Karaoke::CheckIfOver(int x, int *result)
{
	for(size_t i=0; i<syls.size(); i++)
	{
		if(std::abs(x-AD->GetXAtMS(syltimes[i]))<6){
			*result=i;
			return true;
		}
	}

	*result=-1;
	return false;
}

bool Karaoke::GetSylAtX(int x, int *result)
{
	wxArrayInt stimes=syltimes;
	stimes.Insert(AD->curStartMS,0);
	for(size_t i=0; i<syls.size(); i++)
	{
		if(x+2>AD->GetXAtMS(stimes[i]) && x-2<AD->GetXAtMS(stimes[i+1])){
			*result=i;
			return true;
		}
	}
	*result=-1;
	return false;
}

void Karaoke::Join(int line)
{
	syls[line]<<syls[line+1];
	syls.RemoveAt(line+1);
	syltimes[line]=syltimes[line+1];
	syltimes.RemoveAt(line+1);
	kaas.RemoveAt(line+1);
}

//void Karaoke::ChangeSplit(int line, int nletters)
//{
//	wxString tmp=(nletters<0)? syls[line].SubString(0,syls[line].Len()-1-nletters) : syls[line]+syls[line+1].SubString(0,nletters);
//	wxString tmp1=(nletters>0)? syls[line+1].SubString(nletters, syls[line+1].Len()-1) : syls[line].SubString(nletters,syls[line].Len()-1)+syls[line+1];
//	syls[line]=tmp;
//	syls[line+1]=tmp1;
//}

bool Karaoke::SplitSyl(int line, int nletters)
{
	if (syls[line].empty())
		return false;

	wxString tmp, tmp1;
	GetLetters(line, nletters, tmp, tmp1);
	syls[line]=tmp;
	syls.Insert(tmp1, line+1);
	kaas.Insert("k", line+1);
	int start, end;
	GetSylTimes(line,start,end);
	int ttmp=start+((end-start)/2);
	ttmp=ZEROIT(ttmp);
	syltimes.Insert(ttmp,line);
	return true;
}

wxUniChar Karaoke::GetNextChar(int *j, const wxString &text)
{
	bool block = false;
	for (size_t i = *j; i < text.Len(); i++){
		const wxUniChar & ch = text[i];
		if (ch == '{'){
			block = true;
		}
		else if (ch == '}'){ block = false;}
		else if (!block){ 
			*j = i+1;
			return ch;
		}
	}
	return '\t';
}

bool Karaoke::GetLetterAtX(int x, int *syl, int *result)
{

	int tw,th, start, end;

	if(GetSylAtX(x, syl)){
		wxString text;
		GetTextStripped(*syl, text);
		AD->GetTextExtentPixel(text, &tw, &th);
		GetSylTimes(*syl,start,end);
		start=AD->GetXAtMS(start);
		end=AD->GetXAtMS(end);
		int center=start+(((end-start)-tw)/2);
		
		for(size_t i=0;i<text.Len();i++)
		{
			AD->GetTextExtentPixel(text[i], &tw, &th);
			center+=(tw/2);
			if(x<center){*result=i;return true;}
			center+=(tw/2);
		}
		*result=text.Len();
		return true;
	}

	*result=-1; return false;

}

void Karaoke::GetSylTimes(int i, int &start, int &end)
{
	start=(i==0)?AD->curStartMS : syltimes[i-1];
	end=syltimes[i];
}

void Karaoke::GetSylVisibleTimes(int i, int &start, int &end)
{
	start = (i == 0) ? AD->curStartMS : syltimes[i - 1];
	end = (i < syltimes.size()-1)? syltimes[i + 1] : AD->curEndMS;
}

void Karaoke::GetLetters(int line, int nletters, wxString &first, wxString &second){
	const wxString & sylText = syls[line];
	size_t len = sylText.Len();
	if (nletters == 0){
		first = "";
		second = sylText;
		return;
	}
	bool block = false;
	int charsCounter = 0;
	for (size_t i = 0; i < len; i++){
		const wxUniChar & ch = sylText[i];
		if (charsCounter == nletters){
			first = sylText.Mid(0, i);
			second = sylText.Mid(i);
			return;
		}
		if (ch == '{'){
			block = true;
		}
		else if (ch == '}'){ block = false; }
		else if(!block){
			charsCounter++;
		}
	}
	first = sylText;
	second = "";
}

void Karaoke::GetTextStripped(int line, wxString &textStripped)
{
	const wxString & sylText = syls[line];
	size_t len = sylText.Len();
	bool block = false;
	textStripped.Empty();
	for (size_t i = 0; i < len; i++){
		const wxUniChar & ch = sylText[i];
		if (ch == '{'){
			block = true;
		}
		else if (ch == '}'){ block = false; }
		else if (!block){
			textStripped += ch;
		}
	}
	textStripped.Replace("\\N", "");
}
