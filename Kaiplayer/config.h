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


#pragma once

//#include <wx/textfile.h>
#include <wx/tokenzr.h>
//#include <wx/colour.h>
//#include <wx/string.h>
//#include <wx/image.h>
#include <map>
#include <vector>
#include <algorithm>
#include "Styles.h"
#include <wx/utils.h> 
#include "EnumFactory.h"
#include <wx/wx.h>
#include "LogHandler.h"
#undef wxBITMAP_PNG


#ifndef MIN
#define MIN(a,b) ((a)<(b))?(a):(b)
#endif

#ifndef MAX
#define MAX(a,b) ((a)>(b))?(a):(b)
#endif

#ifndef MID
#define MID(a,b,c) MAX((a),MIN((b),(c)))
#endif

//Pamiêtaj, zmiana ostatniej audio opcji wymaga te¿ zmiany przy szukaniu w zapisie
//Dont change enumeration config and colors from 1 to last, zero for non exist trash
#define CFG(CG) \
	CG(AudioAutoCommit,=1)\
	CG(AudioAutoFocus,)\
	CG(AudioAutoScroll,)\
	CG(AudioBoxHeight,)\
	CG(AudioDelay,)\
	CG(AudioDrawKeyframes,)\
	CG(AudioDrawSecondaryLines,)\
	CG(AudioDrawSelectionBackground,)\
	CG(AudioDrawTimeCursor,)\
	CG(AudioDrawVideoPosition,)\
	CG(AudioGrabTimesOnSelect,)\
	CG(AudioHorizontalZoom,)\
	CG(AudioInactiveLinesDisplayMode,)\
	CG(AudioKaraoke,)\
	CG(AudioKaraokeMoveOnClick,)\
	CG(AudioKaraokeSplitMode,)\
	CG(AudioLeadIn,)\
	CG(AudioLeadOut,)\
	CG(AudioLineBoundariesThickness,)\
	CG(AudioLink,)\
	CG(AudioLockScrollOnCursor,)\
	CG(AudioMarkPlayTime,)\
	CG(AudioMergeEveryNWithSyllable,)\
	CG(AudioNextLineOnCommit,)\
	CG(AudioRAMCache,)\
	CG(AudioSnapToKeyframes,)\
	CG(AudioSnapToOtherLines,)\
	CG(AudioSpectrumOn,)\
	CG(AudioSpectrumNonLinearOn,)\
	CG(AudioStartDragSensitivity,)\
	CG(AudioVerticalZoom,)\
	CG(AudioVolume,)\
	CG(AudioWheelDefaultToZoom,)\
	CG(AcceptedAudioStream,)\
	CG(ASSPropertiesTitle,)\
	CG(ASSPropertiesScript,)\
	CG(ASSPropertiesTranslation,)\
	CG(ASSPropertiesEditing,)\
	CG(ASSPropertiesTiming,)\
	CG(ASSPropertiesUpdate,)\
	CG(ASSPropertiesTitleOn,)\
	CG(ASSPropertiesScriptOn,)\
	CG(ASSPropertiesTranslationOn,)\
	CG(ASSPropertiesEditingOn,)\
	CG(ASSPropertiesTimingOn,)\
	CG(ASSPropertiesUpdateOn,)\
	CG(ASSPropertiesAskForChange,)\
	CG(AudioRecent,)\
	CG(AutomationLoadingMethod,)\
	CG(AutomationOldScriptsCompatybility,)\
	CG(AutomationRecent,)\
	CG(AutomationScriptEditor,)\
	CG(AutomationTraceLevel,)\
	CG(AutoMoveTagsFromOriginal,)\
	CG(AutoSaveMaxFiles,)\
	CG(AutoSelectLinesFromLastTab,)\
	CG(ColorpickerRecent,)\
	CG(ConvertASSTagsOnLineStart,)\
	CG(ConvertFPS,)\
	CG(ConvertFPSFromVideo,)\
	CG(ConvertNewEndTimes,)\
	CG(ConvertResolutionWidth,)\
	CG(ConvertResolutionHeight,)\
	CG(ConvertShowSettings,)\
	CG(ConvertStyle,)\
	CG(ConvertStyleCatalog,)\
	CG(ConvertTimePerLetter,)\
	CG(CopyCollumnsSelection,)\
	CG(DictionaryLanguage,)\
	CG(DisableLiveVideoEditing,)\
	CG(DontAskForBadResolution,)\
	CG(EditboxSugestionsOnDoubleClick,)\
	CG(EDITBOX_TIMES_TO_FRAMES_SWITCH,)\
	CG(EditorOn,)\
	CG(FIND_IN_SUBS_FILTERS_RECENT,)\
	CG(FIND_IN_SUBS_PATHS_RECENT,)\
	CG(FIND_REPLACE_STYLES,)\
	CG(FindRecent,)\
	CG(FindReplaceOptions,)\
	CG(FontCollectorAction,)\
	CG(FontCollectorDirectory,)\
	CG(FontCollectorFromMKV,)\
	CG(FontCollectorUseSubsDirectory,)\
	CG(FFMS2VideoSeeking,)\
	CG(GridChangeActiveOnSelection,)\
	CG(GridFontName,)\
	CG(GridFontSize,)\
	CG(GridAddToFilter,)\
	CG(GridFilterAfterLoad,)\
	CG(GridFilterBy,)\
	CG(GridFilterInverted,)\
	CG(GridFilterStyles,)\
	CG(GridHideCollums,)\
	CG(GridHideTags,)\
	CG(GridIgnoreFiltering,)\
	CG(GridLoadSortedSubs,)\
	CG(GridSaveAfterCharacterCount,)\
	CG(GridTagsSwapChar,)\
	CG(InsertEndOffset,)\
	CG(InsertStartOffset,)\
	CG(KEYFRAMES_RECENT,)\
	CG(MoveTimesByTime,)\
	CG(MoveTimesLoadSetTabOptions,)\
	CG(MoveTimesCorrectEndTimes,)\
	CG(MoveTimesForward,)\
	CG(MoveTimesFrames,)\
	CG(MoveTimesOn,)\
	CG(MoveTimesOptions,)\
	CG(MoveTimesWhichLines,)\
	CG(MoveTimesWhichTimes,)\
	CG(MoveTimesStyles,)\
	CG(MoveTimesTime,)\
	CG(MoveVideoToActiveLine,)\
	CG(NoNewLineAfterTimesEdition,)\
	CG(OpenSubsInNewCard,)\
	CG(OpenVideoAtActiveLine,)\
	CG(PasteCollumnsSelection,)\
	CG(PlayAfterSelection,)\
	CG(PostprocessorEnabling,)\
	CG(PostprocessorKeyframeBeforeStart,)\
	CG(PostprocessorKeyframeAfterStart,)\
	CG(PostprocessorKeyframeBeforeEnd,)\
	CG(PostprocessorKeyframeAfterEnd,)\
	CG(PostprocessorLeadIn,)\
	CG(PostprocessorLeadOut,)\
	CG(PostprocessorThresholdStart,)\
	CG(PostprocessorThresholdEnd,)\
	CG(PreviewText,)\
	CG(ProgramLanguage,)\
	CG(ProgramTheme,)\
	CG(ReplaceRecent,)\
	CG(SelectionsRecent,)\
	CG(SelectionsOptions,)\
	CG(SelectVisibleLineAfterFullscreen,)\
	CG(SpellcheckerOn,)\
	CG(StyleEditFilterText,)\
	CG(StyleFilterTextOn,)\
	CG(StyleManagerPosition,)\
	CG(StyleManagerDetachEditor,)\
	CG(SubsAutonaming,)\
	CG(SubsComparisonType,)\
	CG(SubsComparisonStyles,)\
	CG(SubsRecent,)\
	CG(TextFieldAllowNumpadHotkeys,)\
	CG(TlModeShowOriginal,)\
	CG(TL_MODE_HIDE_ORIGINAL_ON_VIDEO,)\
	CG(ToolbarIDs,)\
	CG(ToolbarAlignment,)\
	CG(UpdaterCheckIntensity,)\
	CG(UpdaterCheckOptions,)\
	CG(UpdaterCheckForStable,)\
	CG(UpdaterLastCheck,)\
	CG(VideoFullskreenOnStart,)\
	CG(VideoIndex,)\
	CG(VideoPauseOnClick,)\
	CG(VideoProgressBar,)\
	CG(VideoRecent,)\
	CG(VideoVolume,)\
	CG(VideoWindowSize,)\
	CG(VisualWarningsOff,)\
	CG(WindowMaximized,)\
	CG(WindowPosition,)\
	CG(WindowSize,)\
	CG(EditboxTagButtons,)\
	CG(EditboxTagButton1,)\
	CG(EditboxTagButton2,)\
	CG(EditboxTagButton3,)\
	CG(EditboxTagButton4,)\
	CG(EditboxTagButton5,)\
	CG(EditboxTagButton6,)\
	CG(EditboxTagButton7,)\
	CG(EditboxTagButton8,)\
	CG(EditboxTagButton9,)\
	CG(EditboxTagButton10,)\
	//if you write here a new enum then change configSize
	
