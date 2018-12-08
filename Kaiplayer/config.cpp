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
#include "CsriMod.h"
#include "Tabs.h"//<windows.h>
#include "gitparams.h"
#include <windows.h>



#define ADD_QUOTES_HELPER(s) #s
#define ADD_QUOTES(s) ADD_QUOTES_HELPER(s)

config::config()
{
	wxString gitVersion;
#ifdef GIT_CUR_COMMIT
	gitVersion = "  " + wxString(ADD_QUOTES(GIT_BRANCH)) + " " + wxString(ADD_QUOTES(GIT_CUR_COMMIT)).Left(7);
#endif
	progname = _T("Kainote v") + wxString(VersionKainote) + gitVersion;
#if _DEBUG
	progname += " DEBUG";
#endif
	AudioOpts = false;
	defaultColour = wxColour();
}


config::~config()
{
	for (std::vector<Styles*>::iterator it = assstore.begin(); it != assstore.end(); it++)
	{
		delete (*it);
	}
	assstore.clear();
	if (vsfilter)
		csri_close_renderer(vsfilter);
}

wxString config::GetReleaseDate()
{
	return wxString(__DATE__) + "  " + wxString(__TIME__);
}


csri_rend * config::GetVSFilter()
{
	if (!vsfilter){
		vsfilter = csri_renderer_default();
		csri_info *info = csri_renderer_info(vsfilter);
		wxString name = GetString(VSFILTER_INSTANCE);
		while (info->name != name){
			vsfilter = csri_renderer_next(vsfilter);
			if (!vsfilter)
				break;
			info = csri_renderer_info(vsfilter);
		}
	}
	return vsfilter;
}

wxArrayString config::GetVSFiltersList()
{
	wxArrayString filtersList;
	csri_rend *filter = csri_renderer_default();
	if (!filter)
		return filtersList;
	csri_info *info = csri_renderer_info(filter);
	filtersList.Add(info->name);
	while (1){
		filter = csri_renderer_next(vsfilter);
		if (!filter)
			break;
		info = csri_renderer_info(filter);
		filtersList.Add(info->name);
	}
}

bool config::SetRawOptions(const wxString &textconfig)
{
	wxStringTokenizer cfg(textconfig, _T("\n"));
	int g = 0;
	while (cfg.HasMoreTokens())
	{
		wxString token = cfg.NextToken();
		token.Trim(false);
		token.Trim(true);
		if (token.Len() > 0){ CatchValsLabs(token); g++; }
	}
	if (g > 10){ return true; };
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
	if (opt >= 0 && opt < colorsSize)
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
		stringConfig[opt] = sopt;
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
	wxString Values = line.AfterFirst(L'=');
	//Values.Trim(false);
	//Values.Trim(true);
	wxString Labels = line.BeforeFirst(L'=');
	//Labels.Trim(false);
	Labels.Trim(true);
	stringConfig[GetCONFIGValue(Labels)] = Values;
}
void config::AddStyle(Styles *styl)
{
	assstore.push_back(styl);

}

Styles *config::GetStyle(int i, const wxString &name, Styles* _styl)
{
	if (name != _T("")){
		for (unsigned int j = 0; j < assstore.size(); j++){
			if (name == assstore[j]->Name){ if (_styl){ _styl = assstore[j]; } return assstore[j]; }
		}

	}
	return assstore[i];
}

int config::FindStyle(const wxString &name, int *multiplication)
{
	int isfound = -1;
	for (unsigned int j = 0; j < assstore.size(); j++)
	{
		if (name == assstore[j]->Name){
			isfound = j;
			if (multiplication){
				*multiplication++;
			}
			else{ break; }
		}
	}
	return isfound;
}

int config::StoreSize()
{
	return assstore.size();
}

void config::ChangeStyle(Styles *styl, int i)
{
	Styles *style = assstore[i];
	delete style;
	assstore[i] = styl;
}

void config::DelStyle(int i)
{
	Styles *styl = assstore[i];
	delete styl;
	assstore.erase(assstore.begin() + i);
}

