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

//#include <WindowsX.h>
#include "KaiFrame.h"
#include "Utils.h"
#include <wx/dcclient.h>
#include <wx/dcmemory.h>
#include "config.h"
#include "kainoteApp.h"
#include "wx/msw/private.h"
#include <Dwmapi.h>
#include "GraphicsD2D.h"
#pragma comment(lib, "Dwmapi.lib")
#define GET_X_LPARAM(lp)                        ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp)                        ((int)(short)HIWORD(lp))


KaiFrame::KaiFrame(wxWindow *parent, wxWindowID id, const wxString& title/*=""*/, const wxPoint& pos/*=wxDefaultPosition*/, const wxSize& size/*=wxDefaultSize*/, long _style/*=0*/, const wxString &name /*= ""*/)
	: style(_style)
	, enterClose(false)
	, pushedClose(false)
	, enterMaximize(false)
	, pushedMaximize(false)
	, enterMinimize(false)
	, pushedMinimize(false)
	, isActive(true)
{
	wxSize sizeReal = size;
	if (!sizeReal.IsFullySpecified())
	{
		sizeReal.SetDefaults(GetDefaultSize());
	}

	// notice that we should append this window to wxTopLevelWindows list
	// before calling CreateBase() as it behaves differently for TLW and
	// non-TLW windows
	wxTopLevelWindows.Append(this);

	bool ret = CreateBase(parent, id, pos, sizeReal, style, name);
	if (!ret)
		return;

	if (parent)
		parent->AddChild(this);

	WXDWORD exflags;
	WXDWORD flags = MSWGetCreateWindowFlags(&exflags);
	const wxChar *registredName = wxApp::GetRegisteredClassName(name.c_str(), COLOR_BTNFACE);

	ret = MSWCreate(registredName, title.c_str(), wxDefaultPosition, wxDefaultSize, flags, exflags);

	if (ret)
	{
		MSWUpdateUIState(UIS_INITIALIZE);
	}


	MARGINS borderless = { 0, 0, 0, 0 };
	DwmExtendFrameIntoClientArea(m_hWnd, &borderless);
	SetFont(wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT));

	SetForegroundColour(Options.GetColour(WindowText));
	SetBackgroundColour(Options.GetColour(WindowBackground));

	Bind(wxEVT_PAINT, &KaiFrame::OnPaint, this);
	Bind(wxEVT_LEFT_DOWN, &KaiFrame::OnMouseEvent, this);
	Bind(wxEVT_LEFT_UP, &KaiFrame::OnMouseEvent, this);
	Bind(wxEVT_LEFT_DCLICK, &KaiFrame::OnMouseEvent, this);
	Bind(wxEVT_LEAVE_WINDOW, &KaiFrame::OnMouseEvent, this);
	Bind(wxEVT_MOTION, &KaiFrame::OnMouseEvent, this);
	SetSize(pos.x, pos.y, size.x, size.y);
}

KaiFrame::~KaiFrame()
{
}

