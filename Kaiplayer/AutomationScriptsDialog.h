#ifndef SCRIPTSDIALOG
#define SCRIPTSDIALOG




#include <wx/wx.h>
#include <wx/listctrl.h>
#include <vector>

class kainoteFrame;

class MacrosDialog : public wxDialog
	{
	public:
		MacrosDialog(wxWindow *parent, int script);
		virtual ~MacrosDialog();

	private:
		//wxListCtrl *MacrosList;
		int script,seld;
		bool block;
		wxArrayString names;
		std::vector<bool> actives;
		int height;

		void OnMacro();
		void OnMouseEvents(wxMouseEvent &event);
		void OnPaint(wxPaintEvent &event);
		DECLARE_EVENT_TABLE()
	};


class ScriptsDialog : public wxDialog
	{

public:
	ScriptsDialog(kainoteFrame *Kai);
	virtual ~ScriptsDialog();
	void AddFromSubs();

	wxListCtrl *ScriptsList;

	
private:
	
	kainoteFrame *Kai;

	void OnShowMacros(wxListEvent &event);
	void OnAdd(wxCommandEvent &event);
	void OnDelete(wxCommandEvent &event);
	void OnEdit(wxCommandEvent &event);
	void OnReload(wxCommandEvent &event);
	void OnRescan(wxCommandEvent &event);
	};

enum{
	ID_SLIST=29765,
	ID_MLIST,
	ID_BADD,
	ID_BREMOVE,
	ID_BEDIT,
	ID_BRELOAD,
	ID_BRESCAN
	};

#endif