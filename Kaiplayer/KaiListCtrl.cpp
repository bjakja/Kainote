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

#include "KaiListCtrl.h"
#include "KaiCheckBox.h"
#include "ColorPicker.h"
#include "config.h"
#include "Menu.h"
#include "wx/clipbrd.h"

wxDEFINE_EVENT(LIST_ITEM_DOUBLECLICKED, wxCommandEvent);
wxDEFINE_EVENT(LIST_ITEM_RIGHT_CLICK, wxCommandEvent);

void ItemText::OnPaint(wxMemoryDC *dc, int x, int y, int width, int height, KaiListCtrl *theList)
{
	wxSize ex = dc->GetTextExtent(name);
	
	if(modified){dc->SetTextForeground(Options.GetColour(WindowWarningElements));}
	wxRect cur(x, y, width - 8, height);
	dc->SetClippingRegion(cur);
	dc->DrawLabel(name,cur,wxALIGN_CENTER_VERTICAL);
	dc->DestroyClippingRegion();
	if(modified){dc->SetTextForeground(Options.GetColour(theList->IsThisEnabled()? WindowText : WindowTextInactive));}
}

void ItemColor::OnPaint(wxMemoryDC *dc, int x, int y, int width, int height, KaiListCtrl *theList)
{
	if(col.a){
		wxColour col1=Options.GetColour(StylePreviewColor1);
		wxColour col2=Options.GetColour(StylePreviewColor2);
		int r2 = col.r, g2 = col.g, b2 = col.b;
		int r = col1.Red(), g = col1.Green(), b = col1.Blue();
		int r1 = col2.Red(), g1 = col2.Green(), b1 = col2.Blue();
		int inv_a = 0xFF - col.a;
		int fr = (r2* inv_a / 0xFF) + (r - inv_a * r / 0xFF);
		int fg = (g2* inv_a / 0xFF) + (g - inv_a * g / 0xFF);
		int fb = (b2* inv_a / 0xFF) + (b - inv_a * b / 0xFF);
		wxColour firstMask(fr,fg,fb);
		fr = (r2* inv_a / 0xFF) + (r1 - inv_a * r1 / 0xFF);
		fg = (g2* inv_a / 0xFF) + (g1 - inv_a * g1 / 0xFF);
		fb = (b2* inv_a / 0xFF) + (b1 - inv_a * b1 / 0xFF);
		wxColour secondMask(fr,fg,fb);
		int squareSize = (height-3)/4;
		for(int i = 0; i< 4; i++){
			for(int j = 0; j< 4; j++){
				dc->SetBrush(((i+j) % 2 == 0)? firstMask : secondMask);
				dc->SetPen(*wxTRANSPARENT_PEN);
				dc->DrawRectangle(x+2+(i*squareSize),y+2+(j*squareSize),squareSize,squareSize);
			}
		}
	}
	dc->SetBrush(wxBrush(col.a? *wxTRANSPARENT_BRUSH : col.GetWX()));
	dc->SetPen(wxPen(Options.GetColour(WindowTextInactive)));
	dc->DrawRectangle(x+1,y+1,height-3,height-3);
	
	int fw, fh;
	wxString hextext = col.GetHex(true);
	dc->GetTextExtent(hextext, &fw, &fh);

	if(modified){dc->SetTextForeground(Options.GetColour(WindowWarningElements));}
	wxRect cur(x+height+2, y, width - (height+8), height);
	dc->SetClippingRegion(cur);
	dc->DrawLabel(hextext,cur,wxALIGN_CENTER_VERTICAL);
	dc->DestroyClippingRegion();
	if(modified){dc->SetTextForeground(Options.GetColour(theList->IsThisEnabled()? WindowText : WindowTextInactive));}
}

