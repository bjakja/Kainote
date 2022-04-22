//  Copyright (c) 2016 - 2020, Marcin Drob

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


#ifdef guano
#include "VideoToolbar.h"
#include "Config.h"
#include "Notebook.h"
#include "Provider.h"
#include "RendererVideo.h"
#include <wx/msw/winundef.h>
#include <wx/dc.h>
#include <wx/dcmemory.h>
#include <wx/dcclient.h>

std::vector<AllTagsSetting> VideoToolbar::tags;
std::vector<ShapesSetting> VideoToolbar::shapes;
std::vector<itemdata*> VideoToolbar::icons;

VideoToolbar::VideoToolbar(wxWindow *parent, const wxPoint &pos, const wxSize &size)
	:wxWindow(parent, -1, pos, size)
	, Toggled(0)
	, sel(-1)
	, clicked(false)
	, blockScroll(false)
	, bmp(nullptr)
{
	if (icons.size() == 0){
		//Remember! Adding here elements you must change all in h file!!
		//When icon have black background use 
		icons.push_back(new itemdata(PTR_BITMAP_PNG(L"cross"), _("Wskaźnik pozycji")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG(L"position"), _("Przesuwanie tekstu")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG(L"move"), _("Ruch")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG(L"scale"), _("Skalowanie")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG(L"frz"), _("Obrót wokół osi Z")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG(L"frxy"), _("Obrót wokół osi X / Y")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG(L"cliprect"), _("Wycinki prostokątne")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG(L"clip"), _("Wycinki wektorowe")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG(L"drawing"), _("Rysunki wektorowe")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG(L"MOVEAll"), _("Zmieniacz pozycji")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG(L"ALL_TAGS"), _("Hydra")));
		//it's inactive but I will not change all position when I plan to make a visual for fax
		//then I will need it back
		icons.push_back(new itemdata(PTR_BITMAP_PNG(L"SCALE_ROTATION"), _("Zmieniacz skali i obrotów")));

		//12
		//Here clip icons
		icons.push_back(new itemdata(PTR_BITMAP_PNG(L"Vector_Drag"), _("Przesuń punkty")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG(L"Vector_Line"), _("Dodaj linię")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG(L"Vector_Bezier"), _("Dodaj krzywą Beziera")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG(L"VECTOR_BSPLINE"), _("Dodaj krzywą B-sklejaną")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG(L"Vector_Move"), _("Dodaj nowy oddzielny punkt")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG(L"Vector_Delete"), _("Usuń element")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG(L"VECTOR_INVERT_CLIP"), _("Odwróć wycinek")));
		//7
		//icons move all
		icons.push_back(new itemdata(PTR_BITMAP_PNG(L"MOVEPOS"), _("Przenieś punkty pozycjonowania")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG(L"MOVEMOVESTART"), _("Przenieś startowe punkty ruchu")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG(L"MOVE"), _("Przenieś końcowe punkty ruchu")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG(L"MOVECLIPS"), _("Przenieś wycinki")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG(L"MOVEDRAWINGS"), _("Przenieś rysunki,\nużywać tylko w przypadku,\ngdy chcemy przesunąć punkty rysunku,\nnie łączyć z trzema pierwszymi opcjami")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG(L"MOVEORGS"), _("Przenieś punkty org")));
		//6
		//icon rotation z
		icons.push_back(new itemdata(PTR_BITMAP_PNG(L"TWO_POINTS"), _("Ustaw kąt z 2 punktów.\nPo ustawieniu 2 punktów pod tekstem\nna wideo oblicza z nich kąt.")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG(L"SCALE_ROTATION"), _("Zmiana wszystkich tagów obrotu wokół osi Z")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG(L"SUBSRESAMPLE"), _("Utrzymanie proporcji, by po obrocie rysunki wektorowe i tekst\nnie poprzestawiały się względem siebie.")));
		//3
		//icons scale
		icons.push_back(new itemdata(PTR_BITMAP_PNG(L"FRAME_TO_SCALE"), _("Ustaw skalę według prostokąta.\nPo narysowaniu prostokąta tekst zostanie\nzeskalowany wg jednej osi badź dwóch.")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG(L"SCALE_X"), _("Skaluj szerokość")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG(L"SCALE_LINK"), _("Utrzymuj proporcje")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG(L"SCALE_Y"), _("Skaluj wysokość")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG(L"ORIGINAL_FRAME"), _("Ustaw własny prostokąt dla obecnej skali.\nW przypadku niepożądanych różnic można ustawić\nwłasny prostokąt dla pierwotnej skali.")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG(L"SCALE_ROTATION"), _("Zmiana wszystkich tagów skali")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG(L"SUBSRESAMPLE"), _("Utrzymanie proporcji, by po skalowaniu rysunki wektorowe i tekst\nnie poprzestawiały się względem siebie.")));
		//7
		//icons position
		icons.push_back(new itemdata(PTR_BITMAP_PNG(L"FRAME_TO_SCALE"), _("Ustaw pozycję według prostokąta.\nPo narysowaniu prostokąta tekst zostanie\nspozycjonowany wg jednej osi badź dwóch\ndla wybranego położenia.")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG(L"SCALE_X"), _("Pozycjonuj w osi X")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG(L"SCALE_Y"), _("Pozycjonuj w osi Y")));
		//3
		//icons move 
		icons.push_back(new itemdata(PTR_BITMAP_PNG(L"TWO_POINTS"), _("Ustaw ruch 2 punktów i pozycji wideo.\nPo ustawieniu pierwszego punktu przed tekstem który się porusza.\nPrzesunięciu wideo o tyle klatek,\nby na wideo wciąż był widoczny ten sam element,\ndo którego był ustawiony punkt początkowy.\nRuch generuje postawienie drugiego punktu.")));
		//1
		//icon rotation x / y
		icons.push_back(new itemdata(PTR_BITMAP_PNG(L"SCALE_ROTATION"), _("Zmiana wszystkich tagów obrotu wokół osi X / Y")));

	}
	//adding visual second toolbar elements
	visualItems.push_back(nullptr);//cross
	visualItems.push_back(new PositionItem());
	visualItems.push_back(new MoveItem());
	visualItems.push_back(new ScaleItem());
	visualItems.push_back(new RotationZItem());
	visualItems.push_back(new RotationXYItem());//rotation xy
	visualItems.push_back(new ClipRectangleItem());//clip rectangle
	//clip
	visualItems.push_back(new VectorItem(true));
	//drawing
	visualItems.push_back(new VectorItem(false));
	visualItems.push_back(new MoveAllItem());
	visualItems.push_back(new AllTagsItem());
	//for safty there is still scale rotation item
	//in future replaced for fax visual
	//visualItems.push_back(new ScaleRotationItem());

	Connect(wxEVT_PAINT, (wxObjectEventFunction)&VideoToolbar::OnPaint);
	Connect(wxEVT_SIZE, (wxObjectEventFunction)&VideoToolbar::OnSize);
	Connect(wxEVT_LEFT_DOWN, (wxObjectEventFunction)&VideoToolbar::OnMouseEvent);
	Connect(wxEVT_LEFT_UP, (wxObjectEventFunction)&VideoToolbar::OnMouseEvent);
	Connect(wxEVT_MOTION, (wxObjectEventFunction)&VideoToolbar::OnMouseEvent);
	Connect(wxEVT_LEAVE_WINDOW, (wxObjectEventFunction)&VideoToolbar::OnMouseEvent);
	Connect(wxEVT_MOUSEWHEEL, (wxObjectEventFunction)&VideoToolbar::OnMouseEvent);
	wxWindow::SetFont(*Options.GetFont(-1));

	wxString movopts[6] = { _("Dwukrotnym kliknięciu linii (zawsze włączone)"), _("Każdej zmianie linii"),
		_("Kliknięciu linii lub edycji na pauzie"), _("Kliknięciu linii lub edycji"),
		_("Edycji na pauzie"), _("Edycji") };
	wxString playopts[4] = { _("Nic"), _("Audio do końca linii"), _("Wideo do końca linii"),
		_("Wideo do początku następnej linii") };
	videoSeekAfter = new KaiChoice(this, ID_SEEK_AFTER, wxPoint(2, 1), wxDefaultSize, 6, movopts);
	videoSeekAfter->SetSelection(Options.GetInt(MOVE_VIDEO_TO_ACTIVE_LINE));
	videoSeekAfter->SetToolTip(_("Przesuwaj wideo do aktualnej linii po:"));
	wxSize seekMinSize = videoSeekAfter->GetMinSize();
	videoPlayAfter = new KaiChoice(this, ID_PLAY_AFTER, wxPoint(seekMinSize.GetWidth() + 2, 1), wxDefaultSize, 4, playopts);
	videoPlayAfter->SetSelection(Options.GetInt(VIDEO_PLAY_AFTER_SELECTION));
	videoPlayAfter->SetToolTip(_("Odtwarzaj po zmianie linii:"));
	//wxSize playMinSize = videoPlayAfter->GetMinSize();
	//SetMinSize(wxSize(100, seekMinSize.GetHeight() + 2));

	Bind(wxEVT_COMMAND_CHOICE_SELECTED, [=](wxCommandEvent &evt){
		Options.SetInt(MOVE_VIDEO_TO_ACTIVE_LINE, videoSeekAfter->GetSelection());
		Options.SaveOptions(true, false);
	}, ID_SEEK_AFTER);
	Bind(wxEVT_COMMAND_CHOICE_SELECTED, [=](wxCommandEvent &evt){
		Options.SetInt(VIDEO_PLAY_AFTER_SELECTION, videoPlayAfter->GetSelection());
		Options.SaveOptions(true, false);
	}, ID_PLAY_AFTER);
	Bind(wxEVT_ERASE_BACKGROUND, [=](wxEraseEvent &evt){});
}

