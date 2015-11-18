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
#include "TabDialog.h"
#include <vector>
#include "TabPanel.h"
#include "SpellChecker.h"
#include "AutomationScriptsDialog.h"
#include "stylestore.h"
#include "findreplace.h"
#include "Automation.h"
#include "Toolbar.h"

class kainoteFrame: public wxFrame
{
    public:

        kainoteFrame(wxWindow* parents);
        virtual ~kainoteFrame();

        wxString sftc();
        
		Notebook* Tabs;

		wxBoxSizer *mains;
        void Save(bool dial, int wtab=-1);
		void SaveAll();

        bool OpenFile(wxString filename,bool fulls=false);
		void Label(int iter=0,bool video=false, int wtab=-1);

        
        //void UpdateMenu();
		
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
		bool SpellcheckerOn();
		SpellChecker *SC;
		stylestore *ss;

		TabPanel* GetTab();
		
		void InsertTab(bool sel=true);
		void OpenFiles(wxArrayString,bool intab=false, bool nofreeze=false, bool newtab=false);
		void HideEditor();
		bool SavePrompt(char mode=1, int wtab=-1);
		void UpdateToolbar();
		void OnOpenAudio(wxCommandEvent& event);

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
		void OnSelVid(wxCommandEvent& event);
		void OnAudioSnap(wxCommandEvent& event);
		void OnPageChanged(wxCommandEvent& event);
		void OnPageChange(wxCommandEvent& event);
		void OnPageAdd(wxCommandEvent& event);
		void OnPageClose(wxCommandEvent& event);
		void OnRunScript(wxCommandEvent& event);
		void OnChangeLine(wxCommandEvent& event);
		void OnDelete(wxCommandEvent& event);
		void OnClose1(wxCloseEvent& event);
		void AppendBitmap(wxMenu *menu, int id, wxString text, wxString help, wxBitmap bitmap,bool enable=true, wxMenu *SubMenu=0, wxAcceleratorEntry *entry=0);
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

enum{// nowe idy dodawaj na dole, bo ludzie bêd¹ mieli niespodziankê przy odpaleniu toolbara.
	ID_SAVE=6677,//odt¹d disejblujemy
	ID_SAVEALL,
	ID_SAVEAS,
	ID_SAVETL,
	ID_UNSUBS,
	ID_FIND,
	ID_FINDREP,
	ID_SELLIN,
	ID_OPVIDEOINDEX,
	ID_OPAUDIO,
	ID_OPFROMVID,
	ID_CLOSEAUDIO,
	ID_ASSPROPS,
	ID_STYLEMNGR,
	ID_COLLECTOR,
	ID_MOVEMENT,
	ID_SCALE,
	ID_ROTATEZ,
	ID_ROTATEXY,
	ID_CLIPRECT,
	ID_FAXY,
	ID_CLIPS,
	ID_DRAWINGS,
	ID_ASS,
	ID_SRT,
	ID_TMP,
	ID_MDVD,
	ID_MPL2,
	ID_HIDETAGS,
	ID_CHANGETIME,
	ID_SORT,
	ID_VALL,
	ID_VAUDIO,
	ID_VVIDEO,
	ID_VSUBS,
	ID_RECAUDIO,
	ID_CONV,
	ID_AUTO,//Dot¹d disejblujemy
	ID_PAUSE,
	ID_PREVFRAME,
	ID_NEXTFRAME,
	ID_SETSTIME,
	ID_SETETIME,
	ID_SETVIDATSTART,
	ID_SETVIDATEND,
	ID_REDO1,
	ID_UNDO1,
	ID_SORTSEL,
	

	ID_OPENSUBS=6800,
	ID_OPVIDEO,
	ID_SETTINGS,
	ID_QUIT,
	ID_EDITOR,
	ID_ABOUT,
	ID_HELPERS,
	ID_SELONVID,
	ID_RECSUBS,
	ID_RECVIDEO,

	ID_PREV_LINE=6850,
	ID_NEXT_LINE,
	GRID_JOINWP,
	GRID_JOINWN,
	ID_NEXT_TAB,
	ID_PREV_TAB,
	ID_DELETE,
	ID_DELETE_TEXT,
	ID_SNAP_START,
	ID_SNAP_END,
	ID_P5SEC,
	ID_M5SEC,
	ID_ADDPAGE,
	ID_CLOSEPAGE,
	ID_TABS,
	ID_STATUSBAR1
};

#endif // KAINOTEMAIN_H
