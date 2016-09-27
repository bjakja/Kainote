/***************************************************************
 * Name:      kainoteMain.h
 * Purpose:   Defines Application Frame
 * Author:    Bjakja (bjakja@op.pl)
 * Created:   2012-04-23
 * Copyright: Bjakja (www.costam.com)
 * License:
 **************************************************************/

#ifndef KAINOTEMAIN_H
#define KAINOTEMAIN_H

#pragma once

#include <wx/timer.h>
#include <wx/sizer.h>
#include <wx/menu.h>
#include <wx/filedlg.h>
#include <wx/frame.h>
#include <wx/statusbr.h>
#include "Tabs.h"
#include <vector>
#include "TabPanel.h"
#include "SpellChecker.h"
#include "AutomationScriptsDialog.h"
#include "StyleStore.h"
#include "FindReplace.h"
#include "Automation.h"
#include "Toolbar.h"
#include "EnumFactory.h"

class kainoteFrame: public wxFrame
{
    public:

        kainoteFrame(const wxPoint &pos, const wxSize &size);
        virtual ~kainoteFrame();

        wxString sftc();
        
		Notebook* Tabs;

		wxBoxSizer *mains;
        void Save(bool dial, int wtab=-1);
		void SaveAll();

        bool OpenFile(wxString filename,bool fulls=false);
		void Label(int iter=0,bool video=false, int wtab=-1);
		
		void SetRecent(short what=0);
		void AppendRecent(short what=0,wxMenu *Menu=0);
		void SetAccels(bool all=true);
		wxString FindFile(wxString fn,bool video,bool prompt=true);

		wxMenu* VidsRecMenu;
		wxMenu* SubsRecMenu;
		wxMenu* AudsRecMenu;
		wxMenuBar* MenuBar;

		wxStatusBar* StatusBar1;
		KaiToolbar *Toolbar;
		wxArrayString subsrec;
		wxArrayString videorec;
		wxArrayString audsrec;
		void OnMenuSelected(wxCommandEvent& event);
		void OnMenuSelected1(wxCommandEvent& event);
		void OnRecent(wxCommandEvent& event);
		stylestore *ss;

		TabPanel* GetTab();
		
		void InsertTab(bool sel=true);
		void OpenFiles(wxArrayString,bool intab=false, bool nofreeze=false, bool newtab=false);
		void OpenAudioInTab(TabPanel *pan, int id, const wxString &path);
		void HideEditor();
		bool SavePrompt(char mode=1, int wtab=-1);
		void UpdateToolbar();
		void OnOpenAudio(wxCommandEvent& event);
		void OnMenuClick(wxCommandEvent &event);
		ScriptsDialog *LSD;
		findreplace *FR;
		findreplace *SL;
		Auto::Automation *Auto;
    private:

		
        void OnConversion(char form);
        void OnAssProps();
		void OnMenuOpened(wxMenuEvent& event);
		void OnP5Sec(wxCommandEvent& event);
		void OnM5Sec(wxCommandEvent& event);
		void OnAudioSnap(wxCommandEvent& event);
		void OnPageChanged(wxCommandEvent& event);
		void OnPageChange(wxCommandEvent& event);
		void OnPageAdd(wxCommandEvent& event);
		void OnPageClose(wxCommandEvent& event);
		//void OnRunScript(wxCommandEvent& event);
		
		void OnChangeLine(wxCommandEvent& event);
		void OnDelete(wxCommandEvent& event);
		void OnClose1(wxCloseEvent& event);
		void AppendBitmap(wxMenu *menu, int id, wxString text, wxString help, wxBitmap bitmap,bool enable=true, wxMenu *SubMenu=0);
		static void OnOutofMemory();
       
        wxMenu* ConvMenu;
        wxMenu* FileMenu;
		wxMenu* HelpMenu;
        wxMenu* SubsMenu;
		wxMenu* EditMenu;
		wxMenu* VidMenu;
		wxMenu* AudMenu;
		wxMenu* ViewMenu;
		wxMenu* AutoMenu;
		
		wxLogWindow *mylog;
		/*int fontlastmodif;
		int fontlastmodifl;*/
		
};




enum{
	ID_ADDPAGE=6900,
	ID_CLOSEPAGE,
	ID_TABS,
	ID_STATUSBAR1,
	ID_CONV
};
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
	XX( Stop,=2020) \
	XX( PlayPause,)\
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
	XX( InsertBefore,=5555 ) \
	XX( InsertAfter, ) \
	XX( InsertBeforeVideo, ) \
	XX( InsertAfterVideo, ) \
	XX( Swap, ) \
	XX( Duplicate, ) \
	XX( Join, ) \
	XX( JoinToFirst, ) \
	XX( JoinToLast, ) \
	XX( Copy, ) \
	XX( Paste, ) \
	XX( Cut, ) \
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
	XX( FindReplace,)\
	XX( SelectLines,)\
	XX( VideoIndexing,)\
	XX( OpenAudio,)\
	XX( AudioFromVideo,)\
	XX( CloseAudio,)\
	XX( ASSProperties,)\
	XX( StyleManager,)\
	XX( FontCollector,)\
	XX( CrossPositioner,)\
	XX( Positioner,)\
	XX( Movement,)\
	XX( Scalling,)\
	XX( RotatingZ,)\
	XX( RotatingXY,)\
	XX( RectangleClips,)\
	XX( VectorClips,)\
	XX( VectorDrawings,)\
	XX( MoveAll,)\
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
	XX( PlayPauseG,)\
	XX( PreviousFrame,)\
	XX( NextFrame,)\
	XX( SetStartTime,)\
	XX( SetEndTime,)\
	XX( SetVideoAtStart,)\
	XX( SetVideoAtEnd,)\
	XX( Redo,)\
	XX( Undo,)\
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
//
DECLARE_ENUM(Id,IDS)

#endif // KAINOTEMAIN_H
