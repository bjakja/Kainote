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
#include <wx/bitmap.h>
#include <wx/window.h>
#include <vector>
#include "ListControls.h"
#include "KaiCheckBox.h"
#include "MappedButton.h"
#include "VisualAllTagsEdition.h"

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
	virtual void HideContols() {};
	virtual void ShowContols(VideoToolbar* vt) {};

	int startIconNumber;
	bool clicked = false;
	int selection = -1;
};

class MoveAllItem : public VisualItem
{
public:
	MoveAllItem() : VisualItem() { 
		startIconNumber = 19; 
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
	VectorItem(bool isClip) : VisualItem() { 
		startIconNumber = 12; 
		if (!isClip)
			numIcons = 6;
	};
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
	int numIcons = 7;
	int toggled = 1;
	bool isNormalButton = false;
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

class AllTagsItem : public VisualItem
{
public:
	AllTagsItem();
	void OnMouseEvent(wxMouseEvent& evt, int w, int h, VideoToolbar* vt) override;
	void OnPaint(wxDC& dc, int w, int h, VideoToolbar* vt) override;
	void Synchronize(VisualItem* item) override;
	int GetItemToggled() override;
	void SetItemToggled(int* item) override;
	void HideContols() override;
	void ShowContols(VideoToolbar* vt) override;
private:
	KaiChoice* tagList = NULL;
	//KaiCheckBox* addToExist = NULL;
	MappedButton* edition = NULL;
	int maxWidth = -1;
	int selection = 0;
	//int mode = 0;
	enum {
		ID_TAG_LIST = 12378,
		ID_EDITION
	};
};

class RotationZItem :public VisualItem
{
public:
	RotationZItem() {
		startIconNumber = 25; 
	};
	void OnMouseEvent(wxMouseEvent& evt, int w, int h, VideoToolbar* vt) override;
	void OnPaint(wxDC& dc, int w, int h, VideoToolbar* vt) override;
	void Synchronize(VisualItem* item) override;
	int GetItemToggled() override { return toggled; };
	void SetItemToggled(int* item) override;
private:
	int numIcons = 1;
	int toggled = -1;
};

class ScaleItem :public VisualItem
{
public:
	ScaleItem() {
		startIconNumber = 26;
	};
	void OnMouseEvent(wxMouseEvent& evt, int w, int h, VideoToolbar* vt) override;
	void OnPaint(wxDC& dc, int w, int h, VideoToolbar* vt) override;
	void Synchronize(VisualItem* item) override;
	int GetItemToggled() override { 
		int result = 0;
		for (int i = 0; i < numIcons; i++) {
			if (Toggled[i]) { result |= (1 << i); }
		}
		return result;
	};
	void SetItemToggled(int* item) override;
private:
	static const int numIcons = 5;
	bool Toggled[numIcons] = { false, true, true, false, false };
};


class VideoToolbar: public wxWindow {
public:
	VideoToolbar (wxWindow *parent, const wxPoint &pos, const wxSize &size);
	virtual ~VideoToolbar(){
		for (auto cur = visualItems.begin(); cur != visualItems.end(); cur++){
			if(*cur)
				delete (*cur);
		}
		if (bmp){ 
			delete bmp; 
			bmp = NULL; 
		}
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
	static std::vector<AllTagsSetting>* GetTagsSettings() {
		if (!tags.size()) {
			LoadSettings(&tags);
		}
		return &tags;
	}
	static void SetTagsSettings(std::vector<AllTagsSetting>* _tags) {
		tags = *_tags;
	}
	void DisableVisuals(bool Disable){ 
		iconsEnabled = !Disable; 
		Toggled = 0;
		sel = -1;
		clicked = false;
		blockScroll = false;
		Refresh(false); 
	}
	bool IsVisualsDisabled() {
		return !iconsEnabled;
	}
	int GetStartDrawPos(){
		return startDrawPos;
	}
	bool SetFont(const wxFont &font);
	void SetHeight(int height);
	int GetEndDrawPos() { return endDrawPos; };
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
	const static int toolsSize = 12;
	bool clicked;
	bool iconsEnabled = true;
	bool hasHiddenLists = false;
	bool insufficentPlace = false;
	wxBitmap *bmp;
	std::vector<VisualItem*> visualItems;
	static std::vector<AllTagsSetting> tags;
	int startDrawPos = 2;
	int endDrawPos = 162;
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
	ID_VECTOR_TOOLBAR_NORMAL_BUTTON,
	ID_MOVE_TOOLBAR_EVENT,
	ID_SCALE_ROTATE_TOOLBAR_EVENT,
	ID_SEEK_AFTER=22222,
	ID_PLAY_AFTER
};

