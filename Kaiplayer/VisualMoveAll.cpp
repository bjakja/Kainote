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
#include <wx/regex.h>


enum {
	TAGPOS = 1,
	TAGMOVES,
	TAGMOVEE = 4,
	TAGCLIP = 8,
	TAGP = 16,
	TAGORG = 32
};

MoveAll::MoveAll()
	: Visuals()
	, numElem(-1)
	, selectedTags(1)
{
}

void MoveAll::DrawVisual(int time)
{
	for (size_t i = 0; i < elems.size(); i++){
		if (!(selectedTags & elems[i].type)){ continue; }
		if (elems[i].type == TAGPOS || elems[i].type == TAGMOVES){
			DrawRect(elems[i].elem);
		}
		else if (elems[i].type == TAGMOVEE){
			DrawCircle(elems[i].elem);
		}
		else{
			D3DXCOLOR col = (elems[i].type == TAGCLIP) ? 0xFF0000FF : (elems[i].type == TAGP) ? 0xFFFF00FF : 0xFF8800FF;
			DrawCross(elems[i].elem, col);
		}
	}

}

void MoveAll::OnMouseEvent(wxMouseEvent &evt)
{
	if (blockevents){ return; }
	bool click = evt.LeftDown();
	bool holding = (evt.LeftIsDown() || evt.RightIsDown());

	int x, y;
	evt.GetPosition(&x, &y);

	if (evt.ButtonUp()){
		if (tab->Video->HasCapture()){
			tab->Video->ReleaseMouse();
		}
		if (numElem >= 0){ ChangeInLines(true); }
		if (!hasArrow){ tab->Video->SetCursor(wxCURSOR_ARROW); hasArrow = true; }
		numElem = -1;
	}

	if (click){

		for (size_t i = 0; i < elems.size(); i++){
			if (!(selectedTags & elems[i].type)){ continue; }
			if (abs(elems[i].elem.x - x) < 8 && abs(elems[i].elem.y - y) < 8 || i == elems.size() - 1){
				numElem = i;
				beforeMove = lastmove = elems[i].elem;
				diffs.x = elems[i].elem.x - x;
				diffs.y = elems[i].elem.y - y;
				if (!tab->Video->HasCapture()){
					tab->Video->CaptureMouse();
				}
			}
		}
		firstmove = D3DXVECTOR2(x, y);
		axis = 0;
	}
	else if (evt.RightDown()){

		for (size_t i = 0; i < elems.size(); i++){
			if (!(selectedTags & elems[i].type)){ continue; }
			numElem = i;
			beforeMove = lastmove = elems[i].elem;
			diffs.x = elems[i].elem.x - x;
			diffs.y = elems[i].elem.y - y;
			if (!tab->Video->HasCapture()){
				tab->Video->CaptureMouse();
			}
			break;
		}
		firstmove = D3DXVECTOR2(x, y);
		axis = 0;
	}
	else if (holding && numElem >= 0){


		if (evt.ShiftDown()){
			//if(axis == 0){
			int diffx = abs(firstmove.x - x);
			int diffy = abs(firstmove.y - y);
			if (diffx != diffy){ if (diffx > diffy){ axis = 1; } else{ axis = 2; } }
			//return;
			//}
			lastmove = elems[numElem].elem;
			if (axis == 1){
				elems[numElem].elem.x = x + diffs.x;
				elems[numElem].elem.y = beforeMove.y;
			}
			else if (axis == 2){
				elems[numElem].elem.y = y + diffs.y;
				elems[numElem].elem.x = beforeMove.x;
			}
			D3DXVECTOR2 moving = elems[numElem].elem - lastmove;
			for (size_t j = 0; j < elems.size(); j++){
				if (j == numElem || !(selectedTags & elems[j].type)){ continue; }
				if (axis == 1){
					elems[j].elem.x += moving.x;
				}
				else if (axis == 2){
					elems[j].elem.y += moving.y;
				}
			}
		}
		else{
			lastmove = elems[numElem].elem;
			elems[numElem].elem.x = x + diffs.x;
			elems[numElem].elem.y = y + diffs.y;

			D3DXVECTOR2 moving = elems[numElem].elem - lastmove;
			for (size_t j = 0; j < elems.size(); j++){
				if (j == numElem || !(selectedTags & elems[j].type)){ continue; }
				elems[j].elem.x += moving.x;
				elems[j].elem.y += moving.y;
			}

		}
		ChangeInLines(false);
	}

}

