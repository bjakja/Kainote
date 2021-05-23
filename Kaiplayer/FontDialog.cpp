//  Copyright (c) 2016-2020, Marcin Drob

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
#include <wx/dcclient.h>
#include "Config.h"
#include "Utils.h"
#include "SubsGrid.h"
#include "KaiStaticBoxSizer.h"
#include "FontCatalogList.h"

wxDEFINE_EVENT(FONT_CHANGED, wxCommandEvent);
wxDEFINE_EVENT(SELECTION_CHANGED, wxCommandEvent);

FontList::FontList(wxWindow *parent, long id, const wxPoint &pos, const wxSize &size)
	:wxWindow(parent, id, pos, size)
{
	fonts = new wxArrayString();
	scrollBar = new KaiScrollbar(this, ID_SCROLL1, wxDefaultPosition, wxDefaultSize, wxSB_VERTICAL);
	scrollBar->SetScrollbar(0, 10, 100, 10);

	font = *Options.GetFont(4);
	int fw, fh;
	GetTextExtent(L"#TWFfGH", &fw, &fh, NULL, NULL, &font);
	Height = fh + 4;

	bmp = NULL;
	scPos = 0;
	sel = 0;
	holding = false;
	Bind(wxEVT_ERASE_BACKGROUND, [=](wxEraseEvent& evt) {});
	Bind(wxEVT_SET_FOCUS, [=](wxFocusEvent& evt) {Refresh(false); });
	Bind(wxEVT_KILL_FOCUS, [=](wxFocusEvent& evt) {Refresh(false); });
}

FontList::~FontList(){
	FontEnum.RemoveClient(this);
	if (bmp){ delete bmp; bmp = 0; }
	if (fonts) { delete fonts; fonts = NULL; }
}



void FontList::OnPaint(wxPaintEvent& event)
{
	int w = 0;
	int h = 0;
	int sw = 0;
	int sh = 0;
	GetClientSize(&w, &h);
	scrollBar->GetSize(&sw, &sh);
	scrollBar->SetSize(w - sw, 0, sw, h);
	w -= sw;

	if (bmp) {
		if (bmp->GetWidth() < w || bmp->GetHeight() < h) {
			delete bmp;
			bmp = NULL;
		}
	}
	if (!bmp) bmp = new wxBitmap(w, h);

	// Draw bitmap
	wxMemoryDC dc;
	dc.SelectObject(*bmp);

	int fw = 0, fh = 0, posX = 1, posY = 1;

	dc.SetPen(wxPen(HasFocus()? Options.GetColour(TEXT_FIELD_BORDER_ON_FOCUS) : 
		Options.GetColour(STATICLIST_BORDER)));
	dc.SetBrush(wxBrush(Options.GetColour(STATICLIST_BACKGROUND)));
	dc.SetTextForeground(Options.GetColour(WINDOW_TEXT));
	dc.DrawRectangle(0, 0, w, h);


	int panelrows = (h / Height) + 1;
	int scrows;
	if (scPos < 0){ scPos = 0; }
	if ((scPos + panelrows) >= (int)fonts->size() + 1){
		scrows = fonts->size();
		scPos = (scrows - panelrows) + 1;
		if (panelrows > (int)fonts->size()){
			scPos = 0;
			scrollBar->Enable(false);
		}
		else{
			scrollBar->SetScrollbar(scPos, panelrows, fonts->size() + 1, panelrows - 1);
		}
	}
	else{
		scrows = (scPos + panelrows);
		scrollBar->Enable(true);
		scrollBar->SetScrollbar(scPos, panelrows, fonts->size() + 1, panelrows - 1);
	}


	for (int i = scPos; i < scrows; i++)
	{
		if (i == sel){
			const wxColour & selection = Options.GetColour(STATICLIST_SELECTION);
			dc.SetPen(wxPen(selection));
			dc.SetBrush(wxBrush(selection));
			dc.DrawRectangle(posX, posY, w - 2, Height);
		}

		font.SetFaceName((*fonts)[i]);
		dc.SetFont(font);

		wxRect cur(posX + 4, posY, w - 8, Height);
		dc.SetClippingRegion(cur);
		dc.DrawLabel((*fonts)[i], cur, wxALIGN_CENTER_VERTICAL | wxALIGN_LEFT);
		dc.DestroyClippingRegion();

		posY += Height;
	}

	wxPaintDC pdc(this);
	pdc.Blit(0, 0, w, h, &dc, 0, 0);
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
	int w, h;
	GetClientSize(&w, &h);
	bool click = event.LeftDown();
	bool left_up = event.LeftUp();
	bool dclick = event.LeftDClick();
	int curY = (event.GetY());
	int row = (curY / Height) + scPos;

	if (left_up && !holding) {
		return;
	}

	// Get focus
	if (event.ButtonDown()) {
		SetFocus();
	}

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
		sel = row;
		wxCommandEvent evt(wxEVT_COMMAND_LISTBOX_SELECTED, GetId());
		AddPendingEvent(evt);
		Refresh(false);
		CaptureMouse();
	}



	if (holding) {
		// Find direction
		int minVis = scPos + 1;
		int maxVis = scPos + h / Height - 1;
		int delta = 0;
		if (row < minVis && row != 0) delta = -1;
		if (row > maxVis) delta = 1;

		if (delta) {
			scPos = (MID(row - (h / Height), scPos + delta, row));
			Refresh(false);
			// End the hold if this was a mousedown to avoid accidental
			// selection of extra lines
			if (click) {
				holding = false;
				left_up = true;
				ReleaseMouse();
			}
		}

		if (sel != row){
			sel = row;
			Refresh(false);
			wxCommandEvent evt(wxEVT_COMMAND_LISTBOX_SELECTED, GetId());
			AddPendingEvent(evt);
		}
	}


}

