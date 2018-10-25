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


#include "FontDialog.h"
#include "FontEnumerator.h"
#include <wx/regex.h>
#include "Config.h"
#include "SubsGrid.h"
#include "KaiStaticBoxSizer.h"

wxDEFINE_EVENT(FONT_CHANGED, wxCommandEvent);
wxDEFINE_EVENT(SELECTION_CHANGED, wxCommandEvent);

FontList::FontList(wxWindow *parent,long id,const wxPoint &pos, const wxSize &size)
	:wxWindow(parent,id,pos,size)
{
	scrollBar = new KaiScrollbar(this,ID_SCROLL1,wxDefaultPosition,wxDefaultSize,wxSB_VERTICAL);
	scrollBar->SetScrollbar(0,10,100,10);
	fonts = FontEnum.GetFonts(this, [=](){
		fonts = FontEnum.GetFonts(0,[](){});
		Refresh(false);
	});

	font= wxFont(10,wxSWISS,wxFONTSTYLE_NORMAL,wxNORMAL,false,"Tahoma",wxFONTENCODING_DEFAULT);

	wxClientDC dc(this);
	dc.SetFont(font);
	int fw,fh;
	dc.GetTextExtent("#TWFfGH", &fw, &fh, NULL, NULL, &font);
	Height=fh+4;

	bmp=NULL;
	scPos=0;
	sel=0;
	holding=false;
	Refresh();

}

FontList::~FontList(){
	FontEnum.RemoveClient(this);
	if(bmp)delete bmp; bmp=0;
}



void FontList::OnPaint(wxPaintEvent& event)
{
	wxPaintDC dc(this);
	int w = 0;
	int h = 0;
	int sw=0;
	int sh=0;
	GetClientSize(&w,&h);
	scrollBar->GetSize(&sw,&sh);
	scrollBar->SetSize(w-sw,0,sw,h);
	w -= sw;

	bool direct = false;

	if (direct) {
		DrawFld(dc,w,h);
	}

	else {

		if (bmp) {
			if (bmp->GetWidth() < w || bmp->GetHeight() < h) {
				delete bmp;
				bmp = NULL;
			}
		}
		if (!bmp) bmp = new wxBitmap(w,h);

		// Draw bitmap
		wxMemoryDC bmpDC;
		bmpDC.SelectObject(*bmp);
		DrawFld(bmpDC,w,h);
		dc.Blit(0,0,w,h,&bmpDC,0,0);
	}

}

void FontList::DrawFld(wxDC &dc,int w, int h)
{
	int fw=0,fh=0,posX=1,posY=1;

	dc.SetPen(wxPen(Options.GetColour(StaticListBorder)));
	dc.SetBrush(wxBrush(Options.GetColour(StaticListBackground)));
	dc.SetTextForeground(Options.GetColour(WindowText));
	dc.DrawRectangle(0,0,w,h);


	int panelrows=(h/Height)+1;
	int scrows;
	if(scPos<0){scPos=0;}
	if((scPos+panelrows)>=(int)fonts->size()+1){
		scrows=fonts->size();
		scPos=(scrows-panelrows)+1;
		if(panelrows>(int)fonts->size()){
			scPos=0;
			scrollBar->Enable(false);
		}else{
			scrollBar->SetScrollbar(scPos,panelrows,fonts->size()+1,panelrows-1);}
	}
	else{
		scrows=(scPos+panelrows);
		scrollBar->Enable(true);
		scrollBar->SetScrollbar(scPos,panelrows,fonts->size()+1,panelrows-1);
	}


	for(int i=scPos; i<scrows; i++)
	{
		if(i==sel){
			const wxColour & selection = Options.GetColour(StaticListSelection);
			dc.SetPen(wxPen(selection));
			dc.SetBrush(wxBrush(selection));
			dc.DrawRectangle(posX,posY,w-2,Height);
		}

		font.SetFaceName((*fonts)[i]);
		dc.SetFont(font);

		wxRect cur(posX+4,posY,w-8,Height);	
		dc.SetClippingRegion(cur);
		dc.DrawLabel((*fonts)[i],cur,wxALIGN_CENTER_VERTICAL | wxALIGN_LEFT);
		dc.DestroyClippingRegion();

		posY+=Height;
	}

}

