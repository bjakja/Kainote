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

#include "VideoToolbar.h"
#include "Config.h"


static int startDrawPos = 5;
std::vector< itemdata *> VideoToolbar::icons;

VideoToolbar::VideoToolbar (wxWindow *parent, const wxPoint &pos)
	:wxWindow(parent, -1, pos, wxSize(-1, 22))
	,Toggled(0)
	,clipToggled(toolsSize+1)
	,sel(-1)
	,clicked(false)
	,showClipTools(false)
	,showMoveTools(false)
	,blockScroll(false)
	,bmp(NULL)
{
	if(icons.size()==0){
		//pamiętaj, dodając tutaj elementy, zmień ich wartość w pliku h!!
		icons.push_back(new itemdata(PTR_BITMAP_PNG("cross"),_("Wskaźnik pozycji")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG("position"),_("Przesuwanie tekstu")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG("move"),_("Ruch")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG("scale"),_("Skalowanie")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG("frz"),_("Obrót wokół osi Z")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG("frxy"),_("Obrót wokół osi X / Y")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG("cliprect"),_("Wycinki prostokątne")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG("clip"),_("Wycinki wektorowe")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG("drawing"),_("Rysunki wektorowe")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG("MOVEAll"),_("Zmieniacz pozycji")));
		//tutaj mamy ikony dla clipa
		icons.push_back(new itemdata(PTR_BITMAP_PNG("VectorDrag"),_("Przesuń punkty")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG("VectorLine"),_("Dodaj linię")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG("VectorBezier"),_("Dodaj krzywą Beziera")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG("VECTORBSPLINE"),_("Dodaj krzywą B-sklejaną")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG("VectorMove"),_("Dodaj nowy oddzielny punkt")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG("VectorDelete"),_("Usuń element")));
		//ikony move all
		icons.push_back(new itemdata(PTR_BITMAP_PNG("MOVEPOS"),_("Przenieś punkty pozycjonowania")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG("MOVEMOVESTART"),_("Przenieś startowe punkty ruchu")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG("MOVE"),_("Przenieś końcowe punkty ruchu")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG("MOVECLIPS"),_("Przenieś wycinki")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG("MOVEDRAWINGS"),_("Przenieś rysunki,\nużywać tylko w przypadku,\ngdy chcemy przesunąć punkty rysunku,\nnie łączyć z trzema pierwszymi opcjami")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG("MOVEORGS"),_("Przenieś punkty org")));

	}
	Connect(wxEVT_PAINT, (wxObjectEventFunction)&VideoToolbar::OnPaint);
	Connect(wxEVT_LEFT_DOWN, (wxObjectEventFunction)&VideoToolbar::OnMouseEvent);
	Connect(wxEVT_LEFT_UP, (wxObjectEventFunction)&VideoToolbar::OnMouseEvent);
	Connect(wxEVT_MOTION, (wxObjectEventFunction)&VideoToolbar::OnMouseEvent);
	Connect(wxEVT_LEAVE_WINDOW, (wxObjectEventFunction)&VideoToolbar::OnMouseEvent);
	Connect(wxEVT_MOUSEWHEEL, (wxObjectEventFunction)&VideoToolbar::OnMouseEvent);
	MoveToggled[0]=true;
	for (int i = 1; i < moveToolsSize; i++){
		MoveToggled[i]=false;
	}
}

int VideoToolbar::GetToggled()
{
	return Toggled;
}

