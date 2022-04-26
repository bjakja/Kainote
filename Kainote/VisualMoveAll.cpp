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
#include "VideoBox.h"
#include "SubsGrid.h"
#include "EditBox.h"
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
		byte elemtype = elems[i]->type;
		if (!(selectedTags & elemtype)){ continue; }
		if (elemtype == TAGPOS || elemtype == TAGMOVES){
			DrawRect(elems[i]->elem);
		}
		else if (elemtype == TAGMOVEE){
			DrawCircle(elems[i]->elem);
		}
		else if (elemtype == TAGORG) {
			DrawCross(elems[i]->elem, 0xFF8800FF);
		}
		else{
			//draw shape here
			auto * vectorPoints = elems[i]->vectorPoints;
			if (vectorPoints) {
				size_t size = vectorPoints->size();
				if (elems[i]->type == TAGP && moveValues[6] > 2) {
					D3DXVECTOR2 movePos = CalcMovePos();
					drawingPos.x = (((movePos.x / drawingScale.x)/* - zoomMove.x*/) * zoomScale.x);
					drawingPos.y = (((movePos.y / drawingScale.y)/* - zoomMove.y*/) * zoomScale.y);
				}
				//cannot let to "l" or "b" to be used as missing "m" on start
				if (vectorPoints->at(0).type != L"m") { vectorPoints->at(0).type = L"m"; }
				size_t g = (size < 2) ? 0 : 1;
				size_t lastM = 0;
				while (g < size) {

					if (vectorPoints->at(g).type == L"l") {
						DrawLine(g, vectorPoints);
						g++;
					}
					else if (vectorPoints->at(g).type == L"b" || vectorPoints->at(g).type == L"s") {
						g += DrawCurve(g, vectorPoints, (vectorPoints->at(g).type == L"s"));
					}
					else if (vectorPoints->at(g).type != L"m") {
						g++;
					}

					if (g >= size || vectorPoints->at(g).type == L"m") {

						if (g > 1) {
							line->Begin();
							D3DXVECTOR2 v2[2] = { GetVector(vectorPoints->at(g - 1)), GetVector(vectorPoints->at(lastM)) };
							line->Draw(v2, 2, 0xFFBB0000);
							line->End();
						}
						lastM = g;
						g++;
					}

				}
			}
			D3DXCOLOR col = (elemtype == TAGCLIP) ? 0xFF0000FF : 0xFFFF00FF;
			DrawCross(elems[i]->elem, col);
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
		if (tab->video->HasCapture()){
			tab->video->ReleaseMouse();
		}
		drawingOriginalPos = D3DXVECTOR2(0, 0);
		if (numElem >= 0){ ChangeInLines(true); }
		if (!tab->video->HasArrow()){ tab->video->SetCursor(wxCURSOR_ARROW);}
		numElem = -1;
	}

	if (click){

		for (size_t i = 0; i < elems.size(); i++){
			if (!(selectedTags & elems[i]->type)){ continue; }
			if (fabs(elems[i]->elem.x - x) < 8 && fabs(elems[i]->elem.y - y) < 8 ||
				i == elems.size() - 1){
				numElem = i;
				beforeMove = lastmove = elems[i]->elem;
				diffs.x = elems[i]->elem.x - x;
				diffs.y = elems[i]->elem.y - y;
				if (!tab->video->HasCapture()){
					tab->video->CaptureMouse();
				}
			}
		}
		firstmove = D3DXVECTOR2(x, y);
		axis = 0;
	}
	else if (evt.RightDown()){

		for (size_t i = 0; i < elems.size(); i++){
			if (!(selectedTags & elems[i]->type)){ continue; }
			numElem = i;
			beforeMove = lastmove = elems[i]->elem;
			diffs.x = elems[i]->elem.x - x;
			diffs.y = elems[i]->elem.y - y;
			if (!tab->video->HasCapture()){
				tab->video->CaptureMouse();
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
			lastmove = elems[numElem]->elem;
			if (axis == 1){
				elems[numElem]->elem.x = x + diffs.x;
				elems[numElem]->elem.y = beforeMove.y;
			}
			else if (axis == 2){
				elems[numElem]->elem.y = y + diffs.y;
				elems[numElem]->elem.x = beforeMove.x;
			}
			D3DXVECTOR2 moving = elems[numElem]->elem - lastmove;
			for (size_t j = 0; j < elems.size(); j++){
				if (j == numElem || !(selectedTags & elems[j]->type)){ continue; }
				if (axis == 1){
					elems[j]->elem.x += moving.x;
				}
				else if (axis == 2){
					elems[j]->elem.y += moving.y;
				}
			}
		}
		else{
			lastmove = elems[numElem]->elem;
			elems[numElem]->elem.x = x + diffs.x;
			elems[numElem]->elem.y = y + diffs.y;

			D3DXVECTOR2 moving = elems[numElem]->elem - lastmove;
			for (size_t j = 0; j < elems.size(); j++){
				if (j == numElem || !(selectedTags & elems[j]->type)){ continue; }
				elems[j]->elem.x += moving.x;
				elems[j]->elem.y += moving.y;
			}

		}
		ChangeInLines(false);
	}

}