int VideoToolbar::GetToggled()
{
	return Toggled;
}

int VideoToolbar::GetItemToggled()
{
	if (visualItems[Toggled])
		return visualItems[Toggled]->GetItemToggled();

	return 0;
}

void VideoToolbar::SetItemToggled(int *toggled)
{
	if (visualItems[Toggled]){
		visualItems[Toggled]->SetItemToggled(toggled);
		Refresh(false);
	}
}

void VideoToolbar::OnMouseEvent(wxMouseEvent &evt)
{
	if (!iconsEnabled){ evt.Skip(); return; }

	int x = evt.GetX();
	int w = 0;
	int h = 0;
	GetClientSize(&w, &h);
	bool noelem = false;
	int elem = (x - startDrawPos) / h;
	if (elem < 0){ noelem = true; }
	else if (elem >= toolsSize){
		if (visualItems[Toggled])
			visualItems[Toggled]->OnMouseEvent(evt, endDrawPos, h, this);
		noelem = true;
	}
	if (evt.Leaving() || noelem){ 
		sel = -1; 
		Refresh(false); 
		if (!noelem && HasToolTips()){ UnsetToolTip(); } 
		return; 
	}

	if (elem != sel){
		sel = elem;
		SetToolTip(icons[elem]->help);
		Refresh(false);
	}
	if (evt.LeftDown()){
		if (visualItems[Toggled])
			visualItems[Toggled]->HideContols();

		if (elem == Toggled){ Toggled = 0; }
		else{ Toggled = elem; }

		if (visualItems[Toggled])
			visualItems[Toggled]->ShowContols(this);
		//hide lists only when there is insufficent place
		if (visualItems[Toggled] && insufficentPlace) {
			videoSeekAfter->Show(false);
			videoPlayAfter->Show(false);
			hasHiddenLists = true;
		}
		else if (!videoSeekAfter->IsShown()) {
			videoSeekAfter->Show();
			videoPlayAfter->Show();
			hasHiddenLists = false;
		}
		clicked = true;
		Refresh(false);
	}
	if (evt.LeftUp()){
		clicked = false;
		Refresh(false);
		wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, ID_VIDEO_TOOLBAR_EVENT);
		evt.SetInt(Toggled);
		ProcessEvent(evt);
	}

}

void VideoToolbar::OnPaint(wxPaintEvent &evt)
{
	int w = 0;
	int h = 0;
	GetClientSize(&w, &h);
	if (w == 0 || h == 0){ return; }
	wxMemoryDC tdc;
	if (bmp && (bmp->GetWidth() < w || bmp->GetHeight() < h)) {
		delete bmp;
		bmp = nullptr;
	}
	if (!bmp){ 
		bmp = new wxBitmap(w, h); 
	}
	tdc.SelectObject(*bmp);
	wxColour background = GetParent()->GetBackgroundColour();
	tdc.SetBrush(wxBrush(background));
	tdc.SetPen(wxPen(background));
	tdc.DrawRectangle(0, 0, w, h);
	int posX = startDrawPos;
	int i = 0;
	while (i < toolsSize){
		wxBitmap *icon = icons[i]->icon;
		if (icon->IsOk()){
			bool toggled = i == Toggled;
			if (i == sel){
				tdc.SetBrush(wxBrush(Options.GetColour((toggled || clicked) ? BUTTON_BACKGROUND_PUSHED : BUTTON_BACKGROUND_HOVER)));
				tdc.SetPen(wxPen(Options.GetColour((toggled || clicked) ? BUTTON_BORDER_PUSHED : BUTTON_BORDER_HOVER)));
				tdc.DrawRoundedRectangle(posX, 1, h - 2, h - 2, 2.0);
			}
			else if (toggled){
				tdc.SetBrush(wxBrush(Options.GetColour(BUTTON_BACKGROUND_PUSHED)));
				tdc.SetPen(wxPen(Options.GetColour(BUTTON_BORDER_PUSHED)));
				tdc.DrawRoundedRectangle(posX, 1, h - 2, h - 2, 2.0);
			}
			tdc.DrawBitmap((iconsEnabled) ? *icon : icon->ConvertToDisabled(), 
				posX + ((h - icon->GetHeight()) / 2) - 1, ((h - icon->GetWidth()) / 2));
			posX += h;
		}
		i++;
	}
	if (visualItems[Toggled])
		visualItems[Toggled]->OnPaint(tdc, endDrawPos, h, this);

	wxPaintDC dc(this);
	dc.Blit(0, 0, w, h, &tdc, 0, 0);
}

