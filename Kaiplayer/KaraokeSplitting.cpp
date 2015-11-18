#include "KaraokeSplitting.h"
#include "config.h"
#include "audio_display.h"
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
	bool Everyn=Options.GetBool("Merge Every N With Syllable");
	Dialogue *dial=AD->dialogue;
	wxString Text=dial->Text;
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
		
		bool kpart=false;
		bool syll=false;
		wxString res;
		for(int i=0;i<len;i++)
			{
			wxUniChar ch=Text[i];
			wxUniChar nch=Text[i+1];
			if(ch=='\\' && (nch=='k'||nch=='K')){
				kpart=true;
				if(nch=='K'){kaas.Add("K");}
				}
			else if(kpart){
				if((nch=='o' || nch=='f')&&ch=='k'){kaas.Add("k"+wxString(nch));continue;}
				else if(ch=='k'){kaas.Add("k");}
				if(nch=='}'){stime += (wxAtoi(res)*10); syltimes.Add(stime); res=""; kpart=false; syll=true; continue;}
				res<<nch;
				}
			else if(syll){
				if(nch=='{'){syls.Add(res); res=""; syll=false; continue;}
				if(nch=='}') continue;
					res<<nch;
				}

			}

		}
	else
	{
		wxRegEx reg(_T("\\{([^{]*)\\}"),wxRE_ADVANCED);
		wxRegEx reg1(_T("[ ]*\\\\(.)[ ]*"),wxRE_ADVANCED);
		//reg.ReplaceAll(&textlow,_T(""));
		reg.ReplaceAll(&Text,_T(""));
		//reg1.ReplaceAll(&textlow,_T(" "));
		reg1.ReplaceAll(&Text,_T(" "));
		textlow=Text+" ";
		textlow=textlow.Lower();

		len=textlow.Len();
		int start=0;
		
		for(int i=0; i<len; i++)
		{
			wxUniChar ch=textlow[i];
			wxUniChar nch=(i<len-1)? textlow[i+1] : '\t';
			wxUniChar nnch=(i<len-2)? textlow[i+2] : '\t';
			
			if((Auto && achars.Find(ch)!=-1 && chars.Find(nch)==-1) || (!Auto && ch==' ')){
				//wxLogStatus(Text.SubString(start,i));
				if(Auto && (ch=='n' && aoi1.Find(nch)!=-1)//linia odpowiedzialna za podzia³ n
					|| (aoi2.Find(ch)!=-1 && nch=='n' && (nnch==' '||(Everyn && aoi1.Find(nnch)==-1))) ){continue;}
				syls.Add(Text.SubString(start,i));
				kaas.Add("k");
				start=i+1;
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
	//wxString kkk;
	//wxMessageBox(kkk<<"syl size "<<syls.size());
}
		
		
wxString Karaoke::GetText()
	{
	wxString text;
	//wxLogStatus("sizes, syl: %i",(int)syls.size(),(int)kaas.size());
	for(size_t i=0; i<syls.size(); i++)
		{
		int time=(i==0)? syltimes[i]-AD->curStartMS : (syltimes[i]-syltimes[i-1]);
			time/=10;
		text<<"{\\"<<kaas[i]<<time<<"}"<<syls[i];
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

void Karaoke::ChangeSplit(int line, int nletters)
	{
	wxString tmp=(nletters<0)? syls[line].SubString(0,syls[line].Len()-1-nletters) : syls[line]+syls[line+1].SubString(0,nletters);
	wxString tmp1=(nletters>0)? syls[line+1].SubString(nletters, syls[line+1].Len()-1) : syls[line].SubString(nletters,syls[line].Len()-1)+syls[line+1];
	syls[line]=tmp;
	syls[line+1]=tmp1;
	}

void Karaoke::SplitSyl(int line, int nletters)
	{
	wxString tmp=syls[line].Mid(0,nletters);
	wxString tmp1=syls[line].Mid(nletters);
	//tmp1<<nletters;
	syls[line]=tmp;
	syls.Insert(tmp1, line+1);
	kaas.Insert(_("k"), line+1);
	int start, end;
	GetSylTimes(line,start,end);
	int ttmp=start+((end-start)/2);
	ttmp=ZEROIT(ttmp);
	syltimes.Insert(ttmp,line);
	}

bool Karaoke::GetLetterAtX(int x, int *syl, int *result)
	{
	
	int tw,th, start, end;

	if(GetSylAtX(x, syl)){
		wxFont karafont(11,wxDEFAULT,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_BOLD,false,_T("Verdana"));
		AD->GetTextExtent(syls[*syl],&tw, &th, 0, 0, &karafont);
		GetSylTimes(*syl,start,end);
		start=AD->GetXAtMS(start);
		end=AD->GetXAtMS(end);
		int center=start+(((end-start)-tw)/2);
		wxString text=syls[*syl];
		//if(x<center){*result=0; return true;}
		//if(x+tw<center){*result=text.Len(); return true;}
		for(size_t i=0;i<text.Len();i++)
			{
			AD->GetTextExtent(text[i],&tw, &th, 0, 0, &karafont);
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