void FontList::SetSelection(int pos)
{
	if (pos < 0 || pos >= (int)fonts->size()){ 
		if (pos < 0) {
			sel = -1;
			scPos = 0;
		}
		else {
			sel = fonts->size() - 1;
			scPos = 0;
		}
	}
	else {
		if ((scPos<pos || scPos>pos + 7) && scPos >= sel && scPos < sel + 8) {
			scPos -= (sel - pos);
		}
		else {
			scPos = pos - 3;
		}
		sel = pos;
	}
	Refresh(false);
	wxCommandEvent evt(SELECTION_CHANGED, GetId());
	AddPendingEvent(evt);
}

void FontList::SetSelectionByName(wxString name)
{
	int sell = fonts->Index(name, false);
	if (sell != -1){ SetSelection(sell); }
	else {
		SetSelectionByPartialName(name);
	}
}


void FontList::SetSelectionByPartialName(wxString PartialName)
{
	if (PartialName == L""){ SetSelection(0); return; }
	int newSelection = -1;
	PartialName = PartialName.Lower();

	size_t k = 0;
	int lastMatch = 0;

	for (size_t i = 0; i < fonts->size(); i++){
		wxString fontname = (*fonts)[i].Lower();
		if (fontname.length() < 1 || fontname[0] < PartialName[0])
			continue;

		while (k < PartialName.length() && k < fontname.length()){
			if (fontname[k] == PartialName[k]){
				k++;
				lastMatch = i;
				if (k >= PartialName.length()){
					newSelection = i;
					goto done;
				}
			}
			else if (k > 0 && fontname.Mid(0, k) != PartialName.Mid(0, k)){
				goto done;
			}
			else if (fontname[k] > PartialName[k]){
				newSelection = i;
				goto done;
			}
			else
				break;
		}
	}

done:


	if (newSelection == -1)
		newSelection = lastMatch;


	SetSelection(newSelection);

}

wxString FontList::GetString(int line)
{
	if (line < 0 || line >= (int)fonts->size()) return L"";
	return (*fonts)[line];
}

int FontList::GetSelection()
{
	return sel;
}

