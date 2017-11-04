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

#include "KaiFrame.h"
#include <wx/dcclient.h>
#include <wx/dcmemory.h>
#include "config.h"
#include "wx/msw/private.h"
#include <Dwmapi.h>
#pragma comment(lib, "Dwmapi.lib")


int fborder = 7;
int ftopBorder = 26;


KaiFrame::KaiFrame(wxWindow *parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long _style)
	:wxTopLevelWindow(parent, id, title, wxDefaultPosition, wxDefaultSize,/*wxBORDER_NONE|*/wxMAXIMIZE_BOX|wxSYSTEM_MENU|wxMINIMIZE_BOX|wxCLOSE_BOX|wxRESIZE_BORDER|wxCAPTION)
	,style(_style)
	,enterClose(false)
	,pushedClose(false)
	,enterMaximize(false)
	,pushedMaximize(false)
	,enterMinimize(false)
	,pushedMinimize(false)
	,isActive(true)
{
	//AdjustWindowRectEx(&rcFrame, WS_OVERLAPPEDWINDOW & ~WS_CAPTION, FALSE, NULL);
	//SetWindowLong( m_hWnd, GWL_STYLE, /*GetWindowLong(m_hWnd, GWL_STYLE) | */WS_OVERLAPPEDWINDOW | WS_THICKFRAME | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);
	MARGINS borderless = {0,0,0,0};
	DwmExtendFrameIntoClientArea(m_hWnd, &borderless);
	SetFont(wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT));
	SetForegroundColour(Options.GetColour(WindowText));
	SetBackgroundColour(Options.GetColour(WindowBackground));
	//Bind(wxEVT_SIZE, &KaiFrame::OnSize, this);
	Bind(wxEVT_PAINT, &KaiFrame::OnPaint, this);
	Bind(wxEVT_LEFT_DOWN, &KaiFrame::OnMouseEvent, this);
	Bind(wxEVT_LEFT_UP, &KaiFrame::OnMouseEvent, this);
	Bind(wxEVT_LEFT_DCLICK, &KaiFrame::OnMouseEvent, this);
	Bind(wxEVT_MOTION, &KaiFrame::OnMouseEvent, this);
	Bind(wxEVT_ACTIVATE, &KaiFrame::OnActivate, this);
	Bind(wxEVT_ERASE_BACKGROUND, [=](wxEraseEvent &evt){});
	SetSize(pos.x,pos.y,size.x, size.y);
}

KaiFrame::~KaiFrame()
{

}

