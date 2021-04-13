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


#include "KaiFrame.h"
#include <wx/dc.h>
#include <wx/dcclient.h>
#include <wx/dcmemory.h>
#include "config.h"
#include "kainoteApp.h"
#include "wx/msw/private.h"
#include <Dwmapi.h>
#include "GraphicsD2D.h"
//#include <shellscalingapi.h>
#pragma comment(lib, "Dwmapi.lib")
//#define GET_X_LPARAM(lp)                        ((int)(short)LOWORD(lp))
//#define GET_Y_LPARAM(lp)                        ((int)(short)HIWORD(lp))


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
	wxWindow::SetFont(wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT));

	SetForegroundColour(Options.GetColour(WINDOW_TEXT));
	SetBackgroundColour(Options.GetColour(WINDOW_BACKGROUND));

	Bind(wxEVT_PAINT, &KaiFrame::OnPaint, this);
	Bind(wxEVT_LEFT_DOWN, &KaiFrame::OnMouseEvent, this);
	Bind(wxEVT_LEFT_UP, &KaiFrame::OnMouseEvent, this);
	Bind(wxEVT_LEFT_DCLICK, &KaiFrame::OnMouseEvent, this);
	Bind(wxEVT_LEAVE_WINDOW, &KaiFrame::OnMouseEvent, this);
	Bind(wxEVT_MOTION, &KaiFrame::OnMouseEvent, this);
	LastMonitorRect = GetMonitorRect1(-1, NULL, wxRect(0, 0, 0, 0));
	SetSize(pos.x, pos.y, size.x, size.y);
	Options.SetCoords(WINDOW_SIZE, size.x, size.y);
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

	//GraphicsRenderer *renderer = GraphicsRenderer::GetDirect2DRenderer();
	GraphicsContext* gc = NULL;//renderer->CreateContext(mdc);
	if (!gc){
		mdc.SetFont(GetFont());
		wxColour bg = (isActive) ? Options.GetColour(WINDOW_BORDER_BACKGROUND) : Options.GetColour(WINDOW_BORDER_BACKGROUND_INACTIVE);
		mdc.SetBrush(bg);
		mdc.SetPen((isActive) ? Options.GetColour(WINDOW_BORDER) : Options.GetColour(WINDOW_BORDER_INACTIVE));
		mdc.DrawRectangle(0, 0, w, h);
		wxColour text = (isActive) ? Options.GetColour(WINDOW_HEADER_TEXT) : Options.GetColour(WINDOW_HEADER_TEXT_INACTIVE);
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
			wxColour buttonxbg = (enterClose && !pushedClose) ? Options.GetColour(WINDOW_HOVER_CLOSE_BUTTON) :
				Options.GetColour(WINDOW_PUSHED_CLOSE_BUTTON);
			mdc.SetBrush(buttonxbg);
			mdc.SetPen(buttonxbg);
			mdc.DrawRectangle(w - frameTopBorder - maximizeDiff, 5 + maximizeDiff, buttonScale, buttonScale);
		}
		else if (enterMaximize || pushedMaximize){
			wxColour buttonxbg = (enterMaximize && !pushedMaximize) ? Options.GetColour(WINDOW_HOVER_HEADER_ELEMENT) :
				Options.GetColour(WINDOW_PUSHED_HEADER_ELEMENT);
			mdc.SetBrush(buttonxbg);
			mdc.SetPen(buttonxbg);
			mdc.DrawRectangle(w - (frameTopBorder * 2) - maximizeDiff, 5 + maximizeDiff, buttonScale, buttonScale);
		}
		else if (enterMinimize || pushedMinimize){
			wxColour buttonxbg = (enterMinimize && !pushedMinimize) ? Options.GetColour(WINDOW_HOVER_HEADER_ELEMENT) :
				Options.GetColour(WINDOW_PUSHED_HEADER_ELEMENT);
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
	wxColour text = (isActive) ? Options.GetColour(WINDOW_HEADER_TEXT) : Options.GetColour(WINDOW_HEADER_TEXT_INACTIVE);
	gc->SetFont(GetFont(), text);
	wxColour bg = (isActive) ? Options.GetColour(WINDOW_BORDER_BACKGROUND) : Options.GetColour(WINDOW_BORDER_BACKGROUND_INACTIVE);
	gc->SetBrush(bg);
	gc->SetPen((isActive) ? Options.GetColour(WINDOW_BORDER) : Options.GetColour(WINDOW_BORDER_INACTIVE));
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
		wxColour buttonxbg = (enterClose && !pushedClose) ? Options.GetColour(WINDOW_HOVER_CLOSE_BUTTON) :
			Options.GetColour(WINDOW_PUSHED_CLOSE_BUTTON);
		gc->SetBrush(buttonxbg);
		gc->SetPen(buttonxbg);
		gc->DrawRectangle(w - frameTopBorder - maximizeDiff, 5 + maximizeDiff, buttonScale - 1, buttonScale - 1);
	}
	else if (enterMaximize || pushedMaximize){
		wxColour buttonxbg = (enterMaximize && !pushedMaximize) ? Options.GetColour(WINDOW_HOVER_HEADER_ELEMENT) :
			Options.GetColour(WINDOW_PUSHED_HEADER_ELEMENT);
		gc->SetBrush(buttonxbg);
		gc->SetPen(buttonxbg);
		gc->DrawRectangle(w - (frameTopBorder * 2) - maximizeDiff, 5 + maximizeDiff, buttonScale - 1, buttonScale - 1);
	}
	else if (enterMinimize || pushedMinimize){
		wxColour buttonxbg = (enterMinimize && !pushedMinimize) ? Options.GetColour(WINDOW_HOVER_HEADER_ELEMENT) :
			Options.GetColour(WINDOW_PUSHED_HEADER_ELEMENT);
		gc->SetBrush(buttonxbg);
		gc->SetPen(buttonxbg);
		gc->DrawRectangle(w - (frameTopBorder * 3) - maximizeDiff, 5 + maximizeDiff, buttonScale - 1, buttonScale - 1);
	}
	gc->SetPen(wxPen(text), 2.);
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
	if (uMsg == WM_COPYDATA){
		PCOPYDATASTRUCT pMyCDS = (PCOPYDATASTRUCT)lParam;
		//wxArrayString paths;
		wchar_t * _paths = (wchar_t*)pMyCDS->lpData;
		wxStringTokenizer tkn(_paths, L"|");
		kainoteApp *Kai = (kainoteApp *)wxTheApp;
		if (Kai){
			while (tkn.HasMoreTokens()){
				Kai->paths.Add(tkn.NextToken());
			}
			Kai->openTimer.Start(400, true);
		}
		return true;
	}

	if (uMsg == WM_SIZE)
	{
		int w = LOWORD(lParam);
		int h = HIWORD(lParam);
		
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
		//Options.SetCoords(WINDOW_SIZE, w, h);
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
	//WM_DPICHANGED, used hex value cause off compatybility with win 7 that hasn't that
	if (uMsg == 0x02E0) {
		int ydpi = (int)(short)HIWORD(wParam);
		int currentDPI = Options.GetDPI();
		if (ydpi == currentDPI)
			return 0;

		RECT *newRect = (RECT*)lParam;
		wxRect newRt = wxRect(newRect->left, newRect->top, abs(newRect->right - newRect->left), abs(newRect->bottom - newRect->top));
		std::vector<RECT> monitors;
		wxRect rt = GetMonitorRect1(0, &monitors, newRt);
		
		float fontScale = ((float)ydpi / (float)currentDPI);
		
		Options.FontsRescale(ydpi);

		Notebook *tabs = Notebook::GetTabs();
		//this case should not happen
		//but who knows
		if (!tabs) {
			LastMonitorRect = rt;
			Options.SetCoords(MONITOR_SIZE, rt.width, rt.height);
			Options.SetCoords(MONITOR_POSITION, rt.x, rt.y);
			return 1;
		}

		wxFont *font = Options.GetFont();
		SetFont(*font);

		bool noResize = false;
		if ((newRt.x == rt.width / 2 || newRt.x == 0) && newRt.y == 0 && newRt.width == rt.width / 2) {
			noResize = true;
		}

		int sizex, sizey;
		//Windows bug when shift win arrow is used window is shrink to display rect
		//then event of DPI_CHANGED is sent
		if (fontScale <= 1.f) {
			Options.GetCoords(WINDOW_SIZE, &sizex, &sizey);
		}
		else {
			GetSize(&sizex, &sizey);
		}
		
		int vsizex, vsizey;
		Options.GetCoords(VIDEO_WINDOW_SIZE, &vsizex, &vsizey);
		int audioHeight = Options.GetInt(AUDIO_BOX_HEIGHT);
		float scalex = (float)rt.width / (float)LastMonitorRect.width;
		float scaley = (float)rt.height / (float)LastMonitorRect.height;
		if (!wasWindowsSize) {
			sizex *= scalex;
			sizey *= scaley;
		}
		vsizex *= scalex;
		vsizey *= scaley;
		Options.SetCoords(MONITOR_SIZE, rt.width, rt.height);
		Options.SetCoords(MONITOR_POSITION, rt.x, rt.y);
		Options.SetInt(AUDIO_BOX_HEIGHT, audioHeight);

		

		for (size_t i = 0; i < tabs->Size(); i++) {
			TabPanel *tab = tabs->Page(i);
			wxSize minVideoSize = tab->Video->GetMinSize();
			int panelHeight = tab->Video->GetPanelHeight();
			minVideoSize.y -= panelHeight;
			minVideoSize.x *= scalex;
			minVideoSize.y *= scaley;
			minVideoSize.y += panelHeight;
			tab->Video->SetMinSize(minVideoSize);
			tab->Edit->SetMinSize(wxSize(-1, minVideoSize.y));
			if (tab->Edit->ABox) {
				wxSize asize = tab->Edit->ABox->GetMinSize();
				asize.y *= scaley;
				tab->Edit->ABox->SetMinSize(asize);
				tab->Edit->BoxSizer1->Layout();
			}
			tab->Grid->SetStyle();
			tab->Grid->RefreshColumns();
			if (!tab->Video->IsShown()) {
				tab->MainSizer->Layout();
			}
		}
		wxRect primaryMonitorRect;
		wxRect secondMonitorRect; 
		if (monitors.size() && monitors[0].left == rt.x && monitors[0].top == rt.y) { 
			primaryMonitorRect = rt;
			secondMonitorRect = LastMonitorRect;
		} 
		else {
			primaryMonitorRect = LastMonitorRect;
			secondMonitorRect = rt;
		}
		if (noResize || IsMaximized()) {
			//Options.SetCoords(WINDOW_SIZE, sizex, sizey);
			Layout();
			wasWindowsSize = noResize;
		}
		else if (rt.Contains(newRt)) {
			int posx = rt.x + ((float)(rt.width - sizex) / 2.f),
			posy = rt.y + ((float)(rt.height - sizey) / 2.f);
			Options.SetCoords(WINDOW_SIZE, sizex, sizey);
			if (wasWindowsSize) {
				SetPosition(wxPoint(posx, posy));
				Layout();
				wasWindowsSize = false;
			}else
				SetSize(posx, posy, sizex, sizey);
		}
		else if (secondMonitorRect.x < primaryMonitorRect.x && primaryMonitorRect.y + primaryMonitorRect.height > secondMonitorRect.y) {
				int posx, posy, osizex, osizey;
				GetPosition(&posx, &posy);
				Options.GetCoords(WINDOW_SIZE, &osizex, &osizey);
				posx -= (sizex - osizex);
				if (posx == -1)
					posx = 0;
				if (posy == -1)
					posy = 0;
				Options.SetCoords(WINDOW_SIZE, sizex, sizey);
				if (wasWindowsSize) {
					SetPosition(wxPoint(posx, posy));
					Layout();
					wasWindowsSize = false;
				}else
					SetSize(posx, posy, sizex, sizey);
		}
		else{
			Options.SetCoords(WINDOW_SIZE, sizex, sizey);
			if (wasWindowsSize) {
				Layout();
				wasWindowsSize = false;
			}else
				SetSize(sizex, sizey);
		}
		
		Options.SetCoords(VIDEO_WINDOW_SIZE, vsizex, vsizey);
		LastMonitorRect = rt;
	}
	
	return wxTopLevelWindow::MSWWindowProc(uMsg, wParam, lParam);
}

