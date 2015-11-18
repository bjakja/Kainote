#ifndef STYLELISTBOX_H
#define STYLELISTBOX_H

//(*Headers(Stylelistbox)
#include <wx/stattext.h>
#include <wx/checklst.h>
#include <wx/button.h>
#include <wx/dialog.h>
//*)

class Stylelistbox: public wxDialog
{
	public:

		Stylelistbox(wxWindow* parent, bool styles=true, wxString arr[ ]=0, int count=0, const wxPoint& pos=wxDefaultPosition, int style=0);
		virtual ~Stylelistbox();

		
		wxButton* Button1;
		wxCheckListBox* CheckListBox1;
		wxStaticText* StaticText1;
		wxButton* Button2;
		
};

#endif
