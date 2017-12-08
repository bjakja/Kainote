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
#include "gitparams.h"

#define ADD_QUOTES_HELPER(s) #s
#define ADD_QUOTES(s) ADD_QUOTES_HELPER(s)

config::config()
{
	wxString gitVersion ;
#ifdef GIT_CUR_COMMIT
	gitVersion = " - " + wxString(ADD_QUOTES(GIT_BRANCH)) + "-" + wxString(ADD_QUOTES(GIT_CUR_COMMIT)).Left(7);
#endif
	progname = _T("Kainote v") + wxString(VersionKainote) + gitVersion;
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

const wxColour &config::GetColour(COLOR opt)
{
	auto it = colors.find(opt);
	if(it!=colors.end()){
		return *it->second;
	}
	return defaultColour;
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
		if((!Audio && cur->first <= AudioWheelDefaultToZoom) || (Audio && cur->first > AudioWheelDefaultToZoom)) {continue;}
		TextOpt<<::GetString(cur->first) << _T("=") << cur->second << _T("\r\n");
	}
	return TextOpt;
}

void config::CatchValsLabs(const wxString &line)
{
	wxString Values=line.AfterFirst('=');
	//Values.Trim(false);
	//Values.Trim(true);
	wxString Labels=line.BeforeFirst('=');
	//Labels.Trim(false);
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
	//rawcfg[ASSPropertiesAskForChange] = "true";
	rawcfg[MoveTimesTime] = "2000";
	rawcfg[MoveTimesWhichLines] = "0";
	rawcfg[ConvertResolutionWidth] = "1280";
	rawcfg[ConvertResolutionHeight] = "720";
	rawcfg[ConvertFPS] = "23.976";
	rawcfg[ConvertStyle] = "Default";
	rawcfg[ConvertStyleCatalog] = "Default";
	rawcfg[DictionaryLanguage] = "pl";
	rawcfg[SpellcheckerOn] = "true";
	rawcfg[StyleEditFilterText] = "ĄĆĘŁŃÓŚŹŻąćęłńóśźż";
	rawcfg[FFMS2VideoSeeking] = "2";
	rawcfg[MoveTimesByTime] = "false";
	rawcfg[GridFontName] = "Tahoma";
	rawcfg[GridFontSize] = "10";
	rawcfg[GridSaveAfterCharacterCount] = "1";
	rawcfg[GridTagsSwapChar] = L"☀";
	rawcfg[MoveTimesForward] = "true";
	rawcfg[ConvertNewEndTimes] = "false";
	rawcfg[InsertStartOffset] = "0";
	rawcfg[InsertEndOffset] = "0";
	rawcfg[PlayAfterSelection] = "0";
	rawcfg[PreviewText] = "Podgląd";
	rawcfg[ProgramTheme] = "DeepDark";
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
	rawcfg[AutomationTraceLevel] = "3";
	rawcfg[AutoSaveMaxFiles] = "3";
	rawcfg[GridChangeActiveOnSelection] = "true";
}

