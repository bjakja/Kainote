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


#include "Config.h"

#include <wx/image.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/sizer.h>
#include <wx/gbsizer.h>
#include <wx/event.h>
#include <wx/tokenzr.h>
#include <wx/dcclient.h>
#include <wx/dcmemory.h>
#include <wx/dcscreen.h>
#include <wx/settings.h>
#include "KaiStaticText.h"
#include "ColorPicker.h"
#include "ColorSpace.h"
#include "KainoteApp.h"

#include <stdio.h>


#define STATIC_BORDER_FLAG wxSTATIC_BORDER




static const int spectrum_horz_vert_arrow_size = 4;

ColorPickerSpectrum::ColorPickerSpectrum(wxWindow *parent, wxWindowID id, wxBitmap *_background, int xx, int yy, bool vert, wxSize _size)
	: wxControl(parent, id, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE), x(xx), y(yy), background(_background), vertical(vert)
{
	_size.x += 2;
	_size.y += 2;

	if (vert) _size.x += spectrum_horz_vert_arrow_size + 1;
	//if (direction == Horz) _size.y += spectrum_horz_vert_arrow_size + 1;

	SetClientSize(_size);
	SetMinSize(GetSize());
}

void ColorPickerSpectrum::GetXY(int &xx, int &yy)
{
	xx = x;
	yy = y;
}

void ColorPickerSpectrum::SetXY(int xx, int yy)
{
	x = xx;
	y = yy;
	Refresh(false);
}

void ColorPickerSpectrum::SetBackground(wxBitmap *new_background)
{
	if (background == new_background) return;
	background = new_background;
	Refresh(false);
}

BEGIN_EVENT_TABLE(ColorPickerSpectrum, wxControl)
EVT_PAINT(ColorPickerSpectrum::OnPaint)
EVT_MOUSE_EVENTS(ColorPickerSpectrum::OnMouse)
END_EVENT_TABLE()

DEFINE_EVENT_TYPE(wxSPECTRUM_CHANGE)

void ColorPickerSpectrum::OnPaint(wxPaintEvent &evt)
{
	if (!background) return;


	wxSize siz = GetClientSize();
	wxBitmap bmp(siz.x, siz.y);
	wxMemoryDC dc;
	dc.SelectObject(bmp);
	dc.DrawBitmap(*background, 1, 1);
	//dc.Blit(1, 1, background->GetWidth(), background->GetHeight(), &memdc, 0, 0);

	wxPen invpen(*wxWHITE, 3);
	invpen.SetCap(wxCAP_BUTT);
	wxPen blkpen(Options.GetColour(WindowText), 1);
	blkpen.SetCap(wxCAP_BUTT);

	wxPoint arrow[3];


	dc.SetPen(invpen);
	if (!vertical){
		// Make a little cross
		dc.SetLogicalFunction(wxXOR);
		dc.DrawLine(x - 4, y + 1, x + 7, y + 1);
		dc.DrawLine(x + 1, y - 4, x + 1, y + 7);
		dc.SetLogicalFunction(wxCOPY);
	}
	else{
		// Make a horizontal line stretching all the way across
		dc.SetPen(invpen);
		dc.DrawLine(1, y + 1, background->GetWidth() + 1, y + 1);
		dc.SetPen(blkpen);
		dc.DrawLine(1, y + 1, background->GetWidth() + 1, y + 1);
		// Points for arrow
		// Arrow pointing at current point
		arrow[0] = wxPoint(background->GetWidth() + 2, y + 1);
		arrow[1] = wxPoint(background->GetWidth() + 2 + spectrum_horz_vert_arrow_size, y + 1 - spectrum_horz_vert_arrow_size);
		arrow[2] = wxPoint(background->GetWidth() + 2 + spectrum_horz_vert_arrow_size, y + 1 + spectrum_horz_vert_arrow_size);
		dc.SetPen(*wxTRANSPARENT_PEN);
		dc.SetBrush(wxBrush(Options.GetColour(WindowBackground)));
		dc.DrawRectangle(background->GetWidth() + 2, 0, siz.x - (background->GetWidth() + 2), siz.y);
		dc.SetBrush(Options.GetColour(WindowText));
		dc.DrawPolygon(3, arrow);
	}

	// Border around the spectrum
	dc.SetPen(blkpen);
	dc.SetBrush(*wxTRANSPARENT_BRUSH);
	dc.DrawRectangle(0, 0, background->GetWidth() + 2, background->GetHeight() + 2);

	wxPaintDC pdc(this);
	pdc.Blit(0, 0, siz.x, siz.y, &dc, 0, 0);
}

void ColorPickerSpectrum::OnMouse(wxMouseEvent &evt)
{
	evt.Skip();

	if (!evt.IsButton() && !evt.Dragging()) {
		return;
	}

	int newx = evt.GetX();
	if (newx < 0) newx = 0;
	if (newx >= GetClientSize().x) newx = GetClientSize().x - 1;
	int newy = evt.GetY();
	if (newy < 0) newy = 0;
	if (newy >= GetClientSize().y) newy = GetClientSize().y - 1;

	if (evt.LeftDown()) {
		CaptureMouse();
		SetCursor(wxCursor(wxCURSOR_BLANK));
	}
	else if (evt.LeftUp() && HasCapture()) {
		ReleaseMouse();
		SetCursor(wxNullCursor);
	}

	if (evt.LeftDown() || (HasCapture() && evt.LeftIsDown())) {
		x = newx;
		y = newy;
		Refresh(false);
		wxCommandEvent evt2(wxSPECTRUM_CHANGE, GetId());
		AddPendingEvent(evt2);
	}
}



ColorPickerRecent::ColorPickerRecent(wxWindow *parent, wxWindowID id, int _cols, int _rows, int _cellsize)
	: wxControl(parent, id, wxDefaultPosition, wxDefaultSize, STATIC_BORDER_FLAG)
	, rows(_rows)
	, cols(_cols)
	, cellsize(_cellsize)
	, internal_control_offset(0, 0)
	, background_valid(false)
	, background()
{
	LoadFromString(wxEmptyString);
	SetClientSize(cols*cellsize, rows*cellsize);
	SetMinSize(GetSize());
	SetMaxSize(GetSize());
	SetCursor(*wxCROSS_CURSOR);
}

void ColorPickerRecent::LoadFromString(const wxString &recent_string)
{
	colors.clear();
	wxStringTokenizer toker(recent_string, " ", wxTOKEN_STRTOK);
	//wxString deb;
	while (toker.HasMoreTokens()) {
		AssColor color;
		color.SetAss(toker.NextToken());
		//deb<<color.r<<" "<<color.g<<" "<<color.b<<" ";
		colors.push_back(color.GetWX());
	}
	while ((int)colors.size() < rows*cols) {
		colors.push_back(*wxBLACK);
	}
	//wxMessageBox(deb);
	background_valid = false;
}

wxString ColorPickerRecent::StoreToString()
{
	wxString res;
	for (int i = 0; i < rows*cols; i++) {
		AssColor color(colors[i]);
		res << color.GetAss(true, false) << " ";
	}
	res.Trim(true);
	return res;
}

void ColorPickerRecent::AddColor(wxColour color)
{
	for (std::vector<wxColor>::iterator i = colors.begin(); i != colors.end(); ++i) {
		if (color == *i) {
			colors.erase(i);
			break;
		}
	}

	colors.insert(colors.begin(), color);

	background_valid = false;

	Refresh(false);
}

