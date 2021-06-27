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
#include "VisualDrawingShapes.h"
#include <wx/regex.h>


Visuals *Visuals::Get(int Visual, wxWindow *_parent)
{
	Visuals *visual;
	switch (Visual)
	{
	case CROSS:
		visual = new Cross();
		break;
	case CHANGEPOS:
		visual = new Position();
		break;
	case MOVE:
		visual = new Move();
		break;
	case MOVEALL:
		visual = new MoveAll();
		break;
	case SCALE:
		visual = new Scale();
		break;
	case ROTATEZ:
		visual = new RotationZ();
		break;
	case ROTATEXY:
		visual = new RotationXY();
		break;
	case CLIPRECT:
		visual = new ClipRect();
		break;
	case VECTORCLIP:
		visual = new DrawingAndClip();
	case VECTORDRAW:
		visual = new Shapes();
		break;
	/*case SCALE_ROTATION:
		visual = new ScaleRotation();
		break;*/
	case ALL_TAGS:
		visual = new AllTags();
		break;
	default:
		visual = new Position();
		break;
	}

	visual->tab = (TabPanel*)_parent->GetParent();
	visual->Visual = Visual;
	visual->SetTabPanel(visual->tab);
	return visual;
}


Visuals::Visuals()
{
	from = lastmove = to = zoomMove = D3DXVECTOR2(0, 0);
	coeffW = 1;
	coeffH = 1;
	line = 0;
	font = 0;
	device = 0;
	start = end = oldtime = 0;
	axis = 0;
	blockevents = false;
	zoomScale = D3DXVECTOR2(1.0f, 1.0f);
	dummytext = NULL;
}

Visuals::~Visuals()
{
	SAFE_DELETE(dummytext);
}

//gets dialogues without position to calculate line coliding position
void Visuals::GetDialoguesWithoutPosition(Dialogue* dialogue)
{
	dialoguesWithoutPosition.clear();
	int time = tab->Video->Tell();
	if (time >= dialogue->Start.mstime && time < dialogue->End.mstime) {
		SubsGrid* grid = tab->Grid;

		wxRegEx pos(L"\\\\(pos|move)\\(([^\\)]+)\\)", wxRE_ADVANCED);
		bool tlMode = tab->Grid->hasTLMode;
		int activeLineKey = tab->Grid->currentLine;

		for (size_t i = 0; i < grid->file->GetCount(); i++) {
			Dialogue* dial = grid->file->GetDialogue(i);
			if (!grid->ignoreFiltered && !dial->isVisible || dial->NonDialogue) {
				continue;
			}

			if (i == activeLineKey && dialogue == tab->Edit->line)
				break;
			if (dial == dialogue)
				break;

			if (time >= dial->Start.mstime && time < dial->End.mstime) {
				const wxString& text = dial->GetTextNoCopy();
				if (!pos.Matches(text)) {
					dialoguesWithoutPosition.push_back(dial);
				}
			}

		}
	}
}

//gets lines coliding position
D3DXVECTOR2 Visuals::GetDialogueAdditionalPosition(Dialogue* dialogue)
{
	GetDialoguesWithoutPosition(dialogue);
	D3DXVECTOR2 result = { 0, 0 };
	if (!dialoguesWithoutPosition.size())
		return result;

	size_t dialoguesSize = dialoguesWithoutPosition.size();

	bool tlMode = tab->Grid->hasTLMode;
	wxRegEx an(L"\\\\an([0-9]+)", wxRE_ADVANCED);
	Styles* currentDialogueStyle = tab->Grid->GetStyle(0, dialogue->Style);
	int curlineAn = wxAtoi(currentDialogueStyle->Alignment);
	const wxString& txt = dialogue->GetTextNoCopy();
	if (an.Matches(txt)) {
		curlineAn = wxAtoi(an.GetMatch(txt, 1));
	}
	int clan = curlineAn / 3.1f;

	
	D3DXVECTOR2 dialPos = { 0, 0 };
	D3DXVECTOR2 dialPos1 = { 0, 0 };
	D3DXVECTOR2 bord;
	D3DXVECTOR2 size = GetTextSize(dialogue, &bord, currentDialogueStyle, true);
	size.x += bord.x;
	size.y += bord.y;
	GetRectFromSize(size, curlineAn, dialogue, currentDialogueStyle, &dialPos, &dialPos1);
	int placesForText = 0;
	int placesTaken = 0;

	for (size_t i = 0; i < dialoguesSize; i++){
		Dialogue *dial = dialoguesWithoutPosition[i];
		Styles *currentStyle = tab->Grid->GetStyle(0, dial->Style);
		const wxString &txt = dial->GetTextNoCopy();
		int newan = wxAtoi(currentStyle->Alignment);
		if (an.Matches(txt)){
			newan = wxAtoi(an.GetMatch(txt, 1));
		}
		
		int nan = newan / 3.1f;
		if (nan == clan) {
			D3DXVECTOR2 bord;
			D3DXVECTOR2 size = GetTextSize(dial, &bord, currentStyle, true);
			size.x += bord.x;
			size.y += bord.y;
			D3DXVECTOR2 newpos;
			D3DXVECTOR2 newpos1;
			GetRectFromSize(size, newan, dial, currentStyle, &newpos, &newpos1);
			//check position of current dialogue with this one,
			//if rects colides change position
			if (IsInRect(dialPos, dialPos1, newpos, newpos1)) {
				//top
				if (clan == 2)
					result.y += size.y;
				else if (clan == 1)
					result.y += size.y / 2;
				else if (clan == 0)
					result.y -= size.y;

				placesTaken++;
			}
			// when there is a place for this line then break the loop
			else if (placesTaken < placesForText && placesTaken > 0 && placesForText > 0) {
				break;
			}
			else
				placesForText++;
				
		}
		
	}
	return result;
}

