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
	defaultColour = wxColour();
}


config::~config()
{
	rawcfg.clear();
	for(std::vector<Styles*>::iterator it=assstore.begin(); it!=assstore.end(); it++)
	{
		delete (*it);
	}
	assstore.clear();
	for(std::map<COLOR, wxColour*>::iterator it = colors.begin(); it != colors.end(); it++)
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

wxString config::GetString(CONFIG opt)
{
	return rawcfg[opt];
}


bool config::GetBool(CONFIG opt)
{   
	wxString ropt=rawcfg[opt];
	if(ropt==_T("true")){return true;}
	return false;
}

wxColour &config::GetColour(COLOR opt)
{
	auto it = colors.find(opt);
	if(it!=colors.end()){
		return *it->second;
	}
	return defaultColour;//wxColour("#000000");
}

AssColor config::GetColor(COLOR opt)
{
	auto it = colors.find(opt);
	if(it!=colors.end()){
		return AssColor(*it->second);
	}
	return AssColor();
}

int config::GetInt(CONFIG opt)
{
	return wxAtoi(rawcfg[opt]);
}
float config::GetFloat(CONFIG opt)
{   
	double fl;
	wxString rawfloat=rawcfg[opt];
	if(!rawfloat.ToDouble(&fl)){return 0.0;}
	return fl;
}

void config::SetString(CONFIG opt, const wxString &sopt)
{
	rawcfg[opt]=sopt;
}

void config::SetBool(CONFIG opt, bool bopt)
{
	wxString bopt1 = (bopt)? _T("true") : _T("false");
	rawcfg[opt]=bopt1;
}

void config::SetColour(COLOR opt, wxColour &copt)
{
	auto it = colors.find(opt);
	if(it!=colors.end()){
		delete it->second;
		it->second = new wxColour(copt);
		return;
	}
	colors[opt]=new wxColour(copt);
}

void config::SetColor(COLOR opt, AssColor &copt)
{
	auto it = colors.find(opt);
	if(it!=colors.end()){
		delete it->second;
		it->second = new wxColour(copt.GetWX());
		return;
	}
	colors[opt]=new wxColour(copt.GetWX());
}

void config::SetInt(CONFIG opt, int iopt)
{
	wxString iopt1=_T("");
	rawcfg[opt]=iopt1<<iopt;
}

void config::SetFloat(CONFIG opt, float fopt)
{
	wxString fopt1=_T("");
	fopt1<<fopt;
	fopt1.Replace(",",".");
	rawcfg[opt]=fopt1;
}

wxString config::GetRawOptions(bool Audio)
{
	wxString TextOpt=_T("["+progname+"]\r\n");
	for (std::map<CONFIG,wxString>::iterator cur=rawcfg.begin();cur!=rawcfg.end();cur++) {
		if((!Audio && cur->first <= AudioWheelDefaultToZoom) || (Audio && cur->first >=AudioWheelDefaultToZoom)) {continue;}
		TextOpt<<::GetString(cur->first) << _T("=") << cur->second << _T("\r\n");
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
	rawcfg[GetCONFIGValue(Labels)] = Values;
}
void config::AddStyle(Styles *styl)
{
	assstore.push_back(styl);

}

Styles *config::GetStyle(int i,const wxString &name, Styles* _styl)
{
	if(name!=_T("")){
		for(unsigned int j=0;j<assstore.size();j++){
			if(name==assstore[j]->Name){if(_styl){_styl=assstore[j];} return assstore[j];}
		}

	}
	return assstore[i];
}

int config::FindStyle(const wxString &name, int *multiplication)
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
	rawcfg[MoveTimesTime] = "2000";
	rawcfg[MoveTimesWhichLines] = "0";
	rawcfg[ConvertResolutionWidth] = "1280";
	rawcfg[ConvertResolutionHeight] = "720";
	rawcfg[ConvertFPS] = "23.976";
	rawcfg[ConvertStyle] = "Default";
	rawcfg[ConvertStyleCatalog] = "Default";
	rawcfg[DictionaryLanguage] = "pl";
	rawcfg[SpellcheckerOn] = "true";
	rawcfg[FFMS2VideoSeeking] = "2";
	rawcfg[MoveTimesByTime] = "false";
	rawcfg[GridFontName] = "Tahoma";
	rawcfg[GridFontSize] = "10";
	rawcfg[GridTagsSwapChar] = L"☀";
	rawcfg[MoveTimesForward] = "true";
	rawcfg[ConvertNewEndTimes] = "false";
	rawcfg[InsertStartOffset] = "0";
	rawcfg[InsertEndOffset] = "0";
	rawcfg[PlayAfterSelection] = "0";
	rawcfg[PreviewText] = "Podgląd";
	rawcfg[ProgramTheme] = "Default";
	rawcfg[EditorOn] = "true";
	rawcfg[ConvertShowSettings] = "false";
	rawcfg[MoveTimesOn] = "true";
	rawcfg[MoveTimesWhichTimes] = "0";
	rawcfg[MoveTimesStyles] = "";
	rawcfg[ConvertTimePerLetter] = "110";
	rawcfg[VideoIndex] = "true";
	rawcfg[VideoProgressBar] = "true";
	rawcfg[VideoWindowSize] = "500,350";
	rawcfg[WindowSize] = "800,600";
}

void config::LoadDefaultColors()
{
	colors[AudioBackground] = new wxColour("#000000");
	colors[AudioInactiveLinesBackground] = new wxColour(0x37,0x35,0x64,0x55);//wxColour("#55373564");
	colors[AudioKeyframes] = new wxColour("#4AFF00");
	colors[AudioLineBoundaryEnd] = new wxColour("#E67D00");
	colors[AudioLineBoundaryInactiveLine] = new wxColour("#808080");
	colors[AudioLineBoundaryMark] = new wxColour("#FF00FF");
	colors[AudioLineBoundaryStart] = new wxColour("#D80000");
	colors[AudioPlayCursor] = new wxColour("#FFFFFF");
	colors[AudioSecondsBoundaries] = new wxColour(0x0B,0x8A,0x9F,0x9B);//wxColour("#9B0B8A9F");
	colors[AudioSelectionBackground] = new wxColour(0xFF,0xFF,0xFF,0x37);//wxColour("#37FFFFFF");
	colors[AudioSelectionBackgroundModified] = new wxColour(0xD6,0x00,0x00,0x37);//wxColour("#37D60000");
	colors[AudioSpectrumBackground] = new wxColour("#000000");
	colors[AudioSpectrumEcho] = new wxColour("#8DA8FF");
	colors[AudioSpectrumInner] = new wxColour("#FFFFFF");
	colors[AudioSyllableBoundaries] = new wxColour("#FFFF00");
	colors[AudioSyllableText] = new wxColour("#FF0000");
	colors[AudioWaveform] = new wxColour("#7D6EFE");
	colors[AudioWaveformInactive] = new wxColour("#362C88");
	colors[AudioWaveformModified] = new wxColour("#FFE6E6");
	colors[AudioWaveformSelected] = new wxColour("#DCDAFF");
	colors[TextFieldBackground] = new wxColour("#2A2A2A");
	colors[TextFieldBorder] = new wxColour("#4E4E4E");
	colors[TextFieldBorderOnFocus] = new wxColour("#8C8C8C");
	colors[TextFieldSelection] = new wxColour("#6E6E6E");
	colors[TextFieldSelectionNoFocus] = new wxColour("#65657E");
	colors[ButtonBackground] = new wxColour("#2A2A2A");
	colors[ButtonBackgroundHover] = new wxColour("#333333");
	colors[ButtonBackgroundPushed] = new wxColour("#333333");
	colors[ButtonBorder] = new wxColour("#4E4E4E");
	colors[ButtonBorderHover] = new wxColour("#555555");
	colors[ButtonBorderPushed] = new wxColour("#555555");
	colors[ButtonBorderInactive] = new wxColour("#5A5A5A");
	colors[EditorBackground] = new wxColour("#444444");
	colors[EditorBorder] = new wxColour("#4E4E4E");
	colors[EditorBorderOnFocus] = new wxColour("#8C8C8C");
	colors[EditorBracesBackground] = new wxColour("#000000");
	colors[EditorCurlyBraces] = new wxColour("#7378FE");
	colors[EditorSelection] = new wxColour("#6E6E6E");
	colors[EditorSelectionNoFocus] = new wxColour("#65657E");
	colors[EditorSpellchecker] = new wxColour("#940000");
	colors[EditorTagNames] = new wxColour("#D09404");
	colors[EditorTagOperators] = new wxColour("#FA7676");
	colors[EditorTagValues] = new wxColour("#EC62FB");
	colors[EditorText] = new wxColour("#B7B7B7");
	colors[GridActiveLine] = new wxColour("#4000CA");
	colors[GridBackground] = new wxColour("#444444");
	colors[GridCollisions] = new wxColour("#0010FF");
	colors[GridComment] = new wxColour("#BDBDBD");
	colors[GridComparison] = new wxColour("#DFDADA");
	colors[GridComparisonBackground] = new wxColour("#C0A073");
	colors[GridComparisonBackgroundSelected] = new wxColour("#909F3A");
	colors[GridComparisonCommentBackground] = new wxColour("#978063");
	colors[GridComparisonCommentBackgroundSelected] = new wxColour("#656F31");
	colors[GridDialogue] = new wxColour("#969696");
	colors[GridLabelModified] = new wxColour("#A8FB05");
	colors[GridLabelNormal] = new wxColour("#6551FF");
	colors[GridLabelSaved] = new wxColour("#A398FF");
	colors[GridLines] = new wxColour("#4C4C4C");
	colors[GridSelectedComment] = new wxColour("#839394");
	colors[GridSelectedDialogue] = new wxColour("#B6D5C5");
	colors[GridSpellchecker] = new wxColour("#FA9292");
	colors[GridText] = new wxColour("#121212");
	colors[MenuBackground] = new wxColour("#323232");
	colors[MenuBackgroundSelection] = new wxColour("#39374B");
	colors[MenuBarBackground1] = new wxColour("#4A4A4A");
	colors[MenuBarBackground2] = new wxColour("#222222");
	colors[MenuBarBackgroundSelection] = new wxColour("#39374B");
	colors[MenuBarBorderSelection] = new wxColour("#948FC4");
	colors[MenuBorderSelection] = new wxColour("#5B5689");
	colors[ScrollbarBackground] = new wxColour("#3A3A3A");
	colors[ScrollbarScroll] = new wxColour("#686868");
	colors[ScrollbarScrollHover] = new wxColour("#868686");
	colors[ScrollbarScrollPushed] = new wxColour("#868686");
	colors[SliderBackground] = new wxColour("#282828");
	colors[SliderBackgroundHover] = new wxColour("#333333");
	colors[SliderBackgroundPushed] = new wxColour("#333333");
	colors[SliderBorder] = new wxColour("#4E4E4E");
	colors[SliderPathBackground] = new wxColour("#282828");
	colors[SliderPathBorder] = new wxColour("#434343");
	colors[StaticboxBorder] = new wxColour("#565555");
	colors[StylePreviewColor1] = new wxColour("#9A9A9A");
	colors[StylePreviewColor2] = new wxColour("#686868");
	colors[TabsBackgroundActive] = new wxColour("#2D2D2D");
	colors[TabsBackgroundInactive] = new wxColour("#3C3C3C");
	colors[TabsBackgroundInactiveHover] = new wxColour("#2D2D2D");
	colors[TabsBarArrow] = new wxColour("#9B9B9B");
	colors[TabsBarArrowBackground] = new wxColour("#222222");
	colors[TabsBarArrowBackgroundHover] = new wxColour("#333333");
	colors[TabsBarBackground1] = new wxColour("#222222");
	colors[TabsBarBackground2] = new wxColour("#222222");
	colors[TabsBorderActive] = new wxColour("#000000");
	colors[TabsBorderInactive] = new wxColour("#000000");
	colors[TabsCloseHover] = new wxColour("#D40403");
	colors[TabsTextActive] = new wxColour("#B5B5B5");
	colors[TabsTextInactive] = new wxColour("#9B9B9B");
	colors[TogglebuttonBackgroundToggled] = new wxColour("#393939");
	colors[TogglebuttonBorderToggled] = new wxColour("#636363");
	colors[WindowBackground] = new wxColour("#222222");
	colors[WindowBorder] = new wxColour("#111111");
	colors[WindowBorderBackground] = new wxColour("#786AEB");
	colors[WindowBorderBackgroundInactive] = new wxColour("#818181");
	colors[WindowBorderInactive] = new wxColour("#111111");
	colors[WindowHeaderTextInactive] = new wxColour("#1A1A1A");
	colors[WindowHeaderText] = new wxColour("#1A1A1A");
	colors[WindowHoverHeaderElement] = new wxColour("#D40403");
	colors[WindowBackgroundInactive] = new wxColour("#303030");
	colors[WindowTextInactive] = new wxColour("#4E4E4E");
	colors[WindowPushedHeaderElement] = new wxColour("#E50403");
	colors[WindowHoverCloseButton] = new wxColour("#D40403");
	colors[WindowPushedCloseButton] = new wxColour("#E50403");
	colors[WindowText] = new wxColour("#9B9B9B");
	colors[WindowWarningElements] = new wxColour("#9B9B9B");

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
		themeName = Options.GetString(ProgramTheme);
	}else{
		themeName = _themeName;
		Options.SetString(ProgramTheme, _themeName);
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
		Options.SetString(ProgramTheme, "Default");
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
	for(std::map<COLOR, wxColour*>::iterator it = colors.begin(); it != colors.end(); it++)
	{
		delete it->second;
	}
	colors.clear();
}

void config::SetCoords(CONFIG opt, int coordx, int coordy)
{
	wxString iopt1=_T("");
	rawcfg[opt]=iopt1<<coordx<<","<<coordy;
}

void config::GetCoords(CONFIG opt, int *coordx, int *coordy)
{
	wxString sopt=rawcfg[opt];
	*coordx=wxAtoi(sopt.BeforeFirst(','));
	*coordy=wxAtoi(sopt.AfterFirst(','));
}

void config::SetTable(CONFIG opt, wxArrayString &asopt,wxString split)
{
	wxString sresult;
	wxString ES;
	for(size_t i=0;i<asopt.size();i++)
	{
		wxString endchar=(i==asopt.size()-1)? ES : split;
		sresult<<asopt[i]<<endchar;
	}
	rawcfg[opt]=sresult;
}

void config::SetIntTable(CONFIG opt, wxArrayInt &asopt,wxString split)
{
	wxString sresult;
	wxString ES;
	for(size_t i=0;i<asopt.size();i++)
	{
		wxString endchar=(i==asopt.size()-1)? ES : split;
		sresult<<asopt[i]<<endchar;
	}
	rawcfg[opt]=sresult;
}

void config::GetTable(CONFIG opt, wxArrayString &tbl, wxString split)
{
	wxString strtbl=rawcfg[opt];
	if(strtbl!=""){
		wxStringTokenizer cfgtable(strtbl,split,wxTOKEN_STRTOK);
		while(cfgtable.HasMoreTokens()){
			tbl.Add(cfgtable.NextToken());
		}  
	}
}

void config::GetIntTable(CONFIG opt, wxArrayInt &tbl, wxString split)
{
	wxString strtbl=rawcfg[opt];
	if(strtbl!=""){
		wxStringTokenizer cfgtable(strtbl,split,wxTOKEN_STRTOK);
		while(cfgtable.HasMoreTokens()){
			tbl.Add(wxAtoi(cfgtable.NextToken()));
		}  
	}
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
	rawcfg[AudioAutoCommit] = "true";
	rawcfg[AudioAutoFocus] = "true";
	rawcfg[AudioAutoScroll] = "true";
	rawcfg[AudioBoxHeight] = "169";
	rawcfg[AudioDelay] = "0";
	rawcfg[AudioDrawTimeCursor] = "true";
	rawcfg[AudioDrawKeyframes] = "true";
	rawcfg[AudioDrawSecondaryLines] = "true";
	rawcfg[AudioDrawSelectionBackground] = "true";
	rawcfg[AudioDrawVideoPosition] = "true";
	rawcfg[AudioGrabTimesOnSelect] = "true";
	rawcfg[AudioHorizontalZoom] = "50";
	rawcfg[AudioInactiveLinesDisplayMode] = "1";
	rawcfg[AudioKaraoke] = "false";
	rawcfg[AudioKaraokeSplitMode] = "true";
	rawcfg[AudioLeadIn] = "200";
	rawcfg[AudioLeadOut] = "300";
	rawcfg[AudioLineBoundariesThickness] = "2";
	rawcfg[AudioLink] = "false";
	rawcfg[AudioLockScrollOnCursor] = "false";
	rawcfg[AudioMarkPlayTime] = "1000";
	rawcfg[AudioNextLineOnCommit] = "true";
	rawcfg[AudioRAMCache] = "false";
	rawcfg[AudioSnapToKeyframes] = "false";
	rawcfg[AudioSnapToOtherLines] = "false";
	rawcfg[AudioSpectrumOn] = "false";
	rawcfg[AudioStartDragSensitivity] = "2";
	rawcfg[AudioVerticalZoom] = "50";
	rawcfg[AudioVolume] = "50";
	rawcfg[AudioWheelDefaultToZoom] = "false";

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
	colors[GetCOLORValue(name)]=new wxColour(r, g, b, a);
}

wxString config::GetStringColor(std::map<COLOR, wxColour*>::iterator it)
{
	wxColour *col = it->second;
	if (col->Alpha() < 0xFF)
		return wxString::Format("#%02X%02X%02X%02X", col->Alpha(), col->Red(), col->Green(), col->Blue());
	return wxString::Format("#%02X%02X%02X", col->Red(), col->Green(), col->Blue());
}
wxString config::GetStringColor(COLOR optionName)
{
	wxColour *col = colors[optionName];
	if (col->Alpha() < 0xFF)
		return wxString::Format("#%02X%02X%02X%02X", col->Alpha(), col->Red(), col->Green(), col->Blue());
	return wxString::Format("#%02X%02X%02X", col->Red(), col->Green(), col->Blue());
}

void config::SaveColors(const wxString &path){
	wxString finalpath = path;
	if(path.IsEmpty()){
		finalpath = pathfull + "\\Themes\\" + GetString(ProgramTheme) + ".txt";
	}
	OpenWrite ow(finalpath,true);
	for(auto it = colors.begin(); it != colors.end(); it++){
		ow.PartFileWrite(wxString(::GetString(it->first)) + "=" + GetStringColor(it) + "\r\n");
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

DEFINE_ENUM(CONFIG,CFG);

DEFINE_ENUM(COLOR,CLR);

config Options;