BEGIN_EVENT_TABLE(ColorPickerRecent, wxControl)
EVT_PAINT(ColorPickerRecent::OnPaint)
EVT_LEFT_DOWN(ColorPickerRecent::OnClick)
EVT_SIZE(ColorPickerRecent::OnSize)
END_EVENT_TABLE()

DEFINE_EVENT_TYPE(wxRECENT_SELECT)

void ColorPickerRecent::OnClick(wxMouseEvent &evt)
{
	int cx, cy, i;
	wxSize cs = GetClientSize();
	cx = (evt.GetX() - internal_control_offset.x) * cols / cs.x;
	cy = (evt.GetY() - internal_control_offset.y) * rows / cs.y;
	if (cx < 0 || cx > cols || cy < 0 || cy > rows) return;
	i = cols*cy + cx;
	if (i >= 0 && i < (int)colors.size()) {
		AssColor color(colors[i]);
		wxCommandEvent evt(wxRECENT_SELECT, GetId());
		evt.SetString(color.GetAss(false, false));
		AddPendingEvent(evt);
	}
}

void ColorPickerRecent::OnPaint(wxPaintEvent &evt)
{
	wxPaintDC pdc(this);
	PrepareDC(pdc);

	if (!background_valid) {
		wxSize sz = pdc.GetSize();

		background = wxBitmap(sz.x, sz.y);
		wxMemoryDC dc(background);

		int i = 0;
		dc.SetPen(*wxTRANSPARENT_PEN);

		for (int cy = 0; cy < rows; cy++) {
			for (int cx = 0; cx < cols; cx++) {
				int x, y;
				x = cx * cellsize + internal_control_offset.x;
				y = cy * cellsize + internal_control_offset.y;

				dc.SetBrush(wxBrush(colors[i]));
				dc.DrawRectangle(x, y, x + cellsize, y + cellsize);

				i++;
			}
		}

		background_valid = true;
	}

	pdc.DrawBitmap(background, 0, 0, false);
}

void ColorPickerRecent::OnSize(wxSizeEvent &evt)
{
	wxSize size = GetClientSize();
	background_valid = false;
	//internal_control_offset.x = (size.GetWidth() - cellsize * cols) / 2;
	//internal_control_offset.y = (size.GetHeight() - cellsize * rows) / 2;
	Refresh(false);
}



ColorPickerScreenDropper::ColorPickerScreenDropper(wxWindow *parent, wxWindowID id, int _resx, int _resy, int _magnification, bool _integrated_dropper)
	: wxControl(parent, id, wxDefaultPosition, wxDefaultSize, STATIC_BORDER_FLAG), resx(_resx), resy(_resy), magnification(_magnification), integrated_dropper(_integrated_dropper)
{
	SetClientSize(resx*magnification, resy*magnification);
	SetMinSize(GetSize());
	SetMaxSize(GetSize());
	SetCursor(*wxCROSS_CURSOR);

	capture = wxBitmap(resx, resy);
	wxMemoryDC capdc;
	capdc.SelectObject(capture);
	capdc.SetPen(*wxTRANSPARENT_PEN);
	capdc.SetBrush(*wxWHITE_BRUSH);
	capdc.DrawRectangle(0, 0, resx, resy);
}

BEGIN_EVENT_TABLE(ColorPickerScreenDropper, wxControl)
EVT_PAINT(ColorPickerScreenDropper::OnPaint)
EVT_MOUSE_EVENTS(ColorPickerScreenDropper::OnMouse)
END_EVENT_TABLE()

wxDEFINE_EVENT(wxDROPPER_SELECT, wxCommandEvent);
wxDEFINE_EVENT(wxDROPPER_MOUSE_UP, wxCommandEvent);

void ColorPickerScreenDropper::OnMouse(wxMouseEvent &evt)
{
	int x, y;
	x = evt.GetX() / magnification;
	y = evt.GetY() / magnification;

	if (HasCapture() && (evt.LeftIsDown() || evt.RightIsDown())) {

		wxPoint pos = ClientToScreen(evt.GetPosition());
		DropFromScreenXY(pos.x, pos.y);
		if (evt.RightIsDown())
			SendGetColorEvent(resx / 2, resy / 2);
	}
	else if (evt.LeftDown()) {

		if (x == 0 && y == 0 && integrated_dropper) {
			CaptureMouse();

		}
		else if (x >= 0 && y >= 0 && x < resx && y < resy) {
			SendGetColorEvent(x, y);
		}

	}
	else if (HasCapture() && evt.LeftUp()) {
		ReleaseMouse();
		wxCommandEvent dropperevt(wxDROPPER_MOUSE_UP, GetId());
		AddPendingEvent(dropperevt);
	}
}

void ColorPickerScreenDropper::OnPaint(wxPaintEvent &evt)
{
	wxPaintDC pdc(this);


	wxMemoryDC capdc(capture);


	pdc.SetPen(*wxTRANSPARENT_PEN);

	for (int x = 0; x < resx; x++) {
		for (int y = 0; y < resy; y++) {
			if (x == 0 && y == 0 && integrated_dropper) continue;

			wxColour color;

			capdc.GetPixel(x, y, &color);
			pdc.SetBrush(wxBrush(color));
			pdc.DrawRectangle(x*magnification, y*magnification, magnification, magnification);
		}
	}

	if (integrated_dropper) {
		wxBrush cbrush(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
		pdc.SetBrush(cbrush);
		pdc.DrawRectangle(0, 0, magnification, magnification);
		cbrush.SetStyle(wxCROSSDIAG_HATCH);
		cbrush.SetColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT));
		pdc.SetBrush(cbrush);
		pdc.DrawRectangle(0, 0, magnification, magnification);
	}
}



void ColorPickerScreenDropper::DropFromScreenXY(int x, int y)
{
	wxMemoryDC capdc(capture);
	wxScreenDC screen;


	screen.StartDrawingOnTop();
	capdc.Blit(0, 0, resx, resy, &screen, x - resx / 2, y - resy / 2);
	screen.EndDrawingOnTop();


	Refresh(false);
}

void ColorPickerScreenDropper::SendGetColorEvent(int x, int y)
{
	wxColour color;
	wxMemoryDC capdc(capture);
	capdc.GetPixel(x, y, &color);
	color = wxColour(color.Red(), color.Green(), color.Blue(), wxALPHA_OPAQUE);
	AssColor ass(color);
	wxCommandEvent evt(wxDROPPER_SELECT, GetId());
	evt.SetString(ass.GetAss(false, false));
	AddPendingEvent(evt);
}

//AssColor GetColorFromUser(wxWindow *parent, AssColor original)
//{
//	DialogColorPicker dialog(parent, original);
//	if (dialog.ShowModal() == wxID_OK) {
//		return dialog.GetColor();
//	}
//	else {
//		return original;
//	}
//}

wxDEFINE_EVENT(COLOR_TYPE_CHANGED, wxCommandEvent);
wxDEFINE_EVENT(COLOR_CHANGED, ColorEvent);

