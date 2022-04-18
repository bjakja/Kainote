//  Copyright (c) 2017 - 2020, Marcin Drob

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

#include "KaiDialog.h"
#include "MappedButton.h"
#include "KaiTextCtrl.h"
#include "KaiRadioButton.h"
#include "KaiTabBar.h"
#include "KaiTreeBook.h"
#include "config.h"
#include "wx/dcmemory.h"
#include "wx/dcclient.h"
#include "wx/msw/private.h"
#include <Dwmapi.h>
#include <Windowsx.h>


//#define GET_X_LPARAM(lp)                        ((int)(short)LOWORD(lp))
//#define GET_Y_LPARAM(lp)                        ((int)(short)HIWORD(lp))

int border = 5;
int topBorder = 24;

DialogSizer::DialogSizer(int orient)
	:wxBoxSizer(orient)
{
}


//void DialogSizer::RepositionChildren(const wxSize& minSize)
void DialogSizer::RecalcSizes()
{
	wxSize old_size(m_size);
	m_size.x -= 2 * border;
	m_size.y -= border + topBorder;
	wxPoint old_pos(m_position);
	m_position.x += border;
	m_position.y += topBorder;

	wxBoxSizer::RecalcSizes();

	m_position = old_pos;
	m_size = old_size;
}

wxSize DialogSizer::CalcMin()
{
	wxSize ret(wxBoxSizer::CalcMin());
	ret.x += 2 * border;

	ret.y += border + topBorder;

	return ret;

}

KaiDialog::KaiDialog(wxWindow *parent, wxWindowID id,
	const wxString& title,
	const wxPoint& pos,
	const wxSize& size,
	long _style)
	:loop(NULL)
	, escapeId(wxID_CANCEL)
	, enterId(wxID_OK)
	, enter(false)
	, pushed(false)
	, isActive(true)
	, style(_style)
{
	SetExtraStyle(GetExtraStyle() | wxTOPLEVEL_EX_DIALOG | wxWS_EX_BLOCK_EVENTS);// | wxCLIP_CHILDREN
	Create(parent, id, title, pos, size, wxBORDER_NONE | wxTAB_TRAVERSAL);
	if ( !m_hasFont )
		SetFont(*Options.GetFont());
	
	SetForegroundColour(Options.GetColour(WINDOW_TEXT));
	SetBackgroundColour(Options.GetColour(WINDOW_BACKGROUND));
	Bind(wxEVT_SIZE, &KaiDialog::OnSize, this);
	Bind(wxEVT_PAINT, &KaiDialog::OnPaint, this);
	if (!(_style & wxWANTS_CHARS)){ 
		Bind(wxEVT_CHAR_HOOK, &KaiDialog::OnCharHook, this); 
		Bind(wxEVT_NAVIGATION_KEY, &KaiDialog::OnNavigation, this);
	}
	Bind(wxEVT_LEFT_DOWN, &KaiDialog::OnMouseEvent, this);
	Bind(wxEVT_LEFT_UP, &KaiDialog::OnMouseEvent, this);
	Bind(wxEVT_LEFT_DCLICK, &KaiDialog::OnMouseEvent, this);
	Bind(wxEVT_MOTION, &KaiDialog::OnMouseEvent, this);
	Bind(wxEVT_LEAVE_WINDOW, &KaiDialog::OnMouseEvent, this);
	//Bind(wxEVT_ACTIVATE, &KaiDialog::OnActivate, this);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &KaiDialog::OnEscape, this, escapeId);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &KaiDialog::OnEnter, this, enterId);
	//Bind(wxEVT_ERASE_BACKGROUND,[=](wxEraseEvent &evt){Refresh(false);});
}

KaiDialog::~KaiDialog()
{

}

int KaiDialog::ShowModal()
{
	int result = wxID_CANCEL;
	Show();
	if (IsShown()){
		loop = new wxModalEventLoop(this);
		if (!loop){ return result; }
		result = loop->Run();
		delete loop;
		loop = NULL;
	}
	return result;
}