DECLARE_ENUM(CONFIG,CFG)

#define CLR(CR) \
	CR(WindowBackground,=1)\
	CR(WindowBackgroundInactive,)\
	CR(WindowText,)\
	CR(WindowTextInactive,)\
	CR(WindowBorder,)\
	CR(WindowBorderInactive,)\
	CR(WindowBorderBackground,)\
	CR(WindowBorderBackgroundInactive,)\
	CR(WindowHeaderText,)\
	CR(WindowHeaderTextInactive,)\
	CR(WindowHoverHeaderElement,)\
	CR(WindowPushedHeaderElement,)\
	CR(WindowHoverCloseButton,)\
	CR(WindowPushedCloseButton,)\
	CR(WindowWarningElements,)\
	CR(GridText,)\
	CR(GridBackground,)\
	CR(GridDialogue,)\
	CR(GridComment,)\
	CR(GridSelection,)\
	CR(GridVisibleOnVideo,)\
	CR(GridCollisions,)\
	CR(GridLines,)\
	CR(GridActiveLine,)\
	CR(GridHeader,)\
	CR(GridHeaderText,)\
	CR(GridLabelNormal,)\
	CR(GridLabelModified,)\
	CR(GridLabelSaved,)\
	CR(GridLabelDoubtful,)\
	CR(GridSpellchecker,)\
	CR(GridComparisonOutline,)\
	CR(GridComparisonBackgroundNotMatch,)\
	CR(GridComparisonBackgroundMatch,)\
	CR(GridComparisonCommentBackgroundNotMatch,)\
	CR(GridComparisonCommentBackgroundMatch,)\
	CR(EditorText,)\
	CR(EditorTagNames,)\
	CR(EditorTagValues,)\
	CR(EditorCurlyBraces,)\
	CR(EditorTagOperators,)\
	CR(EditorTemplateVariables,)\
	CR(EditorTemplateCodeMarks,)\
	CR(EditorTemplateFunctions,)\
	CR(EditorTemplateKeywords,)\
	CR(EditorTemplateStrings,)\
	CR(EditorPhraseSearch,)\
	CR(EditorBracesBackground,)\
	CR(EditorBackground,)\
	CR(EditorSelection,)\
	CR(EditorSelectionNoFocus,)\
	CR(EditorBorder,)\
	CR(EditorBorderOnFocus,)\
	CR(EditorSpellchecker,)\
	CR(AudioBackground,)\
	CR(AudioLineBoundaryStart,)\
	CR(AudioLineBoundaryEnd,)\
	CR(AudioLineBoundaryMark,)\
	CR(AudioLineBoundaryInactiveLine,)\
	CR(AudioPlayCursor,)\
	CR(AudioSecondsBoundaries,)\
	CR(AudioKeyframes,)\
	CR(AudioSyllableBoundaries,)\
	CR(AudioSyllableText,)\
	CR(AudioSelectionBackground,)\
	CR(AudioSelectionBackgroundModified,)\
	CR(AudioInactiveLinesBackground,)\
	CR(AudioWaveform,)\
	CR(AudioWaveformInactive,)\
	CR(AudioWaveformModified,)\
	CR(AudioWaveformSelected,)\
	CR(AudioSpectrumBackground,)\
	CR(AudioSpectrumEcho,)\
	CR(AudioSpectrumInner,)\
	CR(TextFieldBackground,)\
	CR(TextFieldBorder,)\
	CR(TextFieldBorderOnFocus,)\
	CR(TextFieldSelection,)\
	CR(TextFieldSelectionNoFocus,)\
	CR(ButtonBackground,)\
	CR(ButtonBackgroundHover,)\
	CR(ButtonBackgroundPushed,)\
	CR(ButtonBackgroundOnFocus,)\
	CR(ButtonBorder,)\
	CR(ButtonBorderHover,)\
	CR(ButtonBorderPushed,)\
	CR(ButtonBorderOnFocus,)\
	CR(ButtonBorderInactive,)\
	CR(TogglebuttonBackgroundToggled,)\
	CR(TogglebuttonBorderToggled,)\
	CR(ScrollbarBackground,)\
	CR(ScrollbarScroll,)\
	CR(ScrollbarScrollHover,)\
	CR(ScrollbarScrollPushed,)\
	CR(StaticboxBorder,)\
	CR(StaticListBorder,)\
	CR(StaticListBackground,)\
	CR(StaticListSelection,)\
	CR(StaticListBackgroundHeadline,)\
	CR(StaticListTextHeadline,)\
	CR(StatusBarBorder,)\
	CR(MenuBarBackground1,)\
	CR(MenuBarBackground2,)\
	CR(MenuBarBorderSelection,)\
	CR(MenuBarBackgroundSelection,)\
	CR(MenuBackground,)\
	CR(MenuBorderSelection,)\
	CR(MenuBackgroundSelection,)\
	CR(TabsBarBackground1,)\
	CR(TabsBarBackground2,)\
	CR(TabsBorderActive,)\
	CR(TabsBorderInactive,)\
	CR(TabsBackgroundActive,)\
	CR(TabsBackgroundInactive,)\
	CR(TabsBackgroundInactiveHover,)\
	CR(TabsBackgroundSecondWindow,)\
	CR(TabsTextActive,)\
	CR(TabsTextInactive,)\
	CR(TabsCloseHover,)\
	CR(TabsBarArrow,)\
	CR(TabsBarArrowBackground,)\
	CR(TabsBarArrowBackgroundHover,)\
	CR(SliderPathBackground,)\
	CR(SliderPathBorder,)\
	CR(SliderBorder,)\
	CR(SliderBorderHover,)\
	CR(SliderBorderPushed,)\
	CR(SliderBackground,)\
	CR(SliderBackgroundHover,)\
	CR(SliderBackgroundPushed,)\
	CR(StylePreviewColor1,)\
	CR(StylePreviewColor2,)\
	//jeœli tu coœ dopiszesz, to musisz zmieniæ przy porównaniu (504 cpp) czy rozmiar tablicy kolorów jest w³aœciwy
