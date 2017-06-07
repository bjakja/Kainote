//  Copyright (c) 2016, Marcin Drob

//  Kainote is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.

//  Kainote is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.

//  You should have received a copy of the GNU General Public License
//  along with Kainote.  If not, see <http://www.gnu.org/licenses/>.

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
	line->Draw(v4,2,0xFFBB0000);
	DrawCross(dist, 0xFFBB0000, false);
	line->End();

	DrawRect(from);
	DrawCircle(to);


}

wxString Move::GetVisual()
{
	int startTime = ZEROIT(tab->Edit->line->Start.mstime);
	return "\\move("+getfloat(((from.x/zoomScale.x)+zoomMove.x)*wspw) + "," +
		getfloat(((from.y/zoomScale.y)+zoomMove.y)*wsph) + "," +
		getfloat(((to.x/zoomScale.x)+zoomMove.x)*wspw) + "," +
		getfloat(((to.y/zoomScale.y)+zoomMove.y)*wsph) + "," +
		getfloat(tbl[4] - startTime) + "," +
		getfloat(tbl[5] - startTime) + ")";
}

void Move::OnMouseEvent(wxMouseEvent &evt)
{
	if(blockevents){return;}
	bool click = evt.LeftDown()||evt.RightDown()||evt.MiddleDown();
	bool holding = (evt.LeftIsDown()||evt.RightIsDown()||evt.MiddleIsDown());
	bool leftc = evt.LeftDown();
	bool rightc = evt.RightDown();

	int x, y;
	evt.GetPosition(&x,&y);

	if(evt.ButtonUp()){
		if(tab->Video->HasCapture()){tab->Video->ReleaseMouse();}
		SetVisual(false,type);
		if(!hasArrow){tab->Video->SetCursor(wxCURSOR_ARROW);hasArrow=true;}
		grabbed = -1;
		moveDistance = to - from;
	}

	if(click){
		tab->Video->CaptureMouse();
		tab->Video->SetCursor(wxCURSOR_SIZING );
		hasArrow=false;
		if(leftc){type=0;}
		if(rightc){type=1;}

		if(abs(to.x-x)<8 && abs(to.y-y)<8){
			grabbed=1; type=1;
			//lastTo = to;
			diffs.x=to.x-x;
			diffs.y=to.y-y;
		}else if(abs(from.x-x)<8 && abs(from.y-y)<8){
			grabbed=0; type=0;
			diffs.x=from.x-x;
			diffs.y=from.y-y;
		}else{
			grabbed= -1;
			if(type==1){
				to.x=x;
				to.y=y;
				//lastTo = to;
			}else{
				from.x=x;
				from.y=y;
			}
			diffs=wxPoint(0,0);
		}
		lastmove = lastTo = to;
		firstmove = lastFrom = from;
		SetVisual(true,type);
		axis = 0;
	}else if(holding){
		if(type==0){
			from.x=x+diffs.x;
			from.y=y+diffs.y;
		}else{
			to.x=x+diffs.x;
			to.y=y+diffs.y;
		}
		if(evt.ShiftDown()){
			if(axis == 0){
				int diffx = abs((type==0)? firstmove.x-x : lastmove.x-x);
				int diffy = abs((type==0)? firstmove.y-y : lastmove.y-y);
				if(diffx != diffy){if(diffx > diffy){axis = 2;}else{axis = 1;}}
			}
			if(type==0){
				if(axis==1){
					from.x = firstmove.x;
				}
				if(axis==2){
					from.y = firstmove.y;
				}
			}else{
				if(axis==1){
					to.x = lastmove.x;
				}
				if(axis ==2){
					to.y = lastmove.y;
				}
			}
		}
		SetVisual(true,type);
	}

}

void Move::SetCurVisual()
{
	D3DXVECTOR2 linepos = GetPosnScale(NULL, NULL, tbl);
	from = to = D3DXVECTOR2(((linepos.x/wspw)-zoomMove.x)*zoomScale.x,
		((linepos.y/wsph)-zoomMove.y)*zoomScale.y);

	if(tbl[6]>3){
		to.x=((tbl[2]/wspw)-zoomMove.x)*zoomScale.x; 
		to.y=((tbl[3]/wsph)-zoomMove.y)*zoomScale.y;
	}
	moveDistance = to - from;
	moveStart=(int)tbl[4];
	if(tbl[4]>tbl[5]){tbl[5]=end;}
	moveEnd=(int)tbl[5];

}

void Move::ChangeVisual(wxString *txt, Dialogue *_dial)
{
	VideoCtrl *video = tab->Video;
	float fps = video->fps;
	bool dshow = video->IsDshow;
	int startTime = ZEROIT(_dial->Start.mstime);
	int endTime = ZEROIT(_dial->End.mstime);
	int framestart = (dshow)? (((float)startTime/1000.f) * fps)+1 : video->VFF->GetFramefromMS(startTime);
	int frameend = (dshow)? (((float)endTime/1000.f) * fps) : video->VFF->GetFramefromMS(endTime)-1;
	int msstart = (dshow)? ((framestart*1000) / fps) + 0.5f : video->VFF->GetMSfromFrame(framestart);
	int msend = (dshow)? ((frameend*1000) / fps) + 0.5f : video->VFF->GetMSfromFrame(frameend);
	int diff = endTime - startTime;
	int moveStartTime = abs(msstart - startTime);
	int moveEndTime = (diff - abs(endTime - msend));
	bool inbracket=false;
	wxPoint tagPos;
	D3DXVECTOR2 textPosition = GetPos(_dial, &inbracket, &tagPos);
	D3DXVECTOR2 moveFrom = lastFrom - from;
	D3DXVECTOR2 moveTo = lastTo - to;
	//wxLogStatus("from %f %f to %f %f",moveFrom.x, moveFrom.y, moveTo.x, moveTo.y);
	wxString tag = "\\move("+getfloat(textPosition.x - ((moveFrom.x/zoomScale.x)+zoomMove.x)*wspw) + "," +
		getfloat(textPosition.y - ((moveFrom.y/zoomScale.y)+zoomMove.y)*wsph) + "," +
		getfloat(textPosition.x + (((moveDistance.x - moveTo.x)/zoomScale.x)+zoomMove.x)*wspw) + "," +
		getfloat(textPosition.y + (((moveDistance.y - moveTo.y)/zoomScale.y)+zoomMove.y)*wsph) + "," +
		std::to_string(moveStartTime) + "," +
		std::to_string(moveEndTime) + ")";
	//jako ¿e pozycje zwracaj¹ len to potrzeba dodaæ jeszcze start;
	tagPos.y += tagPos.x - 1;
	ChangeText(txt, tag, !inbracket, tagPos);
}