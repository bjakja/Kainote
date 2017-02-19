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


#include "VersionKainote.h"
#include "Config.h"
#include "OpennWrite.h"
#include "KaiMessageBox.h"
#include <wx/settings.h>
#include <wx/stdpaths.h>
#include <wx/dir.h>
#include <wx/string.h>
#include <wx/log.h>
#include <wx/mstream.h>
#include <wx/bitmap.h>
#include <wx/msgdlg.h>
#include <windows.h>


config::config()
{
	progname=_T("Kainote v")+wxString(VersionKainote);
#if _DEBUG
	progname+= " DEBUG";
#endif
	AudioOpts=false;
}


config::~config()
{
	rawcfg.clear();
	for(std::vector<Styles*>::iterator it=assstore.begin(); it!=assstore.end(); it++)
	{
		delete (*it);
	}
	assstore.clear();
	for(std::map<wxString, wxColour*>::iterator it = colors.begin(); it != colors.end(); it++)
	{
		delete it->second;
	}
	colors.clear();
}

wxString config::GetReleaseDate()
{
	return wxString(__DATE__)+"  "+wxString(__TIME__);
}


bool config::SetRawOptions(const wxString &textconfig)
{
	wxStringTokenizer cfg(textconfig,_T("\n"));
	int g=0;
	while(cfg.HasMoreTokens())
	{
		wxString token=cfg.NextToken();
		token.Trim(false);
		token.Trim(true);
		if (token.Len()>0){CatchValsLabs(token);g++;}
	}
	if (g>10){return true;};
	return false;
}

wxString config::GetString(wxString lopt)
{
	return rawcfg[lopt];
}


bool config::GetBool(wxString lopt)
{   
	wxString opt=rawcfg[lopt];
	if(opt==_T("true")){return true;}
	return false;
}

wxColour config::GetColour(wxString lopt)
{
	auto it = colors.find(lopt);
	if(it!=colors.end()){
		return *it->second;
	}
	return wxColour("#000000");
}

AssColor config::GetColor(wxString lopt)
{
	auto it = colors.find(lopt);
	if(it!=colors.end()){
		return AssColor(*it->second);
	}
	return AssColor();
}

int config::GetInt(wxString lopt)
{
	return wxAtoi(rawcfg[lopt]);
}
float config::GetFloat(wxString lopt)
{   
	double fl;
	wxString rawfloat=rawcfg[lopt];
	if(!rawfloat.ToDouble(&fl)){return 0.0;}
	return fl;
}

void config::SetString(wxString lopt, wxString sopt)
{
	rawcfg[lopt]=sopt;
}

void config::SetBool(wxString lopt, bool bopt)
{
	wxString bopt1 = (bopt)? _T("true") : _T("false");
	rawcfg[lopt]=bopt1;
}

void config::SetColour(wxString lopt, wxColour copt)
{
	auto it = colors.find(lopt);
	if(it!=colors.end()){
		delete it->second;
		it->second = new wxColour(copt);
		return;
	}
	colors[lopt]=new wxColour(copt);
}

void config::SetColor(wxString lopt, AssColor copt)
{
	auto it = colors.find(lopt);
	if(it!=colors.end()){
		delete it->second;
		it->second = new wxColour(copt.GetWX());
		return;
	}
	colors[lopt]=new wxColour(copt.GetWX());
}

void config::SetInt(wxString lopt, int iopt)
{
	wxString iopt1=_T("");
	rawcfg[lopt]=iopt1<<iopt;
}

void config::SetFloat(wxString lopt, float fopt)
{
	wxString fopt1=_T("");
	fopt1<<fopt;
	fopt1.Replace(",",".");
	rawcfg[lopt]=fopt1;
}