void config::LoadDefaultColors(bool dark, std::map<COLOR, wxColour*> *table)
{
	auto &colours = (table) ? *table : colors;
	colours[WindowBackground] = new wxColour((dark)? "#222222" : "#F0F0F0");
	colours[WindowBackgroundInactive] = new wxColour((dark)? "#303030" : "#EFEFEF");
	colours[WindowText] = new wxColour((dark)? "#9B9B9B" : "#000000");
	colours[WindowTextInactive] = new wxColour((dark)? "#4E4E4E" : "#757575");
	colours[WindowBorder] = new wxColour((dark)? "#111111" : "#4086C3");
	colours[WindowBorderInactive] = new wxColour((dark)? "#353535" : "#D3D3D3");
	colours[WindowBorderBackground] = new wxColour((dark)? "#786AEB" : "#54AFFF");
	colours[WindowBorderBackgroundInactive] = new wxColour((dark)? "#818181" : "#EBEBEB");
	colours[WindowHeaderText] = new wxColour((dark)? "#1A1A1A" : "#000000");
	colours[WindowHeaderTextInactive] = new wxColour((dark)? "#1A1A1A" : "#000000");
	colours[WindowHoverHeaderElement] = new wxColour((dark)? "#044BD4" : "#3665B3");
	colours[WindowPushedHeaderElement] = new wxColour((dark)? "#0451E5" : "#3D6099");
	colours[WindowHoverCloseButton] = new wxColour((dark)? "#D40403" : "#E04343");
	colours[WindowPushedCloseButton] = new wxColour((dark)? "#E50403" : "#993D3D");
	colours[WindowWarningElements] = new wxColour((dark)? "#E0D812" : "#FF0000");
	colours[GridText] = new wxColour((dark)? "#121212" : "#000000");
	colours[GridBackground] = new wxColour((dark)? "#444444" : "#D7D7D7");
	colours[GridDialogue] = new wxColour((dark)? "#969696" : "#C0C0C0");
	colours[GridComment] = new wxColour((dark)? "#BDBDBD" : "#999999");
	colours[GridSelection] = (dark) ? new wxColour(0x5A, 0xD6, 0x93, 76) : new wxColour(0x8E, 0xF8, 0xB5, 76);//#4C5AD693 #4C8EF8B5 new wxColour((dark)? "#B6D5C5" : "#EBF5EF");
	colours[GridVisibleOnVideo] = new wxColour((dark)? "#7878AA" : "#94B9D7");
	colours[GridCollisions] = new wxColour((dark)? "#0010FF" : "#FF0000");
	colours[GridLines] = new wxColour((dark)? "#4C4C4C" : "#606060");
	colours[GridActiveLine] = new wxColour((dark)? "#4000CA" : "#CA0065");
	colours[GridHeader] = new wxColour((dark)? "#6551FF" : "#54AFFF");
	colours[GridHeaderText] = new wxColour((dark)? "#121212" : "#000000");
	colours[GridLabelNormal] = new wxColour((dark)? "#6551FF" : "#54AFFF");
	colours[GridLabelModified] = new wxColour((dark)? "#A8FB05" : "#FBF804");
	colours[GridLabelSaved] = new wxColour((dark)? "#A398FF" : "#B8DFFF");
	colours[GridLabelDoubtful] = new wxColour((dark)? "#FB9708" : "#FB710B");
	colours[GridSpellchecker] = new wxColour((dark)? "#FA9292" : "#FA9292");
	colours[GridComparisonOutline] = new wxColour((dark)? "#FFFFFF" : "#00E1FF");
	colours[GridComparisonBackgroundNotMatch] = new wxColour((dark)? "#C0A073" : "#C8A679");
	colours[GridComparisonBackgroundMatch] = new wxColour((dark)? "#8F8DB3" : "#8F8DB3");
	colours[GridComparisonCommentBackgroundNotMatch] = new wxColour((dark)? "#F3C38F" : "#F3C38F");
	colours[GridComparisonCommentBackgroundMatch] = new wxColour((dark)? "#BEBCFB" : "#BEBCFB");
	colours[EditorText] = new wxColour((dark)? "#B7B7B7" : "#000000");
	colours[EditorTagNames] = new wxColour((dark)? "#E2A000" : "#17AF21");
	colours[EditorTagValues] = new wxColour((dark)? "#EE54FF" : "#BD16D0");
	colours[EditorCurlyBraces] = new wxColour((dark)? "#8489FF" : "#1622D0");
	colours[EditorTagOperators] = new wxColour((dark)? "#F96666" : "#FA0012");
	colours[EditorBracesBackground] = new wxColour((dark)? "#000000" : "#A6A6A6");
	colours[EditorBackground] = new wxColour((dark)? "#444444" : "#FFFFFF");
	colours[EditorSelection] = new wxColour((dark)? "#6E6E6E" : "#99CCFF");
	colours[EditorSelectionNoFocus] = new wxColour((dark)? "#65657E" : "#CCCCCC");
	colours[EditorBorder] = new wxColour((dark)? "#4E4E4E" : "#828790");
	colours[EditorBorderOnFocus] = new wxColour((dark)? "#8C8C8C" : "#569DE5");
	colours[EditorSpellchecker] = new wxColour((dark)? "#940000" : "#FA9292");
	colours[AudioBackground] = new wxColour((dark)? "#000000" : "#000000");
	colours[AudioLineBoundaryStart] = new wxColour((dark)? "#D80000" : "#D80000");
	colours[AudioLineBoundaryEnd] = new wxColour((dark)? "#E67D00" : "#D09916");
	colours[AudioLineBoundaryMark] = new wxColour("#FF00FF");
	colours[AudioLineBoundaryInactiveLine] = new wxColour("#808080");
	colours[AudioPlayCursor] = new wxColour("#FFFFFF");
	colours[AudioSecondsBoundaries] = new wxColour(0x0B,0x8A,0x9F,0x9B);//wxColour((dark)? "#9B0B8A9F" : "");
	colours[AudioKeyframes] = new wxColour("#4AFF00");
	colours[AudioSyllableBoundaries] = new wxColour("#FFFF00");
	colours[AudioSyllableText] = new wxColour("#FF0000");
	colours[AudioSelectionBackground] = new wxColour(0xFF,0xFF,0xFF,0x37);//wxColour((dark)? "#37FFFFFF" : "");
	colours[AudioSelectionBackgroundModified] = new wxColour(0xD6,0x00,0x00,0x37);//wxColour((dark)? "#37D60000" : "");
	colours[AudioInactiveLinesBackground] = new wxColour(0x37,0x35,0x64,0x55);//wxColour((dark)? "#55373564" : "");
	colours[AudioWaveform] = new wxColour("#7D6EFE");
	colours[AudioWaveformInactive] = new wxColour("#362C88");
	colours[AudioWaveformModified] = new wxColour("#FFE6E6");
	colours[AudioWaveformSelected] = new wxColour("#DCDAFF");
	colours[AudioSpectrumBackground] = new wxColour("#000000");
	colours[AudioSpectrumEcho] = new wxColour("#8DA8FF");
	colours[AudioSpectrumInner] = new wxColour("#FFFFFF");
	colours[TextFieldBackground] = new wxColour((dark)? "#2A2A2A" : "#FFFFFF");
	colours[TextFieldBorder] = new wxColour((dark)? "#4E4E4E" : "#ACACAC");
	colours[TextFieldBorderOnFocus] = new wxColour((dark)? "#8C8C8C" : "#569DE5");
	colours[TextFieldSelection] = new wxColour((dark)? "#6E6E6E" : "#3399FF");
	colours[TextFieldSelectionNoFocus] = new wxColour((dark)? "#65657E" : "#A6A6A6");
	colours[ButtonBackground] = new wxColour((dark)? "#2A2A2A" : "#E5E5E5");
	colours[ButtonBackgroundHover] = new wxColour((dark)? "#333333" : "#DCECFC");
	colours[ButtonBackgroundPushed] = new wxColour((dark)? "#333333" : "#C4E0FC");
	colours[ButtonBackgroundOnFocus] = new wxColour((dark)? "#2A2A2A" : "#E5E5E5");
	colours[ButtonBorder] = new wxColour((dark)? "#4E4E4E" : "#ACACAC");
	colours[ButtonBorderHover] = new wxColour((dark)? "#555555" : "#7EB4EA");
	colours[ButtonBorderPushed] = new wxColour((dark)? "#555555" : "#569DE5");
	colours[ButtonBorderOnFocus] = new wxColour((dark)? "#5B5689" : "#569DE5");
	colours[ButtonBorderInactive] = new wxColour((dark)? "#5A5A5A" : "#BFBFBF");
	colours[TogglebuttonBackgroundToggled] = new wxColour((dark)? "#39374B" : "#C4E0FC");
	colours[TogglebuttonBorderToggled] = new wxColour((dark)? "#5B5689" : "#569DE5");
	colours[ScrollbarBackground] = new wxColour((dark)? "#3A3A3A" : "#E7E7E7");
	colours[ScrollbarScroll] = new wxColour((dark)? "#686868" : "#CDCDCD");
	colours[ScrollbarScrollHover] = new wxColour((dark)? "#868686" : "#A6A6A6");
	colours[ScrollbarScrollPushed] = new wxColour((dark)? "#868686" : "#A6A6A6");
	colours[StaticboxBorder] = new wxColour((dark)? "#565555" : "#DDDDDD");
	colours[StaticListBorder] = new wxColour((dark)? "#4E4E4E" : "#ACACAC");
	colours[StaticListBackground] = new wxColour((dark)? "#2A2A2A" : "#FFFFFF");
	colours[StaticListSelection] = new wxColour((dark)? "#2D2879" : "#6DBAFF");
	colours[StaticListBackgroundHeadline] = new wxColour((dark)? "#222222" : "#F6F6F6");
	colours[StaticListTextHeadline] = new wxColour((dark)? "#9B9B9B" : "#000000");
	colours[StatusBarBorder] = new wxColour((dark)? "#000000" : "#ACACAC");
	colours[MenuBarBackground1] = new wxColour((dark)? "#4A4A4A" : "#F0F0F0");
	colours[MenuBarBackground2] = new wxColour((dark)? "#222222" : "#F0F0F0");
	colours[MenuBarBorderSelection] = new wxColour((dark)? "#948FC4" : "#66A7E8");
	colours[MenuBarBackgroundSelection] = new wxColour((dark)? "#39374B" : "#D1E8FF");
	colours[MenuBackground] = new wxColour((dark)? "#323232" : "#F0F0F0");
	colours[MenuBorderSelection] = new wxColour((dark)? "#5B5689" : "#66A7E8");
	colours[MenuBackgroundSelection] = new wxColour((dark)? "#39374B" : "#D1E8FF");
	colours[TabsBarBackground1] = new wxColour((dark)? "#222222" : "#F0F0F0");
	colours[TabsBarBackground2] = new wxColour((dark)? "#222222" : "#F0F0F0");
	colours[TabsBorderActive] = new wxColour((dark)? "#000000" : "#8A8A8A");
	colours[TabsBorderInactive] = new wxColour((dark)? "#000000" : "#ACACAC");
	colours[TabsBackgroundActive] = new wxColour((dark)? "#2D2D2D" : "#C0C0C0");
	colours[TabsBackgroundInactive] = new wxColour((dark)? "#3C3C3C" : "#F0F0F0");
	colours[TabsBackgroundInactiveHover] = new wxColour((dark) ? "#2D2D2D" : "#D1D1D1"); 
	colours[TabsBackgroundSecondWindow] = new wxColour((dark) ? "#181818" : "#AFAFAF");
	colours[TabsTextActive] = new wxColour((dark)? "#B5B5B5" : "#000000");
	colours[TabsTextInactive] = new wxColour((dark)? "#9B9B9B" : "#333333");
	colours[TabsCloseHover] = new wxColour((dark)? "#D40403" : "#E04343");
	colours[TabsBarArrow] = new wxColour((dark)? "#9B9B9B" : "#000000");
	colours[TabsBarArrowBackground] = new wxColour((dark)? "#222222" : "#F0F0F0");
	colours[TabsBarArrowBackgroundHover] = new wxColour((dark)? "#333333" : "#D1D1D1");
	colours[SliderPathBackground] = new wxColour((dark)? "#282828" : "#E7EAEA");
	colours[SliderPathBorder] = new wxColour((dark)? "#434343" : "#D6D6D6");
	colours[SliderBorder] = new wxColour((dark)? "#4E4E4E" : "#ACACAC");
	colours[SliderBorderHover] = new wxColour((dark)? "#555555" : "#7EB4EA");
	colours[SliderBorderPushed] = new wxColour((dark)? "#555555" : "#569DE5");
	colours[SliderBackground] = new wxColour((dark)? "#282828" : "#E5E5E5");
	colours[SliderBackgroundHover] = new wxColour((dark)? "#333333" : "#DCECFC");
	colours[SliderBackgroundPushed] = new wxColour((dark)? "#333333" : "#C4E0FC");
	colours[StylePreviewColor1] = new wxColour("#9A9A9A");
	colours[StylePreviewColor2] = new wxColour("#686868");

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
	if(themeName!="DeepDark" && themeName!="DeepLight"){
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
			if(colors.size()>10){
				if (colors.find((COLOR)0) != colors.end() || colors.size() < StylePreviewColor2){
					LoadMissingColours(path);
					KaiMessageBox(wxString::Format(_("W motywie \"%s\" brakowało część kolorów, zostały doczytane z domyślnego."), themeName));
				}
				return;
			}
		}
		failed = true;
	}
	LoadDefaultColors(themeName!="DeepLight");	
	if(failed){
		Options.SetString(ProgramTheme, "DeepDark");
		KaiMessageBox(_("Nie można zaczytać motywu, zostanie przywrócony domyśny"));
	}
}