void FontList::PutArray(wxArrayString* newList)
{
	delete fonts;
	fonts = NULL;
	fonts = new wxArrayString(*newList);
	size_t size = fonts->size();
	if (sel >= size)
		SetSelection(size? size - 1 : 0);

	Refresh(false);
}

void FontList::Clear()
{
	fonts->clear();
}

void FontList::Append(const wxString& font)
{
	fonts->Add(font);
}

void FontList::Delete(int i)
{
	fonts->RemoveAt(i);
}

int FontList::FindString(const wxString& text, bool caseSensitive)
{
	return 0;
}

void FontList::Scroll(int step)
{
	sel += step;
	scPos += step;
	if (scPos < sel - 7 || scPos > sel){ scPos = sel; }
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

FontDialog::FontDialog(wxWindow *parent, Styles *acst, bool changePointToPixel)
	: KaiDialog(parent, -1, _("Wybierz czcionkę"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
	, pointToPixel(changePointToPixel)
{
	editedStyle = acst;
	SetForegroundColour(Options.GetColour(WINDOW_TEXT));
	SetBackgroundColour(Options.GetColour(WINDOW_BACKGROUND));
	wxAcceleratorEntry entries[4];
	entries[0].Set(wxACCEL_NORMAL, WXK_RETURN, wxID_OK);
	entries[1].Set(wxACCEL_NORMAL, WXK_ESCAPE, wxID_CANCEL);
	entries[2].Set(wxACCEL_NORMAL, WXK_UP, ID_SCROLLUP);
	entries[3].Set(wxACCEL_NORMAL, WXK_DOWN, ID_SCROLLDOWN);
	wxAcceleratorTable accel(4, entries);
	SetAcceleratorTable(accel);


	DialogSizer *Main = new DialogSizer(wxVERTICAL);
	KaiStaticBoxSizer *Cfont = new KaiStaticBoxSizer(wxHORIZONTAL, this, _("Czcionka"));
	KaiStaticBoxSizer *prev = new KaiStaticBoxSizer(wxVERTICAL, this, _("Podgląd"));
	wxBoxSizer *Fattr = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *Bsizer = new wxBoxSizer(wxHORIZONTAL);
	
	Fonts = new FontList(this, ID_FONTLIST, wxDefaultPosition, wxSize(250, 200));

	FontName = new KaiTextCtrl(this, ID_FONT_NAME, acst->Fontname, wxDefaultPosition, wxSize(150, -1), wxTE_PROCESS_ENTER);
	FontSize = new NumCtrl(this, ID_FONTSIZE1, acst->Fontsize, 1, 10000, false, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
	Bold = new KaiCheckBox(this, ID_FONTATTR, _("Pogrubienie"));
	Bold->SetValue(acst->Bold);
	Italic = new KaiCheckBox(this, ID_FONTATTR, _("Kursywa"));
	Italic->SetValue(acst->Italic);
	Underl = new KaiCheckBox(this, ID_FONTATTR, _("Podkreślenie"));
	Underl->SetValue(acst->Underline);
	Strike = new KaiCheckBox(this, ID_FONTATTR, _("Przekreślenie"));
	Strike->SetValue(acst->StrikeOut);
	Preview = new StylePreview(this, -1, wxDefaultPosition, wxSize(-1, 180));
	Preview->DrawPreview(acst);
	Buttok = new MappedButton(this, wxID_OK, L"OK");
	Buttcancel = new MappedButton(this, 8999, _("Anuluj"));
	SetEscapeId(8999);
	Fattr->Add(FontName, 0, wxEXPAND | wxLEFT | wxRIGHT, 5);
	Fattr->Add(FontSize, 0, wxEXPAND | wxALL, 5);
	Fattr->Add(Bold, 1, wxEXPAND | wxALL, 5);
	Fattr->Add(Italic, 1, wxEXPAND | wxALL, 5);
	Fattr->Add(Underl, 1, wxEXPAND | wxALL, 5);
	Fattr->Add(Strike, 1, wxEXPAND | wxALL, 5);

	Cfont->Add(Fonts, 1, wxEXPAND);
	Cfont->Add(Fattr, 0, wxEXPAND);
	FCManagement.LoadCatalogs();
	fontCatalog = new KaiChoice(this, ID_FONT_CATALOG_LIST1, wxDefaultPosition, wxDefaultSize, *FCManagement.GetCatalogNames());
	fontCatalog->Insert(_("Wszystkie czcionki"), 0);
	fontCatalog->Insert(_("Bez katalogu"), 1);
	fontCatalog->SetSelection(0);
	MappedButton* CatalogAdd = new MappedButton(this, ID_CATALOG_ADD1, _("Dodaj"));
	CatalogAdd->SetToolTip(_("Dodaje czcionki do wcześniej utworzonego katalogu"));
	MappedButton* CatalogManage = new MappedButton(this, ID_CATALOG_MANAGE1, _("Zarządzaj"));
	CatalogManage->SetToolTip(_("Umorzliwia zarządzanie katalogami stylów"));
	bool fontFilterOn = Options.GetBool(STYLE_EDIT_FILTER_TEXT_ON);
	Filter = new ToggleButton(this, ID_FILTER1, _("Filtruj"));
	Filter->SetToolTip(_("Filtruje czcionki, by zawierały wpisane znaki"));
	Filter->SetValue(fontFilterOn);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent& evt) {
		wxPoint pos = CatalogAdd->GetPosition();
		wxSize size = CatalogAdd->GetSize();
		wxString font;
		GetFontName(&font);
		FCManagement.AddToCatalog(font, wxPoint(pos.x, pos.y + size.y), this);
		}, ID_CATALOG_ADD1);
	Bind(wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, [=](wxCommandEvent& evt) {
		ChangeCatalog(true);
		}, ID_FILTER1);
	ChangeCatalog();

	Fonts->SetSelectionByName(acst->Fontname);
	KaiStaticBoxSizer* filtersizer = new KaiStaticBoxSizer(wxHORIZONTAL, this, _("Filtrowanie i katalogi czcionek"));
	filtersizer->Add(fontCatalog, 3, wxEXPAND | wxALL, 2);
	filtersizer->Add(CatalogAdd, 1, wxEXPAND | wxALL, 2);
	filtersizer->Add(CatalogManage, 1, wxEXPAND | wxALL, 2);
	filtersizer->Add(Filter, 1, wxEXPAND | wxALL, 2);

	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent& evt) {
		if (!FCL) {
			wxString fontname;
			GetFontName(&fontname);
			FCL = new FontCatalogList(this, fontname);
			Bind(CATALOG_CHANGED, [=](wxCommandEvent& evt) {
				int sel = fontCatalog->GetSelection();
				
				fontCatalog->PutArray(FCManagement.GetCatalogNames());
				fontCatalog->Insert(_("Wszystkie czcionki"), 0);
				fontCatalog->Insert(_("Bez katalogu"), 1);
				if (sel >= fontCatalog->GetCount()) {
					sel = fontCatalog->GetCount() - 1;
				}
				fontCatalog->SetSelection(sel);
				ChangeCatalog();
				FCManagement.SaveCatalogs();
				}, FCL->GetId());
		}

		FCL->Show();
		FCL->CenterOnParent();
	}, ID_CATALOG_MANAGE1);

	Bind(wxEVT_COMMAND_CHOICE_SELECTED, [=](wxCommandEvent& evt) {
		ChangeCatalog();
		}, ID_FONT_CATALOG_LIST1);

	prev->Add(Preview, 1, wxEXPAND | wxALL, 5);

	//Bsizer->Add(ButtApply, 1, wxALL, 5);
	Bsizer->Add(Buttok, 1, wxALL, 5);
	Bsizer->Add(Buttcancel, 1, wxALL, 5);

	Main->Add(Cfont, 0, wxEXPAND | wxALL, 5);
	Main->Add(filtersizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5);
	Main->Add(prev, 1, wxEXPAND | wxBOTTOM | wxLEFT | wxRIGHT, 5);
	Main->Add(Bsizer, 0, wxBOTTOM | wxALIGN_CENTER, 5);

	SetSizerAndFit(Main);

	UpdatePreview();

	fontChangedTimer.SetOwner(this, 12345);
	Bind(wxEVT_TIMER, [=](wxTimerEvent & event){
		wxCommandEvent evt(FONT_CHANGED, GetId());
		evt.SetClientData(this);
		//process event immediately
		GetEventHandler()->ProcessEvent(evt);
		//delete edited style and set result style for next using
		if (editedStyle) {
			delete editedStyle;
			editedStyle = resultStyle;
			resultStyle = NULL;
		}
	}, 12345);

	Connect(ID_FONTLIST, wxEVT_COMMAND_LISTBOX_SELECTED, (wxObjectEventFunction)&FontDialog::OnFontChanged);
	Connect(ID_FONTLIST, SELECTION_CHANGED, (wxObjectEventFunction)&FontDialog::OnUpdatePreview);
	Connect(ID_FONT_NAME, wxEVT_COMMAND_TEXT_UPDATED, (wxObjectEventFunction)&FontDialog::OnUpdateText);
	Connect(ID_FONTSIZE1, NUMBER_CHANGED, (wxObjectEventFunction)&FontDialog::OnUpdatePreview);
	Connect(ID_FONTATTR, wxEVT_COMMAND_CHECKBOX_CLICKED, (wxObjectEventFunction)&FontDialog::OnUpdatePreview);
	Connect(ID_SCROLLUP, ID_SCROLLDOWN, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&FontDialog::OnScrollList);
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
	FontSize->SetString(editedStyle->Fontsize);
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
		*outputStyle = (resultStyle) ? resultStyle : GetFont();
	}
}