void KaiFrame::OnPaint(wxPaintEvent &evt)
{
	int w, h;
	GetSize(&w,&h);
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
	int maximizeDiff = (IsMaximized())? 3 : 0;
	wxIconBundle icons = GetIcons();
	if(icons.GetIconCount()){
		//if(icons.GetIconByIndex(0).GetHeight()!=16){
		wxImage img = wxBitmap(icons.GetIconByIndex(0)).ConvertToImage();
		img = img.Scale(16,16,wxIMAGE_QUALITY_BILINEAR);
		mdc.DrawBitmap(wxBitmap(img), 8 + maximizeDiff, 5 + maximizeDiff);
		//}else{
		//mdc.DrawIcon(icons.GetIconByIndex(0), 4, 4);
		//}
	}
	if(GetTitle()!=""){
		mdc.DrawText(GetTitle(), icons.GetIconCount()? 30 : 6 + maximizeDiff, 5 + maximizeDiff);
	}
	if(enterClose || pushedClose){
		wxColour buttonxbg = (enterClose && !pushedClose)? Options.GetColour(WindowHoverCloseButton) : 
			Options.GetColour(WindowPushedCloseButton);
		mdc.SetBrush(buttonxbg);
		mdc.SetPen(buttonxbg);
		mdc.DrawRectangle(w-25 - maximizeDiff, 4 + maximizeDiff, 18, 18);
	}else if(enterMaximize || pushedMaximize){
		wxColour buttonxbg = (enterMaximize && !pushedMaximize)? Options.GetColour(WindowHoverHeaderElement) : 
			Options.GetColour(WindowPushedHeaderElement);
		mdc.SetBrush(buttonxbg);
		mdc.SetPen(buttonxbg);
		mdc.DrawRectangle(w-50 - maximizeDiff, 4 + maximizeDiff, 18, 18);
	}else if(enterMinimize || pushedMinimize){
		wxColour buttonxbg = (enterMinimize && !pushedMinimize)? Options.GetColour(WindowHoverHeaderElement) : 
			Options.GetColour(WindowPushedHeaderElement);
		mdc.SetBrush(buttonxbg);
		mdc.SetPen(buttonxbg);
		mdc.DrawRectangle(w-75 - maximizeDiff, 4 + maximizeDiff, 18, 18);
	}
	mdc.SetPen(wxPen(text,2));
	mdc.SetBrush(wxBrush(text));
	//draw X
	mdc.DrawLine(w-21 - maximizeDiff,8 + maximizeDiff, w-12 - maximizeDiff,16 + maximizeDiff);
	mdc.DrawLine(w-12 - maximizeDiff,8 + maximizeDiff, w-21 - maximizeDiff,16 + maximizeDiff);
	//draw maximize

	if(IsMaximized()){
		mdc.SetPen(text);
		mdc.SetBrush(*wxTRANSPARENT_BRUSH);
		mdc.DrawRectangle(w-44 - maximizeDiff,8 + maximizeDiff, 9,7);
		mdc.DrawRectangle(w-47 - maximizeDiff,11 + maximizeDiff, 9,7);
		mdc.SetPen(*wxTRANSPARENT_PEN);
	}else{
		mdc.SetPen(*wxTRANSPARENT_PEN);
		mdc.DrawRectangle(w-47 - maximizeDiff,7 + maximizeDiff, 1,12);
		mdc.DrawRectangle(w-47 - maximizeDiff,7 + maximizeDiff, 12,2);
		mdc.DrawRectangle(w-47 - maximizeDiff,18 + maximizeDiff, 12,1);
		mdc.DrawRectangle(w-36 - maximizeDiff,7 + maximizeDiff, 1,12);
	}
	//draw minimize
	mdc.SetBrush(wxBrush(text));
	mdc.DrawRectangle(w-72 - maximizeDiff,17 + maximizeDiff, 12,2);

	dc.Blit(0,0,w,ftopBorder, &mdc, 0, 0);
	dc.Blit(0,ftopBorder,fborder,h-ftopBorder-fborder, &mdc, 0, ftopBorder);
	dc.Blit(w-fborder,ftopBorder,fborder,h-ftopBorder-fborder, &mdc, w-fborder, ftopBorder);
	dc.Blit(0,h-fborder,w,fborder, &mdc, 0, h-fborder);
}

void KaiFrame::SetLabel(const wxString &text)
{
	wxTopLevelWindow::SetLabel(text);
	int w, h;
	GetSize(&w,&h);
	wxRect rc(0,0,w,ftopBorder);
	Refresh(false, &rc);
}

void KaiFrame::OnSize(wxSizeEvent &evt)
{
	//Refresh(false);
	int w, h;
	GetSize(&w,&h);
	wxRect rc(0,0,w,ftopBorder);
	Refresh(false, &rc);
	if(!IsMaximized()){
		wxRect rc1(0,ftopBorder,fborder,h-fborder-ftopBorder);
		Refresh(false, &rc1);
		wxRect rc2(w-fborder,ftopBorder,fborder,h-fborder-ftopBorder);
		Refresh(false, &rc2);
		wxRect rc3(0,h-fborder,w,fborder);
		Refresh(false, &rc3);
	}
	//Update();
	evt.Skip();
}