void ItemColor::OnMouseEvent(wxMouseEvent &event, bool enter, bool leave, KaiListCtrl *theList, Item **changed)
{
	if(event.LeftDClick()){
		DialogColorPicker *dcp = DialogColorPicker::Get(theList,col);
		wxPoint mst=wxGetMousePosition();
		wxSize siz=dcp->GetSize();
		siz.x;
		wxRect rc = wxGetClientDisplayRect();
		mst.x-=(siz.x/2);
		mst.x=MID(rc.x, mst.x, rc.width-siz.x);
		mst.y+=15;
		mst.y=MID(rc.y, mst.y , rc.height-siz.y);
		dcp->Move(mst);
		if (dcp->ShowModal() == wxID_OK) {
			ItemColor *copy = new ItemColor(*this);
			copy->col = dcp->GetColor();
			theList->SetModified(true);
			copy->modified = true;
			(*changed) = copy;
		}
	}else if(event.RightUp()){
		Menu menut;
		menut.Append(7786,_("&Kopiuj"));
		menut.Append(7787,_("&Wklej"));
		int id = menut.GetPopupMenuSelection(event.GetPosition(), theList);
		if(id == 7786){
			wxString whatcopy = col.GetHex(true);
			if (wxTheClipboard->Open())
			{
				wxTheClipboard->SetData( new wxTextDataObject(whatcopy) );
				wxTheClipboard->Close();
				wxTheClipboard->Flush();
			}
		}else if(id == 7787){
			if (wxTheClipboard->Open()){
				if (wxTheClipboard->IsSupported( wxDF_TEXT )){
					wxTextDataObject data;
					wxTheClipboard->GetData( data );
					ItemColor *copy = new ItemColor(*this);
					copy->col.SetAss(data.GetText());
					theList->SetModified(true);
					copy->modified = true;
					(*changed) = copy;
				}
				wxTheClipboard->Close();
			}
		}
	}

};

void ItemColor::Save(){
	if(modified){
		Options.SetColor((COLOR)colOptNum, col);
		modified=false;
	}
}

void ItemCheckBox::OnPaint(wxMemoryDC *dc, int x, int y, int w, int h, KaiListCtrl *theList)
{
	wxString bitmapName = (modified)? "checkbox_selected" :  "checkbox" ;
	wxBitmap checkboxBmp = wxBITMAP_PNG(bitmapName);
	if(enter){BlueUp(&checkboxBmp);}
	dc->DrawBitmap(checkboxBmp, x+1, y + (h-13)/2);

	if(w>18){
		int fw, fh;
		dc->GetTextExtent(name, &fw, &fh);
		wxRect cur(x+18, y, w - 20, h);
		dc->SetClippingRegion(cur);
		dc->DrawLabel(name,cur,wxALIGN_CENTER_VERTICAL);
		dc->DestroyClippingRegion();
		
		
	}
}


void ItemCheckBox::OnMouseEvent(wxMouseEvent &event, bool _enter, bool leave, KaiListCtrl *theList, Item **copy){
	if(event.LeftDown()){
		modified = !modified;
		theList->Refresh(false);
	}
	if(!enter && _enter){
		enter = _enter; theList->Refresh(false);
	}
	if(enter && leave){
		enter = false; theList->Refresh(false);
	}
};

KaiListCtrl::KaiListCtrl(wxWindow *parent, int id, const wxPoint &pos, const wxSize &size, int style)
	:KaiScrolledWindow(parent, id, pos, size, style|wxVERTICAL|wxHORIZONTAL)
	,bmp(NULL)
	,sel(-1)
	,lastSelX(-1)
	,lastSelY(-1)
	,scPosV(0)
	,scPosH(0)
	,lineHeight(17)
	,headerHeight(25)
	,lastCollumn(0)
	,modified(false)
	,hasArrow(true)
	,iter(0)
	,itemList(new List())
{
	SetBackgroundColour(parent->GetBackgroundColour());
	SetForegroundColour(parent->GetForegroundColour());
	SetMinSize(size);
	SetFont(wxFont(9,wxSWISS,wxFONTSTYLE_NORMAL,wxNORMAL,false,"Tahoma",wxFONTENCODING_DEFAULT));
	wxAcceleratorEntry entries[2];
	entries[0].Set(wxACCEL_CTRL, 'Z', 11642);
	entries[1].Set(wxACCEL_CTRL, 'Y', 11643);
	wxAcceleratorTable accel(2, entries);
	SetAcceleratorTable(accel);
	Bind(wxEVT_COMMAND_MENU_SELECTED, &KaiListCtrl::Undo, this, 11642);
	Bind(wxEVT_COMMAND_MENU_SELECTED, &KaiListCtrl::Redo, this, 11643);
}

