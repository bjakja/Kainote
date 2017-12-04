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

#include <map>
#include "ListControls.h"
#include "KaiDialog.h"
#include "EnumFactory.h"

//Po zmianie PlayPause nale¿y go zmieniæ w dshowrenderer by pauzowanie po odtwarzaniu linii zadzia³a³o
#define IDS(XX) \
	XX( AudioCommitAlt,=620 ) \
	XX( AudioPlayAlt, ) \
	XX( AudioPreviousAlt, ) \
	XX( AudioNextAlt, ) \
	XX( AudioCommit,=1620 ) \
	XX( AudioPlay, ) \
	XX( AudioPrevious, ) \
	XX( AudioNext, ) \
	XX( AudioStop, ) \
	XX( AudioPlayBeforeMark, ) \
	XX( AudioPlayAfterMark, ) \
	XX( AudioPlay500MSBefore, ) \
	XX( AudioPlay500MSAfter, ) \
	XX( AudioPlay500MSFirst, ) \
	XX( AudioPlay500MSLast, ) \
	XX( AudioPlayToEnd, ) \
	XX( AudioGoto, ) \
	XX( AudioLeadin, ) \
	XX( AudioLeadout, ) \
	XX( PlayPause,=2021)\
	XX( StopPlayback,) \
	XX( Plus5Second,)\
	XX( Minus5Second,)\
	XX( MinusMinute, ) \
	XX( PlusMinute, ) \
	XX( VolumePlus, ) \
	XX( VolumeMinus, ) \
	XX( PreviousVideo, ) \
	XX( NextVideo, ) \
	XX( PreviousChapter, ) \
	XX( NextChapter, ) \
	XX( FullScreen, ) \
	XX( HideProgressBar, ) \
	XX( DeleteVideo, ) \
	XX( AspectRatio, ) \
	XX( CopyCoords, ) \
	XX( FrameToPNG, ) \
	XX( FrameToClipboard, ) \
	XX( SubbedFrameToPNG, ) \
	XX( SubbedFrameToClipboard, ) \
	XX( PutBold,=4100) \
	XX( PutItalic, ) \
	XX( SplitLine, ) \
	XX( StartDifference, ) \
	XX( EndDifference, ) \
	XX( FindNextDoubtful, ) \
	XX( FindNextUntranslated, ) \
	XX( SetDoubtful, ) \
	XX( InsertBefore,=5555 ) \
	XX( InsertAfter, ) \
	XX( InsertBeforeVideo, ) \
	XX( InsertAfterVideo, ) \
	XX( InsertBeforeWithVideoFrame, ) \
	XX( InsertAfterWithVideoFrame, ) \
	XX( Swap, ) \
	XX( Duplicate, ) \
	XX( Join, ) \
	XX( JoinToFirst, ) \
	XX( JoinToLast, ) \
	XX( Copy, ) \
	XX( Paste, ) \
	XX( Cut, ) \
	XX( ShowPreview, ) \
	XX( HideSelected, ) \
	XX( FilterByNothing, ) \
	XX( FilterByStyles, ) \
	XX( FilterBySelections, ) \
	XX( FilterByDialogues, ) \
	XX( FilterByDoubtful, ) \
	XX( FilterByUntranslated, ) \
	XX( PasteTranslation, ) \
	XX( TranslationDialog, ) \
	XX( SubsFromMKV, ) \
	XX( ContinousPrevious, ) \
	XX( ContinousNext, ) \
	XX( PasteCollumns, ) \
	XX( CopyCollumns, ) \
	XX( FPSFromVideo, ) \
	XX( NewFPS, ) \
	XX( SaveSubs,=6677)\
	XX( SaveAllSubs,)\
	XX( SaveSubsAs,)\
	XX( SaveTranslation,)\
	XX( RemoveSubs,)\
	XX( Search,)\
	XX( FindReplaceDialog,)\
	XX( SelectLinesDialog,)\
	XX( SpellcheckerDialog,)\
	XX( VideoIndexing,)\
	XX( OpenAudio,)\
	XX( AudioFromVideo,)\
	XX( CloseAudio,)\
	XX( ASSProperties,)\
	XX( StyleManager,)\
	XX( SubsResample,)\
	XX( FontCollectorID,)\
	XX( ConvertToASS,)\
	XX( ConvertToSRT,)\
	XX( ConvertToTMP,)\
	XX( ConvertToMDVD,)\
	XX( ConvertToMPL2,)\
	XX( HideTags,)\
	XX( ChangeTime,)\
	XX( ViewAll,)\
	XX( ViewAudio,)\
	XX( ViewVideo,)\
	XX( ViewSubs,)\
	XX( AutoLoadScript,)\
	XX( AutoReloadAutoload,)\
	XX( LoadLastScript,)\
	XX( PlayPauseG,)\
	XX( PreviousFrame,)\
	XX( NextFrame,)\
	XX( VideoZoom,)\
	XX( SetStartTime,)\
	XX( SetEndTime,)\
	XX( SetVideoAtStart,)\
	XX( SetVideoAtEnd,)\
	XX( GoToNextKeyframe,)\
	XX( GoToPrewKeyframe,)\
	XX( Redo,)\
	XX( Undo,)\
	XX( UndoToLastSave,)\
	XX( History,)\
	XX( OpenSubs,=6800)\
	XX( OpenVideo,)\
	XX( Settings,)\
	XX( Quit,)\
	XX( Editor,)\
	XX( About,)\
	XX( Helpers,)\
	XX( Help,)\
	XX( ANSI,)\
	XX( PreviousLine,=6850)\
	XX( NextLine,)\
	XX( JoinWithPrevious,)\
	XX( JoinWithNext,)\
	XX( NextTab,)\
	XX( PreviousTab,)\
	XX( Remove, ) \
	XX( RemoveText, ) \
	XX( SnapWithStart,)\
	XX( SnapWithEnd,)\
	XX( Plus5SecondG,)\
	XX( Minus5SecondG,)\
	XX( SortLines,)\
	XX( SortSelected,)\
	XX( RecentAudio,)\
	XX( RecentVideo,)\
	XX( RecentSubs,)\
	XX( SelectFromVideo,)\
	XX( PlayActualLine,)\
