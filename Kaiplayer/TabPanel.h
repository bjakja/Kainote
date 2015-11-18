#ifndef TABPANEL
#define TABPANEL

#pragma once

#include <wx/wx.h>
#include "Grid.h"
#include "Videobox.h"
#include "EditBox.h"
#include "ChangeTime.h"
#include <wx/statline.h>

class TabPanel : public wxWindow
	{
	public:
		TabPanel(wxWindow *parent,kainoteFrame *kai);
		virtual ~TabPanel();

		Grid* Grid1;
        EditBox* Edit;
		VideoCtrl* Video;
		CTwindow* CTime;

		wxBoxSizer* BoxSizer1;
		wxBoxSizer* BoxSizer2;
		wxBoxSizer* BoxSizer3;

		void SetAccels();

		bool edytor;
		

		wxString SubsName;
		wxString VideoName;
		wxString SubsPath;
		wxString VideoPath;

	private:
		wxDialog* sline;
		bool holding;
		void OnMouseEvent(wxMouseEvent& event);
		void OnFocus(wxChildFocusEvent& event);
		DECLARE_EVENT_TABLE()
	};



#endif