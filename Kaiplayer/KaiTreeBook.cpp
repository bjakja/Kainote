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

#include "KaiTreebook.h"
#include "config.h"
#include <wx/dcmemory.h>
#include <wx/dcclient.h>

int textHeight = 18;

KaiTreebook::KaiTreebook(wxWindow *parent, int id,
        const wxPoint& pos, const wxSize& size, long style)
		:wxWindow(parent, id, pos, size, style)
		,treeWidth(20)
		,selection(0)
		,bmp(NULL)
{

}

KaiTreebook::~KaiTreebook()
{
	for(auto it = Pages.begin(); it != Pages.end(); it++){
		delete *it;
	}
	Pages.clear();
	wxDELETE(bmp);
}
	
void KaiTreebook::AddPage(wxWindow *page, const wxString &name, bool selected)
{
	page->Hide();
	Pages.push_back(new Page(page, name));
	if(selected){
		selection = Pages.size()-1;
	}
}
	
void KaiTreebook::AddSubPage(wxWindow *page, const wxString &name, bool nextTree, bool selected )
{
	page->Hide();
	int lastSubpage = Pages[Pages.size()-1]->whichSubpage;
	if(!lastSubpage || nextTree){Pages[Pages.size()-1]->canCollapse = true;}

	Pages.push_back(new Page(page, name, (!lastSubpage || nextTree)? lastSubpage+1 : lastSubpage));
	if(selected){
		selection = Pages.size()-1;
	}
}
	
void KaiTreebook::Fit()
{
	CalcWidth();
	int width = 0, height = 0;
	for(size_t i = 0; i < Pages.size(); i++){
		wxSize pageSize = Pages[i]->page->GetBestSize();
		if(pageSize.x > width){
			width = pageSize.x;
		}
		if(pageSize.y > height){
			height = pageSize.y;
		}
	}
	SetMinSize(wxSize(width + treeWidth + 10, height));
	SetColours(Options.GetColour(WindowBackground),Options.GetColour(WindowText));
	Pages[selection]->page->Show();
}

int KaiTreebook::GetSelection()
{
	return selection;
}


	
void KaiTreebook::ChangeSelection(int sel)
{
	ChangePage(sel);

	RefreshTree();
}

void KaiTreebook::CalcWidth()
{
	treeWidth = 20;
	for(size_t i = 0; i < Pages.size(); i++){
		wxSize txtsize = GetTextExtent(Pages[i]->name);
		txtsize.x += 25 + (Pages[i]->whichSubpage * 20);
		if(txtsize.x > treeWidth){treeWidth = txtsize.x;}
	}
}

//pamiêtaj!!! nigdy nie zmieniaj selection przed u¿yciem tej funkcji, sama to zrobi,
//w przeciwnym przypadku zrobi siê mix, bo ukryje z³¹ zak³adkê
void KaiTreebook::ChangePage(int page)
{

	Pages[selection]->page->Hide();
	int w=0;
	int h=0;
	GetClientSize (&w, &h);
	Pages[page]->page->SetSize(treeWidth+10, 0, w - (treeWidth+10), h);
	Pages[page]->page->Show();
	selection = page;
}

void KaiTreebook::OnKeyPress(wxKeyEvent& event)
{
	//shithappens na razie
}
	
void KaiTreebook::OnMouseEvent(wxMouseEvent& event)
{
	if(event.LeftDown() || event.LeftDClick()){
		int elem = event.GetY() / textHeight;
		if(elem < 0 || elem >= (int)Pages.size()){return;}
		elem = CalcElement(elem);
		if(elem<0){return;}
		int subPage = Pages[elem]->whichSubpage * 20;
		int posX = 20 + subPage;
		int x = event.GetX();
		if(x >= posX && elem != selection && !event.LeftDClick()){
			ChangePage(elem);
			RefreshTree();
		}else if(Pages[elem]->canCollapse && ((x >= 6 + subPage && x <= 17 + subPage) || event.LeftDClick())){
			Pages[elem]->collapsed = !Pages[elem]->collapsed;
			for(size_t i = elem + 1; i < Pages.size(); i++){
				if(!Pages[i]->whichSubpage){break;}
				if(selection == i){ChangePage(elem);}
			}
			RefreshTree();
		}
	}
}
    
void KaiTreebook::OnSize(wxSizeEvent& event)
{
	RefreshTree();
}
	
