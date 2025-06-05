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

#include "VisualDrawingShapes.h"
#include "VisualClipPoint.h"
#include "VisualClips.h"
#include "TabPanel.h"
#include "SubsGrid.h"
#include "KaiTextCtrl.h"
#include "EditBox.h"
#include "VideoBox.h"
#include "Provider.h"
#include "RendererVideo.h"
#include "VideoToolbar.h"
#include <wx/tokenzr.h>
#include <wx/regex.h>
#include <math.h> 
#include "KaiMessageBox.h"



ClipPoint::ClipPoint()
{
	x = 0;
	y = 0;
	type = L"m";
	start = true;
	isSelected = false;
}

ClipPoint::ClipPoint(float _x, float _y, wxString _type, bool isstart)
{
	x = _x;
	y = _y;
	type = _type;
	start = isstart;
	isSelected = false;
}

bool ClipPoint::IsInPos(D3DXVECTOR2 pos, float diff)
{
	return (abs(pos.x - x) < diff && abs(pos.y - y) < diff);
}

D3DXVECTOR2 ClipPoint::GetVector(DrawingAndClip *parent)
{
	D3DXVECTOR2 v = D3DXVECTOR2((((x + parent->_x) / parent->coeffW) - parent->zoomMove.x) * parent->zoomScale.x,
		(((y + parent->_y) / parent->coeffH) - parent->zoomMove.y) * parent->zoomScale.y);
	return v;
}

float ClipPoint::wx(DrawingAndClip *parent, bool zoomConversion)
{
	float wx = ((x + parent->_x) / parent->coeffW);
	if (zoomConversion){ wx = (wx - parent->zoomMove.x) * parent->zoomScale.x; }
	return wx;
}
float ClipPoint::wy(DrawingAndClip *parent, bool zoomConversion)
{
	float wy = ((y + parent->_y) / parent->coeffH);
	if (zoomConversion){ wy = (wy - parent->zoomMove.y) * parent->zoomScale.y; }
	return wy;
}

DrawingAndClip::DrawingAndClip()
	:Visuals()
	, drawtxt(false)
	, invClip(false)
	, drawSelection(false)
	, drawToolLines(false)
	, grabbed(-1)
	, tool(1)
	, vectorScale(1)
{
}

DrawingAndClip::~DrawingAndClip()
{
	Points.clear();
}

void DrawingAndClip::DrawVisual(int time)
{
	if (drawSelection){
		D3DXVECTOR2 v5[5] = { D3DXVECTOR2(selection.x, selection.y), 
			D3DXVECTOR2(selection.width, selection.y), D3DXVECTOR2(selection.width, selection.height), 
			D3DXVECTOR2(selection.x, selection.height), D3DXVECTOR2(selection.x, selection.y) };
		line->Begin();
		DrawDashedLine(v5, 5);
		line->End();
	}
	size_t size = Points.size();
	if (!size){ return; }

	line->SetWidth(1.0f);
	if (drawToolLines){
		int mPoint = FindPoint(size - 1, L"m", false, true);
		if (mPoint >= 0 && mPoint < (int)size - 1){
			D3DXVECTOR2 v3[3] = { Points[mPoint].GetVector(this), D3DXVECTOR2(x, y), Points[size - 1].GetVector(this) };
			line->Begin();
			DrawDashedLine(v3, (Points[size - 1].type != L"m") ? 3 : 2);
			line->End();
		}
	}
	if (Visual == VECTORDRAW && moveValues[6] > 2){ 
		D3DXVECTOR2 movePos = CalcMovePos(); 
		_x = movePos.x / scale.x;
		_y = movePos.y / scale.y;
	}
	//cannot let to "l" or "b" to be used as missing "m" on start
	if (Points[0].type != L"m"){ Points[0].type = L"m"; }
	size_t g = (size < 2) ? 0 : 1;
	size_t lastM = 0;
	while (g < size){

		if (Points[g].type == L"l"){
			DrawLine(g);
			g++;
		}
		else if (Points[g].type == L"b" || Points[g].type == L"s"){
			g += DrawCurve(g, (Points[g].type == L"s"));
		}
		else if (Points[g].type != L"m"){
			g++;
		}

		if (g >= size || Points[g].type == L"m"){

			if (g < size && Points[g].type == L"m"){ DrawRect(g); }

			if (g > 1){
				line->Begin();
				D3DXVECTOR2 v2[2] = { Points[g - 1].GetVector(this), Points[lastM].GetVector(this) };
				line->Draw(v2, 2, 0xFFBB0000);
				line->End();
				DrawRect(lastM);
				DrawRect(g - 1);

			}
			lastM = g;
			g++;
		}

	}
	if (lastpos >= 0 && lastpos < (int)Points.size()){
		D3DXVECTOR2 pos = Points[lastpos].GetVector(this);
		int rcsize = 3;
		VERTEX v9[4];
		D3DCOLOR color(0xAACC8748);
		CreateVERTEX(&v9[0], pos.x - rcsize, pos.y - rcsize, color);
		CreateVERTEX(&v9[1], pos.x + rcsize, pos.y - rcsize, color);
		CreateVERTEX(&v9[2], pos.x - rcsize, pos.y + rcsize, color);
		CreateVERTEX(&v9[3], pos.x + rcsize, pos.y + rcsize, color);
		HRN(device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, v9, sizeof(VERTEX)), L"primitive failed");
	}
	if (drawCross){
		D3DXVECTOR2 v2[2] = { D3DXVECTOR2(x, 0), D3DXVECTOR2(x, this->VideoSize.GetHeight()) };
		D3DXVECTOR2 v21[2] = { D3DXVECTOR2(0, y), D3DXVECTOR2(this->VideoSize.GetWidth(), y) };
		line->Begin();
		DrawDashedLine(v2, 2, 4, 0xFFFF00FF);
		DrawDashedLine(v21, 2, 4, 0xFFFF00FF);
		line->End();
	}

}


