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

#pragma once
#include <wx/window.h>

class AllTags;

class AllTagsSlider 
{
	friend class AllTags;
public:
	AllTagsSlider() {};
	void SetAllTags(AllTags* _parent);
	void OnMouseEvent(wxMouseEvent& evt);
	void OnDraw();
	void SetPosition(float _left, float _top, float _right, float _bottom) {
		left = _left;
		right = _right;
		top = _top;
		bottom = _bottom;
	}
	float GetThumbValue();
	float GetDiffValue();
	void SetThumbValue(float value, bool setFirstThumbValue = false);
	void SetFirstThumbValue(float value);
	void SetHolding(bool _holding);
	bool GetHolding();
	void ResetOnThumbAndSlider();
private:
	AllTags* parent = NULL;
	bool holding = false;
	//bool changeMoveDiff = false;
	float thumbValue = 0.f;
	float firstThumbValue = 0.f;
	float lastThumbValue = 0.f;
	float left = 0;
	float right = 0;
	float bottom = 0;
	float top = 0;
	float x = 0, y = 0;
	int thumbState = 0;
	bool onThumb = false;
	bool onSlider = false;
};