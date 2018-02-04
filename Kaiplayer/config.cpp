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
	gitVersion = "  " + wxString(ADD_QUOTES(GIT_BRANCH)) + " " + wxString(ADD_QUOTES(GIT_CUR_COMMIT)).Left(7);
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
	for(std::vector<Styles*>::iterator it=assstore.begin(); it!=assstore.end(); it++)
	{
		delete (*it);
	}
	assstore.clear();
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

const wxString & config::GetString(CONFIG opt)
{
	if (opt >= 0 && opt < configSize)
		return stringConfig[opt];

	return emptyString;
}


bool config::GetBool(CONFIG opt)
{   
	if (opt >= 0 && opt < configSize){
		wxString ropt = stringConfig[opt];
		if (ropt == _T("true")){ return true; }
	}
	return false;
}

const wxColour &config::GetColour(COLOR opt)
{
	if (opt >=0 && opt < colorsSize)
		return colors[opt];

	return defaultColour;
}

AssColor config::GetColor(COLOR opt)
{
	if (opt >= 0 && opt < colorsSize)
		return AssColor(colors[opt]);
		
	return AssColor();
}

int config::GetInt(CONFIG opt)
{
	if (opt >= 0 && opt < configSize)
		return wxAtoi(stringConfig[opt]);

	return 0;
}
float config::GetFloat(CONFIG opt)
{   
	if (opt >= 0 && opt < configSize){
		double fl;
		wxString rawfloat = stringConfig[opt];
		if (!rawfloat.ToDouble(&fl)){ return 0.0; }
		return fl;
	}
	return 0.0;
}

void config::SetString(CONFIG opt, const wxString &sopt)
{
	if (opt >= 0 && opt < configSize)
		stringConfig[opt]=sopt;
}

void config::SetBool(CONFIG opt, bool bopt)
{
	if (opt >= 0 && opt < configSize){
		wxString bopt1 = (bopt) ? _T("true") : _T("false");
		stringConfig[opt] = bopt1;
	}
}

void config::SetColour(COLOR opt, wxColour &copt)
{
	if (opt > 0 && opt < colorsSize)
		colors[opt] = copt;
}

void config::SetColor(COLOR opt, AssColor &copt)
{
	if (opt > 0 && opt < colorsSize)
		colors[opt] = copt.GetWX();
}

void config::SetInt(CONFIG opt, int iopt)
{
	if (opt >= 0 && opt < configSize){
		wxString iopt1 = _T("");
		stringConfig[opt] = iopt1 << iopt;
	}
}

void config::SetFloat(CONFIG opt, float fopt)
{
	if (opt >= 0 && opt < configSize){
		wxString fopt1 = _T("");
		fopt1 << fopt;
		fopt1.Replace(",", ".");
		stringConfig[opt] = fopt1;
	}
}

void config::GetRawOptions(wxString &options, bool Audio/*=false*/)
{
	options = _T("[" + progname + "]\r\n");
	for (size_t i = 1; i < configSize; i++) {
		if ((!Audio && i <= AudioWheelDefaultToZoom) || (Audio && i > AudioWheelDefaultToZoom) || stringConfig[i].empty()) { continue; }
		options << ::GetString((CONFIG)i) << _T("=") << stringConfig[i] << _T("\r\n");
	}
}

void config::CatchValsLabs(const wxString &line)
{
	wxString Values=line.AfterFirst('=');
	//Values.Trim(false);
	//Values.Trim(true);
	wxString Labels=line.BeforeFirst('=');
	//Labels.Trim(false);
	Labels.Trim(true);
	stringConfig[GetCONFIGValue(Labels)] = Values;
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
		wxString textfile;
		GetRawOptions(textfile);
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
		path<<pathfull<<_T("\\Catalog\\")<<actualStyleDir<<_T(".sty");
		ow.FileWrite(path, stylefile);
	}
}

