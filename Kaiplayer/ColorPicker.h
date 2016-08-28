#ifndef DIALOG_COLORPICKER_H
#define DIALOG_COLORPICKER_H


#include <wx/dialog.h>
#include <wx/colour.h>
#include <wx/bitmap.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/choice.h>
#include <wx/statbmp.h>
#include <wx/timer.h>
#include <vector>
#include "NumCtrl.h"


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


wxColour GetColorFromUser(wxWindow *parent, wxColour original);

class DialogColorPicker : public wxDialog {
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

	wxTextCtrl *ass_input;		// ASS hex format input
	wxTextCtrl *html_input;		// HTML hex format input
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
	DialogColorPicker(wxWindow *parent, wxColour initial_color);
	virtual ~DialogColorPicker();

	void SetColor(wxColour new_color);
	wxColour GetColor();

	static DialogColorPicker *DCP;
	static DialogColorPicker *Get(wxWindow *parent, wxColour color="#000000");

	DECLARE_EVENT_TABLE()
};

class ButtonColorPicker : public wxButton
{
public:
	ButtonColorPicker(wxWindow *parent, wxColour color, wxSize size=wxDefaultSize);
	virtual ~ButtonColorPicker(){ };
	wxColour GetColor();
private:
	void OnClick(wxCommandEvent &event);
	wxColour color;
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
