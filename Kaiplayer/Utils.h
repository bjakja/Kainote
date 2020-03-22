//  Copyright (c) 2017 - 2020, Marcin Drob

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

#include <wx/colour.h>
#include <wx/string.h>
#include <wx/window.h>

inline wxColour GetColorWithAlpha(const wxColour &colorWithAlpha, const wxColour &background)
{
	int r = colorWithAlpha.Red(), g = colorWithAlpha.Green(), b = colorWithAlpha.Blue();
	int r2 = background.Red(), g2 = background.Green(), b2 = background.Blue();
	int inv_a = 0xFF - colorWithAlpha.Alpha();
	int fr = (r2* inv_a / 0xFF) + (r - inv_a * r / 0xFF);
	int fg = (g2* inv_a / 0xFF) + (g - inv_a * g / 0xFF);
	int fb = (b2* inv_a / 0xFF) + (b - inv_a * b / 0xFF);
	return wxColour(fr, fg, fb);
}

inline wxString GetTruncateText(const wxString &textToTruncate, int width, wxWindow *window)
{
	int w, h;
	window->GetTextExtent(textToTruncate, &w, &h);
	if (w > width){
		int len = textToTruncate.Len() - 1;
		while (w > width && len > 3){
			window->GetTextExtent(textToTruncate.SubString(0, len), &w, &h);
			len--;
		}
		return textToTruncate.SubString(0, len - 2) + L"...";
	}
	return textToTruncate;
}

void SelectInFolder(const wxString & filename);

void OpenInBrowser(const wxString &adress);