void MoveAll::SetCurVisual()
{
	byte alignment = 1;
	drawingPos = GetPosnScale(&scale, &alignment, moveValues);
	if (moveValues[6] > 3){ drawingPos = CalcMovePos(); }
	from = to = D3DXVECTOR2(((drawingPos.x / coeffW) - zoomMove.x) * zoomScale.x,
		((drawingPos.y / coeffH) - zoomMove.y) * zoomScale.y);
	
	Clear();

	double orx = drawingPos.x, ory = drawingPos.y;
	if (FindTag(L"org\\(([^\\)]+)")){
		
		if (GetTwoValueDouble(&orx, &ory)) {
			moveElems* orgelem = new moveElems(
				D3DXVECTOR2(((orx / coeffW) - zoomMove.x) * zoomScale.x,
				((ory / coeffH) - zoomMove.y) * zoomScale.y),
				TAGORG);
			elems.push_back(orgelem);
		}
	}
	if (FindTag(L"(i?clip[^\\)]+)", emptyString, 1)){
		const FindData& data = GetResult();
		wxRegEx re(L"m ([0-9.-]+) ([0-9.-]+)", wxRE_ADVANCED);
		moveElems* elem = new moveElems();
		drawingOriginalPos = D3DXVECTOR2(0, 0);
		int repl = data.finding.Freq(L',');
		if (re.Matches(data.finding)){
			elem->elem = D3DXVECTOR2(((wxAtoi(re.GetMatch(data.finding, 1)) / coeffW) - zoomMove.x) * zoomScale.x,
				((wxAtoi(re.GetMatch(data.finding, 2)) / coeffH) - zoomMove.y) * zoomScale.y);
			std::vector<ClipPoint>* points = new std::vector<ClipPoint>();
			if (repl > 0) {
				wxString clipWithoutScale;
				wxString vscalestr = data.finding.BeforeFirst(L',', &clipWithoutScale);
				int vscale = wxAtoi(vscalestr.AfterFirst(L'('));
				if (vscale > 1)
					vectorClipScale = pow(2, (vscale - 1));
			}
			GetVectorPoints(data.finding, points);
			
			if (points->size()) {
				for (size_t i = 0; i < points->size(); i++) {
					//calculate points for drawing
					//weird but zoomMove have to be last
					(*points)[i].x = ((((*points)[i].x / coeffW) * zoomScale.x) / vectorClipScale) - (zoomMove.x * zoomScale.x);
					(*points)[i].y = ((((*points)[i].y / coeffH) * zoomScale.y) / vectorClipScale) - (zoomMove.y * zoomScale.y);
				}
				elem->vectorPoints = points;
			}
			else
				delete points;
		}
		else{
			wxRegEx re(L"\\(([0-9.-]+)[, ]*([0-9.-]+)[, ]*([0-9.-]+)[, ]*([0-9.-]+)", wxRE_ADVANCED);
			if (repl >= 3 && re.Matches(data.finding)){
				wxString point1 = re.GetMatch(data.finding, 1);
				wxString point2 = re.GetMatch(data.finding, 2);
				wxString point3 = re.GetMatch(data.finding, 3);
				wxString point4 = re.GetMatch(data.finding, 4);
				float pt1 = wxAtof(point1);
				float pt2 = wxAtof(point2);
				float pt3 = wxAtof(point3);
				float pt4 = wxAtof(point4);
				elem->elem = D3DXVECTOR2(((pt1 / coeffW) - zoomMove.x) * zoomScale.x,
					((pt2 / coeffH) - zoomMove.y) * zoomScale.y);
				pt3 = ((pt3 / coeffW) - zoomMove.x) * zoomScale.x;
				pt4 = ((pt4 / coeffH) - zoomMove.y) * zoomScale.y;

				std::vector<ClipPoint>* points = new std::vector<ClipPoint>();
				points->push_back(ClipPoint(elem->elem.x, elem->elem.y, "m", true));
				points->push_back(ClipPoint(pt3, elem->elem.y, "l", true));
				points->push_back(ClipPoint(pt3, pt4, "l", true));
				points->push_back(ClipPoint(elem->elem.x, pt4, "l", true));
				elem->vectorPoints = points;
			}
			
		}
		elem->type = TAGCLIP;
		elems.push_back(elem);
		drawingPos = D3DXVECTOR2(0, 0);
	}
	if (FindTag(L"p([0-9]+)", emptyString, 1)){
		wxString tags[] = { L"p" };
		wxString res;
		ParseData* pdata = tab->edit->line->ParseTags(tags, 1);
		if (pdata->tags.size() >= 2) {
			size_t i = 1;
			while (i < pdata->tags.size()) {
				TagData* tag = pdata->tags[i];
				if (tag->tagName == L"p") {
					int vscale = wxAtoi(tag->value);
					if (vscale > 1) {
						vectorDrawScale = pow(2, (vscale - 1));
					}
				}
				if (tag->tagName == L"pvector") {
					res = tag->value;
					break;
				}
				i++;
			}
		}
		tab->edit->line->ClearParse();
		wxRegEx re(L"m ([.0-9-]+) ([.0-9-]+)", wxRE_ADVANCED);
		if (re.Matches(res)){
			moveElems* elem = new moveElems();
			std::vector<ClipPoint>* points = new std::vector<ClipPoint>();
			GetVectorPoints(res, points);
			//drawing pos and coeff need division by scale 
			drawingScale.x = coeffW / scale.x;
			drawingScale.y = coeffH / scale.y;
			drawingPos.x /= scale.x;
			drawingPos.y /= scale.y;
			if (!points->empty()) {
				drawingOriginalPos = D3DXVECTOR2(0, 0);
				//frz to get rotation z
				float frz = 0;
				if (FindTag(L"frz?([0-9.-]+)")) {
					double frzd = 0.;
					GetDouble(&frzd);
					frz = frzd;
				}
				else {
					Styles* actualStyle = tab->grid->GetStyle(0, tab->edit->line->Style);
					double result = 0.;
					actualStyle->Angle.ToDouble(&result);
					frz = result;
				}
				
				//offset for different an than 7
				D3DXVECTOR2 offsetxy = CalcDrawingSize(alignment, points);
				float rad = 0.01745329251994329576923690768489f;
				//org pivot is origin position of frz rotation
				D3DXVECTOR2 orgpivot = { abs(((float)orx / scale.x) - drawingPos.x), 
					abs(((float)ory / scale.y) - drawingPos.y) };
				float s = sin(-frz * rad);
				float c = cos(-frz * rad);
				for (size_t i = 0; i < points->size(); i++) {
					(*points)[i].x -= offsetxy.x;
					(*points)[i].y -= offsetxy.y;
					if(frz)
						RotateDrawing(&(*points)[i], s, c, orgpivot);
					//calculate points for drawing
					(*points)[i].x = (((((*points)[i].x) / drawingScale.x) - zoomMove.x) * zoomScale.x) / vectorClipScale;
					(*points)[i].y = (((((*points)[i].y) / drawingScale.y) - zoomMove.y) * zoomScale.y) / vectorClipScale;
				}
				
			}
			if (points->size())
				elem->vectorPoints = points;
			else
				delete points;
			float firstPointx = wxAtof(re.GetMatch(res, 1));
			float firstPointy = wxAtof(re.GetMatch(res, 2));

			elem->elem = D3DXVECTOR2((((firstPointx + drawingPos.x) /
				drawingScale.x) - zoomMove.x) * zoomScale.x,
				(((firstPointy + drawingPos.y) /
					drawingScale.y) - zoomMove.y) * zoomScale.y);
			elem->type = TAGP;
			elems.push_back(elem);
			//zoomMove is not needed cause it's already subtract from points
			drawingPos.x = (((drawingPos.x / drawingScale.x)/* - zoomMove.x*/) * zoomScale.x);
			drawingPos.y = (((drawingPos.y / drawingScale.y)/* - zoomMove.y*/) * zoomScale.y);
		}

	}
	if (moveValues[6] == 2){
		moveElems* elem = new moveElems();
		elem->elem = D3DXVECTOR2(((moveValues[0] / coeffW) - zoomMove.x) * zoomScale.x,
			((moveValues[1] / coeffH) - zoomMove.y) * zoomScale.y);
		elem->type = TAGPOS;
		elems.push_back(elem);
	}
	if (moveValues[6] >= 4){
		moveElems* elem = new moveElems();
		elem->elem = D3DXVECTOR2(((moveValues[0] / coeffW) - zoomMove.x) * zoomScale.x,
			((moveValues[1] / coeffH) - zoomMove.y) * zoomScale.y);
		elem->type = TAGMOVES;
		elems.push_back(elem);
		moveElems* elem1 = new moveElems();
		elem1->type = TAGMOVEE;
		elem1->elem = D3DXVECTOR2(((moveValues[2] / coeffW) - zoomMove.x) * zoomScale.x,
			((moveValues[3] / coeffH) - zoomMove.y) * zoomScale.y);
		elems.push_back(elem1);
	}

}