wxString config::GetRawOptions(bool Audio)
{
	wxString TextOpt=_T("["+progname+"]\r\n");
	for (std::map<wxString,wxString>::iterator cur=rawcfg.begin();cur!=rawcfg.end();cur++) {
		if((!Audio && cur->first.StartsWith("Audio")) || (Audio && !cur->first.StartsWith("Audio"))) {continue;}
		TextOpt<<cur->first << _T("=") << cur->second << _T("\r\n");
	}
	return TextOpt;
}

void config::CatchValsLabs(const wxString &line)
{
	wxString Values=line.AfterFirst('=');
	Values.Trim(false);
	Values.Trim(true);
	wxString Labels=line.BeforeFirst('=');
	Labels.Trim(false);
	Labels.Trim(true);
	rawcfg[Labels]=Values;
}
void config::AddStyle(Styles *styl)
{
	assstore.push_back(styl);

}

Styles *config::GetStyle(int i,wxString name, Styles* _styl)
{
	if(name!=_T("")){
		for(unsigned int j=0;j<assstore.size();j++){
			if(name==assstore[j]->Name){if(_styl){_styl=assstore[j];} return assstore[j];}
		}

	}
	return assstore[i];
}

int config::FindStyle(wxString name, int *multiplication)
{
	int isfound=-1;
	for(unsigned int j=0;j<assstore.size();j++)
	{
		if(name==assstore[j]->Name){isfound=j;
		if(multiplication){
			*multiplication++;
		}else{break;}
		}
	}
	return isfound;
}

int config::StoreSize()
{
	return assstore.size();
}

void config::ChangeStyle(Styles *styl,int i)
{
	Styles *style= assstore[i];
	delete style;
	assstore[i]=styl;
}

void config::DelStyle(int i)
{
	Styles *styl= assstore[i];
	delete styl;
	assstore.erase(assstore.begin()+i);
}

void config::SaveOptions(bool cfg, bool style)
{
	OpenWrite ow;
	if(cfg){
		wxString textfile=GetRawOptions();
		wxString path;
		path<<pathfull<<_T("\\Config.txt");
		ow.FileWrite(path, textfile);
	}

	if(style){
		wxString stylefile;
		for (int j=0;j<StoreSize();j++){
			stylefile<<GetStyle(j)->styletext();
		}
		wxString path;
		path<<pathfull<<_T("\\Catalog\\")<<acdir<<_T(".sty");
		ow.FileWrite(path, stylefile);
	}
}

void config::LoadDefaultConfig()
{
	rawcfg[L"Change Time"] = "2000";
	rawcfg[L"Change mode"] = "0";
	rawcfg[L"Convert Resolution W"] = "1280";
	rawcfg[L"Convert Resolution H"] = "720";
	rawcfg[L"Default FPS"] = "23.976";
	rawcfg[L"Default Style"] = "Default";
	rawcfg[L"Default Style Catalog"] = "Default";
	rawcfg[L"Dictionary Name"] = "pl";
	rawcfg[L"Editbox Spellchecker"] = "true";
	rawcfg[L"FFMS2 Video Seeking"] = "2";
	rawcfg[L"Frames"] = "false";
	rawcfg[L"Grid Font Name"] = "Tahoma";
	rawcfg[L"Grid Font Size"] = "10";
	rawcfg[L"Grid tag changing char"] = L"☀";
	rawcfg[L"Move time forward"] = "true";
	rawcfg[L"Move Video To Active Line"] = "0";
	rawcfg[L"New end times"] = "false";
	rawcfg[L"Offset of start time"] = "0";
	rawcfg[L"Offset of end time"] = "0";
	rawcfg[L"Play Afrer Selection"] = "0";
	rawcfg[L"Preview Text"] = "Podgląd";
	rawcfg[L"Program Theme"] = "Default";
	rawcfg[L"Show Editor"] = "true";
	rawcfg[L"Show settings window"] = "false";
	rawcfg[L"Start end times"] = "0";
	rawcfg[L"Styles of time change"] = "";
	rawcfg[L"Time show of letter"] = "110";
	rawcfg[L"Index Video"] = "true";
	rawcfg[L"Video Prog Bar"] = "true";
	rawcfg[L"Video Window Size"] = "500,350";
	rawcfg[L"Window Size"] = "800,600";
}