void KaiFrame::OnMouseEvent(wxMouseEvent &evt)
{
	int w, h;
	GetSize(&w,&h);
	int x = evt.GetX();
	int y = evt.GetY();
	wxRect rc(w-78, 0, 70, ftopBorder);
	if(evt.Leaving()){
		pushedClose = enterClose = pushedMinimize = enterMinimize = pushedMaximize = enterMaximize = false;
		Refresh(false,&rc);
	}
	bool leftdown= evt.LeftDown() || evt.LeftDClick();
	if(leftdown){
		wxActivateEvent evt(wxEVT_ACTIVATE, true);
		OnActivate(evt);
	}
	if(x>=w-25 && x<w-5 && y>=6 && y<21){
		pushedMinimize = enterMinimize = pushedMaximize = enterMaximize = false;
		if(leftdown){pushedClose=true; Refresh(false,&rc);}
		if(!enterClose){enterClose = true; Refresh(false,&rc);}
		if(evt.LeftUp()){
			pushedClose = enterClose = false;
			Refresh(false,&rc);
			Close();
		}
		return;
	}else if(x>=w-50 && x<w-30 && y>=6 && y<21){
		pushedClose = enterClose = pushedMinimize = enterMinimize = false;
		if(leftdown){pushedMaximize=true; Refresh(false,&rc);}
		if(!enterMaximize){enterMaximize = true; Refresh(false,&rc);}
		if(evt.LeftUp()){
			pushedMaximize = enterMaximize = false;
			Refresh(false,&rc);
			Maximize(!IsMaximized());
		}
		return;
	}else if(x>=w-75 && x<w-55 && y>=6 && y<21){
		pushedClose = enterClose = pushedMaximize = enterMaximize = false;
		if(leftdown){pushedMinimize=true; Refresh(false,&rc);}
		if(!enterMinimize){enterMinimize = true; Refresh(false,&rc);}
		if(evt.LeftUp()){
			pushedMinimize = enterMinimize = false;
			Refresh(false,&rc);
			ShowWindow(GetHWND(),SW_SHOWMINNOACTIVE);
		}
		return;
	}else if (enterClose || pushedClose || enterMaximize || pushedMaximize || enterMinimize || pushedMinimize){
		pushedClose = enterClose = pushedMinimize = enterMinimize = pushedMaximize = enterMaximize = false;
		Refresh(false, &rc);
	}
	evt.Skip();
}


void KaiFrame::OnActivate(wxActivateEvent &evt)
{
	isActive = evt.GetActive();
	int w, h;
	GetSize(&w,&h);
	wxRect rc(0,0,w,ftopBorder);
	Refresh(false, &rc);
	if(!IsMaximized()){
		wxRect rc1(0,ftopBorder,fborder,h-fborder-ftopBorder);
		Refresh(false, &rc1);
		wxRect rc2(w-fborder,ftopBorder,fborder,h-fborder-ftopBorder);
		Refresh(false, &rc2);
		wxRect rc3(0,h-fborder,w,fborder);
		Refresh(false, &rc3);
	}
	Update();
}

