#ifndef NEWCATALOG_H
#define NEWCATALOG_H

//(*Headers(NewCatalog)
#include <wx/textctrl.h>
#include <wx/statbox.h>
#include <wx/button.h>
#include <wx/dialog.h>
//*)

class NewCatalog: public wxDialog
{
	public:

		NewCatalog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~NewCatalog();

		
		wxButton* Button1;
		wxStaticBox* StaticBox1;
		wxButton* Button2;
		wxTextCtrl* TextCtrl1;
		

	protected:

		
		static const long ID_TEXTCTRL1;
		static const long ID_STATICBOX1;
		void OnCatalogCommit(wxCommandEvent& event);

	private:

		
};

#endif