void config::LoadMissingColours(const wxString &path)
{
	std::map<COLOR, wxColour*> defaultColors;
	LoadDefaultColors(true, &defaultColors);
	auto j = colors.begin();
	if (j->first == 0){ 
		j++; 
		delete colors.begin()->second;
		colors.erase(colors.begin()); 
	}
	for (auto i = defaultColors.begin(); i != defaultColors.end(); i++){
		if (j != colors.end() && i->first == j->first){
			delete i->second;
			j++;
		} else{
			colors[i->first] = i->second;
		}
	}
	defaultColors.clear();
	SaveColors(path);
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

void config::GetTable(CONFIG opt, wxArrayString &tbl, wxString split, int mode)
{
	wxString strtbl=rawcfg[opt];
	if(strtbl!=""){
		wxStringTokenizer cfgtable(strtbl,split,(wxStringTokenizerMode)mode);
		while(cfgtable.HasMoreTokens()){
			tbl.Add(cfgtable.NextToken());
		}  
	}
}

void config::GetIntTable(CONFIG opt, wxArrayInt &tbl, wxString split, int mode)
{
	wxString strtbl=rawcfg[opt];
	if(strtbl!=""){
		wxStringTokenizer cfgtable(strtbl,split,(wxStringTokenizerMode)mode);
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
	rawcfg[AudioStartDragSensitivity] = "6";
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

wxString getfloat(float num, const wxString &format, bool Truncate)
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
	//wxLogStatus("strnum %s num %f", strnum, num);
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
		if(mst.y<0){
			mst.y = rc.height - siz.y;
		}
	}
	win->Move(mst);
}

wxString MakePolishPlural(int num, const wxString &normal, const wxString &plural2to4, const wxString &pluralRest)
{
	wxString result;
	int div10mod = (num % 10);
	int div100mod = (num % 100);
	if (num == 1 || num == -1){ result = normal; }
	else if((div10mod >=2 && div10mod <= 4) && (div100mod<10 || div100mod>20)){
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