void Visuals::GetRectFromSize(D3DXVECTOR2 size, int an, Dialogue* dial, Styles* style, D3DXVECTOR2* pos, D3DXVECTOR2* pos1)
{
	D3DXVECTOR2 result;
	SetPositionByAn(&result, an, dial, style);
	if (an % 3 == 2) {
		result.x -= (size.x / 2);
	}
	else if (an % 3 == 0) {
		result.x -= size.x;
	}
	if (an < 4) {
		result.y -= size.y;
	}
	else if (an < 7) {
		result.y -= (size.y / 2);
	}
	/*else if (an < 10) {
		
	}*/
	*pos = result;
	pos1->x = result.x + size.x;
	pos1->y = result.y + size.y;
}

bool Visuals::IsInRect(D3DXVECTOR2 pos, D3DXVECTOR2 pos1, D3DXVECTOR2 secondpos, D3DXVECTOR2 secondpos1)
{
	return ((secondpos1.x > pos.x) && (secondpos.x < pos1.x)
		&& (secondpos1.y > pos.y) && (secondpos.y < pos1.y));
}

void Visuals::SetPositionByAn(D3DXVECTOR2* pos, int an, Dialogue* dial, Styles* style)
{
	if (an % 3 == 2) {
		int marginL = (dial->MarginL != 0) ? dial->MarginL : wxAtoi(style->MarginL);
		int marginR = (dial->MarginR != 0) ? dial->MarginR : wxAtoi(style->MarginR);
		pos->x = ((SubsSize.x + marginL - marginR) / 2);
	}
	else if (an % 3 == 0) {
		pos->x = (dial->MarginR != 0) ? dial->MarginR : wxAtoi(style->MarginR);
		pos->x = SubsSize.x - pos->x;
	}
	else {
		pos->x = (dial->MarginL != 0) ? dial->MarginL : wxAtoi(style->MarginL);
	}

	if (an < 4) {
		pos->y = (dial->MarginV != 0) ? dial->MarginV : wxAtoi(style->MarginV);
		pos->y = SubsSize.y - pos->y;
	}
	else if (an < 7) {
		pos->y = (SubsSize.y / 2);
	}
	else {
		pos->y = (dial->MarginV != 0) ? dial->MarginV : wxAtoi(style->MarginV);
	}
}

void Visuals::RenderSubs(wxString *subs, bool redraw /*= true*/)
{
	//visual, renderer should exist
	RendererVideo *renderer = tab->Video->GetRenderer();
	if (renderer && !renderer->OpenSubs(OPEN_HAS_OWN_TEXT, true, subs)){ KaiLog(_("Nie można otworzyć napisów")); }
	tab->Video->SetVisualEdition(true);
	if (redraw){ tab->Video->Render(); }
}

void Visuals::SetVisual(Dialogue* dial, int tool, bool noRefresh)
{
	int nx = 0, ny = 0;
	tab->Grid->GetASSRes(&nx, &ny);
	SubsSize = wxSize(nx, ny);
	start = dial->Start.mstime;
	end = dial->End.mstime;
	notDialogue = dial->IsComment;

	coeffW = ((float)SubsSize.x / (float)(VideoSize.width - VideoSize.x));
	coeffH = ((float)SubsSize.y / (float)(VideoSize.height - VideoSize.y));
	tab->Video->SetVisualEdition(true);

	//set copy of editbox text that every dummy edition have the same text
	//not changed by other dummy editions to avoid mismatches when moving
	bool isOriginal = (tab->Grid->hasTLMode && tab->Edit->TextEdit->GetValue() == L"");
	editor = (isOriginal) ? tab->Edit->TextEditOrig : tab->Edit->TextEdit;
	currentLineText = editor->GetValue();
	ChangeTool(tool);
	SetCurVisual();
}

void Visuals::SizeChanged(wxRect wsize, LPD3DXLINE _line, LPD3DXFONT _font, LPDIRECT3DDEVICE9 _device)
{
	line = _line;
	font = _font;
	device = _device;
	VideoSize = wsize;
	coeffW = ((float)SubsSize.x / (float)(wsize.width - wsize.x));
	coeffH = ((float)SubsSize.y / (float)(wsize.height - wsize.y));

	HRN(device->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE), L"FVF failed");
}
//DrawArrow moves point "to" to end before arrow
//from and to are line coordinates
//diff for move arrow backward from point "to"
void Visuals::DrawArrow(D3DXVECTOR2 from, D3DXVECTOR2 *to, int diff)
{
	D3DXVECTOR2 pdiff = from - (*to);
	float len = sqrt((pdiff.x * pdiff.x) + (pdiff.y * pdiff.y));
	D3DXVECTOR2 diffUnits = (len == 0) ? D3DXVECTOR2(0, 0) : pdiff / len;
	//length can have values less than zero, change to plus to pravent bad arrow drawing
	D3DXVECTOR2 pend = (*to) + (diffUnits * (12 + diff));
	D3DXVECTOR2 halfbase = D3DXVECTOR2(-diffUnits.y, diffUnits.x) * 5.f;

	VERTEX v4[7];
	D3DXVECTOR2 v3[3];
	v3[0] = pend - diffUnits * 12;
	v3[1] = pend + halfbase;
	v3[2] = pend - halfbase;

	CreateVERTEX(&v4[0], v3[0].x, v3[0].y, 0xAA121150);
	CreateVERTEX(&v4[1], v3[1].x, v3[1].y, 0xAA121150);
	CreateVERTEX(&v4[2], v3[2].x, v3[2].y, 0xAA121150);
	CreateVERTEX(&v4[3], v3[0].x, v3[0].y, 0xFFBB0000);
	CreateVERTEX(&v4[4], v3[1].x, v3[1].y, 0xFFBB0000);
	CreateVERTEX(&v4[5], v3[2].x, v3[2].y, 0xFFBB0000);
	CreateVERTEX(&v4[6], v3[0].x, v3[0].y, 0xFFBB0000);

	HRN(device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 1, v4, sizeof(VERTEX)), L"primitive failed");
	HRN(device->DrawPrimitiveUP(D3DPT_LINESTRIP, 3, &v4[3], sizeof(VERTEX)), L"primitive failed");
	*to = pend;
}

