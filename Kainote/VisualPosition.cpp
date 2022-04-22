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

#ifdef guano
#include "config.h"
#include "Visuals.h"
#include "TabPanel.h"
#include "SubsGrid.h"
#include "VideoBox.h"
#include "EditBox.h"
#include <wx/regex.h>

enum {
	LEFT = 1,
	RIGHT,
	TOP = 4,
	BOTTOM = 8,
	INSIDE = 16,
	OUTSIDE = 32
};

Position::Position()
	: Visuals()
{
}


void Position::Draw(int time)
{
	wxMutexLocker lock(clipmutex);
	line->SetAntialias(TRUE);
	line->SetWidth(2.0);

	bool nothintoshow = true;
	for (size_t i = 0; i < data.size(); i++){
		auto pos = data[i];
		Dialogue* dial = tab->grid->GetDialogue(pos.numpos);
		if (!dial)
			continue;
		//don't forget to check if times are in range time >= start && time < end
		if (time >= dial->Start.mstime && time < dial->End.mstime){
			DrawCross(pos.pos);
			DrawRect(pos.pos);
			nothintoshow = false;
		}
	}

	if (hasPositionToRenctangle && !nothintoshow) {
		if (rectangleVisible) {
			D3DXVECTOR2 point1 = PositionToVideo(PositionRectangle[0]);
			D3DXVECTOR2 point2 = PositionToVideo(PositionRectangle[1]);
			D3DXVECTOR2 v4[5];
			v4[0] = point1;
			v4[1].x = point2.x;
			v4[1].y = point1.y;
			v4[2] = point2;
			v4[3].x = point1.x;
			v4[3].y = point2.y;
			v4[4] = point1;
			line->Begin();
			line->Draw(v4, 5, 0xFFBB0000);
			line->End();
		}
		//return;
	}

	line->SetAntialias(FALSE);
	oldtime = time;
	if (nothintoshow){ DrawWarning(notDialogue); blockevents = true; }
	else {
		if (blockevents)
			blockevents = false;

		if (hasHelperLine){
			D3DXVECTOR2 v2[2] = { D3DXVECTOR2(helperLinePos.x, 0), D3DXVECTOR2(helperLinePos.x, this->VideoSize.GetHeight()) };
			D3DXVECTOR2 v21[2] = { D3DXVECTOR2(0, helperLinePos.y), D3DXVECTOR2(this->VideoSize.GetWidth(), helperLinePos.y) };
			line->Begin();
			DrawDashedLine(v2, 2, 4, 0xFFFF00FF);
			DrawDashedLine(v21, 2, 4, 0xFFFF00FF);
			line->End();
			DrawRect(D3DXVECTOR2(helperLinePos.x, helperLinePos.y), false, 4.f);
		}

	}
}

void Position::ChangeTool(int _tool, bool blockSetCurVisual)
{
	if (!hasPositionToRenctangle && _tool & 32) {
		if(!blockSetCurVisual)
			GetPositioningData();
		rectangleVisible = false;
	}
	hasPositionToRenctangle = _tool & 32;
	hasPositionX = _tool & 64;
	hasPositionY = _tool & 128;
	int reset = 32 | 64 | 128;
	int newalignment = _tool | reset;
	newalignment ^= reset;
	if (newalignment != alignment) {
		alignment = newalignment;
		SortPoints();
		SetPosition();
		ChangeMultiline(true);
	}
	else {
		tab->video->Render(false);
	}
}

