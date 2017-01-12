// Copyright (c) 2006, 2007, Niels Martin Hansen
// Copyright (c) 2016, Marcin Drob
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of the Aegisub Group nor the names of its contributors
//     may be used to endorse or promote products derived from this software
//     without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// Aegisub Project http://www.aegisub.org/

#ifndef DIALOG_COLORPICKER_H
#define DIALOG_COLORPICKER_H


#include "KaiDialog.h"
#include <wx/colour.h>
#include <wx/bitmap.h>
#include <wx/textctrl.h>
#include <wx/choice.h>
#include <wx/statbmp.h>
#include <wx/timer.h>
#include <vector>
#include "NumCtrl.h"
#include "Styles.h"
#include "MappedButton.h"

class ColorPickerSpectrum : public wxControl {
private:
	int x, y;
	wxBitmap *background;
	bool vertical;

	void OnPaint(wxPaintEvent &evt);
	void OnMouse(wxMouseEvent &evt);

public:
	ColorPickerSpectrum(wxWindow *parent, wxWindowID id, wxBitmap *_background, int xx, int yy, bool vert, wxSize _size);
	virtual ~ColorPickerSpectrum(){};

	void GetXY(int &xx, int &yy);
	void SetXY(int xx, int yy);
	void SetBackground(wxBitmap *new_background);

	DECLARE_EVENT_TABLE()
};

DECLARE_EVENT_TYPE(wxSPECTRUM_CHANGE, -1)


class ColorPickerRecent : public wxControl {
private:
	int rows, cols;
	int cellsize;
	wxPoint internal_control_offset;
	
	std::vector<wxColour> colors;
	
	bool background_valid;
	wxBitmap background;

	void OnClick(wxMouseEvent &evt);
	void OnPaint(wxPaintEvent &evt);
	void OnSize(wxSizeEvent &evt);

public:
	ColorPickerRecent(wxWindow *parent, wxWindowID id, int _cols, int _rows, int _cellsize);
	virtual ~ColorPickerRecent(){ };

	void LoadFromString(const wxString &recent_string);
	wxString StoreToString();
	void AddColor(wxColour color);
	wxColour GetColor(int n) { return colors.at(n); }

	DECLARE_EVENT_TABLE()
};

DECLARE_EVENT_TYPE(wxRECENT_SELECT, -1)


class ColorPickerScreenDropper : public wxControl {
private:
	wxBitmap capture;
	int resx, resy;
	int magnification;
	bool integrated_dropper;

	void OnMouse(wxMouseEvent &evt);
	void OnPaint(wxPaintEvent &evt);

public:
	ColorPickerScreenDropper(wxWindow *parent, wxWindowID id, int _resx, int _resy, int _magnification, bool _integrated_dropper);
	virtual ~ColorPickerScreenDropper(){ };
	void DropFromScreenXY(int x, int y);

	DECLARE_EVENT_TABLE()
};

DECLARE_EVENT_TYPE(wxDROPPER_SELECT, -1)


AssColor GetColorFromUser(wxWindow *parent, AssColor original);

class DialogColorPicker : public KaiDialog {
private:
	wxColour cur_color;
	bool updating_controls;
	bool spectrum_dirty;
	wxTimer updatecols;

	ColorPickerSpectrum *spectrum;
	ColorPickerSpectrum *slider;
	ColorPickerSpectrum *alphaslider;
	static const int slider_width = 10; // width in pixels of the color slider control

	// 0 = red, 1 = green, 2 = blue
	NumCtrl *rgb_input[3];
	
	// 0 = hue, 1 = saturation, 2 = luminance
	NumCtrl *hsl_input[3];
	
	// 0 = hue, 1 = saturation, 2 = value
	NumCtrl *hsv_input[3];
	wxBitmap *hsv_spectrum;		// s/v spectrum
	wxBitmap *hsv_slider;		// h spectrum
	wxBitmap *alpha_slider;

	wxBitmap eyedropper_bitmap;
	wxPoint eyedropper_grab_point;
	bool eyedropper_is_grabbed;

