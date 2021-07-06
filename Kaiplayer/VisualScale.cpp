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

enum {
	LEFT = 1,
	RIGHT,
	TOP = 4,
	BOTTOM = 8,
	INSIDE = 16,
	OUTSIDE = 32
};

Scale::Scale()
	: Visuals()
	, type(0)
	, grabbed(-1)
{
}

void Scale::DrawVisual(int time)
{
	if (hasScaleToRenctangle) {
		if (rectangleVisible) {
			D3DXVECTOR2 point1 = ScaleToVideo(sizingRectangle[0]);
			D3DXVECTOR2 point2 = ScaleToVideo(sizingRectangle[1]);
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
		if (originalRectangleVisible) {
			D3DXVECTOR2 opoint1 = ScaleToVideo(sizingRectangle[2]);
			D3DXVECTOR2 opoint2 = ScaleToVideo(sizingRectangle[3]);
			D3DXVECTOR2 v4[5];
			v4[0] = opoint1;
			v4[1].x = opoint2.x;
			v4[1].y = opoint1.y;
			v4[2] = opoint2;
			v4[3].x = opoint1.x;
			v4[3].y = opoint2.y;
			v4[4] = opoint1;
			line->Begin();
			line->Draw(v4, 5, 0xFF0000BB);
			line->End();
		}
		return;
	}

	if (time != oldtime && moveValues[6] > 3){
		from = CalcMovePos();
		from.x = ((from.x / coeffW) - zoomMove.x) * zoomScale.x;
		from.y = ((from.y / coeffH) - zoomMove.y) * zoomScale.y;
		to.x = from.x + (scale.x * arrowLengths.x);
		to.y = from.y + (scale.y * arrowLengths.y);
	}

	D3DXVECTOR2 v4[15];
	
	v4[0] = from;//horizontal arrow
	v4[1].x = to.x;
	v4[1].y = from.y;//horizontal arrow
	v4[2] = from;//skew arrow
	v4[3].x = to.x;
	v4[3].y = to.y;//skew arrow
	v4[4] = from;//vertical arrow
	v4[5].x = from.x;
	v4[5].y = to.y;//vertical arrow

	for (int i = 1; i < 6; i += 2){
		DrawArrow(v4[0], &v4[i]);
	}

	line->Begin();
	line->Draw(v4, 2, 0xFFBB0000);
	line->End();
	line->Begin();
	line->Draw(&v4[2], 2, 0xFFBB0000);
	line->End();
	line->Begin();
	line->Draw(&v4[4], 2, 0xFFBB0000);
	line->End();

}

void Scale::OnMouseEvent(wxMouseEvent &evt)
{
	if (blockevents){ return; }
	bool click = evt.LeftDown() || evt.RightDown() || evt.MiddleDown();
	bool holding = (evt.LeftIsDown() || evt.RightIsDown() || evt.MiddleIsDown());
	bool leftc = evt.LeftDown();
	bool rightc = evt.RightDown();
	bool middlec = evt.MiddleDown();

	int x, y;
	evt.GetPosition(&x, &y);

	if (hasScaleToRenctangle) {
		type = ((hasScaleX && hasScaleY) || preserveAspectRatio) ? 2 : hasScaleY ? 1 : 0;
		if (evt.ButtonUp()) {
			if (tab->Video->HasCapture()) { tab->Video->ReleaseMouse(); }
			if (rectangleVisible) {
				if (sizingRectangle[1].y == sizingRectangle[0].y || 
					sizingRectangle[1].x == sizingRectangle[0].x)
					rectangleVisible = false;
				if (originalRectangleVisible) {
					if (sizingRectangle[3].y == sizingRectangle[2].y ||
						sizingRectangle[3].x == sizingRectangle[2].x)
						originalRectangleVisible = false;
				}

				SortPoints();
			}
			if (rectangleVisible) {
				SetScale();
				SetVisual(false);
			}

			if (!tab->Video->HasArrow()) { tab->Video->SetCursor(wxCURSOR_ARROW); }
		}

		if (!holding && (rectangleVisible || originalRectangleVisible)) {

			bool setarrow = false;
			int test = HitTest(D3DXVECTOR2(x, y));
			if (test < INSIDE) {
				setarrow = true;
				tab->Video->SetCursor((test < 4) ? wxCURSOR_SIZEWE :
					(test >= 4 && test % 4 == 0) ? wxCURSOR_SIZENS :
					(test == (TOP + LEFT) || test == (BOTTOM + RIGHT)) ? wxCURSOR_SIZENWSE : wxCURSOR_SIZENESW);
			}
			if (originalRectangleVisible) {
				int test = HitTest(D3DXVECTOR2(x, y), true);
				if (test < INSIDE) {
					setarrow = true;
					tab->Video->SetCursor((test < 4) ? wxCURSOR_SIZEWE :
						(test >= 4 && test % 4 == 0) ? wxCURSOR_SIZENS :
						(test == (TOP + LEFT) || test == (BOTTOM + RIGHT)) ? wxCURSOR_SIZENWSE : wxCURSOR_SIZENESW);
				}
			}
			if (!setarrow) { tab->Video->SetCursor(wxCURSOR_ARROW); }
		}
		if (click) {
			if (!tab->Video->HasCapture()) { tab->Video->CaptureMouse(); }
			rightHolding = evt.RightDown() && hasOriginalRectangle;
			grabbed = OUTSIDE;
			float pointx = ((x / zoomScale.x) + zoomMove.x) * coeffW,
				pointy = ((y / zoomScale.y) + zoomMove.y) * coeffH;
			if (rectangleVisible) {
				grabbed = HitTest(D3DXVECTOR2(x, y), false, true);
				if (grabbed == INSIDE) {
					if (sizingRectangle[0].x <= pointx && 
						sizingRectangle[1].x >= pointx &&
						sizingRectangle[0].y <= pointy && 
						sizingRectangle[1].y >= pointy) {
						diffs.x = x;
						diffs.y = y;
					}
				}
				else if (grabbed < INSIDE) {
					rightHolding = false;
				}
			}
			if (originalRectangleVisible) {
				int grabbed1 = HitTest(D3DXVECTOR2(x, y), true, true);
				if (grabbed1 == INSIDE) {
					if (sizingRectangle[2].x <= pointx &&
						sizingRectangle[3].x >= pointx &&
						sizingRectangle[2].y <= pointy &&
						sizingRectangle[3].y >= pointy) {
						diffs.x = x;
						diffs.y = y;
					}
					grabbed = grabbed1;
				}
				else if (grabbed1 < INSIDE) {
					grabbed = grabbed1;
					rightHolding = true;
				}
			}
			int tablediff = rightHolding ? 2 : 0;
			bool visible = rightHolding ? originalRectangleVisible : rectangleVisible;
			if (!visible || grabbed == OUTSIDE) {
				sizingRectangle[0 + tablediff].x = sizingRectangle[1 + tablediff].x = pointx;
				sizingRectangle[0 + tablediff].y = sizingRectangle[1 + tablediff].y = pointy;
				grabbed = OUTSIDE;
				if (rightHolding)
					originalRectangleVisible = true;
				else
					rectangleVisible = true;
			}
			lastScale = scale;
		}
		else if (holding && grabbed != -1) {
			int tablediff = rightHolding ? 2 : 0;
			if (grabbed < INSIDE) {
				if (grabbed & LEFT || grabbed & RIGHT) {
					x = MID(VideoSize.x, x, VideoSize.width);
					int posInTable = (grabbed & RIGHT) ? 1 : 0;
					sizingRectangle[posInTable + tablediff].x =
						((((x + diffs.x) / zoomScale.x) + zoomMove.x) * coeffW);
					if (grabbed & LEFT && sizingRectangle[0 + tablediff].x > sizingRectangle[1 + tablediff].x) {
						sizingRectangle[0 + tablediff].x = sizingRectangle[1 + tablediff].x;
					}
					if (grabbed & RIGHT && sizingRectangle[1 + tablediff].x < sizingRectangle[0 + tablediff].x) {
						sizingRectangle[1 + tablediff].x = sizingRectangle[0 + tablediff].x;
					}
				}
				if (grabbed & TOP || grabbed & BOTTOM) {
					y = MID(VideoSize.y, y, VideoSize.height);
					int posInTable = (grabbed & BOTTOM) ? 1 : 0;
					sizingRectangle[posInTable + tablediff].y =
						((((y + diffs.y) / zoomScale.y) + zoomMove.y) * coeffH);
					if (grabbed & TOP && sizingRectangle[0 + tablediff].y > sizingRectangle[1 + tablediff].y) {
						sizingRectangle[0 + tablediff].y = sizingRectangle[1 + tablediff].y;
					}
					if (grabbed & BOTTOM && sizingRectangle[1 + tablediff].y < sizingRectangle[0 + tablediff].y) {
						sizingRectangle[1 + tablediff].y = sizingRectangle[0 + tablediff].y;
					}
				}
			}
			else if (grabbed == INSIDE) {
				float movex = (((x - diffs.x) / zoomScale.x) * coeffW),
					movey = (((y - diffs.y) / zoomScale.y) * coeffH);
				sizingRectangle[0 + tablediff].x += movex;
				sizingRectangle[0 + tablediff].y += movey;
				sizingRectangle[1 + tablediff].x += movex;
				sizingRectangle[1 + tablediff].y += movey;
				diffs.x = x;
				diffs.y = y;
			}
			else if (grabbed == OUTSIDE) {
				float pointx = ((x / zoomScale.x) + zoomMove.x) * coeffW,
					pointy = ((y / zoomScale.y) + zoomMove.y) * coeffH;
				sizingRectangle[1 + tablediff].x = pointx;
				sizingRectangle[1 + tablediff].y = pointy;
			}
			//SortPoints();
			SetScale();
			if (rectangleVisible)
				SetVisual(true);
			else
				tab->Video->Render(false);
		}

		return;
	}

	if (evt.ButtonUp()){
		if (tab->Video->HasCapture()){ tab->Video->ReleaseMouse(); }
		SetVisual(false);
		if (!tab->Video->HasArrow()){ tab->Video->SetCursor(wxCURSOR_ARROW); }
		wasUsedShift = false;
	}

	if (!holding){
		if (abs(to.x - x) < 11 && abs(to.y - y) < 11){ 
			if (tab->Video->HasArrow()){ tab->Video->SetCursor(wxCURSOR_SIZING); } 
		}
		else if (abs(to.x - x) < 11 && abs(from.y - y) < 11){ 
			if (tab->Video->HasArrow()){ tab->Video->SetCursor(wxCURSOR_SIZEWE); } 
		}
		else if (abs(to.y - y) < 11 && abs(from.x - x) < 11){ 
			if (tab->Video->HasArrow()){ tab->Video->SetCursor(wxCURSOR_SIZENS); } 
		}
		else if (!tab->Video->HasArrow()){ tab->Video->SetCursor(wxCURSOR_ARROW); }
	}
	if (click || wasUsedShift != evt.ShiftDown()){
		if (leftc){ type = 0; }
		if (rightc){ type = 1; }
		if (middlec || (leftc && evt.ShiftDown())){ type = 2; }
		if (abs(to.x - x) < 11 && abs(from.y - y) < 11){ grabbed = 0; type = 0; }
		else if (abs(to.y - y) < 11 && abs(from.x - x) < 11){ grabbed = 1; type = 1; }
		else if (abs(to.x - x) < 11 && abs(to.y - y) < 11){ grabbed = 2; type = 2; }
		diffs.x = to.x - x;
		diffs.y = to.y - y;
		if (type == 0) { tab->Video->SetCursor(wxCURSOR_SIZEWE); }
		if (type == 1) { tab->Video->SetCursor(wxCURSOR_SIZENS); }
		if (type == 2) { tab->Video->SetCursor(wxCURSOR_SIZING); }
		if ((leftc || evt.LeftIsDown()) && evt.ShiftDown()){
			tab->Video->SetCursor(wxCURSOR_SIZING);
			type = 2;
			diffs.x = x;
			diffs.y = y;
			wasUsedShift = evt.ShiftDown();
			return;
		}
		
		if (grabbed == -1){

			diffs.x = (from.x - x) + (arrowLengths.x * scale.x);
			diffs.y = (from.y - y) + (arrowLengths.y * scale.y);
		}
		wasUsedShift = evt.ShiftDown();
		lastScale = scale;
	}
	else if (holding){
		if (evt.ShiftDown()){
			int diffx = abs(x - diffs.x);
			int diffy = abs(diffs.y - y);
			int move = (diffx > diffy) ? x - diffs.x : diffs.y - y;

			D3DXVECTOR2 copyto = to;
			D3DXVECTOR2 copydiffs = diffs;
			bool normalArrowX = arrowLengths.x > 0;
			bool normalArrowY = arrowLengths.y > 0;
			
			//left top & right bottom with move arrow in x axis
			if ((!normalArrowX && !normalArrowY || normalArrowX && normalArrowY) && diffy < diffx) {
				to.y = to.y + move;
				to.x = to.x + move;
			}//left bottom & right top
			else if ((normalArrowX && !normalArrowY) || !normalArrowX && normalArrowY) {
				to.y = to.y - move;
				to.x = to.x + move;
			}//left top & right bottom with move arrow in y axis
			else {
				to.y = to.y - move;
				to.x = to.x - move;
			}
			diffs.x = x;
			diffs.y = y;
			if ((normalArrowX && (to.x - from.x) < 1 ) || (!normalArrowX && (to.x - from.x) > -1)){
				diffs = copydiffs;
				to = copyto;
			}
		}
		else{
			if (type != 1){
				to.x = x + diffs.x;
			}
			if (type != 0){
				to.y = y + diffs.y;
			}
		}
		scale.x = abs((to.x - from.x) / arrowLengths.x);
		scale.y = abs((to.y - from.y) / arrowLengths.y);

		SetVisual(true);
	}
}

void Scale::SetCurVisual()
{
	D3DXVECTOR2 linepos = GetPosnScale(&scale, &AN, moveValues);
	originalScale = lastScale = scale;

	if (moveValues[6] > 3){ linepos = CalcMovePos(); }
	from = D3DXVECTOR2(((linepos.x / coeffW) - zoomMove.x) * zoomScale.x,
		((linepos.y / coeffH) - zoomMove.y) * zoomScale.y);

	arrowLengths.y = (linepos.y > SubsSize.y / 2) ? -100.f : 100.f, 
		arrowLengths.x = (linepos.x > SubsSize.x / 2) ? -100.f : 100.f;

	to.x = from.x + (scale.x * arrowLengths.x);
	to.y = from.y + (scale.y * arrowLengths.y);

	if (hasScaleToRenctangle) {
		originalSize = GetTextSize(tab->Edit->line, &border);
	}
}

void Scale::ChangeTool(int _tool, bool blockSetCurVisual)
{
	bool oldHasScaleToRenctangle = hasScaleToRenctangle;
	hasScaleToRenctangle = _tool & 1;
	hasScaleX = _tool & 2;
	preserveAspectRatio = _tool & 4;
	hasScaleY = _tool & 8;
	bool oldHasOriginalRectangle = hasOriginalRectangle;
	hasOriginalRectangle = _tool & 16;
	bool oldChangeAllTags = changeAllTags;
	changeAllTags = _tool & 32;
	preserveProportions = _tool & 64;
	replaceTagsInCursorPosition = !changeAllTags;
	if (oldChangeAllTags != changeAllTags && !blockSetCurVisual) {
		SetCurVisual();
	}
	else if (oldHasScaleToRenctangle != hasScaleToRenctangle) {
		originalSize = GetTextSize(tab->Edit->line, &border, NULL, false);
	}
	if (oldHasOriginalRectangle != hasOriginalRectangle) {
		if (hasOriginalRectangle) {
			if (oldChangeAllTags == changeAllTags && !blockSetCurVisual)
				SetCurVisual();
			SetSecondRectScale();
		}
		else
			originalRectangleVisible = false;
	}
	tab->Video->Render(false);
}

void Scale::ChangeVisual(wxString *txt, Dialogue *dial)
{
	wxString tag;

	float Scalex = scale.x / lastScale.x;
	float Scaley = scale.y / lastScale.y;
	if (Scalex > Scaley)
		Scaley = Scalex;
	else if (Scaley > Scalex)
		Scalex = Scaley;
	//Change positions of scaled lines 
	//that the image before scaling don't change
	//it distorts when positions are different then first one
	if (changeAllTags && preserveProportions) {
		bool putInBracket = false;
		wxPoint textPos;
		D3DXVECTOR2 pos = GetPosition(dial, &putInBracket, &textPos);
		//need to restore subtitles position
		D3DXVECTOR2 activeLinePos = { ((from.x / zoomScale.x) + zoomMove.x) * coeffW,
		((from.y / zoomScale.y) + zoomMove.y) * coeffH };
		pos.x = activeLinePos.x + ((pos.x - activeLinePos.x) * Scalex);
		pos.y = activeLinePos.y + ((pos.y - activeLinePos.y) * Scaley);
		wxString posstr = L"\\pos(" + getfloat(pos.x) + "," + getfloat(pos.y) + ")";
		if (moveValues[6] > 2) {
			D3DXVECTOR2 pos1(moveValues[2] - moveValues[0], moveValues[3] - moveValues[1]);
			int startTime = ZEROIT(tab->Edit->line->Start.mstime);
			posstr = L"\\move(" + getfloat(pos.x) + L"," + getfloat(pos.y) + L"," +
				getfloat(pos.x + pos1.x) + L"," + getfloat(pos.y + pos1.y) + L"," +
				getfloat(moveValues[4] - startTime, L"6.0f") + L"," +
				getfloat(moveValues[5] - startTime, L"6.0f") + L")";
		}
		//position returns length instead position of end and needs 
		//different function ChangeTag not works till I change position of end
		//maybe everything switch to replace function using length is better
		if (putInBracket) { posstr = L"{" + posstr + L"}"; }
		txt->replace(textPos.x, textPos.y, posstr);
		ChangeClipScale(txt, activeLinePos, Scalex, Scaley);
		type = 2;
	}
	Styles* style = NULL;
	if (changeAllTags) {
		style = tab->Grid->GetStyle(0, dial->Style);
	}
	if (type != 1){
		//change all tags fscx that are in line and add first one
		//when there is no tags
		if (changeAllTags) {
			auto replfunc = [=](const FindData& data, wxString* result) {
				float scalex = 1.f * Scalex;
				if (!data.finding.empty()) {
					scalex = (wxAtof(data.finding) / 100.f) * Scalex;
				}
				else if (style) {
					scalex = (style->GetScaleXDouble() / 100.f) * Scalex;
				}
				*result = getfloat(scalex * 100);
			};
			ReplaceAll(L"fscx([0-9.-]+)", L"fscx", txt, replfunc, true);
		}
		else {
			tag = L"\\fscx" + getfloat(scale.x * 100);

			FindTag(L"fscx([0-9.-]+)", *txt, 1);
			Replace(tag, txt);
		}
	}
	if (type != 0){
		//change all tags fscx that are in line and add first one
		//when there is no tags
		if (changeAllTags) {
			auto replfunc = [=](const FindData& data, wxString* result) {
				float scaley = 1.f * Scalex;
				if (!data.finding.empty()) {
					scaley = (wxAtof(data.finding) / 100.f) * Scaley;
				}
				else if(style){
					scaley = (style->GetScaleYDouble() / 100.f) * Scaley;
				}
				*result = getfloat(scaley * 100);
			};
			ReplaceAll(L"fscy([0-9.-]+)", L"fscy", txt, replfunc, true);
		}
		else {
			tag = L"\\fscy" + getfloat(scale.y * 100);

			FindTag(L"fscy([0-9.-]+)", *txt, 1);
			Replace(tag, txt);
		}
	}


}

wxPoint Scale::ChangeVisual(wxString* txt)
{
	wxString tag;
	if (type != 1) {
		//change all tags fscx that are in line and add first one
		//when there is no tags
		if (changeAllTags) {
			auto replfunc = [=](const FindData& data, wxString* result) {
				float scalex = scale.x;
				if (!data.finding.empty()) {
					scalex = (wxAtof(data.finding) / 100.f) * (scale.x / lastScale.x);
				}
				*result = getfloat(scalex * 100);
			};
			ReplaceAll(L"fscx([0-9.-]+)", L"fscx", txt, replfunc, true);
		}
		else {
			tag = L"\\fscx" + getfloat(scale.x * 100);

			FindTag(L"fscx([0-9.-]+)", *txt);
			Replace(tag, txt);
		}
	}
	if (type != 0) {
		//change all tags fscx that are in line and add first one
		//when there is no tags
		if (changeAllTags) {
			auto replfunc = [=](const FindData& data, wxString* result) {
				float scaley = scale.y;
				if (!data.finding.empty()) {
					scaley = (wxAtof(data.finding) / 100.f) * (scale.y / lastScale.y);
				}
				*result = getfloat(scaley * 100);
			};
			ReplaceAll(L"fscy([0-9.-]+)", L"fscy", txt, replfunc, true);
		}
		else {
			tag = L"\\fscy" + getfloat(scale.y * 100);

			FindTag(L"fscy([0-9.-]+)", *txt);
			Replace(tag, txt);
		}
	}
	return GetPositionInText();
}

void Scale::OnKeyPress(wxKeyEvent &evt)
{
	int key = evt.GetKeyCode();
	bool left = key == L'A';
	bool right = key == L'D';
	bool up = key == L'W';
	bool down = key == L'S';
	
	if ((left || right || up || down) && evt.GetModifiers() != wxMOD_ALT){

		float unitx = abs(arrowLengths.x / 100);
		float unity = abs(arrowLengths.y / 100);
		float directionX = (left) ? -unitx : (right) ? unitx : 0;
		float directionY = (up) ? -unity : (down) ? unity : 0;
		type = (directionX) ? 0 : 1;
		if (evt.ShiftDown()){
			
			directionX /= 10.f;
			directionY /= 10.f;
		}
		
		to.x += directionX;
		to.y += directionY;
		scale.x = abs((to.x - from.x) / arrowLengths.x);
		scale.y = abs((to.y - from.y) / arrowLengths.y);
		
		SetVisual(true);
		SetVisual(false);
		return;
	}
	evt.Skip();
}

int Scale::HitTest(const D3DXVECTOR2 &pos, bool originalRect, bool diff)
{
	int resultX = 0, resultY = 0, resultInside = 0, resultFinal = 0, oldpointx = 0, oldpointy = 0;
	int tablediff = (originalRect) ? 2 : 0;
	for (int i = 0; i < 2; i++) {
		float pointx = ((sizingRectangle[i + tablediff].x / coeffW) - zoomMove.x) * zoomScale.x,
			pointy = ((sizingRectangle[i + tablediff].y / coeffH) - zoomMove.y) * zoomScale.y;
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

void Scale::SortPoints()
{
	if (sizingRectangle[1].y < sizingRectangle[0].y) {
		float tmpy = sizingRectangle[0].y;
		sizingRectangle[0].y = sizingRectangle[1].y;
		sizingRectangle[1].y = tmpy;
	}
	if (sizingRectangle[1].x < sizingRectangle[0].x) {
		float tmpx = sizingRectangle[0].x;
		sizingRectangle[0].x = sizingRectangle[1].x;
		sizingRectangle[1].x = tmpx;
	}
	if (originalRectangleVisible) {
		if (sizingRectangle[3].y < sizingRectangle[2].y) {
			float tmpy = sizingRectangle[2].y;
			sizingRectangle[2].y = sizingRectangle[3].y;
			sizingRectangle[3].y = tmpy;
		}
		if (sizingRectangle[3].x < sizingRectangle[2].x) {
			float tmpx = sizingRectangle[2].x;
			sizingRectangle[2].x = sizingRectangle[3].x;
			sizingRectangle[3].x = tmpx;
		}
	}
}

void Scale::SetScale()
{
	if (originalRectangleVisible) {
		scale.x = originalScale.x * ((fabs(sizingRectangle[1].x - sizingRectangle[0].x) - border.x) /
			(fabs(sizingRectangle[3].x - sizingRectangle[2].x) - border.x));
		if (preserveAspectRatio)
			scale.y = scale.x;
		else
			scale.y = originalScale.y * ((fabs(sizingRectangle[1].y - sizingRectangle[0].y) - border.y) /
				(fabs(sizingRectangle[3].y - sizingRectangle[2].y) - border.y));
	}
	else {
		scale.x = originalScale.x * ((fabs(sizingRectangle[1].x - sizingRectangle[0].x) - border.x) / originalSize.x);
		if (preserveAspectRatio)
			scale.y = scale.x;
		else
			scale.y = originalScale.y * ((fabs(sizingRectangle[1].y - sizingRectangle[0].y) - border.y) / originalSize.y);
	}
	if (scale.x < 0.f)
		scale.x = 0.f;
	if (scale.y < 0.f)
		scale.y = 0.f;
}

D3DXVECTOR2 Scale::ScaleToVideo(D3DXVECTOR2 point)
{
	float pointx = ((point.x / coeffW) - zoomMove.x) * zoomScale.x,
		pointy = ((point.y / coeffH) - zoomMove.y) * zoomScale.y;
	return D3DXVECTOR2(pointx, pointy);
}

void Scale::SetSecondRectScale()
{
	D3DXVECTOR2 activeLinePos = { ((from.x / zoomScale.x) + zoomMove.x) * coeffW,
		((from.y / zoomScale.y) + zoomMove.y) * coeffH };
	float borderx = border.x / 2;
	float bordery = border.y / 2;
	sizingRectangle[2].x = activeLinePos.x - borderx - 1;
	sizingRectangle[2].y = activeLinePos.y - bordery - 1;
	sizingRectangle[3].x = originalSize.x + activeLinePos.x + borderx - 1;
	sizingRectangle[3].y = originalSize.y + activeLinePos.y + bordery - 1;

	if (AN % 3 == 0) {
		sizingRectangle[2].x -= originalSize.x;
		sizingRectangle[3].x -= originalSize.x;
	}
	else if (AN % 3 == 2) {
		float halfsizex = originalSize.x / 2;
		sizingRectangle[2].x -= halfsizex;
		sizingRectangle[3].x -= halfsizex;
	}
	if (AN < 4) {
		sizingRectangle[2].y -= originalSize.y;
		sizingRectangle[3].y -= originalSize.y;
	}
	else if (AN < 7) {
		float halfsizey = originalSize.y / 2;
		sizingRectangle[2].y -= halfsizey;
		sizingRectangle[3].y -= halfsizey;
	}
	originalRectangleVisible = true;
}

void Scale::ChangeClipScale(wxString* txt, const D3DXVECTOR2& activeLinePos, float Scalex, float Scaley)
{
	if (FindTag(L"(i?clip[^)]+\\))", *txt, 1)) {
		wxString clip1, clip;
		GetTextResult(&clip1);
		std::vector<ClipPoint> points;
		wxString newclip = L"\\" + clip1.BeforeFirst(L'(', &clip) + L"(";
		int vectorScale = 1;
		int clipFreq = clip.Freq(L',');
		if (clipFreq >= 3) {
			wxStringTokenizer tokenzr(clip, L",", wxTOKEN_STRTOK);
			double value;
			float xy[4] = { 0,0,0,0 };
			int counter = 0;
			bool isBad = false;
			while (tokenzr.HasMoreTokens()) {
				wxString token = tokenzr.GetNextToken();
				if (token.ToDouble(&value)) {
					xy[counter] = value;
					counter++;
				}
				else
					isBad = true;
			}
			if (!isBad) {
				points.push_back(ClipPoint(xy[0], xy[1], "m", true));
				points.push_back(ClipPoint(xy[2], xy[1], "l", true));
				points.push_back(ClipPoint(xy[2], xy[3], "l", true));
				points.push_back(ClipPoint(xy[0], xy[3], "l", true));
			}
			else {
				KaiLog(_("Nie mo¿na pobraæ wartoœci clipa prostok¹tnego."));
			}
		}
		else {
			if (clipFreq >= 1) {
				wxString vscale = clip.BeforeFirst(L',', &clip);
				int vscaleint = wxAtoi(vscale);
				if (vscaleint > 0)
					vectorScale = vscaleint;
			}
			GetVectorPoints(clip, &points);
		}
		size_t psize = points.size();
		if (psize) {
			wxString format = L"5.0f";
			wxString lasttype;
			int countB = 0;
			bool spline = false;
			if (vectorScale > 1) {
				newclip << vectorScale << L",";
			}
			for (size_t i = 0; i < psize; i++)
			{
				ClipPoint pos = points[i];
				float x = activeLinePos.x + ((pos.x - activeLinePos.x) * Scalex) + 0.5f;
				float y = activeLinePos.y + ((pos.y - activeLinePos.y) * Scaley) + 0.5f;

				if (countB && !pos.start) {
					newclip << getfloat(x, format) << L" " << getfloat(y, format) << L" ";
					countB++;
				}
				else {
					if (spline) { newclip << L"c "; spline = false; }
					if (lasttype != pos.type || pos.type == L"m") {
						newclip << pos.type << L" ";
						lasttype = pos.type;
					}
					newclip << getfloat(x, format) << L" " << getfloat(y, format) << L" ";
					if (pos.type == L"b" || pos.type == L"s") {
						countB = 1;
						if (pos.type == L"s")
							spline = true;
					}
				}
				//fix for m one after another
				if (pos.type == L"m" && psize > 1 && ((i >= psize - 1) ||
					(i < psize - 1 && points[i + 1].type == L"m"))) {
					newclip << L"l " << getfloat(x, format) << L" " << getfloat(y, format) << L" ";
				}
			}
			if (spline) { newclip << L"c "; }
			newclip.Trim();
			Replace(newclip, txt);
		}
	}
}