void Position::OnMouseEvent(wxMouseEvent &evt)
{

	if (blockevents){ return; }
	bool click = evt.LeftDown();
	bool holding = evt.LeftIsDown();

	int x, y;
	evt.GetPosition(&x, &y);

	if (hasPositionToRenctangle) {
		if (evt.ButtonUp()) {
			if (tab->video->HasCapture()) { tab->video->ReleaseMouse(); }
			if (rectangleVisible) {
				if (PositionRectangle[1].y == PositionRectangle[0].y ||
					PositionRectangle[1].x == PositionRectangle[0].x)
					rectangleVisible = false;

				SortPoints();
			}
			if (rectangleVisible) {
				SetPosition();
				ChangeMultiline(true);
			}

			if (!tab->video->HasArrow()) { tab->video->SetCursor(wxCURSOR_ARROW); }
		}

		if (!holding && rectangleVisible) {

			bool setarrow = false;
			int test = HitTest(D3DXVECTOR2(x, y));
			if (test < INSIDE) {
				setarrow = true;
				tab->video->SetCursor((test < 4) ? wxCURSOR_SIZEWE :
					(test >= 4 && test % 4 == 0) ? wxCURSOR_SIZENS :
					(test == (TOP + LEFT) || test == (BOTTOM + RIGHT)) ? wxCURSOR_SIZENWSE : wxCURSOR_SIZENESW);
			}
			if (!setarrow) { tab->video->SetCursor(wxCURSOR_ARROW); }
		}
		if (click || evt.LeftDClick()) {
			if (!tab->video->HasCapture()) { tab->video->CaptureMouse(); }
			grabbed = OUTSIDE;
			float pointx = ((x / zoomScale.x) + zoomMove.x) * coeffW,
				pointy = ((y / zoomScale.y) + zoomMove.y) * coeffH;
			if (rectangleVisible) {
				grabbed = HitTest(D3DXVECTOR2(x, y), true);
				if (grabbed == INSIDE) {
					if (PositionRectangle[0].x <= pointx &&
						PositionRectangle[1].x >= pointx &&
						PositionRectangle[0].y <= pointy &&
						PositionRectangle[1].y >= pointy) {
						diffs.x = x;
						diffs.y = y;
					}
				}
			}
			if (!rectangleVisible || grabbed == OUTSIDE) {
				PositionRectangle[0].x = PositionRectangle[1].x = pointx;
				PositionRectangle[0].y = PositionRectangle[1].y = pointy;
				grabbed = OUTSIDE;
				rectangleVisible = true;
			}

		}
		else if (holding && grabbed != -1) {
			if (grabbed < INSIDE) {
				if (grabbed & LEFT || grabbed & RIGHT) {
					x = MID(VideoSize.x, x, VideoSize.width);
					int posInTable = (grabbed & RIGHT) ? 1 : 0;
					PositionRectangle[posInTable].x =
						((((x + diffs.x) / zoomScale.x) + zoomMove.x) * coeffW);
					if (grabbed & LEFT && PositionRectangle[0].x > PositionRectangle[1].x) {
						PositionRectangle[0].x = PositionRectangle[1].x;
					}
					if (grabbed & RIGHT && PositionRectangle[1].x < PositionRectangle[0].x) {
						PositionRectangle[1].x = PositionRectangle[0].x;
					}
				}
				if (grabbed & TOP || grabbed & BOTTOM) {
					y = MID(VideoSize.y, y, VideoSize.height);
					int posInTable = (grabbed & BOTTOM) ? 1 : 0;
					PositionRectangle[posInTable].y =
						((((y + diffs.y) / zoomScale.y) + zoomMove.y) * coeffH);
					if (grabbed & TOP && PositionRectangle[0].y > PositionRectangle[1].y) {
						PositionRectangle[0].y = PositionRectangle[1].y;
					}
					if (grabbed & BOTTOM && PositionRectangle[1].y < PositionRectangle[0].y) {
						PositionRectangle[1].y = PositionRectangle[0].y;
					}
				}
			}
			else if (grabbed == INSIDE) {
				float movex = (((x - diffs.x) / zoomScale.x) * coeffW),
					movey = (((y - diffs.y) / zoomScale.y) * coeffH);
				PositionRectangle[0].x += movex;
				PositionRectangle[0].y += movey;
				PositionRectangle[1].x += movex;
				PositionRectangle[1].y += movey;
				diffs.x = x;
				diffs.y = y;
			}
			else if (grabbed == OUTSIDE) {
				float pointx = ((x / zoomScale.x) + zoomMove.x) * coeffW,
					pointy = ((y / zoomScale.y) + zoomMove.y) * coeffH;
				PositionRectangle[1].x = pointx;
				PositionRectangle[1].y = pointy;
			}
			SetPosition();
			if (rectangleVisible)
				ChangeMultiline(false);
			else
				tab->video->Render(false);
		}

		return;
	}

	if (evt.RightDown() || evt.LeftDClick()){
		for (size_t i = 0; i < data.size(); i++){
			if (data[i].numpos == tab->grid->currentLine){
				data[i].pos.x = x;
				data[i].pos.y = y;
				D3DXVECTOR2 diff(data[i].pos.x - data[i].lastpos.x, data[i].pos.y - data[i].lastpos.y);
				data[i].lastpos = data[i].pos;

				for (size_t j = 0; j < data.size(); j++){
					if (j == i){ continue; }
					data[j].pos += diff;
					data[j].lastpos = data[j].pos;
				}
				ChangeMultiline(evt.RightDown());
				break;
			}

		}
		return;
	}

	if (evt.LeftUp()){
		if (tab->video->HasCapture()){ tab->video->ReleaseMouse(); }
		ChangeMultiline(true);
		for (size_t i = 0; i < data.size(); i++){
			data[i].lastpos = data[i].pos;
		}
		if (!tab->video->HasArrow()){ tab->video->SetCursor(wxCURSOR_ARROW);}
		movingHelperLine = false;
	}
	if (movingHelperLine){
		helperLinePos = evt.GetPosition();
		tab->video->Render(false);
		return;
	}

	if (click){
		if (!tab->video->HasCapture()){ tab->video->CaptureMouse(); }
		if (IsInPos(evt.GetPosition(), helperLinePos, 4)){
			movingHelperLine = true;
			return;
		}
		tab->video->SetCursor(wxCURSOR_SIZING);
		wxArrayInt sels;
		tab->grid->file->GetSelections(sels);
		if (sels.size() != data.size()){ SetCurVisual(); tab->video->Render(); }
		firstmove.x = x;
		firstmove.y = y;
		axis = 0;
	}
	else if (holding){
		for (size_t i = 0; i < data.size(); i++){
			data[i].pos.x = data[i].lastpos.x - (firstmove.x - x);
			data[i].pos.y = data[i].lastpos.y - (firstmove.y - y);
			if (evt.ShiftDown()){
				//if(axis == 0){
				int diffx = abs(firstmove.x - x);
				int diffy = abs(firstmove.y - y);
				if (diffx != diffy){ if (diffx > diffy){ axis = 2; } else{ axis = 1; } }
				//}
				if (axis == 1){
					data[i].pos.x = data[i].lastpos.x;
				}
				else if (axis == 2){
					data[i].pos.y = data[i].lastpos.y;
				}
			}
		}
		ChangeMultiline(false);
	}
	if (evt.MiddleDown()){
		wxPoint mousePos = evt.GetPosition();
		hasHelperLine = (hasHelperLine && IsInPos(mousePos, helperLinePos, 4)) ? false : true;
		helperLinePos = mousePos;
		tab->video->Render(false);
	}
}