	KaiTextCtrl *ass_input;		// ASS hex format input
	KaiTextCtrl *html_input;		// HTML hex format input
	NumCtrl *alpha_input;

	//wxWindow *preview_box;
	wxStaticBitmap *preview_box;
	wxBitmap preview_bitmap;
	ColorPickerRecent *recent_box;
	ColorPickerScreenDropper *screen_dropper;
	wxStaticBitmap *screen_dropper_icon;

	void UpdateFromRGB();			// Update all other controls as a result of modifying an RGB control
	void UpdateFromHSL();			// Update all other controls as a result of modifying an HSL control
	void UpdateFromHSV();			// Update all other controls as a result of modifying an HSV control
	void UpdateFromASS();			// Update all other controls as a result of modifying the ASS format control
	void UpdateFromHTML();			// Update all other controls as a result of modifying the HTML format control
	void UpdateSpectrumDisplay();	// Redraw the spectrum display

	wxBitmap *MakeSVSpectrum();
	wxBitmap *MakeAlphaSlider();

	void OnChangeRGB(wxCommandEvent &evt);
	void OnChangeHSL(wxCommandEvent &evt);
	void OnChangeHSV(wxCommandEvent &evt);
	void OnChangeASS(wxCommandEvent &evt);
	void OnChangeHTML(wxCommandEvent &evt);
	void OnChangeAlpha(wxCommandEvent &evt);
	void OnSpectrumChange(wxCommandEvent &evt);
	void OnSliderChange(wxCommandEvent &evt);
	void OnAlphaSliderChange(wxCommandEvent &evt);
	void OnRecentSelect(wxCommandEvent &evt); // also handles dropper pick
	void OnDropperMouse(wxMouseEvent &evt);
	void OnMouse(wxMouseEvent &evt);
	void OnColourCanged(wxTimerEvent &evt);
	

public:
	DialogColorPicker(wxWindow *parent, AssColor initial_color);
	virtual ~DialogColorPicker();

	void SetColor(AssColor new_color);
	AssColor GetColor();

	static DialogColorPicker *DCP;
	static DialogColorPicker *Get(wxWindow *parent, AssColor color = wxColour("#000000"));

	DECLARE_EVENT_TABLE()
};

class ButtonColorPicker : public MappedButton
{
public:
	ButtonColorPicker(wxWindow *parent, AssColor color, wxSize size=wxDefaultSize);
	virtual ~ButtonColorPicker(){ };
	AssColor GetColor();
	void SetColor(const AssColor& color);
	bool SetBackgroundColour(const wxColour& color);
private:
	void OnClick(wxCommandEvent &event);
	AssColor ActualColor;
};
typedef ButtonColorPicker ColorButton;

class TextColorPicker : public wxWindow
{
public:
	TextColorPicker(wxWindow *parent, int id, const AssColor &color, const wxPoint &pos = wxDefaultPosition, const wxSize &size=wxDefaultSize, int style = 0);
	virtual ~TextColorPicker(){};
	AssColor GetColor();
	void SetColor(const AssColor& color);
private:
	void OnDoubleClick(wxMouseEvent &evt);
	void OnPaint(wxPaintEvent &evt);
	void OnSize(wxSizeEvent &evt);
	void OnErase(wxEraseEvent &evt){};
	AssColor color;
};

//Uwa¿aj aby nie stosowaæ idów
enum {
	SELECTOR_SPECTRUM = 14000,
	SELECTOR_SLIDER,
	SELECTOR_ALPHA_SLIDER,
	SELECTOR_MODE,
	SELECTOR_RGB_R,
	SELECTOR_RGB_G,
	SELECTOR_RGB_B,
	SELECTOR_HSL_H,
	SELECTOR_HSL_S,
	SELECTOR_HSL_L,
	SELECTOR_HSV_H,
	SELECTOR_HSV_S,
	SELECTOR_HSV_V,
	SELECTOR_ASS_INPUT,
	SELECTOR_HTML_INPUT,
	SELECTOR_ALPHA_INPUT,
	SELECTOR_RECENT,
	SELECTOR_DROPPER,
	SELECTOR_DROPPER_PICK
};


#endif
