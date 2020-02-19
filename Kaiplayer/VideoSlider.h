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

#include <wx/wx.h>

class VideoCtrl;

class VideoSlider : public wxWindow
{
public:
	VideoSlider(wxWindow *parent, const long int id ,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize, long style=wxWANTS_CHARS, const wxString& name=wxPanelNameStr);
	virtual ~VideoSlider();

	void SetValue(float pos);
	VideoCtrl* VB;
	void SendTime(int msTimePos);
protected:
	void OnPaint(wxPaintEvent& event);
	void OnMouseEvent(wxMouseEvent& event);
	void OnMouseLeave(wxMouseCaptureLostEvent& event);
	void OnKeyPress(wxKeyEvent& event);
	void OnSize(wxSizeEvent& event);
	void OnEraseBackground(wxEraseEvent& rEvent){}
	int position = 0;
	int msTimePosition = 0;
	int labelpos;
	int chapterTime = 0;
	int chapterPos = 0;
	int positionDiff = 0;
	wxString label;
	bool isChapter = false;
	bool showlabel;
	bool block;
	bool holding;
	bool onslider;
	wxBitmap prb;
	wxBitmap prbh;
	//wxTimer videoSeekTimer;
	DECLARE_EVENT_TABLE()
};

class VolSlider : public wxWindow
	{
public:
	VolSlider(wxWindow *parent, const long int id, int apos, const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize, long style=wxWANTS_CHARS, const wxString& name=wxPanelNameStr);
	virtual ~VolSlider();

	void SetValue(int pos);
	int GetValue();
	void OnMouseEvent(wxMouseEvent& event);
protected:
	void OnPaint(wxPaintEvent& event);
	void OnEraseBackground(wxEraseEvent& rEvent){}
	int position;
	bool block;
	bool holding;
	bool onslider;
	wxBitmap start;
	wxBitmap end;
	wxBitmap prbh;
	wxBitmap* bmp;
	//bool blockpaint;
	DECLARE_EVENT_TABLE()
	};

