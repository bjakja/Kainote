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
			if (hasOriginalRectangle) {
				D3DXVECTOR2 opoint1 = ScaleToVideo(sizingOriginalRectangle[0]);
				D3DXVECTOR2 opoint2 = ScaleToVideo(sizingOriginalRectangle[1]);
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

void Scale::GetVisual(wxString *visual)
{
	if (type != 1)
		*visual += L"\\fscx" + getfloat(scale.x * 100);
	if (type != 0)
		*visual += L"\\fscy" + getfloat(scale.y * 100);
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

				SortPoints();
			}
			if (rectangleVisible) {
				SetScale();
				SetVisual(false, type);
			}

			if (!tab->Video->HasArrow()) { tab->Video->SetCursor(wxCURSOR_ARROW); }
		}

		if (!holding && rectangleVisible) {

			bool setarrow = false;
			int test = HitTest(D3DXVECTOR2(x, y), false);
			if (test < INSIDE) {
				setarrow = true;
				tab->Video->SetCursor((test < 4) ? wxCURSOR_SIZEWE :
					(test >= 4 && test % 4 == 0) ? wxCURSOR_SIZENS :
					(test == (TOP + LEFT) || test == (BOTTOM + RIGHT)) ? wxCURSOR_SIZENWSE : wxCURSOR_SIZENESW);
			}
			if (!setarrow) { tab->Video->SetCursor(wxCURSOR_ARROW); }
		}
		if (click) {
			if (!tab->Video->HasCapture()) { tab->Video->CaptureMouse(); }
			grabbed = OUTSIDE;
			int pointx = ((x / zoomScale.x) + zoomMove.x) * coeffW,
				pointy = ((y / zoomScale.y) + zoomMove.y) * coeffH;
			if (rectangleVisible) {
				grabbed = HitTest(D3DXVECTOR2(x, y));
				if (grabbed == INSIDE) {
					if (sizingRectangle[0].x <= pointx && sizingRectangle[1].x >= pointx &&
						sizingRectangle[0].y <= pointy && sizingRectangle[1].y >= pointy) {
						diffs.x = x;
						diffs.y = y;
					}
				}
			}
			if (!rectangleVisible || grabbed == OUTSIDE) {
				sizingRectangle[0].x = sizingRectangle[1].x = pointx;
				sizingRectangle[0].y = sizingRectangle[1].y = pointy;
				grabbed = OUTSIDE;
				rectangleVisible = true;
			}

		}
		else if (holding && grabbed != -1) {

			if (grabbed < INSIDE) {
				if (grabbed & LEFT || grabbed & RIGHT) {
					x = MID(VideoSize.x, x, VideoSize.width);
					sizingRectangle[(grabbed & RIGHT) ? 1 : 0].x = 
						((((x /*+ diffs.x*/) / zoomScale.x) + zoomMove.x) * coeffW);
					if (grabbed & LEFT && sizingRectangle[0].x > sizingRectangle[1].x) { 
						sizingRectangle[0].x = sizingRectangle[1].x;
					}
					if (grabbed & RIGHT && sizingRectangle[1].x < sizingRectangle[0].x) { 
						sizingRectangle[1].x = sizingRectangle[0].x; 
					}
				}
				if (grabbed & TOP || grabbed & BOTTOM) {
					y = MID(VideoSize.y, y, VideoSize.height);
					sizingRectangle[(grabbed & BOTTOM) ? 1 : 0].y = 
						((((y/* + diffs.y*/) / zoomScale.y) + zoomMove.y) * coeffH);
					if (grabbed & TOP && sizingRectangle[0].y > sizingRectangle[1].y) { 
						sizingRectangle[0].y = sizingRectangle[1].y; 
					}
					if (grabbed & BOTTOM && sizingRectangle[1].y < sizingRectangle[0].y) { 
						sizingRectangle[1].y = sizingRectangle[0].y;
					}
				}
			}
			else if (grabbed == INSIDE) {
				float movex = (((x - diffs.x) / zoomScale.x) * coeffW),
					movey = (((y - diffs.y) / zoomScale.y) * coeffH);
				sizingRectangle[0].x += movex;
				sizingRectangle[0].y += movey;
				sizingRectangle[1].x += movex;
				sizingRectangle[1].y += movey;
				diffs.x = x;
				diffs.y = y;
			}
			else if (grabbed == OUTSIDE) {
				int pointx = ((x / zoomScale.x) + zoomMove.x) * coeffW,
					pointy = ((y / zoomScale.y) + zoomMove.y) * coeffH;
				sizingRectangle[1].x = pointx;
				sizingRectangle[1].y = pointy;
			}
			SortPoints();
			SetScale();
			SetVisual(true, type);
		}

		return;
	}

	if (evt.ButtonUp()){
		if (tab->Video->HasCapture()){ tab->Video->ReleaseMouse(); }
		SetVisual(false, type);
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

		SetVisual(true, type);
	}
}