Styles * FontDialog::GetFont()
{
	if (resultStyle){
		delete resultStyle;
		resultStyle = NULL;
	}
	resultStyle = new Styles();
	resultStyle->Bold = Bold->GetValue();
	resultStyle->Italic = Italic->GetValue();
	resultStyle->Underline = Underl->GetValue();
	resultStyle->StrikeOut = Strike->GetValue();
	GetFontName(&resultStyle->Fontname);
	resultStyle->Fontsize = FontSize->GetString(); 
	return resultStyle;
}

FontDialog * FontDialog::Get(wxWindow *parent, Styles *actualStyle, bool changePointToPixel)
{
	if (FDialog && FDialog->GetParent() != parent){
		FDialog->Destroy();
		FDialog = NULL;
	}
	if (!FDialog)
		FDialog = new FontDialog(parent, actualStyle, changePointToPixel);
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
	Styles *style = GetFont();
	if (pointToPixel){
		wxFont tmpfont(style->GetFontSizeDouble(), wxFONTFAMILY_SWISS, 
			wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, style->Fontname);
		style->SetFontSizeDouble(tmpfont.GetPixelSize().GetHeight());
	}
	Preview->DrawPreview(style);
	fontChangedTimer.Start(200, true);
}

void FontDialog::OnUpdateText(wxCommandEvent& event)
{
	Fonts->SetSelectionByPartialName(FontName->GetValue());
}

