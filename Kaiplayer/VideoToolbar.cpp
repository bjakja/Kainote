#include "VideoToolbar.h"
#include "Config.h"


static int startDrawPos = 5;
std::vector< itemdata *> VideoToolbar::icons;

VideoToolbar::VideoToolbar (wxWindow *parent, const wxPoint &pos)
	:wxWindow(parent, -1, pos, wxSize(-1, 22))
	,Toggled(0)
	,clipToggled(toolsSize)
	,sel(-1)
	,clicked(false)
	,showClipTools(false)
	,blockScroll(false)
	,bmp(NULL)
{
	if(icons.size()==0){
		//pamiêtaj, dodaj¹c tutaj elementy, zmieñ ich wartoœæ w pliku h!!
		icons.push_back(new itemdata(PTR_BITMAP_PNG("cross"),_("WskaŸnik pozycji")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG("position"),_("Przesuwanie tekstu")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG("move"),_("Ruch")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG("scale"),_("Skalowanie")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG("frz"),_("Obrót wokó³ osi Z")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG("frxy"),_("Obrót wokó³ osi X / Y")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG("cliprect"),_("Wycinki prostok¹tne")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG("clip"),_("Wycinki wektorowe")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG("drawing"),_("Rysunki wektorowe")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG("MOVEAll"),_("Przesuwanie wielu tagów")));
		//tutaj mamy ikony dla clipa
		icons.push_back(new itemdata(PTR_BITMAP_PNG("VectorDrag"),_("Przesuñ punkty")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG("VectorLine"),_("Dodaj liniê")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG("VectorBezier"),_("Dodaj krzyw¹ Beziera")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG("VECTORBSPLINE"),_("Dodaj krzyw¹ B-sklejan¹")));
		//icons.push_back(new itemdata(PTR_BITMAP_PNG("VECTORBSPLINEPOINT"),_("Dodaj punkt krzywej B-sklejanej")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG("VectorMove"),_("Dodaj nowy oddzielny punkt")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG("VectorDelete"),_("Usuñ element")));
	}
	Connect(wxEVT_PAINT, (wxObjectEventFunction)&VideoToolbar::OnPaint);
	Connect(wxEVT_LEFT_DOWN, (wxObjectEventFunction)&VideoToolbar::OnMouseEvent);
	Connect(wxEVT_LEFT_UP, (wxObjectEventFunction)&VideoToolbar::OnMouseEvent);
	Connect(wxEVT_MOTION, (wxObjectEventFunction)&VideoToolbar::OnMouseEvent);
	Connect(wxEVT_LEAVE_WINDOW, (wxObjectEventFunction)&VideoToolbar::OnMouseEvent);
	Connect(wxEVT_MOUSEWHEEL, (wxObjectEventFunction)&VideoToolbar::OnMouseEvent);
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
		if(posx < 0 || posx > toolbarlen || !showClipTools){
			noelem=true;
		}
		else{ elem = ( posx / h) + toolsSize;}
	}
	if(evt.Leaving() || noelem){sel = -1; Refresh(false); if(HasToolTips()){UnsetToolTip();} return;}
	
	if(elem != sel){
		sel=elem;
		SetToolTip(icons[elem]->help);
		Refresh(false);
	}
	if(evt.LeftDown()){
		if(elem>=toolsSize){clipToggled=elem;}
		else{Toggled=elem;}
		clicked=true;
		Refresh(false);
	}
	if(evt.LeftUp()){
		clicked=false;
		Refresh(false);
		wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, (elem<toolsSize)? ID_VIDEO_TOOLBAR_EVENT : ID_AUX_TOOLBAR_EVENT);
		evt.SetInt((elem>=toolsSize)? clipToggled - toolsSize : Toggled);
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
	for(int i = 0; i < toolsSize + clipToolsSize; i++){
		if(i == toolsSize){
			if(!showClipTools){break;}
			else{
				posX = w - (h * clipToolsSize);
			}
		}
		if(icons[i]->icon->IsOk()){
			if(i==sel){
				tdc.SetBrush(wxBrush(wxSystemSettings::GetColour((i==Toggled || i==clipToggled)? wxSYS_COLOUR_MENUHILIGHT : wxSYS_COLOUR_MENUBAR)));
				tdc.SetPen(wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT)));
				tdc.DrawRoundedRectangle(posX, 1, h-2, h-2, 2.0);
			}else if(i==Toggled || i==clipToggled){
				tdc.SetBrush(wxBrush(wxSystemSettings::GetColour((clicked && i==sel)? wxSYS_COLOUR_BTNFACE : wxSYS_COLOUR_HIGHLIGHT)));
				tdc.SetPen(wxPen(wxSystemSettings::GetColour((clicked && i==sel)? wxSYS_COLOUR_BTNSHADOW : wxSYS_COLOUR_HIGHLIGHT)));
				tdc.DrawRoundedRectangle(posX, 1, h-2, h-2, 2.0);
			}
			
			tdc.DrawBitmap(*(icons[i]->icon),posX+2,3);
			posX+=h;
		}
	}

	wxPaintDC dc(this);
	dc.Blit(0,0,w,h,&tdc,0,0);
}
	
void VideoToolbar::OnSize(wxSizeEvent &evt)
{
	Refresh(false);
}