void VideoToolbar::OnSize(wxSizeEvent &evt)
{
	
	wxSize size = evt.GetSize();

	wxSize seekMinSize = videoSeekAfter->GetBestSize();
	wxSize playMinSize = videoPlayAfter->GetBestSize();
	int seekMinWidth = seekMinSize.GetWidth();
	int playMinWidth = playMinSize.GetWidth();
	int height = size.y - 2;
	int allToolsSize = 20 * size.y;
	//one square for spacing
	int spaceForLists = (size.x - allToolsSize - 6);
	if (spaceForLists < seekMinWidth + playMinWidth){
		seekMinWidth = spaceForLists / 2;
		if (seekMinWidth < 80) {
			seekMinWidth = 80;
			insufficentPlace = true;
		}
		else
			insufficentPlace = false;

		playMinWidth = seekMinWidth;
	}
	/*videoSeekAfter->SetSize(seekMinWidth, height);
	videoPlayAfter->SetSize(seekMinWidth + 2, 1, playMinWidth, height);*/
	videoSeekAfter->SetSize(size.x - 1 - seekMinWidth, 1, seekMinWidth, height);
	videoPlayAfter->SetSize(size.x - (seekMinWidth + playMinWidth + 3), 1, playMinWidth, height);
	startDrawPos = 2;//playMinWidth + seekMinWidth + 6;
	endDrawPos = insufficentPlace? size.x : size.x - (seekMinWidth + playMinWidth + 3);
	if (visualItems[Toggled])
		visualItems[Toggled]->OnSize(this);
	Refresh(false);
}

void VideoToolbar::Synchronize(VideoToolbar *vtoolbar){
	if (visualItems[Toggled]) {
		visualItems[Toggled]->HideContols();
	}
	Toggled = vtoolbar->Toggled;
	sel = vtoolbar->sel;
	if (visualItems[Toggled]) {
		visualItems[Toggled]->ShowContols(this);
		visualItems[Toggled]->Synchronize(vtoolbar->visualItems[Toggled]);
	}

	clicked = vtoolbar->clicked;
	blockScroll = vtoolbar->blockScroll;
	iconsEnabled = vtoolbar->iconsEnabled;
	if (IsShown()){ Refresh(false); }
}

bool VideoToolbar::SetFont(const wxFont &font)
{
	wxFont vFont = font;
	vFont.SetPointSize(font.GetPointSize() - 1);
	videoSeekAfter->SetFont(vFont);
	videoPlayAfter->SetFont(vFont);

	return true;
}
void VideoToolbar::SetHeight(int height)
{
	wxSizeEvent evt;
	wxSize size = GetClientSize();
	size.y = height;
	SetSize(size);
}

void MoveAllItem::OnMouseEvent(wxMouseEvent &evt, int w, int h, VideoToolbar *vt)
{
	int startDrawPos = w - (h * numMoveIcons);
	int x, y;
	evt.GetPosition(&x, &y);
	int elem = ((x - startDrawPos) / h);
	if (evt.Leaving() || elem < 0 || x < startDrawPos){
		selection = -1;
		clicked = false;
		vt->Refresh(false);
		return;
	}
	if (elem >= numMoveIcons)
		return;
	
	if (elem != selection){
		selection = elem;
		vt->SetToolTip(vt->icons[elem + startIconNumber]->help);
		vt->Refresh(false);
	}
	if (evt.LeftDown()){
		MoveToggled[elem] = !MoveToggled[elem];
		clicked = true;
		vt->Refresh(false);
	}
	if (evt.LeftUp()){
		clicked = false;
		vt->Refresh(false);
		wxCommandEvent *evt = new wxCommandEvent(wxEVT_COMMAND_MENU_SELECTED, ID_MOVE_TOOLBAR_EVENT);
		evt->SetInt(GetItemToggled());
		wxQueueEvent(vt, evt);
	}
}

void MoveAllItem::OnPaint(wxDC &dc, int w, int h, VideoToolbar *vt)
{
	int posX = w - (h * numMoveIcons);
	int i = 0;
	while (i < numMoveIcons){
		wxBitmap *icon = vt->icons[i + startIconNumber]->icon;
		if (icon->IsOk()){
			bool toggled = MoveToggled[i];
			if (i == selection){
				dc.SetBrush(wxBrush(Options.GetColour((toggled || clicked) ? BUTTON_BACKGROUND_PUSHED : BUTTON_BACKGROUND_HOVER)));
				dc.SetPen(wxPen(Options.GetColour((toggled || clicked) ? BUTTON_BORDER_PUSHED : BUTTON_BORDER_HOVER)));
				dc.DrawRoundedRectangle(posX, 1, h - 2, h - 2, 2.0);
			}
			else if (toggled){
				dc.SetBrush(wxBrush(Options.GetColour(BUTTON_BACKGROUND_PUSHED)));
				dc.SetPen(wxPen(Options.GetColour(BUTTON_BORDER_PUSHED)));
				dc.DrawRoundedRectangle(posX, 1, h - 2, h - 2, 2.0);
			}

			dc.DrawBitmap(*icon, posX + ((h - icon->GetHeight()) / 2) - 1, ((h - icon->GetWidth()) / 2));
			posX += h;
		}
		i++;
	}
}