void KaiFrame::OnPaint(wxPaintEvent &evt)
{
	int w, h;
	GetSize(&w, &h);
	if (w < 1 || h < 1){ return; }
	
	wxMemoryDC mdc;
	mdc.SelectObject(wxBitmap(w, h));

	GraphicsRenderer *renderer = GraphicsRenderer::GetDirect2DRenderer();
	GraphicsContext *gc = renderer->CreateContext(mdc);
	if (!gc){
		mdc.SetFont(GetFont());
		wxColour bg = (isActive) ? Options.GetColour(WindowBorderBackground) : Options.GetColour(WindowBorderBackgroundInactive);
		mdc.SetBrush(bg);
		mdc.SetPen((isActive) ? Options.GetColour(WindowBorder) : Options.GetColour(WindowBorderInactive));
		mdc.DrawRectangle(0, 0, w, h);
		wxColour text = (isActive) ? Options.GetColour(WindowHeaderText) : Options.GetColour(WindowHeaderTextInactive);
		mdc.SetTextForeground(text);
		int maximizeDiff = (IsMaximized()) ? 3 : 0;
		wxIconBundle icons = GetIcons();
		int iconScale = ((frameTopBorder - 10) / 2) * 2;
		iconScale = (iconScale < 16) ? 16 : iconScale;
		if (icons.GetIconCount()){
			//if(icons.GetIconByIndex(0).GetHeight()!=16){
			wxImage img = wxBitmap(icons.GetIconByIndex(0)).ConvertToImage();
			img = img.Scale(iconScale, iconScale, wxIMAGE_QUALITY_BICUBIC);
			mdc.DrawBitmap(wxBitmap(img), 8 + maximizeDiff, ((frameTopBorder - iconScale) / 2) + maximizeDiff + 1);
			//}else{
			//mdc.DrawIcon(icons.GetIconByIndex(0), 4, 4);
			//}
		}
		if (GetTitle() != L""){
			int startX = icons.GetIconCount() ? iconScale + 14 : 6 + maximizeDiff;
			int maxWidth = w - 75 - maximizeDiff - startX;
			mdc.DrawText(GetTruncateText(GetTitle(), maxWidth, this), startX, 5 + maximizeDiff);
		}

		int buttonScale = ((frameTopBorder - 8) / 2) * 2;
		buttonScale = (buttonScale < 18) ? 18 : buttonScale;

		if (enterClose || pushedClose){
			wxColour buttonxbg = (enterClose && !pushedClose) ? Options.GetColour(WindowHoverCloseButton) :
				Options.GetColour(WindowPushedCloseButton);
			mdc.SetBrush(buttonxbg);
			mdc.SetPen(buttonxbg);
			mdc.DrawRectangle(w - frameTopBorder - maximizeDiff, 5 + maximizeDiff, buttonScale, buttonScale);
		}
		else if (enterMaximize || pushedMaximize){
			wxColour buttonxbg = (enterMaximize && !pushedMaximize) ? Options.GetColour(WindowHoverHeaderElement) :
				Options.GetColour(WindowPushedHeaderElement);
			mdc.SetBrush(buttonxbg);
			mdc.SetPen(buttonxbg);
			mdc.DrawRectangle(w - (frameTopBorder * 2) - maximizeDiff, 5 + maximizeDiff, buttonScale, buttonScale);
		}
		else if (enterMinimize || pushedMinimize){
			wxColour buttonxbg = (enterMinimize && !pushedMinimize) ? Options.GetColour(WindowHoverHeaderElement) :
				Options.GetColour(WindowPushedHeaderElement);
			mdc.SetBrush(buttonxbg);
			mdc.SetPen(buttonxbg);
			mdc.DrawRectangle(w - (frameTopBorder * 3) - maximizeDiff, 5 + maximizeDiff, buttonScale, buttonScale);
		}
		mdc.SetPen(wxPen(text, 2));
		mdc.SetBrush(wxBrush(text));
		//draw X
		mdc.DrawLine(w - frameTopBorder + 3 - maximizeDiff, 8 + maximizeDiff, w - (frameBorder + 6) - maximizeDiff, frameTopBorder - 8 + maximizeDiff);
		mdc.DrawLine(w - (frameBorder + 6) - maximizeDiff, 8 + maximizeDiff, w - frameTopBorder + 3 - maximizeDiff, frameTopBorder - 8 + maximizeDiff);
		//draw maximize

		if (IsMaximized()){
			int realButtonScale = (buttonScale - 6);
			int processScale = (realButtonScale / 3);
			int processScalex2 = processScale * 2;
			mdc.SetPen(text);
			mdc.SetBrush(*wxTRANSPARENT_BRUSH);
			mdc.DrawRectangle(w - (frameTopBorder * 2) + 3 + realButtonScale - processScalex2 - 1 - maximizeDiff, 8 + maximizeDiff, processScalex2 + 1, processScalex2 - 1);
			mdc.DrawRectangle(w - (frameTopBorder * 2) + 3 - maximizeDiff, 8 + realButtonScale - processScalex2 + 0 + maximizeDiff, processScalex2 + 1, processScalex2 - 1);
			mdc.SetPen(*wxTRANSPARENT_PEN);
		}
		else{
			mdc.SetPen(*wxTRANSPARENT_PEN);
			mdc.DrawRectangle(w - (frameTopBorder * 2) + 3 - maximizeDiff, 7 + maximizeDiff, 1, buttonScale - 6);
			mdc.DrawRectangle(w - (frameTopBorder * 2) + 3 - maximizeDiff, 7 + maximizeDiff, buttonScale - 6, 2);
			mdc.DrawRectangle(w - (frameTopBorder * 2) + 3 - maximizeDiff, 7 + maximizeDiff + buttonScale - 6 + maximizeDiff, buttonScale - 6, 1);
			mdc.DrawRectangle(w - (frameTopBorder * 2) + (buttonScale - 4) - maximizeDiff, 7 + maximizeDiff, 1, buttonScale - 6);
		}
		//draw minimize
		mdc.SetBrush(wxBrush(text));
		mdc.DrawRectangle(w - (frameTopBorder * 3) + 3 - maximizeDiff, buttonScale - 1 + maximizeDiff, buttonScale - 6, 2);
	}
	else{
		PaintD2D(gc, w, h);
		delete gc;
	}

	wxPaintDC dc(this);
	dc.Blit(0, 0, w, frameTopBorder, &mdc, 0, 0);
	dc.Blit(0, frameTopBorder, frameBorder, h - frameTopBorder - frameBorder, &mdc, 0, frameTopBorder);
	dc.Blit(w - frameBorder, frameTopBorder, frameBorder, h - frameTopBorder - frameBorder, &mdc, w - frameBorder, frameTopBorder);
	dc.Blit(0, h - frameBorder, w, frameBorder, &mdc, 0, h - frameBorder);
}