void Visuals::DrawCross(D3DXVECTOR2 position, D3DCOLOR color, bool useBegin)
{
	D3DXVECTOR2 cross[4];
	cross[0].x = position.x - 15.0f;
	cross[0].y = position.y;
	cross[1].x = position.x + 15.0f;
	cross[1].y = position.y;
	cross[2].x = position.x;
	cross[2].y = position.y - 15.0f;
	cross[3].x = position.x;
	cross[3].y = position.y + 15.0f;
	if (useBegin){ line->Begin(); }
	line->Draw(cross, 2, color);
	line->Draw(&cross[2], 2, color);
	if (useBegin){ line->End(); }

}

void Visuals::DrawRect(D3DXVECTOR2 pos, bool sel, float rcsize)
{
	D3DCOLOR fill = (sel) ? 0xAAFCE6B1 : 0xAA121150;
	VERTEX v9[9];
	CreateVERTEX(&v9[0], pos.x - rcsize, pos.y - rcsize, fill);
	CreateVERTEX(&v9[1], pos.x + rcsize, pos.y - rcsize, fill);
	CreateVERTEX(&v9[2], pos.x - rcsize, pos.y + rcsize, fill);
	CreateVERTEX(&v9[3], pos.x + rcsize, pos.y + rcsize, fill);
	CreateVERTEX(&v9[4], pos.x - rcsize, pos.y - rcsize, 0xFFBB0000);
	CreateVERTEX(&v9[5], pos.x + rcsize, pos.y - rcsize, 0xFFBB0000);
	CreateVERTEX(&v9[6], pos.x + rcsize, pos.y + rcsize, 0xFFBB0000);
	CreateVERTEX(&v9[7], pos.x - rcsize, pos.y + rcsize, 0xFFBB0000);
	CreateVERTEX(&v9[8], pos.x - rcsize, pos.y - rcsize, 0xFFBB0000);

	HRN(device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, v9, sizeof(VERTEX)), L"primitive failed");
	HRN(device->DrawPrimitiveUP(D3DPT_LINESTRIP, 4, &v9[4], sizeof(VERTEX)), L"primitive failed");
}


void Visuals::DrawCircle(D3DXVECTOR2 pos, bool sel, float crsize)
{
	D3DCOLOR fill = (sel) ? 0xAAFCE6B1 : 0xAA121150;
	VERTEX v5[41];
	float rad = 0.01745329251994329576923690768489f;

	float xx = pos.x;
	float yy = pos.y;
	CreateVERTEX(&v5[0], xx, yy, fill);
	for (int j = 0; j < 20; j++)
	{
		float xx1 = pos.x + (crsize * sin((j * 20) * rad));
		float yy1 = pos.y + (crsize * cos((j * 20) * rad));
		CreateVERTEX(&v5[j + 1], xx1, yy1, fill);
		CreateVERTEX(&v5[j + 21], xx1, yy1, 0xFFBB0000);
		xx = xx1;
		yy = yy1;

	}

	HRN(device->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 18, v5, sizeof(VERTEX)), L"primitive failed");
	HRN(device->DrawPrimitiveUP(D3DPT_LINESTRIP, 18, &v5[21], sizeof(VERTEX)), L"primitive failed");
}

void Visuals::DrawDashedLine(D3DXVECTOR2 *vector, size_t vectorSize, int dashLen, unsigned int color)
{

	D3DXVECTOR2 actualPoint[2];
	for (size_t i = 0; i < vectorSize - 1; i++){
		size_t iPlus1 = (i < (vectorSize - 1)) ? i + 1 : 0;
		D3DXVECTOR2 pdiff = vector[i] - vector[iPlus1];
		float len = sqrt((pdiff.x * pdiff.x) + (pdiff.y * pdiff.y));
		if (len == 0){ return; }
		D3DXVECTOR2 diffUnits = pdiff / len;
		float singleMovement = 1 / (len / (dashLen * 2));
		actualPoint[0] = vector[i];
		actualPoint[1] = actualPoint[0];
		for (float j = 0; j <= 1; j += singleMovement){
			actualPoint[1] -= diffUnits * dashLen;
			if (j + singleMovement >= 1){ actualPoint[1] = vector[iPlus1]; }
			line->Draw(actualPoint, 2, color);
			actualPoint[1] -= diffUnits * dashLen;
			actualPoint[0] -= (diffUnits * dashLen) * 2;
		}
	}
}