void KaiDialog::EndModal(int retCode)
{
	if (loop)loop->Exit(retCode);
	wxTopLevelWindow::Show(false);
}

bool KaiDialog::IsModal() const
{
	return (loop != NULL);
}

bool KaiDialog::Show(bool show)
{
	if (IsShown() == show){
		return false;
	}
	if (!show){
		return Hide();
	}
	else{
		wxWindow *win = FindWindow((setEscapeIdWithFocus) ? escapeId : enterId);
		bool wasShown = wxTopLevelWindow::Show();
		if (win){ win->SetFocus(); }
		return wasShown;
	}
}

bool KaiDialog::Hide()
{
	if (IsShown() || loop){
		EndModal(escapeId); return true;
	}
	return false;
}

bool KaiDialog::IsButtonFocused()
{
	wxWindow *focused = FindFocus();
	return (focused && focused->IsKindOf(wxCLASSINFO(MappedButton)));
}

//void KaiDialog::SetFocusFromNode(wxWindowListNode* node, wxWindowList& list, bool next)
//{
//	if (!node)
//		return;
//
//	auto nextWindow = next? node->GetNext() : node->GetPrevious();
//	while (1) {
//		if (!nextWindow) {
//			nextWindow = next ? list.GetFirst() : list.GetLast();
//			wxObject* data = nextWindow->GetData();
//			if (data) {
//				wxWindow* win = wxDynamicCast(data, wxWindow);
//				if (win && win->IsFocusable()) {
//					win->SetFocus(); return;
//				}
//			}
//		}
//		else if (nextWindow) {
//			wxObject* data = nextWindow->GetData();
//			if (data) {
//				wxWindow* win = wxDynamicCast(data, wxWindow);
//				if (win && win->IsFocusable()) {
//					win->SetFocus(); return;
//				}
//			}
//		}
//		nextWindow = next? nextWindow->GetNext() : nextWindow->GetPrevious();
//	}
//}

wxWindowListNode* KaiDialog::GetTabControl(bool next, wxWindow* focused)
{
	wxWindow* tab = NULL;
	//every new multitab controls have to be added here
	if (focused->IsKindOf(CLASSINFO(KaiTabBar))) {
		KaiTabBar* ktb = wxDynamicCast(focused, KaiTabBar);
		if (ktb)
			tab = ktb->GetTab();
	}
	else if (focused->IsKindOf(CLASSINFO(KaiTreebook))) {
		KaiTreebook* ktb = wxDynamicCast(focused, KaiTreebook);
		if (ktb)
			tab = ktb->GetTab();
	}
	if (tab) {
		const wxWindowList& tablist = tab->GetChildren();
		return next ? tablist.GetFirst() : tablist.GetLast();
	}
	return NULL;
}

wxWindow* KaiDialog::FindCheckedRadiobutton(bool next, wxWindowListNode** listWithRadioButton, wxWindow* focused)
{
	wxWindow* result = NULL;
	bool beforeGroup = false;
	while (1) {
		if ((*listWithRadioButton)) {
			wxObject* data = (*listWithRadioButton)->GetData();
			if (data) {
				wxWindow* win = wxDynamicCast(data, wxWindow);
				if (win && win->IsFocusable()) {
					if (win->IsKindOf(CLASSINFO(KaiRadioButton))) {
						KaiRadioButton* krb = wxDynamicCast(win, KaiRadioButton);
						if (krb) {
							if (beforeGroup) {
								break;
							}
							if (krb->HasFlag(wxRB_GROUP) || krb->HasFlag(wxRB_SINGLE)) {
								if (next && focused->IsKindOf(CLASSINFO(KaiRadioButton)))
									break;
								else if (!next)
									beforeGroup = true;
							}
							if (krb->GetValue()) {
								result = win;
								break;
							}
						}
					}
					else{
						result = win;
						break;
					}
				}
				
			}
		}
		else
			break;

		(*listWithRadioButton) = next ? (*listWithRadioButton)->GetNext() :
			(*listWithRadioButton)->GetPrevious();
	}
	return result;
}

