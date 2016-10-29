#ifndef CTwindow_H
#define CTwindow_H
#include "TimeCtrl.h"
#include "SubsTime.h"
#include "NumCtrl.h"
#include "MappedButton.h"
#include <wx/wx.h>
#include <wx/collpane.h>

class kainoteFrame;



class CTwindow: public wxScrolled<wxWindow>
{
	public:

		CTwindow(wxWindow* parent,kainoteFrame* kfparent,wxWindowID id=-1,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize,long style=0);
		virtual ~CTwindow();
		wxRadioButton* StartVAtime;
		wxRadioButton* EndVAtime;
		wxChoice *WhichLines;
		wxChoice *WhichTimes;
		wxRadioButton* Forward;
		wxRadioButton* Backward;
		
		wxButton* AddStyles;
		MappedButton* MoveTime;
		TimeCtrl* TimeText;
		wxTextCtrl* Stylestext;
		wxCheckBox* videotime;
		wxCheckBox* audiotime;
		wxChoice* CorTime;
		wxCheckBox* LeadIn;
		wxCheckBox* LeadOut;
		wxCheckBox* Continous;
		wxCheckBox* SnapKF;
		NumCtrl* LITime;
		NumCtrl* LOTime;
		NumCtrl* ThresStart;
		NumCtrl* ThresEnd;
		NumCtrl* BeforeStart;
		NumCtrl* AfterStart;
		NumCtrl* BeforeEnd;
		NumCtrl* AfterEnd;
		wxStaticBoxSizer *liosizer;
		wxStaticBoxSizer *consizer;
		wxStaticBoxSizer *snapsizer;

		void Contents(bool addopts=true);
		void RefVals(CTwindow *from=NULL);

		wxBoxSizer *Main;

		
	private:

        char form;
		kainoteFrame* Kai;
		bool isscrollbar;
		int bestsize;
		wxButton *coll;
		
		void OnOKClick(wxCommandEvent& event);
		void OnAddStyles(wxCommandEvent& event);
		void OnSize(wxSizeEvent& event);
		void AudioVideoTime(wxCommandEvent &event);
        void CollapsePane(wxCommandEvent &event);
		void DoTooltips();
		
	
DECLARE_EVENT_TABLE()
};
/*
class mypanel: public wxPanel
{
public:
	mypanel(wxWindow* parent);
	virtual ~mypanel();
private:
	void OnMouseEvent(wxMouseEvent& event);

	CTwindow *CTwin;
	wxWindow *focused;
	DECLARE_EVENT_TABLE()
};
*/

enum{
	ID_RADIOBUTTON1=11134,
	ID_RADIOBUTTON2,
	ID_BSTYLE,
	ID_CLOSE,
	ID_SCROLL,
	ID_MOVE,
	ID_VIDEO,
	ID_AUDIO
};

#endif