void VectorItem::OnMouseEvent(wxMouseEvent &evt, int w, int h, VideoToolbar *vt)
{
	int startDrawPos = w - (h * numIcons) - shapeListWidth;
	if (shapeListSelection)
		return;

	int x, y;
	evt.GetPosition(&x, &y);
	int elem = ((x - startDrawPos) / h);
	if (evt.Leaving() || elem < 0 || x < startDrawPos){
		selection = -1;
		clicked = false;
		vt->Refresh(false);
		if (vt->HasToolTips()) { vt->UnsetToolTip(); }
		return;
	}
	if (elem >= numIcons)
		return;
	
	if (evt.GetWheelRotation() != 0) {
		if (vt->blockScroll){ evt.Skip(); return; }
		int step = evt.GetWheelRotation() / evt.GetWheelDelta();
		toggled -= step;
		if (toggled < 0){ toggled = numIcons - 1; }
		else if (toggled >= numIcons){ toggled = 0; }
		vt->Refresh(false);
		return;
	}

	if (elem != selection){
		selection = elem;
		wxString& tooltext = vt->icons[elem + startIconNumber]->help;
		vt->Refresh(false);
		vt->SetToolTip(tooltext);
	}
	if (evt.LeftDown()){
		//when added more normal buttons make table for it or something like that not just enumerate it by if
		isNormalButton = elem == 6;
		if(!isNormalButton)
			toggled = elem;

		
		clicked = true;
		vt->Refresh(false);
	}
	if (evt.LeftUp()){
		clicked = false;
		vt->Refresh(false);
		wxCommandEvent *evt = new wxCommandEvent(wxEVT_COMMAND_MENU_SELECTED, ID_VECTOR_TOOLBAR_EVENT);
		evt->SetInt(isNormalButton? 6 : toggled);
		wxQueueEvent(vt, evt);
	}
}

void VectorItem::OnPaint(wxDC &dc, int w, int h, VideoToolbar *vt)
{
	int posX = w - (h * numIcons) - shapeListWidth;
	int i = 0;
	while (i < numIcons){
		wxBitmap *icon = vt->icons[i + startIconNumber]->icon;
		if (icon->IsOk()){
			if (!shapeListSelection) {
				if (i == selection) {
					dc.SetBrush(wxBrush(Options.GetColour((toggled == i || clicked) ? BUTTON_BACKGROUND_PUSHED : BUTTON_BACKGROUND_HOVER)));
					dc.SetPen(wxPen(Options.GetColour((toggled == i || clicked) ? BUTTON_BORDER_PUSHED : BUTTON_BORDER_HOVER)));
					dc.DrawRoundedRectangle(posX, 1, h - 2, h - 2, 2.0);
				}
				else if (i == toggled) {
					dc.SetBrush(wxBrush(Options.GetColour(BUTTON_BACKGROUND_PUSHED)));
					dc.SetPen(wxPen(Options.GetColour(BUTTON_BORDER_PUSHED)));
					dc.DrawRoundedRectangle(posX, 1, h - 2, h - 2, 2.0);
				}
			}
			dc.DrawBitmap(*icon, posX + ((h - icon->GetHeight()) / 2) - 1, ((h - icon->GetWidth()) / 2), true);
			posX += h;
		}
		i++;
	}
}

void VectorItem::HideContols()
{
	if (shapeList) {
		shapeListSelection = shapeList->GetSelection();
		shapeList->Destroy();
		shapeList = nullptr;
	}
}

void VectorItem::ShowContols(VideoToolbar* vt)
{
	if (isDrawing) {
		auto *shapes = VideoToolbar::GetShapesSettings();
		wxArrayString list;
		GetNames(shapes, &list);
		list.Insert(_("Wybierz"), 0);
		list.Add(_("Edytuj"));
		shapeList = new KaiChoice(vt, ID_SHAPE_LIST, wxDefaultPosition, wxDefaultSize, list);
		shapeList->SetToolTip(_("Lista gotowych rysunków ASS z możliwością edycji.\nPo wybraniu rysunku z listy należy ustawić kursor\nw miejcu początku przytrzymać lewy przycisk myszy i przeciągnąć."));
		shapeList->SetSelection(shapeListSelection);

		auto sendItemToggled = [=](wxCommandEvent& evt) {
			int listSelection = shapeList->GetSelection();
			if (listSelection > shapes->size()) {
				ShapesEdition se(Notebook::GetTab(), wxPoint(), shapes, shapeListSelection);
				bool isOK = se.ShowModal() == wxID_OK;
				if (isOK) {
					auto *shapes = se.GetShapes();
					VideoToolbar::SetShapesSettings(shapes);
					wxArrayString names;
					GetNames(shapes, &names);
					names.Insert(_("Wybierz"), 0);
					names.Add(_("Edytuj"));
					shapeList->PutArray(&names);
					//starts from 1, 0 is inactive
					if (shapeListSelection > shapes->size()) {
						shapeListSelection = shapes->size();
					}
				}
				shapeList->SetSelection(shapeListSelection);
				if (!isOK)
					return;
			}
			else {
				int shapeListSelectionNew = shapeList->GetSelection();
				bool needRefresh = false;
				if ((shapeListSelection == 0 && shapeListSelectionNew != 0) ||
					(shapeListSelection != 0 && shapeListSelectionNew == 0)){
					needRefresh = true;
				}
				shapeListSelection = shapeListSelectionNew;
				if(needRefresh)
					vt->Refresh(false);
			}
			wxCommandEvent* event = new wxCommandEvent(wxEVT_COMMAND_MENU_SELECTED, ID_MOVE_TOOLBAR_EVENT);
			event->SetInt(GetItemToggled());
			wxQueueEvent(vt, event);
			wxWindow* grandParent = vt->GetGrandParent();
			grandParent->SetFocus();
		};

		vt->Bind(wxEVT_COMMAND_CHOICE_SELECTED, sendItemToggled, ID_SHAPE_LIST);
		OnSize(vt);
	}
}

void VectorItem::OnSize(VideoToolbar* vt)
{
	if (shapeList) {
		int maxWidth = vt->GetEndDrawPos();
		wxSize ans = shapeList->GetBestSize();
		ans.x = 70;
		wxSize vts = vt->GetSize();
		wxPoint pos(maxWidth - 2 - ans.x, 1);
		shapeList->SetPosition(pos);
		ans.y = vts.y - 2;
		shapeList->SetSize(ans);
		shapeListWidth = ans.x + 4;
	}
}

