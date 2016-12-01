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


#include <wx/wx.h>
#include <vector>
#include "ListControls.h"

const static int toolsSize = 10;
const static int clipToolsSize = 6;
const static int moveToolsStart= toolsSize + clipToolsSize;
const static int moveToolsSize = 6;

class itemdata{
public:
	itemdata(wxBitmap *_icon, const wxString& _help){icon=_icon; help=_help;}
	~itemdata(){delete icon;}

	wxBitmap *icon;
	wxString help;
};

class VideoToolbar: public wxWindow {
public:
	VideoToolbar (wxWindow *parent, const wxPoint &pos);
	virtual ~VideoToolbar(){
		if(bmp)delete bmp; bmp=NULL;
	};

	int GetToggled();
	int GetClipToggled(){return clipToggled-Toggled;};
	int GetMoveToggled(){
		int result = 0;
		for(int i = 0; i < moveToolsSize; i++){
			if(MoveToggled[i]){ result |= 1<<i; }
		}
		return result;
	};
	void SetClipToggled(int newtool){ clipToggled = toolsSize + newtool; Refresh(false);};
	void Synchronize(VideoToolbar *vtoolbar);
	void ShowTools(bool show, bool IsClip){
		if(IsClip){clipToggled = toolsSize+1; showClipTools=show; showMoveTools=false;}
		else {showMoveTools=show; showClipTools=false;}
		Refresh(false);
	}
	//void ShowMoveTools(bool show){showMoveTools=show; Refresh(false);}
	bool ClipToolsShown(){return showClipTools;}
	bool MoveToolsShown(){return showMoveTools;}
	static void DestroyIcons(){
		for(auto cur = icons.begin(); cur != icons.end(); cur++){
			delete (*cur);
		}
	}
	KaiChoice *videoSeekAfter;
	KaiChoice *videoPlayAfter;
private:
	
	void OnMouseEvent(wxMouseEvent &evt);
	void OnPaint(wxPaintEvent &evt);
	void OnSize(wxSizeEvent &evt);
	int Toggled;
	int clipToggled;
	bool MoveToggled[moveToolsSize];
	int sel;
	bool clicked;
	bool showClipTools;
	bool showMoveTools;
	bool blockScroll;
	wxBitmap *bmp;
	static std::vector< itemdata*> icons;
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
	ID_SEEK_AFTER=22222,
	ID_PLAY_AFTER
};