wxString Position::GetVisual(int datapos)
{
	return L"\\pos(" + getfloat(((data[datapos].pos.x / zoomScale.x) + zoomMove.x) * coeffW) + L"," +
		getfloat(((data[datapos].pos.y / zoomScale.y) + zoomMove.y) * coeffH) + L")";
}


void Position::SetCurVisual()
{
	int oldalignment = curLineAlingment;
	GetPosnScale(nullptr, &curLineAlingment, moveValues);
	data.clear();
	wxArrayInt sels;
	tab->grid->file->GetSelections(sels);
	bool putInBracket; 
	wxPoint textPosition;

	for (size_t i = 0; i < sels.size(); i++){
		//fix to work with editbox changes
		Dialogue *dial = (sels[i] == tab->grid->currentLine) ? tab->edit->line : tab->grid->GetDialogue(sels[i]);
		if (dial->IsComment){ continue; }
		D3DXVECTOR2 pos = GetPosition(dial, &putInBracket, &textPosition);
		data.push_back(PosData(sels[i], D3DXVECTOR2(((pos.x / coeffW) - zoomMove.x)*zoomScale.x,
			((pos.y / coeffH) - zoomMove.y)*zoomScale.y), textPosition, putInBracket));
	}

	if (hasPositionToRenctangle) {
		GetPositioningData();
		//hasPositionToRenctangle = false;
		if (rectangleVisible && oldalignment != curLineAlingment && oldalignment != -1) {
			SortPoints();
			SetPosition();
			ChangeMultiline(true, true);
		}
	}
}

