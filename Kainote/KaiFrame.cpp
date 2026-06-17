//  Copyright (c) 2016 - 2026, Marcin Drob

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


#include "kainoteApp.h"
#include "KaiFrame.h"
#include "config.h"
#include "TabPanel.h"
#include "SubsGrid.h"
#include "EditBox.h"
#include "VideoBox.h"
#include "Notebook.h"
#include "AudioBox.h"
#include "LogHandler.h"
#include <wx/dc.h>
#include <wx/dcclient.h>
#include <wx/dcmemory.h>
#include "UtilsWindows.h"
#ifdef __WXMSW__
#include "wx/msw/private.h"
#include <Dwmapi.h>
#endif

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
#ifdef __WXMSW__
	wxTopLevelWindows.Append(this);

	bool ret = CreateBase(parent, id, pos, sizeReal, style, name);
	if (!ret)
		return;

	if (parent)
		parent->AddChild(this);

	WXDWORD exflags;
	WXDWORD flags = MSWGetCreateWindowFlags(&exflags);
	const wxChar *registredName = wxApp::GetRegisteredClassName(name.c_str(), COLOR_BTNFACE);

	ret = MSWCreate(registredName, title.c_str(), pos, sizeReal, flags, exflags);

	if (ret)
	{
		MSWUpdateUIState(UIS_INITIALIZE);
	}


	MARGINS borderless = { 0, 0, 0, 0 };
	DwmExtendFrameIntoClientArea(m_hWnd, &borderless);
#else
	bool ret = wxTopLevelWindow::Create(parent, id, title, pos, sizeReal, style, name);
	if (!ret)
		return;
#endif
	wxWindow::SetFont(*Options.GetFont());

	SetForegroundColour(Options.GetColour(WINDOW_TEXT));
	SetBackgroundColour(Options.GetColour(WINDOW_BACKGROUND));

	Bind(wxEVT_PAINT, &KaiFrame::OnPaint, this);
	Bind(wxEVT_LEFT_DOWN, &KaiFrame::OnMouseEvent, this);
	Bind(wxEVT_LEFT_UP, &KaiFrame::OnMouseEvent, this);
	Bind(wxEVT_LEFT_DCLICK, &KaiFrame::OnMouseEvent, this);
	Bind(wxEVT_LEAVE_WINDOW, &KaiFrame::OnMouseEvent, this);
	Bind(wxEVT_MOTION, &KaiFrame::OnMouseEvent, this);
	//LastMonitorRect = GetMonitorRect1(-1, nullptr, wxRect(0, 0, 0, 0));
	//SetSize(pos.x, pos.y, size.x, size.y);
	//Options.SetCoords(WINDOW_SIZE, size.x, size.y);
}

KaiFrame::~KaiFrame()
{
}

void KaiFrame::OnPaint(wxPaintEvent &evt)
{
	int w, h;
	GetSize(&w, &h);
	if (w < 1 || h < 1){ return; }
	const int titleHeight = wxMin(frameTopBorder, h);
	const int sideHeight = h - titleHeight - frameBorder;
	const int rightX = w - frameBorder;
	const int bottomY = h - frameBorder;
	
	wxMemoryDC mdc;
	wxBitmap KaiFrameBitmap(w, h);
	mdc.SelectObject(KaiFrameBitmap);

	//GraphicsRenderer *renderer = GraphicsRenderer::GetDirect2DRenderer();
	GraphicsContext* gc = nullptr;//renderer->CreateContext(mdc);
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
		if (GetTitle() != emptyString){
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
	//else{
		//PaintD2D(gc, w, h);
		//delete gc;
	//}

	wxPaintDC dc(this);
	if (titleHeight > 0){ dc.Blit(0, 0, w, titleHeight, &mdc, 0, 0); }
	if (frameBorder > 0 && sideHeight > 0){ dc.Blit(0, titleHeight, frameBorder, sideHeight, &mdc, 0, titleHeight); }
	if (frameBorder > 0 && rightX >= 0 && sideHeight > 0){ dc.Blit(rightX, titleHeight, frameBorder, sideHeight, &mdc, rightX, titleHeight); }
	if (frameBorder > 0 && bottomY >= 0){ dc.Blit(0, bottomY, w, frameBorder, &mdc, 0, bottomY); }
}



void KaiFrame::SetLabel(const wxString &text)
{
	wxTopLevelWindow::SetLabel(text);
	int w, h;
	GetSize(&w, &h);
	wxRect rc(0, 0, w, frameTopBorder);
	Refresh(false, &rc);
}


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
#ifdef __WXMSW__
				ShowWindow(GetHWND(), SW_SHOWMINNOACTIVE);
#else
				Iconize(true);
#endif
			}
			return;
	}
	else if (enterClose || pushedClose || enterMaximize || pushedMaximize || enterMinimize || pushedMinimize){
		pushedClose = enterClose = pushedMinimize = enterMinimize = pushedMaximize = enterMaximize = false;
		Refresh(false, &rc);
	}
	evt.Skip();
}