KaiListCtrl::KaiListCtrl(wxWindow *parent, int id, int numelem, wxString *list, const wxPoint &pos, 
		const wxSize &size, int style)
	:KaiScrolledWindow(parent, id, pos, size, style|wxVERTICAL)
	,bmp(NULL)
	,sel(-1)
	,lastSelX(-1)
	,lastSelY(-1)
	,scPosV(0)
	,scPosH(0)
	,lineHeight(17)
	,headerHeight(3)
	,lastCollumn(0)
	,modified(false)
	,hasArrow(true)
	,iter(0)
	,itemList(new List())
{
	SetBackgroundColour(parent->GetBackgroundColour());
	SetForegroundColour(parent->GetForegroundColour());
	SetMinSize(size);
	SetFont(wxFont(9,wxSWISS,wxFONTSTYLE_NORMAL,wxNORMAL,false,"Tahoma",wxFONTENCODING_DEFAULT));
	InsertColumn(0, "", TYPE_CHECKBOX, -1);
	for(int i = 0; i < numelem; i++){
		AppendItem(new ItemCheckBox(false, list[i]));
	}
}

KaiListCtrl::KaiListCtrl(wxWindow *parent, int id, const wxArrayString &list, const wxPoint &pos, 
		const wxSize &size, int style)
		:KaiScrolledWindow(parent, id, pos, size, style|wxVERTICAL)
	,bmp(NULL)
	,sel(-1)
	,lastSelX(-1)
	,lastSelY(-1)
	,scPosV(0)
	,scPosH(0)
	,lineHeight(17)
	,headerHeight(3)
	,lastCollumn(0)
	,modified(false)
	,hasArrow(true)
	,iter(0)
	,itemList(new List())
{
	SetBackgroundColour(parent->GetBackgroundColour());
	SetForegroundColour(parent->GetForegroundColour());
	SetMinSize(size);
	SetFont(wxFont(9,wxSWISS,wxFONTSTYLE_NORMAL,wxNORMAL,false,"Tahoma",wxFONTENCODING_DEFAULT));
	InsertColumn(0, "", TYPE_TEXT, -1);
	for(size_t i = 0; i < list.size(); i++){
		AppendItem(new ItemText(list[i]));
	}
}

int KaiListCtrl::InsertColumn(size_t col, const wxString &name, byte type, int width)
{
	if(col>= widths.size()){
		header.Insert(col, new ItemText(name));
		widths.push_back(width);
		return col;
	}
	header.Insert(col, new ItemText(name));
	widths.Insert(col, width);
	return col;
}
	
int KaiListCtrl::AppendItem(Item *item)
{
	ItemRow *newitem = new ItemRow(0, item);
	itemList->push_back(newitem);
	return itemList->size()-1;
}
	

int KaiListCtrl::SetItem(size_t row, size_t col, Item *item)
{
	if(col>=(*itemList)[row]->row.size()){
		(*itemList)[row]->row.push_back(item);
		return row;
	}
	(*itemList)[row]->row.insert((*itemList)[row]->row.begin()+col, item);
	return row;
}
	
Item *KaiListCtrl::GetItem(size_t row, size_t col)
{
	if(row<0 || row >= itemList->size() || col<0 || col >= (*itemList)[row]->row.size()){return NULL;}
	return (*itemList)[row]->row[col];
}


void KaiListCtrl::OnSize(wxSizeEvent& evt)
{
	Refresh(false);
}
	