// Constructor
DialogColorPicker::DialogColorPicker(wxWindow *parent, AssColor initial_color, int colorNum)
	: KaiDialog(parent, 11111, _("Wybierz kolor"), wxDefaultPosition, wxDefaultSize)
{
	SetForegroundColour(Options.GetColour(WindowText));
	SetBackgroundColour(Options.GetColour(WindowBackground));
	wxAcceleratorEntry centries[2];
	centries[0].Set(wxACCEL_NORMAL, WXK_RETURN, wxID_OK);
	centries[1].Set(wxACCEL_NORMAL, WXK_ESCAPE, wxID_CANCEL);
	wxAcceleratorTable caccel(2, centries);
	this->SetAcceleratorTable(caccel);
	updatecols.SetOwner(this, 7789);
	Connect(7789, wxEVT_TIMER, (wxObjectEventFunction)&DialogColorPicker::OnColourChanged);

	hsv_spectrum = 0;
	alphaslider = 0;
	alpha_slider = 0;
	spectrum_dirty = true;

	// generate spectrum slider bar images
	wxImage sliderimg(slider_width, 256, true);
	unsigned char *oslid, *slid;

	oslid = slid = (unsigned char *)malloc(slider_width * 256 * 3);
	for (int y = 0; y < 256; y++) {
		for (int x = 0; x < slider_width; x++) {
			hsv_to_rgb(y, 255, 255, slid, slid + 1, slid + 2);
			slid += 3;
		}
	}
	sliderimg.SetData(oslid);
	hsv_slider = new wxBitmap(sliderimg);

	// Create the controls for the dialog
	wxSizer *spectrum_box = new KaiStaticBoxSizer(wxVERTICAL, this, _("Kolor spektrum"));
	spectrum = new ColorPickerSpectrum(this, SELECTOR_SPECTRUM, 0, -1, -1, false, wxSize(256, 256));
	slider = new ColorPickerSpectrum(this, SELECTOR_SLIDER, 0, -1, -1, true, wxSize(slider_width, 256));
	alphaslider = new ColorPickerSpectrum(this, SELECTOR_ALPHA_SLIDER, 0, -1, -1, true, wxSize(slider_width, 256));

	wxSize colorinput_size(45, -1);
	wxSize textinput_size(80, -1);
	wxSize colorinput_labelsize(70, -1);
	wxSize textinput_labelsize(45, -1);

	wxSizer *rgb_box = new KaiStaticBoxSizer(wxHORIZONTAL, this, _("Kolor RGB"));
	rgb_input[0] = new NumCtrl(this, SELECTOR_RGB_R, "", 0, 255, true, wxDefaultPosition, colorinput_size);
	rgb_input[1] = new NumCtrl(this, SELECTOR_RGB_G, "", 0, 255, true, wxDefaultPosition, colorinput_size);
	rgb_input[2] = new NumCtrl(this, SELECTOR_RGB_B, "", 0, 255, true, wxDefaultPosition, colorinput_size);

	wxSizer *hsl_box = new KaiStaticBoxSizer(wxVERTICAL, this, _("Kolor HSL"));
	hsl_input[0] = new NumCtrl(this, SELECTOR_HSL_H, "", 0, 255, true, wxDefaultPosition, colorinput_size);
	hsl_input[1] = new NumCtrl(this, SELECTOR_HSL_S, "", 0, 255, true, wxDefaultPosition, colorinput_size);
	hsl_input[2] = new NumCtrl(this, SELECTOR_HSL_L, "", 0, 255, true, wxDefaultPosition, colorinput_size);

	wxSizer *hsv_box = new KaiStaticBoxSizer(wxVERTICAL, this, _("Kolor HSV"));
	hsv_input[0] = new NumCtrl(this, SELECTOR_HSV_H, "", 0, 255, true, wxDefaultPosition, colorinput_size);
	hsv_input[1] = new NumCtrl(this, SELECTOR_HSV_S, "", 0, 255, true, wxDefaultPosition, colorinput_size);
	hsv_input[2] = new NumCtrl(this, SELECTOR_HSV_V, "", 0, 255, true, wxDefaultPosition, colorinput_size);

	ass_input = new KaiTextCtrl(this, SELECTOR_ASS_INPUT, "", wxDefaultPosition, textinput_size);
	html_input = new KaiTextCtrl(this, SELECTOR_HTML_INPUT, "", wxDefaultPosition, textinput_size);
	alpha_input = new NumCtrl(this, SELECTOR_ALPHA_INPUT, "", 0, 255, true, wxDefaultPosition, textinput_size);

	preview_bitmap = wxBitmap(40, 40, 24);
	preview_box = new wxStaticBitmap(this, -1, preview_bitmap, wxDefaultPosition, wxSize(40, 40), STATIC_BORDER_FLAG);

	recent_box = new ColorPickerRecent(this, SELECTOR_RECENT, 8, 4, 16);

	eyedropper_bitmap = wxBITMAP_PNG("eyedropper_tool");
	//eyedropper_bitmap.SetMask(new wxMask(eyedropper_bitmap, wxColour(255, 0, 255)));
	screen_dropper_icon = new wxStaticBitmap(this, SELECTOR_DROPPER, eyedropper_bitmap, wxDefaultPosition, wxSize(32, 32), wxSTATIC_BORDER);
	screen_dropper = new ColorPickerScreenDropper(this, SELECTOR_DROPPER_PICK, 7, 7, 8, false);

	wxString types[] = { _("Kolor podstawowy"), _("Kolor zastępczy"), _("Kolor obwódki"), _("Kolor cienia") };
	colorType = new KaiChoice(this, 9766, wxDefaultPosition, wxSize(120, -1), 4, types);
	if (colorNum == -1)
		colorType->Enable(false);
	else
		colorType->SetSelection(colorNum - 1);

	Bind(wxEVT_COMMAND_CHOICE_SELECTED, [=](wxCommandEvent &evt){
		wxCommandEvent ctcevt(COLOR_TYPE_CHANGED, GetId());
		//selections starts from 0, colors from 1
		ctcevt.SetInt(colorType->GetSelection() + 1);
		ProcessEvent(ctcevt);
	}, 9766);

	// Arrange the controls in a nice way
	wxSizer *spectop_sizer = new wxBoxSizer(wxHORIZONTAL);
	spectop_sizer->Add(colorType, 1, wxALIGN_CENTER_VERTICAL | wxLEFT | wxEXPAND, 2);
	spectop_sizer->Add(new KaiStaticText(this, -1, _("Wybrany kolor:")), 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 17);
	spectop_sizer->Add(preview_box);
	wxSizer *spectrum_sizer = new wxBoxSizer(wxHORIZONTAL);
	//spectrum_sizer->Add(spectop_sizer, wxALIGN_CENTER_HORIZONTAL);
	//spectrum_sizer->AddStretchSpacer(1);
	spectrum_sizer->Add(spectrum, 0, wxALL, 2);
	spectrum_sizer->Add(slider, 0, wxALL, 2);
	spectrum_sizer->Add(alphaslider, 0, wxALL, 2);
	spectrum_box->Add(spectop_sizer, 0, wxALL, 3);
	spectrum_box->Add(spectrum_sizer, 0, wxALL, 3);
	wxFlexGridSizer *rgb_sizer = new wxFlexGridSizer(2, 5, 5);
	rgb_sizer->Add(new KaiStaticText(this, -1, _("Czerwony:"), wxDefaultPosition, colorinput_labelsize), 1, wxALIGN_CENTER_VERTICAL);
	rgb_sizer->Add(rgb_input[0], 0);
	rgb_sizer->Add(new KaiStaticText(this, -1, _("Zielony:"), wxDefaultPosition, colorinput_labelsize), 1, wxALIGN_CENTER_VERTICAL);
	rgb_sizer->Add(rgb_input[1], 0);
	rgb_sizer->Add(new KaiStaticText(this, -1, _("Niebieski:"), wxDefaultPosition, colorinput_labelsize), 1, wxALIGN_CENTER_VERTICAL);
	rgb_sizer->Add(rgb_input[2], 0);
	rgb_sizer->AddGrowableCol(0, 1);
	rgb_box->Add(rgb_sizer, 1, wxEXPAND | wxALL | wxALIGN_CENTER_VERTICAL, 3);

	wxFlexGridSizer *ass_input_sizer = new wxFlexGridSizer(2, 5, 5);
	ass_input_sizer->Add(new KaiStaticText(this, -1, "ASS:", wxDefaultPosition, textinput_labelsize), 1, wxALIGN_CENTER_VERTICAL);
	ass_input_sizer->Add(ass_input, 0);
	ass_input_sizer->Add(new KaiStaticText(this, -1, "HTML:", wxDefaultPosition, textinput_labelsize), 1, wxALIGN_CENTER_VERTICAL);
	ass_input_sizer->Add(html_input, 0);
	ass_input_sizer->Add(new KaiStaticText(this, -1, "Alpha:", wxDefaultPosition, textinput_labelsize), 1, wxALIGN_CENTER_VERTICAL);
	ass_input_sizer->Add(alpha_input, 0);
	//ass_input_sizer->AddStretchSpacer();
	ass_input_sizer->AddGrowableCol(0, 1);
	rgb_box->Add(ass_input_sizer, 0, wxALL | wxALIGN_CENTER_VERTICAL | wxEXPAND, 3);

	wxFlexGridSizer *hsl_sizer = new wxFlexGridSizer(2, 5, 5);
	hsl_sizer->Add(new KaiStaticText(this, -1, _("Odcień:"), wxDefaultPosition, colorinput_labelsize), 1, wxALIGN_CENTER_VERTICAL);
	hsl_sizer->Add(hsl_input[0], 0);
	hsl_sizer->Add(new KaiStaticText(this, -1, _("Nasycenie:"), wxDefaultPosition, colorinput_labelsize), 1, wxALIGN_CENTER_VERTICAL);
	hsl_sizer->Add(hsl_input[1], 0);
	hsl_sizer->Add(new KaiStaticText(this, -1, _("Jaskrawość:"), wxDefaultPosition, colorinput_labelsize), 1, wxALIGN_CENTER_VERTICAL);
	hsl_sizer->Add(hsl_input[2], 0);
	hsl_sizer->AddGrowableCol(0, 1);
	hsl_box->Add(hsl_sizer, 0, wxALL | wxALIGN_CENTER_VERTICAL | wxEXPAND, 3);

	wxFlexGridSizer *hsv_sizer = new wxFlexGridSizer(2, 5, 5);
	hsv_sizer->Add(new KaiStaticText(this, -1, _("Odcień:"), wxDefaultPosition, colorinput_labelsize), 1, wxALIGN_CENTER_VERTICAL);
	hsv_sizer->Add(hsv_input[0], 0);
	hsv_sizer->Add(new KaiStaticText(this, -1, _("Nasycenie:"), wxDefaultPosition, colorinput_labelsize), 1, wxALIGN_CENTER_VERTICAL);
	hsv_sizer->Add(hsv_input[1], 0);
	hsv_sizer->Add(new KaiStaticText(this, -1, _("Wartość:"), wxDefaultPosition, colorinput_labelsize), 1, wxALIGN_CENTER_VERTICAL);
	hsv_sizer->Add(hsv_input[2], 0);
	hsv_sizer->AddGrowableCol(0, 1);
	hsv_box->Add(hsv_sizer, 0, wxALL | wxALIGN_CENTER_VERTICAL | wxEXPAND, 3);

	wxSizer *hsx_sizer = new wxBoxSizer(wxHORIZONTAL);
	hsx_sizer->Add(hsl_box);
	hsx_sizer->AddSpacer(5);
	hsx_sizer->Add(hsv_box);

	wxSizer *recent_sizer = new wxBoxSizer(wxVERTICAL);
	recent_sizer->Add(recent_box, 1, wxEXPAND);

	wxSizer *picker_sizer = new wxBoxSizer(wxHORIZONTAL);
	picker_sizer->AddStretchSpacer();
	picker_sizer->Add(screen_dropper_icon, 0, wxALIGN_CENTER | wxRIGHT, 10);
	picker_sizer->Add(screen_dropper, 0, wxALIGN_CENTER);
	picker_sizer->AddStretchSpacer();
	picker_sizer->Add(recent_sizer, 0, wxALIGN_RIGHT | wxRIGHT | wxTOP | wxBOTTOM, 4);
	//picker_sizer->AddStretchSpacer();

	wxSizer *button_sizer = new wxBoxSizer(wxHORIZONTAL);
	button_sizer->Add(new MappedButton(this, wxID_OK, "OK"), 1, wxALL, 4);
	button_sizer->Add(new MappedButton(this, wxID_CANCEL, _("Anuluj")), 1, wxALL, 4);

	wxSizer *input_sizer = new wxBoxSizer(wxVERTICAL);
	input_sizer->Add(rgb_box, 0, wxALIGN_CENTER | wxEXPAND);
	input_sizer->AddSpacer(5);
	input_sizer->Add(hsx_sizer, 0, wxALIGN_CENTER | wxEXPAND);
	input_sizer->AddStretchSpacer(1);
	input_sizer->Add(picker_sizer, 0, wxALIGN_CENTER | wxEXPAND);
	input_sizer->AddStretchSpacer(2);
	input_sizer->Add(button_sizer, 0, wxALIGN_RIGHT | wxALIGN_BOTTOM);

	DialogSizer *main_sizer = new DialogSizer(wxHORIZONTAL);
	main_sizer->Add(spectrum_box, 1, wxALL | wxEXPAND, 5);
	main_sizer->Add(input_sizer, 0, (wxALL&~wxLEFT) | wxEXPAND, 5);

	SetSizerAndFit(main_sizer);
	//main_sizer->SetSizeHints(this);

	//ass_input->SetFocus();


	// Fill the controls
	updating_controls = false;
	SetColor(initial_color, colorNum);
	recent_box->LoadFromString(Options.GetString(ColorpickerRecent));

	// The mouse event handler for the Dropper control must be manually assigned
	// The EVT_MOUSE_EVENTS macro can't take a control id
	screen_dropper_icon->Connect(wxEVT_MOTION, wxMouseEventHandler(DialogColorPicker::OnDropperMouse), 0, this);
	screen_dropper_icon->Connect(wxEVT_LEFT_DOWN, wxMouseEventHandler(DialogColorPicker::OnDropperMouse), 0, this);
	screen_dropper_icon->Connect(wxEVT_LEFT_UP, wxMouseEventHandler(DialogColorPicker::OnDropperMouse), 0, this);
	screen_dropper_icon->Connect(wxEVT_RIGHT_UP, wxMouseEventHandler(DialogColorPicker::OnDropperMouse), 0, this);
	Connect(SELECTOR_RGB_R, NUMBER_CHANGED, (wxObjectEventFunction)&DialogColorPicker::OnChangeRGB);
	Connect(SELECTOR_RGB_G, NUMBER_CHANGED, (wxObjectEventFunction)&DialogColorPicker::OnChangeRGB);
	Connect(SELECTOR_RGB_B, NUMBER_CHANGED, (wxObjectEventFunction)&DialogColorPicker::OnChangeRGB);
	Connect(SELECTOR_HSL_H, NUMBER_CHANGED, (wxObjectEventFunction)&DialogColorPicker::OnChangeHSL);
	Connect(SELECTOR_HSL_S, NUMBER_CHANGED, (wxObjectEventFunction)&DialogColorPicker::OnChangeHSL);
	Connect(SELECTOR_HSL_L, NUMBER_CHANGED, (wxObjectEventFunction)&DialogColorPicker::OnChangeHSL);
	Connect(SELECTOR_HSV_H, NUMBER_CHANGED, (wxObjectEventFunction)&DialogColorPicker::OnChangeHSV);
	Connect(SELECTOR_HSV_S, NUMBER_CHANGED, (wxObjectEventFunction)&DialogColorPicker::OnChangeHSV);
	Connect(SELECTOR_HSV_V, NUMBER_CHANGED, (wxObjectEventFunction)&DialogColorPicker::OnChangeHSV);
	Connect(SELECTOR_ALPHA_INPUT, NUMBER_CHANGED, (wxObjectEventFunction)&DialogColorPicker::OnChangeAlpha);
}