void KaiDialog::SetNextControl(bool next)
{
	// what if someone put only statictext to entire dialog?
	// avoid somehow infinite loop
	bool positionWasReset = false;
	bool goToGrandparent = false;
	wxWindow* focused = FindFocus();
	wxWindow* focusedParent = focused->GetParent();
	if (focusedParent->IsKindOf(CLASSINFO(KaiChoice)) || 
		focusedParent->IsKindOf(CLASSINFO(KaiRadioBox))) {
		focused = focusedParent;
		focusedParent = focused->GetParent();
	}

	wxWindow* focusedGrandParent = (focusedParent->IsTopLevel())? NULL : focusedParent->GetParent();
	bool hasMultiplePages = focused->HasMultiplePages();

	const wxWindowList& list = focusedParent->GetChildren();
	auto node = list.Find(focused);
	if (node) {
		auto nextWindow = next ? node->GetNext() : node->GetPrevious();
		while (1) {
			//if tabbar or treebook is only children then go to its tab
			if (hasMultiplePages && (next || list.GetCount() == 1)) {
				nextWindow = GetTabControl(next, focused);
				//have to disable option to go in this place again
				//even if was found window to avoid unfinite loop
				//when window was found then have to seek nexte element on list
				hasMultiplePages = false;
			}
			if (!nextWindow) {
				//when there was a reset position it means that 
				//there is no controls or there is no focusable controls
				//avoid to infinite loop
				if (positionWasReset)
					break;

				positionWasReset = true;
				//case for tabs and treebook when there are two windows in dialog and controls are third in row.
				//need to move to tab level
				if (focusedGrandParent && focusedGrandParent->HasMultiplePages()) {
					wxWindow * parentOfTabs = focusedGrandParent->GetParent();
					if (parentOfTabs) {
						const wxWindowList& tablist = parentOfTabs->GetChildren();
						auto tabsnode = tablist.Find(focusedGrandParent);
						if (tabsnode) {
							nextWindow = next ? tabsnode->GetNext() : tabsnode;
							if (!nextWindow)
								nextWindow = tabsnode;
							goToGrandparent = true;
						}
					}
				}
				if(!nextWindow) {
					nextWindow = next ? list.GetFirst() : list.GetLast();
				}
			}
			if (nextWindow) {
				wxObject* data = nextWindow->GetData();
				if (data) {
					wxWindow* win = wxDynamicCast(data, wxWindow);
					if (win && win->IsFocusable()) {
						if (win->IsKindOf(CLASSINFO(KaiRadioButton)) && !win->HasFlag(wxRB_SINGLE)) {
							win = FindCheckedRadiobutton(next, &nextWindow, focused);
							if (!win) {
								//get next window before continue
								nextWindow = next ? nextWindow->GetNext() : nextWindow->GetPrevious();
								continue;
							}
						}
						if (!next && win->HasMultiplePages() && !goToGrandparent) {
							auto nodetc = GetTabControl(next, win);
							if (nodetc) {
								wxObject* datatc = nodetc->GetData();
								if (datatc) {
									wxWindow* wintc = wxDynamicCast(datatc, wxWindow);
									if (wintc && wintc->IsFocusable()) {
										wintc->SetFocus();
										return;
									}
									//if window is not focusable then set nextWindow to seek again window
									nextWindow = nodetc;
								}
							}
						}
						else {
							win->SetFocus(); 
							return;
						}
					}
				}
			}
			nextWindow = next ? nextWindow->GetNext() : nextWindow->GetPrevious();
		}
	}
}

void KaiDialog::OnNavigation(wxNavigationKeyEvent& evt)
{
	SetNextControl(evt.GetDirection());
}

