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


std::vector<itemdata*> VideoToolbar::icons;

VideoToolbar::VideoToolbar(wxWindow *parent, const wxPoint &pos, const wxSize &size)
	:wxWindow(parent, -1, pos, size)
	, Toggled(0)
	, sel(-1)
	, clicked(false)
	, blockScroll(false)
	, bmp(NULL)
{
	if (icons.size() == 0){
		//Remember! Adding here elements you must change all in h file!!
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
		icons.push_back(new itemdata(PTR_BITMAP_PNG(L"SCALE_ROTATION"), _("Zmieniacz skali i obrotów")));
		//11
		//Here clip icons
		icons.push_back(new itemdata(PTR_BITMAP_PNG(L"VectorDrag"), _("Przesuń punkty")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG(L"VectorLine"), _("Dodaj linię")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG(L"VectorBezier"), _("Dodaj krzywą Beziera")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG(L"VECTORBSPLINE"), _("Dodaj krzywą B-sklejaną")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG(L"VectorMove"), _("Dodaj nowy oddzielny punkt")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG(L"VectorDelete"), _("Usuń element")));
		//6
		//icons move all
		icons.push_back(new itemdata(PTR_BITMAP_PNG(L"MOVEPOS"), _("Przenieś punkty pozycjonowania")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG(L"MOVEMOVESTART"), _("Przenieś startowe punkty ruchu")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG(L"MOVE"), _("Przenieś końcowe punkty ruchu")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG(L"MOVECLIPS"), _("Przenieś wycinki")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG(L"MOVEDRAWINGS"), _("Przenieś rysunki,\nużywać tylko w przypadku,\ngdy chcemy przesunąć punkty rysunku,\nnie łączyć z trzema pierwszymi opcjami")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG(L"MOVEORGS"), _("Przenieś punkty org")));
		//6
	}
	//adding visual second toolbar elements
	visualItems.push_back(NULL);
	visualItems.push_back(NULL);
	visualItems.push_back(NULL);
	visualItems.push_back(NULL);
	visualItems.push_back(NULL);
	visualItems.push_back(NULL);
	visualItems.push_back(NULL);
	visualItems.push_back(new VectorItem());
	visualItems.push_back(new VectorItem());
	visualItems.push_back(new MoveAllItem());
	visualItems.push_back(new ScaleRotationItem());

	Connect(wxEVT_PAINT, (wxObjectEventFunction)&VideoToolbar::OnPaint);
	Connect(wxEVT_SIZE, (wxObjectEventFunction)&VideoToolbar::OnSize);
	Connect(wxEVT_LEFT_DOWN, (wxObjectEventFunction)&VideoToolbar::OnMouseEvent);
	Connect(wxEVT_LEFT_UP, (wxObjectEventFunction)&VideoToolbar::OnMouseEvent);
	Connect(wxEVT_MOTION, (wxObjectEventFunction)&VideoToolbar::OnMouseEvent);
	Connect(wxEVT_LEAVE_WINDOW, (wxObjectEventFunction)&VideoToolbar::OnMouseEvent);
	Connect(wxEVT_MOUSEWHEEL, (wxObjectEventFunction)&VideoToolbar::OnMouseEvent);
	wxWindow::SetFont(*Options.GetFont(-1));

	wxString movopts[6] = { _("Dwukrotnym kliknięciu na linię (zawsze włączone)"), _("Każdej zmianie linii"),
		_("Kliknięciu na linię lub edycji na pauzie"), _("Kliknięciu na linię lub edycji"),
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
			visualItems[Toggled]->OnMouseEvent(evt, w, h, this);
		noelem = true;
	}
	if (evt.Leaving() || noelem){ sel = -1; Refresh(false); if (HasToolTips()){ UnsetToolTip(); } return; }

	if (elem != sel){
		sel = elem;
		SetToolTip(icons[elem]->help);
		Refresh(false);
	}
	if (evt.LeftDown()){
		if (elem == Toggled){ Toggled = 0; }
		else{ Toggled = elem; }
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
		bmp = NULL;
	}
	if (!bmp){ bmp = new wxBitmap(w, h); }
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
		visualItems[Toggled]->OnPaint(tdc, w, h, this);

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
	int allToolsSize = 18 * size.y;
	//one square for spacing
	int spaceForLists = (size.x - allToolsSize - 6);
	if (spaceForLists < seekMinWidth + playMinWidth){
		seekMinWidth = spaceForLists / 2;
		playMinWidth = seekMinWidth;
		if (seekMinWidth < 70)
			seekMinWidth = 70;
		if (playMinWidth < 70)
			playMinWidth = 70;
	}
	videoSeekAfter->SetSize(seekMinWidth, height);
	videoPlayAfter->SetSize(seekMinWidth + 2, 1, playMinWidth, height);
	
	startDrawPos = playMinWidth + seekMinWidth + 6;
	
	Refresh(false);
}

