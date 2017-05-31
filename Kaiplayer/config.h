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

#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED

//#pragma once
#include <wx/textfile.h>
#include <wx/tokenzr.h>
#include <wx/colour.h>
#include <wx/image.h>
#include <map>
#include <vector>
#include <algorithm>
#include "Styles.h"
#include <wx/utils.h> 
#include "EnumFactory.h"
//#include "kainoteApp.h"
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
#define CFG(CG) \
	CG(AudioAutoCommit,)\
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
	CG(AudioStartDragSensitivity,)\
	CG(AudioVerticalZoom,)\
	CG(AudioVolume,)\
	CG(AudioWheelDefaultToZoom,)\
	CG(AcceptedAudioStream,)\
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
	CG(DictionaryLanguage,)\
	CG(DisableLiveVideoEditing,)\
	CG(DontAskForBadResolution,)\
	CG(EditboxSugestionsOnDoubleClick,)\
	CG(EditorOn,)\
	CG(FindRecent,)\
	CG(FontCollectorAction,)\
	CG(FontCollectorDirectory,)\
	CG(FontCollectorFromMKV,)\
	CG(FontCollectorUseSubsDirectory,)\
	CG(FFMS2VideoSeeking,)\
	CG(GridChangeActiveOnSelection,)\
	CG(GridFontName,)\
	CG(GridFontSize,)\
	CG(GridHideCollums,)\
	CG(GridHideTags,)\
	CG(GridLoadSortedSubs,)\
	CG(GridSaveAfterCharacterCount,)\
	CG(GridTagsSwapChar,)\
	CG(InsertEndOffset,)\
	CG(InsertStartOffset,)\
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
	CG(SelectVisibleLineAfterFullscreen,)\
	CG(SpellcheckerOn,)\
	CG(StyleManagerPosition,)\
	CG(StyleManagerDetachEditor,)\
	CG(SubsAutonaming,)\
	CG(SubsRecent,)\
	CG(TlModeShowOriginal,)\
	CG(ToolbarIDs,)\
	CG(VideoFullskreenOnStart,)\
	CG(VideoIndex,)\
	CG(VideoPauseOnClick,)\
	CG(VideoProgressBar,)\
	CG(VideoRecent,)\
	CG(VideoVolume,)\
	CG(VideoWindowSize,)\
	CG(WindowMaximized,)\
	CG(WindowPosition,)\
	CG(WindowSize,)\
	CG(EditboxTagButtons,=3999)\
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
	
DECLARE_ENUM(CONFIG,CFG)

#define CLR(CR) \
	CR(WindowBackground,)\
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
	CR(GridSelectedDialogue,)\
	CR(GridSelectedComment,)\
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
	CR(GridComparison,)\
	CR(GridComparisonBackground,)\
	CR(GridComparisonBackgroundSelected,)\
	CR(GridComparisonCommentBackground,)\
	CR(GridComparisonCommentBackgroundSelected,)\
	CR(EditorText,)\
	CR(EditorTagNames,)\
	CR(EditorTagValues,)\
	CR(EditorCurlyBraces,)\
	CR(EditorTagOperators,)\
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

DECLARE_ENUM(COLOR,CLR)

class config
{
    private:
    std::map<CONFIG, wxString> rawcfg;
	std::map<COLOR, wxColour*> colors;
   

    public:
	std::vector<Styles*> assstore;
    wxString progname;
	//aktualny katalog --- œcie¿ka do folderu programu
    wxString acdir, pathfull;
    wxArrayString dirs;
	bool AudioOpts;


    wxString GetString(CONFIG opt);
    bool GetBool(CONFIG opt);
    wxColour &GetColour(COLOR opt);
	AssColor GetColor(COLOR opt);
    int GetInt(CONFIG opt);
    float GetFloat(CONFIG opt);
	void GetTable(CONFIG opt, wxArrayString &tbl, wxString split="|");
	void GetIntTable(CONFIG opt, wxArrayInt &tbl, wxString split="|");
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
    wxString GetRawOptions(bool Audio=false);
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
	void LoadDefaultColors(bool dark=true);
	void LoadDefaultAudioConfig();
	bool LoadAudioOpts();
	void SaveAudioOpts();
    void SaveOptions(bool cfg=true, bool style=true);
	void SaveColors(const wxString &path="");
    void LoadStyles(wxString katalog);
    void clearstyles();
	void ClearColors();
	void Sortstyles();
	void SetHexColor(const wxString &nameAndColor);
	wxString GetStringColor(std::map<COLOR, wxColour*>::iterator it);
	wxString GetStringColor(COLOR optionName);
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

enum{
	ASS=1,
	SRT,
	TMP,
	MDVD,
	MPL2,
	FRAME=10
};

extern config Options;

#endif // CONFIG_H_INCLUDED
