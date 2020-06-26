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


#include "Visuals.h"
#include "TabPanel.h"
#include <wx/regex.h>

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
		//don't forget to check if times are in range time >= start && time < end
		if (time >= pos.dial->Start.mstime && time < pos.dial->End.mstime){
			DrawCross(pos.pos);
			DrawRect(pos.pos);
			nothintoshow = false;
		}
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

void Position::OnMouseEvent(wxMouseEvent &evt)
{

	if (blockevents){ return; }
	bool click = evt.LeftDown();
	bool holding = evt.LeftIsDown();

	int x, y;
	evt.GetPosition(&x, &y);

	if (evt.RightDown() || evt.LeftDClick()){
		for (size_t i = 0; i < data.size(); i++){
			if (data[i].numpos == tab->Grid->currentLine){
				data[i].pos.x = x;
				data[i].pos.y = y;
				data[i].lastpos = data[i].pos;
				D3DXVECTOR2 diff(data[i].pos.x - data[i].lastpos.x, data[i].pos.y - data[i].lastpos.y);

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
		if (tab->Video->HasCapture()){ tab->Video->ReleaseMouse(); }
		ChangeMultiline(true);
		for (size_t i = 0; i < data.size(); i++){
			data[i].lastpos = data[i].pos;
		}
		if (!tab->Video->HasArrow()){ tab->Video->SetCursor(wxCURSOR_ARROW);}
		movingHelperLine = false;
	}
	if (movingHelperLine){
		helperLinePos = evt.GetPosition();
		tab->Video->Render(false);
		return;
	}

	if (click){
		if (!tab->Video->HasCapture()){ tab->Video->CaptureMouse(); }
		if (IsInPos(evt.GetPosition(), helperLinePos, 4)){
			movingHelperLine = true;
			return;
		}
		tab->Video->SetCursor(wxCURSOR_SIZING);
		wxArrayInt sels;
		tab->Grid->file->GetSelections(sels);
		if (sels.size() != data.size()){ SetCurVisual(); tab->Video->Render(); }
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
		tab->Video->Render(false);
	}
}


wxString Position::GetVisual(int datapos)
{
	return L"\\pos(" + getfloat(((data[datapos].pos.x / zoomScale.x) + zoomMove.x) * coeffW) + L"," +
		getfloat(((data[datapos].pos.y / zoomScale.y) + zoomMove.y) * coeffH) + L")";
}


void Position::SetCurVisual()
{
	data.clear();
	wxArrayInt sels;
	tab->Grid->file->GetSelections(sels);
	bool putInBracket; 
	wxPoint textPosition;

	for (size_t i = 0; i < sels.size(); i++){
		//fix to work with editbox changes
		Dialogue *dial = (sels[i] == tab->Grid->currentLine) ? tab->Edit->line : tab->Grid->GetDialogue(sels[i]);
		if (dial->IsComment){ continue; }
		D3DXVECTOR2 pos = GetPosition(dial, &putInBracket, &textPosition);
		data.push_back(PosData(dial, sels[i], D3DXVECTOR2(((pos.x / coeffW) - zoomMove.x)*zoomScale.x,
			((pos.y / coeffH) - zoomMove.y)*zoomScale.y), textPosition, putInBracket));
	}
}

void Position::ChangeMultiline(bool all)
{
	bool showOriginalOnVideo = !Options.GetBool(TL_MODE_HIDE_ORIGINAL_ON_VIDEO);
	wxString *dtxt = NULL;
	if (!all && !dummytext){
		bool visible = false;
		selPositions.clear();
		dummytext = tab->Grid->GetVisible(&visible, 0, &selPositions);
		if (selPositions.size() != data.size()){
			//do not show info cause it popup only when a comment is selected with other lines
			//and only first time it don't change anything it can be disabled.
			return;
		}
	}
	if (!all){ dtxt = new wxString(*dummytext); }
	bool skipInvisible = !all && tab->Video->GetState() != Playing;
	int _time = tab->Video->Tell();
	int moveLength = 0;
	const wxString &tlStyle = tab->Grid->GetSInfo(L"TLMode Style");
	for (size_t i = 0; i < data.size(); i++){

		Dialogue *Dial = data[i].dial;
		if (skipInvisible && !(_time >= Dial->Start.mstime && _time <= Dial->End.mstime)){ continue; }
		wxString visual = GetVisual(i);

		wxString txt = Dial->GetTextNoCopy();

		if (data[i].putinBracket){ visual = L"{" + visual + L"}"; }
		txt.replace(data[i].TextPos.x, data[i].TextPos.y, visual);
		if (all){
			tab->Grid->CopyDialogue(data[i].numpos)->SetText(txt);
		}
		else{
			Dialogue Cpy = Dialogue(*Dial);
			if (Dial->TextTl != L"" && tab->Grid->hasTLMode) {
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
		SetModified(VISUAL_POSITION);
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