void config::LoadDefaultColors()
{
	colors[L"Audio Background"] = new wxColour("#000000");
	colors[L"Audio Inactive Lines Background"] = new wxColour(0x37,0x35,0x64,0x55);//wxColour("#55373564");
	colors[L"Audio Keyframes"] = new wxColour("#4AFF00");
	colors[L"Audio Line Boundary End"] = new wxColour("#E67D00");
	colors[L"Audio Line Boundary Inactive Line"] = new wxColour("#808080");
	colors[L"Audio Line Boundary Mark"] = new wxColour("#FF00FF");
	colors[L"Audio Line Boundary Start"] = new wxColour("#D80000");
	colors[L"Audio Play Cursor"] = new wxColour("#FFFFFF");
	colors[L"Audio Seconds Boundaries"] = new wxColour(0x0B,0x8A,0x9F,0x9B);//wxColour("#9B0B8A9F");
	colors[L"Audio Selection Background"] = new wxColour(0xFF,0xFF,0xFF,0x37);//wxColour("#37FFFFFF");
	colors[L"Audio Selection Background Modified"] = new wxColour(0xD6,0x00,0x00,0x37);//wxColour("#37D60000");
	colors[L"Audio Spectrum First Color"] = new wxColour("#000000");
	colors[L"Audio Spectrum Second Color"] = new wxColour("#8DA8FF");
	colors[L"Audio Spectrum Third Color"] = new wxColour("#FFFFFF");
	colors[L"Audio Syllable Boundaries"] = new wxColour("#FFFF00");
	colors[L"Audio Syllable Text"] = new wxColour("#FF0000");
	colors[L"Audio Waveform"] = new wxColour("#7D6EFE");
	colors[L"Audio Waveform Inactive"] = new wxColour("#362C88");
	colors[L"Audio Waveform Modified"] = new wxColour("#FFE6E6");
	colors[L"Audio Waveform Selected"] = new wxColour("#DCDAFF");
	colors[L"Button Background"] = new wxColour("#2A2A2A");
	colors[L"Button Background Hover"] = new wxColour("#333333");
	colors[L"Button Background Pushed"] = new wxColour("#333333");
	colors[L"Button Border"] = new wxColour("#4E4E4E");
	colors[L"Button Border Hover"] = new wxColour("#555555");
	colors[L"Button Border Pushed"] = new wxColour("#555555");
	colors[L"Button Inactive Border"] = new wxColour("#5A5A5A");
	colors[L"Editor Background"] = new wxColour("#444444");
	colors[L"Editor Border"] = new wxColour("#4E4E4E");
	colors[L"Editor Border Focus"] = new wxColour("#8C8C8C");
	colors[L"Editor Curly Braces"] = new wxColour("#7378FE");
	colors[L"Editor Selection"] = new wxColour("#6E6E6E");
	colors[L"Editor Selection No Focus"] = new wxColour("#65657E");
	colors[L"Editor Spellchecker"] = new wxColour("#940000");
	colors[L"Editor Tag Names"] = new wxColour("#D09404");
	colors[L"Editor Tag Operators"] = new wxColour("#FA7676");
	colors[L"Editor Tag Values"] = new wxColour("#EC62FB");
	colors[L"Editor Text"] = new wxColour("#B7B7B7");
	colors[L"Grid Active Line"] = new wxColour("#4000CA");
	colors[L"Grid Background"] = new wxColour("#444444");
	colors[L"Grid Collisions"] = new wxColour("#0010FF");
	colors[L"Grid Comment"] = new wxColour("#BDBDBD");
	colors[L"Grid Comparison"] = new wxColour("#DFDADA");
	colors[L"Grid Comparison Background"] = new wxColour("#C0A073");
	colors[L"Grid Comparison Background Selected"] = new wxColour("#909F3A");
	colors[L"Grid Comparison Background selected"] = new wxColour("#909F3A");
	colors[L"Grid Comparison Comment Background"] = new wxColour("#978063");
	colors[L"Grid Comparison Comment Background Selected"] = new wxColour("#656F31");
	colors[L"Grid Dialogue"] = new wxColour("#969696");
	colors[L"Grid Label Modified"] = new wxColour("#A8FB05");
	colors[L"Grid Label Normal"] = new wxColour("#6551FF");
	colors[L"Grid Label Saved"] = new wxColour("#A398FF");
	colors[L"Grid Lines"] = new wxColour("#4C4C4C");
	colors[L"Grid Selected Comment"] = new wxColour("#839394");
	colors[L"Grid Selected Dialogue"] = new wxColour("#B6D5C5");
	colors[L"Grid Spellchecker"] = new wxColour("#FA9292");
	colors[L"Grid Text"] = new wxColour("#121212");
	colors[L"Menu Background"] = new wxColour("#323232");
	colors[L"Menu Background Selection"] = new wxColour("#39374B");
	colors[L"Menu Bar Background 1"] = new wxColour("#4A4A4A");
	colors[L"Menu Bar Background 2"] = new wxColour("#222222");
	colors[L"Menu Bar Background Selection"] = new wxColour("#39374B");
	colors[L"Menu Bar Border Selection"] = new wxColour("#948FC4");
	colors[L"Menu Border Selection"] = new wxColour("#5B5689");
	colors[L"Scrollbar Background"] = new wxColour("#3A3A3A");
	colors[L"Scrollbar Scroll"] = new wxColour("#686868");
	colors[L"Scrollbar Scroll Hover"] = new wxColour("#868686");
	colors[L"Scrollbar Scroll Pushed"] = new wxColour("#868686");
	colors[L"Slider Background"] = new wxColour("#282828");
	colors[L"Slider Background Hover"] = new wxColour("#333333");
	colors[L"Slider Background Pushed"] = new wxColour("#333333");
	colors[L"Slider Border"] = new wxColour("#4E4E4E");
	colors[L"Slider Path Background"] = new wxColour("#282828");
	colors[L"Slider Path Border"] = new wxColour("#434343");
	colors[L"Staticbox Border"] = new wxColour("#565555");
	colors[L"Style Preview Color1"] = new wxColour("#9A9A9A");
	colors[L"Style Preview Color2"] = new wxColour("#686868");
	colors[L"Tabs Background Active"] = new wxColour("#2D2D2D");
	colors[L"Tabs Background Inactive"] = new wxColour("#3C3C3C");
	colors[L"Tabs Background Inactive Hover"] = new wxColour("#2D2D2D");
	colors[L"Tabs Bar Arrow"] = new wxColour("#9B9B9B");
	colors[L"Tabs Bar Arrow Background"] = new wxColour("#222222");
	colors[L"Tabs Bar Arrow Background Hover"] = new wxColour("#333333");
	colors[L"Tabs Bar Background 1"] = new wxColour("#222222");
	colors[L"Tabs Bar Background 2"] = new wxColour("#222222");
	colors[L"Tabs Border Active"] = new wxColour("#000000");
	colors[L"Tabs Border inactive"] = new wxColour("#000000");
	colors[L"Tabs Close Hover"] = new wxColour("#D40403");
	colors[L"Tabs Text Active"] = new wxColour("#B5B5B5");
	colors[L"Tabs Text Inactive"] = new wxColour("#9B9B9B");
	colors[L"Togglebutton Background Toggled"] = new wxColour("#393939");
	colors[L"Togglebutton Border Toggled"] = new wxColour("#636363");
	colors[L"Window Background"] = new wxColour("#222222");
	colors[L"Window Border"] = new wxColour("#111111");
	colors[L"Window Border Background"] = new wxColour("#786AEB");
	colors[L"Window Border Background Inactive"] = new wxColour("#818181");
	colors[L"Window Border Inactive"] = new wxColour("#111111");
	colors[L"Window Header Inactive Text"] = new wxColour("#1A1A1A");
	colors[L"Window Header Text"] = new wxColour("#1A1A1A");
	colors[L"Window Hover Header Element"] = new wxColour("#D40403");
	colors[L"Window Inactive Background"] = new wxColour("#303030");
	colors[L"Window Inactive Text"] = new wxColour("#4E4E4E");
	colors[L"Window Pushed Header Element"] = new wxColour("#E50403");
	colors[L"Window Text"] = new wxColour("#9B9B9B");
	colors[L"Window Warning Elements"] = new wxColour("#9B9B9B");

}

