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
	gitVersion = L"  " + wxString(ADD_QUOTES(GIT_BRANCH)) + L" " + wxString(ADD_QUOTES(GIT_CUR_COMMIT)).Left(7);
#endif
	progname = L"Kainote v" + wxString(VersionKainote) + gitVersion;
#if _DEBUG
	progname += L" DEBUG";
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

	FontsClear();
}

void config::FontsClear()
{
	for (std::map<int, wxFont*>::iterator it = programFonts.begin(); it != programFonts.end(); it++){
		delete it->second;
	}
	programFonts.clear();
}

wxString config::GetReleaseDate()
{
	return wxString(__DATE__) + L"  " + wxString(__TIME__);
}


csri_rend * config::GetVSFilter()
{
	if (!vsfilter){
		vsfilter = csri_renderer_default();
		csri_info *info = csri_renderer_info(vsfilter);
		wxString name = GetString(VSFILTER_INSTANCE);
		if (!name.empty()){
			while (info->name != name){
				vsfilter = csri_renderer_next(vsfilter);
				if (!vsfilter)
					break;
				info = csri_renderer_info(vsfilter);
			}
			if (!vsfilter)
				vsfilter = csri_renderer_default();
		}
	}
	return vsfilter;
}

void config::ChangeVsfilter()
{
	vsfilter = csri_renderer_default();
	csri_info *info = csri_renderer_info(vsfilter);
	wxString name = GetString(VSFILTER_INSTANCE);
	if (!name.empty()){
		while (info->name != name){
			vsfilter = csri_renderer_next(vsfilter);
			if (!vsfilter)
				break;
			info = csri_renderer_info(vsfilter);
		}
		if (!vsfilter)
			vsfilter = csri_renderer_default();
	}
}

void config::GetVSFiltersList(wxArrayString &filtersList)
{
	csri_rend *filter = csri_renderer_default();
	if (!filter)
		return;
	csri_info *info = csri_renderer_info(filter);
	filtersList.Add(info->name);
	while (1){
		filter = csri_renderer_next(filter);
		if (!filter)
			break;
		info = csri_renderer_info(filter);
		filtersList.Add(info->name);
	}
	//csri_close_renderer(filter);
}

bool config::SetRawOptions(const wxString &textconfig)
{
	wxStringTokenizer cfg(textconfig, L"\n");
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
		if (ropt == L"true"){ return true; }
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
		wxString bopt1 = (bopt) ? L"true" : L"false";
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
		wxString iopt1 = L"";
		stringConfig[opt] = iopt1 << iopt;
	}
}

void config::SetFloat(CONFIG opt, float fopt)
{
	if (opt >= 0 && opt < configSize){
		wxString fopt1 = L"";
		fopt1 << fopt;
		fopt1.Replace(L",", L".");
		stringConfig[opt] = fopt1;
	}
}

