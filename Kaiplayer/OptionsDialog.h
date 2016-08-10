#ifndef OPTIONSDIALOG
#define OPTIONSDIALOG

#include <wx/wx.h>
#include <wx/treebook.h>
#include <wx/listctrl.h>
#include "ColorPicker.h"
#include <vector>
class kainoteFrame;

class ColorButton :public wxButton
{
public:
	ColorButton(wxWindow *parent, int id, const wxColour &col, const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize);
	virtual ~ColorButton(){};
	wxColour GetColour();
private:
	void OnClick(wxCommandEvent &evt);
	DialogColorPicker *dcp;
};

class OptionsBind {
public:
	wxControl *ctrl;
	wxString option;
};


class OptionsDialog : public wxDialog
	{
	public:
	OptionsDialog(wxWindow *parent, kainoteFrame *kaiparent);
	virtual ~OptionsDialog();
	wxTreebook *OptionsTree;
	wxListCtrl *Shortcuts;
	wxChoice* Stylelist;
	wxChoice* Katlist;
	wxButton *okok;

	private:
		std::vector<OptionsBind> handles;

		void ConOpt(wxControl *ctrl,wxString option);
		void OnSaveClick(wxCommandEvent& event);
		void SetOptions(bool saveall=true);
		void OnMapHkey(wxListEvent& event);
		void OnResetHkey(wxListEvent& event);
		//void OnKeyPress(wxKeyEvent& event);
		void OnChangeCatalog(wxCommandEvent& event);
	
	kainoteFrame *Kai;

	unsigned char hkeymodif;
	};

enum{
	ID_BOK=12233,
	ID_BCOMMIT
	};



#endif