void KaiListCtrl::OnPaint(wxPaintEvent& evt)
{
	int w=0;
	int h=0;
	GetClientSize (&w, &h);
	if(w==0||h==0){return;}
	size_t maxVisible = ((h-headerHeight)/lineHeight)+1;
	size_t itemsize = itemList->size()+1;
	if((size_t)scPosV>=itemsize-maxVisible){
		scPosV=itemsize-maxVisible;
	}
	if(scPosV<0){scPosV=0;}
	size_t maxsize=itemsize-1;
	if(itemsize>maxVisible){
		maxsize=MIN(maxVisible+scPosV, itemsize-1);
		if(SetScrollBar(wxVERTICAL, scPosV, maxVisible, itemsize, maxVisible-2)){
			GetClientSize (&w, &h);
		}
	}
	int maxWidth = GetMaxWidth()+10;
	int bitmapw = w;
	if(widths.size()>1){
		if(SetScrollBar(wxHORIZONTAL, scPosH, w, maxWidth, w-2)){
			GetClientSize (&w, &h);
			if(maxWidth <= w){scPosH=0;SetScrollPos(wxHORIZONTAL,0);}
		}
	}
	wxMemoryDC tdc;
	if (bmp && (bmp->GetWidth() < bitmapw || bmp->GetHeight() < h)) {
		delete bmp;
		bmp = NULL;
	}
	if(!bmp){bmp=new wxBitmap(bitmapw, h);}
	tdc.SelectObject(*bmp);
	bool enabled = IsThisEnabled();
	wxColour highlight = Options.GetColour(StaticListSelection);
	wxColour txt = Options.GetColour(WindowText);
	wxColour inactivetxt = Options.GetColour(WindowTextInactive);
	wxColour border = Options.GetColour(StaticListBorder);
	tdc.SetPen(wxPen(border));
	tdc.SetBrush(wxBrush(enabled? Options.GetColour(StaticListBackground) : Options.GetColour(WindowBackgroundInactive)));
	tdc.DrawRectangle(0,0,w,h);
	tdc.SetTextForeground(enabled? txt : inactivetxt);
	tdc.SetFont(GetFont());
	//header
	
	int posX= 5-scPosH;
	int posY=headerHeight;
	for(size_t i = scPosV; i < maxsize; i++){
		auto row = (*itemList)[i]->row;
		for(size_t j = 0; j < widths.size(); j++){
			if(j>=row.size()){
				continue;
			}
			if(widths[j] == -1){widths[j] = w-1;}
			//drawing
			if(i==sel){
				tdc.SetPen(wxPen(highlight));
				tdc.SetBrush(wxBrush(highlight));
				tdc.DrawRectangle(posX-5, posY, widths[j], lineHeight);
			}
			row[j]->OnPaint(&tdc, posX, posY, widths[j], lineHeight, this);
			posX += widths[j];
			
		}
		posY += lineHeight;
		posX = posX=5-scPosH;
		
	}
	posX=5-scPosH;
	
	tdc.SetPen(wxPen(border));
	
	if(headerHeight>4){
		tdc.SetTextForeground(enabled? Options.GetColour(StaticListTextHeadline) : inactivetxt);
		tdc.SetBrush(Options.GetColour(StaticListBackgroundHeadline));
		tdc.DrawRectangle(0,0,w,headerHeight-2);
		for(size_t j = 0; j < widths.size(); j++){
			wxString headerTxt = ((ItemText*)header.row[j])->GetName();
			wxSize ex = tdc.GetTextExtent(headerTxt);
			tdc.DrawText(headerTxt, posX, ((headerHeight - ex.y)/2) );

			posX += widths[j];
			tdc.DrawLine(posX-3, 0, posX-3, h);
		}
		//tdc.DrawLine(0, headerHeight-2, w, headerHeight-2);
	}
	tdc.SetBrush(*wxTRANSPARENT_BRUSH);
	tdc.DrawRectangle(0,0,w,h);

	wxPaintDC dc(this);
	dc.Blit(0,0,w,h,&tdc,0,0);
}
	
