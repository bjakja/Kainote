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

#ifndef _KAILISTCTRL_
#define _KAILISTCTRL_

#include <wx/wx.h>
#include <vector>
#include "KaiScrollbar.h"
#include "Styles.h"

enum{
	TYPE_TEXT,
	TYPE_CHECKBOX,
	TYPE_COLOR
};

class Item{
public:
	Item(byte _type=TYPE_TEXT){type=_type;modified=false;}
	virtual ~Item(){	
	}
	virtual void OnMouseEvent(wxMouseEvent &event, bool enter, bool leave, wxWindow *theList){};
	virtual void OnPaint(wxMemoryDC *dc, int x, int y, int width, int height, wxWindow *theList){};
	bool modified;
	byte type;
};

class ItemText : public Item{
public:
	ItemText(const wxString &txt) : Item(){name = txt;}
	virtual ~ItemText(){		
	}
	void OnMouseEvent(wxMouseEvent &event, bool enter, bool leave, wxWindow *theList){};
	void OnPaint(wxMemoryDC *dc, int x, int y, int width, int height, wxWindow *theList);
	wxString GetName(){return name;}
	wxString name;
};

class ItemColor : public Item{
public:
	ItemColor(const AssColor &color) : Item(TYPE_COLOR){col = color;}
	virtual ~ItemColor(){	
	}
	void OnMouseEvent(wxMouseEvent &event, bool enter, bool leave, wxWindow *theList);
	void OnPaint(wxMemoryDC *dc, int x, int y, int width, int height, wxWindow *theList);
	AssColor col;
};

class ItemCheckBox : public Item{
public:
	ItemCheckBox(bool check, const wxString &_label) : Item(TYPE_CHECKBOX){check = checked; enter=false; label = _label;}
	virtual ~ItemCheckBox(){}
	void OnMouseEvent(wxMouseEvent &event, bool enter, bool leave, wxWindow *theList){};
	void OnPaint(wxMemoryDC *dc, int x, int y, int width, int height, wxWindow *theList);
	bool checked;
	bool enter;
	wxString label;
};

class ItemRow {
public:
	void Insert(size_t col, Item *item, byte _type=TYPE_TEXT){
		if(col >= row.size()){
			row.push_back(item);
			return;
		}
		row.insert(row.begin()+col, item);
	}
	ItemRow(size_t col, Item *item, byte _type=TYPE_TEXT){row.push_back(item);}
	ItemRow(){};
	~ItemRow(){
		for(auto it = row.begin(); it != row.end(); it++){
			delete *it;
		}
		row.clear();
	}
	std::vector< Item*> row;

};


class KaiListCtrl : public KaiScrolledWindow
{
public:
	KaiListCtrl(wxWindow *parent, int id, const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize, int style = 0);
	virtual ~KaiListCtrl(){
		for(auto it = itemList.begin(); it != itemList.end(); it++){
			delete *it;
		}
		itemList.clear();
		if(bmp){delete bmp;}
	};
	int InsertCollumn(size_t col, const wxString &name, byte type, int width);
	int AppendItem(Item *item); 
	int SetItem(size_t row, size_t col, Item *item); 
	Item *GetItem(size_t row, size_t col) const;
private:
	void OnSize(wxSizeEvent& evt);
	void OnPaint(wxPaintEvent& evt);
	void OnMouseEvent(wxMouseEvent &evt);
	void OnScroll(wxScrollWinEvent& event);
	//void OnMouseLost(wxMouseCaptureLostEvent &evt){if(HasCapture()){ReleaseMouse();} /*holding=false; */}
	void OnEraseBackground(wxEraseEvent &evt){};
	int GetMaxWidth();
	ItemRow header;
	std::vector< ItemRow*> itemList;
	wxArrayInt widths;
	wxBitmap *bmp;
	int sel;
	int lastSelX;
	int lastSelY;
	int scPosV;
	int scPosH;
	int lineHeight;
	int headerHeight;

	DECLARE_EVENT_TABLE()
};


#endif