void KaiTreebook::OnPaint(wxPaintEvent& event)
{
	int w=0;
	int h=0;
	GetClientSize (&w, &h);
	if(w==0||h==0){return;}
	wxMemoryDC tdc;
	if (bmp && (bmp->GetWidth() < w || bmp->GetHeight() < h)) {
		delete bmp;
		bmp = NULL;
	}
	if(!bmp){bmp=new wxBitmap(w,h);}
	tdc.SelectObject(*bmp);
	tdc.SetFont(GetFont());
	bool enabled = IsThisEnabled();
	wxColour wbg = Options.GetColour(StaticListBackground);
	wxColour wfg = Options.GetColour(WindowText);
	wxColour selColor = Options.GetColour(StaticListSelection);
	tdc.SetBrush(wxBrush(wbg));
	tdc.SetPen(wxPen(wbg));
	tdc.SetTextForeground(wfg);
	tdc.DrawRectangle(0,0,treeWidth+10,h);
	bool afterBranch = false;
	bool isFirstSubPage = true;
	int halfHeight = (textHeight/2);
	int posY=3;
	int lastPage=0;
	CalcElement(Pages.size(), &lastPage);
	
	tdc.SetPen(wxPen(wfg));
	for(int j = halfHeight + 3; j <= (lastPage * textHeight) + 3 - halfHeight; j+=2){
		tdc.DrawPoint(10, j);
	}
	
	for(size_t i = 0; i < Pages.size(); i++){
		if(afterBranch){
			if(Pages[i]->whichSubpage == 0){
				afterBranch=false;
			}else{
				continue;
			}
		}
		if(Pages[i]->collapsed){afterBranch = true;}
		int subPage = Pages[i]->whichSubpage * 20;
		int posX = 20 + subPage;
		wxRect cur(posX, posY, treeWidth - posX - 2, textHeight);
		if(i == selection){
			wxSize textSize = tdc.GetTextExtent(Pages[i]->name);
			tdc.SetBrush(wxBrush(selColor));
			tdc.SetPen(wxPen(selColor));
			tdc.DrawRectangle(cur.x, cur.y, textSize.x, cur.height);
		}
		tdc.SetClippingRegion(cur);
		tdc.DrawLabel(Pages[i]->name,cur,wxALIGN_CENTER_VERTICAL);
		tdc.DestroyClippingRegion();
		tdc.SetPen(wxPen(wfg));
		for(int k = 10 + subPage; k <= 18 + subPage; k+=2){
			tdc.DrawPoint(k, cur.y+ halfHeight);
		}
		if(subPage){
			int start = (isFirstSubPage)? posY + 1 : posY + 2 - halfHeight;
			for(int l = start; l <= posY + 1 + halfHeight; l+=2){
				tdc.DrawPoint(8 + subPage, l);
			}
			isFirstSubPage = false;
		}else{isFirstSubPage = true;}
		if(Pages[i]->canCollapse){
			tdc.SetBrush(wxBrush(wbg));
			tdc.DrawRectangle(5 + subPage, posY + 4, 11, 11);
			tdc.DrawLine(7 + subPage, posY + halfHeight, 14 + subPage, posY + halfHeight);
			if(Pages[i]->collapsed){
				tdc.DrawLine(10 + subPage, posY + 6, 10 + subPage, posY + 13);
			}
		}
		posY += textHeight;
	}
	

	tdc.SetBrush(*wxTRANSPARENT_BRUSH);
	tdc.SetPen(wxPen(Options.GetColour(StaticListBorder)));
	tdc.DrawRectangle(1,1,treeWidth+4,h-2);
	wxPaintDC dc(this);
	dc.Blit(0,0,w,h,&tdc,0,0);
}

void KaiTreebook::RefreshTree()
{
	int w, h;
	GetClientSize(&w, &h);
	wxRect rc(0,0,treeWidth, h);
	Refresh(false, &rc);

}

int KaiTreebook::CalcElement(int element, int *lastPage)
{
	bool afterBranch = false;
	int clickedElem = 0;
	for(size_t i = 0; i < Pages.size(); i++){
		if(afterBranch){
			if(Pages[i]->whichSubpage == 0){
				afterBranch=false;
			}else{
				continue;
			}
		}
		if(Pages[i]->collapsed){afterBranch = true;}
		if(clickedElem >= element){return i;}
		clickedElem++;
		if(lastPage && !Pages[i]->whichSubpage){*lastPage = clickedElem;}
	}
	return -1;
}

void KaiTreebook::SetColours(const wxColour &bgcol, const wxColour &fgcol)
{
	for(size_t i = 0; i < Pages.size(); i++){
		Pages[i]->page->SetBackgroundColour(bgcol);
		Pages[i]->page->SetForegroundColour(fgcol);
	}
	SetBackgroundColour(bgcol);
	SetForegroundColour(fgcol);
}

BEGIN_EVENT_TABLE(KaiTreebook,wxWindow)
	EVT_PAINT(KaiTreebook::OnPaint)
	EVT_SIZE(KaiTreebook::OnSize)
	//EVT_ERASE_BACKGROUND(KaiTreebook::OnEraseBackground)
	EVT_MOUSE_EVENTS(KaiTreebook::OnMouseEvent)
	EVT_KEY_DOWN(KaiTreebook::OnKeyPress)
	//EVT_MOUSE_CAPTURE_LOST(KaiTextCtrl::OnLostCapture)
END_EVENT_TABLE()