void Visuals::Draw(int time)
{
	//dont forget checking if times are in range time >= start && time < end
	if (!(time >= start && time < end) || (notDialogue && Visual != VECTORDRAW && Visual != VECTORCLIP)){
		DrawWarning(notDialogue); 
		blockevents = true; 
		return; 
	}
	else if (blockevents){ blockevents = false; }
	wxMutexLocker lock(clipmutex);
	line->SetAntialias(TRUE);
	line->SetWidth(2.0);

	DrawVisual(time);
	line->SetAntialias(FALSE);
	oldtime = time;
}

void Visuals::DrawWarning(bool comment)
{
	if (Options.GetBool(VIDEO_VISUAL_WARNINGS_OFF))
		return;

	LPD3DXFONT warningFont;
	HRN(D3DXCreateFont(device, (VideoSize.width - VideoSize.x) / 20, 0, FW_BOLD, 0, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, TEXT("Tahoma"), &warningFont), _("Nie można stworzyć czcionki D3DX"));
	RECT rt = { 0, 0, VideoSize.width, VideoSize.height };
	wxString text = (comment) ? _("Narzędzia edycji wizualnej\nnie działają na komentarzach") :
		_("Linia nie jest widoczna na wideo\nalbo ma zerowy czas trwania");
	DRAWOUTTEXT(warningFont, text, rt, DT_CENTER | DT_VCENTER, 0xFFFF0000);
	SAFE_RELEASE(warningFont);
}

D3DXVECTOR2 Visuals::CalcMovePos()
{
	D3DXVECTOR2 ppos;
	int time = tab->Video->Tell();
	if (moveValues[6] < 6){ moveValues[4] = start; moveValues[5] = end; }
	float tmpt = time - moveValues[4];
	float tmpt1 = moveValues[5] - moveValues[4];
	float actime = tmpt / tmpt1;
	float distx, disty;
	if (time < moveValues[4]){ distx = moveValues[0], disty = moveValues[1]; }
	else if (time > moveValues[5]){ distx = moveValues[2], disty = moveValues[3]; }
	else {
		distx = moveValues[0] - ((moveValues[0] - moveValues[2]) * actime);
		disty = moveValues[1] - ((moveValues[1] - moveValues[3]) * actime);
	}
	ppos.x = distx, ppos.y = disty;
	return ppos;
}

void Visuals::GetVectorPoints(const wxString& vector, std::vector<ClipPoint>* points)
{
	wxStringTokenizer tokens(vector, L" ");
	double tmpx = 0;
	bool gotx = false;
	bool start = false;
	int pointsAfterStart = 1;
	wxString type = L"m";
	while (tokens.HasMoreTokens()) {
		wxString token = tokens.GetNextToken();
		if (token == L"p") { token = L"s"; }
		if (token == L"m" || token == L"l" || token == L"b" || token == L"s") {
			type = token;
			start = true;
			pointsAfterStart = 1;
		}
		else if (token == L"c") { start = true; continue; }
		else if (gotx) {
			double tmpy = 0;
			if (!token.ToCDouble(&tmpy)) { gotx = false; continue; }
			points->push_back(ClipPoint(tmpx, tmpy, type, start));
			gotx = false;
			if ((type == L"l" || type == L"m" && pointsAfterStart == 1) || (type == L"b" && pointsAfterStart == 3)) {
				if (type == L"m") { type = L"l"; }
				start = true;
				pointsAfterStart = 0;
			}
			else {
				start = false;
			}
			pointsAfterStart++;
		}
		else {
			if (token.ToCDouble(&tmpx)) {
				gotx = true;
			}
		}

	}
}

void Visuals::GetMoveTimes(int *start, int *end)
{
	VideoCtrl *video = tab->Video;
	EditBox *edit = tab->Edit;
	Provider *FFMS2 = video->GetFFMS2();
	float fps;
	video->GetFPSAndAspectRatio(&fps, NULL, NULL, NULL);
	int startTime = ZEROIT(edit->line->Start.mstime);
	int endTime = ZEROIT(edit->line->End.mstime);
	int framestart = (!FFMS2) ? (((float)startTime / 1000.f) * fps) + 1 : FFMS2->GetFramefromMS(startTime);
	int frameend = (!FFMS2) ? ((float)endTime / 1000.f) * fps : FFMS2->GetFramefromMS(endTime) - 1;
	int msstart = (!FFMS2) ? ((framestart * 1000) / fps) + 0.5f : FFMS2->GetMSfromFrame(framestart);
	int msend = (!FFMS2) ? ((frameend * 1000) / fps) + 0.5f : FFMS2->GetMSfromFrame(frameend);
	int diff = endTime - startTime;

	if(start)
		*start = abs(msstart - startTime);
	if(end) 
		*end = (diff - abs(endTime - msend));
}