void Scale::SetCurVisual()
{
	D3DXVECTOR2 linepos = GetPosnScale(&scale, &AN, moveValues);
	originalScale = scale;

	if (moveValues[6] > 3){ linepos = CalcMovePos(); }
	from = D3DXVECTOR2(((linepos.x / coeffW) - zoomMove.x) * zoomScale.x,
		((linepos.y / coeffH) - zoomMove.y) * zoomScale.y);

	arrowLengths.y = (linepos.y > SubsSize.y / 2) ? -100.f : 100.f, 
		arrowLengths.x = (linepos.x > SubsSize.x / 2) ? -100.f : 100.f;

	to.x = from.x + (scale.x * arrowLengths.x);
	to.y = from.y + (scale.y * arrowLengths.y);

}

void Scale::ChangeTool(int _tool)
{
	if (!hasScaleToRenctangle && _tool & 1) {
		originalSize = GetTextSize(tab->Edit->line, &border);
	}
	hasScaleToRenctangle = _tool & 1;
	hasScaleX = _tool & 2;
	preserveAspectRatio = _tool & 4;
	hasScaleY = _tool & 8;
	hasOriginalRectangle = _tool & 16;
	
	tab->Video->Render(false);
}

void Scale::ChangeVisual(wxString *txt, Dialogue *dial)
{
	wxString tag;

	if (type != 1){
		tag = L"\\fscx" + getfloat(scale.x * 100);

		FindTag(L"fscx([0-9.-]+)", *txt, 0, 1);
		Replace(tag, txt);
	}
	if (type != 0){
		tag = L"\\fscy" + getfloat(scale.y * 100);

		FindTag(L"fscy([0-9.-]+)", *txt, 1);
		Replace(tag, txt);
	}


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
		
		SetVisual(true, type);
		SetVisual(false, type);
		return;
	}
	evt.Skip();
}

int Scale::HitTest(const D3DXVECTOR2 &pos, bool originalRect, bool diff)
{
	int resultX = 0, resultY = 0, resultInside = 0, resultFinal = 0, oldpointx = 0, oldpointy = 0;
	D3DXVECTOR2* points = originalRect ? sizingOriginalRectangle : sizingRectangle;
	for (int i = 0; i < 2; i++) {
		float pointx = ((points[i].x / coeffW) - zoomMove.x) * zoomScale.x,
			pointy = ((points[i].y / coeffH) - zoomMove.y) * zoomScale.y;
		bool hasResult = false;
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
		float tmpy = sizingRectangle[0].x;
		sizingRectangle[0].x = sizingRectangle[1].x;
		sizingRectangle[1].x = tmpy;
	}
	/*if (sizingOriginalRectangle[1].y < sizingOriginalRectangle[0].y) {
		float tmpy = sizingOriginalRectangle[0].y;
		sizingOriginalRectangle[0].y = sizingOriginalRectangle[1].y;
		sizingOriginalRectangle[1].y = tmpy;
	}
	if (sizingOriginalRectangle[1].x < sizingOriginalRectangle[0].x) {
		float tmpy = sizingOriginalRectangle[0].x;
		sizingOriginalRectangle[0].x = sizingOriginalRectangle[1].x;
		sizingOriginalRectangle[1].x = tmpy;
	}*/
}

void Scale::SetScale()
{
	if (originalRectangleVisible) {
		scale.x = originalScale.x * ((sizingRectangle[1].x - sizingRectangle[0].x) /
			(sizingOriginalRectangle[1].x - sizingOriginalRectangle[0].x));
		if (preserveAspectRatio)
			scale.y = scale.x;
		else
			scale.y = originalScale.y * ((sizingRectangle[1].y - sizingRectangle[0].y) /
				(sizingOriginalRectangle[1].y - sizingOriginalRectangle[0].y));
	}
	else {
		scale.x = originalScale.x * ((sizingRectangle[1].x - sizingRectangle[0].x - border.x) / originalSize.x);
		if (preserveAspectRatio)
			scale.y = scale.x;
		else
			scale.y = originalScale.y * ((sizingRectangle[1].y - sizingRectangle[0].y - border.y) / originalSize.y);
	}
}

D3DXVECTOR2 Scale::ScaleToVideo(D3DXVECTOR2 point)
{
	float pointx = ((point.x / coeffW) - zoomMove.x) * zoomScale.x,
		pointy = ((point.y / coeffH) - zoomMove.y) * zoomScale.y;
	return D3DXVECTOR2(pointx, pointy);
}
