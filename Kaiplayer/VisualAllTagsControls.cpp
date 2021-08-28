//  Copyright (c) 2021, Marcin Drob

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

#include "VisualAllTagsControls.h"
#include "Visuals.h"
#include "TabPanel.h"


void AllTagsSlider::SetAllTags(AllTags* _parent)
{
	parent = _parent;
}

void AllTagsSlider::OnMouseEvent(wxMouseEvent& evt)
{
	float range = parent->actualTag.rangeMax - parent->actualTag.rangeMin;
	if (range <= 0) {
		KaiLog(L"Bad range");
		return;
	}
	float thumbposdiff = -parent->actualTag.rangeMin;
	float sliderRange = right - left;
	float coeff = sliderRange / range;
	float step = parent->actualTag.step * coeff;

	x = evt.GetX();
	y = evt.GetY();
	bool shift = evt.ShiftDown();

	//wheel rotation
	if (evt.GetWheelRotation() != 0) {
		if (evt.ShiftDown()) {
			int step = evt.GetWheelRotation() / evt.GetWheelDelta();
			parent->currentTag -= step;
			if (parent->currentTag < 0)
				parent->currentTag = parent->tags->size() - 1;
			else if (parent->currentTag >= parent->tags->size())
				parent->currentTag = 0;
			int tool = parent->mode << 20;
			tool += parent->currentTag;
			VideoCtrl* vc = parent->tab->Video;
			vc->GetVideoToolbar()->SetItemToggled(&tool);
			parent->ChangeTool(tool, false);
			return;
		}
		int rot = evt.GetWheelRotation() / evt.GetWheelDelta();
		size_t i = 0;
		if (parent->mode != 2)
			firstThumbValue = thumbValue;

		thumbValue = rot < 0 ? thumbValue - parent->actualTag.step : 
			thumbValue + parent->actualTag.step;
		thumbValue = MID(parent->actualTag.rangeMin, thumbValue, parent->actualTag.rangeMax);

		if (firstThumbValue != thumbValue) {
			onThumb = true;
			onSlider = false;
			//set holding before us to know what value use
			holding = true;
			if (parent->tab->Edit->IsCursorOnStart()) {
				parent->SetVisual(false);
			}
			else {
				parent->SetVisual(true);
				parent->SetVisual(false);
			}
			holding = false;
		}
		return;
	}


	float thumbpos = ((thumbValue + thumbposdiff) * coeff) + left;
	float thumbleft = thumbpos - 4;
	float thumbright = thumbpos + 4;
	float thumbtop = top - 10;
	float thumbbottom = bottom + 10;
	//leave the window
	if (evt.Leaving()) {
		if (thumbState != 0) {
			thumbState = 0;
			parent->tab->Video->Render(false);
		}
	}


	//skip unneeded positions
	if (!holding) {

		//outside slider, nothing to do
		if ((x < left - 5 || y < thumbtop || x > right + 5 || y > thumbbottom)) {
			if (thumbState != 0 || onSlider || onThumb) {
				thumbState = 0;
				onSlider = onThumb = false;
				parent->tab->Video->Render(false);
			}
		}

		onThumb = false;
		onSlider = false;
		//on thumb position
		if (x >= thumbleft && x <= thumbright && y >= thumbtop && y <= thumbbottom) {
			onThumb = true;
			if (!evt.LeftDown() && !evt.LeftDClick() && !evt.LeftUp() && thumbState != 1) {
				thumbState = 1;
				parent->tab->Video->Render(false);
			}
		}//on slider
		else {
			if (y >= top - 5 && y <= bottom + 5 && x >= left && x <= right) {
				onSlider = true;
			}
			if (!evt.LeftDown() && !evt.LeftDClick() && !evt.LeftUp()) {
				thumbState = 0;
				parent->tab->Video->Render(false);
			}
		}

	}

	if (holding) {
		//calculate new thumb value from mouse position
		thumbValue = ((x - left) / coeff) - thumbposdiff;
		thumbValue = MID(parent->actualTag.rangeMin, thumbValue, parent->actualTag.rangeMax);
		if (lastThumbValue != thumbValue) {
			if (!shift)
				parent->SetVisual(true);
		}

		lastThumbValue = thumbValue;
	}

	if (evt.LeftDown() || evt.LeftDClick()) {
		if (parent->mode != 2)
			lastThumbValue = firstThumbValue = thumbValue;
		else
			lastThumbValue = thumbValue;

		if (onThumb) {
			thumbState = 2;
			if (!parent->tab->Video->HasCapture()) {
				parent->tab->Video->CaptureMouse();
			}
			parent->tab->Video->Render(false);
			holding = true;
		}
		else if (onSlider) {
			thumbState = 1;
			thumbValue = ((x - left) / coeff) - thumbposdiff;
			thumbValue = MID(parent->actualTag.rangeMin, thumbValue, parent->actualTag.rangeMax);
			//set holding before use setVisual to know what value use
			holding = true;
			if (parent->tab->Edit->IsCursorOnStart()) {
				parent->SetVisual(false);
			}
			else {
				parent->SetVisual(true);
				parent->SetVisual(false);
			}
			holding = false;
		}
	}

	if (evt.LeftUp() && holding) {
		thumbState = 0;
		if (parent->tab->Video->HasCapture()) {
			parent->tab->Video->ReleaseMouse();
		}
		parent->SetVisual(false);
		holding = false;
	}

}