void MoveAll::SetCurVisual()
{
	D3DXVECTOR2 linepos = GetPosnScale(NULL, NULL, moveValues);
	if (moveValues[6] > 3){ linepos = CalcMovePos(); }
	from = to = D3DXVECTOR2(((linepos.x / coeffW) - zoomMove.x)*zoomScale.x,
		((linepos.y / coeffH) - zoomMove.y)*zoomScale.y);
	elems.clear();

	wxString res;
	if (tab->Edit->FindValue(L"org\\(([^\\)]+)", &res, L"", 0, true)){
		wxString rest;
		double orx, ory;

		moveElems elem;
		if (res.BeforeFirst(L',', &rest).ToDouble(&orx)){ elem.elem.x = ((orx / coeffW) - zoomMove.x)*zoomScale.x; }
		if (rest.ToDouble(&ory)){ elem.elem.y = ((ory / coeffH) - zoomMove.y)*zoomScale.y; }
		elem.type = TAGORG;
		elems.push_back(elem);
	}
	if (tab->Edit->FindValue(L"(i?clip[^\\)]+)", &res, L"", 0, true)){
		wxRegEx re(L"m ([0-9.-]+) ([0-9.-]+)", wxRE_ADVANCED);
		moveElems elem;
		if (re.Matches(res)){
			elem.elem = D3DXVECTOR2(((wxAtoi(re.GetMatch(res, 1)) / coeffW) - zoomMove.x)*zoomScale.x,
				((wxAtoi(re.GetMatch(res, 2)) / coeffH) - zoomMove.y)*zoomScale.y);
		}
		else{
			//wxString txt = tab->Edit->TextEdit->GetValue();
			int repl = res.Replace(L",", L",");
			wxRegEx re(L"\\(([0-9.-]+)[, ]*([0-9.-]+)", wxRE_ADVANCED);
			if (repl >= 3 && re.Matches(res)){
				elem.elem = D3DXVECTOR2(((wxAtoi(re.GetMatch(res, 1)) / coeffW) - zoomMove.x)*zoomScale.x,
					((wxAtoi(re.GetMatch(res, 2)) / coeffH) - zoomMove.y)*zoomScale.y);
			}
		}
		elem.type = TAGCLIP;
		elems.push_back(elem);
	}
	if (tab->Edit->FindValue(L"p([0-9]+)", &res, L"", 0, true)){
		res = tab->Edit->TextEdit->GetValue();
		wxRegEx re(L"} ?m ([.0-9-]+) ([.0-9-]+)", wxRE_ADVANCED);
		if (re.Matches(res)){
			moveElems elem;

			elem.elem = D3DXVECTOR2(((wxAtoi(re.GetMatch(res, 1)) / coeffW) - zoomMove.x)*zoomScale.x,
				((wxAtoi(re.GetMatch(res, 2)) / coeffH) - zoomMove.y)*zoomScale.y);
			elem.type = TAGP;
			elems.push_back(elem);
		}

	}
	if (moveValues[6] == 2){
		moveElems elem;
		elem.elem = D3DXVECTOR2(((moveValues[0] / coeffW) - zoomMove.x)*zoomScale.x,
			((moveValues[1] / coeffH) - zoomMove.y)*zoomScale.y);
		elem.type = TAGPOS;
		elems.push_back(elem);
	}
	if (moveValues[6] >= 4){
		moveElems elem;
		elem.elem = D3DXVECTOR2(((moveValues[0] / coeffW) - zoomMove.x)*zoomScale.x,
			((moveValues[1] / coeffH) - zoomMove.y)*zoomScale.y);
		elem.type = TAGMOVES;
		elems.push_back(elem);
		elem.type = TAGMOVEE;
		elem.elem = D3DXVECTOR2(((moveValues[2] / coeffW) - zoomMove.x)*zoomScale.x,
			((moveValues[3] / coeffH) - zoomMove.y)*zoomScale.y);
		elems.push_back(elem);
	}

}

wxString MoveAll::GetVisual()
{
	return L"";
}