void ClipRectangleItem::OnMouseEvent(wxMouseEvent &evt, int w, int h, VideoToolbar *vt)
{
	int startDrawPos = w - (h * numIcons);
	int x, y;
	evt.GetPosition(&x, &y);
	int elem = ((x - startDrawPos) / h);
	if (evt.Leaving() || elem < 0 || x < startDrawPos){
		selection = -1;
		clicked = false;
		vt->Refresh(false);
		if (vt->HasToolTips()) { vt->UnsetToolTip(); }
		return;
	}
	if (elem >= numIcons)
		return;

	if (elem != selection){
		selection = elem;
		vt->SetToolTip(vt->icons[elem + startIconNumber]->help);
		vt->Refresh(false);
	}
	if (evt.LeftDown()){
		toggled = elem;
		clicked = true;
		vt->Refresh(false);
	}
	if (evt.LeftUp()){
		clicked = false;
		vt->Refresh(false);
		wxCommandEvent *evt = new wxCommandEvent(wxEVT_COMMAND_MENU_SELECTED, ID_MOVE_TOOLBAR_EVENT);
		evt->SetInt(toggled);
		wxQueueEvent(vt, evt);
	}
}

void ClipRectangleItem::OnPaint(wxDC &dc, int w, int h, VideoToolbar *vt)
{
	int posX = w - (h * numIcons);
	int i = 0;
	while (i < numIcons){
		wxBitmap *icon = vt->icons[i + startIconNumber]->icon;
		if (icon->IsOk()){
			if (i == selection){
				dc.SetBrush(wxBrush(Options.GetColour(clicked ? BUTTON_BACKGROUND_PUSHED : BUTTON_BACKGROUND_HOVER)));
				dc.SetPen(wxPen(Options.GetColour(clicked ? BUTTON_BORDER_PUSHED : BUTTON_BORDER_HOVER)));
				dc.DrawRoundedRectangle(posX, 1, h - 2, h - 2, 2.0);
			}
			/*else if (i == toggled){
				dc.SetBrush(wxBrush(Options.GetColour(BUTTON_BACKGROUND_PUSHED)));
				dc.SetPen(wxPen(Options.GetColour(BUTTON_BORDER_PUSHED)));
				dc.DrawRoundedRectangle(posX, 1, h - 2, h - 2, 2.0);
			}*/

			dc.DrawBitmap(*icon, posX + ((h - icon->GetHeight()) / 2) - 1, ((h - icon->GetWidth()) / 2), true);
			posX += h;
		}
		i++;
	}
}

AllTagsItem::AllTagsItem()
{
}

void AllTagsItem::OnMouseEvent(wxMouseEvent& evt, int w, int h, VideoToolbar* vt)
{
	
}

void AllTagsItem::OnPaint(wxDC& dc, int w, int h, VideoToolbar* vt)
{
	/*if (!tagList || !edition) {
		ShowContols(vt);
	}
	else if (maxWidth != vt->GetEndDrawPos()) {
		wxSize tlbs = tagList->GetBestSize();
		wxSize ebs = edition->GetBestSize();
		wxPoint pos(maxWidth - 4 - ebs.x, 1);
		edition->SetPosition(pos);
		pos.y = 1;
		pos.x -= tlbs.x + 4;
		tagList->SetPosition(pos);
	}*/
}

void AllTagsItem::Synchronize(VisualItem* item)
{
	AllTagsItem* ati = (AllTagsItem*)item;
	if (ati->tagList) {
		selection = ati->tagList->GetSelection();
		tagList->SetSelection(selection);
		mode = ati->options->GetSelection();
		options->SetSelection(mode);
	}
	else {
		KaiLog(L"No sychronization, pointers released");
	}
}

int AllTagsItem::GetItemToggled()
{
	if (tagList)
		selection = tagList->GetSelection();

	if (options)
		mode = options->GetSelection();

	int items = mode;
	items <<= 20;
	items += selection;

	return items;
}

void AllTagsItem::SetItemToggled(int* item)
{
	int tool = *item;
	if (tagList) {
		selection = tool << 12;
		selection >>= 12;
		if (selection < 0)
			selection = tagList->GetCount() - 1;
		else if (selection >= tagList->GetCount())
			selection = 0;

		tagList->SetSelection(selection);
	}
	if (options) {
		mode = tool >> 20;
	}
}

void AllTagsItem::HideContols()
{
	if (tagList || edition || options) {
		if (tagList) {
			selection = tagList->GetSelection();
			tagList->Destroy();
		}
		if (edition)
			edition->Destroy();
		if (options) {
			mode = options->GetSelection();
			options->Destroy();
		}
		tagList = nullptr;
		edition = nullptr;
		options = nullptr;
	}
}

void AllTagsItem::ShowContols(VideoToolbar* vtoolbar)
{
	if (tagList)
		return;

	auto tags = VideoToolbar::GetTagsSettings();
	wxArrayString list;
	GetNames(tags, &list);
	tagList = new KaiChoice(vtoolbar, ID_TAG_LIST, wxDefaultPosition, wxDefaultSize, list);
	tagList->SetToolTip(_("Lista z tagami obsługiwanymi przez narzędzie"));
	tagList->SetSelection(selection);
	wxString optionsList[] = { _("Dodaj"), _("Wstaw"), _("Pomnóż"), _("Pomnóż+"), _("Gradient tekst"), _("Gradient linia") };
	wxSize wsize = vtoolbar->GetTextExtent(_("Pomnóż+"));
	options = new KaiChoice(vtoolbar, ID_OPTIONS, wxDefaultPosition, wxSize(wsize.x + 26, -1), 6, optionsList);
	options->SetToolTip(_("Opcje zmiany tagów:\nDodaj - zmienia wszystkie tagi dodając ruch z suwaka.\nWstaw - wstawia w miejsce kursora w przypadku jednej linii\nalbo na początku w przypadku wielu linii.\nPomnóż - mnoży ruch suwaka przez numer zaznaczonej linijki\npierwsza linia nie jest zmieniana."));
	options->SetSelection(mode);

	auto sendItemToggled = [=](wxCommandEvent& evt) {
		if (evt.GetId() == ID_TAG_LIST) {
			int sel = tagList->GetSelection();
			if (sel < tags->size() && sel >= 0) {
				AllTagsSetting setting = (*tags)[sel];
				options->SetSelection(setting.tagMode);
			}
		}
		wxCommandEvent* event = new wxCommandEvent(wxEVT_COMMAND_MENU_SELECTED, ID_MOVE_TOOLBAR_EVENT);
		event->SetInt(GetItemToggled());
		wxQueueEvent(vtoolbar, event);
		wxWindow* grandParent = vtoolbar->GetGrandParent();
		grandParent->SetFocus();
	};

	vtoolbar->Bind(wxEVT_COMMAND_CHOICE_SELECTED, sendItemToggled, ID_TAG_LIST);

	vtoolbar->Bind(wxEVT_COMMAND_CHOICE_SELECTED, sendItemToggled, ID_OPTIONS);

	edition = new MappedButton(vtoolbar, ID_EDITION, _("Edytuj"), _("Edycja tagów z listy oraz tworzenie nowych"), wxDefaultPosition, wxDefaultSize, -1);
	vtoolbar->Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent& evt) {
		AllTagsEdition edit(vtoolbar, wxPoint(), tags, tagList->GetSelection());
		if (edit.ShowModal() == wxID_OK) {
			auto tags = edit.GetTags();
			VideoToolbar::SetTagsSettings(tags);
			wxArrayString names;
			GetNames(tags, &names);
			tagList->PutArray(&names);
			selection = tagList->GetSelection();
			wxCommandEvent* event = new wxCommandEvent(wxEVT_COMMAND_MENU_SELECTED, ID_MOVE_TOOLBAR_EVENT);
			event->SetInt(GetItemToggled());
			wxQueueEvent(vtoolbar, event);
		}
		}, ID_EDITION
	);
	OnSize(vtoolbar);
}

