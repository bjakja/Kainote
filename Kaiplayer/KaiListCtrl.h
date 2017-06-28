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

class KaiListCtrl;

class Item{
public:
	Item(byte _type=TYPE_TEXT){type=_type;modified=false;}
	virtual ~Item(){	
	}
	virtual void OnMouseEvent(wxMouseEvent &event, bool enter, bool leave, KaiListCtrl *theList, Item **changed = NULL){};
	virtual void OnPaint(wxMemoryDC *dc, int x, int y, int width, int height, KaiListCtrl *theList){};
	virtual void Save(){};
	virtual Item* Copy(){return NULL;}
	bool modified;
	byte type;
	wxString name;
};

class ItemText : public Item{
public:
	ItemText(const wxString &txt) : Item(){name = txt;}
	virtual ~ItemText(){		
	}
	void OnMouseEvent(wxMouseEvent &event, bool enter, bool leave, KaiListCtrl *theList, Item **changed = NULL){};
	void OnPaint(wxMemoryDC *dc, int x, int y, int width, int height, KaiListCtrl *theList);
	wxString GetName(){return name;}
	void Save(){};
	Item* Copy(){return new ItemText(*this);}
};

class ItemColor : public Item{
public:
	ItemColor(const AssColor &color, int i) : Item(TYPE_COLOR){col = color;colOptNum=i;}
	virtual ~ItemColor(){	
	}
	void OnMouseEvent(wxMouseEvent &event, bool enter, bool leave, KaiListCtrl *theList, Item **changed = NULL);
	void OnPaint(wxMemoryDC *dc, int x, int y, int width, int height, KaiListCtrl *theList);
	void Save();
	Item* Copy(){return new ItemColor(*this);}
	AssColor col;
	int colOptNum;
};

class ItemCheckBox : public Item{
public:
	ItemCheckBox(bool check, const wxString &_label) : Item(TYPE_CHECKBOX){modified = check; enter=false; name = _label;}
	virtual ~ItemCheckBox(){}
	void OnMouseEvent(wxMouseEvent &event, bool enter, bool leave, KaiListCtrl *theList, Item **changed = NULL);
	void OnPaint(wxMemoryDC *dc, int x, int y, int width, int height, KaiListCtrl *theList);
	void Save(){};
	Item* Copy(){return new ItemCheckBox(*this);}
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
class List{
public:
	List(){};
	~List(){
		for(auto itg = garbage.begin(); itg != garbage.end(); itg++){
			delete *itg;
		}
	}
	void push_back(ItemRow* row){
		itemList.push_back(row);
		garbage.push_back(row);
	};
	ItemRow* operator [](const size_t num){
		return itemList[num];
	}
	const size_t size(){return itemList.size();}
	List *Copy(){
		List *copy = new List();
		copy->itemList = itemList;
		return copy;
	}
	void Change(size_t pos, ItemRow* row){
		itemList[pos] = row;
		garbage.push_back(row);
	}
	std::vector< ItemRow*> itemList;
	std::vector< ItemRow*> garbage;
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
		for(auto it = historyList.begin(); it != historyList.end(); it++){
			delete *it;
		}
		historyList.clear();
		delete itemList;
		if(bmp){delete bmp;}
	};
	int InsertColumn(size_t col, const wxString &name, byte type, int width);
	int AppendItem(Item *item); 
	int SetItem(size_t row, size_t col, Item *item); 
	Item *GetItem(size_t row, size_t col);
	void SaveAll(int col);
	void SetModified(bool modif){modified = modif; /*if(modif){PushHistory();}*/}
	bool GetModified(){return modified;}
	int FindItem(int column, const wxString &textItem);
	void ScrollTo(int row);
	size_t GetCount(){return itemList->size();}
	void SetSelection(int selection){sel = selection; Refresh(false);}
	int GetSelection(){return sel;}
	void PushHistory();
	void Undo(wxCommandEvent &evt);
	void Redo(wxCommandEvent &evt);
	void StartEdition();
	Item *CopyRow(int y, int x, bool pushBack = false);
	void SetTextArray(const wxArrayString &Array);
private:
	void OnSize(wxSizeEvent& evt);
	void OnPaint(wxPaintEvent& evt);
	void OnMouseEvent(wxMouseEvent &evt);
	void OnScroll(wxScrollWinEvent& event);
	void OnEraseBackground(wxEraseEvent &evt){};
	int GetMaxWidth();
	ItemRow header;
	List *itemList;
	std::vector<List*> historyList;
	wxArrayInt widths;
	wxBitmap *bmp;
	int sel;
	int lastSelX;
	int lastSelY;
	int diffX;
	int lastCollumn;
	int scPosV;
	int scPosH;
	int lineHeight;
	int headerHeight;
	int iter;
	bool modified;
	bool hasArrow;

	DECLARE_EVENT_TABLE()
	wxDECLARE_ABSTRACT_CLASS(KaiListCtrl);
};


#endif