void DrawingAndClip::SetCurVisual()
{
	D3DXVECTOR2 linepos = GetPosnScale(&scale, &alignment, (Visual == VECTORDRAW) ? moveValues : nullptr);
	bool isDrawing = false;
	wxString clip;
	if (Visual != VECTORDRAW){
		bool found = FindTag(L"(i?clip\\(.*m[^)]*)\\)", emptyString, 1);
		const FindData& data = GetResult();
		clip = data.finding;
		coeffW /= scale.x;
		coeffH /= scale.y;
		_x = 0;
		_y = 0;
		if (found){
			int rres = clip.Freq(L',');
			//it should not possible but sanity check is not too bad
			if (rres >= 3) { 
				clip = emptyString; 
				scale = D3DXVECTOR2(1.f, 1.f); 
				vectorScale = 1; 
			}
			else{ 
				clip = clip.AfterFirst(L'('); 
				if (rres >= 1) {
					wxString clip1;
					wxString vscale = clip.BeforeFirst(L',', &clip1);
					int vscaleint = wxAtoi(vscale);
					if (vscaleint > 0)
						vectorScale = vscaleint;

					CreateClipMask(clip);
					clip = clip1;
					goto done;
				}
			}
		}
		
		CreateClipMask(clip);
	}
	else{
		isDrawing = true;
		wxString tags[] = { L"p" };
		ParseData* pdata = tab->edit->line->ParseTags(tags, 1);
		if (pdata->tags.size() >= 2){
			size_t i = 1;
			while (i < pdata->tags.size()){
				TagData* tdata = pdata->tags[i];
				if (tdata->tagName == L"p") {
					int vscale = wxAtoi(tdata->value);
					if (vscale > 0) {
						vectorScale = vscale;
					}
				}
				else if (tdata->tagName == L"pvector"){
					//this table has every part of drawing if splited by tags before "m"
					clip = tdata->value;
					break;
				}
				i++;
			}
		}
		tab->edit->line->ClearParse();
		_x = linepos.x / scale.x;
		_y = (linepos.y / scale.y);
		coeffW /= scale.x;
		coeffH /= scale.y;
		wxString res;
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
		if (FindTag(L"org(\\([^\\)]+)")) {
			double orx, ory;
			if (GetTwoValueDouble(&orx, &ory)) {
				org = D3DXVECTOR2(orx / scale.x, ory / scale.y);
			}
			else { org = D3DXVECTOR2(_x, _y); }
		}
		else { org = D3DXVECTOR2(_x, _y); }
	}

done:

	Points.clear();
	GetVectorPoints(clip, &Points);
	
	if (!Points.empty() && Visual == VECTORDRAW){
		offsetxy = CalcDrawingSize(alignment, &Points);
		float rad = 0.01745329251994329576923690768489f;
		D3DXVECTOR2 orgpivot = { abs(org.x - _x), abs(org.y - _y) };
		float s = sin(-frz * rad);
		float c = cos(-frz * rad);
		for (size_t i = 0; i < Points.size(); i++){
			//divide points by scale to get original subtitle position
			Points[i].x -= offsetxy.x;
			Points[i].y -= offsetxy.y;
			if(frz)
				RotateDrawing(&Points[i], s, c, orgpivot);
		}
	}
	pointArea = 4.f / zoomScale.x;
}

void DrawingAndClip::GetVisual(wxString *visual)
{
	wxString format = (Visual == VECTORDRAW) ? L"6.2f" : L"6.0f";
	if (Visual == VECTORCLIP && vectorScale > 1){
		*visual << vectorScale << L",";
	}
	wxString lasttype;
	int countB = 0;
	bool spline = false;
	size_t psize = Points.size();
	std::vector<ClipPoint> originalPoints;
	if (Visual == VECTORDRAW) {
		if (frz) {
			float rad = 0.01745329251994329576923690768489f;
			D3DXVECTOR2 orgpivot = { abs(org.x - _x), abs(org.y - _y) };
			float s = sin(frz * rad);
			float c = cos(frz * rad);
			originalPoints = Points;
			for (size_t i = 0; i < psize; i++){
				RotateDrawing(&Points[i], s, c, orgpivot);
			}
		}
		offsetxy = CalcDrawingSize(alignment, &Points);
	}
	else {
		offsetxy = D3DXVECTOR2(0, 0);
	}
	
	
	for (size_t i = 0; i < psize; i++)
	{
		ClipPoint pos = Points[i];
		float x = pos.x + offsetxy.x;
		float y = pos.y + offsetxy.y;
		
		if (countB && !pos.start){
			*visual << getfloat(x, format) << L" " << getfloat(y, format) << L" ";
			countB++;
		}
		else{
			if (spline){ *visual << L"c "; spline = false; }
			if (lasttype != pos.type || pos.type == L"m"){ *visual << pos.type << L" "; lasttype = pos.type; }
			*visual << getfloat(x, format) << L" " << getfloat(y, format) << L" ";
			if (pos.type == L"b" || pos.type == L"s"){ 
				countB = 1; 
				if (pos.type == L"s")
					spline = true;
			}
		}
		//fix for m one after another
		if (pos.type == L"m" && psize > 1 && ((i >= psize - 1) ||
			(i < psize - 1 && Points[i + 1].type == L"m"))){
			*visual << L"l " << getfloat(x, format) << L" " << getfloat(y, format) << L" ";
		}
	}
	if (spline){ *visual << L"c "; }
	visual->Trim();
	if (originalPoints.size()) {
		Points = originalPoints;
	}
}