void FontList::OnSize(wxSizeEvent& event)
{
	Refresh(false);
}

void FontList::OnScroll(wxScrollEvent& event)
{
	int newPos = event.GetPosition();

	if (scPos != newPos) {
		scPos = newPos;
		Refresh(false);
	}
}

void FontList::OnMouseEvent(wxMouseEvent& event)
{
	int w,h;
	GetClientSize (&w, &h);
	bool click = event.LeftDown();
	bool left_up = event.LeftUp();
	bool dclick = event.LeftDClick();
	int curY=(event.GetY());
	int row = (curY / Height)+scPos;

	if (left_up && !holding) {
		return;
	}

	// Get focus
	if (event.ButtonDown()) {
		SetFocus();}

	if (event.GetWheelRotation() != 0) {
		int step = 3 * event.GetWheelRotation() / event.GetWheelDelta();
		scPos -= step;
		Refresh(false);
		return;
	}

	if (left_up && holding) {
		holding = false;
		ReleaseMouse();
	}

	if (row < scPos || row >= (int)fonts->size()) {
		return;
	}

	if (click) {
		holding = true;
		sel=row;
		wxCommandEvent evt(wxEVT_COMMAND_LISTBOX_SELECTED, GetId());
		AddPendingEvent(evt);
		Refresh(false);
		CaptureMouse();
	}



	if (holding) {
		// Find direction
		int minVis = scPos+1;
		int maxVis = scPos+h/Height-1;
		int delta = 0;
		if (row < minVis && row!=0) delta = -1;
		if (row > maxVis) delta = +1;

		if (delta) {
			scPos=(MID(row - (h / Height), scPos + delta, row));
			Refresh(false);
			// End the hold if this was a mousedown to avoid accidental
			// selection of extra lines
			if (click) {
				holding = false;
				left_up = true;
				ReleaseMouse();
			}
		}

		if(sel!=row){
			sel=row;
			Refresh(false);
			wxCommandEvent evt(wxEVT_COMMAND_LISTBOX_SELECTED, GetId());
			AddPendingEvent(evt);
		}
	}


}

void FontList::SetSelection(int pos)
{
	if(pos<0 || pos>=(int)fonts->size()){wxBell();return;}
	if ((scPos<pos || scPos>pos+7) && scPos>=sel && scPos<sel+8){
		scPos -= (sel-pos);
	}else{
		scPos = pos-3;
	}
	sel=pos;

	Refresh(false);
	wxCommandEvent evt(SELECTION_CHANGED, GetId());
	AddPendingEvent(evt);
}

void FontList::SetSelectionByName(wxString name)
{
	int sell=fonts->Index(name,false);
	if(sell!=-1){SetSelection(sell);}
}


void FontList::SetSelectionByPartialName(wxString PartialName)
{
	if(PartialName==""){SetSelection(0);return;}
	int sell=-1;
	PartialName=PartialName.Lower();

	size_t k = 0;

	for (size_t i = 0; i < fonts->size(); i++){
		wxString fontname = (*fonts)[i].Lower();
		if (fontname.Len() < 1 || fontname[0] < PartialName[0])
			continue;

		while (k < PartialName.Len() && k < fontname.Len()){
			if (fontname[k] == PartialName[k]){
				k++;
				if (k >= PartialName.Len()){
					sell = i;
					goto done;
				}
			}
			else if (fontname[k] > PartialName[k]){
				sell = i;
				goto done;
			}
			else
				break;
		}
	}

done:

	if(sell!=-1){
		SetSelection(sell);
	}
}

wxString FontList::GetString(int line)
{
	if(line<0||line>=(int)fonts->size()) return "";
	return (*fonts)[line];
}