void MoveAll::ChangeInLines(bool all)
{
	bool showOriginalOnVideo = !Options.GetBool(TL_MODE_HIDE_ORIGINAL_ON_VIDEO);
	//D3DXVECTOR2 moving;
	D3DXVECTOR2 moving = elems[numElem]->elem - beforeMove;
	drawingOriginalPos = moving;
	int _time = tab->video->Tell();
	wxArrayInt sels;
	tab->grid->file->GetSelections(sels);
	wxString *dtxt;
	if (!all){
		if (!dummytext || selPositions.size() != sels.size()){
			selPositions.clear();
			bool visible = false;
			// release dummy text when sels size is different
			SAFE_DELETE(dummytext);
			dummytext = tab->grid->GetVisible(&visible, 0, &selPositions);
			if (selPositions.size() != sels.size()){
				//KaiLog(L"Sizes mismatch");
				return;
			}
		}

		dtxt = new wxString(*dummytext);
	}
	bool skipInvisible = !all && tab->video->GetState() != Playing;
	wxString tmp;
	//bool isOriginal=(tab->Grid1->transl && tab->edit->TextEdit->GetValue()==emptyString);
	//MTextEditor *GLOBAL_EDITOR=(isOriginal)? tab->edit->TextEditTl : tab->edit->TextEdit;
	//wxString origText=GLOBAL_EDITOR->GetValue();
	const wxString &tlModeStyle = tab->grid->GetSInfo(L"TLMode Style");
	int moveLength = 0;

	for (size_t i = 0; i < sels.size(); i++){
		wxString txt;
		Dialogue *Dial = tab->grid->GetDialogue(sels[i]);

		if (skipInvisible && !(_time >= Dial->Start.mstime && _time <= Dial->End.mstime)){ continue; }
		bool istexttl = (tab->grid->hasTLMode && Dial->TextTl != emptyString);
		txt = (istexttl) ? Dial->TextTl : Dial->Text;

		for (int k = 0; k < 6; k++){
			byte type = selectedTags & (1 << k);
			if (!type){ continue; }
			bool vector = type == TAGCLIP || type == TAGP;
			float newcoeffW = type == TAGP ? coeffW / scale.x : coeffW;
			float newcoeffH = type == TAGP ? coeffH / scale.y : coeffH;
			float vectorScale = 1.f;
			wxString delimiter = (vector) ? L" " : L",";
			wxString tagpattern = (type == TAGPOS) ? L"pos\\(([^\\)]+)" : 
				(type == TAGORG) ? L"org\\(([^\\)]+)" : (type == TAGCLIP) ? L"i?clip\\(([^\\)]+)" : 
				(type == TAGP) ? L"p[0-9-]+[^}]*} ?m ([^{]+)" : L"move\\(([^\\)]+)";
			wxRegEx re(tagpattern, wxRE_ADVANCED);
			size_t startMatch = 0, lenMatch = 0;
			size_t textPosition = 0;
			while (re.Matches(txt.Mid(textPosition))){
				//reseting vector scale
				moving = drawingOriginalPos;
				wxString visual;
				//tmp = re.GetMatch(txt, 1);
				if (re.GetMatch(&startMatch, &lenMatch, 1)) {
					tmp = txt.Mid(startMatch + textPosition, lenMatch);
					if (type == TAGCLIP){
						int replacements = tmp.Freq(L',');
						if (replacements == 1){
							wxString vectorclip = tmp;
							//set a scale 
							wxString clipScale = vectorclip.BeforeFirst(L',', &tmp);
							visual = clipScale + L",";
							int cscale = wxAtoi(clipScale);
							vectorScale = pow(2, (cscale - 1));
							moving = drawingOriginalPos * vectorScale;
						}
						else if (replacements > 1){
							delimiter = L",";
						}
					}
					wxStringTokenizer tkn(tmp, delimiter, wxTOKEN_STRTOK);
					int count = 0;
					while (tkn.HasMoreTokens()){
						wxString token = tkn.GetNextToken().Trim().Trim(false);
						double val;
						if (token.ToDouble(&val)){
							if (count % 2 == 0){ val += (((moving.x / zoomScale.x)) * newcoeffW); }
							else{ val += (((moving.y / zoomScale.y)) * newcoeffH); }
							if (type == TAGMOVES && count > 1){ 
								visual += token + delimiter; 
								continue; 
							}
							else if (type == TAGMOVEE && count != 2 && count != 3){ 
								visual += token + delimiter; 
								count++; 
								continue; 
							}
							if (vector){ 
								visual << getfloat(val, (type == TAGCLIP) ? L"6.0f" : L"6.2f") << delimiter; 
							}
							else{ 
								visual += getfloat(val) + delimiter; 
							}
							count++;
						}
						else{
							visual += token + delimiter;
							if (!vector){ count++; }
						}
					}
			
					visual.RemoveLast();
					if (lenMatch){ txt.erase(txt.begin() + startMatch + textPosition, txt.begin() + startMatch + lenMatch + textPosition); }
					txt.insert(startMatch + textPosition, visual);

				}
				else {
					//if cannot get positions the loop will be looping forever
					//to prevent just increase text position
					textPosition++;
				}
				textPosition += startMatch + lenMatch;
			}

		}
		if (all){
			tab->grid->CopyDialogue(sels[i])->SetText(txt);
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
		SetModified(VISUAL_POSITION_SHIFTER);
	}
	else{
		RenderSubs(dtxt);
	}

}

void MoveAll::ChangeTool(int _tool, bool blockSetCurVisual)
{
	if (selectedTags == _tool)
		return;

	selectedTags = _tool;
	if ((_tool & TAGPOS || _tool & TAGMOVES || _tool & TAGMOVEE) && _tool & TAGP){
		selectedTags ^= TAGP;
	}
	tab->video->Render(false);
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
		directionX = (directionX / coeffW)/* - zoomMove.x) * zoomScale.x*/;
		directionY = (directionY / coeffH)/* - zoomMove.y) * zoomScale.y*/;
		numElem = -1;
		for (size_t j = 0; j < elems.size(); j++){
			if (!(selectedTags & elems[j]->type)){ continue; }
			if (numElem == -1) {
				numElem = j;
				beforeMove = elems[j]->elem;
			}
			elems[j]->elem.x += directionX;
			elems[j]->elem.y += directionY;
		}
		if (numElem != -1)
			ChangeInLines(true);
		return;
	}
	evt.Skip();
}

