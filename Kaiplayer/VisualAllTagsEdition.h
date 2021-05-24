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
#include "KaiDialog.h"
#include <vector>

class AllTagsSetting
{
public:
	AllTagsSetting() {};
	AllTagsSetting(const wxString& _name, const wxString& _tag,
		float _rangeMin, float _rangeMax, float _value,
		float _step, unsigned char _mode = 0) {
		name = _name;
		tag = _tag;
		rangeMin = _rangeMin;
		rangeMax = _rangeMax;
		value = _value;
		step = _step;
		mode = _mode;
	};
	wxString name;
	wxString tag;
	float rangeMin = 0.f;
	float rangeMax = 0.f;
	float value = 0.f;
	float step = 0.f;
	float value2 = 0.f;
	bool has2value = false;
	unsigned char mode = 0;
};

class AllTagsEdition : public KaiDialog
{
public:
	AllTagsEdition(wxWindow* parent, const wxPoint& pos, std::vector<AllTagsSetting>* _tags);
	virtual ~AllTagsEdition() {};
private:
	std::vector<AllTagsSetting>* tags;
};

void LoadSettings(std::vector<AllTagsSetting>* tags);

void GetNames(std::vector<AllTagsSetting>* tags, wxArrayString *nameList);