void config::LoadDefaultConfig()
{
	//rawcfg[ASSPropertiesAskForChange] = "true";
	stringConfig[MoveTimesTime] = "2000";
	stringConfig[MoveTimesWhichLines] = "0";
	stringConfig[ConvertResolutionWidth] = "1280";
	stringConfig[ConvertResolutionHeight] = "720";
	stringConfig[ConvertFPS] = "23.976";
	stringConfig[ConvertStyle] = "Default";
	stringConfig[ConvertStyleCatalog] = "Default";
	stringConfig[DictionaryLanguage] = "pl";
	stringConfig[SpellcheckerOn] = "true";
	stringConfig[StyleEditFilterText] = "ĄĆĘŁŃÓŚŹŻąćęłńóśźż";
	stringConfig[FFMS2VideoSeeking] = "2";
	stringConfig[MoveTimesByTime] = "false";
	stringConfig[GridFontName] = "Tahoma";
	stringConfig[GridFontSize] = "10";
	stringConfig[GridSaveAfterCharacterCount] = "1";
	stringConfig[GridTagsSwapChar] = L"☀";
	stringConfig[MoveTimesForward] = "true";
	stringConfig[ConvertNewEndTimes] = "false";
	stringConfig[InsertStartOffset] = "0";
	stringConfig[InsertEndOffset] = "0";
	stringConfig[PlayAfterSelection] = "0";
	stringConfig[PreviewText] = "Podgląd";
	stringConfig[ProgramTheme] = "DeepDark";
	stringConfig[EditorOn] = "true";
	stringConfig[ConvertShowSettings] = "false";
	stringConfig[MoveTimesOn] = "true";
	stringConfig[MoveTimesWhichTimes] = "0";
	stringConfig[MoveTimesStyles] = "";
	stringConfig[ConvertTimePerLetter] = "110";
	stringConfig[VideoIndex] = "true";
	stringConfig[VideoProgressBar] = "true";
	stringConfig[VideoWindowSize] = "500,350";
	stringConfig[WindowSize] = "800,600";
	stringConfig[AutomationTraceLevel] = "3";
	stringConfig[AutoSaveMaxFiles] = "3";
	stringConfig[GridChangeActiveOnSelection] = "true";
}

