#include "Visuals.h"
#include "TabPanel.h"

Move::Move()
	: Visuals()
	, moveStart(0)
	, moveEnd(0)
	, type(0)
	, grabbed(-1)
{
}

void Move::DrawVisual(int time)
{
	D3DXVECTOR2 v4[6];
	v4[0].x=from.x;
	v4[0].y=from.y;
	v4[1].x=to.x;
	v4[1].y=to.y;
	//drawarrow od razu przesuwa punkt tak by linia koñczy³a siê przed strza³k¹
	DrawArrow(from, &v4[1], 6);


	float tmpt=time-moveStart;
	float tmpt1=moveEnd-moveStart;
	float actime= tmpt/tmpt1;
	D3DXVECTOR2 dist;
	if(time < moveStart){dist.x=from.x, dist.y= from.y;}
	else if(time > moveEnd){dist.x=to.x, dist.y= to.y;}
	else {
		dist.x= from.x -((from.x-to.x)*actime); 
		dist.y = from.y -((from.y-to.y)*actime);
	}

	line->Begin();
	line->Draw(v4,2,0xFFFF0000);
	DrawCross(dist, 0xFFFF0000, false);
	line->End();

	DrawRect(from);
	DrawCircle(to);


}

wxString Move::GetVisual()
{
	return "\\move("+getfloat(from.x*wspw)+","+getfloat(from.y*wsph)+","+
		getfloat(to.x*wspw)+","+getfloat(to.y*wsph)+","+
		getfloat(tbl[4]-tab->Edit->line->Start.mstime)+","+getfloat(tbl[5]-tab->Edit->line->Start.mstime)+")";
}

void Move::OnMouseEvent(wxMouseEvent &evt)
{
	if(blockevents){return;}
	bool click = evt.LeftDown()||evt.RightDown()||evt.MiddleDown();
	bool holding = (evt.LeftIsDown()||evt.RightIsDown()||evt.MiddleIsDown());
	bool leftc = evt.LeftDown();
	bool rightc = evt.RightDown();

	int x, y;
	if(tab->Video->isfullskreen){wxGetMousePosition(&x,&y);}
	else{evt.GetPosition(&x,&y);}

	if(evt.ButtonUp()){
		if(tab->Video->HasCapture()){tab->Video->ReleaseMouse();}
		SetVisual(GetVisual(),false,type);
		if(!hasArrow){tab->Video->SetCursor(wxCURSOR_ARROW);hasArrow=true;}
		grabbed = -1;
	}

	if(click){
		tab->Video->CaptureMouse();
		tab->Video->SetCursor(wxCURSOR_SIZING );
		hasArrow=false;
		if(leftc){type=0;}
		if(rightc){type=1;}

		if(abs(to.x-x)<8 && abs(to.y-y)<8){
			grabbed=1;type=1;
			diffs.x=to.x-x;
			diffs.y=to.y-y;
		}else if(abs(from.x-x)<8 && abs(from.y-y)<8){
			grabbed=0;type=0;
			diffs.x=from.x-x;
			diffs.y=from.y-y;
		}else{
			grabbed= -1;
			if(type==1){
				to.x=x;
				to.y=y;
			}else{
				from.x=x;
				from.y=y;
			}
			diffs=wxPoint(0,0);
		}
		lastmove = to;
		firstmove = from;
		SetVisual(GetVisual(),true,type);

	}else if(holding){
		if(type==0){
			from.x=x+diffs.x;
			from.y=y+diffs.y;
		}else{
			to.x=x+diffs.x;
			to.y=y+diffs.y;
		}
		if(evt.ShiftDown()){
			if(type==0){
				if(abs(from.x - firstmove.x)<15){
					from.x = firstmove.x;
				}
				if(abs(from.y - firstmove.y)<15){
					from.y = firstmove.y;
				}
			}else{
				if(abs(to.x - lastmove.x)<15){
					to.x = lastmove.x;
				}
				if(abs(to.y - lastmove.y)<15){
					to.y = lastmove.y;
				}
			}
		}
		SetVisual(GetVisual(),true,type);
	}

}

void Move::SetCurVisual()
{
	D3DXVECTOR2 linepos = GetPosnScale(NULL, NULL, tbl);
	from = to = D3DXVECTOR2(linepos.x/wspw,linepos.y/wsph);

	if(tbl[6]>3){to.x=tbl[2]/wspw, to.y=tbl[3]/wsph;}
	moveStart=(int)tbl[4];
	if(tbl[4]>tbl[5]){tbl[5]=end;}
	moveEnd=(int)tbl[5];

}