void DrawingAndClip::SetClip(bool dummy, bool redraw, bool changeEditorText)
{

	EditBox *edit = tab->edit;
	SubsGrid *grid = tab->grid;
	bool isOriginal = (grid->hasTLMode && edit->TextEdit->GetValue() == emptyString);
	//GLOBAL_EDITOR
	TextEditor *editor = (isOriginal) ? edit->TextEditOrig : edit->TextEdit;
	wxString clip;
	GetVisual(&clip);
	if (edit->IsCursorOnStart()) {
		bool showOriginalOnVideo = !Options.GetBool(TL_MODE_HIDE_ORIGINAL_ON_VIDEO);
		wxString *dtxt;
		wxArrayInt sels;
		grid->file->GetSelections(sels);
		bool skipInvisible = dummy && tab->video->GetState() != Playing;
		if ((dummy && !dummytext) || selPositions.size() != sels.size()) {
			bool visible = false;
			selPositions.clear();
			dummytext = grid->GetVisible(&visible, 0, &selPositions);
			if (selPositions.size() != sels.size()) {
				//KaiLog(L"Sizes mismatch");
				return;
			}
		}
		if (dummy) { dtxt = new wxString(*dummytext); }
		int _time = tab->video->Tell();
		int moveLength = 0;
		const wxString &tlStyle = tab->grid->GetSInfo(L"TLMode Style");
		for (size_t i = 0; i < sels.size(); i++) {

			Dialogue *Dial = grid->GetDialogue(sels[i]);
			if (skipInvisible && !(_time >= Dial->Start.mstime && _time <= Dial->End.mstime)) { continue; }

			wxString txt = Dial->GetTextNoCopy();
			ChangeVectorVisual(&txt, &clip);
			if (!dummy) {
				grid->CopyDialogue(sels[i])->SetText(txt);
			}
			else {
				Dialogue Cpy = Dialogue(*Dial);
				if (Dial->TextTl != emptyString && grid->hasTLMode) {
					Cpy.TextTl = txt;
					wxString tlLines;
					if (showOriginalOnVideo)
						Cpy.GetRaw(&tlLines, false, tlStyle);

					Cpy.GetRaw(&tlLines, true);
					dtxt->insert(selPositions[i] + moveLength, tlLines);
					moveLength += tlLines.length();
				}
				else {
					Cpy.Text = txt;
					wxString thisLine;
					Cpy.GetRaw(&thisLine);
					dtxt->insert(selPositions[i] + moveLength, thisLine);
					moveLength += thisLine.length();
				}
			}


		}

		if (!dummy) {
			tab->video->SetVisualEdition(true);
			if (edit->splittedTags) { edit->TextEditOrig->SetModified(); }
			//dummy = true cause of deselecting points with multiline editing
			//SetClip() should not be used
			grid->SetModified((Visual == VECTORCLIP) ? VISUAL_VECTOR_CLIP : VISUAL_DRAWING, true, true);
			grid->Refresh();
		}
		else {
			if (Visual == VECTORCLIP) {
				CreateClipMask(clip);
			}
			RenderSubs(dtxt);
		}
		return;
	}
	if (clip == emptyString && Visual == VECTORCLIP) {
		wxString txt = editor->GetValue();
		clipMask.Empty();
		if (FindTag(L"(i?clip\\(.*m[^)]*)\\)", txt, 1)) {
			Replace(emptyString, &txt);
			txt.Replace(L"{}", emptyString);
			if (changeEditorText) {
				editor->SetTextS(txt, false, true);
				editor->SetModified();
				edit->Send(VISUAL_VECTOR_CLIP, false);
			}
			return;
		}
		tab->video->SetVisualEdition(false);
		RenderSubs(tab->grid->GetVisible(), redraw);
		return;
	}
	if (dummy) {

		if (!dummytext) {
			bool vis = false;
			dummytext = grid->GetVisible(&vis, &textplaced);
			if (!vis) { 
				SAFE_DELETE(dummytext); 

				return; 
			}
			//vector clip
			if (Visual == VECTORCLIP) {
				wxString tmp = L"clip(";
				wxString txt = editor->GetValue();
				wxPoint pos;
				ChangeVectorVisual(&txt, &clip, &pos, &tmp);

				dummytext->replace(textplaced.x, textplaced.y, txt);
				textplaced.y = txt.length();
				CreateClipMask(clip, &tmp);
				dumplaced.x = pos.x + textplaced.x;
				dumplaced.y = pos.y + textplaced.x;
				if (changeEditorText) {
					editor->SetTextS(txt, false, true);
					editor->SetModified();
				}

			}
			else {//vector drawings
				size_t cliplen = clip.length();
				wxString txt = editor->GetValue();
				wxPoint pos;
				//pos here has length instead second pos
				ChangeVectorVisual(&txt, &clip, &pos);

				if (changeEditorText) {
					editor->SetTextS(txt, false, true);
					editor->SetModified();
				}

				dummytext->replace(textplaced.x, textplaced.y, txt);
				textplaced.y = txt.length();
				dumplaced.x = pos.x + textplaced.x;
				dumplaced.y = dumplaced.x + cliplen;

			}
		}
		else {
			//clip change
			dummytext->replace(dumplaced.x, dumplaced.y - dumplaced.x, clip);
			//new positions for new change
			int oldy = dumplaced.y;
			dumplaced.y = dumplaced.x + clip.length();
			textplaced.y += (dumplaced.y - oldy);
			if (Visual == VECTORCLIP) {
				//change mask clip
				int endclip = clipMask.Find(L')', true);
				int startclip = clipMask.Find(L'(', true);
				clipMask.replace(startclip + 1, endclip - (startclip + 1), clip);
			}
			//get line text
			wxString txt = dummytext->Mid(textplaced.x, textplaced.y);
			//put in text field
			if (changeEditorText) {
				editor->SetTextS(txt, false, true);
				editor->SetModified();
			}
		}

		//tab->video->SetVisualEdition(false);
		wxString *dtxt = new wxString(*dummytext);
		RenderSubs(dtxt, redraw);

	}
	else {

		editor->SetModified();
		edit->UpdateChars();
		tab->video->SetVisualEdition(true);
		if (edit->splittedTags) { edit->TextEditOrig->SetModified(); }
		edit->Send((Visual == VECTORCLIP) ? VISUAL_VECTOR_CLIP : VISUAL_DRAWING, false, false, true);
	}
}