int config::LoadOptions()
{
	wxStandardPathsBase &paths = wxStandardPaths::Get();
	pathfull=paths.GetExecutablePath().BeforeLast('\\');
	wxString path;
	path<<pathfull<<_T("\\Config.txt");
	OpenWrite ow;
	wxString txt;
	bool isgood=false;
	bool diffVersions=false;
	if(!ow.FileOpen(path,&txt,false)){
		LoadDefaultConfig();
		isgood = true;
	}else{
		wxString ver= txt.BeforeFirst(']').Mid(1);
		if(ver!=progname){LoadDefaultConfig();diffVersions=true;}
		isgood = SetRawOptions(txt.AfterFirst('\n'));
	}

	acdir = _T("Default");
	path=_T("");
	path<<pathfull<<_T("\\Catalog\\");
	wxDir kat(path);
	if(!kat.IsOpened()){
		ow.FileWrite(path<<acdir<<_T(".sty"),_T("Style: Default,Garamond,30,&H00FFFFFF,&H000000FF,&H00FF0000,&H00000000,0,0,0,0,100,100,0,0,0,2,2,2,10,10,10,1"));
		AddStyle(new Styles());dirs.Add(acdir);
	}
	else{
		wxArrayString tmp;kat.GetAllFiles(path,&tmp,_T(""), wxDIR_FILES);
		for(size_t i=0;i<tmp.GetCount();i++){
			wxString fullpath=tmp[i].AfterLast('\\');
			if(fullpath.EndsWith(_T(".sty"))){dirs.Add(fullpath.BeforeLast('.'));}
		}
	}
	LoadStyles(acdir);
	LoadColors();
	return isgood;
}