void config::GetRawOptions(wxString &options, bool Audio/*=false*/)
{
	options = L"[" + progname + L"]\r\n";
	for (size_t i = 1; i < configSize; i++) {
		if ((!Audio && i <= AudioWheelDefaultToZoom) || (Audio && i > AudioWheelDefaultToZoom) ||
			stringConfig[i].empty()) {
			continue;
		}
		options << ::GetString((CONFIG)i) << L"=" << stringConfig[i] << L"\r\n";
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
	if (name != L""){
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
		path << configPath << L"\\Config.txt";
		ow.FileWrite(path, textfile);
	}

	if (style){
		wxString stylefile;
		for (int j = 0; j < StoreSize(); j++){
			stylefile << GetStyle(j)->GetRaw();
		}
		wxString path;
		path << pathfull << L"\\Catalog\\" << actualStyleDir << L".sty";
		ow.FileWrite(path, stylefile);
	}
}

void config::LoadDefaultConfig()
{
	stringConfig[MoveTimesTime] = L"2000";
	stringConfig[MoveTimesWhichLines] = L"0";
	stringConfig[ConvertResolutionWidth] = L"1280";
	stringConfig[ConvertResolutionHeight] = L"720";
	stringConfig[ConvertFPS] = L"23.976";
	stringConfig[ConvertStyle] = L"Default";
	stringConfig[ConvertStyleCatalog] = L"Default";
	stringConfig[DictionaryLanguage] = L"pl";
	stringConfig[SpellcheckerOn] = L"true";
	stringConfig[StyleEditFilterText] = L"ĄĆĘŁŃÓŚŹŻąćęłńóśźż";
	stringConfig[FFMS2VideoSeeking] = L"2";
	stringConfig[MoveTimesByTime] = L"false";
	stringConfig[GridFontName] = L"Tahoma";
	stringConfig[GridFontSize] = L"10";
	stringConfig[PROGRAM_FONT] = L"Tahoma";
	stringConfig[PROGRAM_FONT_SIZE] = L"10";
	stringConfig[GridSaveAfterCharacterCount] = L"1";
	stringConfig[GridTagsSwapChar] = L"☀";
	stringConfig[MoveTimesForward] = L"true";
	stringConfig[ConvertNewEndTimes] = L"false";
	stringConfig[InsertStartOffset] = L"0";
	stringConfig[InsertEndOffset] = L"0";
	stringConfig[PlayAfterSelection] = L"0";
	stringConfig[PreviewText] = L"Podgląd";
	stringConfig[ProgramTheme] = L"DarkSentro";
	stringConfig[EditorOn] = L"true";
	stringConfig[ConvertShowSettings] = L"false";
	stringConfig[MoveTimesOn] = L"true";
	stringConfig[MoveTimesWhichTimes] = L"0";
	stringConfig[MoveTimesStyles] = L"";
	stringConfig[ConvertTimePerLetter] = L"110";
	stringConfig[VideoIndex] = L"true";
	stringConfig[VideoProgressBar] = L"true";
	stringConfig[VideoWindowSize] = L"500,350";
	stringConfig[WindowSize] = L"800,600";
	stringConfig[AutomationTraceLevel] = L"3";
	stringConfig[AutoSaveMaxFiles] = L"3";
	stringConfig[GridChangeActiveOnSelection] = L"true";
}

//remember, create table[colorsSize] without this size it will crash
void config::LoadDefaultColors(bool dark, wxColour *table)
{
	wxColour *colours = (table) ? table : colors;
	colours[WindowBackground].Set((dark) ? L"#202225" : L"#BFBFBF");
	colours[WindowBackgroundInactive].Set((dark) ? L"#4E5155" : L"#C3C3C3");
	colours[WindowText].Set((dark) ? L"#AEAFB2" : L"#000000");
	colours[WindowTextInactive].Set((dark) ? L"#7D7F83" : L"#0E0082");
	colours[WindowBorder].Set((dark) ? L"#2F3136" : L"#BFBFBF");
	colours[WindowBorderInactive].Set((dark) ? L"#4E5155" : L"#C3C3C3");
	colours[WindowBorderBackground].Set((dark) ? L"#2F3136" : L"#7F7F7F");
	colours[WindowBorderBackgroundInactive].Set((dark) ? L"#4E5155" : L"#AEAEAE");
	colours[WindowHeaderText].Set((dark) ? L"#AEAFB2" : L"#000000");
	colours[WindowHeaderTextInactive].Set((dark) ? L"#7D7F83" : L"#242424");
	colours[WindowHoverHeaderElement].Set((dark) ? L"#5D636D" : L"#BFBFBF");
	colours[WindowPushedHeaderElement].Set((dark) ? L"#788291" : L"#EEEEEE");
	colours[WindowHoverCloseButton].Set((dark) ? L"#A22525" : L"#925B1F");
	colours[WindowPushedCloseButton].Set((dark) ? L"#E24443" : L"#FF6968");
	colours[WindowWarningElements].Set((dark) ? L"#8791FD" : L"#1A5325");
	colours[GridText].Set((dark) ? L"#FFFFFF" : L"#000000");
	colours[GridBackground].Set((dark) ? L"#50565F" : L"#EEEEEE");
	colours[GridDialogue].Set((dark) ? L"#50565F" : L"#EEEEEE");
	colours[GridComment].Set((dark) ? L"#4880D0" : L"#4880D0");
	colours[GridSelection] = (dark) ? wxColour(0x87, 0x91, 0xFD, 75) : wxColour(0x0E, 0x00, 0x82, 75);//#4B0E0082 #4B8791FD #4B1A237E//#4C5AD693 #4C8EF8B5 new wxColour((dark)? L"#B6D5C5" : L"#EBF5EF");
	colours[GridVisibleOnVideo].Set((dark) ? L"#202225" : L"#BFBFBF");
	colours[GridCollisions].Set((dark) ? L"#F1FF00" : L"#3600AA");
	colours[GridLines].Set((dark) ? L"#202225" : L"#BFBFBF");
	colours[GridActiveLine].Set((dark) ? L"#8791FD" : L"#000000");
	colours[GridHeader].Set((dark) ? L"#2F3136" : L"#7F7F7F");
	colours[GridHeaderText].Set((dark) ? L"#8791FD" : L"#000000");
	colours[GridLabelNormal].Set((dark) ? L"#2F3136" : L"#7F7F7F");
	colours[GridLabelModified].Set((dark) ? L"#322F4E" : L"#B0ADD8");
	colours[GridLabelSaved].Set((dark) ? L"#202225" : L"#BFBFBF");
	colours[GridLabelDoubtful].Set((dark) ? L"#925B1F" : L"#925B1F");
	colours[GridSpellchecker].Set((dark) ? L"#940000" : L"#FF6968");
	colours[GridComparisonOutline].Set((dark) ? L"#2700FF" : L"#FFFFFF");
	colours[GridComparisonBackgroundNotMatch].Set((dark) ? L"#272B32" : L"#FF000C");
	colours[GridComparisonBackgroundMatch].Set((dark) ? L"#3A3E45" : L"#B7AC00");
	colours[GridComparisonCommentBackgroundNotMatch].Set((dark) ? L"#003176" : L"#9C0000");
	colours[GridComparisonCommentBackgroundMatch].Set((dark) ? L"#3662A1" : L"#817900");
	colours[EditorText].Set((dark) ? L"#F4F4F4" : L"#000000");
	colours[EditorTagNames].Set((dark) ? L"#00C3FF" : L"#787600");
	colours[EditorTagValues].Set((dark) ? L"#0076FF" : L"#34C200");
	colours[EditorCurlyBraces].Set((dark) ? L"#8791FD" : L"#7E3B00");
	colours[EditorTagOperators].Set((dark) ? L"#00C3FF" : L"#787600");
	colours[EditorTemplateVariables].Set((dark) ? L"#BDB76B" : L"#BDB76B");
	colours[EditorTemplateCodeMarks].Set((dark) ? L"#FB4544" : L"#FB4544");
	colours[EditorTemplateFunctions].Set((dark) ? L"#00C3FF" : L"#00C3FF");
	colours[EditorTemplateKeywords].Set((dark) ? L"#8D80D3" : L"#8D80D3");
	colours[EditorTemplateStrings].Set((dark) ? L"#11A243" : L"#11A243");
	colours[EditorPhraseSearch].Set((dark) ? L"#0B5808" : L"#0B5808");
	colours[EditorBracesBackground].Set((dark) ? L"#F4F4F4" : L"#F4F4F4");
	colours[EditorBackground].Set((dark) ? L"#50565F" : L"#EEEEEE");
	colours[EditorSelection].Set((dark) ? L"#646D8A" : L"#BBC0FB");
	colours[EditorSelectionNoFocus].Set((dark) ? L"#646D8A" : L"#BBC0FB");
	colours[EditorBorder].Set((dark) ? L"#36393E" : L"#EEEEEE");
	colours[EditorBorderOnFocus].Set((dark) ? L"#8791FD" : L"#000000");
	colours[EditorSpellchecker].Set((dark) ? L"#940000" : L"#FF6968");
	colours[AudioBackground].Set((dark) ? L"#36393E" : L"#36393E");
	colours[AudioLineBoundaryStart].Set((dark) ? L"#940000" : L"#940000");
	colours[AudioLineBoundaryEnd].Set((dark) ? L"#940000" : L"#940000");
	colours[AudioLineBoundaryMark].Set(L"#FFFFFF");
	colours[AudioLineBoundaryInactiveLine].Set(L"#00D77D");
	colours[AudioPlayCursor].Set(L"#8791FD");
	colours[AudioSecondsBoundaries].Set(0xF4, 0xF4, 0xF4, 0x37);//#37F4F4F4//wxColour((dark)? L"#9B0B8A9F" : L"");
	colours[AudioKeyframes].Set(L"#F4F4F4");
	colours[AudioSyllableBoundaries].Set(L"#202225");
	colours[AudioSyllableText].Set(L"#8791FD");
	colours[AudioSelectionBackground].Set(0xFF, 0xFF, 0xFF, 0x37);//wxColour((dark)? L"#37FFFFFF" : L"");
	colours[AudioSelectionBackgroundModified].Set(0xFF, 0xFF, 0xFF, 0x37);//wxColour((dark)? L"#37D60000" : L"");
	colours[AudioInactiveLinesBackground].Set(0x00, 0x00, 0x00, 0x55);//wxColour((dark)? L"#55373564" : L"");
	colours[AudioWaveform].Set(L"#202225");
	colours[AudioWaveformInactive].Set(L"#2F3136");
	colours[AudioWaveformModified].Set(L"#FF6968");
	colours[AudioWaveformSelected].Set(L"#DCDAFF");
	colours[AudioSpectrumBackground].Set(L"#000000");
	colours[AudioSpectrumEcho].Set(L"#674FD7");
	colours[AudioSpectrumInner].Set(L"#F4F4F4");
	colours[TextFieldBackground].Set((dark) ? L"#2F3136" : L"#7F7F7F");
	colours[TextFieldBorder].Set((dark) ? L"#36393E" : L"#7F7F7F");
	colours[TextFieldBorderOnFocus].Set((dark) ? L"#8791FD" : L"#000000");
	colours[TextFieldSelection].Set((dark) ? L"#646D8A" : L"#BBDEFB");
	colours[TextFieldSelectionNoFocus].Set((dark) ? L"#383E4F" : L"#93B2CC");
	colours[ButtonBackground].Set((dark) ? L"#2F3136" : L"#A0A0A0");
	colours[ButtonBackgroundHover].Set((dark) ? L"#53516B" : L"#DEDEDE");
	colours[ButtonBackgroundPushed].Set((dark) ? L"#4F535B" : L"#EFEFEF");
	colours[ButtonBackgroundOnFocus].Set((dark) ? L"#4F535B" : L"#EFEFEF");
	colours[ButtonBorder].Set((dark) ? L"#2F3136" : L"#A0A0A0");
	colours[ButtonBorderHover].Set((dark) ? L"#53516B" : L"#DEDEDE");
	colours[ButtonBorderPushed].Set((dark) ? L"#4F535B" : L"#EFEFEF");
	colours[ButtonBorderOnFocus].Set((dark) ? L"#4F535B" : L"#EFEFEF");
	colours[ButtonBorderInactive].Set((dark) ? L"#4F535B" : L"#A0A0A0");
	colours[TogglebuttonBackgroundToggled].Set((dark) ? L"#5C5889" : L"#656565");
	colours[TogglebuttonBorderToggled].Set((dark) ? L"#5C5889" : L"#656565");
	colours[ScrollbarBackground].Set((dark) ? L"#2F3136" : L"#7F7F7F");
	colours[ScrollbarScroll].Set((dark) ? L"#484B51" : L"#ACACAC");
	colours[ScrollbarScrollHover].Set((dark) ? L"#8982D5" : L"#C5C5C5");
	colours[ScrollbarScrollPushed].Set((dark) ? L"#9E96FF" : L"#DDDDDD");
	colours[StaticboxBorder].Set((dark) ? L"#36393E" : L"#A0A0A0");
	colours[StaticListBorder].Set((dark) ? L"#36393E" : L"#EEEEEE");
	colours[StaticListBackground].Set((dark) ? L"#36393E" : L"#EEEEEE");
	colours[StaticListSelection].Set((dark) ? L"#4F535B" : L"#BFBFBF");
	colours[StaticListBackgroundHeadline].Set((dark) ? L"#2F3136" : L"#A0A0A0");
	colours[StaticListTextHeadline].Set((dark) ? L"#AEAFB2" : L"#000000");
	colours[StatusBarBorder].Set((dark) ? L"#2F3136" : L"#A0A0A0");
	colours[MenuBarBackground1].Set((dark) ? L"#202225" : L"#BFBFBF");
	colours[MenuBarBackground2].Set((dark) ? L"#202225" : L"#BFBFBF");
	colours[MENU_BAR_BACKGROUND_HOVER].Set((dark) ? L"#53516B" : L"#DEDEDE");
	colours[MenuBarBorderSelection].Set((dark) ? L"#53516B" : L"#DEDEDE");
	colours[MenuBarBackgroundSelection].Set((dark) ? L"#575478" : L"#BBDEFB");
	colours[MenuBackground].Set((dark) ? L"#202225" : L"#BFBFBF");
	colours[MenuBorderSelection].Set((dark) ? L"#53516B" : L"#DEDEDE");
	colours[MenuBackgroundSelection].Set((dark) ? L"#53516B" : L"#DEDEDE");
	colours[TabsBarBackground1].Set((dark) ? L"#2F3136" : L"#A0A0A0");
	colours[TabsBarBackground2].Set((dark) ? L"#2F3136" : L"#A0A0A0");
	colours[TabsBorderActive].Set((dark) ? L"#202225" : L"#BFBFBF");
	colours[TabsBorderInactive].Set((dark) ? L"#202225" : L"#A0A0A0");
	colours[TabsBackgroundActive].Set((dark) ? L"#53516B" : L"#BFBFBF");
	colours[TabsBackgroundInactive].Set((dark) ? L"#2F3136" : L"#A0A0A0");
	colours[TabsBackgroundInactiveHover].Set((dark) ? L"#53516B" : L"#BFBFBF");
	colours[TabsBackgroundSecondWindow].Set((dark) ? L"#383654" : L"#BFBFBF");
	colours[TabsTextActive].Set((dark) ? L"#FFFFFF" : L"#000000");
	colours[TabsTextInactive].Set((dark) ? L"#7D7F83" : L"#EEEEEE");
	colours[TabsCloseHover].Set((dark) ? L"#A22525" : L"#925B1F");
	colours[TabsBarArrow].Set((dark) ? L"#202225" : L"#BFBFBF");
	colours[TabsBarArrowBackground].Set((dark) ? L"#2F3136" : L"#A0A0A0");
	colours[TabsBarArrowBackgroundHover].Set((dark) ? L"#76777B" : L"#797979");
	colours[SliderPathBackground].Set((dark) ? L"#2F3136" : L"#7F7F7F");
	colours[SliderPathBorder].Set((dark) ? L"#2F3136" : L"#7F7F7F");
	colours[SliderBorder].Set((dark) ? L"#5C5889" : L"#000000");
	colours[SliderBorderHover].Set((dark) ? L"#8982D5" : L"#000000");
	colours[SliderBorderPushed].Set((dark) ? L"#9E96FF" : L"#000000");
	colours[SliderBackground].Set((dark) ? L"#5C5889" : L"#ACACAC");
	colours[SliderBackgroundHover].Set((dark) ? L"#8982D5" : L"#C5C5C5");
	colours[SliderBackgroundPushed].Set((dark) ? L"#9E96FF" : L"#DDDDDD");
	colours[WINDOW_RESIZER_DOTS].Set((dark) ? L"#2F3136" : L"#FFFFFF");
	colours[FIND_RESULT_FILENAME_FOREGROUND].Set((dark) ? L"#BB0099" : L"#5D67CF");
	colours[FIND_RESULT_FILENAME_BACKGROUND].Set((dark) ? L"#440033" : L"#000000");
	colours[FIND_RESULT_FOUND_PHRASE_FOREGROUND].Set((dark) ? L"#FFFFFF" : L"#FFFFFF");
	colours[FIND_RESULT_FOUND_PHRASE_BACKGROUND].Set((dark) ? L"#8791FD" : L"#5D67CF");
	colours[StylePreviewColor1].Set(L"#434343");
	colours[StylePreviewColor2].Set(L"#626262");

}

int config::LoadOptions()
{
	wxStandardPathsBase &paths = wxStandardPaths::Get();
	pathfull = paths.GetExecutablePath().BeforeLast(L'\\');
	configPath = pathfull + L"\\Config";
	wxString path;
	path << configPath << L"\\Config.txt";
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

	actualStyleDir = L"Default";
	path = L"";
	path << pathfull << L"\\Catalog\\";
	wxDir kat(path);
	if (!kat.IsOpened()){
		ow.FileWrite(path << actualStyleDir << L".sty",
			L"Style: Default,Garamond,30,&H00FFFFFF,&H000000FF,&H00FF0000,&H00000000,0,0,0,0,100,100,0,0,0,2,2,2,10,10,10,1");
		AddStyle(new Styles()); dirs.Add(actualStyleDir);
	}
	else{
		wxArrayString tmp; kat.GetAllFiles(path, &tmp, L"", wxDIR_FILES);
		for (size_t i = 0; i < tmp.GetCount(); i++){
			wxString fullpath = tmp[i].AfterLast(L'\\');
			if (fullpath.EndsWith(L".sty")){ dirs.Add(fullpath.BeforeLast(L'.')); }
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
	if (themeName != L"DarkSentro" && themeName != L"LightSentro"){
		wxString path = pathfull + L"\\Themes\\" + themeName + L".txt";
		OpenWrite ow;
		wxString txtColors;
		if (ow.FileOpen(path, &txtColors, false)){
			wxStringTokenizer cfg(txtColors, L"\n");
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
	LoadDefaultColors(themeName != L"LightSentro");
	if (failed){
		Options.SetString(ProgramTheme, L"DarkSentro");
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
	path << pathfull << L"\\Catalog\\" << katalog << L".sty";
	OpenWrite ow;
	for (std::vector<Styles*>::iterator it = assstore.begin(); it != assstore.end(); it++){
		delete (*it);
	}
	assstore.clear();
	wxString stylee;
	if (ow.FileOpen(path, &stylee, false)){
		wxStringTokenizer cfg(stylee, L"\n");
		while (cfg.HasMoreTokens()){
			wxString token = cfg.NextToken();
			if (token.StartsWith(L"Style: ")){
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
	wxString iopt1 = L"";
	stringConfig[opt] = iopt1 << coordx << L"," << coordy;
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
	if (strtbl != L""){
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
	stringConfig[AudioAutoCommit] = L"true";
	stringConfig[AudioAutoFocus] = L"true";
	stringConfig[AudioAutoScroll] = L"true";
	stringConfig[AudioBoxHeight] = L"169";
	stringConfig[AUDIO_CACHE_FILES_LIMIT] = L"10";
	stringConfig[AudioDelay] = L"0";
	stringConfig[AudioDrawTimeCursor] = L"true";
	stringConfig[AudioDrawKeyframes] = L"true";
	stringConfig[AudioDrawSecondaryLines] = L"true";
	stringConfig[AudioDrawSelectionBackground] = L"true";
	stringConfig[AudioDrawVideoPosition] = L"true";
	stringConfig[AudioGrabTimesOnSelect] = L"true";
	stringConfig[AudioHorizontalZoom] = L"50";
	stringConfig[AudioInactiveLinesDisplayMode] = L"1";
	stringConfig[AudioKaraoke] = L"false";
	stringConfig[AudioKaraokeSplitMode] = L"true";
	stringConfig[AudioLeadIn] = L"200";
	stringConfig[AudioLeadOut] = L"300";
	stringConfig[AudioLineBoundariesThickness] = L"2";
	stringConfig[AudioLink] = L"false";
	stringConfig[AudioLockScrollOnCursor] = L"false";
	stringConfig[AudioMarkPlayTime] = L"1000";
	stringConfig[AudioNextLineOnCommit] = L"true";
	stringConfig[AudioRAMCache] = L"false";
	stringConfig[AudioSnapToKeyframes] = L"false";
	stringConfig[AudioSnapToOtherLines] = L"false";
	stringConfig[AudioSpectrumOn] = L"false";
	stringConfig[AudioStartDragSensitivity] = L"6";
	stringConfig[AudioVerticalZoom] = L"50";
	stringConfig[AudioVolume] = L"50";
	stringConfig[AudioWheelDefaultToZoom] = L"false";

}

bool config::LoadAudioOpts()
{
	OpenWrite ow;
	wxString txt;
	if (!ow.FileOpen(configPath + L"\\AudioConfig.txt", &txt, false)){
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
	ow.FileWrite(configPath + L"\\AudioConfig.txt", audioOpts);
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
		return wxString::Format(L"#%02X%02X%02X%02X", col.Alpha(), col.Red(), col.Green(), col.Blue());
	return wxString::Format(L"#%02X%02X%02X", col.Red(), col.Green(), col.Blue());
}

void config::SaveColors(const wxString &path){
	wxString finalpath = path;
	if (path.IsEmpty()){
		finalpath = pathfull + L"\\Themes\\" + GetString(ProgramTheme) + L".txt";
	}
	OpenWrite ow(finalpath, true);
	for (size_t i = 1; i < colorsSize; i++){
		ow.PartFileWrite(wxString(::GetString((COLOR)i)) + L"=" + GetStringColor(i) + L"\r\n");
	}
}

wxFont *config::GetFont(int offset)
{
	auto it = programFonts.find(10 + offset);
	if (it != programFonts.end()){
		return it->second;
	}
	
	int fontSize = GetInt(PROGRAM_FONT_SIZE);
	if (!fontSize)
		fontSize = 10;
	fontSize += offset;//*= ((10.f + (float)offset + 0.5f) / 10.f);
	wxString fontName = GetString(PROGRAM_FONT);
	if (fontName.empty())
		fontName = L"Tahoma";

	wxFont *newFont = new wxFont(fontSize, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, fontName);
	programFonts.insert(std::pair<int, wxFont*>(10 + offset, newFont));
	return newFont;
}

wxString getfloat(float num, const wxString &format, bool Truncate)
{
	wxString strnum = wxString::Format(L"%" + format, num);
	//if(strnum.find(L'.')!= -1){return strnum.Trim(false);}
	if (!Truncate || format.EndsWith(L".0f")){ return strnum.Trim(false); }
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
	wxRect rt(MonRects[0].left, MonRects[0].top,
		abs(MonRects[0].right - MonRects[0].left), abs(MonRects[0].bottom - MonRects[0].top));
	if (wmonitor == -1 || MonRects.size() == 1){
		return rt;
	}
	else if (wmonitor == 0){
		int x = position.x;
		int y = position.y;
		for (size_t i = 0; i < MonRects.size(); i++){
			if (MonRects[i].left <= x && x < MonRects[i].right && MonRects[i].top <= y && y < MonRects[i].bottom){
				if (MonitorRects)
					*MonitorRects = MonRects;
				return wxRect(MonRects[i].left, MonRects[i].top,
					abs(MonRects[i].right - MonRects[i].left), abs(MonRects[i].bottom - MonRects[i].top));
			}
		}
	}
	else{
		if (MonitorRects)
			*MonitorRects = MonRects;
		return wxRect(MonRects[wmonitor].left, MonRects[wmonitor].top,
			abs(MonRects[wmonitor].right - MonRects[wmonitor].left), abs(MonRects[wmonitor].bottom - MonRects[wmonitor].top));
	}
	return rt;
}

bool IsNumber(const wxString &test) {
	bool isnumber = true;
	wxString testchars = L"0123456789";
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