void Position::ChangeMultiline(bool all, bool dummy)
{
	bool showOriginalOnVideo = !Options.GetBool(TL_MODE_HIDE_ORIGINAL_ON_VIDEO);
	wxString *dtxt = nullptr;
	if (!all && !dummytext){
		bool visible = false;
		selPositions.clear();
		dummytext = tab->grid->GetVisible(&visible, 0, &selPositions);
		if (selPositions.size() != data.size()){
			//do not show info cause it popup only when a comment is selected with other lines
			//and only first time it don't change anything it can be disabled.
			return;
		}
	}
	if (!all){ dtxt = new wxString(*dummytext); }
	bool skipInvisible = !all && tab->video->GetState() != Playing;
	int _time = tab->video->Tell();
	int moveLength = 0;
	const wxString &tlStyle = tab->grid->GetSInfo(L"TLMode Style");
	for (size_t i = 0; i < data.size(); i++){
		size_t k = data[i].numpos;
		Dialogue *Dial = (k == tab->grid->currentLine) ? tab->edit->line : tab->grid->GetDialogue(k);
		if (!Dial)
			continue;

		if (skipInvisible && !(_time >= Dial->Start.mstime && _time <= Dial->End.mstime)){ continue; }
		wxString visual = GetVisual(i);

		wxString txt = Dial->GetTextNoCopy();

		if (data[i].putinBracket){ visual = L"{" + visual + L"}"; }
		txt.replace(data[i].TextPos.x, data[i].TextPos.y, visual);
		if (all){
			tab->grid->CopyDialogue(data[i].numpos)->SetText(txt);
		}
		else{
			Dialogue Cpy = Dialogue(*Dial);
			if (Dial->TextTl != emptyString && tab->grid->hasTLMode) {
				Cpy.TextTl = txt;
				wxString tlLines;
				if (showOriginalOnVideo)
					Cpy.GetRaw(&tlLines, false, tlStyle);

				Cpy.GetRaw(&tlLines, true);
				dtxt->insert(selPositions[i] + moveLength, tlLines);
				moveLength += tlLines.length();
			}
			else{
				Cpy.Text = txt;
				wxString thisLine;
				Cpy.GetRaw(&thisLine);
				dtxt->insert(selPositions[i] + moveLength, thisLine);
				moveLength += thisLine.length();
			}
		}


	}

	if (all){
		SetModified(VISUAL_POSITION, dummy);
	}
	else{
		RenderSubs(dtxt);
	}

}

void Position::OnKeyPress(wxKeyEvent &evt)
{
	int key = evt.GetKeyCode();
	bool left = key == L'A';
	bool right = key == L'D';
	bool up = key == L'W';
	bool down = key == L'S';

	if ((left || right || up || down) && evt.GetModifiers() != wxMOD_ALT){
		float directionX = (left) ? -1 : (right) ? 1 : 0;
		float directionY = (up) ? -1 : (down) ? 1 : 0;
		if (evt.ShiftDown()){
			/*if (directionX)
				directionY = directionX;
			else if (directionY)
				directionX = directionY;*/
			directionX /= 10.f;
			directionY /= 10.f;
		}
		/*if (evt.ControlDown()){
			directionX /= 10;
			directionY /= 10;
		}*/
		directionX = (directionX / coeffW);
		directionY = (directionY / coeffH);

		for (size_t i = 0; i < data.size(); i++){
			data[i].pos.x = data[i].lastpos.x + directionX;
			data[i].pos.y = data[i].lastpos.y + directionY;
		}
		ChangeMultiline(true);
		return;
	}
	evt.Skip();
}

int Position::HitTest(const D3DXVECTOR2& pos, bool diff)
{
	int resultX = 0, resultY = 0, resultInside = 0, resultFinal = 0, oldpointx = 0, oldpointy = 0;
	for (int i = 0; i < 2; i++) {
		float pointx = ((PositionRectangle[i].x / coeffW) - zoomMove.x) * zoomScale.x,
			pointy = ((PositionRectangle[i].y / coeffH) - zoomMove.y) * zoomScale.y;
		//bool hasResult = false;
		if (abs(pos.x - pointx) < 5) {
			if (diff) {
				diffs.x = pointx - pos.x;
			}
			resultX |= (i + 1);
		}
		if (abs(pos.y - pointy) < 5) {
			if (diff) {
				diffs.y = pointy - pos.y;
			}
			resultY |= ((i + 1) * 4);
		}
		if (i) {
			resultInside |= (resultX ||
				(oldpointx <= pointx && oldpointx <= pos.x && pointx >= pos.x) ||
				(oldpointx >= pointx && oldpointx >= pos.x && pointx <= pos.x)) ? INSIDE : OUTSIDE;
			resultInside |= (resultY ||
				(oldpointx <= pointx && oldpointy <= pos.y && pointy >= pos.y) ||
				(oldpointx >= pointx && oldpointy >= pos.y && pointy <= pos.y)) ? INSIDE : OUTSIDE;
		}
		else {
			oldpointx = pointx;
			oldpointy = pointy;
		}
	}

	resultFinal = (resultInside & OUTSIDE) ? OUTSIDE : INSIDE;
	if (resultFinal == INSIDE) {
		resultFinal |= resultX;
		resultFinal |= resultY;
		if (resultFinal > INSIDE) { resultFinal ^= INSIDE; }
	}
	return resultFinal;
}

void Position::SortPoints()
{
	if (PositionRectangle[1].y < PositionRectangle[0].y) {
		float tmpy = PositionRectangle[0].y;
		PositionRectangle[0].y = PositionRectangle[1].y;
		PositionRectangle[1].y = tmpy;
	}
	if (PositionRectangle[1].x < PositionRectangle[0].x) {
		float tmpx = PositionRectangle[0].x;
		PositionRectangle[0].x = PositionRectangle[1].x;
		PositionRectangle[1].x = tmpx;
	}
}