// Destructor
DialogColorPicker::~DialogColorPicker()
{

	delete hsv_spectrum;
	delete hsv_slider;
	delete alpha_slider;

	if (screen_dropper_icon->HasCapture()) screen_dropper_icon->ReleaseMouse();
	DCP = NULL;
}


// Sets the currently selected color, and updates all controls
void DialogColorPicker::SetColor(AssColor new_color, int numColor /*= 0*/, bool SendVideoEvent /*= true*/)
{
	cur_color = new_color.GetWX();
	rgb_input[0]->SetInt(new_color.r);
	rgb_input[1]->SetInt(new_color.g);
	rgb_input[2]->SetInt(new_color.b);
	alpha_input->SetInt(new_color.a);
	updating_controls = false;
	spectrum_dirty = true;
	if (numColor){
		colorType->SetSelection(numColor - 1);
		colorType->Enable(numColor != -1);
	}
	UpdateFromRGB(SendVideoEvent);
}


// Get the currently selected color
AssColor DialogColorPicker::GetColor()
{
	recent_box->AddColor(cur_color);
	Options.SetString(ColorpickerRecent, recent_box->StoreToString());
	return cur_color;
}


// Use the values entered in the RGB controls to update the other controls
void DialogColorPicker::UpdateFromRGB(bool SendVideoEvent /*= true*/)
{
	if (updating_controls) return;
	updating_controls = true;

	unsigned char r, g, b, h, s, l, h2, s2, v2;
	r = rgb_input[0]->GetInt();
	g = rgb_input[1]->GetInt();
	b = rgb_input[2]->GetInt();
	rgb_to_hsl(r, g, b, &h, &s, &l);
	rgb_to_hsv(r, g, b, &h2, &s2, &v2);
	hsl_input[0]->SetInt(h);
	hsl_input[1]->SetInt(s);
	hsl_input[2]->SetInt(l);
	hsv_input[0]->SetInt(h2);
	hsv_input[1]->SetInt(s2);
	hsv_input[2]->SetInt(v2);
	cur_color = wxColour(r, g, b, 0xFF - alpha_input->GetInt());
	ass_input->SetValue(AssColor(cur_color).GetAss(false, false));
	html_input->SetValue(color_to_html(cur_color));
	UpdateSpectrumDisplay(SendVideoEvent);

	updating_controls = false;
}