void KaiDialog::OnCharHook(wxKeyEvent &evt)
{
	const int key = evt.GetKeyCode();
	if (evt.GetModifiers() != 0){
		evt.Skip();
		return;
	}

	if (key == WXK_ESCAPE || key == WXK_RETURN){
		wxWindow* focused = FindFocus();
		if (focused) {
			if (key == WXK_RETURN && focused->IsKindOf(wxCLASSINFO(MappedButton))){ evt.Skip(); return; }
			bool processEnter = (focused->GetWindowStyle() & wxTE_PROCESS_ENTER) != 0;
			if (!processEnter && focused->IsKindOf(wxCLASSINFO(KaiTextCtrl))) {
				evt.Skip(); return;
			}
		}
		wxCommandEvent evt(wxEVT_COMMAND_BUTTON_CLICKED, (key == WXK_ESCAPE) ? escapeId : enterId);
		ProcessEvent(evt);
		return;
	}
	/*else if (key == WXK_TAB && (evt.GetModifiers() == 0 || evt.GetModifiers() == wxMOD_SHIFT)){
		bool nextControl = evt.GetModifiers() == 0;
		SetNextControl(nextControl);
		return;
	}*/
	evt.Skip();
}

void KaiDialog::OnPaint(wxPaintEvent &evt)
{
	int w, h;
	GetClientSize(&w, &h);
	if (w < 1 || h < 1){ return; }
	wxPaintDC dc(this);
	wxMemoryDC mdc;
	wxBitmap kaiDialogBitmap(w, h);
	mdc.SelectObject(kaiDialogBitmap);
	mdc.SetFont(GetFont());
	wxColour bg = (isActive) ? Options.GetColour(WINDOW_BORDER_BACKGROUND) : Options.GetColour(WINDOW_BORDER_BACKGROUND_INACTIVE);
	mdc.SetBrush(bg);
	mdc.SetPen((isActive) ? Options.GetColour(WINDOW_BORDER) : Options.GetColour(WINDOW_BORDER_INACTIVE));
	mdc.DrawRectangle(0, 0, w, h);
	wxColour text = (isActive) ? Options.GetColour(WINDOW_HEADER_TEXT) : Options.GetColour(WINDOW_HEADER_TEXT_INACTIVE);
	mdc.SetTextForeground(text);
	wxIconBundle icon = GetIcons();
	if (icon.GetIconCount()){
		mdc.DrawIcon(icon.GetIconByIndex(0), 6, (topBorder - 16) / 2);
	}
	wxString title = GetTitle();
	if (title != emptyString){
		int start = icon.GetIconCount() ? 28 : 8;
		int removed = 0, fw = 0, fh = 0;
		mdc.GetTextExtent(title, &fw, &fh);
		while (fw > w - 22 - start && title != emptyString){
			mdc.GetTextExtent(title, &fw, &fh);
			title = title.RemoveLast();
			removed++;
		}
		if (removed > 0){
			title = title.RemoveLast(2).Trim() + L"...";
		}
		mdc.DrawText(title, start, 4);
	}
	if (enter || pushed){
		wxColour buttonxbg = (enter && !pushed) ? Options.GetColour(WINDOW_HOVER_CLOSE_BUTTON) :
			Options.GetColour(WINDOW_PUSHED_CLOSE_BUTTON);
		mdc.SetBrush(buttonxbg);
		mdc.SetPen(buttonxbg);
		//mdc.DrawRectangle(w - 25, 3, 18, 18);
		int buttonScale = ((topBorder - 8) / 2) * 2;
		buttonScale = (buttonScale < 18) ? 18 : buttonScale;
		mdc.DrawRectangle(w - topBorder - 1, 4, buttonScale, buttonScale);
	}
	//mdc.DrawText("X", w-20, 4);
	mdc.SetPen(wxPen(text, 2));
	mdc.DrawLine(w - topBorder + 3, 8, w - (border + 8), topBorder - 8);
	mdc.DrawLine(w - (border + 8), 8, w - topBorder + 3, topBorder - 8);
	//mdc.DrawLine(w - 21, 7, w - 12, 16);
	//mdc.DrawLine(w - 12, 7, w - 21, 16);
	dc.Blit(0, 0, w, topBorder, &mdc, 0, 0);
	dc.Blit(0, topBorder, border, h - topBorder - border, &mdc, 0, topBorder);
	dc.Blit(w - border, topBorder, border, h - topBorder - border, &mdc, w - border, topBorder);
	dc.Blit(0, h - border, w, border, &mdc, 0, h - border);
}

