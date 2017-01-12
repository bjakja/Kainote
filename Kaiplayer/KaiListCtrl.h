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

wxDECLARE_EVENT(LIST_ITEM_DOUBLECLICKED, wxCommandEvent);
wxDECLARE_EVENT(LIST_ITEM_RIGHT_CLICK, wxCommandEvent);

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
	virtual void Save(){};
	bool modified;
	byte type;
	wxString name;
};

class ItemText : public Item{
public:
	ItemText(const wxString &txt) : Item(){name = txt;}
	virtual ~ItemText(){		
	}
	void OnMouseEvent(wxMouseEvent &event, bool enter, bool leave, wxWindow *theList){};
	void OnPaint(wxMemoryDC *dc, int x, int y, int width, int height, wxWindow *theList);
	wxString GetName(){return name;}
	void Save(){};
};

class ItemColor : public Item{
public:
	ItemColor(const AssColor &color, const wxString &_name) : Item(TYPE_COLOR){col = color; name = _name;}
	virtual ~ItemColor(){	
	}
	void OnMouseEvent(wxMouseEvent &event, bool enter, bool leave, wxWindow *theList);
	void OnPaint(wxMemoryDC *dc, int x, int y, int width, int height, wxWindow *theList);
	void Save();
	AssColor col;
};

class ItemCheckBox : public Item{
public:
	ItemCheckBox(bool check, const wxString &_label) : Item(TYPE_CHECKBOX){modified = check; enter=false; name = _label;}
	virtual ~ItemCheckBox(){}
	void OnMouseEvent(wxMouseEvent &event, bool enter, bool leave, wxWindow *theList);
	void OnPaint(wxMemoryDC *dc, int x, int y, int width, int height, wxWindow *theList);
	void Save(){};
	bool enter;
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
	//CheckboxList
	KaiListCtrl(wxWindow *parent, int id, int numelem = 0, wxString *list = 0, const wxPoint &pos = wxDefaultPosition, 
		const wxSize &size = wxDefaultSize, int style = 0);
	//nomralList
	KaiListCtrl(wxWindow *parent, int id, const wxArrayString &list, const wxPoint &pos = wxDefaultPosition, 
		const wxSize &size = wxDefaultSize, int style = 0);
	virtual ~KaiListCtrl(){
		for(auto it = itemList.begin(); it != itemList.end(); it++){
			delete *it;
		}
		itemList.clear();
		if(bmp){delete bmp;}
	};
	int InsertColumn(size_t col, const wxString &name, byte type, int width);
	int AppendItem(Item *item); 
	int SetItem(size_t row, size_t col, Item *item); 
	Item *GetItem(size_t row, size_t col) const;
	void SaveAll(int col);
	void SetModified(bool modif){modified = modif;}
	bool GetModified(){return modified;}
	int FindItem(int column, const wxString &textItem);
	void ScrollTo(int row);
	size_t GetCount(){return itemList.size();}
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
	bool modified;

	DECLARE_EVENT_TABLE()
	wxDECLARE_ABSTRACT_CLASS(KaiListCtrl);
};


#endif