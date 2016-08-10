
#include "Visuals.h"
#include "TabPanel.h"

Position::Position()
	: Visuals()
{
}

void Position::DrawVisual(int time)
{
	DrawCross(from);
	DrawRect(from);
}

void Position::OnMouseEvent(wxMouseEvent &evt)
{
	
	if(blockevents){return;}
	bool click = evt.LeftDown()||evt.RightDown()||evt.MiddleDown();
	bool holding = (evt.LeftIsDown()||evt.RightIsDown()||evt.MiddleIsDown());
	
	int x, y;
	if(tab->Video->isfullskreen){wxGetMousePosition(&x,&y);}
	else{evt.GetPosition(&x,&y);}
	
	if(evt.ButtonUp()){
		if(tab->Video->HasCapture()){tab->Video->ReleaseMouse();}
		tab->Edit->SetVisual(GetVisual(),false,0);
		if(!hasArrow){tab->Video->SetCursor(wxCURSOR_ARROW);hasArrow=true;}
	}

	if(click){
		tab->Video->SetCursor(wxCURSOR_SIZING);
		hasArrow=false;
		firstmove.x=x;
		firstmove.y=y;
		lastmove=from;
	}else if(holding){
		from.x = lastmove.x-(firstmove.x-x);
		from.y = lastmove.y-(firstmove.y-y);
		tab->Edit->SetVisual(GetVisual(),true,0);
	}
}
	
wxString Position::GetVisual()
{
	return "\\pos("+getfloat(from.x*wspw)+","+getfloat(from.y*wsph)+")";
}
	
void Position::SetCurVisual()
{
	D3DXVECTOR2 linepos = tab->Edit->GetPosnScale(NULL, NULL, tbl);
	from = D3DXVECTOR2(linepos.x/wspw,linepos.y/wsph);
	
}