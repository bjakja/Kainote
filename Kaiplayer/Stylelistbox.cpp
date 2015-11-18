#include "Stylelistbox.h"

#include <wx/intl.h>
#include <wx/string.h>


Stylelistbox::Stylelistbox(wxWindow* parent, bool styles, wxString arr[ ], int count,const wxPoint& pos, int style)
	: wxDialog(parent, -1, (styles)?_("Wybór styli") : _("Wybór kolumn"))
{
	SetClientSize(wxSize(302,282));
	CheckListBox1 = new wxCheckListBox(this, -1, wxPoint(24,27), wxSize(248,208), count, arr, style);
	Button1 = new wxButton(this, wxID_OK, _("Ok"), wxPoint(24,248));
	Button2 = new wxButton(this, wxID_CANCEL, _("Anuluj"), wxPoint(112,248));
	StaticText1 = new wxStaticText(this, -1, (styles)?_("Wybierz style") : _("Wybierz kolumny"), wxPoint(24,8));
	CenterOnParent();
}

Stylelistbox::~Stylelistbox()
{
}