// Use the values entered in the HSL controls to update the other controls
void DialogColorPicker::UpdateFromHSL()
{
	if (updating_controls) return;
	updating_controls = true;

	unsigned char r, g, b, h, s, l, h2, s2, v2;
	h = hsl_input[0]->GetInt();
	s = hsl_input[1]->GetInt();
	l = hsl_input[2]->GetInt();
	hsl_to_rgb(h, s, l, &r, &g, &b);
	hsl_to_hsv(h, s, l, &h2, &s2, &v2);
	rgb_input[0]->SetInt(r);
	rgb_input[1]->SetInt(g);
	rgb_input[2]->SetInt(b);
	hsv_input[0]->SetInt(h2);
	hsv_input[1]->SetInt(s2);
	hsv_input[2]->SetInt(v2);
	cur_color = wxColour(r, g, b, 0xFF - alpha_input->GetInt());
	ass_input->SetValue(AssColor(cur_color).GetAss(false, false));
	html_input->SetValue(color_to_html(cur_color));
	UpdateSpectrumDisplay();

	updating_controls = false;
}


void DialogColorPicker::UpdateFromHSV()
{
	if (updating_controls) return;
	updating_controls = true;

	unsigned char r, g, b, h, s, l, h2, s2, v2;

	h2 = hsv_input[0]->GetInt();
	s2 = hsv_input[1]->GetInt();
	v2 = hsv_input[2]->GetInt();
	hsv_to_rgb(h2, s2, v2, &r, &g, &b);
	hsv_to_hsl(h2, s2, v2, &h, &s, &l);
	rgb_input[0]->SetInt(r);
	rgb_input[1]->SetInt(g);
	rgb_input[2]->SetInt(b);
	hsl_input[0]->SetInt(h);
	hsl_input[1]->SetInt(s);
	hsl_input[2]->SetInt(l);
	cur_color = wxColour(r, g, b, 0xFF - alpha_input->GetInt());
	ass_input->SetValue(AssColor(cur_color).GetAss(false, false));
	html_input->SetValue(color_to_html(cur_color));
	UpdateSpectrumDisplay();

	updating_controls = false;
}


// Use the value entered in the ASS hex control to update the other controls
void DialogColorPicker::UpdateFromASS()
{
	if (updating_controls) return;
	updating_controls = true;

	unsigned char r, g, b, h, s, l, h2, s2, v2;
	AssColor ass;
	ass.SetAss(ass_input->GetValue());
	r = ass.r;
	g = ass.g;
	b = ass.b;
	rgb_to_hsl(r, g, b, &h, &s, &l);
	rgb_to_hsv(r, g, b, &h2, &s2, &v2);
	rgb_input[0]->SetInt(r);
	rgb_input[1]->SetInt(g);
	rgb_input[2]->SetInt(b);
	hsl_input[0]->SetInt(h);
	hsl_input[1]->SetInt(s);
	hsl_input[2]->SetInt(l);
	hsv_input[0]->SetInt(h2);
	hsv_input[1]->SetInt(s2);
	hsv_input[2]->SetInt(v2);
	cur_color = wxColour(r, g, b, 0xFF - alpha_input->GetInt());
	html_input->SetValue(color_to_html(cur_color));
	UpdateSpectrumDisplay();

	updating_controls = false;
}


void DialogColorPicker::UpdateFromHTML()
{
	if (updating_controls) return;
	updating_controls = true;

	unsigned char r, g, b, h, s, l, h2, s2, v2;
	cur_color = html_to_color(html_input->GetValue());
	r = cur_color.Red();
	g = cur_color.Green();
	b = cur_color.Blue();
	rgb_to_hsl(r, g, b, &h, &s, &l);
	rgb_to_hsv(r, g, b, &h2, &s2, &v2);
	rgb_input[0]->SetInt(r);
	rgb_input[1]->SetInt(g);
	rgb_input[2]->SetInt(b);
	hsl_input[0]->SetInt(h);
	hsl_input[1]->SetInt(s);
	hsl_input[2]->SetInt(l);
	hsv_input[0]->SetInt(h2);
	hsv_input[1]->SetInt(s2);
	hsv_input[2]->SetInt(v2);
	cur_color = wxColour(r, g, b, 0xFF - alpha_input->GetInt());
	ass_input->SetValue(AssColor(cur_color).GetAss(false, false));
	UpdateSpectrumDisplay();

	updating_controls = false;
}


