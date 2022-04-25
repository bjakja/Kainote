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
#include "ListControls.h"
#include "KaiTextCtrl.h"
#include "MappedButton.h"
#include "KaiStaticBoxSizer.h"
#include "KaiStaticText.h"
#include "KaiCheckBox.h"
#include "NumCtrl.h"
#include "VisualAllTagsEdition.h"
#include <vector>
#include <wx/arrstr.h>

enum TagType {
	IS_HEX_ALPHA = 1,
	IS_HEX_COLOR = 2,
	IS_VECTOR = 4,
	IS_T_ANIMATION = 8,
};

enum TagPasteMode {
	ADD = 0,
	INSERT,
	MULTIPLY,
	MULTIPLY_PLUS,
	GRADIENT_TEXT,
	GRADIENT_LINE
};

class AllTagsSetting
{
public:
	AllTagsSetting() {};
	AllTagsSetting(const wxString& _name, const wxString& _tag,
		float _rangeMin, float _rangeMax, float _value,
		float _step, unsigned char numDigitsAfterDot, unsigned char _mode = 0, int _tagMode = 0) {
		name = _name;
		tag = _tag;
		rangeMin = _rangeMin;
		rangeMax = _rangeMax;
		values[0] = _value;
		step = _step;
		digitsAfterDot = numDigitsAfterDot;
		mode = _mode;
		tagMode = _tagMode;
	};
	AllTagsSetting(const wxString& _name) {
		name = _name; 
		tag = name;
		step = 1.f;
		rangeMax = 100.f;
	};
	wxString name;
	wxString tag;
	float rangeMin = 0.f;
	float rangeMax = 0.f;
	float step = 0.f;
	float values[4] = { 0.f, 0.f, 0.f , 0.f };
	unsigned char mode = 0;
	unsigned char digitsAfterDot = 0;
	unsigned char numOfValues = 1;
	int tagMode = 0;
};

class AllTagsEdition : public KaiDialog
{
public:
	AllTagsEdition(wxWindow* parent, const wxPoint& pos, std::vector<AllTagsSetting>* _tags, int curTag);
	virtual ~AllTagsEdition() {};
	std::vector<AllTagsSetting>* GetTags() { return &tags; };
private:
	void OnSave(wxCommandEvent& evt);
	void OnResetDefault(wxCommandEvent& evt);
	void OnAddTag(wxCommandEvent& evt);
	void OnRemoveTag(wxCommandEvent& evt);
	void OnListChanged(wxCommandEvent& evt);
	void UpdateTag();
	void SetTagFromSettings();
	void SetTag(int num);
	bool CheckModified();
	void Save(int id);
	enum {
		ID_TAG_LIST = 7809,
		ID_ADDITIONAL_VALUES_LIST,
		ID_INSERT_MODES_LIST,
		ID_BUTTON_REMOVE_TAG,
		ID_BUTTON_ADD_TAG,
		ID_BUTTON_OK,
		ID_BUTTON_COMMIT,
		ID_BUTTON_RESET_DEFAULT
	};
	KaiChoice* tagList;
	KaiTextCtrl* newTagName;
	KaiTextCtrl* tagName;
	KaiTextCtrl* tagWithoutSlash;
	NumCtrl* minValue;
	NumCtrl* maxValue;
	NumCtrl* step;
	KaiChoice* mode;
	NumCtrl* digitAfterDot;
	NumCtrl* values[4] = { nullptr, nullptr, nullptr, nullptr };
	KaiChoice* numOfValues;
	KaiChoice* tagInsertMode;
	std::vector<AllTagsSetting> tags;
	AllTagsSetting currentTag;
	int selection = 0;
};

void LoadSettings(std::vector<AllTagsSetting>* tags);

void GetNames(std::vector<AllTagsSetting>* tags, wxArrayString *nameList);

void SaveSettings(std::vector<AllTagsSetting>* tags);