//remember, create table[colorsSize] without this size it will crash
void config::LoadDefaultColors(bool dark, wxColour *table)
{
	wxColour *colours = (table) ? table : colors;
	colours[WindowBackground].Set((dark)? "#222222" : "#F0F0F0");
	colours[WindowBackgroundInactive].Set((dark)? "#303030" : "#EFEFEF");
	colours[WindowText].Set((dark)? "#9B9B9B" : "#000000");
	colours[WindowTextInactive].Set((dark)? "#4E4E4E" : "#757575");
	colours[WindowBorder].Set((dark)? "#111111" : "#4086C3");
	colours[WindowBorderInactive].Set((dark)? "#353535" : "#D3D3D3");
	colours[WindowBorderBackground].Set((dark)? "#786AEB" : "#54AFFF");
	colours[WindowBorderBackgroundInactive].Set((dark)? "#818181" : "#EBEBEB");
	colours[WindowHeaderText].Set((dark)? "#1A1A1A" : "#000000");
	colours[WindowHeaderTextInactive].Set((dark)? "#1A1A1A" : "#000000");
	colours[WindowHoverHeaderElement].Set((dark)? "#044BD4" : "#3665B3");
	colours[WindowPushedHeaderElement].Set((dark)? "#0451E5" : "#3D6099");
	colours[WindowHoverCloseButton].Set((dark)? "#D40403" : "#E04343");
	colours[WindowPushedCloseButton].Set((dark)? "#E50403" : "#993D3D");
	colours[WindowWarningElements].Set((dark)? "#E0D812" : "#FF0000");
	colours[GridText].Set((dark)? "#121212" : "#000000");
	colours[GridBackground].Set((dark)? "#444444" : "#D7D7D7");
	colours[GridDialogue].Set((dark)? "#969696" : "#C0C0C0");
	colours[GridComment].Set((dark)? "#BDBDBD" : "#999999");
	colours[GridSelection] = (dark) ? wxColour(0x5A, 0xD6, 0x93, 76) :  wxColour(0x8E, 0xF8, 0xB5, 76);//#4C5AD693 #4C8EF8B5 new wxColour((dark)? "#B6D5C5" : "#EBF5EF");
	colours[GridVisibleOnVideo].Set((dark)? "#7878AA" : "#94B9D7");
	colours[GridCollisions].Set((dark)? "#0010FF" : "#FF0000");
	colours[GridLines].Set((dark)? "#4C4C4C" : "#606060");
	colours[GridActiveLine].Set((dark)? "#4000CA" : "#CA0065");
	colours[GridHeader].Set((dark)? "#6551FF" : "#54AFFF");
	colours[GridHeaderText].Set((dark)? "#121212" : "#000000");
	colours[GridLabelNormal].Set((dark)? "#6551FF" : "#54AFFF");
	colours[GridLabelModified].Set((dark)? "#A8FB05" : "#FBF804");
	colours[GridLabelSaved].Set((dark)? "#A398FF" : "#B8DFFF");
	colours[GridLabelDoubtful].Set((dark)? "#FB9708" : "#FB710B");
	colours[GridSpellchecker].Set((dark)? "#FA9292" : "#FA9292");
	colours[GridComparisonOutline].Set((dark)? "#FFFFFF" : "#00E1FF");
	colours[GridComparisonBackgroundNotMatch].Set((dark)? "#C0A073" : "#C8A679");
	colours[GridComparisonBackgroundMatch].Set((dark)? "#8F8DB3" : "#8F8DB3");
	colours[GridComparisonCommentBackgroundNotMatch].Set((dark)? "#F3C38F" : "#F3C38F");
	colours[GridComparisonCommentBackgroundMatch].Set((dark)? "#BEBCFB" : "#BEBCFB");
	colours[EditorText].Set((dark)? "#B7B7B7" : "#000000");
	colours[EditorTagNames].Set((dark)? "#E2A000" : "#17AF21");
	colours[EditorTagValues].Set((dark)? "#EE54FF" : "#BD16D0");
	colours[EditorCurlyBraces].Set((dark)? "#8489FF" : "#1622D0");
	colours[EditorTagOperators].Set((dark)? "#F96666" : "#FA0012");
	colours[EditorBracesBackground].Set((dark)? "#000000" : "#A6A6A6");
	colours[EditorBackground].Set((dark)? "#444444" : "#FFFFFF");
	colours[EditorSelection].Set((dark)? "#6E6E6E" : "#99CCFF");
	colours[EditorSelectionNoFocus].Set((dark)? "#65657E" : "#CCCCCC");
	colours[EditorBorder].Set((dark)? "#4E4E4E" : "#828790");
	colours[EditorBorderOnFocus].Set((dark)? "#8C8C8C" : "#569DE5");
	colours[EditorSpellchecker].Set((dark)? "#940000" : "#FA9292");
	colours[AudioBackground].Set((dark)? "#000000" : "#000000");
	colours[AudioLineBoundaryStart].Set((dark)? "#D80000" : "#D80000");
	colours[AudioLineBoundaryEnd].Set((dark)? "#E67D00" : "#D09916");
	colours[AudioLineBoundaryMark].Set("#FF00FF");
	colours[AudioLineBoundaryInactiveLine].Set("#808080");
	colours[AudioPlayCursor].Set("#FFFFFF");
	colours[AudioSecondsBoundaries].Set(0x0B,0x8A,0x9F,0x9B);//wxColour((dark)? "#9B0B8A9F" : "");
	colours[AudioKeyframes].Set("#4AFF00");
	colours[AudioSyllableBoundaries].Set("#FFFF00");
	colours[AudioSyllableText].Set("#FF0000");
	colours[AudioSelectionBackground].Set(0xFF,0xFF,0xFF,0x37);//wxColour((dark)? "#37FFFFFF" : "");
	colours[AudioSelectionBackgroundModified].Set(0xD6,0x00,0x00,0x37);//wxColour((dark)? "#37D60000" : "");
	colours[AudioInactiveLinesBackground].Set(0x37,0x35,0x64,0x55);//wxColour((dark)? "#55373564" : "");
	colours[AudioWaveform].Set("#7D6EFE");
	colours[AudioWaveformInactive].Set("#362C88");
	colours[AudioWaveformModified].Set("#FFE6E6");
	colours[AudioWaveformSelected].Set("#DCDAFF");
	colours[AudioSpectrumBackground].Set("#000000");
	colours[AudioSpectrumEcho].Set("#8DA8FF");
	colours[AudioSpectrumInner].Set("#FFFFFF");
	colours[TextFieldBackground].Set((dark)? "#2A2A2A" : "#FFFFFF");
	colours[TextFieldBorder].Set((dark)? "#4E4E4E" : "#ACACAC");
	colours[TextFieldBorderOnFocus].Set((dark)? "#8C8C8C" : "#569DE5");
	colours[TextFieldSelection].Set((dark)? "#6E6E6E" : "#3399FF");
	colours[TextFieldSelectionNoFocus].Set((dark)? "#65657E" : "#A6A6A6");
	colours[ButtonBackground].Set((dark)? "#2A2A2A" : "#E5E5E5");
	colours[ButtonBackgroundHover].Set((dark)? "#333333" : "#DCECFC");
	colours[ButtonBackgroundPushed].Set((dark)? "#333333" : "#C4E0FC");
	colours[ButtonBackgroundOnFocus].Set((dark)? "#2A2A2A" : "#E5E5E5");
	colours[ButtonBorder].Set((dark)? "#4E4E4E" : "#ACACAC");
	colours[ButtonBorderHover].Set((dark)? "#555555" : "#7EB4EA");
	colours[ButtonBorderPushed].Set((dark)? "#555555" : "#569DE5");
	colours[ButtonBorderOnFocus].Set((dark)? "#5B5689" : "#569DE5");
	colours[ButtonBorderInactive].Set((dark)? "#5A5A5A" : "#BFBFBF");
	colours[TogglebuttonBackgroundToggled].Set((dark)? "#39374B" : "#C4E0FC");
	colours[TogglebuttonBorderToggled].Set((dark)? "#5B5689" : "#569DE5");
	colours[ScrollbarBackground].Set((dark)? "#3A3A3A" : "#E7E7E7");
	colours[ScrollbarScroll].Set((dark)? "#686868" : "#CDCDCD");
	colours[ScrollbarScrollHover].Set((dark)? "#868686" : "#A6A6A6");
	colours[ScrollbarScrollPushed].Set((dark)? "#868686" : "#A6A6A6");
	colours[StaticboxBorder].Set((dark)? "#565555" : "#DDDDDD");
	colours[StaticListBorder].Set((dark)? "#4E4E4E" : "#ACACAC");
	colours[StaticListBackground].Set((dark)? "#2A2A2A" : "#FFFFFF");
	colours[StaticListSelection].Set((dark)? "#2D2879" : "#6DBAFF");
	colours[StaticListBackgroundHeadline].Set((dark)? "#222222" : "#F6F6F6");
	colours[StaticListTextHeadline].Set((dark)? "#9B9B9B" : "#000000");
	colours[StatusBarBorder].Set((dark)? "#000000" : "#ACACAC");
	colours[MenuBarBackground1].Set((dark)? "#4A4A4A" : "#F0F0F0");
	colours[MenuBarBackground2].Set((dark)? "#222222" : "#F0F0F0");
	colours[MenuBarBorderSelection].Set((dark)? "#948FC4" : "#66A7E8");
	colours[MenuBarBackgroundSelection].Set((dark)? "#39374B" : "#D1E8FF");
	colours[MenuBackground].Set((dark)? "#323232" : "#F0F0F0");
	colours[MenuBorderSelection].Set((dark)? "#5B5689" : "#66A7E8");
	colours[MenuBackgroundSelection].Set((dark)? "#39374B" : "#D1E8FF");
	colours[TabsBarBackground1].Set((dark)? "#222222" : "#F0F0F0");
	colours[TabsBarBackground2].Set((dark)? "#222222" : "#F0F0F0");
	colours[TabsBorderActive].Set((dark)? "#000000" : "#8A8A8A");
	colours[TabsBorderInactive].Set((dark)? "#000000" : "#ACACAC");
	colours[TabsBackgroundActive].Set((dark)? "#2D2D2D" : "#C0C0C0");
	colours[TabsBackgroundInactive].Set((dark)? "#3C3C3C" : "#F0F0F0");
	colours[TabsBackgroundInactiveHover].Set((dark) ? "#2D2D2D" : "#D1D1D1"); 
	colours[TabsBackgroundSecondWindow].Set((dark) ? "#181818" : "#AFAFAF");
	colours[TabsTextActive].Set((dark)? "#B5B5B5" : "#000000");
	colours[TabsTextInactive].Set((dark)? "#9B9B9B" : "#333333");
	colours[TabsCloseHover].Set((dark)? "#D40403" : "#E04343");
	colours[TabsBarArrow].Set((dark)? "#9B9B9B" : "#000000");
	colours[TabsBarArrowBackground].Set((dark)? "#222222" : "#F0F0F0");
	colours[TabsBarArrowBackgroundHover].Set((dark)? "#333333" : "#D1D1D1");
	colours[SliderPathBackground].Set((dark)? "#282828" : "#E7EAEA");
	colours[SliderPathBorder].Set((dark)? "#434343" : "#D6D6D6");
	colours[SliderBorder].Set((dark)? "#4E4E4E" : "#ACACAC");
	colours[SliderBorderHover].Set((dark)? "#555555" : "#7EB4EA");
	colours[SliderBorderPushed].Set((dark)? "#555555" : "#569DE5");
	colours[SliderBackground].Set((dark)? "#282828" : "#E5E5E5");
	colours[SliderBackgroundHover].Set((dark)? "#333333" : "#DCECFC");
	colours[SliderBackgroundPushed].Set((dark)? "#333333" : "#C4E0FC");
	colours[StylePreviewColor1].Set("#9A9A9A");
	colours[StylePreviewColor2].Set("#686868");

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

	actualStyleDir = _T("Default");
	path=_T("");
	path<<pathfull<<_T("\\Catalog\\");
	wxDir kat(path);
	if(!kat.IsOpened()){
		ow.FileWrite(path<<actualStyleDir<<_T(".sty"),_T("Style: Default,Garamond,30,&H00FFFFFF,&H000000FF,&H00FF0000,&H00000000,0,0,0,0,100,100,0,0,0,2,2,2,10,10,10,1"));
		AddStyle(new Styles());dirs.Add(actualStyleDir);
	}
	else{
		wxArrayString tmp;kat.GetAllFiles(path,&tmp,_T(""), wxDIR_FILES);
		for(size_t i=0;i<tmp.GetCount();i++){
			wxString fullpath=tmp[i].AfterLast('\\');
			if(fullpath.EndsWith(_T(".sty"))){dirs.Add(fullpath.BeforeLast('.'));}
		}
	}
	LoadStyles(actualStyleDir);
	LoadColors();
	return isgood;
}