void KaiFrame::PaintD2D(GraphicsContext *gc, int w, int h)
{
	wxColour text = (isActive) ? Options.GetColour(WindowHeaderText) : Options.GetColour(WindowHeaderTextInactive);
	gc->SetFont(GetFont(), text);
	wxColour bg = (isActive) ? Options.GetColour(WindowBorderBackground) : Options.GetColour(WindowBorderBackgroundInactive);
	gc->SetBrush(bg);
	gc->SetPen((isActive) ? Options.GetColour(WindowBorder) : Options.GetColour(WindowBorderInactive));
	gc->DrawRectangle(0, 0, w - 1, h - 1);
	
	int maximizeDiff = (IsMaximized()) ? 3 : 0;
	wxIconBundle icons = GetIcons();
	int iconScale = ((frameTopBorder - 10) / 2) * 2;
	iconScale = (iconScale < 16) ? 16 : iconScale;
	if (icons.GetIconCount()){
		//if(icons.GetIconByIndex(0).GetHeight()!=16){
		wxImage img = wxBitmap(icons.GetIconByIndex(0)).ConvertToImage();
		img = img.Scale(iconScale, iconScale, wxIMAGE_QUALITY_BICUBIC);
		gc->DrawBitmap(wxBitmap(img), 8 + maximizeDiff, ((frameTopBorder - iconScale) / 2) + maximizeDiff + 1, img.GetWidth(), img.GetHeight());
		//}else{
		//gc->DrawIcon(icons.GetIconByIndex(0), 4, 4);
		//}
	}
	if (GetTitle() != L""){
		int startX = icons.GetIconCount() ? iconScale + 14 : 6 + maximizeDiff;
		int maxWidth = w - 75 - maximizeDiff - startX;
		gc->DrawTextU(GetTruncateText(GetTitle(), maxWidth, this), startX, 5 + maximizeDiff);
	}

	int buttonScale = ((frameTopBorder - 8) / 2) * 2;
	buttonScale = (buttonScale < 18) ? 18 : buttonScale;

	if (enterClose || pushedClose){
		wxColour buttonxbg = (enterClose && !pushedClose) ? Options.GetColour(WindowHoverCloseButton) :
			Options.GetColour(WindowPushedCloseButton);
		gc->SetBrush(buttonxbg);
		gc->SetPen(buttonxbg);
		gc->DrawRectangle(w - frameTopBorder - maximizeDiff, 5 + maximizeDiff, buttonScale, buttonScale);
	}
	else if (enterMaximize || pushedMaximize){
		wxColour buttonxbg = (enterMaximize && !pushedMaximize) ? Options.GetColour(WindowHoverHeaderElement) :
			Options.GetColour(WindowPushedHeaderElement);
		gc->SetBrush(buttonxbg);
		gc->SetPen(buttonxbg);
		gc->DrawRectangle(w - (frameTopBorder * 2) - maximizeDiff, 5 + maximizeDiff, buttonScale, buttonScale);
	}
	else if (enterMinimize || pushedMinimize){
		wxColour buttonxbg = (enterMinimize && !pushedMinimize) ? Options.GetColour(WindowHoverHeaderElement) :
			Options.GetColour(WindowPushedHeaderElement);
		gc->SetBrush(buttonxbg);
		gc->SetPen(buttonxbg);
		gc->DrawRectangle(w - (frameTopBorder * 3) - maximizeDiff, 5 + maximizeDiff, buttonScale, buttonScale);
	}
	gc->SetPen(wxPen(text, 2));
	gc->SetBrush(wxBrush(text));
	//draw X
	GraphicsPathData *path = gc->CreatePath();
	path->MoveToPoint(w - frameTopBorder + 3 - maximizeDiff, 8 + maximizeDiff);
	path->AddLineToPoint(w - (frameBorder + 6) - maximizeDiff, frameTopBorder - 8 + maximizeDiff);
	path->MoveToPoint(w - (frameBorder + 6) - maximizeDiff, 8 + maximizeDiff);
	path->AddLineToPoint(w - frameTopBorder + 3 - maximizeDiff, frameTopBorder - 8 + maximizeDiff);
	gc->StrokePath(path);

	//draw maximize

	if (IsMaximized()){
		int realButtonScale = (buttonScale - 6);
		int processScale = (realButtonScale / 3);
		int processScalex2 = processScale * 2;
		gc->SetPen(text);
		gc->SetBrush(*wxTRANSPARENT_BRUSH);
		gc->DrawRectangle(w - (frameTopBorder * 2) + 3 + realButtonScale - processScalex2 - 1 - maximizeDiff, 8 + maximizeDiff, processScalex2 + 1, processScalex2 - 1);
		gc->DrawRectangle(w - (frameTopBorder * 2) + 3 - maximizeDiff, 8 + realButtonScale - processScalex2 + 0 + maximizeDiff, processScalex2 + 1, processScalex2 - 1);
		gc->SetPen(*wxTRANSPARENT_PEN);
	}
	else{
		gc->SetPen(*wxTRANSPARENT_PEN);
		gc->DrawRectangle(w - (frameTopBorder * 2) + 3 - maximizeDiff, 7 + maximizeDiff, 1, buttonScale - 6);
		gc->DrawRectangle(w - (frameTopBorder * 2) + 3 - maximizeDiff, 7 + maximizeDiff, buttonScale - 6, 2);
		gc->DrawRectangle(w - (frameTopBorder * 2) + 3 - maximizeDiff, 7 + maximizeDiff + buttonScale - 6 + maximizeDiff, buttonScale - 6, 1);
		gc->DrawRectangle(w - (frameTopBorder * 2) + (buttonScale - 4) - maximizeDiff, 7 + maximizeDiff, 1, buttonScale - 6);
	}
	//draw minimize
	gc->SetBrush(wxBrush(text));
	gc->DrawRectangle(w - (frameTopBorder * 3) + 3 - maximizeDiff, buttonScale - 1 + maximizeDiff, buttonScale - 6, 2);
}

