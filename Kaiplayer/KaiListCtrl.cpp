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
#include "config.h"

void ItemText::OnPaint(wxMemoryDC *dc, int x, int y, int width, int height, bool isSel)
{
	if(isSel){
		wxColour highlight = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT);
		dc->SetPen(wxPen(highlight));
		dc->SetBrush(wxBrush(highlight));
		dc->DrawRectangle(x, y, width, height);
	}
	wxSize ex = dc->GetTextExtent(name);
	//dc->DrawText(name, x, y + ((height - ex.y)/2));
	wxRect cur(x+2, y, width - 4, height);
	dc->SetClippingRegion(cur);
	dc->DrawLabel(name,cur,wxALIGN_CENTER_VERTICAL);
	dc->DestroyClippingRegion();
}

void ItemColor::OnPaint(wxMemoryDC *dc, int x, int y, int width, int height, bool isSel)
{
	if(isSel){
		wxColour highlight = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT);
		dc->SetPen(wxPen(highlight));
		dc->SetBrush(wxBrush(highlight));
		dc->DrawRectangle(x, y, width, height);
	}
	if(col.a){
		wxColour col1=Options.GetColour("Style Preview Color1");
		wxColour col2=Options.GetColour("Style Preview Color2");
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
	dc->SetPen(wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT)));
	dc->DrawRectangle(x+1,y+1,height-3,height-3);
	
	int fw, fh;
	wxString hextext = col.GetHex(true);
	dc->GetTextExtent(hextext, &fw, &fh);
	//dc->DrawText(hextext, height+2, (height - fh)/2);
	wxRect cur(x+height+2, y, width - (height+4), height);
	dc->SetClippingRegion(cur);
	dc->DrawLabel(hextext,cur,wxALIGN_CENTER_VERTICAL);
	dc->DestroyClippingRegion();
}

void ItemCheckBox::OnPaint(wxMemoryDC *dc, int x, int y, int w, int h, bool isSel)
{
	if(isSel){
		wxColour highlight = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT);
		dc->SetPen(wxPen(highlight));
		dc->SetBrush(wxBrush(highlight));
		dc->DrawRectangle(x, y, w, h);
	}
	wxString bitmapName = (checked)? "checkbox_selected" :  "checkbox" ;
	wxBitmap checkboxBmp = wxBITMAP_PNG(bitmapName);
	if(enter){BlueUp(&checkboxBmp);}
	dc->DrawBitmap(checkboxBmp, x+1, y + (h-13)/2);

	if(w>18){
		int fw, fh;
		dc->GetTextExtent(label, &fw, &fh);
		wxRect cur(x+18, y, w - 20, h);
		dc->SetClippingRegion(cur);
		dc->DrawLabel(label,cur,wxALIGN_CENTER_VERTICAL);
		dc->DestroyClippingRegion();
		
		
	}
}

KaiListCtrl::KaiListCtrl(wxWindow *parent, int id, const wxPoint &pos, const wxSize &size, int style)
	:KaiScrolledWindow(parent, id, pos, size, style|wxVERTICAL|wxHORIZONTAL)
	,bmp(NULL)
	,sel(-1)
	,lastSel(-1)
	,scPosV(0)
	,scPosH(0)
	,lineHeight(17)
{
	SetBackgroundColour(parent->GetBackgroundColour());
	SetMinSize(size);
	SetFont(wxFont(9,wxSWISS,wxFONTSTYLE_NORMAL,wxNORMAL,false,"Tahoma",wxFONTENCODING_DEFAULT));
}

int KaiListCtrl::InsertCollumn(size_t col, const wxString &name, byte type, int width)
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
	itemList.push_back(new ItemRow(0, item));
	return itemList.size()-1;
}
	

int KaiListCtrl::SetItem(size_t row, size_t col, Item *item)
{
	if(col>=itemList[row]->row.size()){
		itemList[row]->row.push_back(item);
		return row;
	}
	itemList[row]->row.insert(itemList[row]->row.begin()+col, item);
	return row;
}
	
