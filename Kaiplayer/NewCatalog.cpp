#include "NewCatalog.h"

//(*InternalHeaders(NewCatalog)
#include <wx/intl.h>
#include <wx/string.h>
#include <wx/wx.h>
//*)

//(*IdInit(NewCatalog)
const long NewCatalog::ID_TEXTCTRL1 = wxNewId();
const long NewCatalog::ID_STATICBOX1 = wxNewId();
//*)

NewCatalog::NewCatalog(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	wxTextValidator valid(wxFILTER_EXCLUDE_CHAR_LIST);
	wxArrayString excludes;
	excludes.Add(_T("\\"));
	excludes.Add(_T("/"));
	excludes.Add(_T("*"));
	excludes.Add(_T("?"));
	excludes.Add(_T(":"));
	excludes.Add(_T("\""));
	excludes.Add(_T("<"));
	excludes.Add(_T(">"));
	excludes.Add(_T("|"));
	valid.SetExcludes(excludes);

	Create(parent, id, _("Wybór nazwy katalogu"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
	SetClientSize(wxSize(290,112));
	Move(wxDefaultPosition);
	TextCtrl1 = new wxTextCtrl(this, ID_TEXTCTRL1, wxEmptyString, wxPoint(24,35), wxSize(244,21), wxTE_PROCESS_ENTER, valid, _T("ID_TEXTCTRL1"));
	StaticBox1 = new wxStaticBox(this, ID_STATICBOX1, _("Podaj nazwê nowego katalogu"), wxPoint(8,8), wxSize(274,64), 0, _T("ID_STATICBOX1"));
	Button1 = new wxButton(this, wxID_OK, _("Utwórz"), wxPoint(24,83), wxDefaultSize, 0, wxDefaultValidator, _T("wxID_OK"));
	Button2 = new wxButton(this, wxID_CANCEL, _("Anuluj"), wxPoint(112,83), wxDefaultSize, 0, wxDefaultValidator, _T("wxID_CANCEL"));

	Connect(ID_TEXTCTRL1,wxEVT_COMMAND_TEXT_ENTER,(wxObjectEventFunction)&NewCatalog::OnCatalogCommit);
}

NewCatalog::~NewCatalog()
{
	//(*Destroy(NewCatalog)
	//*)
}

void NewCatalog::OnCatalogCommit(wxCommandEvent& event)
{
	EndModal(wxID_OK);
}
