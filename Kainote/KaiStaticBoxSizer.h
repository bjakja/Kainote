//  Copyright (c) 2016 - 2026, Marcin Drob

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

#include <wx/window.h>
#include <wx/statbox.h>
#include <wx/sizer.h>
#include <wx/dc.h>

struct tagRECT;


class KaiStaticBox : public
#ifdef _WIN32
	wxStaticBox
#else
	wxWindow
#endif
{
public:
	KaiStaticBox(wxWindow *parent, const wxString& label);
	KaiStaticBox(wxWindow *parent, int numLabels, wxString* labels);
	virtual ~KaiStaticBox(){};
	wxSize CalcBorders();
	bool Enable(bool enable = true);
private:
	void PaintForeground(wxDC& dc, const tagRECT& rc);
	wxArrayString labels;
	int heightText;
};

class KaiStaticBoxSizer : public wxBoxSizer
{
public:
	KaiStaticBoxSizer(int orient, wxWindow *parent, const wxString& _label);
	KaiStaticBoxSizer(int orient, wxWindow *parent, int n, wxString * _labels);
	virtual ~KaiStaticBoxSizer();
	void ShowItems( bool show ) override;
	bool Enable(bool enable);
	// wxWindowBase, not wxWindow: with the old signature this stopped
	// overriding wxSizer::Detach in wx 3.3, so ~wxWindowBase never cleared box
	// and the destructor below deleted it a second time.
	bool Detach( wxWindowBase *window ) override;

private:
	// wxBoxSizer overrides RepositionChildren(), so its RecalcSizes()
	// compatibility shim never runs and an override of it is dead code.
	void RepositionChildren(const wxSize& minSize) override;
	wxSize CalcMin() override;
	KaiStaticBox *box;
};