void DialogColorPicker::UpdateSpectrumDisplay(bool SendVideoEvent /*= true*/)
{
	if (spectrum_dirty){
		slider->SetBackground(hsv_slider);
		slider->SetXY(0, hsv_input[0]->GetInt());
		spectrum->SetBackground(MakeSVSpectrum());
		spectrum->SetXY(hsv_input[1]->GetInt(), hsv_input[2]->GetInt());
		spectrum_dirty = false;
	}

	alphaslider->SetBackground(MakeAlphaSlider());
	alphaslider->SetXY(0, alpha_input->GetInt());


	wxBitmap tempBmp = preview_box->GetBitmap();
	{
		wxMemoryDC previewdc;
		previewdc.SelectObject(tempBmp);
		previewdc.SetPen(*wxTRANSPARENT_PEN);
		previewdc.SetBrush(wxBrush(cur_color));
		previewdc.DrawRectangle(0, 0, 40, 40);
	}
	preview_box->SetBitmap(tempBmp);
	if (IsShown() && SendVideoEvent){
		updatecols.Start(100, true);
	}
}


wxBitmap *DialogColorPicker::MakeSVSpectrum()
{
	if (hsv_spectrum) delete hsv_spectrum;

	wxImage spectrum_image(256, 256, false);
	unsigned char *ospec, *spec;

	ospec = spec = (unsigned char *)malloc(256 * 256 * 3);

	int h = hsv_input[0]->GetInt();
	unsigned char maxr, maxg, maxb;
	hsv_to_rgb(h, 255, 255, &maxr, &maxg, &maxb);

	for (int v = 0; v < 256; v++) {
		int rr, rg, rb;
		rr = (255 - maxr) * v / 256;
		rg = (255 - maxg) * v / 256;
		rb = (255 - maxb) * v / 256;
		for (int s = 0; s < 256; s++) {
			int r, g, b;
			r = 255 - rr * s / 256 - (255 - v);
			g = 255 - rg * s / 256 - (255 - v);
			b = 255 - rb * s / 256 - (255 - v);
			*spec++ = r;
			*spec++ = g;
			*spec++ = b;
		}
	}
	spectrum_image.SetData(ospec);
	hsv_spectrum = new wxBitmap(spectrum_image);

	return hsv_spectrum;
}

wxBitmap *DialogColorPicker::MakeAlphaSlider()
{
	if (alpha_slider) delete alpha_slider;
	wxImage aslider_image(10, 256, false);
	unsigned char *ospec, *spec;

	ospec = spec = (unsigned char *)malloc(10 * 256 * 3);
	const wxColour & kol1 = Options.GetColour(StylePreviewColor1);
	const wxColour & kol2 = Options.GetColour(StylePreviewColor2);
	byte b3 = kol1.Blue();
	byte g3 = kol1.Green();
	byte r3 = kol1.Red();

	byte b1 = kol2.Blue();
	byte g1 = kol2.Green();
	byte r1 = kol2.Red();

	byte b2 = cur_color.Blue();
	byte g2 = cur_color.Green();
	byte r2 = cur_color.Red();
	//byte a=cur_color.Alpha();

	bool ch = false;
	bool ch1 = false;
	for (int i = 0; i < 256; i++)
	{
		unsigned char inv_a = 0xFF - i;
		if ((i % 5) == 0){ ch1 = !ch1; }
		ch = ch1;
		for (int j = 0; j < 10; j++)
		{
			int k = ((i * 10) + j) * 3;
			if ((j % 5) == 0 && j>0){ ch = !ch; }
			int b = (ch) ? b3 : b1;
			int g = (ch) ? g3 : g1;
			int r = (ch) ? r3 : r1;
			spec[k] = (r2* inv_a / 0xFF) + (r - inv_a * r / 0xFF);
			spec[k + 1] = (g2* inv_a / 0xFF) + (g - inv_a * g / 0xFF);
			spec[k + 2] = (b2* inv_a / 0xFF) + (b - inv_a * b / 0xFF);
		}

	}


	aslider_image.SetData(ospec);
	alpha_slider = new wxBitmap(aslider_image);

	return alpha_slider;
}



BEGIN_EVENT_TABLE(DialogColorPicker, KaiDialog)
EVT_TEXT(SELECTOR_ASS_INPUT, DialogColorPicker::OnChangeASS)
EVT_TEXT(SELECTOR_HTML_INPUT, DialogColorPicker::OnChangeHTML)
EVT_COMMAND(SELECTOR_SPECTRUM, wxSPECTRUM_CHANGE, DialogColorPicker::OnSpectrumChange)
EVT_COMMAND(SELECTOR_SLIDER, wxSPECTRUM_CHANGE, DialogColorPicker::OnSliderChange)
EVT_COMMAND(SELECTOR_ALPHA_SLIDER, wxSPECTRUM_CHANGE, DialogColorPicker::OnAlphaSliderChange)
EVT_COMMAND(SELECTOR_RECENT, wxRECENT_SELECT, DialogColorPicker::OnRecentSelect)
EVT_COMMAND(SELECTOR_DROPPER_PICK, wxDROPPER_SELECT, DialogColorPicker::OnRecentSelect)
EVT_MOUSE_EVENTS(DialogColorPicker::OnMouse)
END_EVENT_TABLE()



void DialogColorPicker::OnChangeRGB(wxCommandEvent &evt)
{
	if (!updating_controls)
		spectrum_dirty = true;
	UpdateFromRGB();
}


void DialogColorPicker::OnChangeHSL(wxCommandEvent &evt)
{
	if (!updating_controls)
		spectrum_dirty = true;
	UpdateFromHSL();
}


void DialogColorPicker::OnChangeHSV(wxCommandEvent &evt)
{
	if (!updating_controls)
		spectrum_dirty = true;
	UpdateFromHSV();
}


void DialogColorPicker::OnChangeASS(wxCommandEvent &evt)
{
	if (!updating_controls)
		spectrum_dirty = true;
	UpdateFromASS();
}


void DialogColorPicker::OnChangeHTML(wxCommandEvent &evt)
{
	if (!updating_controls)
		spectrum_dirty = true;
	UpdateFromHTML();
}

void DialogColorPicker::OnChangeAlpha(wxCommandEvent &evt)
{
	if (!updating_controls)
	{	//spectrum_dirty = true;

		cur_color = wxColour(cur_color.Red(), cur_color.Green(), cur_color.Blue(), 0xFF - alpha_input->GetInt());
		alphaslider->SetBackground(MakeAlphaSlider());
		alphaslider->SetXY(0, alpha_input->GetInt());
		//spectrum_dirty=false;
		if (IsShown()){
			updatecols.Start(100, true);
		}
	}
}

void DialogColorPicker::OnSpectrumChange(wxCommandEvent &evt)
{
	updating_controls = true;
	int x, y;
	spectrum->GetXY(x, y);
	hsv_input[1]->SetInt(x);
	hsv_input[2]->SetInt(y);
	updating_controls = false;
	UpdateFromHSV();
}


void DialogColorPicker::OnSliderChange(wxCommandEvent &evt)
{
	spectrum_dirty = true;
	int x, y; // only y is used, x is garbage for this control
	slider->GetXY(x, y);

	hsv_input[0]->SetInt(y);
	UpdateFromHSV();
}

void DialogColorPicker::OnAlphaSliderChange(wxCommandEvent &evt)
{
	spectrum_dirty = true;
	int x, y; // only y is used, x is garbage for this control
	alphaslider->GetXY(x, y);

	alpha_input->SetInt(y);
	cur_color = wxColour(cur_color.Red(), cur_color.Green(), cur_color.Blue(), 0xFF - alpha_input->GetInt());
	if (IsShown()){
		updatecols.Start(100, true);
	}
}