void DrawingAndClip::ChangeVectorVisual(wxString *txt, wxString *clip, wxPoint* changePos, wxString* clipMaskTag)
{

	if (Visual == VECTORCLIP) {
		wxString tmp = L"clip(";
		//check if it has vector clip, rectangle clip is skipped, 
		//not used cause it can be along with vector clip in one line
		bool fv = FindTag(L"(i?clip\\(.*m[^)]*)\\)", *txt, 1);
		GetTextResult(&tmp);
		//probably not used, but better to leave it as is to avoid some rare cases.
		if (clip->empty() && fv) {
			Replace(emptyString, txt);
			txt->Replace(L"{}", emptyString);
			return;
		}
		wxString restClip;
		wxString clipName = tmp.BeforeFirst(L'(', &restClip) + L"(";

		wxString tclip = L"\\" + clipName + *clip + L")";
		int move = Replace(tclip, txt);
		if (changePos) {
			if(clipMaskTag)
				*clipMaskTag = (clipName[0] == L'c') ? L"iclip(" : L"clip(";

			changePos->x = GetPositionInText().x;
			changePos->x += clipName.length() + 1 + move;
			changePos->y = changePos->x + clip->length();
		}
	}
	else {//vector drawings
		//wxString tmp = emptyString;
		bool isf;
		bool hasP1 = true;
		size_t cliplen = clip->length();
		isf = FindTag(L"p([0-9]+)", *txt, 1);
		if (!isf) {
			Replace(L"\\p1", txt);
			hasP1 = false;
		}
		isf = FindTag(L"pos|move\\(([,. 0-9-]+)\\)", *txt, 1);
		if (!isf) {
			float xx = _x * scale.x;
			float yy = _y * scale.y;
			Replace(L"\\pos(" + getfloat(xx) + L"," + getfloat(yy) + L")", txt);
		}
		isf = FindTag(L"an([0-9])", *txt, 1);
		if (!isf) {
			Replace(L"\\an" + getfloat(alignment, L"1.0f"), txt);
		}

		int bracketPos = 0;
		while (1) {
			bracketPos = txt->find(L"}", bracketPos);
			if (bracketPos < 0 || bracketPos == txt->length() - 1) {
				break;
			}
			bracketPos++;
			wxString mcheck = txt->Mid(bracketPos, 2);
			if (!mcheck.StartsWith(L"m ") && !mcheck.StartsWith(L"{")) {
				txt->insert(bracketPos, L"{");
				bracketPos++;
				bracketPos = txt->find(L"{", bracketPos);
				if (bracketPos < 0) {
					*txt << L"}";
					break;
				}
				txt->insert(bracketPos, L"}");
			}
		}

		wxString afterP1 = txt->Mid(GetPositionInText().y);
		int afterBracket = afterP1.find(L"}") + 1;
		int Mpos = afterBracket;
		if (hasP1) {
			wxString afterP1Brakcet = afterP1.Mid(afterBracket);
			Mpos = afterP1Brakcet.find(L"m ");
			if (Mpos == -1)
				Mpos = afterBracket;
			else
				Mpos += afterBracket;
		}
		wxString startM = afterP1.Mid(Mpos);
		int endDrawing = startM.find(L"{");
		wxString p0;
		if (endDrawing == -1) {
			if (hasP1) { endDrawing = startM.length(); }
			else { endDrawing = 0; }
			p0 += L"{\\p0}";
		}
		else if (!hasP1) {
			p0 += L"{\\p0}";
		}
		size_t startOfDrawing = Mpos + GetPositionInText().y;
		txt->replace(startOfDrawing, endDrawing, *clip + p0);
		if (shapeSelection) {
			int diff = 0;
			SetScale(txt, startOfDrawing, &diff);
			startOfDrawing += diff;
			endDrawing += diff;
		}
		if (changePos) {
			changePos->x = startOfDrawing;
			changePos->y = endDrawing;
		}
	}
}

void DrawingAndClip::SetPos(int x, int y)
{
	_x = x;
	_y = y;
}

// pos in screen position
int DrawingAndClip::CheckPos(D3DXVECTOR2 pos, bool retlast, bool wsp)
{
	if (wsp){ 
		pos.x = (pos.x * coeffW) - _x; 
		pos.y = (pos.y * coeffH) - _y; 
	}
	float coeffPointArea = pointArea * coeffW;
	for (size_t i = 0; i < Points.size(); i++)
	{
		if (Points[i].IsInPos(pos, coeffPointArea)){
			return i; 
		}
	}
	return (retlast) ? Points.size() : -1;
}

