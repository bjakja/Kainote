#include "VideoToolbar.h"
#include "Config.h"

const static int toolsSize = 9;
const static int clipToolsSize = 5;
static int startDrawPos = 5;
std::vector< wxBitmap*> VideoToolbar::icons;

VideoToolbar::VideoToolbar (wxWindow *parent, const wxPoint &pos, const wxSize &size)
	:wxWindow(parent, -1, pos, size)
	,Toggled(0)
	,clicked(false)
{
	if(icons.size()==0){
		wxString name;
		for(int i = 0; i < toolsSize + clipToolsSize; i++)
		{
			name = "VISUALTOOL";
			name << i+1;
			icons.push_back(&wxBITMAP_PNG(name));
		}
	}
}

int VideoToolbar::GetToggled()
{
	return Toggled;
}

void VideoToolbar::OnMouseEvent(wxMouseEvent &evt)
{
	int w=0;
	int h=0;
	GetClientSize (&w, &h);
	



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
	int posX = startDrawPos;
	for(int i = 0; i < toolsSize; i++){
		if(icons[i]->IsOk()){
			if(i==Toggled){
				tdc.SetBrush(wxBrush(wxSystemSettings::GetColour(clicked? wxSYS_COLOUR_BTNFACE : wxSYS_COLOUR_HIGHLIGHT)));
				tdc.SetPen(wxPen(wxSystemSettings::GetColour(clicked? wxSYS_COLOUR_BTNSHADOW : wxSYS_COLOUR_HIGHLIGHT)));
				tdc.DrawRoundedRectangle(posX, 2, h-4, h-4, 2.0);
			}
			tdc.DrawBitmap(*(icons[i]),posX,4);
			posX+=h;
		}
	}
	if(showClipTools){
		posX = w - (h * clipToolsSize);
		for(int i = toolsSize; i < toolsSize + clipToolsSize; i++){
			if(icons[i]->IsOk()){
				tdc.DrawBitmap(*(icons[i]),posX,4);
				posX+=h;
			}
		}

	}
}
	
void VideoToolbar::OnSize(wxSizeEvent &evt)
{
	Refresh(false);
}