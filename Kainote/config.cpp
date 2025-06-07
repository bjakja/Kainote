//  Copyright (c) 2016-2020, Marcin Drob

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
#include "OpennWrite.h"
#include "KaiMessageBox.h"
#include <wx/settings.h>
#include <wx/stdpaths.h>
#include <wx/dir.h>
#include <wx/string.h>
#include <wx/log.h>
#include "CsriMod.h"
#include "Notebook.h"
#include "gitparams.h"
//#include <windows.h>
#include "ConfigConverter.h"
#include "SubtitlesProviderManager.h"
#include <ShlObj.h>
#include <wx/msw/private.h>
#include <wx/mstream.h>
#include <wx/dc.h>
#include "Config.h"
#include "UtilsWindows.h"
#include <locale>;

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
	InitLanguagesTable();
	HDC dc = ::GetDC(nullptr);
	fontDPI = ::GetDeviceCaps(dc, LOGPIXELSY);
	::ReleaseDC(nullptr, dc);
}


config::~config()
{
	for (std::vector<Styles*>::iterator it = assstore.begin(); it != assstore.end(); it++)
	{
		delete (*it);
	}
	assstore.clear();

	FontsClear();
	SubtitlesProviderManager::DestroySubsProvider();
}

void config::FontsClear()
{
	for (std::map<int, wxFont*>::iterator it = programFonts.begin(); it != programFonts.end(); it++){
		delete it->second;
	}
	programFonts.clear();
}

int config::GetDPI()
{
	return fontDPI;
}

void config::FontsRescale(int dpi)
{
	/*for (std::map<int, wxFont*>::iterator it = programFonts.begin(); it != programFonts.end(); it++) {
		int newPixelSize = -(int)((it->first * ((double)dpi) / 72.0) + 0.5);
		it->second->SetPixelSize(wxSize(0, newPixelSize));
	}*/
	FontsClear();
	fontDPI = dpi;
}

wxString config::GetReleaseDate()
{
	return wxString(__DATE__) + L"  " + wxString(__TIME__);
}

bool config::SetRawOptions(const wxString &textconfig)
{
	wxStringTokenizer cfg(textconfig, L"\n");
	int g = 0;
	wxString token;
	wxString textBlock;
	bool block = false;
	while (cfg.HasMoreTokens())
	{
		token = cfg.NextToken();
		if (token.EndsWith(L"{")){
			block = true;
			textBlock << token << L"\n";
			continue;
		}
		else if (block){
			if (token == L"}"){
				textBlock << token;
				block = false;
				CatchValsLabs(textBlock);
				textBlock = emptyString;
				g++;
			}else
				textBlock << token << L"\n";

			continue;
		}
		token.Trim(false);
		token.Trim(true);
		if (token.length() > 0){ CatchValsLabs(token); g++; }
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
		wxString iopt1 = emptyString;
		stringConfig[opt] = iopt1 << iopt;
	}
}

void config::SetFloat(CONFIG opt, float fopt)
{
	if (opt >= 0 && opt < configSize){
		wxString fopt1 = emptyString;
		fopt1 << fopt;
		fopt1.Replace(L",", L".");
		stringConfig[opt] = fopt1;
	}
}