void Position::SetPosition()
{
	if(hasPositionToRenctangle && rectangleVisible) {
		int an = alignment;
		float bordery = border[0].y;
		float borderx = border[0].x;
		float bordery1 = border[1].y;
		float borderx1 = border[1].x;
		//need to use rectangle[0] > rectangle[1]
		float rectx = PositionRectangle[0].x < PositionRectangle[1].x ? 
			PositionRectangle[0].x : PositionRectangle[1].x;
		float recty = PositionRectangle[0].y < PositionRectangle[1].y ?
			PositionRectangle[0].y : PositionRectangle[1].y;
		float rectx1 = PositionRectangle[0].x > PositionRectangle[1].x ?
			PositionRectangle[0].x : PositionRectangle[1].x;
		float recty1 = PositionRectangle[0].y > PositionRectangle[1].y ?
			PositionRectangle[0].y : PositionRectangle[1].y;
		float x = rectx, y = recty;
		//x coordinate
		if (an <= 15) {
			if (an % 3 == 0) {
				x = rectx1 - (textSize.x + borderx1 - 2);
			}
			else if (an % 3 == 1) {
				x += borderx + 2;
			}
			else if (an % 3 == 2) {
				x += (rectx1 - (rectx + textSize.x) + borderx) / 2;
			}
		}
		//before
		else if (an <= 18) {
			x = rectx - (textSize.x + borderx1 - 2);
		}
		//after
		else if (an <= 21) {
			x = rectx1 + borderx;
		}
		//y coordinate
		//bottom
		if (an <= 3) {
			y = recty1 - bordery;
		}
		//middle
		else if (an <= 6) {
			y += (((recty1 - recty) + textSize.y + extlead.y) / 2);
		}
		//top
		else if (an <= 9) {
			y += textSize.y + bordery1 - (extlead.x - extlead.y);
		}
		//below
		else if (an <= 12) {
			y = recty1 + textSize.y + bordery;
		}
		//above
		else if (an <= 15) {
			y = recty;
		}//before - after
		else if (an <= 21) {
			//bottom
			if (an % 3 == 0) {
				y = recty1 - bordery;
			}
			//middle
			else if (an % 3 == 2) {
				y += (((recty1 - recty) + textSize.y + extlead.y) / 2);
			}
			//top
			else if (an % 3 == 1) {
				y += textSize.y + bordery1 - (extlead.x - extlead.y);
			}
		}
		
		for (size_t i = 0; i < data.size(); i++) {
			if (data[i].numpos == tab->grid->currentLine) {
				if(hasPositionX)
					data[i].pos.x = x + curLinePosition.x;
				if (hasPositionY)
					data[i].pos.y = y + curLinePosition.y;
				data[i].pos = PositionToVideo(data[i].pos, hasPositionX, hasPositionY);
				D3DXVECTOR2 diff(data[i].pos.x - data[i].lastpos.x, data[i].pos.y - data[i].lastpos.y);
				data[i].lastpos = data[i].pos;

				for (size_t j = 0; j < data.size(); j++) {
					if (j == i) { continue; }
					data[j].pos += diff;
					data[j].lastpos = data[j].pos;
				}
				break;
			}

		}
	}
	
}

D3DXVECTOR2 Position::PositionToVideo(D3DXVECTOR2 point, bool changeX, bool changeY)
{
	float pointx = changeX? ((point.x / coeffW) - zoomMove.x) * zoomScale.x : point.x,
		pointy = changeY? ((point.y / coeffH) - zoomMove.y) * zoomScale.y : point.y;
	return D3DXVECTOR2(pointx, pointy);
}

void Position::GetPositioningData()
{
	textSize = GetTextSize(tab->edit->line, nullptr, nullptr, true, &extlead, &drawingPosition, border);
	curLinePosition = drawingPosition;
	//no alignment? get it
	if(curLineAlingment == -1)
		GetPosnScale(nullptr, &curLineAlingment, moveValues);

	//make from current line position an7
	if (curLineAlingment % 3 == 0) {
		curLinePosition.x += textSize.x;
	}
	else if (curLineAlingment % 3 == 2) {
		curLinePosition.x += (textSize.x) / 2;
	}
	if (curLineAlingment <= 3) {
		//curLinePosition.y = 0;
	}
	else if (curLineAlingment <= 6) {
		curLinePosition.y -= (textSize.y / 2);
	}
	else if (curLineAlingment <= 9) {
		curLinePosition.y -= textSize.y;
	}
}
#endif