//
DECLARE_ENUM(Id,IDS)


enum{
	GLOBAL_HOTKEY=0,
	GRID_HOTKEY,
	EDITBOX_HOTKEY,
	VIDEO_HOTKEY,
	AUDIO_HOTKEY
};
class idAndType{
public:
	idAndType(int _id=0, char _type=GLOBAL_HOTKEY){id= _id; Type= _type;}
	bool operator < (const idAndType match){ return id < match.id;};
	bool operator > (const idAndType match){ return id > match.id;};
	bool operator <= (const idAndType match){ return id <= match.id;};
	bool operator >= (const idAndType match){ return id >= match.id;};
	bool operator == (const idAndType match){ return id == match.id;};
	bool operator != (const idAndType match){ return id != match.id;};
	int id;
	char Type;
};

bool operator < (const idAndType match, const idAndType match1);
bool operator > (const idAndType match, const idAndType match1);
bool operator <= (const idAndType match, const idAndType match1);
bool operator >= (const idAndType match, const idAndType match1);
bool operator == (const idAndType match, const idAndType match1);
bool operator != (const idAndType match, const idAndType match1);
bool operator == (const idAndType &match, const int match1);
bool operator == (const int match1 ,const idAndType &match);
bool operator >= (const idAndType &match, const int match1);
bool operator >= ( const int match1, const idAndType &match);
bool operator <= (const idAndType &match, const int match1);
bool operator <= (const int match1 ,const idAndType &match);
bool operator > (const idAndType &match, const int match1);
bool operator > ( const int match1, const idAndType &match);
bool operator < (const idAndType &match, const int match1);
bool operator < (const int match1 ,const idAndType &match);
bool operator != (const idAndType &match, const int match1);
bool operator != ( const int match1, const idAndType &match);

class hdata{
public:
	hdata(const wxString &accName, const wxString &_Accel){
		Name=accName; Accel=_Accel;
	}
	hdata(const wxString &acc){
		Accel=acc;
	}
	hdata(){}
	wxString Name;
	wxString Accel;
};

class HkeysDialog : public KaiDialog
{
	public:
	HkeysDialog(wxWindow *parent, wxString name, char hotkeyWindow = GLOBAL_HOTKEY, bool showWindowSelection=true);
	virtual ~HkeysDialog();
	wxString hotkey;
	wxString hkname;
	KaiChoice *global;
	char type;

	private:
	void OnKeyPress(wxKeyEvent& event);
};

class Hotkeys
{
private:

public:
	Hotkeys();
	~Hotkeys();
	int LoadHkeys(bool Audio=false);
	void LoadDefault(std::map<idAndType, hdata> &_hkeys, bool Audio=false);
	void SaveHkeys(bool Audio=false);
	void SetHKey(const idAndType &itype, wxString name, wxString hotkey);
	wxAcceleratorEntry GetHKey(const idAndType itype, const hdata *hkey=0);
	wxString GetMenuH(const idAndType &itype, const wxString &name="");
	void FillTable();
	void ResetKey(const idAndType *itype, int id=0, char type=GLOBAL_HOTKEY);
	wxString GetDefaultKey(const idAndType &itype);
	int OnMapHkey(int id, wxString name,wxWindow *parent, char hotkeyWindow = GLOBAL_HOTKEY, bool showWindowSelection=true);
	int OnMapHkey(int *returnId, wxString name,wxWindow *parent);
	void SetAccels(bool all=false);
	wxString GetName(const idAndType itype);
	std::map<idAndType, hdata> hkeys;
	std::map<int, wxString> keys;
	bool AudioKeys;
	int lastScirptId;
};


extern Hotkeys Hkeys;