void VideoToolbar::OnMouseEvent(wxMouseEvent &evt)
{
	if (evt.GetWheelRotation() != 0) {
		if(blockScroll || !showClipTools){evt.Skip(); return;}
		int step = evt.GetWheelRotation() / evt.GetWheelDelta();
		clipToggled-=step;
		if(clipToggled < toolsSize){clipToggled=toolsSize+clipToolsSize-1;}
		else if(clipToggled >= toolsSize+clipToolsSize){clipToggled=toolsSize;}
		Refresh(false);
		return;
	}
	int x=evt.GetX();
	int w=0;
	int h=0;
	GetClientSize (&w, &h);
	bool noelem=false;
	int elem = (x - startDrawPos) / h;//startDrawPos +
	if(elem<0){noelem=true;}
	else if(elem>=toolsSize){
		int toolbarlen = h * clipToolsSize;
		int posx = (x - (w - toolbarlen));
		if(posx < 0 || posx > toolbarlen || (!showClipTools && !showMoveTools)){
			noelem=true;
		}
		else{ 
			elem = ( posx / h) + toolsSize;
		}
		if(showMoveTools){
			elem+=clipToolsSize;
		}
	}
	if(evt.Leaving() || noelem){sel = -1; Refresh(false); if(HasToolTips()){UnsetToolTip();} return;}
	
	if(elem != sel){
		sel=elem;
		SetToolTip(icons[elem]->help);
		Refresh(false);
	}
	if(evt.LeftDown()){
		if(elem>=moveToolsStart){
			int newi = elem-moveToolsStart; MoveToggled[newi] = !MoveToggled[newi];
		}
		else if(elem>=toolsSize){clipToggled=elem;}
		else{Toggled=elem;}
		clicked=true;
		Refresh(false);
	}
	if(evt.LeftUp()){
		clicked=false;
		Refresh(false);
		wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, (elem<toolsSize)? ID_VIDEO_TOOLBAR_EVENT : (elem< moveToolsStart)? ID_VECTOR_TOOLBAR_EVENT : ID_MOVE_TOOLBAR_EVENT);
		evt.SetInt((elem>=moveToolsStart)? GetMoveToggled() : (elem>=toolsSize)? clipToggled - toolsSize : Toggled);
		ProcessEvent(evt);
	}

}
	
void VideoToolbar::OnPaint(wxPaintEvent &evt)
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
	tdc.SetBrush(wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_MENUBAR)));
	tdc.SetPen(wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_MENUBAR)));
	tdc.DrawRectangle(0,0,w,h);
	//wxLogStatus("Paint");
	int posX = startDrawPos;
	int i = 0;
	while(i < toolsSize + clipToolsSize + moveToolsSize){
		if(i == toolsSize){
			if(showMoveTools){i=moveToolsStart; posX = w - (h * moveToolsSize);}
			else if(!showClipTools){break;}
			else{
				posX = w - (h * clipToolsSize);
			}
		}
		if(icons[i]->icon->IsOk()){
			bool toggled = i==Toggled || i==clipToggled || (i >= moveToolsStart && MoveToggled[i-moveToolsStart]);
			if(i==sel){
				tdc.SetBrush(wxBrush(wxSystemSettings::GetColour( (toggled)? wxSYS_COLOUR_MENUHILIGHT : wxSYS_COLOUR_MENUBAR)));
				tdc.SetPen(wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT)));
				tdc.DrawRoundedRectangle(posX, 1, h-2, h-2, 2.0);
			}else if(toggled){
				tdc.SetBrush(wxBrush(wxSystemSettings::GetColour((clicked && i==sel)? wxSYS_COLOUR_BTNFACE : wxSYS_COLOUR_HIGHLIGHT)));
				tdc.SetPen(wxPen(wxSystemSettings::GetColour((clicked && i==sel)? wxSYS_COLOUR_BTNSHADOW : wxSYS_COLOUR_HIGHLIGHT)));
				tdc.DrawRoundedRectangle(posX, 1, h-2, h-2, 2.0);
			}
			
			tdc.DrawBitmap(*(icons[i]->icon),posX+2,3);
			posX+=h;
		}
		i++;
	}

	wxPaintDC dc(this);
	dc.Blit(0,0,w,h,&tdc,0,0);
}
	
void VideoToolbar::OnSize(wxSizeEvent &evt)
{
	Refresh(false);
}

void VideoToolbar::Synchronize(VideoToolbar *vtoolbar){
	Toggled = vtoolbar->Toggled;
	clipToggled = vtoolbar->clipToggled;
	for(int i = 0; i < moveToolsSize; i++){
		MoveToggled[i] = vtoolbar->MoveToggled[i];
	}
	sel = vtoolbar->sel;
	clicked = vtoolbar->clicked;
	showClipTools = vtoolbar->showClipTools;
	showMoveTools = vtoolbar->showMoveTools;
	blockScroll = vtoolbar->blockScroll;
	if(IsShown()){Refresh(false);}
}