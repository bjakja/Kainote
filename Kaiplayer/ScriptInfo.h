#ifndef SCRIPTINFO_H
#define SCRIPTINFO_H


#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/statbox.h>
#include <wx/choice.h>
#include <wx/button.h>
#include <wx/dialog.h>
#include "NumCtrl.h"


class ScriptInfo: public wxDialog
{
	public:

		ScriptInfo(wxWindow* parent,int w, int h);
		virtual ~ScriptInfo();

		NumCtrl* height;
		wxTextCtrl* script;
		NumCtrl* width;
		wxTextCtrl* update;
		wxButton* Button1;
		wxChoice* wrapstyle;
		wxCheckBox* CheckBox2;
		wxChoice* collision;
		wxTextCtrl* editing;
		wxTextCtrl* title;
		wxButton* Button2;
		wxCheckBox* CheckBox1;
		wxTextCtrl* timing;
		wxTextCtrl* translation;
		wxButton* Fvideo;

		void DoTooltips();
private:
		wxSize res;
		void OnVideoRes(wxCommandEvent& event);
};

#endif
