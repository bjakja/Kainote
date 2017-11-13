//  Copyright (c) 2016, Marcin Drob

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

#include "SubsGridWindow.h"


class kainoteFrame;


class SubsGrid: public SubsGridWindow
{
public:

	SubsGrid(wxWindow* parent, kainoteFrame* kfparent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0);
	virtual ~SubsGrid();
	void MoveTextTL(char mode);
	void ResizeSubs(float xnsize, float ynsize, bool stretch);
	void OnMkvSubs(wxCommandEvent &event);
	void ConnectAcc(int id);
	void OnAccelerator(wxCommandEvent &event);
	void OnJoin(wxCommandEvent &event);
	void ContextMenu(const wxPoint &pos, bool dummy=false);
	bool SwapAssProperties(); 
	void RefreshSubsOnVideo(int newActiveLine, bool scroll = true);
	
protected:

    wxArrayInt selarr;

private:
	void CopyRows(int id);
    void OnInsertBefore();
	void OnInsertAfter();
	void OnDuplicate();
	void OnJoinF(int id);
	void OnPaste(int id);
	void OnPasteTextTl();
	void OnJoinToFirst(int id);
	void OnInsertBeforeVideo(bool frameTime = false);
	void OnInsertAfterVideo(bool frameTime = false);
	void OnSetFPSFromVideo();
	void OnSetNewFPS();
	void OnMakeContinous(int id);
	void Filter(int id);

	DECLARE_EVENT_TABLE()
};


