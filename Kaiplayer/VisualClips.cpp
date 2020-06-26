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

#include "TabPanel.h"
#include "Visuals.h"
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
	return (abs(pos.x - x) <= diff && abs(pos.y - y) <= diff);
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
	if (Visual == VECTORDRAW && moveValues[6]>2){ 
		D3DXVECTOR2 movePos = CalcMovePos(); 
		_x = movePos.x; 
		_y = movePos.y; 
	}
	//cannot let to "l" or "b" to be used as missing "m" on start
	if (Points[0].type != L"m"){ Points[0].type = L"m"; }
	size_t g = (size < 2) ? 0 : 1;
	size_t lastM = 0;
	bool minusminus = false;
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
		CreateVERTEX(&v9[0], pos.x - rcsize, pos.y - rcsize, 0xAACC8748);
		CreateVERTEX(&v9[1], pos.x + rcsize, pos.y - rcsize, 0xAACC8748);
		CreateVERTEX(&v9[2], pos.x - rcsize, pos.y + rcsize, 0xAACC8748);
		CreateVERTEX(&v9[3], pos.x + rcsize, pos.y + rcsize, 0xAACC8748);
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
	wxString clip;
	D3DXVECTOR2 linepos = GetPosnScale(&scale, &alignment, (Visual == VECTORDRAW) ? moveValues : NULL);

	if (Visual != VECTORDRAW){
		bool found = tab->Edit->FindValue(L"(i?clip[^)]+\\))", &clip, L"", 0, 1);
		if (found){
			int rres = clip.Replace(L",", L",");
			if (rres >= 3) { clip = L""; scale = D3DXVECTOR2(1.f, 1.f); vectorScale = 1; }
			else{ clip = clip.AfterFirst((rres > 0) ? L',' : L'('); }
		}
		coeffW /= scale.x;
		coeffH /= scale.y;
		_x = 0;
		_y = 0;
	}
	else{
		bool isOriginal = (tab->Grid->hasTLMode && tab->Edit->TextEdit->GetValue() == L"");
		//GLOBAL_EDITOR
		TextEditor *GLOBAL_EDITOR = (isOriginal) ? tab->Edit->TextEditOrig : tab->Edit->TextEdit;
		wxString tags[] = { L"p" };
		tab->Edit->line->ParseTags(tags, 1);
		ParseData *pdata = tab->Edit->line->parseData;
		if (pdata->tags.size() >= 2){
			size_t i = 1;
			while (i < pdata->tags.size()){
				if (!pdata->tags[i]->value.IsNumber()){
					clip = pdata->tags[1]->value;
					break;
				}
				i++;
			}
		}
		tab->Edit->line->ClearParse();
		_x = linepos.x / scale.x;
		_y = (linepos.y / scale.y);
		coeffW /= scale.x;
		coeffH /= scale.y;
	}

	Points.clear();
	wxStringTokenizer tokens(clip, L" ");
	double tmpx = 0;
	bool gotx = false;
	bool start = false;
	int pointsAfterStart = 1;
	wxString type = L"m";
	while (tokens.HasMoreTokens()){
		wxString token = tokens.GetNextToken();
		if (token == L"p"){ token = L"s"; }
		if (token == L"m" || token == L"l" || token == L"b" || token == L"s"){
			type = token;
			start = true;
			pointsAfterStart = 1;
		}
		else if (token == L"c"){ start = true; continue; }
		else if (gotx){
			double tmpy = 0;
			if (!token.ToCDouble(&tmpy)){ gotx = false; continue; }
			Points.push_back(ClipPoint(tmpx, tmpy, type, start));
			gotx = false;
			if ((type == L"l" || type == L"m" && pointsAfterStart == 1) || (type == L"b" && pointsAfterStart == 3)){
				if (type == L"m"){ type = L"l"; }
				start = true;
				pointsAfterStart = 0;
			}
			else{
				start = false;
			}
			pointsAfterStart++;
		}
		else{
			if (token.ToCDouble(&tmpx)){
				gotx = true;
			}
		}

	}

	if (!Points.empty() && Visual == VECTORDRAW){
		D3DXVECTOR2 xyoffset = CalcWH();
		for (size_t i = 0; i < Points.size(); i++){
			Points[i].x -= xyoffset.x;
			Points[i].y -= xyoffset.y;
		}
	}
	pointArea = 4.f / zoomScale.x;
}

