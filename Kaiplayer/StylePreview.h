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

#pragma once


#include <wx/wx.h>
#include "Styles.h"
#include "KaiTextCtrl.h"
#include <vector>
#include "SubtitlesProviderManager.h"


class StylePreview : public wxWindow
	{
	public:

		StylePreview(wxWindow *parent, int id, const wxPoint& pos, const wxSize& size);
		virtual ~StylePreview();

		void DrawPreview(Styles *styl = NULL);



	private:
		wxMutex mutex;
		int pitch, width, height, b, g, r, b1, g1, r1;
		

		wxBitmap *bmpframe = NULL;
		KaiTextCtrl *PrevText;
		Styles *previewStyle;

		void OnPaint(wxPaintEvent& event);
		void SubsText(wxString *text);
		void OnMouseEvent(wxMouseEvent& event);

		DECLARE_EVENT_TABLE()

		SubtitlesProviderManager *m_SubtitlesProvider = NULL;
	};

