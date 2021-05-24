//  Copyright (c) 2016 - 2020, Marcin Drob

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

#include "config.h"
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
	if (hasHelperLine){
		D3DXVECTOR2 v2[2] = { D3DXVECTOR2(helperLinePos.x, 0), D3DXVECTOR2(helperLinePos.x, this->VideoSize.GetHeight()) };
		D3DXVECTOR2 v21[2] = { D3DXVECTOR2(0, helperLinePos.y), D3DXVECTOR2(this->VideoSize.GetWidth(), helperLinePos.y) };
		line->Begin();
		DrawDashedLine(v2, 2, 4, 0xFFFF00FF);
		DrawDashedLine(v21, 2, 4, 0xFFFF00FF);
		line->End();
		DrawRect(D3DXVECTOR2(helperLinePos.x, helperLinePos.y), false, 4.f);
	}

	D3DXVECTOR2 v4[6];
	v4[0].x = from.x;
	v4[0].y = from.y;
	v4[1].x = to.x;
	v4[1].y = to.y;
	//drawarrow moves line point to ends before arrow, it modifies v4[1]
	DrawArrow(from, &v4[1], 6);


	float tmpt = time - moveStart;
	float tmpt1 = moveEnd - moveStart;
	float actime = tmpt / tmpt1;
	D3DXVECTOR2 dist;
	if (time < moveStart){ dist.x = from.x, dist.y = from.y; }
	else if (time > moveEnd){ dist.x = to.x, dist.y = to.y; }
	else {
		dist.x = from.x - ((from.x - to.x)*actime);
		dist.y = from.y - ((from.y - to.y)*actime);
	}

	line->Begin();
	line->Draw(v4, 2, 0xFFBB0000);
	DrawCross(dist, 0xFFBB0000, false);
	line->End();

	DrawRect(from);
	DrawCircle(to);


}

void Move::GetVisual(wxString *visual)
{
	int startTime = ZEROIT(tab->Edit->line->Start.mstime);
	*visual = L"\\move(" + getfloat(((from.x / zoomScale.x) + zoomMove.x) * coeffW) + L"," +
		getfloat(((from.y / zoomScale.y) + zoomMove.y) * coeffH) + L"," +
		getfloat(((to.x / zoomScale.x) + zoomMove.x) * coeffW) + L"," +
		getfloat(((to.y / zoomScale.y) + zoomMove.y) * coeffH) + L"," +
		getfloat(moveValues[4] - startTime) + L"," +
		getfloat(moveValues[5] - startTime) + L")";
}

void Move::OnMouseEvent(wxMouseEvent &evt)
{
	if (blockevents){ return; }
	bool click = evt.LeftDown() || evt.RightDown();
	bool holding = evt.LeftIsDown() || evt.RightIsDown();
	bool leftc = evt.LeftDown();
	bool rightc = evt.RightDown();
	bool shift = evt.ShiftDown();

	int x, y;
	evt.GetPosition(&x, &y);

	if (evt.ButtonUp()){
		if (tab->Video->HasCapture()){ tab->Video->ReleaseMouse(); }
		SetVisual(false, type);
		if (!tab->Video->HasArrow()){ tab->Video->SetCursor(wxCURSOR_ARROW); }
		grabbed = -1;
		moveDistance = to - from;
		movingHelperLine = false;
	}

	if (movingHelperLine){
		helperLinePos = evt.GetPosition();
		tab->Video->Render(false);
		return;
	}

	if (click){
		tab->Video->CaptureMouse();
		if (IsInPos(evt.GetPosition(), helperLinePos, 4)){
			movingHelperLine = true;
			return;
		}
		tab->Video->SetCursor(wxCURSOR_SIZING);
		if (leftc){ type = 0; }
		if (rightc){ type = 1; }

		if (abs(from.x - x) < 8 && abs(from.y - y) < 8 && leftc){
			grabbed = 0; type = 0;
			diffs.x = from.x - x;
			diffs.y = from.y - y;
		}
		else if (abs(to.x - x) < 8 && abs(to.y - y) < 8) {
			grabbed = 1; type = 1;
			diffs.x = to.x - x;
			diffs.y = to.y - y;
		}
		else if (!shift) {
			grabbed = -1;
			if (type == 1){
				to.x = x;
				to.y = y;
			}
			else{
				from.x = x;
				from.y = y;
			}
			diffs = wxPoint(0, 0);
		}
		lastmove = lastTo = to;
		firstmove = lastFrom = from;
		SetVisual(true, type);
		axis = 0;
	}
	if (holding){
		if (type == 0){
			from.x = x + diffs.x;
			from.y = y + diffs.y;
		}
		else{
			to.x = x + diffs.x;
			to.y = y + diffs.y;
		}
		if (shift){
			//if(axis == 0){
			int diffx = abs((type == 0) ? firstmove.x - x : lastmove.x - x);
			int diffy = abs((type == 0) ? firstmove.y - y : lastmove.y - y);
			if (diffx != diffy){ if (diffx > diffy){ axis = 2; } else{ axis = 1; } }
			//}
			if (type == 0){
				if (axis == 1){
					from.x = firstmove.x;
				}
				if (axis == 2){
					from.y = firstmove.y;
				}
			}
			else{
				if (axis == 1){
					to.x = lastmove.x;
				}
				if (axis == 2){
					to.y = lastmove.y;
				}
			}
		}
		SetVisual(true, type);
	}
	if (evt.MiddleDown()){
		wxPoint mousePos = evt.GetPosition();
		hasHelperLine = (hasHelperLine && IsInPos(mousePos, helperLinePos, 4)) ? false : true;
		helperLinePos = mousePos;
		tab->Video->Render(false);
	}
}