void DialogColorPicker::OnRecentSelect(wxCommandEvent &evt)
{
	// The colour picked is stored in the event string
	// Allows this event handler to be shared by recent and dropper controls
	// Ugly hack?
	AssColor color;
	color.SetAss(evt.GetString());
	SetColor(color.GetWX());
}


void DialogColorPicker::OnDropperMouse(wxMouseEvent &evt)
{
	if (evt.LeftDown() && !screen_dropper_icon->HasCapture()) {

		screen_dropper_icon->SetCursor(wxCursor("eyedropper_cursor"));
		screen_dropper_icon->SetBitmap(wxNullBitmap);
		screen_dropper_icon->CaptureMouse();
		eyedropper_grab_point = evt.GetPosition();
		eyedropper_is_grabbed = false;
	}

	if (evt.LeftUp()) {
#define ABS(x) (x < 0 ? -x : x)
		wxPoint ptdiff = evt.GetPosition() - eyedropper_grab_point;
		bool release_now = eyedropper_is_grabbed || ABS(ptdiff.x) + ABS(ptdiff.y) > 7;
		if (release_now) {
			screen_dropper_icon->ReleaseMouse();
			eyedropper_is_grabbed = false;
			screen_dropper_icon->SetCursor(wxNullCursor);
			screen_dropper_icon->SetBitmap(eyedropper_bitmap);
		}
		else {
			eyedropper_is_grabbed = true;
		}
	}

	if (screen_dropper_icon->HasCapture()) {
		wxPoint scrpos = screen_dropper_icon->ClientToScreen(evt.GetPosition());
		screen_dropper->DropFromScreenXY(scrpos.x, scrpos.y);
		if (evt.RightUp()){//we do not have access to resx and resy, it's 7, after change needs to be fix
			screen_dropper->SendGetColorEvent(3, 3);
			screen_dropper_icon->ReleaseMouse();
			eyedropper_is_grabbed = false;
			screen_dropper_icon->SetCursor(wxNullCursor);
			screen_dropper_icon->SetBitmap(eyedropper_bitmap);
		}
	}
}



/// @brief Hack to redirect events to the screen dropper icon
/// @param evt 
///
void DialogColorPicker::OnMouse(wxMouseEvent &evt)
{
	if (screen_dropper_icon->HasCapture()) {
		wxPoint dropper_pos = screen_dropper_icon->ScreenToClient(ClientToScreen(evt.GetPosition()));
		evt.m_x = dropper_pos.x;
		evt.m_y = dropper_pos.y;
		screen_dropper_icon->GetEventHandler()->ProcessEvent(evt);
	}
	else
		evt.Skip();
}




DialogColorPicker *DialogColorPicker::DCP = NULL;

DialogColorPicker * DialogColorPicker::Get(wxWindow *parent, AssColor color /*= wxColour("#000000")*/, int colorType /*= -1*/)
{
	int x = -1, y = -1;
	if (DCP && parent != DCP->GetParent()){
		DCP->GetPosition(&x, &y);
		DCP->Destroy(); DCP = NULL;
	}
	if (!DCP){
		DCP = new DialogColorPicker(parent, color, colorType);
		if (x != -1 || y != -1){ DCP->SetPosition(wxPoint(x, y)); }
	}
	else{
		DCP->SetColor(color, colorType);
	}
	return DCP;
}
void DialogColorPicker::OnColourChanged(wxTimerEvent &event)
{
	AssColor akol;
	akol.SetWX(cur_color, 0xFF - cur_color.Alpha());
	ColorEvent evt(COLOR_CHANGED, GetId(), akol, colorType->GetSelection() + 1);
	AddPendingEvent(evt);
}


ButtonColorPicker::ButtonColorPicker(wxWindow *parent, AssColor _color, wxSize size)
	: MappedButton(parent, -1, "", "", wxDefaultPosition, size, -1)
	, ActualColor(_color)
{
	SetBackgroundColour(_color.GetWX());
	Connect(wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&ButtonColorPicker::OnClick);
}

AssColor ButtonColorPicker::GetColor()
{
	return ActualColor;
}

void ButtonColorPicker::SetColor(const AssColor& color)
{
	ActualColor = color;
}

bool ButtonColorPicker::SetBackgroundColour(const wxColour& color)
{
	ActualColor = AssColor(color);
	MappedButton::SetBackgroundColour(color);
	return true;
}

void ButtonColorPicker::OnClick(wxCommandEvent &event)
{
	DialogColorPicker *dcp = DialogColorPicker::Get(this, ActualColor);
	wxPoint mst = wxGetMousePosition();
	wxSize siz = dcp->GetSize();
	siz.x;
	wxRect rc = wxGetClientDisplayRect();
	mst.x -= (siz.x / 2);
	mst.x = MID(rc.x, mst.x, rc.width - siz.x);
	mst.y += 15;
	mst.y = MID(rc.y, mst.y, rc.height - siz.y);
	dcp->Move(mst);
	if (dcp->ShowModal() == wxID_OK) {
		ActualColor = dcp->GetColor();
		SetBackgroundColour(ActualColor.GetWX());
	}
}

