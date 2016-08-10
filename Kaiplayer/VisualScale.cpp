#include "Visuals.h"
#include "TabPanel.h"

Scale::Scale()
	: Visuals()
	, type(0)
	, grabbed(-1)
{
}

void Scale::DrawVisual(int time)
{
	if(time != oldtime && tbl[6]>3){
		from=CalcMovePos();
		from.x/=wspw; from.y/=wsph;
		to=from;
	}

	D3DXVECTOR2 v4[15];
	int addy=(AN>3)?60 : -60, addx= (AN % 3 == 0)?-60 : 60;
	
	float movex=from.x+addx, movey=from.y+addy;
	
	if(type!=1){movex=to.x;}//strza³ka w poziomie i czêœæ strza³ki po skosie
	else{movex=from.x+(scale.x*addx);}
	if(type>0){movey=to.y;}//strza³ka w pionie i czêœæ strza³ki po skosie
	else{movey=from.y+(scale.y*addy);}
	if(movex==from.x){movex=from.x+addx;}
	else if(movey==from.y){movey=from.y+addy;}
	
	lastmove.x = movex;
	lastmove.y = movey;
	v4[0]=from;//strza³ka pozioma
	v4[1].x=movex;
	v4[1].y=from.y;//strza³ka pozioma
	v4[2]=from;//strza³ka skoœna
	v4[3].x=movex;
	v4[3].y=movey;//strza³ka skoœna
	v4[4]=from;//strza³ka pionowa
	v4[5].x=from.x;
	v4[5].y=movey;//strza³ka pionowa
	
	for(int i=1; i<6; i+=2){
		DrawArrow(v4[0], &v4[i]);
	}

	line->Begin();
	line->Draw(v4,2,0xFFFF0000);
	line->End();
	line->Begin();
	line->Draw(&v4[2],2,0xFFFF0000);
	line->End();
	line->Begin();
	line->Draw(&v4[4],2,0xFFFF0000);
	line->End();
	
}

wxString Scale::GetVisual()
{
	wxString result;
	if(to.x==from.x){to.x=from.x+60.f;}
	if(to.y==from.y){to.y=from.y+60.f;}
		
	if(type!=1){
		float res=(abs(to.x - from.x))/60.f;
		result += "\\fscx" + getfloat(res*100);
		scale.x=res;
	}if(type!=0){
		float res=(abs(to.y - from.y))/60.f;
		result += "\\fscy" + getfloat(res*100);
		scale.y=res;
	}

	return result;
}

void Scale::OnMouseEvent(wxMouseEvent &evt)
{
	if(blockevents){return;}
	bool click = evt.LeftDown()||evt.RightDown()||evt.MiddleDown();
	bool holding = (evt.LeftIsDown()||evt.RightIsDown()||evt.MiddleIsDown());
	bool leftc = evt.LeftDown();
	bool rightc = evt.RightDown();
	bool middlec = evt.MiddleDown();

	int x, y;
	if(tab->Video->isfullskreen){wxGetMousePosition(&x,&y);}
	else{evt.GetPosition(&x,&y);}

	if(evt.ButtonUp()){
		if(tab->Video->HasCapture()){tab->Video->ReleaseMouse();}
		tab->Edit->SetVisual(GetVisual(),false,type);
		if(!hasArrow){tab->Video->SetCursor(wxCURSOR_ARROW);hasArrow=true;}
	}

	if(!holding){
		if(abs(lastmove.x-x)<8 && abs(lastmove.y-y)<8){if(hasArrow){tab->Video->SetCursor(wxCURSOR_SIZING);hasArrow=false;}}
		else if(abs(lastmove.x-x)<8 && abs(from.y-y)<8){if(hasArrow){tab->Video->SetCursor(wxCURSOR_SIZEWE);hasArrow=false;}}
		else if(abs(lastmove.y-y)<8 && abs(from.x-x)<8){if(hasArrow){tab->Video->SetCursor(wxCURSOR_SIZENS);hasArrow=false;}}
		else if(!hasArrow){tab->Video->SetCursor(wxCURSOR_ARROW);hasArrow=true;}
	}
	if(click){
		if(leftc){type=0;}
		if(rightc){type=1;}
		if(middlec){type=2;}
		if(abs(lastmove.x-x)<8 && abs(from.y-y)<8){grabbed=0;type=0;}
		else if(abs(lastmove.y-y)<8 && abs(from.x-x)<8){grabbed=1;type=1;}
		else if(abs(lastmove.x-x)<8 && abs(lastmove.y-y)<8){grabbed=2;type=2;}
		diffs.x=lastmove.x-x;
		diffs.y=lastmove.y-y;
		if(type==0){tab->Video->SetCursor(wxCURSOR_SIZEWE);}
		if(type==1){tab->Video->SetCursor(wxCURSOR_SIZENS);}
		if(type==2){tab->Video->SetCursor(wxCURSOR_SIZING);}
		hasArrow=false;
		if(grabbed==-1){
			int addy=(AN>3)?60 : -60, addx= (AN % 3 == 0)?-60 : 60;
			diffs.x=(from.x-x)+(addx*scale.x);
			diffs.y=(from.y-y)+(addy*scale.y);
		}
		to.x=x;to.y=y;
	}else if(holding){
		if(type!=1){
			to.x=x+diffs.x;
		}
		if(type!=0){
			to.y=y+diffs.y;
		}
		tab->Edit->SetVisual(GetVisual(),true,type);
	}
}

void Scale::SetCurVisual()
{
	D3DXVECTOR2 linepos = tab->Edit->GetPosnScale(&scale, &AN, tbl);
	if(tbl[6]>3){linepos=CalcMovePos();}
	from = D3DXVECTOR2(linepos.x/wspw,linepos.y/wsph);

	int addy=(AN>3)?60 : -60, addx= (AN % 3 == 0)?-60 : 60;
	to.x=from.x+(scale.x*addx);
	to.y=from.y+(scale.y*addy);

}