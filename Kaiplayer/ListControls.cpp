
#include "ListControls.h"
#include "Config.h"


static const wxFont font = wxFont(10,wxSWISS,wxFONTSTYLE_NORMAL,wxNORMAL,false,"Tahoma");
wxBitmap KaiChoice::normal;
wxBitmap KaiChoice::pushed;

	KaiChoice::KaiChoice(wxWindow *parent, int id, const wxPoint& pos,
    const wxSize& size, int n, const wxString choices[],
    long style, const wxValidator& validator)
{
	list = new wxArrayString(n,choices);
	KaiChoice(parent, id, pos, size, *list, style, validator);
}

KaiChoice::KaiChoice(wxWindow *parent, int id, const wxPoint& pos,
    const wxSize& size, const wxArrayString &choices,
    long style, const wxValidator& validator)
	:wxWindow(parent, id, pos, size, style)
	,bmp(NULL)
	,list(NULL)
	,listMenu(NULL)
	,listIsShown(false)
	,enter(false)
	,choiceChanged(false)
	,choice(-1)
{
	if(!list){list = new wxArrayString(choices);}
	listMenu= new Menu();
	Bind(wxEVT_PAINT, &KaiChoice::OnPaint, this);
	Bind(wxEVT_SIZE, &KaiChoice::OnSize, this);
	Bind(wxEVT_LEFT_DOWN, &KaiChoice::OnMouseEvent, this);
	Bind(wxEVT_LEFT_UP, &KaiChoice::OnMouseEvent, this);
	Bind(wxEVT_LEAVE_WINDOW, &KaiChoice::OnMouseEvent, this);
	Bind(wxEVT_ENTER_WINDOW, &KaiChoice::OnMouseEvent, this);
	Bind(wxEVT_MOUSEWHEEL, &KaiChoice::OnMouseEvent, this);
	Bind(wxEVT_KEY_UP, &KaiChoice::OnKeyPress, this);

	for(size_t i = 0; i < list->size(); i++){
		listMenu->Append(8000+i, (*list)[i]);
	}
	//SetMinSize(wxSize(24, 24));
	SetMaxSize(wxSize(1000, 50));
	if(!normal.IsOk()){normal = wxBITMAP_PNG("Button_Normal");}
	if(!pushed.IsOk()){pushed = wxBITMAP_PNG("Button_Pushed");}
	actualBmp=normal;
}

KaiChoice::~KaiChoice()
{
	delete list;
	delete listMenu;
	delete bmp;
}


void KaiChoice::OnSize(wxSizeEvent& event)
{
	Refresh(false);
}

void KaiChoice::OnPaint(wxPaintEvent& event)
{
	wxColour background = GetParent()->GetBackgroundColour();
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
	tdc.SetFont(font);
	tdc.SetBrush(wxBrush(background));
	tdc.SetPen(wxPen(background));
	tdc.DrawRectangle(0,0,w,h);
	if(actualBmp.IsOk()){
		wxImage img=actualBmp.ConvertToImage();
		img = img.Scale(w, h, wxIMAGE_QUALITY_BICUBIC);
		tdc.DrawBitmap(wxBitmap(img), 0, 0);
	}
	//wxBitmap end=normal.GetSubBitmap(wxRect(86-diff,0,diff,5));
	wxPoint points[3];
	int pos1 = h/2;
	int pos  = w - 10;
	points[0]=wxPoint(pos-6,pos1-2);
	points[1]=wxPoint(pos,pos1-2);
	points[2]=wxPoint(pos-3,pos1+2);
	tdc.SetPen(*wxTRANSPARENT_PEN);
	tdc.SetBrush(wxBrush("#000000"));
	tdc.DrawPolygon(3,points);
	if(choice>=0){
		int fh=0, fw=w, ex=0, et=0;
		wxString txt = (*list)[choice];
		int removed=0;
		while(fw > w - 15){
			tdc.GetTextExtent(txt, &fh, &fw, &ex, &et, &font);
			txt = txt.RemoveLast();
			removed++;
		}
		if(removed<2){
			txt = (*list)[choice];
		}else{
			txt = txt.RemoveLast(2)+"...";
		}
		tdc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
		tdc.DrawText(txt, 4, (h-fh));
		
	}
	wxPaintDC dc(this);
	dc.Blit(0,0,w,h,&tdc,0,0);
}

void KaiChoice::OnMouseEvent(wxMouseEvent &event)
{
	if(event.Entering()){
		enter=true;
		wxImage img=normal.ConvertToImage();
		int size=normal.GetWidth()*normal.GetHeight()*3;
		byte *data=img.GetData();
			
		for(int i=0; i<size; i++)
		{
			if(data[i]<226){data[i]+=30;}
		}
		tmpbmp = wxBitmap(img);
		SetBitmap(tmpbmp);
		
		return;
	}
	if(event.Leaving()&&enter){
		enter=false;
		SetBitmap(normal);
		return;
	}
			
	if(event.LeftDown()){
		SetBitmap(pushed);
		if(listIsShown){
			listMenu->HideMenu();
		}else{
			ShowList();
		}
	}
	if(event.LeftUp()){
		
		SetBitmap(tmpbmp);
		if(choiceChanged){
			wxCommandEvent evt(wxEVT_COMMAND_CHOICE_SELECTED, GetId());
			this->ProcessEvent(evt);
			choiceChanged=false;
		}
	}
	if (event.GetWheelRotation() != 0) {
		if(!HasFocus()){event.Skip(); return;}
		int step = event.GetWheelRotation() / event.GetWheelDelta();
		choice -=step;
		if(choice<0){choice=0;}
		else if(choice > (int)list->size()){choice = list->size();}
		Refresh(false);
		if(choiceChanged){
			wxCommandEvent evt(wxEVT_COMMAND_CHOICE_SELECTED, GetId());
			this->ProcessEvent(evt);
			choiceChanged=false;
		}
	}
}

void KaiChoice::OnKeyPress(wxKeyEvent &event)
{
	if(event.GetKeyCode() == WXK_RETURN){
		ShowList();
	}
}

void KaiChoice::ShowList()
{
	listIsShown = true;
	int elem = listMenu->GetPopupMenuSelection(wxPoint(0, GetSize().GetY()), this)-8000;
	if(elem>=0){choice = elem; choiceChanged=true; Refresh(false);}
	listIsShown=false; 
}


void KaiChoice::SetSelection(int sel)
{
	choice=sel; Refresh(false);
}
	
void KaiChoice::Clear()
{
	list->Clear();
	listMenu->Clear();
}
	
void KaiChoice::Append(wxString what)
{
	list->Add(what);
	listMenu->Append(8000+list->size()-1, what);
}
	
//void KaiChoice::Prepend(wxString what)
//{
//
//}
//	
//void KaiChoice::Insert(wxString what, int position)
//{
//
//}