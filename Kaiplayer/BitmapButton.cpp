#include "BitmapButton.h"
#include "Config.h"

BitmapButton::BitmapButton(wxWindow* parent, wxBitmap bitmap,wxBitmap bitmap1, int id, const wxPoint& pos, const wxSize& size)
	:  wxStaticBitmap(parent, id, bitmap,pos,size)
{
	idd=id;
	enter=false;
	bmp=bitmap;
	bmp1=bitmap1;
	Bind(wxEVT_LEFT_DOWN, &BitmapButton::OnLeftDown, this);
	Bind(wxEVT_LEFT_UP, &BitmapButton::OnLeftDown, this);
	Bind(wxEVT_LEAVE_WINDOW, &BitmapButton::OnLeftDown, this);
	Bind(wxEVT_ENTER_WINDOW, &BitmapButton::OnLeftDown, this);
}
    
BitmapButton::~BitmapButton()
{

}


void BitmapButton::ChangeBitmap(bool play)
{
	wxString fbmp=(play)? "play" : "pause";
	bmp = CreateBitmapFromPngResource(fbmp);
	bmp1 = CreateBitmapFromPngResource(fbmp+"1");
	if(enter){
		img=bmp.ConvertToImage();
		int size=bmp.GetWidth()*bmp.GetHeight()*3;
		byte *data=img.GetData();
		for(int i=0; i<size; i++)
		{
			if(data[i]<226){data[i]+=30;}
		}
		SetBitmap(wxBitmap(img));
	}else{
		SetBitmap(bmp);
	}
}


void BitmapButton::OnLeftDown(wxMouseEvent& event)
{
	if(event.Entering()){
		enter=true;
		img=bmp.ConvertToImage();
		int size=bmp.GetWidth()*bmp.GetHeight()*3;
		byte *data=img.GetData();
			
		for(int i=0; i<size; i++)
		{
			if(data[i]<226){data[i]+=30;}
		}
		SetBitmap(wxBitmap(img));
		
		return;
	}
	if(event.Leaving()&&enter){
		enter=false;
		SetBitmap(bmp);
		return;
	}
			
	if(event.LeftDown()){
		SetBitmap(bmp1);
	}
	if(event.LeftUp()){
		
		SetBitmap(wxBitmap(img));
		wxCommandEvent evt(wxEVT_COMMAND_BUTTON_CLICKED,idd);this->ProcessEvent(evt);
	}
}
	