void Move::SetCurVisual()
{
	D3DXVECTOR2 linepos = GetPosnScale(NULL, NULL, moveValues);
	from = to = D3DXVECTOR2(((linepos.x / coeffW) - zoomMove.x) * zoomScale.x,
		((linepos.y / coeffH) - zoomMove.y) * zoomScale.y);

	if (moveValues[6] > 3){
		to.x = ((moveValues[2] / coeffW) - zoomMove.x) * zoomScale.x;
		to.y = ((moveValues[3] / coeffH) - zoomMove.y) * zoomScale.y;
	}
	moveDistance = to - from;
	int startIter = 4, endIter = 5;
	if (moveValues[4] > moveValues[5]){ startIter = 5; endIter = 4; }
	moveStart = (int)moveValues[startIter];
	moveEnd = (int)moveValues[endIter];

}

void Move::ChangeVisual(wxString *txt, Dialogue *_dial)
{
	bool putinbracket = false;
	wxPoint tagPos;
	D3DXVECTOR2 textPosition = GetPosition(_dial, &putinbracket, &tagPos);
	D3DXVECTOR2 moveFrom = lastFrom - from;
	D3DXVECTOR2 moveTo = lastTo - to;
	int moveStartTime = 0, moveEndTime = 0;
	wxString tagBefore = putinbracket ? L"" : txt->SubString(tagPos.x, tagPos.y);
	wxArrayString values = wxStringTokenize(tagBefore, L",", wxTOKEN_STRTOK);
	if (putinbracket || values.size() < 6){
		GetMoveTimes(&moveStartTime, &moveEndTime);
	}
	else{
		wxString t2 = values[5];
		wxString t1 = values[4];
		t2.Replace(L")", L"");
		moveStartTime = wxAtoi(t1);
		moveEndTime = wxAtoi(t2);
	}
	wxString tag = L"\\move(" + getfloat(textPosition.x - ((moveFrom.x / zoomScale.x) + zoomMove.x) * coeffW) + L"," +
		getfloat(textPosition.y - ((moveFrom.y / zoomScale.y) + zoomMove.y) * coeffH) + L"," +
		getfloat(textPosition.x + (((moveDistance.x - moveTo.x) / zoomScale.x) + zoomMove.x) * coeffW) + L"," +
		getfloat(textPosition.y + (((moveDistance.y - moveTo.y) / zoomScale.y) + zoomMove.y) * coeffH) + L"," +
		std::to_wstring(moveStartTime) + L"," +
		std::to_wstring(moveEndTime) + L")";
	//positions returns length that need to add start
	tagPos.y += tagPos.x - 1;
	ChangeText(txt, tag, !putinbracket, tagPos);
}

void Move::OnKeyPress(wxKeyEvent &evt)
{

}