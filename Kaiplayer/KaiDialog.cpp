//  Copyright (c) 2017, Marcin Drob

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
#include "config.h"
#include "wx/msw/private.h"
#include <Dwmapi.h>
#pragma comment(lib, "Dwmapi.lib")
int border = 5;
int topBorder = 24;

DialogSizer::DialogSizer(int orient)
	:wxBoxSizer(orient)
{
}


void DialogSizer::RecalcSizes()
{
	wxSize old_size( m_size );
	m_size.x -= 2*border;
	m_size.y -= border + topBorder;
	wxPoint old_pos( m_position );
	m_position.x += border;
	m_position.y += topBorder;

	wxBoxSizer::RecalcSizes();

	m_position = old_pos;
	m_size = old_size;
}

wxSize DialogSizer::CalcMin()
{
	wxSize ret( wxBoxSizer::CalcMin() );
	ret.x += 2*border;

	ret.y += border + topBorder;

	return ret;

}

KaiDialog::KaiDialog(wxWindow *parent, wxWindowID id,
					 const wxString& title,
					 const wxPoint& pos,
					 const wxSize& size,
					 long _style)
					 :loop(NULL)
					 ,escapeId(wxID_CANCEL)
					 ,enterId(wxID_OK)
					 ,enter(false)
					 ,pushed(false)
					 ,isActive(true)
					 ,style(_style)
{
	SetExtraStyle(GetExtraStyle() | wxTOPLEVEL_EX_DIALOG | wxWS_EX_BLOCK_EVENTS);// | wxCLIP_CHILDREN
	Create(parent,id, title, pos, size, wxBORDER_NONE|wxTAB_TRAVERSAL);
	if ( !m_hasFont )
		SetFont(wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT));
	SetForegroundColour(Options.GetColour(WindowText));
	SetBackgroundColour(Options.GetColour(WindowBackground));
	Bind(wxEVT_SIZE, &KaiDialog::OnSize, this);
	Bind(wxEVT_PAINT, &KaiDialog::OnPaint, this);
	if(!(_style & wxWANTS_CHARS)){Bind(wxEVT_CHAR_HOOK, &KaiDialog::OnCharHook, this);}
	Bind(wxEVT_LEFT_DOWN, &KaiDialog::OnMouseEvent, this);
	Bind(wxEVT_LEFT_UP, &KaiDialog::OnMouseEvent, this);
	Bind(wxEVT_LEFT_DCLICK, &KaiDialog::OnMouseEvent, this);
	Bind(wxEVT_MOTION, &KaiDialog::OnMouseEvent, this);
	//Bind(wxEVT_ACTIVATE, &KaiDialog::OnActivate, this);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &KaiDialog::OnEscape, this, escapeId);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &KaiDialog::OnEnter, this, enterId);
	//Bind(wxEVT_ERASE_BACKGROUND,[=](wxEraseEvent &evt){});
	wxWindow *win = FindWindow(enterId);
	if(win){win->SetFocus();}
}

KaiDialog::~KaiDialog()
{

}

int KaiDialog::ShowModal()
{
	int result = wxID_CANCEL;
	Show();
	if(IsShown()){
		loop = new wxModalEventLoop(this);
		if(!loop){return result;}
		result = loop->Run();
		delete loop;
		loop = NULL;
	}
	return result;
}

void KaiDialog::EndModal(int retCode)
{
	if(loop)loop->Exit(retCode);
	wxTopLevelWindow::Show(false);
}

bool KaiDialog::IsModal() const
{
	return (loop != NULL);
}

bool KaiDialog::Show(bool show)
{
	if(IsShown() == show){
		return false;
	}
	if(!show){
		return Hide();
	}else{
		return wxTopLevelWindow::Show();
	}
}

bool KaiDialog::Hide()
{
	if(IsShown() || loop){
		EndModal(escapeId);return true;
	}
	return false;
}

bool KaiDialog::IsButtonFocused()
{
	wxWindow *focused = FindFocus();
	return (focused && focused->IsKindOf(wxCLASSINFO(MappedButton)));
}

void KaiDialog::OnCharHook(wxKeyEvent &evt)
{
	const int key = evt.GetKeyCode();
	if(key == WXK_ESCAPE || key == WXK_RETURN){
		if(key == WXK_RETURN && IsButtonFocused()){evt.Skip(); return;}
		wxCommandEvent evt(wxEVT_COMMAND_BUTTON_CLICKED, (key == WXK_ESCAPE)? escapeId : enterId);
		ProcessEvent(evt);
		return;
	}else if(key == WXK_TAB){
		const wxWindowList list = GetChildren();
		auto result = list.Find(FindFocus());
		if(result){
			auto nextWindow = result->GetNext();
			while(1){
				if(!nextWindow){
					wxObject *data = list.GetFirst()->GetData();
					if(data){
						wxWindow *win = wxDynamicCast(data,wxWindow);
						if(win){win->SetFocus();}
					}
					return;
				}else if(nextWindow->GetData()->CanBeFocused()){
					nextWindow->GetData()->SetFocus();
					return;
				}
				nextWindow = nextWindow->GetNext();
			}
		}
		return;
	}
	evt.Skip();
}