#ifdef __WXMSW__
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

	if (uMsg == WM_MOVE) {
		if (!IsIconized()) {
			int x = (int)(short)LOWORD(lParam);
			int y = (int)(short)HIWORD(lParam);

			Options.GetCoords(WINDOW_POSITION, &lastPosition.x, &lastPosition.y);
			Options.SetCoords(WINDOW_POSITION, x, y);
		}
	}

	if (uMsg == WM_SIZE)
	{
		int w = (int)(short)LOWORD(lParam);
		int h = (int)(short)HIWORD(lParam);
		
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
		if (!IsIconized()) {
			Options.GetCoords(WINDOW_SIZE, &lastSize.x, &lastSize.y);
			Options.SetCoords(WINDOW_SIZE, w, h);
		}
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
		//AdjustWindowRectEx(&rcFrame, WS_OVERLAPPEDWINDOW & ~WS_CAPTION, FALSE, nullptr);
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
		//need to be changed to compare monitor size
		if (ydpi == currentDPI)
			return 0;

		wxRect oldRt;
		Options.GetCoords(MONITOR_SIZE, &oldRt.width, &oldRt.height);
		Options.GetCoords(MONITOR_POSITION, &oldRt.x, &oldRt.y);
	
		RECT *newRect = (RECT*)lParam;
		wxRect newRt = wxRect(newRect->left, newRect->top, abs(newRect->right - newRect->left), abs(newRect->bottom - newRect->top));
		std::vector<RECT> monitors;
		wxRect rt = GetMonitorRect1(0, &monitors, newRt);
		int sizex = lastSize.x, sizey = lastSize.y;
		int lposx = lastPosition.x, lposy = lastPosition.y;

		if (!IsMonitorRect(&monitors, oldRt)) {
			if (!GetMonitorWithSize(&monitors, &oldRt)) {
				KaiLogSilent(wxString::Format(L"There is no monitor with size x: %d, y: %d, w: %d, h: %d", oldRt.x, oldRt.y, oldRt.width, oldRt.height));
				//how to calculate the position, maybe not need cause program was on that monitor
				//or maybe it changed after removed
			}
		}

		/*HMONITOR mon = MonitorFromWindow(GetHWND(), MONITOR_DEFAULTTONEAREST);
		MONITORINFO info;
		info.cbSize = sizeof(MONITORINFO);
		if (GetMonitorInfoW(mon, &info)) {
			x -= info.rcMonitor.left;
			y -= info.rcMonitor.top;
		}*/

		KaiLogSilent(wxString::Format(L"old Kainote position and size x: %d, y: %d, w: %d, h: %d", 
			lastPosition.x, lastPosition.y, lastSize.x, lastSize.y));
		KaiLogSilent(wxString::Format(L"old monitor x: %d, y: %d, w: %d, h: %d", oldRt.x, oldRt.y, oldRt.width, oldRt.height));
		KaiLogSilent(wxString::Format(L"new monitor x: %d, y: %d, w: %d, h: %d", rt.x, rt.y, rt.width, rt.height));
		
		float fontScale = ((float)ydpi / (float)currentDPI);
		
		Options.FontsRescale(ydpi);
		
		Notebook *tabs = Notebook::GetTabs();
		//this case should not happen
		//but who knows
		if (!tabs) {
			KaiLogSilent(L"No tabs");
			Options.SetCoords(MONITOR_SIZE, rt.width, rt.height);
			Options.SetCoords(MONITOR_POSITION, rt.x, rt.y);
			return 1;
		}
		
		KainoteFrame::Get()->Freeze();
		
		//check if window is set on half of monitor
		bool noResize = false;
		if ((newRt.x == rt.width / 2 || newRt.x == 0) && newRt.y == 0 && newRt.width == rt.width / 2) {
			noResize = true;
		}
		
		//Windows bug when shift-win-arrow is used window is shrink to display rect
		//then event of DPI_CHANGED is sent
		float scalex = (float)rt.width / (float)oldRt.width;
		float scaley = (float)rt.height / (float)oldRt.height;

		KaiLogSilent(wxString::Format(L"scale x: %s, y: %s", getfloat(scalex).wc_str(), getfloat(scaley).wc_str()));
		
		int vsizex, vsizey;
		Options.GetCoords(VIDEO_WINDOW_SIZE, &vsizex, &vsizey);
		int audioHeight = Options.GetInt(AUDIO_BOX_HEIGHT);
		
		vsizex *= scalex;
		vsizey *= scaley;
		audioHeight *= scaley;

		if (!wasWindowsSize) {
			sizex *= scalex;
			sizey *= scaley;
			KaiLogSilent(wxString::Format(L"size x: %d, y: %d", sizex, sizey));
			//I don't know now why sometimes it set Kainote size to 200 x 60 or something like that
			//block it
			if (sizex < 1000 || sizey < 700) {
				sizex = 1000; sizey = 700;
			}
		}
		newRt.width = sizex;
		newRt.height = sizey;
		
		newRt.x = ((lposx - oldRt.x) * scalex) + rt.x;
		newRt.y = ((lposy - oldRt.y) * scaley) + rt.y;
		bool wasBadPosition = false;
		if (newRt.x + (newRt.width / 2) > rt.width + rt.x || newRt.x + (newRt.width / 2) < rt.x) {
			newRt.x = rt.x + ((float)(rt.width - sizex) / 2.f);
			wasBadPosition = true;
		}
		if (newRt.y + (newRt.height / 2) > rt.height + rt.y || newRt.y + (newRt.height / 2) < rt.y) {
			newRt.y = rt.y + ((float)(rt.height - sizey) / 2.f);
			wasBadPosition = true;
		}

		KaiLogSilent(wxString::Format(L"pos x: %d, y: %d", newRt.x, newRt.y));
		
		Options.SetCoords(MONITOR_SIZE, rt.width, rt.height);
		Options.SetCoords(MONITOR_POSITION, rt.x, rt.y);
		Options.SetInt(AUDIO_BOX_HEIGHT, audioHeight);

		//setFont to all windows
		wxFont* font = Options.GetFont();
		SetFont(*font);
		
		//rescale all tabs
		for (size_t i = 0; i < tabs->Size(); i++) {
			TabPanel *tab = tabs->Page(i);
			wxSize minVideoSize = tab->video->GetMinSize();
			//int panelHeight = tab->video->GetPanelHeight();
			//minVideoSize.y -= panelHeight;
			minVideoSize.x *= scalex;
			minVideoSize.y *= scaley;
			//minVideoSize.y += panelHeight;
			tab->video->SetMinSize(minVideoSize);
			tab->edit->SetMinSize(wxSize(-1, minVideoSize.y));
			if (tab->edit->ABox) {
				wxSize asize = tab->edit->ABox->GetMinSize();
				asize.y *= scaley;
				tab->edit->ABox->SetMinSize(asize);
				tab->edit->BoxSizer1->Layout();
			}
			tab->grid->SetStyle();
			tab->grid->RefreshColumns();
			if (!tab->video->IsShown()) {
				tab->MainSizer->Layout();
			}
		}


		bool moveToScreenOnLeft = oldRt.x == rt.x + rt.width && (lposx + (lastSize.x / 2)) <= oldRt.x;
		bool moveToScreenOnRight = oldRt.x + oldRt.width == rt.x && (lposx + (lastSize.x / 2)) >= rt.x;
		bool moveToScreenOnTop = oldRt.y == rt.y + rt.height && (lposy + (lastSize.y / 2)) <= oldRt.y;
		bool moveToScreenOnBottom = oldRt.y + oldRt.height == rt.y && (lposy + (lastSize.y / 2)) >= rt.y;
		
		if (noResize || IsMaximized()) {
			Options.SetCoords(WINDOW_SIZE, sizex, sizey);
			Layout();
			wasWindowsSize = noResize;
		}
		else if ((moveToScreenOnLeft || moveToScreenOnRight || moveToScreenOnTop || moveToScreenOnBottom) && !wasBadPosition) {
			int posx, posy;
			GetPosition(&posx, &posy);
			wxPoint posOnScreen = wxGetMousePosition();
			wxRect mouseMonitor = GetMonitorRect1(0/*-1*/, nullptr, wxRect(posOnScreen.x, posOnScreen.y, 1, 1));
			if(mouseMonitor.x > rt.x || mouseMonitor.x > oldRt.x)
				posx -= (sizex - lastSize.x);

			Options.SetCoords(WINDOW_SIZE, sizex, sizey);
			if (wasWindowsSize) {
				SetPosition(wxPoint(posx, posy));
				Layout();
				wasWindowsSize = false;
			}
			else
				SetSize(posx, posy, sizex, sizey);

			Options.SetCoords(WINDOW_POSITION, posx, posy);
			KaiLogSilent(wxString::Format(L"moving by mouse to other monitor x: %d, y: %d, w: %d, h: %d", posx, posy, sizex, sizey));
		}
		else{
			Options.SetCoords(WINDOW_SIZE, sizex, sizey);
			KaiLogSilent(wxString::Format(L"shortcut and windows moving x: %d, y: %d, w: %d, h: %d noresize %d", newRt.x, newRt.y, sizex, sizey, (int)wasWindowsSize));
			if (wasWindowsSize) {
				SetPosition(wxPoint(rt.x, rt.y));
				Layout();
				wasWindowsSize = false;
			}
			else {
				SetSize(newRt.x, newRt.y, newRt.width, newRt.height);
			}
			Options.SetCoords(WINDOW_POSITION, newRt.x, newRt.y);
		}
		
		KainoteFrame::Get()->Thaw();
	Options.SetCoords(VIDEO_WINDOW_SIZE, vsizex, vsizey);
		//LastMonitorRect = rt;
	}
	
	return wxTopLevelWindow::MSWWindowProc(uMsg, wParam, lParam);
}
#endif



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