void KaiListCtrl::OnMouseEvent(wxMouseEvent &evt)
{
	if (evt.LeftUp() && HasCapture()){
		ReleaseMouse();
	}
	if(evt.ButtonDown()){
		SetFocus();
	}
	if (evt.GetWheelRotation() != 0) {
		int w=0;
		int h=0;
		GetClientSize (&w, &h);
		int step = evt.GetWheelRotation() / evt.GetWheelDelta();
		scPosV -=step;
		size_t maxVisible = ((h-headerHeight)/lineHeight)+1;
		if(scPosV<0){scPosV=0;}
		else if((size_t)scPosV > itemList->size()+1-maxVisible){scPosV = itemList->size()+1-maxVisible;}
		Refresh(false);
		return;
	}
	wxPoint cursor = evt.GetPosition();

	int elemY = ((cursor.y-headerHeight)/lineHeight) + scPosV;
	if(elemY<0 || cursor.y <= headerHeight){
		//tu napisz chwytanie headera
		//if header < 5 wtedy nic nie robimy
		if(headerHeight > 5){
			if(!hasArrow && evt.LeftDown()){
				if(!HasCapture()){CaptureMouse();}
				diffX = cursor.x;
				return;
			}else if(!hasArrow && evt.LeftIsDown()){
				widths[lastCollumn] += (cursor.x - diffX);
				if(widths[lastCollumn]<20){widths[lastCollumn]=20;}
				diffX = cursor.x;
				Refresh(false);
				return;
			}
			int maxwidth=5-scPosH;
			//bool isonpos= false;
			for(size_t i = 0; i < widths.size(); i++){
				maxwidth += widths[i];
				if(abs(maxwidth - cursor.x)<5){
					if(hasArrow){
						SetCursor(wxCURSOR_SIZEWE);
						hasArrow = false;
						lastCollumn = i;
					}
					return;
				}
			}
			
		}
		if(!hasArrow){ SetCursor(wxCURSOR_ARROW); hasArrow=true;}
		return;
	}
	if(!hasArrow){ SetCursor(wxCURSOR_ARROW); hasArrow=true;}
	Item *copy=NULL;
	if((size_t)elemY>=itemList->size()){
		//tu ju¿ nic nie zrobimy, jesteœmy poza elemetami na samym dole
		if(lastSelX != -1 && lastSelY !=-1){
			(*itemList)[lastSelY]->row[lastSelX]->OnMouseEvent(wxMouseEvent(), false, true, this, &copy);
			lastSelX=-1;lastSelY=-1;
		}
		if(copy){delete copy;}
		return;
	}
	int elemX = -1;
	int startX = 5-scPosH; 
	for(size_t i = 0; i < widths.size(); i++){
		if(cursor.x > startX && cursor.x <= startX + widths[i]){
			elemX=i;
			bool enter = (elemX != lastSelX || elemY != lastSelY) || evt.Entering();
			(*itemList)[elemY]->row[elemX]->OnMouseEvent(evt, enter, false, this, &copy);
			if(copy!=NULL){
				ItemRow *newRow = new ItemRow();
				for(size_t g = 0; g < (*itemList)[elemY]->row.size(); g++){
					if(g==elemX){
						newRow->row.push_back(copy);
						continue;
					}
					newRow->row.push_back((*itemList)[elemY]->row[g]->Copy());
				}
				//(*itemList)[elemY]=newRow;
				itemList->Change(elemY, newRow);
				PushHistory();
				Refresh(false);
			}
			break;
		}
		startX += widths[i];
	}
	if((elemX != lastSelX || elemY != lastSelY || evt.Leaving()) && lastSelX != -1 && lastSelY !=-1){
		(*itemList)[lastSelY]->row[lastSelX]->OnMouseEvent(wxMouseEvent(), false, true, this, &copy);
		if(copy){delete copy; copy = NULL;}
	}
	
	if(evt.LeftDown()){
		
		sel = elemY;

		Refresh(false);
	}
	if(evt.LeftDClick()){
		wxCommandEvent evt2(LIST_ITEM_DOUBLECLICKED, GetId()); 
		evt2.SetInt(elemY);
		AddPendingEvent(evt2);
	}
	if(evt.RightDown()){
		wxCommandEvent evt2(LIST_ITEM_RIGHT_CLICK, GetId()); 
		evt2.SetInt(elemY);
		AddPendingEvent(evt2);
	}
	lastSelY = elemY;
	lastSelX = elemX;
}