int FontList::GetSelection()
{
	return sel;
}

void FontList::Scroll(int step)
{
	sel+=step;
	scPos+=step;
	if (scPos<sel-7 || scPos>sel){scPos=sel;}
	Refresh(false);
}

BEGIN_EVENT_TABLE(FontList, wxWindow)
EVT_PAINT(FontList::OnPaint)
EVT_SIZE(FontList::OnSize)
EVT_COMMAND_SCROLL(ID_SCROLL1, FontList::OnScroll)
EVT_MOUSE_EVENTS(FontList::OnMouseEvent)
//EVT_KEY_DOWN(FontList::OnKeyPress)
END_EVENT_TABLE()

FontDialog *FontDialog::FDialog = NULL;

FontDialog::FontDialog(wxWindow *parent, Styles *acst)
	:KaiDialog(parent,-1,_("Wybierz czcionkę"))
{
	editedStyle = acst;
	SetForegroundColour(Options.GetColour(WindowText));
	SetBackgroundColour(Options.GetColour(WindowBackground));
	wxAcceleratorEntry entries[4];
	entries[0].Set(wxACCEL_NORMAL, WXK_RETURN, wxID_OK);
	entries[1].Set(wxACCEL_NORMAL, WXK_ESCAPE, wxID_CANCEL);
	entries[2].Set(wxACCEL_NORMAL, WXK_UP,ID_SCROLLUP);
	entries[3].Set(wxACCEL_NORMAL, WXK_DOWN,ID_SCROLLDOWN);
	wxAcceleratorTable accel(4, entries);
	SetAcceleratorTable(accel);


	DialogSizer *Main= new DialogSizer(wxVERTICAL);
	KaiStaticBoxSizer *Cfont= new KaiStaticBoxSizer(wxHORIZONTAL,this,_("Czcionka"));
	KaiStaticBoxSizer *prev= new KaiStaticBoxSizer(wxVERTICAL,this,_("Podgląd"));
	wxBoxSizer *Fattr= new wxBoxSizer(wxVERTICAL);
	//wxBoxSizer *Flist= new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *Bsizer= new wxBoxSizer(wxHORIZONTAL);
	FontName = new KaiTextCtrl(this,ID_FONT_NAME,acst->Fontname,wxDefaultPosition,wxSize(150,-1),wxTE_PROCESS_ENTER);

	Fonts= new FontList(this,ID_FONTLIST,wxDefaultPosition,wxSize(180,-1));
	Fonts->SetSelectionByName(acst->Fontname);
	//Flist->Add(FontName,0,wxEXPAND|wxBOTTOM,3);
	//Flist->Add(Fonts,0,wxEXPAND);

	Preview = new StylePreview(this,-1,wxDefaultPosition, wxSize(-1,110));
	Preview->DrawPreview(acst);
	FontSize= new NumCtrl(this,ID_FONTSIZE1,acst->Fontsize,1,10000,false,wxDefaultPosition,wxDefaultSize,wxTE_PROCESS_ENTER);
	Bold= new KaiCheckBox(this,ID_FONTATTR,_("Pogrubienie"));
	Bold->SetValue(acst->Bold);
	Italic= new KaiCheckBox(this,ID_FONTATTR,_("Kursywa"));
	Italic->SetValue(acst->Italic);
	Underl= new KaiCheckBox(this,ID_FONTATTR,_("Podkreślenie"));
	Underl->SetValue(acst->Underline);
	Strike= new KaiCheckBox(this,ID_FONTATTR,_("Przekreślenie"));
	Strike->SetValue(acst->StrikeOut);
	//MappedButton *ButtApply = new MappedButton(this, wxID_APPLY, "Zastosuj");
	Buttok= new MappedButton(this,wxID_OK,"OK");
	Buttcancel= new MappedButton(this,8999,_("Anuluj"));
	SetEscapeId(8999);
	Fattr->Add(FontName,0,wxEXPAND|wxLEFT|wxRIGHT,5);
	Fattr->Add(FontSize,0,wxEXPAND|wxALL,5);
	Fattr->Add(Bold,1,wxEXPAND|wxALL,5);
	Fattr->Add(Italic,1,wxEXPAND|wxALL,5);
	Fattr->Add(Underl,1,wxEXPAND|wxALL,5);
	Fattr->Add(Strike,1,wxEXPAND|wxALL,5);

	Cfont->Add(Fonts,0,wxEXPAND);
	Cfont->Add(Fattr,0,wxEXPAND);

	prev->Add(Preview,0,wxEXPAND|wxALL,5);

	//Bsizer->Add(ButtApply, 1, wxALL, 5);
	Bsizer->Add(Buttok,1,wxALL,5);
	Bsizer->Add(Buttcancel,1,wxALL,5);

	Main->Add(Cfont,0,wxEXPAND|wxALL,5);
	Main->Add(prev,0,wxEXPAND|wxBOTTOM|wxLEFT|wxRIGHT,5);
	Main->Add(Bsizer,0,wxBOTTOM|wxALIGN_CENTER,5);

	SetSizerAndFit(Main);

	UpdatePreview();

	fontChangedTimer.SetOwner(this, 12345);
	Bind(wxEVT_TIMER, [=](wxTimerEvent & event){
		wxCommandEvent evt(FONT_CHANGED, GetId());
		evt.SetClientData(this);
		AddPendingEvent(evt);
	}, 12345);

	Connect(ID_FONTLIST,wxEVT_COMMAND_LISTBOX_SELECTED,(wxObjectEventFunction)&FontDialog::OnFontChanged);
	Connect(ID_FONTLIST, SELECTION_CHANGED, (wxObjectEventFunction)&FontDialog::OnUpdatePreview);
	Connect(ID_FONT_NAME,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&FontDialog::OnUpdateText);
	Connect(ID_FONTSIZE1, NUMBER_CHANGED, (wxObjectEventFunction)&FontDialog::OnUpdatePreview);
	Connect(ID_FONTATTR,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&FontDialog::OnUpdatePreview);
	Connect(ID_SCROLLUP,ID_SCROLLDOWN,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&FontDialog::OnScrollList);
	//Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent & evt){
	//	//fontChangedTimer.Start()
	//}, wxID_APPLY);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent & evt){
		if (editedStyle){
			delete editedStyle;
			editedStyle = NULL;
		}
		if (resultStyle){
			delete resultStyle;
			resultStyle = NULL;
		}
		if (IsModal())
			EndModal(wxID_CANCEL);
		else
			Show(false);

	}, 8999);

	MoveToMousePosition(this);
}