void KaiDialog::OnSize(wxSizeEvent &evt)
{
	Refresh(false);
	evt.Skip();
}

void KaiDialog::OnMouseEvent(wxMouseEvent &evt)
{
	int w, h;
	GetClientSize(&w, &h);
	int x = evt.GetX();
	int y = evt.GetY();
	wxRect rc(w - topBorder - 5, 0, topBorder - 1, topBorder - 1);
	if (evt.Leaving()){
		pushed = enter = false;
		Refresh(false, &rc);
		return;
	}
	bool leftdown = evt.LeftDown() || evt.LeftDClick();
	if (leftdown){
		wxActivateEvent evt(wxEVT_ACTIVATE, true);
		OnActivate(evt);
	}
	if (x >= w - topBorder - 1 && x < w - border - 3 && y >= 5 && y < topBorder - 3){
		if (leftdown){ pushed = true; Refresh(false, &rc); }
		if (!enter){ enter = true; Refresh(false, &rc); }
		if (evt.LeftUp()){
			pushed = enter = false;
			Refresh(false, &rc);
			MappedButton *btn = wxDynamicCast(FindWindow(escapeId), MappedButton);
			if (btn && btn->IsShown() && btn->IsEnabled()){
				wxCommandEvent evt(wxEVT_COMMAND_BUTTON_CLICKED, escapeId);
				ProcessEvent(evt);
				return;
			}
			EndModal(escapeId);

		}
		return;
	}
	else if (enter || pushed){
		pushed = enter = false;
		Refresh(false, &rc);
	}
	//if(leftdown && y<=topBorder){
	//::SendMessage( m_hWnd, WM_SYSCOMMAND, 0xF012, 0);
	//}
	evt.Skip();
}


void KaiDialog::SetSizerAndFit1(wxSizer *sizer, bool deleteOld)
{
	SetSizer(sizer, deleteOld);
	wxSize siz = sizer->GetMinSize();
	sizer->SetDimension(border, topBorder, siz.x + (2 * border), siz.y + border + topBorder);
	//sizer->SetSizeHints(this);
}

//void KaiDialog::OnActivate(wxActivateEvent &evt)
//{
//	isActive = evt.GetActive();
//	int w, h;
//	GetClientSize(&w,&h);
//	wxRect rc(0,0,w,topBorder);
//	Refresh(false, &rc);
//	wxRect rc1(0,topBorder,border,h-border-topBorder);
//	Refresh(false, &rc1);
//	wxRect rc2(w-border,topBorder,border,h-border-topBorder);
//	Refresh(false, &rc2);
//	wxRect rc3(0,h-border,w,border);
//	Refresh(false, &rc3);
//	Update();
//}

void KaiDialog::SetEnterId(int _enterId, bool bind)
{
	Unbind(wxEVT_COMMAND_BUTTON_CLICKED, &KaiDialog::OnEnter, this, enterId);
	enterId = _enterId;
	if(bind)
		Bind(wxEVT_COMMAND_BUTTON_CLICKED, &KaiDialog::OnEnter, this, enterId);

}
void KaiDialog::SetEscapeId(int _escapeId, bool setFocus)
{
	Unbind(wxEVT_COMMAND_BUTTON_CLICKED, &KaiDialog::OnEscape, this, escapeId);
	escapeId = _escapeId;
	setEscapeIdWithFocus = setFocus;
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &KaiDialog::OnEscape, this, escapeId);
}