void config::LoadColors(const wxString &_themeName){
	wxString themeName;
	ClearColors();
	if(_themeName.IsEmpty()){
		themeName = Options.GetString("Program Theme");
	}else{
		themeName = _themeName;
		Options.SetString("Program Theme", _themeName);
	}
	bool failed = false;
	if(themeName!="Default"){
		wxString path = pathfull + L"\\Themes\\"+ themeName + L".txt";
		OpenWrite ow;
		wxString txtColors;
		if(ow.FileOpen(path, &txtColors, false)){
			wxStringTokenizer cfg(txtColors,_T("\n"));
			int g=0;
			while(cfg.HasMoreTokens())
			{
				wxString token=cfg.NextToken();
				token.Trim(false);
				token.Trim(true);
				if (token.Len()>0){SetHexColor(token);g++;}
			}
			if(colors.size()>10){return;}
		}
		failed = true;
	}
	LoadDefaultColors();	
	if(failed){
		Options.SetString("Program Theme", "Default");
		KaiMessageBox(_("Nie można zaczytać motywu, zostanie przywrócony domyśny"));
	}
}


void config::LoadStyles(wxString katalog)
{
	acdir=katalog;
	wxString path;
	path<<pathfull<<_T("\\Catalog\\")<<katalog<<_T(".sty");
	OpenWrite ow;
	for(std::vector<Styles*>::iterator it=assstore.begin(); it!=assstore.end(); it++){
		delete (*it);
	}
	assstore.clear();
	wxString stylee;
	if(ow.FileOpen(path, &stylee,false)){
		wxStringTokenizer cfg(stylee,_T("\n"));
		while(cfg.HasMoreTokens()){
			wxString token=cfg.NextToken();
			if (token.StartsWith(_T("Style: "))){
				AddStyle(new Styles(token));
			}
		}
	}
}