void AllTagsSlider::OnDraw()
{
	float range = parent->actualTag.rangeMax - parent->actualTag.rangeMin;
	if (range <= 0) {
		KaiLog(L"Bad range");
		return;
	}

	float sliderRange = right - left;
	float coeff = sliderRange / range;
	float step = parent->actualTag.step * coeff;
	float thumbposdiff = -parent->actualTag.rangeMin;
	float thumbtop = top - 7;
	float thumbbottom = bottom + 7;

	D3DCOLOR fill = 0xAA121150;
	VERTEX v9[9];
	CreateVERTEX(&v9[0], left, top, fill);
	CreateVERTEX(&v9[1], right, top, fill);
	CreateVERTEX(&v9[2], left, bottom, fill);
	CreateVERTEX(&v9[3], right, bottom, fill);
	CreateVERTEX(&v9[4], left, top, 0xFFBB0000);
	CreateVERTEX(&v9[5], right, top, 0xFFBB0000);
	CreateVERTEX(&v9[6], right, bottom, 0xFFBB0000);
	CreateVERTEX(&v9[7], left, bottom, 0xFFBB0000);
	CreateVERTEX(&v9[8], left, top, 0xFFBB0000);

	HRN(parent->device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, v9, sizeof(VERTEX)), L"primitive failed");
	HRN(parent->device->DrawPrimitiveUP(D3DPT_LINESTRIP, 4, &v9[4], sizeof(VERTEX)), L"primitive failed");

	float lastPos = 0;
	parent->line->Begin();
	int j = 0;
	float rightend = right + (step / 2);
	float distance = (abs(parent->actualTag.rangeMax) > 999 || 
		abs(parent->actualTag.rangeMin) > 999) ? 15 : 10;
	for (float i = left; i <= rightend; i += step) {
		if (i - lastPos > distance) {
			D3DXVECTOR2 linepoints[] = { D3DXVECTOR2(i, top - 5), D3DXVECTOR2(i, top + 10) };
			parent->line->Draw(linepoints, 2, 0xFFBB0000);
			lastPos = i;
			float thumbOnSliderValue = ((i - left) / coeff) - thumbposdiff;
			bool ismod0 = j % 4 == 0;
			if (j % 4 == 2 || ismod0) {
				RECT rect = { (long)i - 50, ismod0 ? (long)thumbbottom + 2 : (long)thumbtop - 54, (long)i + 50, ismod0 ? (long)thumbbottom + 54 : (long)thumbtop - 2 };
				int align = ismod0 ? DT_CENTER : DT_CENTER | DT_BOTTOM;
				DRAWOUTTEXT(parent->font, getfloat(thumbOnSliderValue, L"5.1f"), rect, align, 0xFFFFFFFF);
			}
			j++;
		}
	}
	parent->line->End();


	float thumbpos = ((thumbValue + thumbposdiff) * coeff) + left;
	float thumbleft = thumbpos - 4;
	float thumbright = thumbpos + 4;
	fill = (thumbState == 1) ? 0xAACC8748 : (thumbState == 2) ? 0xAAFCE6B1 : 0xAA121150;
	CreateVERTEX(&v9[0], thumbleft, thumbtop, fill);
	CreateVERTEX(&v9[1], thumbright, thumbtop, fill);
	CreateVERTEX(&v9[2], thumbleft, thumbbottom, fill);
	CreateVERTEX(&v9[3], thumbright, thumbbottom, fill);
	CreateVERTEX(&v9[4], thumbleft, thumbtop, 0xFFBB0000);
	CreateVERTEX(&v9[5], thumbright, thumbtop, 0xFFBB0000);
	CreateVERTEX(&v9[6], thumbright, thumbbottom, 0xFFBB0000);
	CreateVERTEX(&v9[7], thumbleft, thumbbottom, 0xFFBB0000);
	CreateVERTEX(&v9[8], thumbleft, thumbtop, 0xFFBB0000);

	HRN(parent->device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, v9, sizeof(VERTEX)), L"primitive failed");
	HRN(parent->device->DrawPrimitiveUP(D3DPT_LINESTRIP, 4, &v9[4], sizeof(VERTEX)), L"primitive failed");

	if (onThumb) {
		RECT rect = { (long)thumbleft - 50, (long)thumbbottom + 10, (long)thumbright + 50, (long)thumbbottom + 50 };
		wxString num = getfloat(thumbValue, parent->floatFormat);
		DRAWOUTTEXT(parent->font, num, rect, DT_CENTER, 0xFFFFFFFF);
	}
	if (onSlider) {
		float thumbOnSliderValue = ((x - left) / coeff) - thumbposdiff;
		thumbOnSliderValue = MID(parent->actualTag.rangeMin, thumbOnSliderValue, parent->actualTag.rangeMax);
		RECT rect = { (long)x - 50, (long)y + 20, (long)x + 50, (long)y + 70 };
		DRAWOUTTEXT(parent->font, getfloat(thumbOnSliderValue, parent->floatFormat), rect, DT_CENTER, 0xFFFFFFFF);
	}

	
}

float AllTagsSlider::GetThumbValue()
{
	return thumbValue;
}

float AllTagsSlider::GetDiffValue()
{
	return holding || parent->mode == 2 ? thumbValue - firstThumbValue : 0;
}

void AllTagsSlider::SetThumbValue(float value, bool setFirstThumbValue)
{
	if (setFirstThumbValue)
		firstThumbValue = thumbValue;

	thumbValue = value;
}

void AllTagsSlider::SetFirstThumbValue(float value)
{
	firstThumbValue = thumbState = value;
}

void AllTagsSlider::SetHolding(bool _holding)
{
	holding = _holding;
}

bool AllTagsSlider::GetHolding()
{
	return holding;
}

void AllTagsSlider::ResetOnThumbAndSlider()
{
	onThumb = onSlider = false;
}
