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
//#include <regex>


Visuals *Visuals::Get(int Visual, wxWindow *_parent)
{
	Visuals *visual;
	switch (Visual)
	{
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
	case VECTORDRAW:
		visual = new DrawingAndClip();
		break;
	case SCALE_ROTATION:
		visual = new ScaleRotation();
		break;
	default:
		visual = new Position();
		break;
	}

	visual->tab = (TabPanel*)_parent->GetParent();
	visual->Visual = Visual;
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

void Visuals::GetDialoguesWithoutPosition()
{
	int time = tab->Video->Tell();
	SubsGrid *grid = tab->Grid;

	wxRegEx pos(L"\\\\(pos|move)\\(([^\\)]+)\\)", wxRE_ADVANCED);
	bool tlMode = tab->Grid->hasTLMode;
	int activeLineKey = tab->Grid->currentLine;

	for (size_t i = 0; i < grid->file->GetCount(); i++){
		Dialogue *dial = grid->file->GetDialogue(i);
		if (!grid->ignoreFiltered && !dial->isVisible || dial->NonDialogue || activeLineKey == i){ continue; }

		if (time >= dial->Start.mstime && time < dial->End.mstime){
			const wxString &text = dial->GetTextNoCopy();
			if (!pos.Matches(text)){
				dialoguesWithoutPosition.push_back(dial);
			}
		}
	}
}

int Visuals::GetDialoguePosition()
{
	if (!dialoguesWithoutPosition.size())
		return 0;

	bool tlMode = tab->Grid->hasTLMode;

	for (size_t i = 0; i < dialoguesWithoutPosition.size(); i++){
		Dialogue *dial = dialoguesWithoutPosition[i];
		Styles *acstyle = tab->Grid->GetStyle(0, dial->Style);
		const wxString &txt = dial->GetTextNoCopy();
		int newan = wxAtoi(acstyle->Alignment);
		wxRegEx an(L"\\\\an([0-9]+)", wxRE_ADVANCED);
		if (an.Matches(txt)){
			newan = wxAtoi(an.GetMatch(txt, 1));
		}
		//if (((AN - 1) / 3) == ((newan - 1) / 3)){

		//}
		
	}
	return 0;
}

void Visuals::RenderSubs(wxString *subs, bool redraw /*= true*/)
{
	if (!tab->Video->OpenSubs(subs)){ KaiLog(_("Nie można otworzyć napisów")); }
	tab->Video->hasVisualEdition = true;
	if (redraw){ tab->Video->Render(); }
}

void Visuals::SetVisual(int _start, int _end, bool notDial, bool noRefresh)
{
	int nx = 0, ny = 0;
	tab->Grid->GetASSRes(&nx, &ny);
	SubsSize = wxSize(nx, ny);
	start = _start;
	end = _end;
	notDialogue = notDial;

	coeffW = ((float)SubsSize.x / (float)(VideoSize.width - VideoSize.x));
	coeffH = ((float)SubsSize.y / (float)(VideoSize.height - VideoSize.y));
	tab->Video->hasVisualEdition = true;

	SetCurVisual();
	if (Visual == VECTORCLIP){
		SetClip(GetVisual(), true, true, false); 
		return;
	}
	tab->Video->Render(!noRefresh, false);
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
	if (!(time >= start && time < end) || (notDialogue && Visual != VECTORDRAW)){ 
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

//Getting position and scale with vector drawing need to put scale in rest of cases can put one value or none
D3DXVECTOR2 Visuals::GetPosnScale(D3DXVECTOR2 *scale, byte *AN, double *tbl)
{
	bool beforeCursor = !(Visual >= VECTORCLIP || Visual == MOVE || Visual == CHANGEPOS);
	bool draw = (Visual == VECTORCLIP || Visual == VECTORDRAW);
	D3DXVECTOR2 ppos(0.0f, 0.0f);
	EditBox *edit = tab->Edit;
	SubsGrid *grid = tab->Grid;
	wxString txt = edit->TextEdit->GetValue();
	TextEditor *Editor = edit->TextEdit;
	if (grid->hasTLMode && txt == L""){ txt = edit->TextEditOrig->GetValue(); Editor = edit->TextEditOrig; }


	Styles *acstyl = grid->GetStyle(0, edit->line->Style);
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
		tbl[4] += edit->line->Start.mstime; tbl[5] += edit->line->Start.mstime;
		tbl[6] = ipos;
		if (ipos > 1){ ppos.x = tbl[0]; ppos.y = tbl[1]; foundpos = true; }

	}
	else{
		if (tbl){ tbl[6] = 0; }
		ppos.x = (edit->line->MarginL != 0) ? edit->line->MarginL : wxAtoi(acstyl->MarginL);
		ppos.y = (edit->line->MarginV != 0) ? edit->line->MarginV : wxAtoi(acstyl->MarginV);
	}

	if (tbl && tbl[6] < 4){
		VideoCtrl *video = tab->Video;
		float fps = video->fps;
		bool dshow = video->IsDshow;
		int startTime = ZEROIT(edit->line->Start.mstime);
		int endTime = ZEROIT(edit->line->End.mstime);
		int framestart = (dshow) ? (((float)startTime / 1000.f) * fps) + 1 : video->VFF->GetFramefromMS(startTime);
		int frameend = (dshow) ? ((float)endTime / 1000.f) * fps : video->VFF->GetFramefromMS(endTime) - 1;
		int msstart = (dshow) ? ((framestart * 1000) / fps) + 0.5f : video->VFF->GetMSfromFrame(framestart);
		int msend = (dshow) ? ((frameend * 1000) / fps) + 0.5f : video->VFF->GetMSfromFrame(frameend);
		int diff = endTime - startTime;

		tbl[4] = startTime + abs(msstart - startTime);
		tbl[5] = startTime + (diff - abs(endTime - msend));
	}

	wxString sxfd, syfd;
	bool scx = edit->FindValue(L"fscx([.0-9-]+)", &sxfd, L"", 0, !beforeCursor);
	bool scy = edit->FindValue(L"fscy([.0-9-]+)", &syfd, L"", 0, !beforeCursor);
	double fscx = 100.0, fscy = 100.0;
	if (scx){
		sxfd.ToDouble(&fscx);
	}
	else{
		acstyl->ScaleX.ToDouble(&fscx);
	}
	if (scy){
		syfd.ToDouble(&fscy);
	}
	else{
		acstyl->ScaleY.ToDouble(&fscy);
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
		tmpan = wxAtoi(acstyl->Alignment);
		wxRegEx an(L"\\\\an([0-9]+)", wxRE_ADVANCED);
		if (an.Matches(txt)){
			tmpan = wxAtoi(an.GetMatch(txt, 1));
		}
		if (AN){ *AN = tmpan; }
		if (foundpos){ return ppos; }
		int x, y;
		grid->GetASSRes(&x, &y);
		if (tmpan % 3 == 2){
			ppos.x = (x / 2);
		}
		else if (tmpan % 3 == 0){
			ppos.x = (edit->line->MarginR != 0) ? edit->line->MarginR : wxAtoi(acstyl->MarginR);
			ppos.x = x - ppos.x;
		}
		if (tmpan < 4){
			ppos.y = (edit->line->MarginV != 0) ? edit->line->MarginV : wxAtoi(acstyl->MarginV);
			ppos.y = y - ppos.y;
		}
		else if (tmpan < 7){
			ppos.y = (y / 2);
		}
	}


	return ppos;
}

//function return 1 when need to add bracket or 0
int ChangeText(wxString *txt, const wxString &what, bool inbracket, const wxPoint &pos)
{
	if (!inbracket){
		txt->insert(pos.x, L"{" + what + L"}");
		return 1;
	}
	if (pos.x < pos.y){ txt->erase(txt->begin() + pos.x, txt->begin() + pos.y + 1); }
	txt->insert(pos.x, what);
	return 0;
}

void Visuals::SetClip(wxString clip, bool dummy, bool redraw, bool changeEditorText)
{

	EditBox *edit = tab->Edit;
	SubsGrid *grid = tab->Grid;
	bool isOriginal = (grid->hasTLMode && edit->TextEdit->GetValue() == L"");
	//Editor
	TextEditor *Editor = (isOriginal) ? edit->TextEditOrig : edit->TextEdit;
	if (clip == L""){

		wxString tmp;
		wxString txt = Editor->GetValue();
		if (edit->FindValue(L"(i?clip.)[^)]*\\)", &tmp, txt, 0, 1)){
			ChangeText(&txt, L"", edit->InBracket, edit->Placed);
			txt.Replace(L"{}", L"");
			if (changeEditorText){
				Editor->SetTextS(txt, false, true);
				Editor->SetModified();
				edit->Send(VISUAL_VECTOR_CLIP, false);
			}
			return;
		}
		tab->Video->hasVisualEdition = false;
		RenderSubs(tab->Grid->GetVisible(), redraw);
		return;
	}
	if (dummy){

		if (!dummytext){
			bool vis = false;
			dummytext = grid->GetVisible(&vis, &textplaced);
			if (!vis){ SAFE_DELETE(dummytext); return; }
			//vector clip
			if (Visual == VECTORCLIP){
				wxString tmp = L"clip(";
				wxString txt = Editor->GetValue();
				bool fv = edit->FindValue(L"(i?clip.)[^)]*\\)", &tmp, txt, 0, 1);
				wxString tmp1 = (tmp[0] == L'c') ? L"iclip(" : L"clip(";
				wxString tclip = L"\\" + tmp + clip + L")";
				edit->Placed.x += tmp.length() + 1 + ChangeText(&txt, tclip, edit->InBracket, edit->Placed);
				edit->Placed.y = edit->Placed.x + clip.length();

				dummytext->replace(textplaced.x, textplaced.y, txt);
				textplaced.y = txt.length();
				int nx = 0, ny = 0;
				grid->GetASSRes(&nx, &ny);
				Dialogue *maskDialogue = edit->line->Copy();
				wxString text;
				text << L"{\\p1\\bord0\\shad0\\fscx100\\fscy100\\1c&H000000&\\1a&H77&\\pos(0,0)\\an7\\" << tmp1 << clip << L")}m 0 0 l " <<
					nx << L" 0 " << nx << L" " << ny << L" 0 " << ny;
				maskDialogue->SetTextElement(TXT, text);
				maskDialogue->GetRaw(dummytext);
				dumplaced.x = edit->Placed.x + textplaced.x; 
				dumplaced.y = edit->Placed.y + textplaced.x;
				delete maskDialogue;
				if (changeEditorText){
					Editor->SetTextS(txt, false, true);
					Editor->SetModified();
				}

			}
			else{//vector drawings
				wxString tmp = L"";
				bool isf;
				bool hasP1 = true;
				size_t cliplen = clip.length();
				wxString txt = Editor->GetValue();
				isf = edit->FindValue(L"p([0-9]+)", &tmp, txt, 0, 1);
				if (!isf){
					ChangeText(&txt, L"\\p1", edit->InBracket, edit->Placed);
					hasP1 = false;
				}
				isf = edit->FindValue(L"pos\\(([,. 0-9-]+)\\)", &tmp, txt, 0, 1);
				if (!isf){
					DrawingAndClip *drawing = (DrawingAndClip*)this;
					float xx = drawing->_x * drawing->scale.x;
					float yy = drawing->_y * drawing->scale.y;
					ChangeText(&txt, L"\\pos(" + getfloat(xx) + L"," + getfloat(yy) + L")", edit->InBracket, edit->Placed);
				}
				isf = edit->FindValue(L"an([0-9])", &tmp, txt, 0, 1);
				if (!isf){
					DrawingAndClip *drawing = (DrawingAndClip*)this;
					ChangeText(&txt, L"\\an" + getfloat(drawing->alignment, L"1.0f"), edit->InBracket, edit->Placed);
				}

				int bracketPos = 0;
				while (1){
					bracketPos = txt.find(L"}", bracketPos);
					if (bracketPos < 0 || bracketPos == txt.length() - 1){
						break;
					}
					bracketPos++;
					wxString mcheck = txt.Mid(bracketPos, 2);
					if (!mcheck.StartsWith(L"m ") && !mcheck.StartsWith(L"{")){
						txt.insert(bracketPos, L"{");
						bracketPos++;
						bracketPos = txt.find(L"{", bracketPos);
						if (bracketPos < 0){
							txt << L"}";
							break;
						}
						txt.insert(bracketPos, L"}");
					}
				}
				
				wxString afterP1 = txt.Mid(edit->Placed.y);
				int Mpos = -1;
				//FIXME: removing first bracket
				if (hasP1){ Mpos = afterP1.find(L"m "); }
				if (Mpos == -1){ Mpos = afterP1.find(L"}") + 1; }
				wxString startM = afterP1.Mid(Mpos);
				int endClip = startM.find(L"{");
				if (endClip == -1){
					if (isf){ endClip = startM.length(); }
					else{ endClip = 0; }
					clip += L"{\\p0}";
				}
				else if (!hasP1){
					clip += L"{\\p0}";
				}
				txt.replace(Mpos + edit->Placed.y, endClip, clip);

				if (changeEditorText){
					Editor->SetTextS(txt, false, true);
					Editor->SetModified();
				}

				dummytext->replace(textplaced.x, textplaced.y, txt);
				textplaced.y = txt.length();
				dumplaced.x = edit->Placed.y + Mpos + textplaced.x; dumplaced.y = dumplaced.x + cliplen;

			}
		}
		else{
			//clip change
			dummytext->replace(dumplaced.x, dumplaced.y - dumplaced.x, clip);
			//new positions for new change
			int oldy = dumplaced.y;
			dumplaced.y = dumplaced.x + clip.length();
			textplaced.y += (dumplaced.y - oldy);
			if (Visual == VECTORCLIP){
				//change mask clip
				int endclip = dummytext->Find(L')', true);
				int startclip = dummytext->Find(L'(', true);
				dummytext->replace(startclip + 1, endclip - (startclip + 1), clip);
			}
			//get line text
			wxString txt = dummytext->Mid(textplaced.x, textplaced.y);
			//put in text field
			if (changeEditorText){
				Editor->SetTextS(txt, false, true);
				Editor->SetModified();
			}
		}

		tab->Video->hasVisualEdition = false;
		wxString *dtxt = new wxString(*dummytext);
		RenderSubs(dtxt, redraw);

	}
	else{

		Editor->SetModified();
		edit->UpdateChars();
		tab->Video->hasVisualEdition = true;
		if (edit->splittedTags){ edit->TextEditOrig->SetModified(); }
		edit->Send((Visual == VECTORCLIP) ? VISUAL_VECTOR_CLIP : VISUAL_DRAWING, false, false, true);
	}
}

//Put visuals in line text
void Visuals::SetVisual(bool dummy, int type)
{
	EditBox *edit = tab->Edit;
	SubsGrid *grid = tab->Grid;

	bool isOriginal = (grid->hasTLMode && edit->TextEdit->GetValue() == L"");
	//Get editor
	TextEditor *Editor = (isOriginal) ? edit->TextEditOrig : edit->TextEdit;
	//two stages, stage first selected lines
	if (edit->IsCursorOnStart()){
		bool showOriginalOnVideo = !Options.GetBool(TL_MODE_HIDE_ORIGINAL_ON_VIDEO);
		wxString *dtxt;
		wxArrayInt sels;
		tab->Grid->file->GetSelections(sels);
		bool skipInvisible = dummy && tab->Video->GetState() != Playing;
		if (dummy && !dummytext){
			bool visible = false;
			selPositions.clear();
			dummytext = tab->Grid->GetVisible(&visible, 0, &selPositions);
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
				tab->Grid->CopyDialogue(sels[i])->SetText(txt);
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

		if (!dummy){
			tab->Video->hasVisualEdition = true;
			if (tab->Edit->splittedTags){ tab->Edit->TextEditOrig->SetModified(); }
			tab->Grid->SetModified((Visual == MOVE) ? VISUAL_MOVE :
				(Visual == SCALE) ? VISUAL_SCALE : (Visual == ROTATEZ) ? VISUAL_ROTATION_Z :
				(Visual == ROTATEXY) ? VISUAL_ROTATION_X_Y : VISUAL_RECT_CLIP, true);
			tab->Grid->Refresh();
		}
		else{
			RenderSubs(dtxt);
		}
		return;
	}
	//put it on to editor
	if (dummy){
		wxString txt = Editor->GetValue();
		int mode = false;
		if (Visual == MOVE){ mode = 1; }
		else if (Visual == CLIPRECT){ mode = 2; }
		wxString tmp;
		wxString xytype = (type == 0) ? L"x" : L"y";
		wxString frxytype = (type == 1) ? L"x" : L"y";

		wxString tagpattern = (type == 100) ? L"(org).+" :
			(Visual == MOVE) ? L"(move|pos).+" :
			(Visual == SCALE) ? L"(fsc" + xytype + L").+" :
			(Visual == ROTATEZ) ? L"(frz?)[.0-9-]+" :
			(Visual == ROTATEXY) ? L"(fr" + frxytype + L").+" :
			L"(i?clip).+";
		edit->FindValue(tagpattern, &tmp, txt, 0, mode);

		if (type == 2 && Visual > 0){
			if (edit->Placed.x < edit->Placed.y){ txt.erase(txt.begin() + edit->Placed.x, txt.begin() + edit->Placed.y + 1); }
			wxString tagpattern = (Visual == SCALE) ? L"(fscx).+" : (Visual == ROTATEZ) ? L"(frz?)[.0-9-]+" : L"(frx).+";
			edit->FindValue(tagpattern, &tmp, txt, 0, mode);
		}

		ChangeText(&txt, GetVisual(), edit->InBracket, edit->Placed);
		if (!dummytext){
			bool vis = false;
			dummytext = grid->GetVisible(&vis, &dumplaced);
			if (!vis){ SAFE_DELETE(dummytext); return; }
		}
		Editor->SetTextS(txt, false, false);
		Editor->SetSelection(edit->Placed.x, edit->Placed.x, true);
		dummytext->replace(dumplaced.x, dumplaced.y, txt);
		dumplaced.y = txt.length();
		wxString *dtxt = new wxString(*dummytext);
		RenderSubs(dtxt);
	}
	else{
		//Editor->Refresh(false);
		Editor->SetModified();
		tab->Video->hasVisualEdition = true;
		if (edit->splittedTags){ edit->TextEditOrig->SetModified(); }
		edit->Send((Visual == MOVE) ? VISUAL_MOVE :
			(Visual == SCALE) ? VISUAL_SCALE : (Visual == ROTATEZ) ? VISUAL_ROTATION_Z :
			(Visual == ROTATEXY) ? VISUAL_ROTATION_X_Y : VISUAL_RECT_CLIP, false, false, true);

	}
}

D3DXVECTOR2 Visuals::GetPosition(Dialogue *Dial, bool *putinBracket, wxPoint *TextPos)
{
	
	//to fix bug with positioning multiple lines without \pos
	//need to make method that seek for these lines
	//calculate right position and pass results to positioning and move.
	*putinBracket = false;
	D3DXVECTOR2 result;
	Styles *acstyl = tab->Grid->GetStyle(0, Dial->Style);
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

	result.x = (tab->Edit->line->MarginL != 0) ? tab->Edit->line->MarginL : wxAtoi(acstyl->MarginL);
	result.y = (tab->Edit->line->MarginV != 0) ? tab->Edit->line->MarginV : wxAtoi(acstyl->MarginV);

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
	tmpan = wxAtoi(acstyl->Alignment);
	wxRegEx an(L"\\\\an([0-9]+)", wxRE_ADVANCED);
	if (an.Matches(txt)){
		tmpan = wxAtoi(an.GetMatch(txt, 1));
	}
	int x, y;
	tab->Grid->GetASSRes(&x, &y);
	if (tmpan % 3 == 2){
		result.x = (x / 2);
	}
	else if (tmpan % 3 == 0){
		result.x = (Dial->MarginR != 0) ? Dial->MarginR : wxAtoi(acstyl->MarginR);
		result.x = x - result.x;
	}
	if (tmpan < 4){
		result.y = (Dial->MarginV != 0) ? Dial->MarginV : wxAtoi(acstyl->MarginV);
		result.y = y - result.y;
	}
	else if (tmpan < 7){
		result.y = (y / 2);
	}

	return result;
}

void Visuals::ChangeOrg(wxString *txt, Dialogue *_dial, float coordx, float coordy)
{
	wxString val;
	double orgx = 0, orgy = 0;
	bool PutinBrackets = false;
	wxPoint strPos;
	if (tab->Edit->FindValue(L"org\\((.+)\\)", &val, *txt, 0, 1)){
		wxString orgystr;
		wxString orgxstr = val.BeforeFirst(L',', &orgystr);
		orgxstr.ToCDouble(&orgx);
		orgystr.ToCDouble(&orgy);
		PutinBrackets = false;
		strPos = tab->Edit->Placed;
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
	}
	strPos.y += strPos.x - 1;
	ChangeText(txt, L"\\org(" + getfloat(orgx + coordx) + L"," + getfloat(orgy + coordy) + L")", !PutinBrackets, strPos);
}