DECLARE_ENUM(COLOR,CLR)

class config
{
private:
	static const unsigned int configSize = EditboxTagButton10 + 1;
	wxString stringConfig[configSize];
	static const unsigned int colorsSize = StylePreviewColor2 + 1;
	wxColour colors[colorsSize];
	

    public:
	std::vector<Styles*> assstore;
    wxString progname;
	//aktualny katalog --- œcie¿ka do folderu programu
    wxString actualStyleDir, pathfull;
    wxArrayString dirs;
	bool AudioOpts;


    const wxString &GetString(CONFIG opt);
    bool GetBool(CONFIG opt);
    const wxColour &GetColour(COLOR opt);
	AssColor GetColor(COLOR opt);
    int GetInt(CONFIG opt);
    float GetFloat(CONFIG opt);
	void GetTable(CONFIG opt, wxArrayString &tbl, wxString split="|", int mode = 4);
	void GetIntTable(CONFIG opt, wxArrayInt &tbl, wxString split="|", int mode = 4);
	void GetCoords(CONFIG opt, int* coordx, int* coordy);

    void SetString(CONFIG opt, const wxString &sopt);
    void SetBool(CONFIG opt, bool bopt);
    void SetColour(COLOR opt, wxColour &copt);
	void SetColor(COLOR opt, AssColor &copt);
    void SetInt(CONFIG opt, int iopt);
    void SetFloat(CONFIG opt, float fopt);
	void SetTable(CONFIG opt, wxArrayString &iopt,wxString split="|");
	void SetIntTable(CONFIG opt, wxArrayInt &iopt,wxString split="|");
	void SetCoords(CONFIG opt, int coordx, int coordy);
    void GetRawOptions(wxString &options, bool Audio=false);
    void AddStyle(Styles *styl);
    void ChangeStyle(Styles *styl,int i);
    Styles *GetStyle(int i,const wxString &name=_T(""), Styles* styl=NULL);
    int FindStyle(const wxString &name, int *multiplication=NULL);
    void DelStyle(int i);
    int StoreSize();
    void CatchValsLabs(const wxString &rawoptions);
    bool SetRawOptions(const wxString &textconfig);
    int LoadOptions();
	void LoadColors(const wxString &themeName="");
	void LoadDefaultConfig();
	void LoadDefaultColors(bool dark = true, wxColour *table = NULL);
	void LoadDefaultAudioConfig();
	void LoadMissingColours(const wxString &path);
	bool LoadAudioOpts();
	void SaveAudioOpts();
    void SaveOptions(bool cfg=true, bool style=true);
	void SaveColors(const wxString &path="");
    void LoadStyles(const wxString &katalog);
    void clearstyles();
	void Sortstyles();
	void SetHexColor(const wxString &nameAndColor);
	//wxString GetStringColor(unsigned int);
	wxString GetStringColor(size_t optionName);
	wxString GetReleaseDate();
    config();
    ~config();
	wxColour defaultColour;
};
bool sortfunc(Styles *styl1,Styles *styl2);
//formatowanie w tym przypadku wygl¹da tak, 
//liczba która mówi ile cyfr przed przecinkiem i ile po, np 5.3f;
wxString getfloat(float num, const wxString &format="5.3f", bool Truncate=true);
wxBitmap CreateBitmapFromPngResource(const wxString& t_name);
wxBitmap *CreateBitmapPointerFromPngResource(const wxString& t_name);
wxImage CreateImageFromPngResource(const wxString& t_name);
#define wxBITMAP_PNG(x) CreateBitmapFromPngResource(x)
#define PTR_BITMAP_PNG(x) CreateBitmapPointerFromPngResource(x)
void MoveToMousePosition(wxWindow *win);
wxString MakePolishPlural(int num, const wxString &normal, const wxString &plural24, const wxString &pluralRest);
BOOL __stdcall MonitorEnumProc1(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData);
wxRect GetMonitorRect(int wmonitor, std::vector<tagRECT> *MonitorRects, const wxPoint &position, bool workArea);
static const wxString emptyString;
bool IsNumber(const wxString &txt);

#ifdef _M_IX86
void SetThreadName(DWORD id, const char *name);
#else
void SetThreadName(size_t id, const char *name);
#endif

enum{
	ASS=1,
	SRT,
	TMP,
	MDVD,
	MPL2,
	FRAME=10
};

extern config Options;