void AllTagsItem::OnSize(VideoToolbar* vt)
{
	if (!tagList)
		return;

	maxWidth = vt->GetEndDrawPos();
	wxSize tlbs = tagList->GetBestSize();
	wxSize ebs = edition->GetBestSize();
	wxSize obs = vt->GetTextExtent(_("Pomnóż+"));
	obs.x += 26;
	obs.y = tlbs.y;
	wxSize vts = vt->GetSize();
	wxPoint pos(maxWidth - 2 - ebs.x, 1);
	edition->SetPosition(pos);
	ebs.y = vts.y - 2;
	edition->SetSize(ebs);
	pos.x -= obs.x + 2;
	options->SetPosition(pos);
	obs.y = vts.y - 2;
	options->SetSize(obs);
	pos.x -= tlbs.x + 2;
	tagList->SetPosition(pos);
	tlbs.y = vts.y - 2;
	tagList->SetSize(tlbs);
}

void RotationZItem::OnMouseEvent(wxMouseEvent& evt, int w, int h, VideoToolbar* vt)
{
	int startDrawPos = w - (h * numIcons);
	int x, y;
	evt.GetPosition(&x, &y);
	int elem = ((x - startDrawPos) / h);
	bool isGrayed = elem == 1 && Toggled[2];
	if (evt.Leaving() || elem < 0 || x < startDrawPos) {
		selection = -1;
		clicked = false;
		vt->Refresh(false);
		if (vt->HasToolTips()) { vt->UnsetToolTip(); }
		return;
	}
	if (elem >= numIcons)
		return;


	if (elem != selection) {
		selection = elem;
		vt->SetToolTip(vt->icons[elem + startIconNumber]->help);
		vt->Refresh(false);
	}
	if (evt.LeftDown() && !isGrayed) {
		Toggled[elem] = !Toggled[elem];
		if (elem == 2 && Toggled[elem]) {
			Toggled[1] = true;
		}
		clicked = true;
		vt->Refresh(false);
	}
	if (evt.LeftUp() && !isGrayed) {
		clicked = false;
		vt->Refresh(false);
		wxCommandEvent* evt = new wxCommandEvent(wxEVT_COMMAND_MENU_SELECTED, ID_MOVE_TOOLBAR_EVENT);
		evt->SetInt(GetItemToggled());
		wxQueueEvent(vt, evt);
	}
}

void RotationZItem::OnPaint(wxDC& dc, int w, int h, VideoToolbar* vt)
{
	int posX = w - (h * numIcons);
	int i = 0;
	while (i < numIcons) {
		wxBitmap* icon = vt->icons[i + startIconNumber]->icon;
		if (icon->IsOk()) {
			if (i == selection) {
				dc.SetBrush(wxBrush(Options.GetColour((Toggled[i] || clicked) ? BUTTON_BACKGROUND_PUSHED : BUTTON_BACKGROUND_HOVER)));
				dc.SetPen(wxPen(Options.GetColour((Toggled[i] || clicked) ? BUTTON_BORDER_PUSHED : BUTTON_BORDER_HOVER)));
				dc.DrawRoundedRectangle(posX, 1, h - 2, h - 2, 2.0);
			}
			else if (Toggled[i]) {
				dc.SetBrush(wxBrush(Options.GetColour(BUTTON_BACKGROUND_PUSHED)));
				dc.SetPen(wxPen(Options.GetColour(BUTTON_BORDER_PUSHED)));
				dc.DrawRoundedRectangle(posX, 1, h - 2, h - 2, 2.0);
			}

			dc.DrawBitmap(*icon, posX + ((h - icon->GetHeight()) / 2) - 1, ((h - icon->GetWidth()) / 2), true);
			posX += h;
		}
		i++;
	}
}

void RotationZItem::Synchronize(VisualItem* item)
{
	RotationZItem* rzi = (RotationZItem*)item;
	for (int i = 0; i < numIcons; i++) {
		Toggled[i] = rzi->Toggled[i];
	}
}

void ScaleItem::OnMouseEvent(wxMouseEvent& evt, int w, int h, VideoToolbar* vt)
{
	int startDrawPos = w - (h * numIcons);
	int x, y;
	evt.GetPosition(&x, &y);
	int elem = ((x - startDrawPos) / h);
	//block elements 1-4 when rectangle to scale is off
	bool isGrayed = ((elem > 0 && elem < 5) && !Toggled[0]) || 
		//block untoggle axis x and y when link both axis is on
		(elem == 1 || elem == 3) && Toggled[2] || 
		//block turn off uncheck changing all values 
		//when preserve proportions or rectangle to scale is on
		elem == 5 && (Toggled[6] || Toggled[0]);
	if (evt.Leaving() || elem < 0 || x < startDrawPos) {
		selection = -1;
		clicked = false;
		vt->Refresh(false);
		if (vt->HasToolTips()) { vt->UnsetToolTip(); }
		return;
	}
	if (elem >= numIcons)
		return;


	if (elem != selection) {
		selection = elem;
		vt->SetToolTip(vt->icons[elem + startIconNumber]->help);
		vt->Refresh(false);
	}
	if (evt.LeftDown() && !isGrayed) {
		Toggled[elem] = !Toggled[elem];
		//icon 1 or 2 for scale x and y
		//one of scale have to be selected
		if (elem == 1 || elem == 3) {
			if (!Toggled[1] && !Toggled[3]) {
				int nsel = elem == 1 ? 3 : 1;
				Toggled[nsel] = true;
			}
		}
		//select scale x when preserve aspect ratio is used
		if (elem == 2 && Toggled[elem]) {
			Toggled[1] = true;
		}
		//when preserve aspect ratio or rectangle to scale is on 
		//then changing all values also have to be on 
		if ((elem == 6 && Toggled[elem]) || (elem == 0 && Toggled[elem])) {
			Toggled[5] = true;
		}
		clicked = true;
		vt->Refresh(false);
	}
	if (evt.LeftUp() && !isGrayed) {
		clicked = false;
		vt->Refresh(false);
		wxCommandEvent* evt = new wxCommandEvent(wxEVT_COMMAND_MENU_SELECTED, ID_MOVE_TOOLBAR_EVENT);
		evt->SetInt(GetItemToggled());
		wxQueueEvent(vt, evt);
	}
}