//Getting position and scale with vector drawing need to put scale in rest of cases can put one value or none
D3DXVECTOR2 Visuals::GetPosnScale(D3DXVECTOR2 *scale, byte *AN, double *tbl)
{
	bool beforeCursor = !(Visual >= VECTORCLIP || Visual == MOVE || 
		Visual == CHANGEPOS) && replaceTagsInCursorPosition;

	bool draw = (Visual == VECTORCLIP || Visual == VECTORDRAW);
	D3DXVECTOR2 ppos(0.0f, 0.0f);
	EditBox *edit = tab->Edit;
	SubsGrid *grid = tab->Grid;
	wxString txt = edit->TextEdit->GetValue();
	TextEditor *editor = edit->TextEdit;
	if (grid->hasTLMode && txt == L""){ 
		txt = edit->TextEditOrig->GetValue(); 
		editor = edit->TextEditOrig; 
	}


	Styles *currentStyle = grid->GetStyle(0, edit->line->Style);
	bool foundpos = false;
	wxRegEx pos(L"\\\\(pos|move)\\(([^\\)]+)\\)", wxRE_ADVANCED);
	if (pos.Matches(txt) && tbl){
		wxString type = pos.GetMatch(txt, 1);
		wxString txtpos = pos.GetMatch(txt, 2);
		wxStringTokenizer tkz(txtpos, L",");
		int ipos = 0; //tbl[4]=0; tbl[5]=0;
		while (tkz.HasMoreTokens() && ipos < 6){
			wxString token = tkz.GetNextToken();
			if (!token.ToDouble(&tbl[ipos])){ tbl[ipos] = 0; }
			ipos++;
		}
		tbl[4] += edit->line->Start.mstime; 
		tbl[5] += edit->line->Start.mstime;
		tbl[6] = ipos;
		if (ipos > 1){ ppos.x = tbl[0]; ppos.y = tbl[1]; foundpos = true; }

	}
	else{
		if (tbl){ tbl[6] = 0; }
		ppos.x = (edit->line->MarginL != 0) ? edit->line->MarginL : wxAtoi(currentStyle->MarginL);
		ppos.y = (edit->line->MarginV != 0) ? edit->line->MarginV : wxAtoi(currentStyle->MarginV);
		D3DXVECTOR2 additional = GetDialogueAdditionalPosition(edit->line);
		ppos.y += additional.y;
	}

	if (tbl && tbl[6] < 4){
		int startTime = ZEROIT(edit->line->Start.mstime);
		int start, end;
		GetMoveTimes(&start, &end);
		tbl[4] = startTime + start;
		tbl[5] = startTime + end;
	}

	double fscx = 100.0, fscy = 100.0;
	if (!(FindTag(L"fscx([.0-9-]+)", txt, !beforeCursor) && GetDouble(&fscx))){
		currentStyle->ScaleX.ToDouble(&fscx);
	}
	if (!(FindTag(L"fscy([.0-9-]+)", txt, !beforeCursor) && GetDouble(&fscy))){
		currentStyle->ScaleY.ToDouble(&fscy);
	}
	if (scale){
		scale->x = fscx / 100.f;
		scale->y = fscy / 100.f;
	}
	if (draw){
		wxRegEx drawscale;
		if (Visual == VECTORCLIP){
			*scale = D3DXVECTOR2(1.f, 1.f);
			drawscale.Compile(L"\\\\i?clip\\(([0-9]+),", wxRE_ADVANCED);
		}
		else{
			drawscale.Compile(L"\\\\p([0-9]+)", wxRE_ADVANCED);
		}
		int dscale = 1;
		if (drawscale.Matches(txt)){
			((DrawingAndClip*)this)->vectorScale = dscale = wxAtoi(drawscale.GetMatch(txt, 1));
		}
		dscale = pow(2.f, (dscale - 1.f));
		scale->x /= dscale;
		scale->y /= dscale;
	}
	if (Visual != VECTORCLIP){
		int tmpan;
		tmpan = wxAtoi(currentStyle->Alignment);
		wxRegEx an(L"\\\\an([0-9]+)", wxRE_ADVANCED);
		if (an.Matches(txt)){
			tmpan = wxAtoi(an.GetMatch(txt, 1));
		}
		if (AN){ *AN = tmpan; }
		if (foundpos){ return ppos; }
		
		SetPositionByAn(&ppos, tmpan, edit->line, currentStyle);
	}


	return ppos;
}