Item *KaiListCtrl::GetItem(size_t row, size_t col) const
{
	return itemList[row]->row[col];
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
	size_t maxVisible = (h/lineHeight)+1;
	size_t itemsize = itemList.size();
	if((size_t)scPosV>=itemsize-maxVisible){scPosV=itemsize-maxVisible;}
	if(scPosV<0){scPosV=0;}
	size_t maxsize=itemsize;
	if(itemsize>maxVisible){
		maxsize=maxVisible+scPosV;
		if(SetScrollBar(wxVERTICAL, scPosV, maxVisible, itemsize)){
			GetClientSize (&w, &h);
		}
	}
	int maxWidth = GetMaxWidth();
	int bitmapw = w;
	if(maxWidth> w){
		/*if(SetScrollBar(wxHORIZONTAL, scPosH, w, maxWidth)){
			GetClientSize (&w, &h);
		}*/
	}
	wxMemoryDC tdc;
	if (bmp && (bmp->GetWidth() < bitmapw || bmp->GetHeight() < h)) {
		delete bmp;
		bmp = NULL;
	}
	if(!bmp){bmp=new wxBitmap(bitmapw, h);}
	tdc.SelectObject(*bmp);
	wxColour highlight = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT);
	tdc.SetPen(wxPen("#000000"));
	tdc.SetBrush(wxBrush(GetBackgroundColour()));
	tdc.DrawRectangle(0,0,w,h);
	tdc.SetTextForeground(GetForegroundColour());
	tdc.SetFont(GetFont());
	//header
	int posX=scPosH+5;
	for(size_t j = 0; j < widths.size(); j++){
		wxString headerTxt = ((ItemText*)header.row[j])->GetName();
		wxSize ex = tdc.GetTextExtent(headerTxt);
		tdc.DrawText(headerTxt, posX, ((25 - ex.y)/2) );
		posX += widths[j];
	}
	posX=scPosH+5;
	int posY=25;
	for(size_t i = scPosV; i < maxsize; i++){
		auto row = itemList[i]->row;
		for(size_t j = 0; j < widths.size(); j++){
			if(j>=row.size()){
				continue;
			}
			//drawing
			row[j]->OnPaint(&tdc, posX, posY, widths[j], lineHeight, i==sel);
			posX += widths[j];
			
		}
		posY += lineHeight;
		posX = posX=scPosH+5;
		
	}
	wxPaintDC dc(this);
	dc.Blit(-scPosH,0,w+scPosH,h,&tdc,0,0);
}
	
void KaiListCtrl::OnMouseEvent(wxMouseEvent &evt)
{
	
	if (evt.GetWheelRotation() != 0) {
		int w=0;
		int h=0;
		GetClientSize (&w, &h);
		int step = evt.GetWheelRotation() / evt.GetWheelDelta();
		scPosV -=step;
		size_t maxVisible = ((h-25)/lineHeight)+1;
		if(scPosV<0){scPosV=0;}
		else if((size_t)scPosV > itemList.size()-maxVisible){scPosV = itemList.size()-maxVisible;}
		Refresh(false);
		return;
	}
	wxPoint cursor = evt.GetPosition();
	lastSel=sel;
	sel = ((cursor.y-20)/lineHeight) + scPosV;
	Refresh(false);
}

void KaiListCtrl::OnScroll(wxScrollWinEvent& event)
{
	size_t newPos=0;
	int orient = event.GetOrientation();
	size_t scPos = (orient = wxVERTICAL)? scPosV : scPosH;
	if(event.GetEventType()==wxEVT_SCROLLWIN_LINEUP)
	{
		newPos=scPos-1;
		if(newPos<0){newPos=0;return;}
	}
	else if(event.GetEventType()==wxEVT_SCROLLWIN_LINEDOWN)
	{
		newPos=scPos+1;
		if(newPos>=itemList.size()){newPos=itemList.size()-1;return;}
	}
	else if(event.GetEventType()==wxEVT_SCROLLWIN_PAGEUP)
	{
		wxSize size=GetClientSize();
		newPos=scPos;
		newPos-=(size.y/lineHeight) - 1;
		newPos=MAX(0,newPos);
	}
	else if(event.GetEventType()==wxEVT_SCROLLWIN_PAGEDOWN)
	{
		wxSize size=GetClientSize();
		newPos=scPos;
		newPos+=(size.y/lineHeight) - 1;
		newPos=MIN(newPos,itemList.size()-1);
	}
	else{newPos = event.GetPosition();}
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

BEGIN_EVENT_TABLE(KaiListCtrl,KaiScrolledWindow)
	EVT_PAINT(KaiListCtrl::OnPaint)
	EVT_SIZE(KaiListCtrl::OnSize)
	EVT_SCROLLWIN(KaiListCtrl::OnScroll)
	EVT_LEFT_DOWN(KaiListCtrl::OnMouseEvent)
	EVT_MOUSEWHEEL(KaiListCtrl::OnMouseEvent)
	EVT_ERASE_BACKGROUND(KaiListCtrl::OnEraseBackground)
END_EVENT_TABLE()