void FontDialog::OnScrollList(wxCommandEvent& event)
{

	int step = (event.GetId() == ID_SCROLLUP) ? -1 : 1;
	Fonts->Scroll(step);
	FontName->SetValue(Fonts->GetString(Fonts->GetSelection()));
	UpdatePreview();
}

void FontDialog::ChangeCatalog(bool save)
{
	wxArrayString* fonts = GetFontsTable(save);

	int sel = fontCatalog->GetSelection();
	/*if (sel == -1) {
		fontCatalog->SetSelection(0);
		sel = 0;
	}*/
	if (sel == 0) {
		Fonts->PutArray(fonts);
	}
	else if (sel == 1) {
		//there is also second option, another table and adding instead deleting
		Fonts->PutArray(fonts);
		std::map<wxString, fontList>* map = FCManagement.GetCatalogsMap();
		for (auto it = map->begin(); it != map->end(); it++) {
			for (auto& font : (*it->second)) {
				int fontpos = Fonts->FindString(font, true);
				if (fontpos != -1)
					Fonts->Delete(fontpos);
			}
		}
	}
	else {
		wxString newCatalog = fontCatalog->GetValue();
		wxArrayString* cfonts = FCManagement.GetCatalogFonts(newCatalog);
		Fonts->Clear();
		if (cfonts) {
			for (auto& font : *cfonts) {
				if(fonts->Index(font) != -1)
					Fonts->Append(font);
			}
		}
	}
	if (editedStyle) {
		Fonts->SetSelectionByName(editedStyle->Fontname);
	}else
		Fonts->Refresh(false);
}