void ScaleItem::OnPaint(wxDC& dc, int w, int h, VideoToolbar* vt)
{
	int posX = w - (h * numIcons);
	int i = 0;
	while (i < numIcons) {
		wxBitmap* icon = vt->icons[i + startIconNumber]->icon;
		bool isGrayed = ((i > 0 && i < 5) && !Toggled[0]) || i == 3 && Toggled[2];
		if (icon->IsOk()) {
			if (i == selection && !isGrayed) {
				dc.SetBrush(wxBrush(Options.GetColour((Toggled[i] || clicked) ? BUTTON_BACKGROUND_PUSHED : BUTTON_BACKGROUND_HOVER)));
				dc.SetPen(wxPen(Options.GetColour((Toggled[i] || clicked) ? BUTTON_BORDER_PUSHED : BUTTON_BORDER_HOVER)));
				dc.DrawRoundedRectangle(posX, 1, h - 2, h - 2, 2.0);
			}
			else if (Toggled[i] && !isGrayed) {
				dc.SetBrush(wxBrush(Options.GetColour(BUTTON_BACKGROUND_PUSHED)));
				dc.SetPen(wxPen(Options.GetColour(BUTTON_BORDER_PUSHED)));
				dc.DrawRoundedRectangle(posX, 1, h - 2, h - 2, 2.0);
			}

			dc.DrawBitmap(isGrayed ? icon->ConvertToDisabled() : *icon, posX + ((h - icon->GetHeight()) / 2) - 1, ((h - icon->GetWidth()) / 2), true);
			posX += h;
		}
		i++;
	}
}

void ScaleItem::Synchronize(VisualItem* item)
{
	ScaleItem* si = (ScaleItem*)item;
	for (int i = 0; i < numIcons; i++) {
		Toggled[i] = si->Toggled[i];
	}
}


void PositionItem::OnMouseEvent(wxMouseEvent& evt, int w, int h, VideoToolbar* vt)
{
	int startDrawPos = w - (h * numIcons);
	if (alignment) {
		wxSize asize = alignment->GetSize();
		startDrawPos -= (asize.x + 6);
	}
	int x, y;
	evt.GetPosition(&x, &y);
	int elem = ((x - startDrawPos) / h);
	bool isGrayed = elem > 0 && !Toggled[0];
	if (evt.Leaving() || elem < 0 || x < startDrawPos) {
		selection = -1;
		clicked = false;
		vt->Refresh(false);
		if (vt->HasToolTips()) { vt->UnsetToolTip(); }
		return;
	}
	if (elem >= numIcons)
		return;


	if (elem != selection) {
		selection = elem;
		vt->SetToolTip(vt->icons[elem + startIconNumber]->help);
		vt->Refresh(false);
	}
	if (evt.LeftDown() && !isGrayed) {
		Toggled[elem] = !Toggled[elem];
		//icon 1 or 2 for scale x and y
		//one of scale have to be selected
		if (elem == 1 || elem == 2) {
			if (!Toggled[1] && !Toggled[2]) {
				int nsel = elem == 1 ? 2 : 1;
				Toggled[nsel] = true;
			}
		}
		clicked = true;
		vt->Refresh(false);
	}
	if (evt.LeftUp() && !isGrayed) {
		clicked = false;
		vt->Refresh(false);
		wxCommandEvent* evt = new wxCommandEvent(wxEVT_COMMAND_MENU_SELECTED, ID_MOVE_TOOLBAR_EVENT);
		evt->SetInt(GetItemToggled());
		wxQueueEvent(vt, evt);
	}
}

void PositionItem::OnPaint(wxDC& dc, int w, int h, VideoToolbar* vt)
{
	int posX = w - (h * numIcons);
	if (alignment) {
		wxSize asize = alignment->GetSize();
		posX -= (asize.x + 6);
	}
	int i = 0;
	while (i < numIcons) {
		wxBitmap* icon = vt->icons[i + startIconNumber]->icon;
		bool isGrayed = i > 0 && !Toggled[0];
		if (icon->IsOk()) {
			if (i == selection && !isGrayed) {
				dc.SetBrush(wxBrush(Options.GetColour((Toggled[i] || clicked) ? BUTTON_BACKGROUND_PUSHED : BUTTON_BACKGROUND_HOVER)));
				dc.SetPen(wxPen(Options.GetColour((Toggled[i] || clicked) ? BUTTON_BORDER_PUSHED : BUTTON_BORDER_HOVER)));
				dc.DrawRoundedRectangle(posX, 1, h - 2, h - 2, 2.0);
			}
			else if (Toggled[i] && !isGrayed) {
				dc.SetBrush(wxBrush(Options.GetColour(BUTTON_BACKGROUND_PUSHED)));
				dc.SetPen(wxPen(Options.GetColour(BUTTON_BORDER_PUSHED)));
				dc.DrawRoundedRectangle(posX, 1, h - 2, h - 2, 2.0);
			}

			dc.DrawBitmap(isGrayed ? icon->ConvertToDisabled() : *icon, posX + ((h - icon->GetHeight()) / 2) - 1, ((h - icon->GetWidth()) / 2), true);
			posX += h;
		}
		i++;
	}
}

void PositionItem::Synchronize(VisualItem* item)
{
	PositionItem* pi = (PositionItem*)item;
	for (int i = 0; i < numIcons; i++) {
		Toggled[i] = pi->Toggled[i];
	}
	an = pi->an;
	alignment->SetSelection(an);
}

void PositionItem::HideContols()
{
	if (alignment) {
		alignment->Destroy();
		alignment = nullptr;
	}
}