void FontDialog::SetStyle()
{
	FontName->SetValue(editedStyle->Fontname);
	Fonts->SetSelectionByName(editedStyle->Fontname);
	FontSize->SetValue(editedStyle->Fontsize);
	Bold->SetValue(editedStyle->Bold);
	Italic->SetValue(editedStyle->Italic);
	Underl->SetValue(editedStyle->Underline);
	Strike->SetValue(editedStyle->StrikeOut);
	Preview->DrawPreview(editedStyle);
}

FontDialog::~FontDialog()
{
	if (editedStyle){
		delete editedStyle;
		editedStyle = NULL;
	}
	if (resultStyle){
		delete resultStyle;
		resultStyle = NULL;
	}
	FDialog = NULL;
}

void FontDialog::GetStyles(Styles **inputStyle, Styles **outputStyle)
{
	if (inputStyle != NULL && outputStyle != NULL){
		*inputStyle = editedStyle;
		if (!resultStyle)
			bool thisisbad = true;
		*outputStyle = (resultStyle) ? resultStyle : GetFont();
	}
}

Styles * FontDialog::GetFont()
{
	if (resultStyle){
		if (editedStyle){
			delete editedStyle;
			editedStyle = resultStyle;
		}
		else{
			delete resultStyle;
		}
		resultStyle = NULL;
	}
	resultStyle = new Styles();
	resultStyle->Bold = Bold->GetValue();
	resultStyle->Italic = Italic->GetValue();
	resultStyle->Underline = Underl->GetValue();
	resultStyle->StrikeOut = Strike->GetValue();
	resultStyle->Fontname = Fonts->GetString(Fonts->GetSelection());
	wxString fontSize;
	fontSize << FontSize->GetValue();
	if (fontSize != ""){ resultStyle->Fontsize = fontSize; }
	return resultStyle;
}

