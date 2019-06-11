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


static int startDrawPos = 146;
std::vector<itemdata*> VideoToolbar::icons;

VideoToolbar::VideoToolbar(wxWindow *parent, const wxPoint &pos)
	:wxWindow(parent, -1, pos, wxSize(-1, 22))
	, Toggled(0)
	, sel(-1)
	, clicked(false)
	, blockScroll(false)
	, bmp(NULL)
{
	if (icons.size() == 0){
		//pamiętaj, dodając tutaj elementy, zmień ich wartość w pliku h!!
		icons.push_back(new itemdata(PTR_BITMAP_PNG("cross"), _("Wskaźnik pozycji")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG("position"), _("Przesuwanie tekstu")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG("move"), _("Ruch")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG("scale"), _("Skalowanie")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG("frz"), _("Obrót wokół osi Z")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG("frxy"), _("Obrót wokół osi X / Y")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG("cliprect"), _("Wycinki prostokątne")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG("clip"), _("Wycinki wektorowe")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG("drawing"), _("Rysunki wektorowe")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG("MOVEAll"), _("Zmieniacz pozycji")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG("SCALE_ROTATION"), _("Zmieniacz skali i obrotów")));
		//tutaj mamy ikony dla clipa
		icons.push_back(new itemdata(PTR_BITMAP_PNG("VectorDrag"), _("Przesuń punkty")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG("VectorLine"), _("Dodaj linię")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG("VectorBezier"), _("Dodaj krzywą Beziera")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG("VECTORBSPLINE"), _("Dodaj krzywą B-sklejaną")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG("VectorMove"), _("Dodaj nowy oddzielny punkt")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG("VectorDelete"), _("Usuń element")));
		//ikony move all
		icons.push_back(new itemdata(PTR_BITMAP_PNG("MOVEPOS"), _("Przenieś punkty pozycjonowania")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG("MOVEMOVESTART"), _("Przenieś startowe punkty ruchu")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG("MOVE"), _("Przenieś końcowe punkty ruchu")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG("MOVECLIPS"), _("Przenieś wycinki")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG("MOVEDRAWINGS"), _("Przenieś rysunki,\nużywać tylko w przypadku,\ngdy chcemy przesunąć punkty rysunku,\nnie łączyć z trzema pierwszymi opcjami")));
		icons.push_back(new itemdata(PTR_BITMAP_PNG("MOVEORGS"), _("Przenieś punkty org")));
	}

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
	Connect(wxEVT_LEFT_DOWN, (wxObjectEventFunction)&VideoToolbar::OnMouseEvent);
	Connect(wxEVT_LEFT_UP, (wxObjectEventFunction)&VideoToolbar::OnMouseEvent);
	Connect(wxEVT_MOTION, (wxObjectEventFunction)&VideoToolbar::OnMouseEvent);
	Connect(wxEVT_LEAVE_WINDOW, (wxObjectEventFunction)&VideoToolbar::OnMouseEvent);
	Connect(wxEVT_MOUSEWHEEL, (wxObjectEventFunction)&VideoToolbar::OnMouseEvent);
	wxString movopts[6] = { _("Dwukrotnym kliknięciu na linię (zawsze włączone)"), _("Każdej zmianie linii"),
		_("Kliknięciu na linię lub edycji na pauzie"), _("Kliknięciu na linię lub edycji"),
		_("Edycji na pauzie"), _("Edycji") };
	wxString playopts[4] = { _("Nic"), _("Audio do końca linii"), _("Wideo do końca linii"),
		_("Wideo do początku następnej linii") };
	videoSeekAfter = new KaiChoice(this, ID_SEEK_AFTER, wxPoint(2, 0), wxSize(70, 22), 6, movopts);
	videoSeekAfter->SetSelection(Options.GetInt(MoveVideoToActiveLine));
	videoSeekAfter->SetToolTip(_("Przesuwaj wideo do aktualnej linii po:"));
	videoPlayAfter = new KaiChoice(this, ID_PLAY_AFTER, wxPoint(72, 0), wxSize(70, 22), 4, playopts);
	videoPlayAfter->SetSelection(Options.GetInt(PlayAfterSelection));
	videoPlayAfter->SetToolTip(_("Odtwarzaj po zmianie linii:"));
	Bind(wxEVT_COMMAND_CHOICE_SELECTED, [=](wxCommandEvent &evt){
		Options.SetInt(MoveVideoToActiveLine, videoSeekAfter->GetSelection());
		Options.SaveOptions(true, false);
	}, ID_SEEK_AFTER);
	Bind(wxEVT_COMMAND_CHOICE_SELECTED, [=](wxCommandEvent &evt){
		Options.SetInt(PlayAfterSelection, videoPlayAfter->GetSelection());
		Options.SaveOptions(true, false);
	}, ID_PLAY_AFTER);


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
				tdc.SetBrush(wxBrush(Options.GetColour((toggled || clicked) ? ButtonBackgroundPushed : ButtonBackgroundHover)));
				tdc.SetPen(wxPen(Options.GetColour((toggled || clicked) ? ButtonBorderPushed : ButtonBorderHover)));
				tdc.DrawRoundedRectangle(posX, 1, h - 2, h - 2, 2.0);
			}
			else if (toggled){
				tdc.SetBrush(wxBrush(Options.GetColour(ButtonBackgroundPushed)));
				tdc.SetPen(wxPen(Options.GetColour(ButtonBorderPushed)));
				tdc.DrawRoundedRectangle(posX, 1, h - 2, h - 2, 2.0);
			}

			tdc.DrawBitmap((iconsEnabled) ? *icon : icon->ConvertToDisabled(), posX + 2, 3);
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
				dc.SetBrush(wxBrush(Options.GetColour((toggled || clicked) ? ButtonBackgroundPushed : ButtonBackgroundHover)));
				dc.SetPen(wxPen(Options.GetColour((toggled || clicked) ? ButtonBorderPushed : ButtonBorderHover)));
				dc.DrawRoundedRectangle(posX, 1, h - 2, h - 2, 2.0);
			}
			else if (toggled){
				dc.SetBrush(wxBrush(Options.GetColour(ButtonBackgroundPushed)));
				dc.SetPen(wxPen(Options.GetColour(ButtonBorderPushed)));
				dc.DrawRoundedRectangle(posX, 1, h - 2, h - 2, 2.0);
			}

			dc.DrawBitmap(*icon, posX + 2, 3);
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
				dc.SetBrush(wxBrush(Options.GetColour((toggled == i || clicked) ? ButtonBackgroundPushed : ButtonBackgroundHover)));
				dc.SetPen(wxPen(Options.GetColour((toggled == i || clicked) ? ButtonBorderPushed : ButtonBorderHover)));
				dc.DrawRoundedRectangle(posX, 1, h - 2, h - 2, 2.0);
			}
			else if (i == toggled){
				dc.SetBrush(wxBrush(Options.GetColour(ButtonBackgroundPushed)));
				dc.SetPen(wxPen(Options.GetColour(ButtonBorderPushed)));
				dc.DrawRoundedRectangle(posX, 1, h - 2, h - 2, 2.0);
			}

			dc.DrawBitmap(*icon, posX + 2, 3);
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
				dc.SetBrush(wxBrush(Options.GetColour((toggled == i || clicked) ? ButtonBackgroundPushed : ButtonBackgroundHover)));
				dc.SetPen(wxPen(Options.GetColour((toggled == i || clicked) ? ButtonBorderPushed : ButtonBorderHover)));
				dc.DrawRoundedRectangle(posX, 1, h - 2, h - 2, 2.0);
			}
			else if (i == toggled){
				dc.SetBrush(wxBrush(Options.GetColour(ButtonBackgroundPushed)));
				dc.SetPen(wxPen(Options.GetColour(ButtonBorderPushed)));
				dc.DrawRoundedRectangle(posX, 1, h - 2, h - 2, 2.0);
			}

			dc.DrawBitmap(*icon, posX + 2, 3);
			posX += h;
		}
		i++;
	}
}