// pos in clip position
void DrawingAndClip::MovePoint(D3DXVECTOR2 pos, int point)
{
	Points[point].x = pos.x;
	Points[point].y = pos.y;
}
// pos in screen position	
void DrawingAndClip::AddCurve(D3DXVECTOR2 pos, int whereis, wxString type)
{
	pos.x = (pos.x * coeffW) - _x; 
	pos.y = (pos.y * coeffH) - _y;
	wxPoint oldpos;
	int prevPoint = whereis - 1;
	if (whereis == 0)
		prevPoint = 0;
	//where put in bezier in the middle needs to move point for 1
	//cause it puts curve as previous line
	if (whereis != Points.size()){ whereis++; }
	oldpos.x = Points[prevPoint].x;
	oldpos.y = Points[prevPoint].y;
	int diffx = (pos.x - oldpos.x) / 3.0f;
	int diffy = (pos.y - oldpos.y) / 3.0f;
	Points.insert(Points.begin() + whereis, ClipPoint(pos.x - (diffx * 2), pos.y - (diffy * 2), type, true));
	Points.insert(Points.begin() + whereis + 1, ClipPoint(pos.x - diffx, pos.y - diffy, type, false));
	Points.insert(Points.begin() + whereis + 2, ClipPoint(pos.x, pos.y, type, false));
	acpoint = Points[whereis + 2];
}
// pos in screen position
void DrawingAndClip::AddCurvePoint(D3DXVECTOR2 pos, int whereis)
{
	bool isstart = false;
	int isInRange = (int)Points.size() > whereis;
	if (Points[(whereis == 0) ? 0 : whereis - 1].type == L"s" || (isInRange && Points[whereis].type == L"s"))
	{
		if (isInRange && Points[whereis].start) {
			Points[whereis].start = false;
			isstart = true;
		}

		Points.insert(Points.begin() + whereis, ClipPoint((pos.x*coeffW) - _x, (pos.y*coeffH) - _y, L"s", isstart));
	}
	else{ wxBell(); }
}
// pos in screen position	
void DrawingAndClip::AddLine(D3DXVECTOR2 pos, int whereis)
{
	Points.insert(Points.begin() + whereis, ClipPoint((pos.x*coeffW) - _x, (pos.y*coeffH) - _y, L"l", true));
	acpoint = Points[whereis];
}
// pos in screen position	
void DrawingAndClip::AddMove(D3DXVECTOR2 pos, int whereis)
{
	Points.insert(Points.begin() + whereis, ClipPoint((pos.x*coeffW) - _x, (pos.y*coeffH) - _y, L"m", true));
	acpoint = Points[whereis];
}

void DrawingAndClip::DrawLine(int i)
{
	line->Begin();
	int diff = 1;
	if (Points[i - 1].type == L"s"){
		int j = i - 2;
		while (j >= 0){
			if (Points[j].type != L"s"){ break; }
			j--;
		}
		diff = (i - j) - 2;
	}
	D3DXVECTOR2 v2[2] = { Points[i - diff].GetVector(this), Points[i].GetVector(this) };
	line->Draw(v2, 2, 0xFFBB0000);
	line->End();
	if (i > 1){ DrawRect(i - 1); }

}

void DrawingAndClip::DrawRect(int coord)
{
	Visuals::DrawRect(Points[coord].GetVector(this), Points[coord].isSelected, 3.0f);
}

void DrawingAndClip::DrawCircle(int coord)
{
	Visuals::DrawCircle(Points[coord].GetVector(this), Points[coord].isSelected, 3.0f);
}

int DrawingAndClip::DrawCurve(int i, bool bspline)
{
	line->Begin();
	std::vector<D3DXVECTOR2> v4;

	int pts = 3;
	if (bspline) {

		int acpos = i - 1;
		int bssize = 1;
		int spos = i + 1;
		while (spos < (int)Points.size()) {
			if (Points[spos].start) { break; }
			bssize++; spos++;
		}
		pts = bssize;
		bssize++;
		for (int k = 0; k < bssize; k++) {
			Curve(acpos, &v4, true, bssize, k);
		}
		D3DXVECTOR2* v2 = new D3DXVECTOR2[pts + 2];
		for (int j = 0, g = i - 1; j < bssize; j++, g++)
		{
			v2[j] = Points[g].GetVector(this);
		}
		v2[bssize] = Points[i - 1].GetVector(this);
		line->Draw(v2, pts + 2, 0xFFAA33AA);
		int iplus1 = (i + bssize - 2 < (int)Points.size() - 1) ? i + 1 : 0;
		if (i - 1 != 0 || iplus1 != 0) {
			D3DXVECTOR2 v3[3] = { Points[i - 1].GetVector(this), v4[0], Points[iplus1].GetVector(this) };
			line->Draw(v3, 3, 0xFFBB0000);
		}
		delete[] v2;
	}
	else {
		ClipPoint tmp = Points[i - 1];
		if (Points[i - 1].type == L"s") {
			int diff = 2;
			int j = i - 2;
			while (j >= 0) {
				if (Points[j].type != L"s") { break; }
				j--;
			}
			diff = (i - j) - 2;
			Points[i - 1] = Points[i - diff];
		}
		Curve(i - 1, &v4, false);
		D3DXVECTOR2 v2[4] = { Points[i - 1].GetVector(this), Points[i].GetVector(this),
			Points[i + 1].GetVector(this), Points[i + 2].GetVector(this) };
		line->Draw(v2, 2, 0xFF0000FF);
		line->Draw(&v2[2], 2, 0xFF0000FF);
		Points[i - 1] = tmp;
	}
	line->Draw(&v4[0], v4.size(), 0xFFBB0000);
	line->End();
	if (i > 1) { DrawRect(i - 1); }
	for (int j = 1; j < pts; j++) { DrawCircle(i + j - 1); }
	return pts;
}