void MoveAll::ChangeInLines(bool all)
{
	bool showOriginalOnVideo = !Options.GetBool(TL_MODE_HIDE_ORIGINAL_ON_VIDEO);
	//D3DXVECTOR2 moving;
	D3DXVECTOR2 moving = elems[numElem].elem - beforeMove;
	int _time = tab->Video->Tell();
	wxArrayInt sels;
	tab->Grid->file->GetSelections(sels);
	wxString *dtxt;
	if (!all){
		if (!dummytext){
			selPositions.clear();
			bool visible = false;
			dummytext = tab->Grid->GetVisible(&visible, 0, &selPositions);
			if (selPositions.size() != sels.size()){
				KaiLog(L"Sizes mismatch");
				return;
			}
		}

		dtxt = new wxString(*dummytext);
	}
	bool skipInvisible = !all && tab->Video->GetState() != Playing;
	wxString tmp;
	//bool isOriginal=(tab->Grid1->transl && tab->Edit->TextEdit->GetValue()==L"");
	//MTextEditor *Editor=(isOriginal)? tab->Edit->TextEditTl : tab->Edit->TextEdit;
	//wxString origText=Editor->GetValue();
	const wxString &tlModeStyle = tab->Grid->GetSInfo(L"TLMode Style");
	int moveLength = 0;

	for (size_t i = 0; i < sels.size(); i++){
		wxString txt;
		Dialogue *Dial = tab->Grid->GetDialogue(sels[i]);

		if (skipInvisible && !(_time >= Dial->Start.mstime && _time <= Dial->End.mstime)){ continue; }
		bool istexttl = (tab->Grid->hasTLMode && Dial->TextTl != L"");
		txt = (istexttl) ? Dial->TextTl : Dial->Text;

		for (int k = 0; k < 6; k++){
			byte type = selectedTags & (1 << k);
			if (!type){ continue; }
			bool vector = type == TAGCLIP || type == TAGP;
			wxString delimiter = (vector) ? L" " : L",";
			wxString tagpattern = (type == TAGPOS) ? L"pos\\(([^\\)]+)" : (type == TAGORG) ? L"org\\(([^\\)]+)" : (type == TAGCLIP) ? L"i?clip\\(([^\\)]+)" : (type == TAGP) ? L"p[0-9-]+[^}]*} ?m ([^{]+)" : L"move\\(([^\\)]+)";
			wxRegEx re(tagpattern, wxRE_ADVANCED);
			size_t startMatch = 0, lenMatch = 0;
			if (re.Matches(txt)){
				wxString visual;
				//wxString tag=re.GetMatch(txt, 1); tag też nigdzie nie jest potrzebny, bo wycinamy tylko jego wartość.
				tmp = re.GetMatch(txt, 1);
				if (type == TAGCLIP){
					int replacements = tmp.Replace(L',', L',');
					if (replacements == 1){
						tmp = tmp.After(L',');
					}
					else if (replacements > 1){
						delimiter = L",";
					}
				}
				//re.GetMatch(&startMatch, &lenMatch, 2); niepotrzebny drugi raz użycie tego samego
				wxStringTokenizer tkn(tmp, delimiter, wxTOKEN_STRTOK);
				int count = 0;
				while (tkn.HasMoreTokens()){
					wxString token = tkn.GetNextToken().Trim().Trim(false);
					double val;
					if (token.ToDouble(&val)){
						if (count % 2 == 0){ val += (((moving.x / zoomScale.x)) * coeffW); }
						else{ val += (((moving.y / zoomScale.y)) * coeffH); }
						if (type == TAGMOVES && count > 1){ visual += token + delimiter; continue; }
						else if (type == TAGMOVEE && count != 2 && count != 3){ visual += token + delimiter; count++; continue; }
						if (vector){ visual << getfloat(val, (type == TAGCLIP) ? L"6.0f" : L"6.2f") << delimiter; }
						else{ visual += getfloat(val) + delimiter; }
						count++;
					}
					else{
						visual += token + delimiter;
						if (!vector){ count++; }
					}
				}
				if (re.GetMatch(&startMatch, &lenMatch, 1)){
					visual.RemoveLast();
					if (lenMatch){ txt.erase(txt.begin() + startMatch, txt.begin() + startMatch + lenMatch); }
					txt.insert(startMatch, visual);

				}
			}

		}
		if (all){
			tab->Grid->CopyDialogue(sels[i])->SetText(txt);
		}
		else{
			Dialogue Cpy = Dialogue(*Dial);
			if (istexttl) {
				Cpy.TextTl = txt;
				wxString tlLines;
				if(showOriginalOnVideo)
					Cpy.GetRaw(&tlLines, false, tlModeStyle);

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
		tab->Video->hasVisualEdition = true;
		if (tab->Edit->splittedTags){ tab->Edit->TextEditOrig->SetModified(); }
		tab->Grid->SetModified(VISUAL_POSITION_SHIFTER, true);
		tab->Grid->Refresh();
	}
	else{
		RenderSubs(dtxt);
	}

}

void MoveAll::ChangeTool(int _tool)
{
	if (selectedTags == _tool)
		return;

	selectedTags = _tool;
	if ((_tool & TAGPOS || _tool & TAGMOVES || _tool & TAGMOVEE) && _tool & TAGP){
		selectedTags ^= TAGP;
	}
	tab->Video->Render(false);
}

void MoveAll::OnKeyPress(wxKeyEvent &evt)
{
	int key = evt.GetKeyCode();
	bool left = key == 'A';
	bool right = key == 'D';
	bool up = key == 'W';
	bool down = key == 'S';

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
		directionX = ((directionX / coeffW) - zoomMove.x) * zoomScale.x;
		directionY = ((directionY / coeffH) - zoomMove.y) * zoomScale.y;
		for (size_t j = 0; j < elems.size(); j++){
			if (!(selectedTags & elems[j].type)){ continue; }
			if (numElem == -1){
				numElem = j;
			}
			beforeMove = elems[j].elem;
			elems[j].elem.x += directionX;
			elems[j].elem.y += directionY;
		}
		if (numElem != -1)
			ChangeInLines(true);
		return;
	}
	evt.Skip();
}