//void KaiFrame::GetClientSize(int *w, int *h) const
//{
//	wxTopLevelWindow::GetClientSize(w, h);
//	*w -= (frameBorder * 2);
//	*h -= (frameTopBorder + frameBorder);
//}
//
//wxSize KaiFrame::GetClientSize() const
//{
//	wxSize size = wxTopLevelWindow::GetClientSize();
//	size.x -= (frameBorder * 2);
//	size.y -= (frameTopBorder + frameBorder);
//	return size;
//}
//void KaiFrame::SetClientSize(const wxSize &size)
//{
//	SetClientSize(size.x, size.y);
//}
//void KaiFrame::SetClientSize(int x, int y)
//{
//	x += (frameBorder * 2);
//	y += (frameTopBorder + frameBorder);
//	wxWindow::SetClientSize(x, y);
//}


bool KaiFrame::SetFont(const wxFont &font)
{
	int fw, fh;
	GetTextExtent(GetTitle(), &fw, &fh, 0, 0, &font);
	frameTopBorder = (fh + 10 < 26) ? 26 : fh + 10;
	wxWindow::SetFont(font);
	KaiScrollbar::SetThickness(this);

	const wxWindowList& siblings = GetChildren();
	wxWindowList::compatibility_iterator nodeThis = siblings.GetFirst();
	if (nodeThis){
		for (wxWindowList::compatibility_iterator nodeAfter = nodeThis;
			nodeAfter;
			nodeAfter = nodeAfter->GetNext()){

			wxWindow *win = nodeAfter->GetData();
			win->SetFont(font);
		}
		Refresh(false);
	}
	return true;
}



wxIMPLEMENT_ABSTRACT_CLASS(KaiFrame, wxTopLevelWindow);