void KaiFrame::SetLabel(const wxString &text)
{
	wxTopLevelWindow::SetLabel(text);
	int w, h;
	GetSize(&w, &h);
	wxRect rc(0, 0, w, frameTopBorder);
	Refresh(false, &rc);
}

//void KaiFrame::OnSize(wxSizeEvent &evt)
//{
//	//Refresh(false);
//	int w, h;
//	GetSize(&w, &h);
//	wxRect rc(0, 0, w, ftopBorder);
//	Refresh(false, &rc);
//	if (!IsMaximized()){
//		wxRect rc1(0, ftopBorder, fborder, h - fborder - ftopBorder);
//		Refresh(false, &rc1);
//		wxRect rc2(w - fborder, ftopBorder, fborder, h - fborder - ftopBorder);
//		Refresh(false, &rc2);
//		wxRect rc3(0, h - fborder, w, fborder);
//		Refresh(false, &rc3);
//	}
//	//Update();
//	evt.Skip();
//}

void KaiFrame::OnMouseEvent(wxMouseEvent &evt)
{
	int w, h;
	GetSize(&w, &h);
	int x = evt.GetX();
	int y = evt.GetY();
	int maximizeDiff = (IsMaximized()) ? 3 : 0;
	wxRect rc(w - (frameTopBorder * 3.5), 0, (frameTopBorder * 3.5), frameTopBorder);
	if (evt.Leaving()){
		pushedClose = enterClose = pushedMinimize = enterMinimize = pushedMaximize = enterMaximize = false;
		Refresh(false, &rc);
		return;
	}
	bool leftdown = evt.LeftDown() || evt.LeftDClick();
	if (leftdown){
		wxActivateEvent evt(wxEVT_ACTIVATE, true);
		OnActivate(evt);
	}
	if (x >= w - frameTopBorder - maximizeDiff && x < w - frameBorder - 1 - maximizeDiff && 
		y >= 5 + maximizeDiff && y < frameTopBorder - 3 + maximizeDiff){
		pushedMinimize = enterMinimize = pushedMaximize = enterMaximize = false;
		if (leftdown){ pushedClose = true; Refresh(false, &rc); }
		if (!enterClose){ enterClose = true; Refresh(false, &rc); }
		if (evt.LeftUp()){
			pushedClose = enterClose = false;
			Refresh(false, &rc);
			Close();
		}
		return;
	}
	else if (x >= w - (frameTopBorder * 2) - maximizeDiff && x < w - frameTopBorder - 8 - maximizeDiff && 
		y >= 5 + maximizeDiff && y < frameTopBorder - 3 + maximizeDiff){
		pushedClose = enterClose = pushedMinimize = enterMinimize = false;
		if (leftdown){ pushedMaximize = true; Refresh(false, &rc); }
		if (!enterMaximize){ enterMaximize = true; Refresh(false, &rc); }
		if (evt.LeftUp()){
			pushedMaximize = enterMaximize = false;
			Refresh(false, &rc);
			Maximize(!IsMaximized());
		}
		return;
	}
	else if (x >= w - (frameTopBorder * 3) - maximizeDiff && x < w - (frameTopBorder * 2) - 8 - maximizeDiff && 
		y >= 5 + maximizeDiff && y < frameTopBorder - 3 + maximizeDiff){
		pushedClose = enterClose = pushedMaximize = enterMaximize = false;
		if (leftdown){ pushedMinimize = true; Refresh(false, &rc); }
		if (!enterMinimize){ enterMinimize = true; Refresh(false, &rc); }
		if (evt.LeftUp()){
			pushedMinimize = enterMinimize = false;
			Refresh(false, &rc);
			ShowWindow(GetHWND(), SW_SHOWMINNOACTIVE);
		}
		return;
	}
	else if (enterClose || pushedClose || enterMaximize || pushedMaximize || enterMinimize || pushedMinimize){
		pushedClose = enterClose = pushedMinimize = enterMinimize = pushedMaximize = enterMaximize = false;
		Refresh(false, &rc);
	}
	evt.Skip();
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
		wxRect rc(0, 0, w, frameTopBorder);
		Refresh(false, &rc);
		if (!IsMaximized()){
			wxRect rc1(0, frameTopBorder, frameBorder, h - frameBorder - frameTopBorder);
			Refresh(false, &rc1);
			wxRect rc2(w - frameBorder, frameTopBorder, frameBorder, h - frameBorder - frameTopBorder);
			Refresh(false, &rc2);
			wxRect rc3(0, h - frameBorder, w, frameBorder);
			Refresh(false, &rc3);
		}
		//Cannot use update here cause window blinking even when video is paused
		//and there is some trash on left top border
		//Update();
	}
	if (uMsg == WM_ERASEBKGND){
		return 0;
	}

	if (uMsg == WM_NCACTIVATE){
		//when restoring from minimize it gives wParam as 2097153, that's need to be set > 0, not == TRUE
		isActive = wParam > 0;
		int w, h;
		GetSize(&w, &h);
		wxRect rc(0, 0, w, frameTopBorder);
		Refresh(false, &rc);
		if (!IsMaximized()){
			wxRect rc1(0, frameTopBorder, frameBorder, h - frameBorder - frameTopBorder);
			Refresh(false, &rc1);
			wxRect rc2(w - frameBorder, frameTopBorder, frameBorder, h - frameBorder - frameTopBorder);
			Refresh(false, &rc2);
			wxRect rc3(0, h - frameBorder, w, frameBorder);
			Refresh(false, &rc3);
		}
		return 1;
	}
	if (uMsg == WM_CLOSE){
		Close();
		return 0;
	}
	if (uMsg == WM_GETMINMAXINFO){
		MINMAXINFO * pInfo = (MINMAXINFO*)lParam;
		pInfo->ptMinTrackSize.x = 600;
		pInfo->ptMinTrackSize.y = 400;
		return 0;
	}
	if ((uMsg == WM_NCCALCSIZE)){

		return 0;
	}
	if (uMsg == WM_NCHITTEST){
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
		if (x >= frameBorder && x <= WindowRect.right - WindowRect.left - (frameTopBorder * 3.5) && y >= frameBorder && y <= frameTopBorder){
			result = HTCAPTION;
		}
		else if (style & wxRESIZE_BORDER && !IsMaximized()){

			if (x < frameBorder && y < frameBorder)
				result = HTTOPLEFT;
			else if (x > WindowRect.right - WindowRect.left - frameBorder && y < frameBorder)
				result = HTTOPRIGHT;
			else if (x > WindowRect.right - WindowRect.left - frameBorder && y > WindowRect.bottom - WindowRect.top - frameBorder)
				result = HTBOTTOMRIGHT;
			else if (x < frameBorder && y > WindowRect.bottom - WindowRect.top - frameBorder)
				result = HTBOTTOMLEFT;
			else if (x < frameBorder)
				result = HTLEFT;
			else if (y < frameBorder)
				result = HTTOP;
			else if (x > WindowRect.right - WindowRect.left - frameBorder)
				result = HTRIGHT;
			else if (y > WindowRect.bottom - WindowRect.top - frameBorder)
				result = HTBOTTOM;
			else
				result = HTCLIENT;
		}
		else{
			return HTCLIENT;
		}
		if (result != HTCLIENT && (enterClose || pushedClose || enterMaximize || pushedMaximize || enterMinimize || pushedMinimize)){
			pushedClose = enterClose = pushedMinimize = enterMinimize = pushedMaximize = enterMaximize = false;
			wxRect rc(0, 0, WindowRect.right - WindowRect.left, frameTopBorder);
			Refresh(false, &rc);
			//Update();
		}
		return result;
	}
	if (uMsg == WM_COPYDATA){
		PCOPYDATASTRUCT pMyCDS = (PCOPYDATASTRUCT)lParam;
		wxArrayString paths;
		wchar_t * _paths = (wchar_t*)pMyCDS->lpData;
		wxStringTokenizer tkn(_paths, L"|");
		while (tkn.HasMoreTokens()){
			paths.Add(tkn.NextToken());
		}
		kainoteApp *Kai = (kainoteApp *)wxTheApp;
		if (Kai){
			Kai->paths.insert(Kai->paths.end(), paths.begin(), paths.end());
			Kai->timer.Start(400, true);
		}
		return true;
	}

	return wxTopLevelWindow::MSWWindowProc(uMsg, wParam, lParam);
}

void KaiFrame::GetClientSize(int *w, int *h) const
{
	wxTopLevelWindow::GetClientSize(w, h);
	*w -= (frameBorder * 2);
	*h -= (frameTopBorder + frameBorder);
}

wxSize KaiFrame::GetClientSize() const
{
	wxSize size = wxTopLevelWindow::GetClientSize();
	size.x -= (frameBorder * 2);
	size.y -= (frameTopBorder + frameBorder);
	return size;
}
void KaiFrame::SetClientSize(const wxSize &size)
{
	SetClientSize(size.x, size.y);
}
void KaiFrame::SetClientSize(int x, int y)
{
	x += (frameBorder * 2);
	y += (frameTopBorder + frameBorder);
	wxWindow::SetClientSize(x, y);
}


bool KaiFrame::SetFont(const wxFont &font)
{
	int fw, fh;
	GetTextExtent(GetTitle(), &fw, &fh, 0, 0, &font);
	frameTopBorder = (fh + 10 < 26) ? 26 : fh + 10;
	return wxWindow::SetFont(font);
}

wxIMPLEMENT_ABSTRACT_CLASS(KaiFrame, wxTopLevelWindow);