void config::clearstyles()
{
	for(std::vector<Styles*>::iterator it=assstore.begin(); it!=assstore.end(); it++){
		delete (*it);
	}
	assstore.clear();
}

void config::ClearColors()
{
	for(std::map<wxString, wxColour*>::iterator it = colors.begin(); it != colors.end(); it++)
	{
		delete it->second;
	}
	colors.clear();
}

void config::SetCoords(wxString lopt, int coordx, int coordy)
{
	wxString iopt1=_T("");
	rawcfg[lopt]=iopt1<<coordx<<","<<coordy;
}

void config::GetCoords(wxString lopt, int *coordx, int *coordy)
{
	wxString sopt=rawcfg[lopt];
	*coordx=wxAtoi(sopt.BeforeFirst(','));
	*coordy=wxAtoi(sopt.AfterFirst(','));
}

void config::SetTable(wxString lopt, wxArrayString asopt,wxString split)
{
	wxString sresult;
	wxString ES;
	for(size_t i=0;i<asopt.size();i++)
	{
		wxString endchar=(i==asopt.size()-1)? ES : split;
		sresult<<asopt[i]<<endchar;
	}
	rawcfg[lopt]=sresult;
}

void config::SetIntTable(wxString lopt, wxArrayInt asopt,wxString split)
{
	wxString sresult;
	wxString ES;
	for(size_t i=0;i<asopt.size();i++)
	{
		wxString endchar=(i==asopt.size()-1)? ES : split;
		sresult<<asopt[i]<<endchar;
	}
	rawcfg[lopt]=sresult;
}

wxArrayString config::GetTable(wxString lopt, wxString split)
{
	wxArrayString strcfg;
	wxString strtbl=rawcfg[lopt];
	if(strtbl!=""){
		wxStringTokenizer cfgtable(strtbl,split,wxTOKEN_STRTOK);
		while(cfgtable.HasMoreTokens()){
			strcfg.Add(cfgtable.NextToken());
		}  
	}
	return strcfg;
}

wxArrayInt config::GetIntTable(wxString lopt, wxString split)
{
	wxArrayInt intcfg;
	wxString strtbl=rawcfg[lopt];
	if(strtbl!=""){
		wxStringTokenizer cfgtable(strtbl,split,wxTOKEN_STRTOK);
		while(cfgtable.HasMoreTokens()){
			intcfg.Add(wxAtoi(cfgtable.NextToken()));
		}  
	}
	return intcfg;
}

bool sortfunc(Styles *styl1,Styles *styl2){
	wxString str1=styl1->Name;
	wxString str2=styl2->Name;
	return (str1.CmpNoCase(str2)<0);
}

void config::Sortstyles()
{
	std::sort(assstore.begin(),assstore.end(),sortfunc);
}

