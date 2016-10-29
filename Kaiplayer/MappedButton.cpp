
#include "MappedButton.h"
#include "Hotkeys.h"

//w tooltipach nie nale¿y ustawiaæ () bo zostan¹ usuniête
MappedButton::MappedButton(wxWindow *parent, int id, const wxString& label, const wxString& toolTip,
             const wxPoint& pos, const wxSize& size, long style)
			 :wxButton(parent, id, label, pos, size, style)
{
	Bind(wxEVT_LEFT_UP, &MappedButton::OnLeftClick, this);
	SetToolTip(toolTip);
}

MappedButton::~MappedButton()
{
}

void MappedButton::SetToolTip(const wxString &_toolTip)
{
	wxString key = Hkeys.GetMenuH(GetId());
	wxString toolTip = (_toolTip=="")? GetToolTipText().BeforeFirst('(').Trim() : _toolTip;
	if(key!="")
	{
		toolTip = toolTip + " ("+key+")";
	}
	wxButton::SetToolTip(toolTip);
}
	
void MappedButton::OnLeftClick(wxMouseEvent &evt)
{
	if(evt.ShiftDown()){
		wxString wins[1]={"Globalny"};
		//upewnij siê, ¿e da siê zmieniæ idy na nazwy, 
		//mo¿e i trochê spowolni operacjê ale skoñczy siê ci¹g³e wywalanie hotkeysów
		//mo¿e od razu funkcji onmaphotkey przekazaæ item by zrobi³a co trzeba
		wxString buttonName = (GetLabelText()!="")? GetLabelText() : GetToolTipText().BeforeFirst('(').Trim();
		Hkeys.OnMapHkey( GetId(), _("Przycisk ") + GetLabelText(), this, wins, 1);
		SetToolTip();
		Hkeys.SetAccels(true);
		Hkeys.SaveHkeys();
		
		return;
	}
	evt.Skip();
}