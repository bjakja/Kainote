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
#include "KaiCheckBox.h"
#include "NumCtrl.h"
#include "ColorPicker.h"
#include "TimeCtrl.h"
#include "KaiStaticText.h"

class DummyVideo : public KaiDialog
{
public:
	DummyVideo(wxWindow* parent);
	virtual ~DummyVideo() {};
	wxString GetDummyText();
private:
	void OnResolutionChoose(wxCommandEvent& evt);
	KaiChoice* videoResolution;
	NumCtrl* videoResolutionWidth;
	NumCtrl* videoResolutionHeight;
	ButtonColorPicker* color;
	KaiCheckBox* pattern;
	KaiChoice* frameRate;
	TimeCtrl* duration;
	KaiStaticText* frameDuration;

	enum {
		ID_VIDEO_RESOLUTION = 5678
	};
};