int MoveAll::DrawCurve(int i, std::vector<ClipPoint>* vectorPoints, bool bspline)
{
	line->Begin();
	std::vector<D3DXVECTOR2> v4;

	int pts = 3;
	if (bspline) {

		int acpos = i - 1;
		int bssize = 1;
		int spos = i + 1;
		while (spos < (int)vectorPoints->size()) {
			if (vectorPoints->at(spos).start) { break; }
			bssize++; spos++;
		}
		pts = bssize;
		bssize++;
		for (int k = 0; k < bssize; k++) {
			Curve(acpos, vectorPoints, &v4, true, bssize, k);
		}
		D3DXVECTOR2* v2 = new D3DXVECTOR2[pts + 2];
		for (int j = 0, g = i - 1; j < bssize; j++, g++){
			v2[j] = GetVector(vectorPoints->at(g));
		}
		v2[bssize] = GetVector(vectorPoints->at(i - 1));
		line->Draw(v2, pts + 2, 0xFFAA33AA);
		int iplus1 = (i + bssize - 2 < (int)vectorPoints->size() - 1) ? i + 1 : 0;
		if (i - 1 != 0 || iplus1 != 0) {
			D3DXVECTOR2 v3[3] = { GetVector(vectorPoints->at(i - 1)), v4[0], 
				GetVector(vectorPoints->at(iplus1)) };
			line->Draw(v3, 3, 0xFFBB0000);
		}
		delete[] v2;
	}
	else {
		ClipPoint tmp = vectorPoints->at(i - 1);
		if (tmp.type == L"s") {
			int diff = 2;
			int j = i - 2;
			while (j >= 0) {
				if (vectorPoints->at(j).type != L"s") { break; }
				j--;
			}
			diff = (i - j) - 2;
			vectorPoints->at(i - 1) = vectorPoints->at(i - diff);
		}
		Curve(i - 1, vectorPoints, &v4, false);
		vectorPoints->at(i - 1) = tmp;
	}
	line->Draw(&v4[0], v4.size(), 0xFFBB0000);
	line->End();
	return pts;
}