void KaiListCtrl::OnScroll(wxScrollWinEvent& event)
{
	size_t newPos=0;
	int orient = event.GetOrientation();
	size_t scPos = (orient == wxVERTICAL)? scPosV : scPosH;
	newPos = event.GetPosition();
	if (scPos != newPos) {
		if(orient == wxVERTICAL){
			scPosV = newPos;
		}else{
			scPosH = newPos;
		}
		Refresh(false);
	}
}

int KaiListCtrl::GetMaxWidth()
{
	int maxWidth=0;
	for(size_t i =0; i < widths.size(); i++){
		maxWidth += widths[i];
	}
	return maxWidth;
}

void KaiListCtrl::SaveAll(int col)
{
	if(!modified){return;}
	for(size_t i = 0; i<itemList->size(); i++){
		if((*itemList)[i]->row.size()<=(size_t)col){continue;}
		(*itemList)[i]->row[col]->Save();
	}
	modified = false;
}

int KaiListCtrl::FindItem(int column, const wxString &textItem)
{
	for(size_t i = 0; i<itemList->size(); i++){
		if(column < 0){
			for(size_t j = 0; j < (*itemList)[i]->row.size(); j++){
				if((*itemList)[i]->row[j]->name == textItem){
					return i;
				}
			}
		}
		else if((*itemList)[i]->row.size()<=(size_t)column){continue;}
		else if((*itemList)[i]->row[column]->name == textItem){
			return i;
		}
	}
	return -1;
}

void KaiListCtrl::ScrollTo(int row){
	scPosV = row;
	SetScrollpos(wxVERTICAL,row);
	Refresh(false);
}

void KaiListCtrl::PushHistory()
{
	if(iter != historyList.size()){
		for(auto it = historyList.begin()+iter+1; it != historyList.end(); it++)
		{
			delete (*it);
		}
		historyList.erase(historyList.begin()+iter+1, historyList.end());
	}
	historyList.push_back(itemList);
	itemList = itemList->Copy();
	iter++;
}

void KaiListCtrl::StartEdition()
{
	historyList.push_back(itemList);
	itemList = itemList->Copy();
}

void KaiListCtrl::Undo(wxCommandEvent &evt)
{
	if(iter>0){
		iter--;
		delete itemList;
		itemList = historyList[iter]->Copy();
		Refresh(false);
	}
}
	
void KaiListCtrl::Redo(wxCommandEvent &evt)
{
	if(iter< historyList.size()-1){
		iter++;
		delete itemList;
		itemList = historyList[iter]->Copy();
		Refresh(false);
	}
}

Item *KaiListCtrl::CopyRow(int y, int x, bool pushBack)
{
	ItemRow *newRow = new ItemRow();
	for(size_t g = 0; g < (*itemList)[y]->row.size(); g++){
		newRow->row.push_back((*itemList)[y]->row[g]->Copy());
	}
	if(pushBack){
		itemList->push_back(newRow);
		int newy = itemList->size()-1;
		if(x < 0 || x >= (*itemList)[newy]->row.size()){return NULL;}
		return (*itemList)[newy]->row[x];
	}else{
		itemList->Change(y, newRow);
	}
	if(x < 0 || x >= (*itemList)[y]->row.size()){return NULL;}
	return (*itemList)[y]->row[x];
}


BEGIN_EVENT_TABLE(KaiListCtrl,KaiScrolledWindow)
	EVT_PAINT(KaiListCtrl::OnPaint)
	EVT_SIZE(KaiListCtrl::OnSize)
	EVT_SCROLLWIN(KaiListCtrl::OnScroll)
	EVT_MOUSE_EVENTS(KaiListCtrl::OnMouseEvent)
	EVT_ERASE_BACKGROUND(KaiListCtrl::OnEraseBackground)
END_EVENT_TABLE()

wxIMPLEMENT_ABSTRACT_CLASS(KaiListCtrl, wxWindow);