wxString DrawingAndClip::GetVisual()
{
	wxString format = (Visual == VECTORDRAW) ? L"6.2f" : L"6.0f";
	wxString clip;
	if (Visual == VECTORCLIP && vectorScale > 1){
		clip << vectorScale << L",";
	}
	wxString lasttype;
	int cntb = 0;
	bool spline = false;
	offsetxy = (Visual == VECTORDRAW) ? CalcWH() : D3DXVECTOR2(0, 0);
	size_t psize = Points.size();
	for (size_t i = 0; i < psize; i++)
	{
		ClipPoint pos = Points[i];
		float x = pos.x + offsetxy.x;
		float y = pos.y + offsetxy.y;
		if (cntb && !pos.start){
			clip << getfloat(x, format) << L" " << getfloat(y, format) << L" ";
			cntb++;
			//if(cntb>2 && pos.type=="b"){cntb=0;}
		}
		else{
			if (spline){ clip << L"c "; spline = false; }
			if (lasttype != pos.type || pos.type == L"m"){ clip << pos.type << L" "; lasttype = pos.type; }
			clip << getfloat(x, format) << L" " << getfloat(y, format) << L" ";
			if (pos.type == L"b" || pos.type == L"s"){ cntb = 1; if (pos.type == L"s"){ spline = true; } }
		}
		//fix for m one after another
		if (pos.type == L"m" && psize > 1 && ((i >= psize - 1) ||
			(i < psize - 1 && Points[i + 1].type == L"m"))){
			clip << L"l " << getfloat(x, format) << L" " << getfloat(y, format) << L" ";
		}
	}
	if (spline){ clip << L"c "; }
	return clip.Trim();
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
	for (size_t i = 0; i < Points.size(); i++)
	{
		if (Points[i].IsInPos(pos, pointArea)){ return (retlast && i == 0) ? Points.size() : i; }
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
	//where put in bezier in the middle needs to move point for 1
	//cause it puts curve as previous line
	if (whereis != Points.size()){ whereis++; }
	oldpos.x = Points[whereis - 1].x;
	oldpos.y = Points[whereis - 1].y;
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
	if (Points[whereis - 1].type == L"s" || ((int)Points.size() > whereis && Points[whereis].type == L"s"))
	{
		Points.insert(Points.begin() + whereis, ClipPoint((pos.x*coeffW) - _x, (pos.y*coeffH) - _y, L"s", false));
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
	//ClipPoint tmp(0,0,"r",true);
	//if(Points[i-1].type=="s"){tmp=Points[i-1];Points[i-1]=Points[i-2];}
	if (bspline){

		int acpos = i - 1;
		int bssize = 1;
		int spos = i + 1;
		while (spos < (int)Points.size()){
			if (Points[spos].start){ break; }
			bssize++; spos++;
		}
		pts = bssize;
		bssize++;
		for (int k = 0; k < bssize; k++){
			Curve(acpos, &v4, true, bssize, k);
		}
		D3DXVECTOR2 *v2 = new D3DXVECTOR2[pts + 2];
		//if(tmp.type=="s"){Points.insert(Points.begin()+i-1,tmp);}
		for (int j = 0, g = i - 1; j < bssize; j++, g++)
		{
			v2[j] = Points[g].GetVector(this);
		}
		v2[bssize] = Points[i - 1].GetVector(this);
		line->Draw(v2, pts + 2, 0xFFAA33AA);
		int iplus1 = (i + bssize - 2 < (int)Points.size() - 1) ? i + 1 : 0;
		if (i - 1 != 0 || iplus1 != 0){
			D3DXVECTOR2 v3[3] = { Points[i - 1].GetVector(this), v4[0], Points[iplus1].GetVector(this) };
			line->Draw(v3, 3, 0xFFBB0000);
		}
		delete[] v2;
	}
	else{
		ClipPoint tmp = Points[i - 1];
		if (Points[i - 1].type == L"s"){
			int diff = 2;
			int j = i - 2;
			while (j >= 0){
				if (Points[j].type != L"s"){ break; }
				j--;
			}
			diff = (i - j) - 2;
			Points[i - 1] = Points[i - diff];
		}
		Curve(i - 1, &v4, false);
		//if(tmp.type=="s"){Points[i-1]=tmp;}
		D3DXVECTOR2 v2[4] = { Points[i - 1].GetVector(this), Points[i].GetVector(this), Points[i + 1].GetVector(this), Points[i + 2].GetVector(this) };
		line->Draw(v2, 2, 0xFF0000FF);
		line->Draw(&v2[2], 2, 0xFF0000FF);
		Points[i - 1] = tmp;
	}
	line->Draw(&v4[0], v4.size(), 0xFFBB0000);
	line->End();
	if (i > 1){ DrawRect(i - 1); }
	for (int j = 1; j < pts; j++){ DrawCircle(i + j - 1); }
	return pts;
}



void DrawingAndClip::Curve(int pos, std::vector<D3DXVECTOR2> *table, bool bspline, int spoints, int acpt)
{
	float a[4], b[4];
	float x[4], y[4];
	for (int g = 0; g < 4; g++)
	{
		if (acpt > (spoints - 1)){ acpt = 0; }
		//if(g==0 && Points[pos].type=="s" ){acpt--;}
		x[g] = Points[pos + acpt].wx(this, true);
		y[g] = Points[pos + acpt].wy(this, true);
		//if(g==0 && Points[pos].type=="s" ){acpt++;}
		acpt++;
	}

	if (bspline){
		a[3] = (-x[0] + 3 * x[1] - 3 * x[2] + x[3]) / 6.0;
		a[2] = (3 * x[0] - 6 * x[1] + 3 * x[2]) / 6.0;
		a[1] = (-3 * x[0] + 3 * x[2]) / 6.0;
		a[0] = (x[0] + 4 * x[1] + x[2]) / 6.0;
		b[3] = (-y[0] + 3 * y[1] - 3 * y[2] + y[3]) / 6.0;
		b[2] = (3 * y[0] - 6 * y[1] + 3 * y[2]) / 6.0;
		b[1] = (-3 * y[0] + 3 * y[2]) / 6.0;
		b[0] = (y[0] + 4 * y[1] + y[2]) / 6.0;
	}
	else{
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
		p_x = a[0] + t*(a[1] + t*(a[2] + t*a[3]));
		p_y = b[0] + t*(b[1] + t*(b[2] + t*b[3]));
		table->push_back(D3DXVECTOR2(p_x, p_y));
		//if(bspline && t==0){Visuals::DrawRect((*table)[table->size()-1]);}
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
		VideoCtrl *vc = tab->Video;
		Fullscreen * fullScreen = vc->GetFullScreenWindow();
		if (vc->IsFullScreen() && fullScreen){
			fullScreen->vToolbar->SetItemToggled(&tool);
		}
		else{
			vc->GetVideoToolbar()->SetItemToggled(&tool);
		}
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
			tab->Video->Render(false);
			
		}
		else if (pos == -1 && !tab->Video->HasArrow()){
			if (lastpos >= 0 && lastpos < (int)psize){
				lastpos = -1;
				tab->Video->Render(false);
			}
		}
		if (tool >= 1 && tool <= 3 && pos == -1 && !event.Leaving()){
			if (psize < 1){ return; }
			drawToolLines = true;
			tab->Video->Render(false);
		}
		else if (drawToolLines){
			drawToolLines = false;
			tab->Video->Render(false);
		}
		if (!drawSelection){
			if (tool == 0 || pos != -1 || tool > 3){ drawCross = true; tab->Video->Render(false); }
		}
		if (event.Leaving() && drawCross){
			drawCross = false;
			tab->Video->Render(false);
		}
	}



	if (event.LeftUp() || event.RightUp() || event.MiddleUp()){
		if (!drawSelection){
			SetClip(GetVisual(), false);
		}
		else{
			drawSelection = false;
			tab->Video->Render(false);
		}
		if (tab->Video->HasCapture()){
			tab->Video->ReleaseMouse();
		}
		return;
	}
	if (event.ButtonUp()){
		return;
	}

	if (event.MiddleDown() || (tool == 5 && click)){
		grabbed = -1;
		size_t i = (psize > 1) ? 1 : 0;
		for (size_t i = 0; i < psize; i++)
		{
			float pointx = Points[i].wx(this), pointy = Points[i].wy(this);
			if (abs(pointx - zx) < pointArea && abs(pointy - zy) < pointArea)
			{
				int j = i;
				int er = 1;
				bool isM = (Points[i].type == L"m");
				if (isM){
					Points.erase(Points.begin() + i, Points.begin() + i + 1);
					if (i >= Points.size()){
						SetClip(GetVisual(), true);
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
				SetClip(GetVisual(), true);
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
				tab->Video->CaptureMouse();
				//snapYminus = false; snapYplus = false; snapXminus = false; snapXplus = false;
				//KaiLog(wxString::Format(L"left down on point %i %i", grabbed, (int)drawSelection));
				firstmove = D3DXVECTOR2(zx, zy);
				break;
			}
		}

		if (tool >= 1 && tool <= 3 && (grabbed == -1 || right))
		{
			//KaiLog(wxString::Format(L"tool apply %i", tool));
			if (Points.empty()){ AddMove(xy, 0); SetClip(GetVisual(), true); return; }
			int pos = CheckPos(xy, true);
			switch (tool){
			case 1: AddLine(xy, pos); break;
			case 2: AddCurve(xy, pos); break;
			case 3:
				if (Points[(pos == (int)psize) ? psize - 1 : pos].type == L"s"){
					AddCurvePoint(xy, pos); break;//bspline point
				}
				AddCurve(xy, pos, L"s"); break;//bspline
			default:
				KaiLog(wxString::Format(L"Bad tool %i", tool));
			}
			SetClip(GetVisual(), true);
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
			SetClip(GetVisual(), true);
			//grabbed= psize-1;
			tool = 1;
			tab->Video->GetVideoToolbar()->SetItemToggled(&tool);
		}
		else if (grabbed == -1){
			tab->Video->CaptureMouse();
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
		Points[grabbed].x = ((zx + diffs.x)*coeffW) - _x;
		Points[grabbed].y = ((zy + diffs.y)*coeffH) - _y;
		//new feature snapping to other points
		//add some method to turn it on
		if (event.AltDown())
			Points[grabbed] = FindSnapPoint(Points[grabbed], grabbed);

		if (!Points[grabbed].isSelected){ Points[grabbed].isSelected = true; }

		if (event.ShiftDown()){
			
			int diffx = abs(firstmove.x - x);
			int diffy = abs(firstmove.y - y);
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
		SetClip(GetVisual(), true);
		lastpos = -1;
	}
	if (drawSelection){
		selection.width = x;
		selection.height = y;
		SelectPoints();
		tab->Video->Render(false);
	}


}



D3DXVECTOR2 DrawingAndClip::CalcWH()
{
	if (alignment == 7 || Points.size() < 1){ return D3DXVECTOR2(0, 0); }
	float offx = 0, offy = 0;
	//it's probably unused cause calculations is wrong
	//pixel size in place point size and no float calculation
	if (textwithclip != L""){
		Styles *textstyle = tab->Grid->GetStyle(0, tab->Edit->line->Style);
		wxFont stylefont(textstyle->GetFontSizeDouble(), wxFONTFAMILY_SWISS, 
			(textstyle->Italic) ? wxFONTSTYLE_ITALIC : wxFONTSTYLE_NORMAL,
			(textstyle->Bold) ? wxFONTWEIGHT_BOLD : wxFONTWEIGHT_NORMAL, 
			textstyle->Underline, textstyle->Fontname);//, textstyle->Encoding
		int ex = 0, ey = 0, eb = 0, et = 0;
		wxClientDC dc(tab);
		dc.GetTextExtent(textwithclip, &ex, &ey, &eb, &et, &stylefont);
		offx = ex / coeffW;
		offy = eb / coeffH;
	}
	float minx = FLT_MAX;
	float miny = FLT_MAX;
	float maxx = -FLT_MAX;
	float maxy = -FLT_MAX;
	for (size_t i = 0; i < Points.size(); i++)
	{
		ClipPoint p = Points[i];
		if (p.x < minx){ minx = p.x; }
		if (p.y < miny){ miny = p.y; }
		if (p.x > maxx){ maxx = p.x; }
		if (p.y > maxy){ maxy = p.y; }
	}
	D3DXVECTOR2 sizes((maxx - minx) + offx, (maxy - miny) + offy);
	D3DXVECTOR2 result = D3DXVECTOR2(0, 0);
	if (alignment % 3 == 2){
		result.x = sizes.x / 2.0;
	}
	else if (alignment % 3 == 0){
		result.x = sizes.x;
	}
	if (alignment < 4){
		result.y = sizes.y;
	}
	else if (alignment < 7){
		result.y = sizes.y / 2.0;
	}
	return result;
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

	if (evt.ControlDown() && evt.GetKeyCode() == L'A'){
		ChangeSelection(true);
		tab->Video->Render(false);
	}
	//if strzałki then

}