void MoveAll::Curve(int pos, std::vector<ClipPoint>* vectorPoints, std::vector<D3DXVECTOR2>* table, bool bspline, int nBsplinePoints, int currentPoint)
{
	float a[4], b[4];
	float x[4], y[4];
	for (int g = 0; g < 4; g++)
	{
		if (currentPoint > (nBsplinePoints - 1)) { currentPoint = 0; }
		D3DXVECTOR2 point = GetVector((*vectorPoints)[pos + currentPoint]);
		x[g] = point.x;
		y[g] = point.y;
		currentPoint++;
	}

	if (bspline) {
		a[3] = (-x[0] + 3 * x[1] - 3 * x[2] + x[3]) / 6.0;
		a[2] = (3 * x[0] - 6 * x[1] + 3 * x[2]) / 6.0;
		a[1] = (-3 * x[0] + 3 * x[2]) / 6.0;
		a[0] = (x[0] + 4 * x[1] + x[2]) / 6.0;
		b[3] = (-y[0] + 3 * y[1] - 3 * y[2] + y[3]) / 6.0;
		b[2] = (3 * y[0] - 6 * y[1] + 3 * y[2]) / 6.0;
		b[1] = (-3 * y[0] + 3 * y[2]) / 6.0;
		b[0] = (y[0] + 4 * y[1] + y[2]) / 6.0;
	}
	else {
		a[3] = -x[0] + 3 * x[1] - 3 * x[2] + x[3];
		a[2] = 3 * x[0] - 6 * x[1] + 3 * x[2];
		a[1] = -3 * x[0] + 3 * x[1];
		a[0] = x[0];
		b[3] = -y[0] + 3 * y[1] - 3 * y[2] + y[3];
		b[2] = 3 * y[0] - 6 * y[1] + 3 * y[2];
		b[1] = -3 * y[0] + 3 * y[1];
		b[0] = y[0];
	}

	float maxaccel1 = fabs(2 * b[2]) + fabs(6 * b[3]);
	float maxaccel2 = fabs(2 * a[2]) + fabs(6 * a[3]);
	float maxaccel = maxaccel1 > maxaccel2 ? maxaccel1 : maxaccel2;
	float h = 1.0f;
	if (maxaccel > 4.0f) h = sqrt(4.0f / maxaccel);
	float p_x, p_y;
	for (float t = 0; t < 1.0; t += h)
	{
		p_x = a[0] + t * (a[1] + t * (a[2] + t * a[3]));
		p_y = b[0] + t * (b[1] + t * (b[2] + t * b[3]));
		table->push_back(D3DXVECTOR2(p_x, p_y));
	}
	p_x = a[0] + a[1] + a[2] + a[3];
	p_y = b[0] + b[1] + b[2] + b[3];
	table->push_back(D3DXVECTOR2(p_x, p_y));
}

D3DXVECTOR2 MoveAll::GetVector(const ClipPoint& point)
{
	return D3DXVECTOR2(point.x + drawingPos.x + drawingOriginalPos.x, point.y + drawingPos.y + drawingOriginalPos.y);
}

void MoveAll::DrawLine(int i, std::vector<ClipPoint>* vectorPoints)
{
	line->Begin();
	int diff = 1;
	if (vectorPoints->at(i - 1).type == L"s") {
		int j = i - 2;
		while (j >= 0) {
			if (vectorPoints->at(j).type != L"s") { break; }
			j--;
		}
		diff = (i - j) - 2;
	}
	D3DXVECTOR2 v2[2] = { GetVector(vectorPoints->at(i - diff)), GetVector(vectorPoints->at(i)) };
	line->Draw(v2, 2, 0xFFBB0000);
	line->End();
}

void MoveAll::Clear()
{
	for (auto cur = elems.begin(); cur != elems.end(); cur++) {
		delete (*cur);
	}
	elems.clear();
}


