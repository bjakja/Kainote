//  Copyright (c) 2017, Marcin Drob

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

#include <wx/toplevel.h>
#include <wx/evtloop.h>
#include <wx/sizer.h>

class DialogSizer : public wxBoxSizer
{
public:
	DialogSizer(int orient);
	virtual ~DialogSizer(){};
	void RecalcSizes();
	wxSize CalcMin();
};

#define wxDEFAULT_DIALOG_STYLE  (wxCAPTION | wxSYSTEM_MENU | wxCLOSE_BOX)

class KaiDialog : public wxTopLevelWindow
{
public:
	KaiDialog(wxWindow *parent, wxWindowID id,
             const wxString& title,
             const wxPoint& pos = wxDefaultPosition,
             const wxSize& size = wxDefaultSize,
             long _style = wxDEFAULT_DIALOG_STYLE);
	virtual ~KaiDialog();
	bool Show(bool show=true);
	bool Hide();
	int ShowModal();
    void EndModal(int retCode);
    bool IsModal() const;
	void SetEnterId(int _enterId);
	void SetEscapeId(int _escapeId, bool setFocus=false);
    int GetEnterId() const { return enterId; }
	int GetEscapeId() const { return escapeId; }
	void SetSizerAndFit1(wxSizer *sizer, bool deleteOld = true);
	void SetLabel(const wxString &text);
private:
	bool IsButtonFocused();
	void OnCharHook(wxKeyEvent &evt);
	void OnPaint(wxPaintEvent &evt);
	void OnSize(wxSizeEvent &evt);
	void OnMouseEvent(wxMouseEvent &evt);
	//void OnActivate(wxActivateEvent &evt);
	void OnEnter(wxCommandEvent &evt);
	void OnEscape(wxCommandEvent &evt);
	WXLRESULT MSWWindowProc(WXUINT message, WXWPARAM wParam, WXLPARAM lParam);

	int escapeId;
	int enterId;
	long style;
	wxPoint diff;
	bool enter;
	bool pushed;
	bool isActive;
	bool setEscapeIdWithFocus = false;
	wxModalEventLoop *loop;
	wxDECLARE_ABSTRACT_CLASS(KaiDialog);
};