void config::SaveOptions(bool cfg, bool style)
{
	OpenWrite ow;
	if (cfg){
		wxString textfile;
		GetRawOptions(textfile);
		wxString path;
		path << configPath << _T("\\Config.txt");
		ow.FileWrite(path, textfile);
	}

	if (style){
		wxString stylefile;
		for (int j = 0; j < StoreSize(); j++){
			stylefile << GetStyle(j)->GetRaw();
		}
		wxString path;
		path << pathfull << _T("\\Catalog\\") << actualStyleDir << _T(".sty");
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
	stringConfig[ProgramTheme] = "DarkSentro";
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
	colours[WindowBackground].Set((dark) ? "#202225" : "#BFBFBF");
	colours[WindowBackgroundInactive].Set((dark) ? "#4E5155" : "#C3C3C3");
	colours[WindowText].Set((dark) ? "#AEAFB2" : "#000000");
	colours[WindowTextInactive].Set((dark) ? "#7D7F83" : "#0E0082");
	colours[WindowBorder].Set((dark) ? "#2F3136" : "#BFBFBF");
	colours[WindowBorderInactive].Set((dark) ? "#4E5155" : "#C3C3C3");
	colours[WindowBorderBackground].Set((dark) ? "#2F3136" : "#7F7F7F");
	colours[WindowBorderBackgroundInactive].Set((dark) ? "#4E5155" : "#AEAEAE");
	colours[WindowHeaderText].Set((dark) ? "#AEAFB2" : "#000000");
	colours[WindowHeaderTextInactive].Set((dark) ? "#7D7F83" : "#242424");
	colours[WindowHoverHeaderElement].Set((dark) ? "#5D636D" : "#BFBFBF");
	colours[WindowPushedHeaderElement].Set((dark) ? "#788291" : "#EEEEEE");
	colours[WindowHoverCloseButton].Set((dark) ? "#A22525" : "#925B1F");
	colours[WindowPushedCloseButton].Set((dark) ? "#E24443" : "#FF6968");
	colours[WindowWarningElements].Set((dark) ? "#8791FD" : "#1A5325");
	colours[GridText].Set((dark) ? "#FFFFFF" : "#000000");
	colours[GridBackground].Set((dark) ? "#50565F" : "#EEEEEE");
	colours[GridDialogue].Set((dark) ? "#50565F" : "#EEEEEE");
	colours[GridComment].Set((dark) ? "#4880D0" : "#4880D0");
	colours[GridSelection] = (dark) ? wxColour(0x87, 0x91, 0xFD, 75) : wxColour(0x0E, 0x00, 0x82, 75);//#4B0E0082 #4B8791FD #4B1A237E//#4C5AD693 #4C8EF8B5 new wxColour((dark)? "#B6D5C5" : "#EBF5EF");
	colours[GridVisibleOnVideo].Set((dark) ? "#202225" : "#BFBFBF");
	colours[GridCollisions].Set((dark) ? "#F1FF00" : "#3600AA");
	colours[GridLines].Set((dark) ? "#202225" : "#BFBFBF");
	colours[GridActiveLine].Set((dark) ? "#8791FD" : "#000000");
	colours[GridHeader].Set((dark) ? "#2F3136" : "#7F7F7F");
	colours[GridHeaderText].Set((dark) ? "#8791FD" : "#000000");
	colours[GridLabelNormal].Set((dark) ? "#2F3136" : "#7F7F7F");
	colours[GridLabelModified].Set((dark) ? "#322F4E" : "#B0ADD8");
	colours[GridLabelSaved].Set((dark) ? "#202225" : "#BFBFBF");
	colours[GridLabelDoubtful].Set((dark) ? "#925B1F" : "#925B1F");
	colours[GridSpellchecker].Set((dark) ? "#940000" : "#FF6968");
	colours[GridComparisonOutline].Set((dark) ? "#2700FF" : "#FFFFFF");
	colours[GridComparisonBackgroundNotMatch].Set((dark) ? "#272B32" : "#FF000C");
	colours[GridComparisonBackgroundMatch].Set((dark) ? "#3A3E45" : "#B7AC00");
	colours[GridComparisonCommentBackgroundNotMatch].Set((dark) ? "#003176" : "#9C0000");
	colours[GridComparisonCommentBackgroundMatch].Set((dark) ? "#3662A1" : "#817900");
	colours[EditorText].Set((dark) ? "#F4F4F4" : "#000000");
	colours[EditorTagNames].Set((dark) ? "#00C3FF" : "#787600");
	colours[EditorTagValues].Set((dark) ? "#0076FF" : "#34C200");
	colours[EditorCurlyBraces].Set((dark) ? "#8791FD" : "#7E3B00");
	colours[EditorTagOperators].Set((dark) ? "#00C3FF" : "#787600");
	colours[EditorTemplateVariables].Set((dark) ? "#BDB76B" : "#BDB76B");
	colours[EditorTemplateCodeMarks].Set((dark) ? "#FB4544" : "#FB4544");
	colours[EditorTemplateFunctions].Set((dark) ? "#00C3FF" : "#00C3FF");
	colours[EditorTemplateKeywords].Set((dark) ? "#8D80D3" : "#8D80D3");
	colours[EditorTemplateStrings].Set((dark) ? "#11A243" : "#11A243");
	colours[EditorPhraseSearch].Set((dark) ? "#0B5808" : "#0B5808");
	colours[EditorBracesBackground].Set((dark) ? "#F4F4F4" : "#F4F4F4");
	colours[EditorBackground].Set((dark) ? "#50565F" : "#EEEEEE");
	colours[EditorSelection].Set((dark) ? "#646D8A" : "#BBC0FB");
	colours[EditorSelectionNoFocus].Set((dark) ? "#646D8A" : "#BBC0FB");
	colours[EditorBorder].Set((dark) ? "#36393E" : "#EEEEEE");
	colours[EditorBorderOnFocus].Set((dark) ? "#8791FD" : "#000000");
	colours[EditorSpellchecker].Set((dark) ? "#940000" : "#FF6968");
	colours[AudioBackground].Set((dark) ? "#36393E" : "#36393E");
	colours[AudioLineBoundaryStart].Set((dark) ? "#940000" : "#940000");
	colours[AudioLineBoundaryEnd].Set((dark) ? "#940000" : "#940000");
	colours[AudioLineBoundaryMark].Set("#FFFFFF");
	colours[AudioLineBoundaryInactiveLine].Set("#00D77D");
	colours[AudioPlayCursor].Set("#8791FD");
	colours[AudioSecondsBoundaries].Set(0xF4, 0xF4, 0xF4, 0x37);//#37F4F4F4//wxColour((dark)? "#9B0B8A9F" : "");
	colours[AudioKeyframes].Set("#F4F4F4");
	colours[AudioSyllableBoundaries].Set("#202225");
	colours[AudioSyllableText].Set("#8791FD");
	colours[AudioSelectionBackground].Set(0xFF, 0xFF, 0xFF, 0x37);//wxColour((dark)? "#37FFFFFF" : "");
	colours[AudioSelectionBackgroundModified].Set(0xFF, 0xFF, 0xFF, 0x37);//wxColour((dark)? "#37D60000" : "");
	colours[AudioInactiveLinesBackground].Set(0x00, 0x00, 0x00, 0x55);//wxColour((dark)? "#55373564" : "");
	colours[AudioWaveform].Set("#202225");
	colours[AudioWaveformInactive].Set("#2F3136");
	colours[AudioWaveformModified].Set("#FF6968");
	colours[AudioWaveformSelected].Set("#DCDAFF");
	colours[AudioSpectrumBackground].Set("#000000");
	colours[AudioSpectrumEcho].Set("#674FD7");
	colours[AudioSpectrumInner].Set("#F4F4F4");
	colours[TextFieldBackground].Set((dark) ? "#2F3136" : "#7F7F7F");
	colours[TextFieldBorder].Set((dark) ? "#36393E" : "#7F7F7F");
	colours[TextFieldBorderOnFocus].Set((dark) ? "#8791FD" : "#000000");
	colours[TextFieldSelection].Set((dark) ? "#646D8A" : "#BBDEFB");
	colours[TextFieldSelectionNoFocus].Set((dark) ? "#383E4F" : "#93B2CC");
	colours[ButtonBackground].Set((dark) ? "#2F3136" : "#A0A0A0");
	colours[ButtonBackgroundHover].Set((dark) ? "#53516B" : "#DEDEDE");
	colours[ButtonBackgroundPushed].Set((dark) ? "#4F535B" : "#EFEFEF");
	colours[ButtonBackgroundOnFocus].Set((dark) ? "#4F535B" : "#EFEFEF");
	colours[ButtonBorder].Set((dark) ? "#2F3136" : "#A0A0A0");
	colours[ButtonBorderHover].Set((dark) ? "#53516B" : "#DEDEDE");
	colours[ButtonBorderPushed].Set((dark) ? "#4F535B" : "#EFEFEF");
	colours[ButtonBorderOnFocus].Set((dark) ? "#4F535B" : "#EFEFEF");
	colours[ButtonBorderInactive].Set((dark) ? "#4F535B" : "#A0A0A0");
	colours[TogglebuttonBackgroundToggled].Set((dark) ? "#5C5889" : "#656565");
	colours[TogglebuttonBorderToggled].Set((dark) ? "#5C5889" : "#656565");
	colours[ScrollbarBackground].Set((dark) ? "#2F3136" : "#7F7F7F");
	colours[ScrollbarScroll].Set((dark) ? "#484B51" : "#ACACAC");
	colours[ScrollbarScrollHover].Set((dark) ? "#8982D5" : "#C5C5C5");
	colours[ScrollbarScrollPushed].Set((dark) ? "#9E96FF" : "#DDDDDD");
	colours[StaticboxBorder].Set((dark) ? "#36393E" : "#A0A0A0");
	colours[StaticListBorder].Set((dark) ? "#36393E" : "#EEEEEE");
	colours[StaticListBackground].Set((dark) ? "#36393E" : "#EEEEEE");
	colours[StaticListSelection].Set((dark) ? "#4F535B" : "#BFBFBF");
	colours[StaticListBackgroundHeadline].Set((dark) ? "#2F3136" : "#A0A0A0");
	colours[StaticListTextHeadline].Set((dark) ? "#AEAFB2" : "#000000");
	colours[StatusBarBorder].Set((dark) ? "#2F3136" : "#A0A0A0");
	colours[MenuBarBackground1].Set((dark) ? "#202225" : "#BFBFBF");
	colours[MenuBarBackground2].Set((dark) ? "#202225" : "#BFBFBF");
	colours[MENU_BAR_BACKGROUND_HOVER].Set((dark) ? "#53516B" : "#DEDEDE");
	colours[MenuBarBorderSelection].Set((dark) ? "#53516B" : "#DEDEDE");
	colours[MenuBarBackgroundSelection].Set((dark) ? "#575478" : "#BBDEFB");
	colours[MenuBackground].Set((dark) ? "#202225" : "#BFBFBF");
	colours[MenuBorderSelection].Set((dark) ? "#53516B" : "#DEDEDE");
	colours[MenuBackgroundSelection].Set((dark) ? "#53516B" : "#DEDEDE");
	colours[TabsBarBackground1].Set((dark) ? "#2F3136" : "#A0A0A0");
	colours[TabsBarBackground2].Set((dark) ? "#2F3136" : "#A0A0A0");
	colours[TabsBorderActive].Set((dark) ? "#202225" : "#BFBFBF");
	colours[TabsBorderInactive].Set((dark) ? "#202225" : "#A0A0A0");
	colours[TabsBackgroundActive].Set((dark) ? "#53516B" : "#BFBFBF");
	colours[TabsBackgroundInactive].Set((dark) ? "#2F3136" : "#A0A0A0");
	colours[TabsBackgroundInactiveHover].Set((dark) ? "#53516B" : "#BFBFBF");
	colours[TabsBackgroundSecondWindow].Set((dark) ? "#383654" : "#BFBFBF");
	colours[TabsTextActive].Set((dark) ? "#FFFFFF" : "#000000");
	colours[TabsTextInactive].Set((dark) ? "#7D7F83" : "#EEEEEE");
	colours[TabsCloseHover].Set((dark) ? "#A22525" : "#925B1F");
	colours[TabsBarArrow].Set((dark) ? "#202225" : "#BFBFBF");
	colours[TabsBarArrowBackground].Set((dark) ? "#2F3136" : "#A0A0A0");
	colours[TabsBarArrowBackgroundHover].Set((dark) ? "#76777B" : "#797979");
	colours[SliderPathBackground].Set((dark) ? "#2F3136" : "#7F7F7F");
	colours[SliderPathBorder].Set((dark) ? "#2F3136" : "#7F7F7F");
	colours[SliderBorder].Set((dark) ? "#5C5889" : "#000000");
	colours[SliderBorderHover].Set((dark) ? "#8982D5" : "#000000");
	colours[SliderBorderPushed].Set((dark) ? "#9E96FF" : "#000000");
	colours[SliderBackground].Set((dark) ? "#5C5889" : "#ACACAC");
	colours[SliderBackgroundHover].Set((dark) ? "#8982D5" : "#C5C5C5");
	colours[SliderBackgroundPushed].Set((dark) ? "#9E96FF" : "#DDDDDD");
	colours[WINDOW_RESIZER_DOTS].Set((dark) ? "#2F3136" : "#FFFFFF");
	colours[FIND_RESULT_FILENAME_FOREGROUND].Set((dark) ? "#BB0099" : "#5D67CF");
	colours[FIND_RESULT_FILENAME_BACKGROUND].Set((dark) ? "#440033" : "#000000");
	colours[FIND_RESULT_FOUND_PHRASE_FOREGROUND].Set((dark) ? "#FFFFFF" : "#FFFFFF");
	colours[FIND_RESULT_FOUND_PHRASE_BACKGROUND].Set((dark) ? "#8791FD" : "#5D67CF");
	colours[StylePreviewColor1].Set("#434343");
	colours[StylePreviewColor2].Set("#626262");

}

int config::LoadOptions()
{
	wxStandardPathsBase &paths = wxStandardPaths::Get();
	pathfull = paths.GetExecutablePath().BeforeLast(L'\\');
	configPath = pathfull + L"\\Config";
	wxString path;
	path << pathfull << _T("\\Config.txt");
	OpenWrite ow;
	wxString txt;
	int isgood = 0;
	bool diffVersions = false;
	if (!ow.FileOpen(path, &txt, false)){
		LoadDefaultConfig();
		isgood = 2;
	}
	else{
		wxString ver = txt.BeforeFirst(L']').Mid(1);
		if (ver != progname){ LoadDefaultConfig(); diffVersions = true; }
		isgood = SetRawOptions(txt.AfterFirst(L'\n'));
	}

	actualStyleDir = _T("Default");
	path = _T("");
	path << pathfull << _T("\\Catalog\\");
	wxDir kat(path);
	if (!kat.IsOpened()){
		ow.FileWrite(path << actualStyleDir << _T(".sty"), _T("Style: Default,Garamond,30,&H00FFFFFF,&H000000FF,&H00FF0000,&H00000000,0,0,0,0,100,100,0,0,0,2,2,2,10,10,10,1"));
		AddStyle(new Styles()); dirs.Add(actualStyleDir);
	}
	else{
		wxArrayString tmp; kat.GetAllFiles(path, &tmp, _T(""), wxDIR_FILES);
		for (size_t i = 0; i < tmp.GetCount(); i++){
			wxString fullpath = tmp[i].AfterLast(L'\\');
			if (fullpath.EndsWith(_T(".sty"))){ dirs.Add(fullpath.BeforeLast(L'.')); }
		}
	}
	LoadStyles(actualStyleDir);
	LoadColors();
	return isgood;
}

void config::LoadColors(const wxString &_themeName){
	wxString themeName;
	if (_themeName.IsEmpty()){
		themeName = Options.GetString(ProgramTheme);
	}
	else{
		themeName = _themeName;
		Options.SetString(ProgramTheme, _themeName);
	}
	bool failed = false;
	if (themeName != "DarkSentro" && themeName != "LightSentro"){
		wxString path = pathfull + L"\\Themes\\" + themeName + L".txt";
		OpenWrite ow;
		wxString txtColors;
		if (ow.FileOpen(path, &txtColors, false)){
			wxStringTokenizer cfg(txtColors, _T("\n"));
			int g = 0;
			while (cfg.HasMoreTokens())
			{
				wxString token = cfg.NextToken();
				token.Trim(false);
				token.Trim(true);
				if (token.Len() > 6){ SetHexColor(token); g++; }
			}
			if (g > 10){
				if (colors[0].IsOk() || g < StylePreviewColor2){
					LoadMissingColours(path);
					KaiMessageBox(wxString::Format(_("W motywie \"%s\" brakowało część kolorów, zostały doczytane z domyślnego."), themeName));
				}
				return;
			}
		}
		failed = true;
	}
	LoadDefaultColors(themeName != "LightSentro");
	if (failed){
		Options.SetString(ProgramTheme, "DarkSentro");
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
	actualStyleDir = katalog;
	wxString path;
	path << pathfull << _T("\\Catalog\\") << katalog << _T(".sty");
	OpenWrite ow;
	for (std::vector<Styles*>::iterator it = assstore.begin(); it != assstore.end(); it++){
		delete (*it);
	}
	assstore.clear();
	wxString stylee;
	if (ow.FileOpen(path, &stylee, false)){
		wxStringTokenizer cfg(stylee, _T("\n"));
		while (cfg.HasMoreTokens()){
			wxString token = cfg.NextToken();
			if (token.StartsWith(_T("Style: "))){
				AddStyle(new Styles(token));
			}
		}
	}
}

void config::clearstyles()
{
	for (std::vector<Styles*>::iterator it = assstore.begin(); it != assstore.end(); it++){
		delete (*it);
	}
	assstore.clear();
}

void config::SetCoords(CONFIG opt, int coordx, int coordy)
{
	wxString iopt1 = _T("");
	stringConfig[opt] = iopt1 << coordx << "," << coordy;
}

void config::GetCoords(CONFIG opt, int *coordx, int *coordy)
{
	wxString sopt = stringConfig[opt];
	*coordx = wxAtoi(sopt.BeforeFirst(L','));
	*coordy = wxAtoi(sopt.AfterFirst(L','));
}

void config::SetTable(CONFIG opt, wxArrayString &asopt, wxString split)
{
	wxString sresult;
	wxString ES;
	for (size_t i = 0; i < asopt.size(); i++)
	{
		wxString endchar = (i == asopt.size() - 1) ? ES : split;
		sresult << asopt[i] << endchar;
	}
	stringConfig[opt] = sresult;
}

void config::SetIntTable(CONFIG opt, wxArrayInt &asopt, wxString split)
{
	wxString sresult;
	wxString ES;
	for (size_t i = 0; i < asopt.size(); i++)
	{
		wxString endchar = (i == asopt.size() - 1) ? ES : split;
		sresult << asopt[i] << endchar;
	}
	stringConfig[opt] = sresult;
}

void config::GetTable(CONFIG opt, wxArrayString &tbl, wxString split, int mode)
{
	wxString strtbl = stringConfig[opt];
	if (strtbl != ""){
		wxStringTokenizer cfgtable(strtbl, split, (wxStringTokenizerMode)mode);
		while (cfgtable.HasMoreTokens()){
			tbl.Add(cfgtable.NextToken());
		}
	}
}

void config::GetIntTable(CONFIG opt, wxArrayInt &tbl, wxString split, int mode)
{
	wxString strtbl = stringConfig[opt];
	if (strtbl != ""){
		wxStringTokenizer cfgtable(strtbl, split, (wxStringTokenizerMode)mode);
		while (cfgtable.HasMoreTokens()){
			tbl.Add(wxAtoi(cfgtable.NextToken()));
		}
	}
}

bool sortfunc(Styles *styl1, Styles *styl2){
	wxString str1 = styl1->Name;
	wxString str2 = styl2->Name;
	return (str1.CmpNoCase(str2) < 0);
}

void config::Sortstyles()
{
	std::sort(assstore.begin(), assstore.end(), sortfunc);
}

void config::LoadDefaultAudioConfig()
{
	stringConfig[AudioAutoCommit] = "true";
	stringConfig[AudioAutoFocus] = "true";
	stringConfig[AudioAutoScroll] = "true";
	stringConfig[AudioBoxHeight] = "169";
	stringConfig[AUDIO_CACHE_FILES_LIMIT] = "10";
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
	if (!ow.FileOpen(configPath + _T("\\AudioConfig.txt"), &txt, false)){
		LoadDefaultAudioConfig();
		return true;
	}
	else{
		wxString ver = txt.BeforeFirst(L']').Mid(1);
		if (ver != progname){ LoadDefaultAudioConfig(); }
	}
	return (AudioOpts = SetRawOptions(txt.AfterFirst(L'\n')));
}

void config::SaveAudioOpts()
{
	OpenWrite ow;
	wxString audioOpts;
	GetRawOptions(audioOpts, true);
	ow.FileWrite(configPath + _T("\\AudioConfig.txt"), audioOpts);
}

void config::SetHexColor(const wxString &nameAndColor)
{
	wxString kol = nameAndColor.AfterFirst(L'=');
	kol.Trim(false);
	kol.Trim(true);
	wxString name = nameAndColor.BeforeFirst(L'=');
	name.Trim(false);
	name.Trim(true);
	long a = 0xFF, r, g, b;
	int diff = 0;
	if (kol.Len() >= 9){
		kol.SubString(1, 2).ToLong(&a, 16);
		diff = 2;
	}
	kol.SubString(diff + 1, diff + 2).ToLong(&r, 16);
	kol.SubString(diff + 3, diff + 4).ToLong(&g, 16);
	kol.SubString(diff + 5, diff + 6).ToLong(&b, 16);
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
	if (path.IsEmpty()){
		finalpath = pathfull + "\\Themes\\" + GetString(ProgramTheme) + ".txt";
	}
	OpenWrite ow(finalpath, true);
	for (size_t i = 1; i < colorsSize; i++){
		ow.PartFileWrite(wxString(::GetString((COLOR)i)) + "=" + GetStringColor(i) + "\r\n");
	}
}

wxString getfloat(float num, const wxString &format, bool Truncate)
{
	wxString strnum = wxString::Format(_T("%" + format), num);
	//if(strnum.find(L'.')!= -1){return strnum.Trim(false);}
	if (!Truncate || format.EndsWith(".0f")){ return strnum.Trim(false); }
	int rmv = 0;
	bool trim = false;
	for (int i = strnum.Len() - 1; i > 0; i--)
	{
		if (strnum[i] == L'0'){ rmv++; }//&&!trim
		//else if(strnum[i]==L'9'){rmv++;trim=true;}
		else if (strnum[i] == L'.'){ rmv++; break; }//}if(!trim){
		else{/*if(trim){int tmpc=static_cast < int >(strnum.GetChar(i));tmpc++;strnum[i]=(wxUniChar)tmpc;}*/break; }
	}
	if (rmv){ strnum.RemoveLast(rmv); }
	return strnum.Trim(false);
}


bool LoadDataFromResource(char*& t_data, DWORD& t_dataSize, const wxString& t_name)
{
	bool     r_result = false;
	HGLOBAL  a_resHandle = 0;
	HRSRC    a_resource;

	a_resource = FindResource(0, t_name.wchar_str(), RT_RCDATA);

	if (0 != a_resource)
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

	char*       a_data = 0;
	DWORD       a_dataSize = 0;

	if (LoadDataFromResource(a_data, a_dataSize, t_name))
	{
		r_bitmapPtr = GetBitmapFromMemory(a_data, a_dataSize);
	}

	return r_bitmapPtr;
}

wxBitmap *CreateBitmapPointerFromPngResource(const wxString& t_name)
{
	wxBitmap  *r_bitmapPtr = NULL;

	char*       a_data = 0;
	DWORD       a_dataSize = 0;

	if (LoadDataFromResource(a_data, a_dataSize, t_name))
	{
		wxMemoryInputStream a_is(a_data, a_dataSize);
		r_bitmapPtr = new wxBitmap(wxImage(a_is, wxBITMAP_TYPE_PNG, -1), -1);
	}

	return r_bitmapPtr;
}

wxImage CreateImageFromPngResource(const wxString& t_name)
{
	wxImage   image;

	char*       a_data = 0;
	DWORD       a_dataSize = 0;

	if (LoadDataFromResource(a_data, a_dataSize, t_name))
	{
		wxMemoryInputStream a_is(a_data, a_dataSize);
		image = wxImage(a_is, wxBITMAP_TYPE_PNG, -1);
	}

	return image;
}

void MoveToMousePosition(wxWindow *win)
{
	wxPoint mst = wxGetMousePosition();
	wxSize siz = win->GetSize();
	siz.x;
	wxRect rc = GetMonitorRect(0, NULL, mst, true);
	mst.x -= (siz.x / 2);
	mst.x = MID(rc.x, mst.x, (rc.width + rc.x) - siz.x);
	mst.y += 15;
	if (mst.y + siz.y > rc.height + rc.y){
		mst.y = mst.y - siz.y - 30;
		if (mst.y < rc.y){
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
	else if ((div10mod >= 2 && div10mod <= 4) && (div100mod < 10 || div100mod>20)){
		result = plural2to4;
	}
	else{
		result = pluralRest;
	}
	wxString finalResult;
	return finalResult << num << " " << result;
}

BOOL CALLBACK MonitorEnumProc1(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
	std::pair<std::vector<RECT>, bool> *pair = (std::pair<std::vector<RECT>, bool> *)dwData;
	MONITORINFO monitorinfo;
	ZeroMemory(&monitorinfo, sizeof(monitorinfo));
	monitorinfo.cbSize = sizeof(monitorinfo);

	if (!GetMonitorInfo(hMonitor, &monitorinfo)){
		KaiLog(_("Nie można pobrać informacji o monitorze"));
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

wxRect GetMonitorRect(int wmonitor, std::vector<tagRECT> *MonitorRects, const wxPoint &position, bool workArea){
	std::vector<RECT> MonRects;
	std::pair<std::vector<RECT>, bool> *pair = new std::pair<std::vector<RECT>, bool>(MonRects, workArea);
	EnumDisplayMonitors(NULL, NULL, MonitorEnumProc1, (LPARAM)pair);
	MonRects = pair->first;
	delete pair;
	if (MonRects.size() == 0){
		bool ktos_ukradl_ci_monitor = false;
		assert(ktos_ukradl_ci_monitor);
	}
	wxRect rt(MonRects[0].left, MonRects[0].top, abs(MonRects[0].right - MonRects[0].left), abs(MonRects[0].bottom - MonRects[0].top));
	if (wmonitor == -1 || MonRects.size() == 1){ return rt; }
	else if (wmonitor == 0){
		int x = position.x;
		int y = position.y;
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

bool IsNumber(const wxString &test) {
	bool isnumber = true;
	wxString testchars = "0123456789";
	for (size_t i = 0; i < test.Len(); i++){
		wxUniChar ch = test.GetChar(i);
		if (testchars.Find(ch) == -1){ isnumber = false; break; }
	}
	return isnumber;
}

#ifdef _M_IX86

typedef struct tagTHREADNAME_INFO
{
	DWORD dwType;
	LPCSTR szName;
	DWORD dwThreadID;
	DWORD dwFlags;
} THREADNAME_INFO;

void SetThreadName(DWORD dwThreadID, LPCSTR szThreadName)
{
	THREADNAME_INFO info;
	info.dwType = 0x1000;
	info.szName = szThreadName;
	info.dwThreadID = dwThreadID;
	info.dwFlags = 0;

	__try
	{
		::RaiseException(0x406D1388, 0, sizeof(info) / sizeof(DWORD), (DWORD *)& info);
	}
	__except (EXCEPTION_CONTINUE_EXECUTION)
	{
	}
}
#else
typedef struct tagTHREADNAME_INFO
{
	size_t dwType;
	LPCSTR szName;
	size_t dwThreadID;
	size_t dwFlags;
} THREADNAME_INFO;

void SetThreadName(size_t dwThreadID, LPCSTR szThreadName)
{
	THREADNAME_INFO info;
	info.dwType = 0x1000;
	info.szName = szThreadName;
	info.dwThreadID = dwThreadID;
	info.dwFlags = 0;

	__try
	{
		::RaiseException(0x406D1388, 0, sizeof(info) / sizeof(size_t), (size_t *)& info);
	}
	__except (EXCEPTION_CONTINUE_EXECUTION)
	{
	}
}
#endif

DEFINE_ENUM(CONFIG, CFG);

DEFINE_ENUM(COLOR, CLR);

config Options;