void config::LoadColors(const wxString &_themeName){
	wxString themeName;
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
				if (token.Len()>6){SetHexColor(token);g++;}
			}
			if(g>10){
				if (colors[0].IsOk() || g < StylePreviewColor2){
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
	wxColour defaultColors[colorsSize];
	LoadDefaultColors(true, defaultColors);
	for (size_t i = 1; i < colorsSize; i++){
		if (!colors[i].IsOk())
			colors[i] = defaultColors[i];
	}
	SaveColors(path);
}


void config::LoadStyles(const wxString &katalog)
{
	actualStyleDir=katalog;
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

void config::SetCoords(CONFIG opt, int coordx, int coordy)
{
	wxString iopt1=_T("");
	stringConfig[opt]=iopt1<<coordx<<","<<coordy;
}

void config::GetCoords(CONFIG opt, int *coordx, int *coordy)
{
	wxString sopt=stringConfig[opt];
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
	stringConfig[opt]=sresult;
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
	stringConfig[opt]=sresult;
}

void config::GetTable(CONFIG opt, wxArrayString &tbl, wxString split, int mode)
{
	wxString strtbl=stringConfig[opt];
	if(strtbl!=""){
		wxStringTokenizer cfgtable(strtbl,split,(wxStringTokenizerMode)mode);
		while(cfgtable.HasMoreTokens()){
			tbl.Add(cfgtable.NextToken());
		}  
	}
}

void config::GetIntTable(CONFIG opt, wxArrayInt &tbl, wxString split, int mode)
{
	wxString strtbl=stringConfig[opt];
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
	stringConfig[AudioAutoCommit] = "true";
	stringConfig[AudioAutoFocus] = "true";
	stringConfig[AudioAutoScroll] = "true";
	stringConfig[AudioBoxHeight] = "169";
	stringConfig[AudioDelay] = "0";
	stringConfig[AudioDrawTimeCursor] = "true";
	stringConfig[AudioDrawKeyframes] = "true";
	stringConfig[AudioDrawSecondaryLines] = "true";
	stringConfig[AudioDrawSelectionBackground] = "true";
	stringConfig[AudioDrawVideoPosition] = "true";
	stringConfig[AudioGrabTimesOnSelect] = "true";
	stringConfig[AudioHorizontalZoom] = "50";
	stringConfig[AudioInactiveLinesDisplayMode] = "1";
	stringConfig[AudioKaraoke] = "false";
	stringConfig[AudioKaraokeSplitMode] = "true";
	stringConfig[AudioLeadIn] = "200";
	stringConfig[AudioLeadOut] = "300";
	stringConfig[AudioLineBoundariesThickness] = "2";
	stringConfig[AudioLink] = "false";
	stringConfig[AudioLockScrollOnCursor] = "false";
	stringConfig[AudioMarkPlayTime] = "1000";
	stringConfig[AudioNextLineOnCommit] = "true";
	stringConfig[AudioRAMCache] = "false";
	stringConfig[AudioSnapToKeyframes] = "false";
	stringConfig[AudioSnapToOtherLines] = "false";
	stringConfig[AudioSpectrumOn] = "false";
	stringConfig[AudioStartDragSensitivity] = "6";
	stringConfig[AudioVerticalZoom] = "50";
	stringConfig[AudioVolume] = "50";
	stringConfig[AudioWheelDefaultToZoom] = "false";

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
	wxString audioOpts;
	GetRawOptions(audioOpts, true);
	ow.FileWrite(pathfull + _T("\\AudioConfig.txt"), audioOpts);
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
	colors[GetCOLORValue(name)].Set(r, g, b, a);
}

wxString config::GetStringColor(size_t optionName)
{
	wxColour &col = colors[optionName];
	if (col.Alpha() < 0xFF)
		return wxString::Format("#%02X%02X%02X%02X", col.Alpha(), col.Red(), col.Green(), col.Blue());
	return wxString::Format("#%02X%02X%02X", col.Red(), col.Green(), col.Blue());
}

void config::SaveColors(const wxString &path){
	wxString finalpath = path;
	if(path.IsEmpty()){
		finalpath = pathfull + "\\Themes\\" + GetString(ProgramTheme) + ".txt";
	}
	OpenWrite ow(finalpath,true);
	for (size_t i = 1; i < colorsSize; i++){
		ow.PartFileWrite(wxString(::GetString((COLOR)i)) + "=" + GetStringColor(i) + "\r\n");
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
	wxRect rc = GetMonitorRect(0, NULL, win->GetParent(), true);//wxGetClientDisplayRect();
	mst.x-=(siz.x/2);
	mst.x = MID(rc.x, mst.x, (rc.width + rc.x) - siz.x);
	mst.y+=15;
	if(mst.y + siz.y > rc.height + rc.y){
		mst.y = mst.y - siz.y - 30;
		if (mst.y<rc.y){
			mst.y = (rc.height + rc.y) - siz.y;
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

BOOL CALLBACK MonitorEnumProc1(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
	std::pair<std::vector<RECT>, bool> *pair = (std::pair<std::vector<RECT>, bool> *)dwData;
	MONITORINFO monitorinfo;
	ZeroMemory(&monitorinfo, sizeof(monitorinfo));
	monitorinfo.cbSize = sizeof(monitorinfo);

	if (!GetMonitorInfo(hMonitor, &monitorinfo)){
		wxLogStatus(_("Nie można pobrać informacji o monitorze"));
		return TRUE;
	}
	//podstawowy monitor ma być pierwszy w tablicy
	if (monitorinfo.dwFlags == MONITORINFOF_PRIMARY){
		pair->first.insert(pair->first.begin(), (pair->second) ? monitorinfo.rcWork : monitorinfo.rcMonitor);
		return TRUE;
	}
	pair->first.push_back((pair->second) ? monitorinfo.rcWork : monitorinfo.rcMonitor);
	return TRUE;
}

wxRect GetMonitorRect(int wmonitor, std::vector<RECT> *MonitorRects, wxWindow *mainWindow, bool workArea){
	std::vector<RECT> MonRects;// = new std::vector<RECT>();
	std::pair<std::vector<RECT>, bool> *pair = new std::pair<std::vector<RECT>, bool>(MonRects, workArea);
	EnumDisplayMonitors(NULL, NULL, MonitorEnumProc1, (LPARAM)pair);
	MonRects = pair->first;
	delete pair;
	if (MonRects.size()==0){
		bool ktos_ukradl_ci_monitor = false;
		assert(ktos_ukradl_ci_monitor);
	}
	wxRect rt(MonRects[0].left, MonRects[0].top, abs(MonRects[0].right - MonRects[0].left), abs(MonRects[0].bottom - MonRects[0].top));
	if (wmonitor == -1 || MonRects.size() == 1){ return rt; }
	else if (wmonitor == 0){
		wxRect rect = mainWindow->GetRect();
		int x = (rect.width / 2) + rect.x;
		int y = (rect.height / 2) + rect.y;
		for (size_t i = 0; i < MonRects.size(); i++){
			if (MonRects[i].left <= x && x < MonRects[i].right && MonRects[i].top <= y && y < MonRects[i].bottom){
				if (MonitorRects)
					*MonitorRects = MonRects;
				return wxRect(MonRects[i].left, MonRects[i].top, abs(MonRects[i].right - MonRects[i].left), abs(MonRects[wmonitor].bottom - MonRects[wmonitor].top));
			}
		}
	}
	else{
		if (MonitorRects)
			*MonitorRects = MonRects;
		return wxRect(MonRects[wmonitor].left, MonRects[wmonitor].top, abs(MonRects[wmonitor].right - MonRects[wmonitor].left), abs(MonRects[wmonitor].bottom - MonRects[wmonitor].top));
	}
	return rt;
}

DEFINE_ENUM(CONFIG,CFG);

DEFINE_ENUM(COLOR,CLR);

config Options;