//Put visuals in line text
void Visuals::SetVisual(bool dummy)
{
	EditBox *edit = tab->Edit;
	SubsGrid *grid = tab->Grid;

	bool isOriginal = (grid->hasTLMode && edit->TextEdit->GetValue() == L"");
	//Get editor
	TextEditor *editor = (isOriginal) ? edit->TextEditOrig : edit->TextEdit;
	//two stages, stage first selected lines
	if (edit->IsCursorOnStart()){
		bool showOriginalOnVideo = !Options.GetBool(TL_MODE_HIDE_ORIGINAL_ON_VIDEO);
		wxString *dtxt;
		wxArrayInt sels;
		grid->file->GetSelections(sels);
		bool skipInvisible = dummy && tab->Video->GetState() != Playing;
		if (dummy && (!dummytext || selPositions.size() != sels.size())){
			bool visible = false;
			selPositions.clear();
			//need to check if can delete when sizes are different dummytext is valid pointer
			SAFE_DELETE(dummytext);
			dummytext = grid->GetVisible(&visible, 0, &selPositions);
			if (selPositions.size() != sels.size()){
				//KaiLog(L"Sizes mismatch");
				return;
			}
		}
		if (dummy){ dtxt = new wxString(*dummytext); }
		int _time = tab->Video->Tell();
		int moveLength = 0;
		const wxString &tlStyle = tab->Grid->GetSInfo(L"TLMode Style");
		for (size_t i = 0; i < sels.size(); i++){

			Dialogue *Dial = grid->GetDialogue(sels[i]);
			if (skipInvisible && !(_time >= Dial->Start.mstime && _time <= Dial->End.mstime)){ continue; }

			wxString txt = Dial->GetTextNoCopy();
			ChangeVisual(&txt, Dial);
			if (!dummy){
				grid->CopyDialogue(sels[i])->SetText(txt);
			}
			else{
				Dialogue Cpy = Dialogue(*Dial);
				if (Dial->TextTl != L"" && grid->hasTLMode) {
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

		if (!dummy){
			SetModified((Visual == MOVE) ? VISUAL_MOVE :
				(Visual == SCALE) ? VISUAL_SCALE : 
				(Visual == ROTATEZ) ? VISUAL_ROTATION_Z :
				(Visual == ROTATEXY) ? VISUAL_ROTATION_X_Y : 
				(Visual == CLIPRECT)? VISUAL_RECT_CLIP : VISUAL_ALL_TAGS, true);
		}
		else{
			RenderSubs(dtxt);
		}
		return;
	}
	//put it on to editor
	if (dummy){
		//in text position need to always get new value
		//no addition to old value, always placed a new one
		wxString txt = replaceTagsInCursorPosition? editor->GetValue() : currentLineText;
		wxPoint positionInText = ChangeVisual(&txt);
		if (!dummytext){
			bool vis = false;
			dummytext = grid->GetVisible(&vis, &dumplaced);
			if (!vis){ SAFE_DELETE(dummytext); return; }
		}
		editor->SetTextS(txt, false, false);
		editor->SetSelection(positionInText.x, positionInText.x, true);
		dummytext->replace(dumplaced.x, dumplaced.y, txt);
		dumplaced.y = txt.length();
		wxString *dtxt = new wxString(*dummytext);
		RenderSubs(dtxt);
	}
	else{
		editor->SetModified();
		currentLineText = editor->GetValue();
		tab->Video->SetVisualEdition(true);
		if (edit->splittedTags){ edit->TextEditOrig->SetModified(); }
		edit->Send((Visual == MOVE) ? VISUAL_MOVE :
			(Visual == SCALE) ? VISUAL_SCALE : (Visual == ROTATEZ) ? VISUAL_ROTATION_Z :
			(Visual == ROTATEXY) ? VISUAL_ROTATION_X_Y : 
			(Visual == CLIPRECT) ? VISUAL_RECT_CLIP : VISUAL_ALL_TAGS, false, false, true);

	}
}

D3DXVECTOR2 Visuals::GetPosition(Dialogue *Dial, bool *putinBracket, wxPoint *TextPos)
{
	
	//to fix bug with positioning multiple lines without \pos
	//need to make method that seek for these lines
	//calculate right position and pass results to positioning and move.
	*putinBracket = false;
	D3DXVECTOR2 result;
	Styles *currentStyle = tab->Grid->GetStyle(0, Dial->Style);
	const wxString &txt = Dial->GetText();
	bool foundpos = false;
	wxRegEx pos(L"\\\\(pos|move)\\(([^\\)]+)\\)", wxRE_ADVANCED);
	if (pos.Matches(txt)){
		wxString txtpos = pos.GetMatch(txt, 2);
		double posx = 0, posy = 0;
		wxString rest, rest1;
		bool res1 = txtpos.BeforeFirst(L',', &rest).ToCDouble(&posx);
		bool res2 = rest.BeforeFirst(L',', &rest1).ToCDouble(&posy);
		size_t startMatch, lenMatch;
		if (pos.GetMatch(&startMatch, &lenMatch, 0)){
			TextPos->x = startMatch;
			TextPos->y = lenMatch;
		}
		result = D3DXVECTOR2(posx, posy);
		if (res1 && res2){ 
			return result; 
		}
	}

	if (txt != L"" && txt[0] == L'{'){
		TextPos->x = 1;
		TextPos->y = 0;
	}
	else{
		TextPos->x = 0;
		TextPos->y = 0;
		*putinBracket = true;
	}
	int tmpan;
	tmpan = wxAtoi(currentStyle->Alignment);
	wxRegEx an(L"\\\\an([0-9]+)", wxRE_ADVANCED);
	if (an.Matches(txt)){
		tmpan = wxAtoi(an.GetMatch(txt, 1));
	}
	SetPositionByAn(&result, tmpan, Dial, currentStyle);
	D3DXVECTOR2 additional = GetDialogueAdditionalPosition(Dial);
	result.y += additional.y;
	return result;
}

void Visuals::ChangeOrg(wxString *txt, Dialogue *_dial, float coordx, float coordy)
{
	wxString val;
	double orgx = 0, orgy = 0;
	bool PutinBrackets = false;
	wxPoint strPos;
	if (FindTag(L"org(\\(.+)\\)", *txt, 1)){
		const FindData& data = GetResult();
		GetTwoValueDouble(&orgx, &orgy);
		PutinBrackets = false;
		strPos = data.positionInText;
	}
	else{
		D3DXVECTOR2 pos = GetPosition(_dial, &PutinBrackets, &strPos);
		orgx = pos.x;
		orgy = pos.y;
		if (strPos.y == 0){
			wxString posTag = L"\\pos(" + getfloat(pos.x) + L"," + getfloat(pos.y) + L")";
			int append = ChangeText(txt, posTag, !PutinBrackets, strPos);
			strPos.x += posTag.length() + append;
			PutinBrackets = false;
		}
		else
		{
			strPos.y = 0;
		}
	}
	strPos.y += strPos.x - 1;
	ChangeText(txt, L"\\org(" + getfloat(orgx + coordx) + L"," + getfloat(orgy + coordy) + L")", !PutinBrackets, strPos);
}

void Visuals::SetModified(int action, bool dummy)
{
	tab->Video->SetVisualEdition(true);
	if (tab->Edit->splittedTags){ 
		tab->Edit->TextEditOrig->SetModified(); 
	}
	tab->Grid->SetModified(action, true, dummy);
	if (dummy) {
		VideoCtrl* vb = tab->Video;
		if (vb->IsShown() || vb->IsFullScreen()) { vb->OpenSubs(OPEN_DUMMY); }
		if (vb->GetState() == Paused)
			vb->Render();
	}
	tab->Grid->Refresh();
}

bool Visuals::GetTextExtents(const wxString & text, Styles *style, float* width, float* height, float* descent, float* extlead)
{
	float fwidth = 0, fheight = 0, fdescent = 0, fextlead = 0;
	float fontsize = style->GetFontSizeDouble() * 32;
	float spacing = wxAtof(style->Spacing) * 32;

	
	size_t thetextlen = text.length();
	if (!thetextlen) {
		*width = 0;
		*height = 0;
		if (descent)
			*descent = 0;
		if (extlead)
			*extlead = 0;
		return true;
	}

	const wchar_t* thetext = text.wc_str();

	
	SIZE sz;
	HDC thedc = CreateCompatibleDC(0);
	if (!thedc) return false;
	SetMapMode(thedc, MM_TEXT);

	LOGFONTW lf;
	ZeroMemory(&lf, sizeof(lf));
	lf.lfHeight = (LONG)fontsize;
	lf.lfWeight = style->Bold ? FW_BOLD : FW_NORMAL;
	lf.lfItalic = style->Italic;
	lf.lfUnderline = style->Underline;
	lf.lfStrikeOut = style->StrikeOut;
	lf.lfCharSet = wxAtoi(style->Encoding);
	lf.lfOutPrecision = OUT_TT_PRECIS;
	lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	lf.lfQuality = ANTIALIASED_QUALITY;
	lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
	_tcsncpy(lf.lfFaceName, style->Fontname.wc_str(), 32);

	HFONT thefont = CreateFontIndirect(&lf);
	if (!thefont) return false;
	SelectObject(thedc, thefont);

	if (spacing != 0) {
		fwidth = 0;
		for (unsigned int i = 0; i < thetextlen; i++) {
			GetTextExtentPoint32(thedc, &thetext[i], 1, &sz);
			fwidth += sz.cx + spacing;
			fheight = sz.cy;
		}
	}
	else {
		GetTextExtentPoint32(thedc, thetext, (int)thetextlen, &sz);
		fwidth = sz.cx;
		fheight = sz.cy;
	}


	TEXTMETRIC tm;
	GetTextMetrics(thedc, &tm);
	fdescent = tm.tmDescent;
	fextlead = tm.tmExternalLeading;

	DeleteObject(thedc);
	DeleteObject(thefont);
	float scalex = wxAtof(style->ScaleX) / 100.f;
	float scaley = wxAtof(style->ScaleY) / 100.f;

	*width = scalex * (fwidth / 32);
	*height = scaley * (fheight / 32);
	if(descent)
		*descent = scaley * (fdescent / 32);
	if(extlead)
		*extlead = scaley * (fextlead / 32);

	return true;
}

D3DXVECTOR2 Visuals::GetTextSize(Dialogue* dial, D3DXVECTOR2* border, Styles* style, bool keepExtraLead, D3DXVECTOR2* extralead)
{
	wxString tags[] = { L"p", L"fscx", L"fscy", L"fsp", L"fs", L"fn", L"bord", L"xbord", L"ybord" };
	D3DXVECTOR2 result = D3DXVECTOR2(0.f, 0.f);
	const wxString& text = dial->GetTextNoCopy();
	if (!text.length())
		return result;

	Styles* measuringStyle = NULL;
	if (style)
		measuringStyle = style->Copy();
	else
		measuringStyle = tab->Grid->GetStyle(0, tab->Edit->line->Style)->Copy();

	dial->ParseTags(tags, 9, true);
	ParseData *presult = dial->parseData;
	float bord = measuringStyle->GetOtlineDouble();
	float xbord = bord;
	float xbord1 = bord;
	float ybord1 = bord;
	wxString drawingText;
	bool wasPlain = false;
	float maxwidth = 0.f;
	float maxheight = 0.f;
	float extlead = 0.f;
	float descent = 0.f;
	if (extralead) {
		*extralead = { 0, 0 };
	}
	for (auto tag : presult->tags) {
		if (tag->tagName == L"p") {
			if (!tag->value.IsNumber()) {
				drawingText << tag->value;
			}
			else if(tag->value == L"0"){
				D3DXVECTOR2 drawingsize = GetDrawingSize(drawingText);
				result += drawingsize;
				drawingText.clear();
			}
		}//set first and last bord
		else if (tag->tagName.EndsWith(L"ord")) {
			bool isx = tag->tagName[0] != L'y';
			bool isy = tag->tagName[0] != L'x';
			float bord = wxAtof(tag->value);
			if (isx) {
				//last bord in last bracket
				xbord1 = bord;
				//last bord in first bracket
				if (!wasPlain)
					xbord = bord;
			}
			if (isy) {
				//last bord in last bracket
				ybord1 = bord;
			}
		}//plain text for measuring
		else if (tag->tagName == L"plain") {
			if (!tag->value.empty()) {
				wasPlain = true;
				size_t i = 0;
				size_t g = 0;
				float fwidth = 0;
				float fheight = 0;
				while (i != -1) {
					i = tag->value.find(L"\\N", (i > 0)? i + 2 : i);
					wxString pltext = tag->value.Mid(g, i - g);
					if (GetTextExtents(pltext, measuringStyle, &fwidth, &fheight, &extlead, &descent)) {
						maxwidth += fwidth;
						if(!result.y && !keepExtraLead)
							fheight -= (extlead - descent);
						if (extralead) {
							if (extralead->x < extlead)
								extralead->x = extlead;
							if (extralead->y < descent)
								extralead->y = descent;
						}

						if (maxheight < fheight)
							maxheight = fheight;
						if (i != -1) {
							if (maxwidth > result.x)
								result.x = maxwidth;

							result.y += maxheight;
							maxwidth = 0.f;
							maxheight = 0.f;
						}
					}
					else {
						KaiLog("Cannot measure text: " + tag->value.Mid(g, i - g));
					}
					g = i + 2;
				}
			}
		}//assign to style
		else {
			if (!TagValueToStyle(measuringStyle, tag->tagName, tag->value))
				KaiLog(L"Cannot assign style to tag: " + tag->tagName + " with value: " + tag->value);
		}
	}
	if (!drawingText.empty()) {
		D3DXVECTOR2 drawingsize = GetDrawingSize(drawingText);
		result += drawingsize;
	}
	//don't know if it's needed
	if (maxwidth || maxheight) {
		if (maxwidth > result.x)
			result.x = maxwidth;

		result.y += maxheight;
	}
	if (extralead) {
		if (extralead->x < extlead)
			extralead->x = extlead;
		if (extralead->y < descent)
			extralead->y = descent;
	}
	if (border) {
		border->x = xbord + xbord1;
		border->y = ybord1 * 2;
	}
	dial->ClearParse();
	delete measuringStyle;
	return result;
}

D3DXVECTOR2 Visuals::GetDrawingSize(const wxString& drawing)
{
	double minx = DBL_MAX;
	double miny = DBL_MAX;
	double maxx = -DBL_MAX;
	double maxy = -DBL_MAX;
	wxStringTokenizer tokenzr(drawing, L" ");
	double value;
	bool isX = true;
	while (tokenzr.HasMoreTokens()) {
		wxString token = tokenzr.GetNextToken();
		if (token == L"m" || token == L"l" || token == L"b" || token == L"s" || token == L"c")
			continue;
		if (token.EndsWith(L"c"))
			token = token.Mid(0, token.length() - 1);

		if (token.ToDouble(&value)) {
			if (isX) {
				if (value < minx) { minx = value; }
				if (value > maxx) { maxx = value; }
				isX = false;
			}
			else {
				if (value < miny) { miny = value; }
				if (value > maxy) { maxy = value; }
				isX = true;
			}
		}
		else {
			KaiLog(wxString::Format(_("Zła wartość punktu rysunku %s."), token));
		}
	}
	return D3DXVECTOR2(maxx - minx, maxy - miny);
}

void Visuals::RotateZ(D3DXVECTOR2* point, float sinOfAngle, float cosOfAngle, D3DXVECTOR2 orgpivot)
{
	float x = point->x - orgpivot.x;
	float y = point->y - orgpivot.y;
	point->x = (x * cosOfAngle) - (y * sinOfAngle) + orgpivot.x;
	point->y = (x * sinOfAngle) + (y * cosOfAngle) + orgpivot.y;
}

void Visuals::RotateDrawing(ClipPoint* point, float sinOfAngle, float cosOfAngle, D3DXVECTOR2 orgpivot)
{
	float x = point->x + orgpivot.x;
	float y = point->y + orgpivot.y;
	point->x = (x * cosOfAngle) - (y * sinOfAngle) - orgpivot.x;
	point->y = (x * sinOfAngle) + (y * cosOfAngle) - orgpivot.y;
}

D3DXVECTOR2 Visuals::CalcDrawingSize(int alignment, std::vector<ClipPoint>* points, bool withoutAlignment)
{
	if ((alignment == 7 && !withoutAlignment) || points->size() < 1) { return D3DXVECTOR2(0, 0); }

	float minx = FLT_MAX;
	float miny = FLT_MAX;
	float maxx = -FLT_MAX;
	float maxy = -FLT_MAX;
	for (size_t i = 0; i < points->size(); i++)
	{
		ClipPoint p = points->at(i);
		if (p.x < minx) { minx = p.x; }
		if (p.y < miny) { miny = p.y; }
		if (p.x > maxx) { maxx = p.x; }
		if (p.y > maxy) { maxy = p.y; }
	}
	D3DXVECTOR2 sizes((maxx - minx), (maxy - miny));
	if (withoutAlignment)
		return sizes;

	D3DXVECTOR2 result = D3DXVECTOR2(0, 0);
	if (alignment % 3 == 2) {
		result.x = sizes.x / 2.0;
	}
	else if (alignment % 3 == 0) {
		result.x = sizes.x;
	}
	if (alignment < 4) {
		result.y = sizes.y;
	}
	else if (alignment < 7) {
		result.y = sizes.y / 2.0;
	}
	return result;
}
