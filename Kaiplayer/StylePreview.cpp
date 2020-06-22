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


#include "StylePreview.h"
#include "config.h"


StylePreview::StylePreview(wxWindow *parent, int id, const wxPoint& pos, const wxSize& size)
	: wxWindow(parent, id, pos, size)
{
	previewStyle = NULL;
	m_SubtitlesProvider = new SubtitlesProviderManager();
	Bind(wxEVT_SIZE, [=](wxSizeEvent &evt){ DrawPreview(0); });
	Bind(wxEVT_ERASE_BACKGROUND, [=](wxEraseEvent &evt){});
}
StylePreview::~StylePreview()
{
	SAFE_DELETE(m_SubtitlesProvider);
	wxDELETE(previewStyle);
	wxDELETE(bmpframe);
}

void StylePreview::DrawPreview(Styles *style)
{
	wxMutexLocker lock(mutex);
	if (style){
		wxDELETE(previewStyle);
		previewStyle = style->Copy();
	}
	else if (!previewStyle){
		return;
	}
	
	wxDELETE(bmpframe);

	GetClientSize(&width, &height);
	pitch = width * 4;
	wxString *dat = new wxString();
	SubsText(dat);
	m_SubtitlesProvider->SetVideoParameters(wxSize(width, height), 4, false);
	m_SubtitlesProvider->OpenString(dat);

	const wxColour & kol1 = Options.GetColour(STYLE_PREVIEW_COLOR1);
	const wxColour & kol2 = Options.GetColour(STYLE_PREVIEW_COLOR2);
	b = kol1.Blue();
	g = kol1.Green();
	r = kol1.Red();

	b1 = kol2.Blue();
	g1 = kol2.Green();
	r1 = kol2.Red();

	unsigned char *data = new unsigned char[width * height * 4];


	bool ch = false;
	bool ch1 = false;
	for (int i = 0; i < height; i++)
	{
		if ((i % 10) == 0){ ch1 = !ch1; }
		ch = ch1;
		for (int j = 0; j < width; j++)
		{
			int k = ((i * width) + j) * 4;
			if ((j % 10) == 0 && j > 0){ ch = !ch; }
			data[k] = (ch) ? b : b1;
			data[k + 1] = (ch) ? g : g1;
			data[k + 2] = (ch) ? r : r1;
			data[k + 3] = 255;
		}

	}

	m_SubtitlesProvider->Draw(data, 1000);

	wxImage preview(width, height, true);
	unsigned char *data1 = (unsigned char *)malloc(width * height * 3);
	//int bb,gg,rr;
	for (int i = 0; i < (width * height); i++)
	{
		data1[i * 3] = data[(i * 4) + 2];
		data1[(i * 3) + 1] = data[(i * 4) + 1];
		data1[(i * 3) + 2] = data[i * 4];
	}
	preview.SetData(data1);

	bmpframe = new wxBitmap(preview);
	Refresh(false);
	delete[] data;
	data = 0;
}

void StylePreview::OnPaint(wxPaintEvent& event)
{
	if (!bmpframe) return;

	wxPaintDC dc(this);

	wxMemoryDC memdc;
	memdc.SelectObject(*bmpframe);
	dc.Blit(0, 0, bmpframe->GetWidth(), bmpframe->GetHeight(), &memdc, 0, 0);

}

void StylePreview::SubsText(wxString *text)
{
	previewStyle->Alignment = L"5";
	wchar_t bom = 0xFEFF;
	*text << wxString(bom);
	*text << L"[Script Info]\r\nPlayResX: " << width << L"\r\nPlayResY: " << height 
		<< L"\r\nScaledBorderAndShadow: Yes\r\nScriptType: v4.00+\r\nWrapStyle: 0"
		<< L"\r\n[V4+ Styles]\r\nFormat: Name, Fontname, Fontsize, PrimaryColour, SecondaryColour, OutlineColour, BackColour, Bold, Italic, Underline, StrikeOut, ScaleX, ScaleY, Spacing, Angle, BorderStyle, Outline, Shadow, Alignment, MarginL, MarginR, MarginV, Encoding\r\n"
		<< previewStyle->GetRaw() << L"\r\n \r\n[Events]\r\nFormat: Layer, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text\r\nDialogue: 0,0:00:00.00,1:01:26.00," 
		<< previewStyle->Name << L",,0000,0000,0000,," << Options.GetString(STYLE_PREVIEW_TEXT) << L"\r\n"
		/*Dialogue: 0,0:00:01.00,1:01:26.00,jakistamstyl,,0000,0000,0000,,Jakiśtam testowy tekst"*/;

	//previewStyle->Alignment = "5";
	//wchar_t bom = 0xFEFF;
	//*text << wxString(bom);
	//*text << "[Script Info]\r\nPlayResX: " << width << "\r\nPlayResY: " << height << "\r\nScaledBorderAndShadow: Yes\r\nScriptType: v4.00+\r\nWrapStyle: 0"
	//	<< "\r\n[V4+ Styles]\r\nFormat: Name, Fontname, Fontsize, PrimaryColour, SecondaryColour, OutlineColour, BackColour, Bold, Italic, Underline, StrikeOut, ScaleX, ScaleY, Spacing, Angle, BorderStyle, Outline, Shadow, Alignment, MarginL, MarginR, MarginV, Encoding\r\n"
	//	<< previewStyle->GetRaw() << "\r\n \r\n[Events]\r\nFormat: Layer, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text\r\nDialogue: 0,0:00:00.00,1:01:26.00," << previewStyle->Name << ",,0000,0000,0000,," << Options.GetString(STYLE_PREVIEW_TEXT) <<
	//	"\r\n"/*Dialogue: 0,0:00:01.00,1:01:26.00,jakistamstyl,,0000,0000,0000,,Jakiśtam testowy tekst"*/;
}

void StylePreview::OnMouseEvent(wxMouseEvent& event)
{
	if (event.LeftDown()){
		if (PrevText){
			Options.SetString(STYLE_PREVIEW_TEXT, PrevText->GetValue());
			DrawPreview();
			Options.SaveOptions();
			PrevText->Destroy();
			PrevText = NULL;
		}
		else{
			wxSize siz = GetClientSize();
			PrevText = new KaiTextCtrl(this, -1, Options.GetString(STYLE_PREVIEW_TEXT), wxPoint(0, 0), wxSize(siz.x, -1));
			PrevText->Show();
		}

	}
}

BEGIN_EVENT_TABLE(StylePreview, wxWindow)
EVT_PAINT(StylePreview::OnPaint)
EVT_MOUSE_EVENTS(StylePreview::OnMouseEvent)
END_EVENT_TABLE()
