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


#include "DropFiles.h"
#include "kainoteFrame.h"
#include "Notebook.h"
#include <wx/event.h>

DragnDrop::DragnDrop(KainoteFrame* kfparent)
{
	Kai = kfparent;
	int timerId = 8989;
	timer.SetOwner(Kai, timerId);
	Kai->Bind(wxEVT_TIMER, [=](wxTimerEvent &evt) { OnDropTimer(evt); }, timerId);
}

bool DragnDrop::OnDropFiles(wxCoord posx, wxCoord posy, const wxArrayString& filenames)
{
	files = filenames;
	x = posx;
	y = posy;
	timer.Start(50, true);
	return true;
}

void DragnDrop::OnDropTimer(wxTimerEvent & evt)
{
	if (files.size() > 1) {
		Kai->OpenFiles(files);
	}
	else if (files.size() > 0) {
		wxString ext = files[0].AfterLast(L'.').Lower();
		bool isLuaScript = ext == L"lua" || ext == L"moon";
		int w, h;
		Kai->Tabs->GetClientSize(&w, &h);
		if (!isLuaScript) {
			if (y >= h - Kai->Tabs->GetHeight()) {
				int pixels;
				int tab = Kai->Tabs->FindTab(x, &pixels);
				if (tab < 0) { Kai->InsertTab(); }
				else if (Kai->Tabs->iter != tab) { Kai->Tabs->ChangePage(tab); }
			}
			else {
				int tabByPos = Kai->Tabs->GetIterByPos(wxPoint(x, y));
				if (Kai->Tabs->iter != tabByPos) { Kai->Tabs->ChangePage(tabByPos); }
			}
		}
		Kai->OpenFile(files[0]);
	}
}