//TextColorPicker::TextColorPicker(wxWindow *parent, int id, const AssColor &_color, const wxPoint &pos, const wxSize &size, int style)
//	:wxWindow(parent, id, pos, size, style)
//{
//	color = _color;
//	wxSize newSize = size;
//	SetFont(parent->GetFont());
//	int fw, fh;
//	GetTextExtent(color.GetHex(true), &fw, &fh);
//	if (size.x < 1){
//		newSize.x = fw + 16;
//		if (newSize.x < 60){ newSize.x = 60; }
//	}
//	if (size.y < 1){
//		newSize.y = fh + 6;
//	}
//	SetMinSize(newSize);
//	//SetBestSize(newSize);
//	Bind(wxEVT_LEFT_DCLICK, &TextColorPicker::OnDoubleClick, this);
//	Bind(wxEVT_SIZE, &TextColorPicker::OnSize, this);
//	Bind(wxEVT_PAINT, &TextColorPicker::OnPaint, this);
//	Bind(wxEVT_ERASE_BACKGROUND, &TextColorPicker::OnErase, this);
//	SetFont(parent->GetFont());
//	SetBackgroundColour(parent->GetBackgroundColour());
//}
//
//AssColor TextColorPicker::GetColor()
//{
//	return color;
//}
//
//void TextColorPicker::SetColor(const AssColor& _color)
//{
//	color = _color;
//}
//
//
//void TextColorPicker::OnDoubleClick(wxMouseEvent &evt)
//{
//	DialogColorPicker *dcp = DialogColorPicker::Get(this, color);
//	wxPoint mst = wxGetMousePosition();
//	wxSize siz = dcp->GetSize();
//	siz.x;
//	wxRect rc = wxGetClientDisplayRect();
//	mst.x -= (siz.x / 2);
//	mst.x = MID(rc.x, mst.x, rc.width - siz.x);
//	mst.y += 15;
//	mst.y = MID(rc.y, mst.y, rc.height - siz.y);
//	dcp->Move(mst);
//	if (dcp->ShowModal() == wxID_OK) {
//		color = dcp->GetColor();
//		Refresh(false);
//	}
//}
//
//void TextColorPicker::OnPaint(wxPaintEvent &evt)
//{
//	int w = 0;
//	int h = 0;
//	GetClientSize(&w, &h);
//	if (w == 0 || h == 0){ return; }
//	wxMemoryDC tdc;
//	tdc.SelectObject(wxBitmap(w, h));
//	tdc.SetFont(GetFont());
//	wxColour background = GetBackgroundColour();
//	tdc.SetBrush(wxBrush(background));
//	tdc.SetPen(wxPen(background));
//	tdc.DrawRectangle(0, 0, w, h);
//
//	if (color.a){
//		const wxColour & col1 = Options.GetColour(StylePreviewColor1);
//		const wxColour & col2 = Options.GetColour(StylePreviewColor2);
//		int r2 = color.r, g2 = color.g, b2 = color.b;
//		int r = col1.Red(), g = col1.Green(), b = col1.Blue();
//		int r1 = col2.Red(), g1 = col2.Green(), b1 = col2.Blue();
//		int inv_a = 0xFF - color.a;
//		int fr = (r2* inv_a / 0xFF) + (r - inv_a * r / 0xFF);
//		int fg = (g2* inv_a / 0xFF) + (g - inv_a * g / 0xFF);
//		int fb = (b2* inv_a / 0xFF) + (b - inv_a * b / 0xFF);
//		wxColour firstMask(fr, fg, fb);
//		fr = (r2* inv_a / 0xFF) + (r1 - inv_a * r1 / 0xFF);
//		fg = (g2* inv_a / 0xFF) + (g1 - inv_a * g1 / 0xFF);
//		fb = (b2* inv_a / 0xFF) + (b1 - inv_a * b1 / 0xFF);
//		wxColour secondMask(fr, fg, fb);
//		int squareSize = (h - 3) / 4;
//		for (int i = 0; i < 4; i++){
//			for (int j = 0; j < 4; j++){
//				tdc.SetBrush(((i + j) % 2 == 0) ? firstMask : secondMask);
//				tdc.SetPen(*wxTRANSPARENT_PEN);
//				tdc.DrawRectangle(2 + (i*squareSize), 2 + (j*squareSize), squareSize, squareSize);
//			}
//		}
//	}
//	tdc.SetBrush(wxBrush(color.a ? *wxTRANSPARENT_BRUSH : color.GetWX()));
//	tdc.SetPen(wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT)));
//	tdc.DrawRectangle(1, 1, h - 3, h - 3);
//
//	int fw, fh;
//	wxString hextext = color.GetHex(true);
//	GetTextExtent(hextext, &fw, &fh);
//	tdc.SetTextForeground(GetForegroundColour());
//	tdc.DrawText(hextext, h + 2, (h - fh) / 2);
//
//	wxPaintDC dc(this);
//	dc.Blit(0, 0, w, h, &tdc, 0, 0);
//}
//
//void TextColorPicker::OnSize(wxSizeEvent &evt)
//{
//	Refresh(false);
//}

SimpleColorPickerDialog::SimpleColorPickerDialog(wxWindow *parent, const AssColor &actualColor, int colorNum)
	: KaiDialog(parent, -1, _("Próbnik koloru"))
	, color(actualColor)
{
	DialogSizer *ds = new DialogSizer(wxVERTICAL);
	wxString types[] = { _("Kolor podstawowy"), _("Kolor zastępczy"), _("Kolor obwódki"), _("Kolor cienia") };
	colorType = new KaiChoice(this, 9764, wxDefaultPosition, wxDefaultSize, 4, types);
	if (colorNum == -1)
		colorType->Enable(false);
	else
		colorType->SetSelection(colorNum - 1);
	Bind(wxEVT_COMMAND_CHOICE_SELECTED, [=](wxCommandEvent &evt){
		wxCommandEvent ctcevt(COLOR_TYPE_CHANGED, GetId());
		//selections starts from 0, colors from 1
		ctcevt.SetInt(colorType->GetSelection() + 1);
		ProcessEvent(ctcevt);
	}, 9764);

	HexColor = new KaiTextCtrl(this, -1, actualColor.GetAss(false, false));
	dropper = new ColorPickerScreenDropper(this, 9765, 7, 7, 8, false);
	dropper->Bind(wxEVT_MOUSE_CAPTURE_LOST, [=](wxMouseCaptureLostEvent &evt){ ReleaseMouse(); });
	Bind(wxDROPPER_MOUSE_UP, [=](wxCommandEvent &evt){
		if (moveWindowToMousePosition->GetValue())
			MoveToMousePosition(this);
	}, 9765);
	Bind(wxDROPPER_SELECT, [=](wxCommandEvent &evt){
		wxString stringColor = evt.GetString();
		color.Copy(stringColor);
		HexColor->SetValue(stringColor);
		Colorize();
		ColorEvent colorevt(COLOR_CHANGED, GetId(), color, colorType->GetSelection() + 1);
		AddPendingEvent(colorevt);
	}, 9765);
	moveWindowToMousePosition = new KaiCheckBox(this, -1, _("Przenoś okno\nw miejsce wyboru koloru"));
	moveWindowToMousePosition->SetValue(true);
	wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
	MappedButton *OK = new MappedButton(this, wxID_OK, "OK");
	MappedButton *cancel = new MappedButton(this, wxID_CANCEL, _("Anuluj"));
	ds->Add(colorType, 0, wxALL | wxEXPAND, 4);
	ds->Add(HexColor, 0, wxLEFT | wxRIGHT | wxEXPAND, 4);
	ds->Add(dropper, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 4);
	ds->Add(moveWindowToMousePosition, 0, wxLEFT | wxRIGHT, 4);
	buttonSizer->Add(OK, 1, wxALL, 2);
	buttonSizer->Add(cancel, 1, wxALL, 2);
	ds->Add(buttonSizer, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 2);
	SetSizerAndFit(ds);
	MoveToMousePosition(this);
	Colorize();
	//dropper->CaptureMouse();
	Bind(wxEVT_IDLE, &SimpleColorPickerDialog::OnIdle, this);
}

void SimpleColorPickerDialog::SetColor(const AssColor &_color)
{
	color = _color;
	HexColor->SetValue(color.GetAss(false));
	Colorize();
}

void SimpleColorPickerDialog::Colorize()
{

	int result = (color.r > 127) + (color.g > 127) + (color.b > 127);
	int kols = (result < 2) ? 255 : 0;
	HexColor->SetForegroundColour(wxColour(kols, kols, kols));
	HexColor->SetBackgroundColour(color.GetWX());
}

void SimpleColorPickerDialog::OnIdle(wxIdleEvent& event)
{
	event.Skip();

	if (IsShown())
	{
		wxPoint pos = ScreenToClient(wxGetMousePosition());
		wxRect rect(GetSize());

		if (rect.Contains(pos))
		{
			if (dropper->HasCapture())
			{
				dropper->ReleaseMouse();
				dropper->SetCursor(wxCURSOR_ARROW);
			}
		}
		else
		{
			if (!dropper->HasCapture())
			{
				dropper->CaptureMouse();
				dropper->SetCursor(wxCursor("eyedropper_cursor"));
			}
		}
	}
}

SimpleColorPicker::SimpleColorPicker(wxWindow *parent, const AssColor &actualColor, int colorType)
{
	scpd = new SimpleColorPickerDialog(parent, actualColor, colorType);
}

SimpleColorPicker::~SimpleColorPicker()
{
	scpd->Destroy();
}

bool SimpleColorPicker::PickColor(AssColor *returnColor)
{
	if (scpd->ShowModal() == wxID_OK){
		if (!returnColor)
			*returnColor = scpd->GetColor();
		return true;
	}
	return false;
}