void KaiDialog::OnPaint(wxPaintEvent &evt)
{
	int w, h;
	GetClientSize(&w,&h);
	if(w<1 || h<1){return;}
	wxPaintDC dc(this);
	wxMemoryDC mdc;
	mdc.SelectObject(wxBitmap(w,h));
	mdc.SetFont(GetFont());
	wxColour bg = (isActive)? Options.GetColour(WindowBorderBackground) : Options.GetColour(WindowBorderBackgroundInactive);
	mdc.SetBrush(bg);
	mdc.SetPen((isActive)? Options.GetColour(WindowBorder) : Options.GetColour(WindowBorderInactive));
	mdc.DrawRectangle(0,0,w,h);
	wxColour text = (isActive)? Options.GetColour(WindowHeaderText) : Options.GetColour(WindowHeaderTextInactive);
	mdc.SetTextForeground(text);
	wxIconBundle icon = GetIcons();
	if(icon.GetIconCount()){
		mdc.DrawIcon(icon.GetIconByIndex(0), 4, 4);
	}
	if(GetTitle()!=""){
		mdc.DrawText(GetTitle(), icon.GetIconCount()? 26 : 6, 4);
	}
	if(enter || pushed){
		wxColour buttonxbg = (enter && !pushed)? Options.GetColour(WindowHoverCloseButton) : 
			Options.GetColour(WindowPushedCloseButton);
		mdc.SetBrush(buttonxbg);
		mdc.SetPen(buttonxbg);
		mdc.DrawRectangle(w-25, 3, 18, 18);
	}
	//mdc.DrawText("X", w-20, 4);
	mdc.SetPen(wxPen(text,2));
	mdc.DrawLine(w-21,7, w-12,16);
	mdc.DrawLine(w-12,7, w-21,16);
	dc.Blit(0,0,w,topBorder, &mdc, 0, 0);
	dc.Blit(0,topBorder,border,h-topBorder-border, &mdc, 0, topBorder);
	dc.Blit(w-border,topBorder,border,h-topBorder-border, &mdc, w-border, topBorder);
	dc.Blit(0,h-border,w,border, &mdc, 0, h-border);
}

void KaiDialog::OnSize(wxSizeEvent &evt)
{
	Refresh(false);
	evt.Skip();
}

void KaiDialog::OnMouseEvent(wxMouseEvent &evt)
{
	int w, h;
	GetClientSize(&w,&h);
	int x = evt.GetX();
	int y = evt.GetY();
	wxRect rc(w-25, 3, 18, 18);
	if(evt.Leaving()){
		pushed = enter = false;
		Refresh(false,&rc);
	}
	bool leftdown= evt.LeftDown() || evt.LeftDClick();
	if(leftdown){
		wxActivateEvent evt(wxEVT_ACTIVATE, true);
		OnActivate(evt);
	}
	if(x>=w-25 && x<w-5 && y>=6 && y<21){
		if(leftdown){pushed=true; Refresh(false,&rc);}
		if(!enter){enter = true; Refresh(false,&rc);}
		if(evt.LeftUp()){
			pushed = enter = false;
			Refresh(false,&rc);
			EndModal(escapeId);
			wxCommandEvent evt(wxEVT_COMMAND_BUTTON_CLICKED, escapeId);
			ProcessEvent(evt);
		}
		return;
	}else if (enter || pushed){
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
	sizer->SetDimension(border, topBorder, siz.x+(2*border), siz.y+border+topBorder);
	//sizer->SetSizeHints(this);
}

//void KaiDialog::OnActivate(wxActivateEvent &evt)
//{
//	wxLogStatus("focus");
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

void KaiDialog::SetEnterId(int _enterId)
{
	Unbind(wxEVT_COMMAND_BUTTON_CLICKED, &KaiDialog::OnEnter, this, enterId);
	enterId = _enterId;
	//Bind(wxEVT_COMMAND_BUTTON_CLICKED, &KaiDialog::OnEnter,enterId);
	wxWindow *win = FindWindow(enterId);
	if(win){win->SetFocus();}
}
void KaiDialog::SetEscapeId(int _escapeId)
{
	Unbind(wxEVT_COMMAND_BUTTON_CLICKED, &KaiDialog::OnEscape, this, escapeId);
	escapeId = _escapeId;
	//Bind(wxEVT_COMMAND_BUTTON_CLICKED, &KaiDialog::OnEscape,escapeId);
}

void KaiDialog::OnEnter(wxCommandEvent &evt)
{
	EndModal(enterId);evt.Skip();
}
void KaiDialog::OnEscape(wxCommandEvent &evt)
{
	EndModal(escapeId);evt.Skip();
}

WXLRESULT KaiDialog::MSWWindowProc(WXUINT uMsg, WXWPARAM wParam, WXLPARAM lParam)
{
	//if(uMsg == WM_SIZING){
	//	RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE | RDW_INTERNALPAINT);//| RDW_NOERASE
	//}
	if(uMsg == WM_ACTIVATE){
		isActive = (wParam != 0);//evt.GetActive();
		int w, h;
		GetClientSize(&w,&h);
		wxRect rc(0,0,w,topBorder);
		Refresh(false, &rc);
		wxRect rc1(0,topBorder,border,h-border-topBorder);
		Refresh(false, &rc1);
		wxRect rc2(w-border,topBorder,border,h-border-topBorder);
		Refresh(false, &rc2);
		wxRect rc3(0,h-border,w,border);
		Refresh(false, &rc3);
		Update();
	}
	if(uMsg == WM_NCHITTEST){
		RECT WindowRect;
		int x, y;

		GetWindowRect(m_hWnd, &WindowRect);
		x = GET_X_LPARAM(lParam) - WindowRect.left;
		y = GET_Y_LPARAM(lParam) - WindowRect.top;

		if (x >= border && x <= WindowRect.right - WindowRect.left - 30 && y >= border && y <= topBorder)
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

			if(result != HTCLIENT && (enter || pushed)){
				enter = pushed = false; 
				wxRect rc(0,0,WindowRect.right - WindowRect.left,topBorder);
				Refresh(false, &rc);
			}
			return result;
		}else 
			return HTCLIENT;

	}

	return wxTopLevelWindow::MSWWindowProc(uMsg, wParam, lParam);
}

wxIMPLEMENT_ABSTRACT_CLASS(KaiDialog, wxTopLevelWindow);