WXLRESULT KaiFrame::MSWWindowProc(WXUINT uMsg, WXWPARAM wParam, WXLPARAM lParam)
{
	 if (uMsg == WM_SIZE)
    {
        int w = LOWORD(lParam);
		int h = HIWORD(lParam);
		/*if(width<800 || height < 600){
			return 1;
		}*/
		wxRect rc(0,0,w,ftopBorder);
		Refresh(false, &rc);
		if(!IsMaximized()){
			wxRect rc1(0,ftopBorder,fborder,h-fborder-ftopBorder);
			Refresh(false, &rc1);
			wxRect rc2(w-fborder,ftopBorder,fborder,h-fborder-ftopBorder);
			Refresh(false, &rc2);
			wxRect rc3(0,h-fborder,w,fborder);
			Refresh(false, &rc3);
		}
    }
	 
	/*if(uMsg == 28){
		isActive = !isActive;
		int w, h;
		GetSize(&w,&h);
	wxRect rc(0,0,w,ftopBorder);
	Refresh(false, &rc);
	if(!IsMaximized()){
		wxRect rc1(0,ftopBorder,fborder,h-fborder-ftopBorder);
		Refresh(false, &rc1);
		wxRect rc2(w-fborder,ftopBorder,fborder,h-fborder-ftopBorder);
		Refresh(false, &rc2);
		wxRect rc3(0,h-fborder,w,fborder);
		Refresh(false, &rc3);
	}
	return 1;
	}*/
	if (uMsg == WM_GETMINMAXINFO){
		MINMAXINFO * pInfo = (MINMAXINFO*)lParam;
		pInfo->ptMinTrackSize.x = 600;
		pInfo->ptMinTrackSize.y = 400;
		return 0;
	}
	if ((uMsg == WM_NCCALCSIZE)){

		return 0;
	}
	if(uMsg == WM_NCHITTEST){
		//RECT rcFrame = { 0 };
		//AdjustWindowRectEx(&rcFrame, WS_OVERLAPPEDWINDOW & ~WS_CAPTION, FALSE, NULL);
		RECT WindowRect;
		int x, y;

		GetWindowRect(m_hWnd, &WindowRect);
		x = GET_X_LPARAM(lParam) - WindowRect.left;
		y = GET_Y_LPARAM(lParam) - WindowRect.top;
		int result = 0;
		//int w = WindowRect.right - WindowRect.left;
		//int h = WindowRect.bottom - WindowRect.top;
		if (x >= fborder && x <= WindowRect.right - WindowRect.left - 76 && y >= fborder && y <= ftopBorder){
			result = HTCAPTION;
		}else if (style & wxRESIZE_BORDER && !IsMaximized()){

			if (x < fborder && y < fborder)
				result = HTTOPLEFT;
			else if (x > WindowRect.right - WindowRect.left - fborder && y < fborder)
				result = HTTOPRIGHT;
			else if (x > WindowRect.right - WindowRect.left - fborder && y > WindowRect.bottom - WindowRect.top - fborder)
				result = HTBOTTOMRIGHT;
			else if (x < fborder && y > WindowRect.bottom - WindowRect.top - fborder)
				result = HTBOTTOMLEFT;
			else if (x < fborder)
				result = HTLEFT;
			else if (y < fborder)
				result = HTTOP;
			else if (x > WindowRect.right - WindowRect.left - fborder)
				result = HTRIGHT;
			else if (y > WindowRect.bottom - WindowRect.top - fborder)
				result = HTBOTTOM;
			else
				result = HTCLIENT;
		}else{
			return HTCLIENT;
		}
		if(result != HTCLIENT && (enterClose || pushedClose || enterMaximize || pushedMaximize || enterMinimize || pushedMinimize)){
			pushedClose = enterClose = pushedMinimize = enterMinimize = pushedMaximize = enterMaximize = false;
			wxRect rc(0,0,WindowRect.right - WindowRect.left,ftopBorder);
			Refresh(false, &rc);
		}
		return result;
	}

	return wxTopLevelWindow::MSWWindowProc(uMsg, wParam, lParam);
}

void KaiFrame::GetClientSize(int *w, int *h) const
{
	wxTopLevelWindow::GetClientSize(w, h);
	*w -= (fborder * 2);
	*h -= (ftopBorder + fborder);
}

wxSize KaiFrame::GetClientSize() const
{
	wxSize size = wxTopLevelWindow::GetClientSize();
	size.x -= (fborder * 2);
	size.y -= (ftopBorder + fborder);
	return size;
}
void KaiFrame::SetClientSize(const wxSize &size)
{
	SetClientSize(size.x, size.y);
}
void KaiFrame::SetClientSize(int x, int y)
{
	x+=(fborder * 2);
	y+=(ftopBorder + fborder);
	wxWindow::SetClientSize(x,y);
}


wxIMPLEMENT_ABSTRACT_CLASS(KaiFrame, wxTopLevelWindow);