void config::LoadDefaultAudioConfig()
{
	rawcfg[L"Audio Autocommit"] = "true";
	rawcfg[L"Audio Autofocus"] = "true";
	rawcfg[L"Audio Autoscroll"] = "true";
	rawcfg[L"Audio Box Height"] = "169";
	rawcfg[L"Audio Delay"] = "0";
	rawcfg[L"Audio Draw Cursor Time"] = "true";
	rawcfg[L"Audio Draw Keyframes"] = "true";
	rawcfg[L"Audio Draw Secondary Lines"] = "true";
	rawcfg[L"Audio Draw Selection Background"] = "true";
	rawcfg[L"Audio Draw video Position"] = "true";
	rawcfg[L"Audio Grab Times On Select"] = "true";
	rawcfg[L"Audio Horizontal Zoom"] = "50";
	rawcfg[L"Audio Inactive Lines Display Mode"] = "1";
	rawcfg[L"Audio Lead In"] = "200";
	rawcfg[L"Audio Lead Out"] = "300";
	rawcfg[L"Audio Line Boundaries Thickness"] = "2";
	rawcfg[L"Audio Link"] = "false";
	rawcfg[L"Audio Lock Scroll On Cursor"] = "false";
	rawcfg[L"Audio Mark Play Time"] = "1000";
	rawcfg[L"Audio Next Line On Commit"] = "true";
	rawcfg[L"Audio RAM Cache"] = "false";
	rawcfg[L"Audio Sample Rate"] = "0";
	rawcfg[L"Audio Snap To Keyframes"] = "false";
	rawcfg[L"Audio Snap To Other Lines"] = "false";
	rawcfg[L"Audio Spectrum"] = "false";
	rawcfg[L"Audio Start Drag Sensitivity"] = "2";
	rawcfg[L"Audio Vertical Zoom"] = "50";
	rawcfg[L"Audio Volume"] = "50";
	rawcfg[L"Audio Wheel Default To Zoom"] = "false";

}

bool config::LoadAudioOpts()
{
	OpenWrite ow;
	wxString txt;
	if(!ow.FileOpen(pathfull+_T("\\AudioConfig.txt"), &txt ,false)){
		LoadDefaultAudioConfig();
		return true;
	}else{
		wxString ver= txt.BeforeFirst(']').Mid(1);
		if(ver!=progname){LoadDefaultAudioConfig();}
	}
	return (AudioOpts=SetRawOptions(txt.AfterFirst('\n')));
}

void config::SaveAudioOpts()
{
	OpenWrite ow;
	ow.FileWrite(pathfull+_T("\\AudioConfig.txt"), GetRawOptions(true));
}

void config::SetHexColor(const wxString &nameAndColor)
{
	wxString kol=nameAndColor.AfterFirst('=');
	kol.Trim(false);
	kol.Trim(true);
	wxString name=nameAndColor.BeforeFirst('=');
	name.Trim(false);
	name.Trim(true);
	long a=0xFF, r, g, b;
	int diff = 0;
	if(kol.Len()>=9){
		kol.SubString(1,2).ToLong(&a, 16);
		diff=2;
	}
	kol.SubString(diff+1,diff+2).ToLong(&r, 16);
	kol.SubString(diff+3,diff+4).ToLong(&g, 16);
	kol.SubString(diff+5,diff+6).ToLong(&b, 16);
	colors[name]=new wxColour(r, g, b, a);
}

wxString config::GetStringColor(std::map<wxString, wxColour*>::iterator it)
{
	wxColour *col = it->second;
	if (col->Alpha() < 0xFF)
		return wxString::Format("#%02X%02X%02X%02X", col->Alpha(), col->Red(), col->Green(), col->Blue());
	return wxString::Format("#%02X%02X%02X", col->Red(), col->Green(), col->Blue());
}
wxString config::GetStringColor(const wxString &optionName)
{
	wxColour *col = colors[optionName];
	if (col->Alpha() < 0xFF)
		return wxString::Format("#%02X%02X%02X%02X", col->Alpha(), col->Red(), col->Green(), col->Blue());
	return wxString::Format("#%02X%02X%02X", col->Red(), col->Green(), col->Blue());
}

void config::SaveColors(const wxString &path){
	wxString finalpath = path;
	if(path.IsEmpty()){
		finalpath = pathfull + "\\Themes\\" + GetString("Program Theme") + ".txt";
	}
	OpenWrite ow(finalpath,true);
	for(auto it = colors.begin(); it != colors.end(); it++){
		ow.PartFileWrite(it->first + "=" + GetStringColor(it) + "\r\n");
	}
}