void config::GetRawOptions(wxString &options, bool Audio/*=false*/)
{
	options = L"[" + progname + L"]\r\n";
	for (size_t i = 1; i < configSize; i++) {
		if ((!Audio && i <= AUDIO_WHEEL_DEFAULT_TO_ZOOM) || (Audio && i > AUDIO_WHEEL_DEFAULT_TO_ZOOM) ||
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
	if (name != emptyString){
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

void config::SaveOptions(bool cfg, bool style, bool crashed)
{
	OpenWrite ow;
	if (cfg){
		wxString textfile;
		GetRawOptions(textfile);
		if (crashed)
			textfile << L"___Program Crashed___";

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

void config::LoadDefaultConfig(wxString * defaultOptions)
{
	wxString * configTable = (defaultOptions) ? defaultOptions : stringConfig;
	configTable[SHIFT_TIMES_TIME] = L"2000";
	configTable[SHIFT_TIMES_WHICH_LINES] = L"0";
	configTable[CONVERT_RESOLUTION_WIDTH] = L"1280";
	configTable[CONVERT_RESOLUTION_HEIGHT] = L"720";
	configTable[CONVERT_FPS] = L"23.976";
	configTable[CONVERT_STYLE] = L"Default";
	configTable[CONVERT_STYLE_CATALOG] = L"Default";
	configTable[DICTIONARY_LANGUAGE] = L"pl";
	configTable[SPELLCHECKER_ON] = L"true";
	configTable[STYLE_EDIT_FILTER_TEXT] = L"ĄĆĘŁŃÓŚŹŻąćęłńóśźż";
	configTable[FFMS2_VIDEO_SEEKING] = L"2";
	configTable[SHIFT_TIMES_BY_TIME] = L"false";
	configTable[GRID_FONT] = L"Tahoma";
	configTable[GRID_FONT_SIZE] = L"10";
	configTable[PROGRAM_FONT] = L"Tahoma";
	configTable[PROGRAM_FONT_SIZE] = L"10";
	configTable[GRID_SAVE_AFTER_CHARACTER_COUNT] = L"1";
	configTable[GRID_TAGS_SWAP_CHARACTER] = L"☀";
	configTable[SHIFT_TIMES_MOVE_FORWARD] = L"true";
	configTable[CONVERT_NEW_END_TIMES] = L"false";
	configTable[GRID_INSERT_START_OFFSET] = L"0";
	configTable[GRID_INSERT_END_OFFSET] = L"0";
	configTable[VIDEO_PLAY_AFTER_SELECTION] = L"0";
	configTable[STYLE_PREVIEW_TEXT] = L"Podgląd";
	configTable[PROGRAM_THEME] = L"DarkSentro";
	configTable[EDITOR_ON] = L"true";
	configTable[CONVERT_SHOW_SETTINGS] = L"false";
	configTable[SHIFT_TIMES_ON] = L"true";
	configTable[SHIFT_TIMES_WHICH_TIMES] = L"0";
	configTable[SHIFT_TIMES_STYLES] = emptyString;
	configTable[CONVERT_TIME_PER_CHARACTER] = L"110";
	configTable[VIDEO_INDEX] = L"true";
	configTable[VIDEO_PROGRESS_BAR] = L"true";
	configTable[VIDEO_WINDOW_SIZE] = L"500,350";
	configTable[WINDOW_SIZE] = L"800,600";
	configTable[AUTOMATION_TRACE_LEVEL] = L"3";
	configTable[AUTOSAVE_MAX_FILES] = L"3";
	configTable[GRID_CHANGE_ACTIVE_ON_SELECTION] = L"true";
}

//remember, create table[colorsSize] without this size it will crash
void config::LoadDefaultColors(bool dark, wxColour *table)
{
	wxColour *colours = (table) ? table : colors;
	colours[WINDOW_BACKGROUND].Set((dark) ? L"#202225" : L"#BFBFBF");
	colours[WINDOW_BACKGROUND_INACTIVE].Set((dark) ? L"#4E5155" : L"#C3C3C3");
	colours[WINDOW_TEXT].Set((dark) ? L"#AEAFB2" : L"#000000");
	colours[WINDOW_TEXT_INACTIVE].Set((dark) ? L"#7D7F83" : L"#0E0082");
	colours[WINDOW_BORDER].Set((dark) ? L"#2F3136" : L"#BFBFBF");
	colours[WINDOW_BORDER_INACTIVE].Set((dark) ? L"#4E5155" : L"#C3C3C3");
	colours[WINDOW_BORDER_BACKGROUND].Set((dark) ? L"#2F3136" : L"#7F7F7F");
	colours[WINDOW_BORDER_BACKGROUND_INACTIVE].Set((dark) ? L"#4E5155" : L"#AEAEAE");
	colours[WINDOW_HEADER_TEXT].Set((dark) ? L"#AEAFB2" : L"#000000");
	colours[WINDOW_HEADER_TEXT_INACTIVE].Set((dark) ? L"#7D7F83" : L"#242424");
	colours[WINDOW_HOVER_HEADER_ELEMENT].Set((dark) ? L"#5D636D" : L"#BFBFBF");
	colours[WINDOW_PUSHED_HEADER_ELEMENT].Set((dark) ? L"#788291" : L"#EEEEEE");
	colours[WINDOW_HOVER_CLOSE_BUTTON].Set((dark) ? L"#A22525" : L"#925B1F");
	colours[WINDOW_PUSHED_CLOSE_BUTTON].Set((dark) ? L"#E24443" : L"#FF6968");
	colours[WINDOW_WARNING_ELEMENTS].Set((dark) ? L"#8791FD" : L"#1A5325");
	colours[GRID_TEXT].Set((dark) ? L"#FFFFFF" : L"#000000");
	colours[GRID_BACKGROUND].Set((dark) ? L"#50565F" : L"#EEEEEE");
	colours[GRID_DIALOGUE].Set((dark) ? L"#50565F" : L"#EEEEEE");
	colours[GRID_COMMENT].Set((dark) ? L"#4880D0" : L"#4880D0");
	colours[GRID_SELECTION] = (dark) ? wxColour(0x87, 0x91, 0xFD, 75) : wxColour(0x0E, 0x00, 0x82, 75);//#4B0E0082 #4B8791FD #4B1A237E//#4C5AD693 #4C8EF8B5 new wxColour((dark)? L"#B6D5C5" : L"#EBF5EF");
	colours[GRID_LINE_VISIBLE_ON_VIDEO].Set((dark) ? L"#202225" : L"#BFBFBF");
	colours[GRID_COLLISIONS].Set((dark) ? L"#F1FF00" : L"#3600AA");
	colours[GRID_LINES].Set((dark) ? L"#202225" : L"#BFBFBF");
	colours[GRID_ACTIVE_LINE].Set((dark) ? L"#8791FD" : L"#000000");
	colours[GRID_HEADER].Set((dark) ? L"#2F3136" : L"#7F7F7F");
	colours[GRID_HEADER_TEXT].Set((dark) ? L"#8791FD" : L"#000000");
	colours[GRID_LABEL_NORMAL].Set((dark) ? L"#2F3136" : L"#7F7F7F");
	colours[GRID_LABEL_MODIFIED].Set((dark) ? L"#322F4E" : L"#B0ADD8");
	colours[GRID_LABEL_SAVED].Set((dark) ? L"#202225" : L"#BFBFBF");
	colours[GRID_LABEL_DOUBTFUL].Set((dark) ? L"#925B1F" : L"#925B1F");
	colours[GRID_SPELLCHECKER].Set((dark) ? L"#940000" : L"#FF6968");
	colours[GRID_COMPARISON_OUTLINE].Set((dark) ? L"#2700FF" : L"#FFFFFF");
	colours[GRID_COMPARISON_BACKGROUND_NOT_MATCH].Set((dark) ? L"#272B32" : L"#FF000C");
	colours[GRID_COMPARISON_BACKGROUND_MATCH].Set((dark) ? L"#3A3E45" : L"#B7AC00");
	colours[GRID_COMPARISON_COMMENT_BACKGROUND_NOT_MATCH].Set((dark) ? L"#003176" : L"#9C0000");
	colours[GRID_COMPARISON_COMMENT_BACKGROUND_MATCH].Set((dark) ? L"#3662A1" : L"#817900");
	colours[EDITOR_TEXT].Set((dark) ? L"#F4F4F4" : L"#000000");
	colours[EDITOR_TAG_NAMES].Set((dark) ? L"#00C3FF" : L"#787600");
	colours[EDITOR_TAG_VALUES].Set((dark) ? L"#0076FF" : L"#34C200");
	colours[EDITOR_CURLY_BRACES].Set((dark) ? L"#8791FD" : L"#7E3B00");
	colours[EDITOR_TAG_OPERATORS].Set((dark) ? L"#00C3FF" : L"#787600");
	colours[EDITOR_SPLIT_LINES_AND_DRAWINGS].Set((dark) ? L"#ADABAB" : L"#817F7F");
	colours[EDITOR_TEMPLATE_VARIABLES].Set((dark) ? L"#BDB76B" : L"#BDB76B");
	colours[EDITOR_TEMPLATE_CODE_MARKS].Set((dark) ? L"#FB4544" : L"#FB4544");
	colours[EDITOR_TEMPLATE_FUNCTIONS].Set((dark) ? L"#00C3FF" : L"#00C3FF");
	colours[EDITOR_TEMPLATE_KEYWORDS].Set((dark) ? L"#8D80D3" : L"#8D80D3");
	colours[EDITOR_TEMPLATE_STRINGS].Set((dark) ? L"#11A243" : L"#11A243");
	colours[EDITOR_PHRASE_SEARCH].Set((dark) ? L"#0B5808" : L"#0B5808");
	colours[EDITOR_BRACES_BACKGROUND].Set((dark) ? L"#F4F4F4" : L"#F4F4F4");
	colours[EDITOR_BACKGROUND].Set((dark) ? L"#50565F" : L"#EEEEEE");
	colours[EDITOR_SELECTION].Set((dark) ? L"#646D8A" : L"#BBC0FB");
	colours[EDITOR_SELECTION_NO_FOCUS].Set((dark) ? L"#646D8A" : L"#BBC0FB");
	colours[EDITOR_BORDER].Set((dark) ? L"#36393E" : L"#EEEEEE");
	colours[EDITOR_BORDER_ON_FOCUS].Set((dark) ? L"#8791FD" : L"#000000");
	colours[EDITOR_SPELLCHECKER].Set((dark) ? L"#940000" : L"#FF6968");
	colours[AUDIO_BACKGROUND].Set((dark) ? L"#36393E" : L"#36393E");
	colours[AUDIO_LINE_BOUNDARY_START].Set((dark) ? L"#940000" : L"#940000");
	colours[AUDIO_LINE_BOUNDARY_END].Set((dark) ? L"#940000" : L"#940000");
	colours[AUDIO_LINE_BOUNDARY_MARK].Set(L"#FFFFFF");
	colours[AUDIO_LINE_BOUNDARY_INACTIVE_LINE].Set(L"#00D77D");
	colours[AUDIO_PLAY_CURSOR].Set(L"#8791FD");
	colours[AUDIO_SECONDS_BOUNDARIES].Set(0xF4, 0xF4, 0xF4, 0x37);//#37F4F4F4//wxColour((dark)? L"#9B0B8A9F" : emptyString);
	colours[AUDIO_KEYFRAMES].Set(L"#F4F4F4");
	colours[AUDIO_SYLLABLE_BOUNDARIES].Set(L"#202225");
	colours[AUDIO_SYLLABLE_TEXT].Set(L"#8791FD");
	colours[AUDIO_SELECTION_BACKGROUND].Set(0xFF, 0xFF, 0xFF, 0x37);//wxColour((dark)? L"#37FFFFFF" : emptyString);
	colours[AUDIO_SELECTION_BACKGROUND_MODIFIED].Set(0xFF, 0xFF, 0xFF, 0x37);//wxColour((dark)? L"#37D60000" : emptyString);
	colours[AUDIO_INACTIVE_LINES_BACKGROUND].Set(0x00, 0x00, 0x00, 0x55);//wxColour((dark)? L"#55373564" : emptyString);
	colours[AUDIO_WAVEFORM].Set(L"#202225");
	colours[AUDIO_WAVEFORM_INACTIVE].Set(L"#2F3136");
	colours[AUDIO_WAVEFORM_MODIFIED].Set(L"#FF6968");
	colours[AUDIO_WAVEFORM_SELECTED].Set(L"#DCDAFF");
	colours[AUDIO_SPECTRUM_BACKGROUND].Set(L"#000000");
	colours[AUDIO_SPECTRUM_ECHO].Set(L"#674FD7");
	colours[AUDIO_SPECTRUM_INNER].Set(L"#F4F4F4");
	colours[TEXT_FIELD_BACKGROUND].Set((dark) ? L"#2F3136" : L"#7F7F7F");
	colours[TEXT_FIELD_BORDER].Set((dark) ? L"#36393E" : L"#7F7F7F");
	colours[TEXT_FIELD_BORDER_ON_FOCUS].Set((dark) ? L"#8791FD" : L"#000000");
	colours[TEXT_FIELD_SELECTION].Set((dark) ? L"#646D8A" : L"#BBDEFB");
	colours[TEXT_FIELD_SELECTION_NO_FOCUS].Set((dark) ? L"#383E4F" : L"#93B2CC");
	colours[BUTTON_BACKGROUND].Set((dark) ? L"#2F3136" : L"#A0A0A0");
	colours[BUTTON_BACKGROUND_HOVER].Set((dark) ? L"#53516B" : L"#DEDEDE");
	colours[BUTTON_BACKGROUND_PUSHED].Set((dark) ? L"#4F535B" : L"#EFEFEF");
	colours[BUTTON_BACKGROUND_ON_FOCUS].Set((dark) ? L"#4F535B" : L"#EFEFEF");
	colours[BUTTON_BORDER].Set((dark) ? L"#2F3136" : L"#A0A0A0");
	colours[BUTTON_BORDER_HOVER].Set((dark) ? L"#53516B" : L"#DEDEDE");
	colours[BUTTON_BORDER_PUSHED].Set((dark) ? L"#4F535B" : L"#EFEFEF");
	colours[BUTTON_BORDER_ON_FOCUS].Set((dark) ? L"#4F535B" : L"#EFEFEF");
	colours[BUTTON_BORDER_INACTIVE].Set((dark) ? L"#4F535B" : L"#A0A0A0");
	colours[TOGGLE_BUTTON_BACKGROUND_TOGGLED].Set((dark) ? L"#5C5889" : L"#656565");
	colours[TOGGLE_BUTTON_BORDER_TOGGLED].Set((dark) ? L"#5C5889" : L"#656565");
	colours[SCROLLBAR_BACKGROUND].Set((dark) ? L"#2F3136" : L"#7F7F7F");
	colours[SCROLLBAR_THUMB].Set((dark) ? L"#484B51" : L"#ACACAC");
	colours[SCROLLBAR_THUMB_HOVER].Set((dark) ? L"#8982D5" : L"#C5C5C5");
	colours[SCROLLBAR_THUMB_PUSHED].Set((dark) ? L"#9E96FF" : L"#DDDDDD");
	colours[STATICBOX_BORDER].Set((dark) ? L"#36393E" : L"#A0A0A0");
	colours[STATICLIST_BORDER].Set((dark) ? L"#36393E" : L"#EEEEEE");
	colours[STATICLIST_BACKGROUND].Set((dark) ? L"#36393E" : L"#EEEEEE");
	colours[STATICLIST_SELECTION].Set((dark) ? L"#4F535B" : L"#BFBFBF");
	colours[STATICLIST_BACKGROUND_HEADLINE].Set((dark) ? L"#2F3136" : L"#A0A0A0");
	colours[STATICLIST_TEXT_HEADLINE].Set((dark) ? L"#AEAFB2" : L"#000000");
	colours[STATUSBAR_BORDER].Set((dark) ? L"#2F3136" : L"#A0A0A0");
	colours[MENUBAR_BACKGROUND1].Set((dark) ? L"#202225" : L"#BFBFBF");
	colours[MENUBAR_BACKGROUND2].Set((dark) ? L"#202225" : L"#BFBFBF");
	colours[MENUBAR_BACKGROUND_HOVER].Set((dark) ? L"#53516B" : L"#DEDEDE");
	colours[MENUBAR_BORDER_SELECTION].Set((dark) ? L"#53516B" : L"#DEDEDE");
	colours[MENUBAR_BACKGROUND_SELECTION].Set((dark) ? L"#575478" : L"#BBDEFB");
	colours[MENUBAR_BACKGROUND].Set((dark) ? L"#202225" : L"#BFBFBF");
	colours[MENU_BORDER_SELECTION].Set((dark) ? L"#53516B" : L"#DEDEDE");
	colours[MENU_BACKGROUND_SELECTION].Set((dark) ? L"#53516B" : L"#DEDEDE");
	colours[TABSBAR_BACKGROUND1].Set((dark) ? L"#2F3136" : L"#A0A0A0");
	colours[TABSBAR_BACKGROUND2].Set((dark) ? L"#2F3136" : L"#A0A0A0");
	colours[TABS_BORDER_ACTIVE].Set((dark) ? L"#202225" : L"#BFBFBF");
	colours[TABS_BORDER_INACTIVE].Set((dark) ? L"#202225" : L"#A0A0A0");
	colours[TABS_BACKGROUND_ACTIVE].Set((dark) ? L"#53516B" : L"#BFBFBF");
	colours[TABS_BACKGROUND_INACTIVE].Set((dark) ? L"#2F3136" : L"#A0A0A0");
	colours[TABS_BACKGROUND_INACTIVE_HOVER].Set((dark) ? L"#53516B" : L"#BFBFBF");
	colours[TABS_BACKGROUND_SECOND_WINDOW].Set((dark) ? L"#383654" : L"#BFBFBF");
	colours[TABS_TEXT_ACTIVE].Set((dark) ? L"#FFFFFF" : L"#000000");
	colours[TABS_TEXT_INACTIVE].Set((dark) ? L"#7D7F83" : L"#EEEEEE");
	colours[TABS_CLOSE_HOVER].Set((dark) ? L"#A22525" : L"#925B1F");
	colours[TABSBAR_ARROW].Set((dark) ? L"#202225" : L"#BFBFBF");
	colours[TABSBAR_ARROW_BACKGROUND].Set((dark) ? L"#2F3136" : L"#A0A0A0");
	colours[TABSBAR_ARROW_BACKGROUND_HOVER].Set((dark) ? L"#76777B" : L"#797979");
	colours[SLIDER_PATH_BACKGROUND].Set((dark) ? L"#2F3136" : L"#7F7F7F");
	colours[SLIDER_PATH_BORDER].Set((dark) ? L"#2F3136" : L"#7F7F7F");
	colours[SLIDER_BORDER].Set((dark) ? L"#5C5889" : L"#000000");
	colours[SLIDER_BORDER_HOVER].Set((dark) ? L"#8982D5" : L"#000000");
	colours[SLIDER_BORDER_PUSHED].Set((dark) ? L"#9E96FF" : L"#000000");
	colours[SLIDER_BACKGROUND].Set((dark) ? L"#5C5889" : L"#ACACAC");
	colours[SLIDER_BACKGROUND_HOVER].Set((dark) ? L"#8982D5" : L"#C5C5C5");
	colours[SLIDER_BACKGROUND_PUSHED].Set((dark) ? L"#9E96FF" : L"#DDDDDD");
	colours[WINDOW_RESIZER_DOTS].Set((dark) ? L"#2F3136" : L"#FFFFFF");
	colours[FIND_RESULT_FILENAME_FOREGROUND].Set((dark) ? L"#BB0099" : L"#5D67CF");
	colours[FIND_RESULT_FILENAME_BACKGROUND].Set((dark) ? L"#440033" : L"#000000");
	colours[FIND_RESULT_FOUND_PHRASE_FOREGROUND].Set((dark) ? L"#FFFFFF" : L"#FFFFFF");
	colours[FIND_RESULT_FOUND_PHRASE_BACKGROUND].Set((dark) ? L"#8791FD" : L"#5D67CF");
	colours[STYLE_PREVIEW_COLOR1].Set(L"#434343");
	colours[STYLE_PREVIEW_COLOR2].Set(L"#626262");

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
		if (txt.EndsWith(L"___Program Crashed___", &txt)){
			hasCrashed = true;
		}
		if (ver != progname){ LoadDefaultConfig(); diffVersions = true; }
		txt = txt.AfterFirst(L'\n');
		if (ConfigNeedToConvert(ver)){
			ConfigConverter::Get()->ConvertConfig(&txt);
			ow.FileWrite(path, txt);
		}
		isgood = SetRawOptions(txt);
	}

	actualStyleDir = L"Default";
	path = emptyString;
	path << pathfull << L"\\Catalog\\";
	wxDir kat(path);
	if (!kat.IsOpened()){
		ow.FileWrite(path << actualStyleDir << L".sty",
			L"Style: Default,Garamond,30,&H00FFFFFF,&H000000FF,&H00FF0000,&H00000000,0,0,0,0,100,100,0,0,0,2,2,2,10,10,10,1");
		AddStyle(new Styles()); dirs.Add(actualStyleDir);
	}
	else{
		wxArrayString tmp; kat.GetAllFiles(path, &tmp, emptyString, wxDIR_FILES);
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
		themeName = Options.GetString(PROGRAM_THEME);
	}
	else{
		themeName = _themeName;
		Options.SetString(PROGRAM_THEME, _themeName);
	}
	bool failed = false;
	if (themeName != L"DarkSentro" && themeName != L"LightSentro"){
		wxString path = pathfull + L"\\Themes\\" + themeName + L".txt";
		OpenWrite ow;
		wxString txtColors;
		if (ow.FileOpen(path, &txtColors, false)){
			if (txtColors.BeforeFirst(L'\n').Find(L'_') == -1){
				ConfigConverter::Get()->ConvertColors(&txtColors);
				ow.FileWrite(path, txtColors);
			}
			wxStringTokenizer cfg(txtColors, L"\n");
			int g = 0;
			while (cfg.HasMoreTokens())
			{
				wxString token = cfg.NextToken();
				token.Trim(false);
				token.Trim(true);
				if (token.length() > 6){ SetHexColor(token); g++; }
			}
			if (g > 10){
				if (colors[0].IsOk() || g < STYLE_PREVIEW_COLOR2){
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
		Options.SetString(PROGRAM_THEME, L"DarkSentro");
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
	wxString iopt1 = emptyString;
	stringConfig[opt] = iopt1 << coordx << L"," << coordy;
}

void config::GetCoords(CONFIG opt, int *coordx, int *coordy)
{
	wxString sopt = stringConfig[opt];
	*coordx = wxAtoi(sopt.BeforeFirst(L','));
	*coordy = wxAtoi(sopt.AfterFirst(L','));
}

void config::SetTable(CONFIG opt, wxArrayString &asopt)
{
	wxString sresult = L"{\n";
	//wxString ES;
	for (size_t i = 0; i < asopt.size(); i++)
	{
		//wxString endchar = (i == asopt.size() - 1) ? ES : split;
		sresult << L"\t" << asopt[i] << L"\n";
	}
	stringConfig[opt] = sresult + L"}";
}

void config::SetStringTable(CONFIG opt, wxArrayString &asopt, wxString split)
{
	wxString sresult;
	wxString ES;
	for (size_t i = 0; i < asopt.size(); i++)
	{
		wxString endchar = (i == asopt.size() - 1) ? ES : split;
	}
	stringConfig[opt] = sresult;
}

void config::SetIntTable(CONFIG opt, wxArrayInt &asopt)
{
	wxString sresult = L"{\n";
	for (size_t i = 0; i < asopt.size(); i++)
	{
		sresult << L"\t" << asopt[i] << L"\n";
	}
	stringConfig[opt] = sresult + L"}";
}

void config::GetTable(CONFIG opt, wxArrayString &tbl, int mode)
{
	wxString strtbl = stringConfig[opt];
	size_t length = strtbl.length();
	if (length> 4){
		//remove "{\n" from start i "\n}" from end
		strtbl = strtbl.Mid(2, length - 4);
		wxStringTokenizer cfgtable(strtbl, L"\n", (wxStringTokenizerMode)mode);
		while (cfgtable.HasMoreTokens()){
			tbl.Add(cfgtable.NextToken().Mid(1));
		}
	}
}

void config::GetIntTable(CONFIG opt, wxArrayInt &tbl, int mode)
{
	wxString strtbl = stringConfig[opt];
	size_t length = strtbl.length();
	if (length> 4){
		//remove "{\n" from start i "\n}" from end
		strtbl = strtbl.Mid(2, length - 4);
		wxStringTokenizer cfgtable(strtbl, L"\n", (wxStringTokenizerMode)mode);
		while (cfgtable.HasMoreTokens()){
			tbl.Add(wxAtoi(cfgtable.NextToken().Mid(1)));
		}
	}
}

void config::GetTableFromString(CONFIG opt, wxArrayString &tbl, wxString split, int mode)
{
	wxString strtbl = stringConfig[opt];
	if (strtbl != emptyString){
		wxStringTokenizer cfgtable(strtbl, split, (wxStringTokenizerMode)mode);
		while (cfgtable.HasMoreTokens()){
			tbl.Add(cfgtable.NextToken());
		}
	}
}

bool sortfunc(Styles *style1, Styles *style2){
	const std::collate<wchar_t>& f = std::use_facet<std::collate<wchar_t>>(KainoteFrame::GetLocale());
	const wchar_t* s1 = style1->Name.wc_str();
	const wchar_t* s2 = style2->Name.wc_str();
	return (f.compare(&s1[0], &s1[0] + wcslen(s1),
		&s2[0], &s2[0] + wcslen(s2)) < 0);
	
}

void config::Sortstyles()
{
	std::sort(assstore.begin(), assstore.end(), sortfunc);
}

void config::LoadDefaultAudioConfig(wxString * defaultOptions)
{
	wxString * configTable = (defaultOptions) ? defaultOptions : stringConfig;
	configTable[AUDIO_AUTO_COMMIT] = L"true";
	configTable[AUDIO_AUTO_FOCUS] = L"true";
	configTable[AUDIO_AUTO_SCROLL] = L"true";
	configTable[AUDIO_BOX_HEIGHT] = L"169";
	configTable[AUDIO_CACHE_FILES_LIMIT] = L"10";
	configTable[AUDIO_DELAY] = L"0";
	configTable[AUDIO_DRAW_TIME_CURSOR] = L"true";
	configTable[AUDIO_DRAW_KEYFRAMES] = L"true";
	configTable[AUDIO_DRAW_SECONDARY_LINES] = L"true";
	configTable[AUDIO_DRAW_SELECTION_BACKGROUND] = L"true";
	configTable[AUDIO_DRAW_VIDEO_POSITION] = L"true";
	configTable[AUDIO_GRAB_TIMES_ON_SELECT] = L"true";
	configTable[AUDIO_HORIZONTAL_ZOOM] = L"50";
	configTable[AUDIO_INACTIVE_LINES_DISPLAY_MODE] = L"1";
	configTable[AUDIO_KARAOKE] = L"false";
	configTable[AUDIO_KARAOKE_SPLIT_MODE] = L"true";
	configTable[AUDIO_LEAD_IN_VALUE] = L"200";
	configTable[AUDIO_LEAD_OUT_VALUE] = L"300";
	configTable[AUDIO_LINE_BOUNDARIES_THICKNESS] = L"2";
	configTable[AUDIO_LINK] = L"false";
	configTable[AUDIO_LOCK_SCROLL_ON_CURSOR] = L"false";
	configTable[AUDIO_MARK_PLAY_TIME] = L"1000";
	configTable[AUDIO_NEXT_LINE_ON_COMMIT] = L"true";
	configTable[AUDIO_RAM_CACHE] = L"false";
	configTable[AUDIO_SNAP_TO_KEYFRAMES] = L"false";
	configTable[AUDIO_SNAP_TO_OTHER_LINES] = L"false";
	configTable[AUDIO_SPECTRUM_ON] = L"false";
	configTable[AUDIO_START_DRAG_SENSITIVITY] = L"6";
	configTable[AUDIO_VERTICAL_ZOOM] = L"50";
	configTable[AUDIO_VOLUME] = L"50";
	configTable[AUDIO_WHEEL_DEFAULT_TO_ZOOM] = L"false";

}

void config::ResetDefault()
{
	wxString options[configSize];
	Options.LoadDefaultAudioConfig(options);
	Options.LoadDefaultConfig(options);
	for (size_t i = 0; i < configSize; i++){
		stringConfig[i] = options[i];
	}
}

bool config::ConfigNeedToConvert(const wxString & fullVersion)
{
	int first = fullVersion.find(L".");//0.8.0.build
	if (first > -1){
		wxString ver = fullVersion.Mid(first + 5).BeforeFirst(L' ');
		int version = wxAtoi(ver);
		return (version < 1142);
	}
	return true;
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
		txt = txt.AfterFirst(L'\n');
		if (ver != progname){ LoadDefaultAudioConfig(); }
		
		if (ConfigNeedToConvert(ver)){
			ConfigConverter::Get()->ConvertConfig(&txt);
			ow.FileWrite(configPath + L"\\AudioConfig.txt", txt);
		}
	}
	return (AudioOpts = SetRawOptions(txt));
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
		finalpath = pathfull + L"\\Themes\\" + GetString(PROGRAM_THEME) + L".txt";
	}
	OpenWrite ow(finalpath, true);
	//ow.PartFileWrite(L"[" + progname + L"]\n");
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
	fontSize += offset;
	wxString fontName = GetString(PROGRAM_FONT);
	if (fontName.empty())
		fontName = L"Tahoma";

	int newPixelSize = -(int)(((double)fontSize * ((double)fontDPI) / 72.0) + 0.5);

	wxFont *newFont = new wxFont(wxSize(0, newPixelSize), wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, fontName);
	programFonts.insert(std::pair<int, wxFont*>(10 + offset, newFont));
	return newFont;
}

const wxString &config::FindLanguage(const wxString & symbol)
{
	const auto &it = Languages.find(symbol);
	if (it != Languages.end())
		return it->second;

	return symbol;
}

bool config::CheckLastKeyEvent(int id, int timeInterval)
{
	DWORD nowTime = timeGetTime();
	if (nowTime < lastCheckedTime && lastCheckedTime != 0)
		lastCheckedTime = nowTime;
	else if (id == lastCheckedId && nowTime < lastCheckedTime + timeInterval){
		return true;
	}

	lastCheckedTime = nowTime;
	lastCheckedId = id;
	return false;
}

void SelectInFolder(const wxString& filename)
{
	CoInitialize(0);
	ITEMIDLIST* pidl = ILCreateFromPathW(filename.wc_str());
	if (pidl) {
		SHOpenFolderAndSelectItems(pidl, 0, 0, 0);
		ILFree(pidl);
	}
	CoUninitialize();
}

void OpenInBrowser(const wxString& adress)
{
	WinStruct<SHELLEXECUTEINFO> sei;
	sei.lpFile = adress.c_str();
	sei.lpVerb = wxT("open");
	sei.nShow = SW_RESTORE;
	sei.fMask = SEE_MASK_FLAG_NO_UI; // we give error message ourselves
	ShellExecuteEx(&sei);
}

bool IsNumberFloat(const wxString& test) {
	bool isnumber = true;
	wxString testchars = L"0123456789.";
	for (size_t i = 0; i < test.length(); i++) {
		wxUniChar ch = test[i];
		if (testchars.Find(ch) == -1) { isnumber = false; break; }
	}
	return isnumber;
}

wxString getfloat(float num, const wxString& format, bool Truncate)
{
	wxString strnum = wxString::Format(L"%" + format, num);
	//if(strnum.find(L'.')!= -1){return strnum.Trim(false);}
	if (!Truncate || format.EndsWith(L".0f")) { return strnum.Trim(false); }
	int rmv = 0;
	bool trim = false;
	for (int i = strnum.Len() - 1; i > 0; i--)
	{
		if (strnum[i] == L'0') { rmv++; }//&&!trim
		//else if(strnum[i]==L'9'){rmv++;trim=true;}
		else if (strnum[i] == L'.') { rmv++; break; }//}if(!trim){
		else {/*if(trim){int tmpc=static_cast < int >(strnum.GetChar(i));tmpc++;strnum[i]=(wxUniChar)tmpc;}*/break; }
	}
	if (rmv) { strnum.RemoveLast(rmv); }
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
		a_resHandle = LoadResource(nullptr, a_resource);
		if (0 != a_resHandle)
		{
			t_data = (char*)LockResource(a_resHandle);
			t_dataSize = SizeofResource(nullptr, a_resource);
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

	char* a_data = 0;
	DWORD       a_dataSize = 0;

	if (LoadDataFromResource(a_data, a_dataSize, t_name))
	{
		r_bitmapPtr = GetBitmapFromMemory(a_data, a_dataSize);
	}

	return r_bitmapPtr;
}

wxBitmap* CreateBitmapPointerFromPngResource(const wxString& t_name)
{
	wxBitmap* r_bitmapPtr = nullptr;

	char* a_data = 0;
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

	char* a_data = 0;
	DWORD       a_dataSize = 0;

	if (LoadDataFromResource(a_data, a_dataSize, t_name))
	{
		wxMemoryInputStream a_is(a_data, a_dataSize);
		image = wxImage(a_is, wxBITMAP_TYPE_PNG, -1);
	}

	return image;
}

void MoveToMousePosition(wxWindow* win)
{
	wxPoint mst = wxGetMousePosition();
	wxSize siz = win->GetSize();
	siz.x;
	wxRect rc = GetMonitorWorkArea(0, nullptr, mst, true);
	mst.x -= (siz.x / 2);
	mst.x = MID(rc.x, mst.x, (rc.width + rc.x) - siz.x);
	mst.y += 15;
	if (mst.y + siz.y > rc.height + rc.y) {
		mst.y = mst.y - siz.y - 30;
		if (mst.y < rc.y) {
			mst.y = (rc.height + rc.y) - siz.y;
		}
	}
	win->Move(mst);
}

wxString MakePolishPlural(int num, const wxString& normal, const wxString& plural2to4, const wxString& pluralRest)
{
	wxString result;
	int div10mod = (num % 10);
	int div100mod = (num % 100);
	if (num == 1 || num == -1) { result = normal; }
	else if ((div10mod >= 2 && div10mod <= 4) && (div100mod < 10 || div100mod>20)) {
		result = plural2to4;
	}
	else {
		result = pluralRest;
	}
	wxString finalResult;
	return finalResult << num << " " << result;
}

bool IsNumber(const wxString& test) {
	bool isnumber = true;
	wxString testchars = L"0123456789";
	for (size_t i = 0; i < test.length(); i++) {
		wxUniChar ch = test[i];
		if (testchars.Find(ch) == -1) { isnumber = false; break; }
	}
	return isnumber;
}

void DrawDashedLine(wxDC* dc, wxPoint* vector, size_t vectorSize, int dashLen, const wxColour& color)
{

	wxPoint actualPoint[2];
	wxPen tmppen = dc->GetPen();
	dc->SetPen(color);
	for (size_t i = 0; i < vectorSize - 1; i++) {
		size_t iPlus1 = (i < (vectorSize - 1)) ? i + 1 : 0;
		wxPoint pdiff = vector[i] - vector[iPlus1];
		int len = sqrt((pdiff.x * pdiff.x) + (pdiff.y * pdiff.y));
		if (len == 0) { return; }
		wxPoint diffUnits = pdiff / len;
		float singleMovement = 1.f / (len / (float)(dashLen * 2));
		if (singleMovement == 0) {
			return;
		}
		actualPoint[0] = vector[i];
		actualPoint[1] = actualPoint[0];
		for (float j = 0; j <= 1; j += singleMovement) {
			actualPoint[1] -= diffUnits * dashLen;
			if (j + singleMovement >= 1) { actualPoint[1] = vector[iPlus1]; }
			dc->DrawLines(2, actualPoint);
			actualPoint[1] -= diffUnits * dashLen;
			actualPoint[0] -= (diffUnits * dashLen) * 2;
		}
	}
	dc->SetPen(tmppen);
}

size_t FindFromEnd(const wxString& text, const wxString& whatToFind, bool ignoreCase)
{
	const wxString casetext = ignoreCase ? text.Lower() : text;
	const wxString caseWTF = ignoreCase ? whatToFind.Lower() : whatToFind;
	size_t len = casetext.length();
	size_t wtflen = caseWTF.length();
	if (!len || !wtflen)
		return -1;

	wxUniChar wtfch = caseWTF[wtflen - 1];
	for (size_t i = len - 1; i + 1 > 0; i--) {
		const wxUniChar ch = casetext[i];
		if (ch == wtfch) {
			if (wtflen == 1)
				return i;
			for (size_t k = 1; k < wtflen && (i - k) + 1 > 0; k++) {
				const wxUniChar cch = casetext[i - k];
				if (cch != caseWTF[(wtflen - 1) - k]) {
					break;
				}
				if (k == wtflen - 1) {
					return i - k;
				}
			}
		}
	}
	return -1;
}

DEFINE_ENUM(CONFIG, CFG);

DEFINE_ENUM(COLOR, CLR);

config Options;