void VideoToolbar::Synchronize(VideoToolbar *vtoolbar){
	Toggled = vtoolbar->Toggled;
	sel = vtoolbar->sel;
	if (visualItems[Toggled])
		visualItems[Toggled]->Synchronize(vtoolbar->visualItems[Toggled]);

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
	if (evt.Leaving() || elem < 0){
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
	int startDrawPos = w - (h * numIcons);
	int x, y;
	evt.GetPosition(&x, &y);
	int elem = ((x - startDrawPos) / h);
	if (evt.Leaving() || elem < 0){
		selection = -1;
		clicked = false;
		vt->Refresh(false);
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
		wxCommandEvent *evt = new wxCommandEvent(wxEVT_COMMAND_MENU_SELECTED, ID_VECTOR_TOOLBAR_EVENT);
		evt->SetInt(toggled);
		wxQueueEvent(vt, evt);
	}
}

void VectorItem::OnPaint(wxDC &dc, int w, int h, VideoToolbar *vt)
{
	int posX = w - (h * numIcons);
	int i = 0;
	while (i < numIcons){
		wxBitmap *icon = vt->icons[i + startIconNumber]->icon;
		if (icon->IsOk()){
			if (i == selection){
				dc.SetBrush(wxBrush(Options.GetColour((toggled == i || clicked) ? BUTTON_BACKGROUND_PUSHED : BUTTON_BACKGROUND_HOVER)));
				dc.SetPen(wxPen(Options.GetColour((toggled == i || clicked) ? BUTTON_BORDER_PUSHED : BUTTON_BORDER_HOVER)));
				dc.DrawRoundedRectangle(posX, 1, h - 2, h - 2, 2.0);
			}
			else if (i == toggled){
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

void ScaleRotationItem::OnMouseEvent(wxMouseEvent &evt, int w, int h, VideoToolbar *vt)
{
	int startDrawPos = w - (h * numIcons);
	int x, y;
	evt.GetPosition(&x, &y);
	int elem = ((x - startDrawPos) / h);
	if (evt.Leaving() || elem < 0){
		selection = -1;
		clicked = false;
		vt->Refresh(false);
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
		wxCommandEvent *evt = new wxCommandEvent(wxEVT_COMMAND_MENU_SELECTED, ID_SCALE_ROTATE_TOOLBAR_EVENT);
		evt->SetInt(toggled);
		wxQueueEvent(vt, evt);
	}
}

void ScaleRotationItem::OnPaint(wxDC &dc, int w, int h, VideoToolbar *vt)
{
	int posX = w - (h * numIcons);
	int i = 0;
	while (i < numIcons){
		wxBitmap *icon = vt->icons[i + startIconNumber]->icon;
		if (icon->IsOk()){
			if (i == selection){
				dc.SetBrush(wxBrush(Options.GetColour((toggled == i || clicked) ? BUTTON_BACKGROUND_PUSHED : BUTTON_BACKGROUND_HOVER)));
				dc.SetPen(wxPen(Options.GetColour((toggled == i || clicked) ? BUTTON_BORDER_PUSHED : BUTTON_BORDER_HOVER)));
				dc.DrawRoundedRectangle(posX, 1, h - 2, h - 2, 2.0);
			}
			else if (i == toggled){
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