void KaiDialog::OnEnter(wxCommandEvent &evt)
{
	if (IsModal())
		EndModal(enterId);

	evt.Skip();
}
void KaiDialog::OnEscape(wxCommandEvent &evt)
{
	EndModal(escapeId); evt.Skip();
}

void KaiDialog::SetLabel(const wxString &text)
{
	wxTopLevelWindow::SetLabel(text);
	int w, h;
	GetSize(&w, &h);
	wxRect rc(0, 0, w, topBorder);
	Refresh(false, &rc);
}
WXLRESULT KaiDialog::MSWWindowProc(WXUINT uMsg, WXWPARAM wParam, WXLPARAM lParam)
{
	//if(uMsg == WM_SIZING){
	//	RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE | RDW_INTERNALPAINT);//| RDW_NOERASE
	//}
	if (uMsg == WM_ACTIVATE){
		isActive = (wParam != 0);//evt.GetActive();
		int w, h;
		GetClientSize(&w, &h);
		wxRect rc(0, 0, w, topBorder);
		Refresh(false, &rc);
		wxRect rc1(0, topBorder, border, h - border - topBorder);
		Refresh(false, &rc1);
		wxRect rc2(w - border, topBorder, border, h - border - topBorder);
		Refresh(false, &rc2);
		wxRect rc3(0, h - border, w, border);
		Refresh(false, &rc3);
		Update();
	}
	if (uMsg == WM_NCHITTEST){
		RECT WindowRect;
		int x, y;

		GetWindowRect(m_hWnd, &WindowRect);
		x = GET_X_LPARAM(lParam) - WindowRect.left;
		y = GET_Y_LPARAM(lParam) - WindowRect.top;

		if (x >= border && x <= WindowRect.right - WindowRect.left - topBorder - 5 && y >= border && y <= topBorder)
			return HTCAPTION;
		else if (style & wxRESIZE_BORDER){
			int result = 0;
			if (x < border && y < border)
				result = HTTOPLEFT;
			else if (x > WindowRect.right - WindowRect.left - border && y < border)
				result = HTTOPRIGHT;
			else if (x > WindowRect.right - WindowRect.left - border && y > WindowRect.bottom - WindowRect.top - border)
				result = HTBOTTOMRIGHT;
			else if (x < border && y > WindowRect.bottom - WindowRect.top - border)
				result = HTBOTTOMLEFT;
			else if (x < border)
				result = HTLEFT;
			else if (y < border)
				result = HTTOP;
			else if (x > WindowRect.right - WindowRect.left - border)
				result = HTRIGHT;
			else if (y > WindowRect.bottom - WindowRect.top - border)
				result = HTBOTTOM;
			else
				result = HTCLIENT;

			if (result != HTCLIENT && (enter || pushed)){
				enter = pushed = false;
				wxRect rc(0, 0, WindowRect.right - WindowRect.left, topBorder);
				Refresh(false, &rc);
			}
			return result;
		}
		else
			return HTCLIENT;

	}

	return wxTopLevelWindow::MSWWindowProc(uMsg, wParam, lParam);
}

bool KaiDialog::SetFont(const wxFont &font)
{
	int fw, fh;
	GetTextExtent(GetTitle(), &fw, &fh, 0, 0, &font);
	topBorder = (fh + 8 < 24) ? 24 : fh + 8;
	//cannot set font in dialogs cause of static text sizer
	//is not window cannot make it change sizes
	/*const wxWindowList& siblings = GetChildren();
	for (wxWindowList::compatibility_iterator nodeAfter = siblings.GetFirst();
		nodeAfter;
		nodeAfter = nodeAfter->GetNext()){

		wxWindow *win = nodeAfter->GetData();
		win->SetFont(font);
	}*/

	wxWindow::SetFont(font);
	//Layout();
	return true;
}

wxIMPLEMENT_ABSTRACT_CLASS(KaiDialog, wxTopLevelWindow);