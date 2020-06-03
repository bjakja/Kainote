//  Copyright (c) 2016 - 2020, Marcin Drob

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

#include "TimeCtrl.h"
#include "SubsTime.h"
#include "NumCtrl.h"
#include "MappedButton.h"
#include "ListControls.h"
#include "KaiRadioButton.h"
#include <wx/wx.h>
#include <wx/collpane.h>
#include "KaiStaticBoxSizer.h"

class KainoteFrame;



class ShiftTimesWindow: public wxWindow
{
public:
	
	ShiftTimesWindow(wxWindow* parent, KainoteFrame* kfparent, wxWindowID id = -1, 
		const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style=0);
	virtual ~ShiftTimesWindow();
	KaiRadioButton* StartVAtime;
	KaiRadioButton* EndVAtime;
	KaiChoice *WhichLines;
	KaiChoice *WhichTimes;
	KaiChoice *ProfilesList;
	KaiRadioButton* Forward;
	KaiRadioButton* Backward;
	KaiCheckBox* DisplayFrames;
	KaiCheckBox* MoveTagTimes;

	MappedButton* AddStyles;
	MappedButton* MoveTime;
	MappedButton* NewProfile;
	MappedButton* RemoveProfile;
	TimeCtrl* TimeText;
	KaiTextCtrl* Stylestext;
	KaiCheckBox* MoveToVideoTime;
	KaiCheckBox* MoveToAudioTime;
	KaiChoice* EndTimeCorrection;
	//postprocessor controls
	KaiCheckBox* LeadIn;
	KaiCheckBox* LeadOut;
	KaiCheckBox* Continous;
	KaiCheckBox* SnapKF;
	NumCtrl* LITime;
	NumCtrl* LOTime;
	NumCtrl* ThresStart;
	NumCtrl* ThresEnd;
	NumCtrl* BeforeStart;
	NumCtrl* AfterStart;
	NumCtrl* BeforeEnd;
	NumCtrl* AfterEnd;

	KaiStaticBoxSizer *liosizer;
	KaiStaticBoxSizer *consizer;
	KaiStaticBoxSizer *snapsizer;
	KaiStaticBoxSizer *profileSizer;
	KaiScrollbar *scroll;
	wxWindow *panel; 

	void Contents(bool addopts = true);
	void RefVals(ShiftTimesWindow *from = NULL);
	void OnOKClick(wxCommandEvent& event);
	wxBoxSizer *Main;
	bool SetBackgroundColour(const wxColour &col);
	bool SetForegroundColour(const wxColour &col);
	bool SetFont(const wxFont &font);

private:

	char form;
	KainoteFrame* Kai;
	bool isscrollbar;
	int scPos;
	MappedButton *coll;

	void OnAddStyles(wxCommandEvent& event);
	void OnChangeDisplayUnits(wxCommandEvent& event);
	void OnAddProfile(wxCommandEvent& event);
	void OnRemoveProfile(wxCommandEvent& event);
	void OnChangeProfile(wxCommandEvent& event);
	void OnEdition(wxCommandEvent& event);
	void ChangeDisplayUnits(bool times);
	void OnSize(wxSizeEvent& event);
	void OnScroll(wxScrollEvent& event);
	void OnMouseScroll(wxMouseEvent& event);
	void AudioVideoTime(wxCommandEvent &event);
	void CollapsePane(wxCommandEvent &event);
	void DoTooltips(bool normal = true);
	void SaveOptions();
	void CreateControls(bool normal = true);
	void GetProfilesNames(wxArrayString &list);
	void CreateProfile(const wxString &name, bool overwrite = false);
	void SetProfile(const wxString &name);

	DECLARE_EVENT_TABLE()
};


enum{
	ID_RADIOBUTTON1 = 11124, 
	ID_RADIOBUTTON2,
	ID_BSTYLE,
	ID_CLOSE,
	ID_SCROLL,
	ID_VIDEO,
	ID_AUDIO
};