void FontDialog::ReloadFonts()
{
	ChangeCatalog(false);
}

void FontDialog::GetFontName(wxString* fontname)
{
	int sel = fontCatalog->GetSelection();
	if (sel >= 0 && FontName->GetValue() == editedStyle->Fontname) {
		*fontname = editedStyle->Fontname;
	}
	else {
		*fontname = Fonts->GetString(Fonts->GetSelection());
	}
}

wxArrayString* FontDialog::GetFontsTable(bool save)
{
	bool filterOn = Filter->GetValue();
	wxArrayString* fonts;
	wxString fontFilterText = Options.GetString(STYLE_EDIT_FILTER_TEXT);
	if (fontFilterText.IsEmpty() || !filterOn) {
		fonts = FontEnum.GetFonts(this, [=]() {
			ReloadFonts();
			});
	}
	else {
		fonts = FontEnum.GetFilteredFonts(this, [=]() {
			ReloadFonts();
			}, fontFilterText);
	}
	if (save)
		Options.SetBool(STYLE_EDIT_FILTER_TEXT_ON, filterOn);

	return fonts;
}

FontPickerButton::FontPickerButton(wxWindow *parent, int id, const wxFont& font,
	const wxPoint& pos, const wxSize& size, long style)
	: MappedButton(parent, id, font.GetFaceName() + L" " + std::to_wstring(font.GetPointSize()), -1, pos, size, style)
{
	ChangeFont(font);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &FontPickerButton::OnClick, this, GetId());
}

void FontPickerButton::ChangeFont(const wxFont &font)
{
	wxSize newSize = GetMinSize();
	SetLabelText(font.GetFaceName() + L" " + std::to_wstring(font.GetPointSize()));
	SetFont(font);
	int fw, fh;
	GetTextExtent(GetLabelText(), &fw, &fh);
	bool isChanged = false;
	if (newSize.x < fw + 16){
		newSize.x = fw + 16;
		isChanged = true;
	}
	if (newSize.y < fh + 10){
		newSize.y = fh + 10;
		isChanged = true;
	}
	//if (isChanged){
		SetMinSize(newSize);
		GetParent()->Layout();
	//}
}

wxFont FontPickerButton::GetSelectedFont(){
	return GetFont();
}

void FontPickerButton::OnClick(wxCommandEvent &evt)
{
	wxFont font = GetFont();
	Styles *mstyle = new Styles();
	mstyle->Fontname = font.GetFaceName();
	mstyle->SetFontSizeDouble(font.GetPointSize());
	FontDialog * FD = FontDialog::Get(this, mstyle, true);
	if (FD->ShowModal() == wxID_OK){
		Styles *retstyle = FD->GetFont();
		font.SetFaceName(retstyle->Fontname);
		font.SetPointSize(MID(retstyle->GetFontSizeDouble(), 8, 20));
		ChangeFont(font);
		//delete retstyle;
	}
}

wxIMPLEMENT_ABSTRACT_CLASS(FontPickerButton, MappedButton);