FontDialog * FontDialog::Get(wxWindow *parent, Styles *actualStyle)
{
	if (FDialog && FDialog->GetParent() != parent){
		FDialog->Destroy();
		FDialog = NULL;
	}
	if (!FDialog)
		FDialog = new FontDialog(parent, actualStyle);
	else{
		if (FDialog->editedStyle){
			delete FDialog->editedStyle;
			FDialog->editedStyle = NULL;
		}
		FDialog->editedStyle = actualStyle;
		if (FDialog->resultStyle)
			delete FDialog->resultStyle;
		FDialog->resultStyle = NULL;
		FDialog->SetStyle();
		MoveToMousePosition(FDialog);
	}

	return FDialog;
}

void FontDialog::OnFontChanged(wxCommandEvent& event)
{
	FontName->SetValue(Fonts->GetString(Fonts->GetSelection()));
	UpdatePreview();
}

void FontDialog::OnUpdatePreview(wxCommandEvent& event)
{
	UpdatePreview();
}

void FontDialog::UpdatePreview()
{
	Styles *styl=GetFont();
	Preview->DrawPreview(styl);
	fontChangedTimer.Start(200, true);
}

void FontDialog::OnUpdateText(wxCommandEvent& event)
{
	Fonts->SetSelectionByPartialName(FontName->GetValue());
}

void FontDialog::OnScrollList(wxCommandEvent& event)
{

	int step=(event.GetId()==ID_SCROLLUP)? -1 : 1;
	Fonts->Scroll(step);
	FontName->SetValue(Fonts->GetString(Fonts->GetSelection()));
	UpdatePreview();
}

FontPickerButton::FontPickerButton(wxWindow *parent, int id, const wxFont& font,
								   const wxPoint& pos, const wxSize& size, long style)
								   : MappedButton(parent, id, font.GetFaceName() + " " + std::to_string(font.GetPointSize()), -1, pos, size, style)
{
	ChangeFont(font);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &FontPickerButton::OnClick, this, GetId());
}

void FontPickerButton::ChangeFont(const wxFont &font)
{
	SetLabelText(font.GetFaceName() + " " + std::to_string(font.GetPointSize()));
	SetFont(font);
	int fw, fh;
	GetTextExtent(GetLabelText(), &fw, &fh);
	wxSize newSize = GetMinSize();
	bool isChanged = false;
	if(newSize.x < fw+16){
		newSize.x = fw+16;
		isChanged = true;
	}
	if(newSize.y < fh+10){
		newSize.y = fh+10;
		isChanged = true;
	}
	if(isChanged){SetMinSize(newSize);}
	GetParent()->Layout();
}

wxFont FontPickerButton::GetSelectedFont(){
	return GetFont();
}

void FontPickerButton::OnClick(wxCommandEvent &evt)
{
	wxFont font = GetFont();
	Styles *mstyle=new Styles();
	mstyle->Fontname = font.GetFaceName();
	mstyle->Fontsize = std::to_string(font.GetPointSize());
	FontDialog * FD = FontDialog::Get(this, mstyle);
	if(FD->ShowModal()==wxID_OK){
		Styles *retstyle = FD->GetFont();
		font.SetFaceName(retstyle->Fontname);
		font.SetPointSize(wxAtoi(retstyle->Fontsize));
		ChangeFont(font);
		//delete retstyle;
	}
}

wxIMPLEMENT_ABSTRACT_CLASS(FontPickerButton, MappedButton);