void PositionItem::ShowContols(VideoToolbar* vt)
{
	if (alignment)
		return;

	wxString alignments[] = { _("Lewo-dół"), _("Środek-dół"), _("Prawo-dół"),
		_("Lewo-środek"), _("Środek"), _("Prawo-środek"), 
		_("Lewo-góra"), _("Środek-góra"), _("Prawo-góra"), 
		//above options
		_("Lewo-poniżej"), _("Środek-poniżej"), _("Prawo-poniżej"),
		_("Lewo-ponad"), _("Środek-ponad"), _("Prawo-ponad"),
		_("Przed-góra"), _("Przed-środek"), _("Przed-dół"), 
		_("Za-góra"), _("Za-środek"), _("Za-dół") };
	alignment = new KaiChoice(vt, ID_ALIGNMENT, wxDefaultPosition, wxDefaultSize, 21, alignments);
	vt->Bind(wxEVT_COMMAND_CHOICE_SELECTED, [=](wxCommandEvent& evt) {
		wxCommandEvent* evt1 = new wxCommandEvent(wxEVT_COMMAND_MENU_SELECTED, ID_MOVE_TOOLBAR_EVENT);
		an = alignment->GetSelection();
		evt1->SetInt(GetItemToggled());
		wxQueueEvent(vt, evt1);
		}, ID_ALIGNMENT);
	alignment->SetSelection(an);
	alignment->SetToolTip(_("Położenie tekstu, działa podobnie jak w stylach"));
	OnSize(vt);
}

void PositionItem::OnSize(VideoToolbar* vt)
{
	if (!alignment)
		return;

	int maxWidth = vt->GetEndDrawPos();
	wxSize ans = alignment->GetBestSize();
	wxSize vts = vt->GetSize();
	wxPoint pos(maxWidth - 2 - ans.x, 1);
	alignment->SetPosition(pos);
	ans.y = vts.y - 2;
	alignment->SetSize(ans);
}

void MoveItem::OnMouseEvent(wxMouseEvent& evt, int w, int h, VideoToolbar* vt)
{
	int startDrawPos = w - (h * numIcons);
	int x, y;
	evt.GetPosition(&x, &y);
	int elem = ((x - startDrawPos) / h);
	if (evt.Leaving() || elem < 0 || x < startDrawPos) {
		selection = -1;
		clicked = false;
		vt->Refresh(false);
		if (vt->HasToolTips()) { vt->UnsetToolTip(); }
		return;
	}
	if (elem >= numIcons)
		return;

	if (elem != selection) {
		selection = elem;
		vt->SetToolTip(vt->icons[elem + startIconNumber]->help);
		vt->Refresh(false);
	}
	if (evt.LeftDown()) {
		toggled = (toggled == elem) ? -1 : elem;
		clicked = true;
		vt->Refresh(false);
	}
	if (evt.LeftUp()) {
		clicked = false;
		vt->Refresh(false);
		wxCommandEvent* evt = new wxCommandEvent(wxEVT_COMMAND_MENU_SELECTED, ID_MOVE_TOOLBAR_EVENT);
		evt->SetInt(toggled);
		wxQueueEvent(vt, evt);
	}
}

void MoveItem::OnPaint(wxDC& dc, int w, int h, VideoToolbar* vt)
{
	int posX = w - (h * numIcons);
	int i = 0;
	while (i < numIcons) {
		wxBitmap* icon = vt->icons[i + startIconNumber]->icon;
		if (icon->IsOk()) {
			if (i == selection) {
				dc.SetBrush(wxBrush(Options.GetColour((toggled == i || clicked) ? BUTTON_BACKGROUND_PUSHED : BUTTON_BACKGROUND_HOVER)));
				dc.SetPen(wxPen(Options.GetColour((toggled == i || clicked) ? BUTTON_BORDER_PUSHED : BUTTON_BORDER_HOVER)));
				dc.DrawRoundedRectangle(posX, 1, h - 2, h - 2, 2.0);
			}
			else if (i == toggled) {
				dc.SetBrush(wxBrush(Options.GetColour(BUTTON_BACKGROUND_PUSHED)));
				dc.SetPen(wxPen(Options.GetColour(BUTTON_BORDER_PUSHED)));
				dc.DrawRoundedRectangle(posX, 1, h - 2, h - 2, 2.0);
			}

			dc.DrawBitmap(*icon, posX + ((h - icon->GetHeight()) / 2) - 1, ((h - icon->GetWidth()) / 2), true);
			posX += h;
		}
		i++;
	}
}

void MoveItem::Synchronize(VisualItem* item)
{
	MoveItem* mi = (MoveItem*)item;
	toggled = mi->toggled;
}

void RotationXYItem::OnMouseEvent(wxMouseEvent& evt, int w, int h, VideoToolbar* vt)
{
	int startDrawPos = w - (h * numIcons);
	int x, y;
	evt.GetPosition(&x, &y);
	int elem = ((x - startDrawPos) / h);
	if (evt.Leaving() || elem < 0 || x < startDrawPos) {
		selection = -1;
		clicked = false;
		vt->Refresh(false);
		if (vt->HasToolTips()) { vt->UnsetToolTip(); }
		return;
	}
	if (elem >= numIcons)
		return;

	if (elem != selection) {
		selection = elem;
		vt->SetToolTip(vt->icons[elem + startIconNumber]->help);
		vt->Refresh(false);
	}
	if (evt.LeftDown()) {
		toggled = (toggled == elem) ? -1 : elem;
		clicked = true;
		vt->Refresh(false);
	}
	if (evt.LeftUp()) {
		clicked = false;
		vt->Refresh(false);
		wxCommandEvent* evt = new wxCommandEvent(wxEVT_COMMAND_MENU_SELECTED, ID_MOVE_TOOLBAR_EVENT);
		evt->SetInt(toggled);
		wxQueueEvent(vt, evt);
	}
}

void RotationXYItem::OnPaint(wxDC& dc, int w, int h, VideoToolbar* vt)
{
	int posX = w - (h * numIcons);
	int i = 0;
	while (i < numIcons) {
		wxBitmap* icon = vt->icons[i + startIconNumber]->icon;
		if (icon->IsOk()) {
			if (i == selection) {
				dc.SetBrush(wxBrush(Options.GetColour((toggled == i || clicked) ? BUTTON_BACKGROUND_PUSHED : BUTTON_BACKGROUND_HOVER)));
				dc.SetPen(wxPen(Options.GetColour((toggled == i || clicked) ? BUTTON_BORDER_PUSHED : BUTTON_BORDER_HOVER)));
				dc.DrawRoundedRectangle(posX, 1, h - 2, h - 2, 2.0);
			}
			else if (i == toggled) {
				dc.SetBrush(wxBrush(Options.GetColour(BUTTON_BACKGROUND_PUSHED)));
				dc.SetPen(wxPen(Options.GetColour(BUTTON_BORDER_PUSHED)));
				dc.DrawRoundedRectangle(posX, 1, h - 2, h - 2, 2.0);
			}

			dc.DrawBitmap(*icon, posX + ((h - icon->GetHeight()) / 2) - 1, ((h - icon->GetWidth()) / 2), true);
			posX += h;
		}
		i++;
	}
}

void RotationXYItem::Synchronize(VisualItem* item)
{
	RotationXYItem* rxyi = (RotationXYItem*)item;
	toggled = rxyi->toggled;
}

#endif