void DrawingAndClip::Curve(int pos, std::vector<D3DXVECTOR2>* table, bool bspline, int spoints, int acpt)
{
	float a[4], b[4];
	float x[4], y[4];
	for (int g = 0; g < 4; g++)
	{
		if (acpt > (spoints - 1)) { acpt = 0; }
		x[g] = Points[pos + acpt].wx(this, true);
		y[g] = Points[pos + acpt].wy(this, true);
		acpt++;
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

void DrawingAndClip::OnMouseEvent(wxMouseEvent &event)
{
	if (event.GetWheelRotation() != 0) {
		int step = event.GetWheelRotation() / event.GetWheelDelta();
		tool -= step;
		VideoBox *vc = tab->video;
		vc->GetVideoToolbar()->SetItemToggled(&tool);
		return;
	}
	if (blockevents){ return; }
	event.GetPosition(&x, &y);


	float zx = (x / zoomScale.x) + zoomMove.x;
	float zy = (y / zoomScale.y) + zoomMove.y;
	D3DXVECTOR2 xy(zx, zy);
	bool click = event.LeftDown();
	bool leftisdown = event.LeftIsDown();
	bool right = event.RightDown();
	bool ctrl = event.ControlDown();
	size_t psize = Points.size();

	if (!event.ButtonDown() && !leftisdown){
		int pos = CheckPos(xy);
		if (pos != -1/* && hasArrow*//* && !ctrl*/){
			acpoint = Points[pos];
			lastpos = pos;
			tab->video->Render(false);
			
		}
		else if (pos == -1 /*&& !tab->video->HasArrow()*/){
			if (lastpos >= 0 && lastpos < (int)psize){
				lastpos = -1;
				tab->video->Render(false);
			}
		}
		if (tool >= 1 && tool <= 3 && pos == -1 && !event.Leaving()){
			if (psize < 1){ return; }
			drawToolLines = true;
			tab->video->Render(false);
		}
		else if (drawToolLines){
			drawToolLines = false;
			tab->video->Render(false);
		}
		if (!drawSelection){
			if (tool == 0 || pos != -1 || tool > 3){ drawCross = true; tab->video->Render(false); }
		}
		if (event.Leaving() && drawCross){
			drawCross = false;
			tab->video->Render(false);
		}
	}



	if (event.LeftUp() || event.RightUp() || event.MiddleUp()){
		if (!drawSelection){
			SetClip(false);
		}
		else{
			drawSelection = false;
			tab->video->Render(false);
		}
		if (tab->video->HasCapture()){
			tab->video->ReleaseMouse();
		}
		return;
	}
	if (event.ButtonUp()){
		return;
	}
	//remove points
	if (event.MiddleDown() || (tool == 5 && click)){
		grabbed = -1;
		size_t i = (psize > 1) ? 1 : 0;
		for (size_t i = 0; i < psize; i++)
		{
			float pointx = Points[i].wx(this, false), pointy = Points[i].wy(this, false);
			if (abs(pointx - zx) < pointArea && abs(pointy - zy) < pointArea)
			{
				int j = i;
				int er = 1;
				bool isM = (Points[i].type == L"m");
				if (isM){
					Points.erase(Points.begin() + i, Points.begin() + i + 1);
					if (i >= Points.size()){
						SetClip(true);
						return;
					}
					psize--;
				}

				if (!(Points[j].start)){
					for (j = i - 1; j >= 0; j--){
						if (Points[j].start){ break; }
					}
					if (j == 0 && !Points[j].start){ Points[j].start = true; }
				}
				if (Points[j].type == L"s"){
					size_t k;
					for (k = j + 1; k < psize; k++){
						if (Points[k].start){ break; }
					}
					if (k - j < 4){ er = 2; }
					else { j = i; }
				}

				if (Points[j].type == L"b" || er == 2){
					er = 2;
					if (j + 2 == i || isM){
						er = 3;

					}
					else{
						Points[j + 2].type = L"l";
						Points[j + 2].start = true;
					}
				}



				if (isM){
					if (er > 1){
						Points.erase(Points.begin() + j, Points.begin() + j + er - 1);
					}
					Points[i].type = L"m";
					Points[i].start = true;
					if (i + 1 < Points.size()){ Points[i + 1].start = true; }
				}
				else{
					Points.erase(Points.begin() + j, Points.begin() + j + er);
				}
				SetClip(true);
				break;
			}
		}
		return;
	}

	if (click || right){
		grabbed = -1;
		axis = 0;
		for (size_t i = 0; i < psize; i++)
		{
			float pointx = Points[i].wx(this), pointy = Points[i].wy(this);
			if (abs(pointx - zx) < pointArea && abs(pointy - zy) < pointArea)
			{
				lastpoint = acpoint = Points[i];
				if (!acpoint.isSelected && !ctrl){
					ChangeSelection();
				}
				Points[i].isSelected = (right)? false : (ctrl) ? !Points[i].isSelected : true;
				grabbed = i;
				diffs.x = pointx - zx;
				diffs.y = pointy - zy;
				tab->video->CaptureMouse();
				firstmove = D3DXVECTOR2(zx, zy);
				break;
			}
		}

		if (tool >= 1 && tool <= 3 && (grabbed == -1 || right))
		{
			if (Points.empty()){ AddMove(xy, 0); SetClip(true); return; }
			int pos = CheckPos(xy, true);
			switch (tool){
			case 1: 
				if (right && grabbed != -1) {
					pos++;
					pos = CheckCurve(pos);
					if (pos < 0) {
						wxBell();
						return;
					}
				}
				AddLine(xy, pos); 
				break;
			case 2: AddCurve(xy, pos); break;
			case 3:
				if (right && grabbed != -1) {
					pos++;
					pos = CheckCurve(pos, false);
					if (pos < 0) {
						wxBell();
						return;
					}
				}
				if (Points[(pos == (int)psize) ? psize - 1 : pos].type == L"s"){
					AddCurvePoint(xy, pos); break;//bspline point
				}
				AddCurve(xy, pos, L"s"); break;//bspline
			default:
				KaiLog(wxString::Format(L"Bad tool %i", tool));
			}
			SetClip(true);
			return;
		}


		if (tool == 4 && grabbed == -1)
		{
			int pos = CheckPos(xy, true);
			if (psize > 0 && Points[(pos == (int)psize) ? psize - 1 : pos].type == L"m"){
				KaiMessageBox(_("Ze względu na błędy Vsfiltra możliwość wstawiania dwóch \"m\" po sobie została zablokowana"), 
					_("Uwaga"));
				return;
			}
			AddMove(xy, pos);
			SetClip(true);
			tool = 1;
			tab->video->GetVideoToolbar()->SetItemToggled(&tool);
		}
		else if (grabbed == -1){
			tab->video->CaptureMouse();
			drawSelection = true;
			drawCross = false;
			selection = wxRect(x, y, x, y);
			SelectPoints();
		}
		return;
	}

	if (leftisdown && grabbed != -1 && event.Dragging())
	{
		//drawtxt=true;
		zx = MID(0, zx, VideoSize.width - VideoSize.x);
		zy = MID(0, zy, VideoSize.height - VideoSize.y);
		Points[grabbed].x = ((zx + diffs.x) * coeffW) - _x;
		Points[grabbed].y = ((zy + diffs.y) * coeffH) - _y;
		//new feature snapping to other points
		//add some method to turn it on
		if (event.AltDown())
			Points[grabbed] = FindSnapPoint(Points[grabbed], grabbed);

		if (!Points[grabbed].isSelected){ Points[grabbed].isSelected = true; }

		if (event.ShiftDown()){
			
			int diffx = abs(firstmove.x - zx);
			int diffy = abs(firstmove.y - zy);
			if (diffx != diffy){ if (diffx > diffy){ axis = 2; } else{ axis = 1; } }
			
			if (axis == 1){
				Points[grabbed].x = lastpoint.x;
			}
			if (axis == 2){
				Points[grabbed].y = lastpoint.y;
			}
			
		}
		if (Points[grabbed].isSelected){
			float movementx = acpoint.x - Points[grabbed].x;
			float movementy = acpoint.y - Points[grabbed].y;
			for (size_t i = 0; i < psize; i++){
				if (Points[i].isSelected && i != grabbed){
					Points[i].x = Points[i].x - movementx;
					Points[i].y = Points[i].y - movementy;
				}
			}
		}
		acpoint = Points[grabbed];
		SetClip(true);
		lastpos = -1;
	}
	if (drawSelection){
		selection.width = x;
		selection.height = y;
		SelectPoints();
		tab->video->Render(false);
	}


}


void DrawingAndClip::SelectPoints(){
	int x = (selection.x < selection.width) ? selection.x : selection.width,
		y = (selection.y < selection.height) ? selection.y : selection.height,
		r = (selection.x > selection.width) ? selection.x : selection.width,
		b = (selection.y > selection.height) ? selection.y : selection.height;
	for (size_t i = 0; i < Points.size(); i++){
		D3DXVECTOR2 point = Points[i].GetVector(this);
		if (point.x >= x && point.x <= r &&
			point.y >= y && point.y <= b){
			Points[i].isSelected = true;
		}
		else{
			Points[i].isSelected = false;
		}

	}

}

void DrawingAndClip::ChangeSelection(bool select)
{
	for (size_t i = 0; i < Points.size(); i++){
		Points[i].isSelected = select;
	}
}

int DrawingAndClip::FindPoint(int pos, wxString type, bool nextStart, bool fromEnd)
{
	int j = pos;
	if (nextStart){
		while ((fromEnd) ? j >= 0 : j < (int)Points.size()){
			if (Points[j].start){ break; }
			if (fromEnd){ j--; }
			else{ j++; }
		}
	}
	else{
		while ((fromEnd) ? j >= 0 : j < (int)Points.size()){
			if (Points[j].type == type){ break; }
			if (fromEnd){ j--; }
			else{ j++; }
		}
	}
	return j;
}

ClipPoint DrawingAndClip::FindSnapPoint(const ClipPoint &pos, size_t pointToSkip/*bool coeff*/)
{
	bool xfound = false, yfound = false;
	float modPosx = /*coeff? (pos.x * coeffW) - _x : */pos.x;
	float modPosy = /*coeff? (pos.y * coeffH) - _y : */pos.y;
	const float maxdiff = 10.f;
	for (size_t i = 0; i < Points.size(); i++){
		if (i == pointToSkip)
			continue;

		if (!xfound && abs(Points[i].x - modPosx) <= maxdiff){
			xfound = true;
			modPosx = Points[i].x;
		}
		if (!yfound && abs(Points[i].y - modPosy) <= maxdiff){
			yfound = true;
			modPosy = Points[i].y;
		}
	}
	/*if (coeff){
		modPosx = (modPosx + _x) / coeffW;
		modPosy = (modPosy + _y) / coeffH;
	}*/
	ClipPoint returncp = pos;
	returncp.x = modPosx;
	returncp.y = modPosy;
	return returncp;
}

void DrawingAndClip::OnKeyPress(wxKeyEvent &evt)
{
	int keyCode = evt.GetKeyCode();
	if (evt.ControlDown() && keyCode == L'A'){
		ChangeSelection(true);
		tab->video->Render(false);
	}
	else if(keyCode == L'W' || keyCode == L'S' || keyCode == L'A' || keyCode == L'D'){
		float x = 0; float y = 0;
		float increase = (evt.ShiftDown() && Visual == VECTORDRAW)? 0.1f : 1.f;
		switch (keyCode)
		{
		case L'A':
			x = -increase; break;
		case L'D':
			x = increase; break;
		case L'S':
			y = increase; break;
		case L'W':
			y = -increase; break;

		default:
			break;
		}

		OnMoveSelected(x, y);
	}

}

void DrawingAndClip::OnMoveSelected(float x, float y)
{
	size_t psize = Points.size();
	bool modified = false;
	for (size_t i = 0; i < psize; i++)
	{
		if (Points[i].isSelected)
		{
			Points[i].x += x;
			Points[i].y += y;
			modified = true;
		}
	}
	if (modified) {
		SetClip(true);
		SetClip(false);
	}
}

int DrawingAndClip::CheckCurve(int pos, bool checkSpline)
{
	size_t pointsSize = Points.size();
	if (pos < 0 || pos > pointsSize) {
		return pointsSize;
	}
	else if (pos < 2 || pos >= pointsSize)
		return pos;
	else if (Points[pos].type == L"b") {
		int start = 1;
		int end = pointsSize - 1;
		int result = -1;
		for (int i = pos - 1; i >= 1; i--) {
			if (Points[i].type != L"b") {
				start = i + 1;
				break;
			}
		}
		for (int i = pos + 1; i < pointsSize; i++) {
			if (Points[i].type != L"b") {
				end = i;
				break;
			}
		}

		if (end - start > 4 || start == pos) {
			for (int i = start; i <= end; i += 3) {
				if (pos == i)
					result = i;
				else if (i > pos)
					break;
			}
		}
		return result;
	}
	else if (checkSpline && Points[pos].type == L"s")
		return -1;

	return pos;
}

void DrawingAndClip::AppendClipMask(wxString * mask)
{
	if(!clipMask.empty())
	*mask << clipMask;
}

void DrawingAndClip::CreateClipMask(const wxString& clip, wxString * clipTag)
{
	int nx = SubsSize.x, ny = SubsSize.y;
	
	Dialogue *maskDialogue = tab->edit->line->Copy();
	wxString tmp;

	if (!clip.empty() && (clipTag || 
		(FindTag(L"(i?clip.)[^)]*\\)", maskDialogue->GetTextNoCopy(), 1)) && GetTextResult(&tmp))) {
		wxString tmp1 = clipTag? *clipTag : (tmp[0] == L'c') ? wxString(L"iclip(") : wxString(L"clip(");
		wxString text;
		text << L"{\\p1\\bord0\\shad0\\fscx100\\fscy100\\frz0\\1c&H000000&\\1a&H77&\\pos(0,0)\\an7\\" << tmp1 << clip << L")}m 0 0 l " <<
			nx << L" 0 " << nx << L" " << ny << L" 0 " << ny << L"\r\n";
		maskDialogue->SetTextElement(TXT, text);
		clipMask.Empty();
		maskDialogue->GetRaw(&clipMask);
	}
	else {
		clipMask.clear();
	}
	delete maskDialogue;
}

void DrawingAndClip::InvertClip()
{
	SubsGrid* grid = tab->grid;
	wxArrayInt sels;
	grid->file->GetSelections(sels);
	wxRegEx re(L"\\\\(i?clip)\\(([^)]*)\\)", wxRE_ADVANCED);
	if (!re.IsValid())
		return;

	Dialogue* cdial = grid->GetDialogue(grid->currentLine);
	const wxString& ctxt = cdial->GetTextNoCopy();
	wxString clip;
	size_t movement = 0;
	while (1) {
		wxString clippedTxt = ctxt.Mid(movement);
		if (re.Matches(clippedTxt)) {
			size_t start = 0, len = 0;
			if (re.GetMatch(&start, &len, 2)) {
				wxString clipBody = ctxt.Mid(movement + start, len);
				if (clipBody.find(L'm') != -1) {
					wxString curclip = re.GetMatch(clippedTxt, 1);
					if (curclip.StartsWith(L"i"))
						clip = L"clip";
					else
						clip = L"iclip";
				}
			}
			movement += start + len;
		}
		else
			break;
	}
	if (clip.empty())
		return;

	bool changed = false;
	for (size_t i = 0; i < sels.size(); i++) {

		Dialogue* Dialc = grid->CopyDialogue(sels[i]);
		wxString &txt = Dialc->GetText();
		size_t movement = 0;
		while (1) {
			wxString clippedTxt = txt.Mid(movement);
			if (re.Matches(clippedTxt)) {
				size_t start = 0, len = 0;
				if (re.GetMatch(&start, &len, 2)) {
					wxString clipBody = txt.Mid(movement + start, len);
					if (clipBody.find(L'm') != -1) {
						size_t start1 = 0, end1 = 0;
						if (re.GetMatch(&start, &len, 1)) {
							txt.replace(movement + start, len, clip);
							changed = true;
						}
					}
				}
				else {
					start = clippedTxt.find(L"clip");
					if (start == -1)
						break;

					len = 4;
				}
				movement += start + len;
			}
			else
				break;
		}
	}
	if (changed) {
		grid->SetModified(VISUAL_VECTOR_CLIP);
	}
}


void DrawingAndClip::SetZoom(D3DXVECTOR2 move, D3DXVECTOR2 zoomscale)
{
	Visuals::SetZoom(move, zoomscale);
	pointArea = 4.f / zoomScale.x;
}

void DrawingAndClip::ChangeTool(int _tool, bool blockSetCurVisual) {
	int shapeSelectionNew = _tool >> 6;
	//set shape to Shapes class above
	if (Visual == VECTORDRAW) {
		SetShape(shapeSelectionNew);
		bool needRefresh = false;
		if ((shapeSelection == 0 && shapeSelectionNew != 0) ||
			(shapeSelection != 0 && shapeSelectionNew == 0)) {
			needRefresh = true;
		}
		if (shapeSelectionNew == 0 && shapeSelection != 0 && !blockSetCurVisual) {
			//set a new values of coeffs to prevent second divide by scale
			coeffW = ((float)SubsSize.x / (float)(VideoSize.width - VideoSize.x));
			coeffH = ((float)SubsSize.y / (float)(VideoSize.height - VideoSize.y));
			SetCurVisual();
		}
		shapeSelection = shapeSelectionNew;
		if (needRefresh) {
			tab->video->Render(false);
		}
	}

	int clipTool = _tool << 26;
	clipTool >>= 26;
	//invert clip
	if (clipTool == 6) {
		InvertClip();
	}
	else
		tool = clipTool;
};
