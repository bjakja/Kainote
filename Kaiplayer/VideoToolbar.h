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
#include <vector>
#include "ListControls.h"

class itemdata{
public:
	itemdata(wxBitmap *_icon, const wxString& _help){icon=_icon; help=_help;}
	~itemdata(){delete icon;}

	wxBitmap *icon;
	wxString help;
};

class VideoToolbar;

class VisualItem
{
public:
	VisualItem(){};
	virtual void OnMouseEvent(wxMouseEvent &evt, int w, int h, VideoToolbar *vt){};
	virtual void OnPaint(wxDC &dc, int w, int h, VideoToolbar *vt){};
	virtual void Synchronize(VisualItem * item){};
	virtual int GetItemToggled(){ return 0; };
	virtual void SetItemToggled(int *item){};

	int startIconNumber;
	bool clicked = false;
	int selection = -1;
};

class MoveAllItem : public VisualItem
{
public:
	MoveAllItem() : VisualItem() { 
		startIconNumber = 17; 
		MoveToggled[0] = true;
		for (int i = 1; i < numMoveIcons; i++){
			MoveToggled[i] = false;
		}
	};
	void OnMouseEvent(wxMouseEvent &evt, int w, int h, VideoToolbar *vt);
	void OnPaint(wxDC &dc, int w, int h, VideoToolbar *vt);
	void Synchronize(VisualItem * item){
		MoveAllItem *mai = (MoveAllItem*)item;
		for (int i = 0; i < numMoveIcons; i++)
			MoveToggled[i] = mai->MoveToggled[i];
	};
	int GetItemToggled(){
		int result = 0;
		for (int i = 0; i < numMoveIcons; i++){
			if (MoveToggled[i]){ result |= 1 << i; }
		}
		return result;
	};
	//virtual void SetItemToggled(int item){};
private:
	static const int numMoveIcons = 6;
	bool MoveToggled[numMoveIcons];
};

class VectorItem : public VisualItem
{
public:
	VectorItem() : VisualItem() { startIconNumber = 11; };
	void OnMouseEvent(wxMouseEvent &evt, int w, int h, VideoToolbar *vt);
	void OnPaint(wxDC &dc, int w, int h, VideoToolbar *vt);
	void Synchronize(VisualItem * item){
		VectorItem *ci = (VectorItem*)item;
		toggled = ci->toggled;
	};
	int GetItemToggled(){ return toggled; };
	void SetItemToggled(int *item){ 
		toggled = *item;
		if (toggled < 0)
			toggled = (*item) = numIcons - 1;
		else if (toggled >= numIcons)
			toggled = (*item) = 0;
	};
	int numIcons = 6;
	int toggled = 1;
};

class ScaleRotationItem : public VisualItem
{
public:
	ScaleRotationItem() : VisualItem() { startIconNumber = 3; };
	void OnMouseEvent(wxMouseEvent &evt, int w, int h, VideoToolbar *vt);
	void OnPaint(wxDC &dc, int w, int h, VideoToolbar *vt);
	void Synchronize(VisualItem * item){
		ScaleRotationItem *sri = (ScaleRotationItem*)item;
		toggled = sri->toggled;
	};
	int GetItemToggled(){ return toggled; };
	void SetItemToggled(int *item){ 
		toggled = (*item); 
		if (toggled < 0)
			toggled = (*item) = numIcons - 1;
		else if (toggled >= numIcons)
			toggled = (*item) = 0;
	};
	int numIcons = 3;
	int toggled = 0;
};

class VideoToolbar: public wxWindow {
public:
	VideoToolbar (wxWindow *parent, const wxPoint &pos, const wxSize &size);
	virtual ~VideoToolbar(){
		for (auto cur = visualItems.begin(); cur != visualItems.end(); cur++){
			delete (*cur);
		}
		if (bmp){ delete bmp; bmp = NULL; }
	};

	int GetToggled();
	int GetItemToggled();
	//item change toggled automatically when less than 0 or greater than size
	void SetItemToggled(int *toggled);
	void Synchronize(VideoToolbar *vtoolbar);
	static void DestroyIcons(){
		for(auto cur = icons.begin(); cur != icons.end(); cur++){
			delete (*cur);
		}
	}
	void DisableVisuals(bool Disable){ 
		iconsEnabled = !Disable; 
		Toggled = 0;
		sel = -1;
		clicked = false;
		blockScroll = false;
		Refresh(false); 
	}
	int GetStartDrawPos(){
		return startDrawPos;
	}
	bool SetFont(const wxFont &font);
	KaiChoice *videoSeekAfter;
	KaiChoice *videoPlayAfter;
	static std::vector<itemdata*> icons;
	bool blockScroll;
private:
	
	void OnMouseEvent(wxMouseEvent &evt);
	void OnPaint(wxPaintEvent &evt);
	void OnSize(wxSizeEvent &evt);
	int Toggled;
	int sel;
	const static int toolsSize = 11;
	bool clicked;
	bool iconsEnabled = true;
	wxBitmap *bmp;
	std::vector<VisualItem*> visualItems;
	int startDrawPos = 146;
};

enum{
	BUTTON_POS=1,
	BUTTON_MOVE_START=2,
	BUTTON_MOVE_END=4,
	BUTTON_CLIP=8,
	BUTTON_DRAW=16,
	BUTTON_ORG=32,
	ID_VIDEO_TOOLBAR_EVENT=21909,
	ID_VECTOR_TOOLBAR_EVENT,
	ID_MOVE_TOOLBAR_EVENT,
	ID_SCALE_ROTATE_TOOLBAR_EVENT,
	ID_SEEK_AFTER=22222,
	ID_PLAY_AFTER
};