wxString getfloat(float num, wxString format, bool Truncate)
{
	wxString strnum=wxString::Format(_T("%"+format),num);
	//if(strnum.find('.')!= -1){return strnum.Trim(false);}
	if(!Truncate || format.EndsWith(".0f")){return strnum.Trim(false);}
	int rmv=0;
	bool trim=false;
	for(int i=strnum.Len()-1;i>0;i--)
	{
		if(strnum[i]=='0'){rmv++;}//&&!trim
		//else if(strnum[i]=='9'){rmv++;trim=true;}
		else if(strnum[i]=='.'){rmv++;break;}//}if(!trim){
		else{/*if(trim){int tmpc=static_cast < int >(strnum.GetChar(i));tmpc++;strnum[i]=(wxUniChar)tmpc;}*/break;}
	}
	if(rmv){strnum.RemoveLast(rmv);}
	return strnum.Trim(false);
}


bool LoadDataFromResource(char*& t_data, DWORD& t_dataSize, const wxString& t_name)
{
	bool     r_result    = false;
	HGLOBAL  a_resHandle = 0;
	HRSRC    a_resource;

	a_resource = FindResource(0, t_name.wchar_str(), RT_RCDATA);

	if(0 != a_resource)
	{
		a_resHandle = LoadResource(NULL, a_resource);
		if (0 != a_resHandle)
		{
			t_data = (char*)LockResource(a_resHandle);
			t_dataSize = SizeofResource(NULL, a_resource);
			r_result = true;
		}
	}

	return r_result;
}


wxBitmap GetBitmapFromMemory(const char* t_data, const DWORD t_size)
{
	wxMemoryInputStream a_is(t_data, t_size);
	return wxBitmap(wxImage(a_is, wxBITMAP_TYPE_PNG, -1), -1);
}

wxBitmap CreateBitmapFromPngResource(const wxString& t_name)
{
	wxBitmap   r_bitmapPtr;

	char*       a_data      = 0;
	DWORD       a_dataSize  = 0;

	if(LoadDataFromResource(a_data, a_dataSize, t_name))
	{
		r_bitmapPtr = GetBitmapFromMemory(a_data, a_dataSize);
	}

	return r_bitmapPtr;
}

wxBitmap *CreateBitmapPointerFromPngResource(const wxString& t_name)
{
	wxBitmap  *r_bitmapPtr = NULL;

	char*       a_data      = 0;
	DWORD       a_dataSize  = 0;

	if(LoadDataFromResource(a_data, a_dataSize, t_name))
	{
		wxMemoryInputStream a_is(a_data, a_dataSize);
		r_bitmapPtr = new wxBitmap(wxImage(a_is, wxBITMAP_TYPE_PNG, -1), -1);
	}

	return r_bitmapPtr;
}

wxImage CreateImageFromPngResource(const wxString& t_name)
{
	wxImage   image;

	char*       a_data      = 0;
	DWORD       a_dataSize  = 0;

	if(LoadDataFromResource(a_data, a_dataSize, t_name))
	{
		wxMemoryInputStream a_is(a_data, a_dataSize);
		image=wxImage(a_is, wxBITMAP_TYPE_PNG, -1);
	}

	return image;
}

void MoveToMousePosition(wxWindow *win)
{
	wxPoint mst=wxGetMousePosition();
	wxSize siz=win->GetSize();
	siz.x;
	wxRect rc = wxGetClientDisplayRect();
	mst.x-=(siz.x/2);
	mst.x=MID(0,mst.x, rc.width - siz.x);
	mst.y+=15;
	if(mst.y + siz.y > rc.height){
		mst.y = mst.y - siz.y - 30;
	}
	win->Move(mst);
}

wxString MakePolishPlural(int num, const wxString &normal, const wxString &plural2to4, const wxString &pluralRest)
{
	wxString result;
	int div10mod = (num % 10);
	int div100mod = (num % 100);
	if(num==1){result = normal;}
	else if((div10mod >=2 && div10mod <= 4) && (div100mod<10 && div100mod>20)){
		result = plural2to4;
	}else{
		result = pluralRest;
	}
	wxString finalResult;
	return finalResult<<num<<" "<<result;
}

config Options;




