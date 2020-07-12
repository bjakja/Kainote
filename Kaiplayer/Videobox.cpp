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



#include <wx/gdicmn.h>
#include <wx/regex.h>
#include <wx/dir.h>
#include "Videobox.h"
#include "KainoteMain.h"
#include "Hotkeys.h"
#include <shellapi.h>
#include <wx/clipbrd.h>
#include "ColorSpaceConverter.h"
#include "Menu.h"
#include "KaiMessageBox.h"
#include "KaiStaticText.h"
#include "RendererDirectShow.h"
#include "RendererFFMS2.h"

#pragma warning ( disable: 4482 )

class CRecycleFile : public SHFILEOPSTRUCT {
protected:
public:
	CRecycleFile();
	~CRecycleFile() { }
	int Recycle(const wchar_t *pszPath, BOOL bDelete = FALSE);
};

CRecycleFile::CRecycleFile()
{
	memset((SHFILEOPSTRUCT*)this, 0, sizeof(SHFILEOPSTRUCT));
	fFlags |= FOF_SILENT;
	fFlags |= FOF_NOERRORUI;
	fFlags |= FOF_NOCONFIRMATION;
}


int CRecycleFile::Recycle(const wchar_t *pszPath, BOOL bDelete)
{

	wchar_t buf[_MAX_PATH + 1];
	wcscpy(buf, pszPath);
	buf[wcslen(buf) + 1] = 0;

	wFunc = FO_DELETE;
	pFrom = buf;
	pTo = NULL;
	if (bDelete) {
		fFlags &= ~FOF_ALLOWUNDO;
	}
	else {
		fFlags |= FOF_ALLOWUNDO;
	}
	return SHFileOperation(this);

}

class AspectRatioDialog : public KaiDialog
{
public:
	AspectRatioDialog(VideoCtrl *parent, float AspectRatio);
	virtual ~AspectRatioDialog(){};

	KaiSlider *slider;
	KaiStaticText *actual;
	void OnSlider(wxCommandEvent &event);
	VideoCtrl *_parent;
};

AspectRatioDialog::AspectRatioDialog(VideoCtrl *parent, float AspectRatio)
	: KaiDialog(parent->GetMessageWindowParent(), -1, L"", wxDefaultPosition, wxDefaultSize)
{
	_parent = parent;
	DialogSizer *sizer = new DialogSizer(wxVERTICAL);
	actual = new KaiStaticText(this, -1, wxString::Format(_("Proporcje ekranu: %5.3f"), 1.f / AspectRatio));
	slider = new KaiSlider(this, 7767, AspectRatio * 700000, 100000, 1000000, wxDefaultPosition, wxSize(400, -1), wxHORIZONTAL | wxSL_INVERSE);
	Connect(7767, wxEVT_SCROLL_THUMBTRACK, (wxObjectEventFunction)&AspectRatioDialog::OnSlider);
	sizer->Add(actual, 0, wxALL, 3);
	sizer->Add(slider, 1, wxEXPAND | wxALL, 3);
	SetSizerAndFit(sizer);
	MoveToMousePosition(this);
}

void AspectRatioDialog::OnSlider(wxCommandEvent &event)
{
	float AspectRatio = slider->GetValue() / 700000.0f;
	_parent->SetAspectRatio(AspectRatio);
	actual->SetLabelText(wxString::Format(_("Proporcje ekranu: %5.3f"), 1.f / AspectRatio));
}


VideoCtrl::VideoCtrl(wxWindow *parent, KainoteFrame *kfpar, const wxSize &size)
	: wxWindow(parent, -1, wxDefaultPosition, size)
	, Kai(kfpar)
	, tab((TabPanel*)parent)
	//, m_HasArrow(true)
	, m_IsMenuShown(false)
	, m_ArrowEater(false)
	, actualFile(0)
	, prevchap(-1)
	, m_FullScreenWindow(NULL)
	, m_blockRender(false)
	, m_IsOnAnotherMonitor(false)
	, m_ShownKeyframe(false)
	, m_IsFullscreen(false)
{
	int fw;
	GetTextExtent(L"#TWFfGH", &fw, &m_ToolBarHeight);
	m_ToolBarHeight += 8;
	m_PanelHeight = 30 + (m_ToolBarHeight * 2) - 8;

	m_VideoPanel = new wxWindow(this, -1, wxPoint(0, size.y - m_PanelHeight), wxSize(size.x, m_PanelHeight));
	m_VideoPanel->SetBackgroundColour(Options.GetColour(WINDOW_BACKGROUND));
	m_VideoPanel->SetCursor(wxCURSOR_ARROW);

	m_SeekingSlider = new VideoSlider(m_VideoPanel, ID_SLIDER, wxPoint(0, 1), wxSize(size.x, m_ToolBarHeight - 8));
	m_SeekingSlider->VB = this;
	m_ButtonPreviousFile = new BitmapButton(m_VideoPanel, CreateBitmapFromPngResource(L"backward"), CreateBitmapFromPngResource(L"backward1"),
		VIDEO_PREVIOUS_FILE, _("Poprzedni plik"), wxPoint(5, m_ToolBarHeight - 6), wxSize(26, 26));
	m_ButtonPause = new BitmapButton(m_VideoPanel, CreateBitmapFromPngResource(L"play"), CreateBitmapFromPngResource(L"play1"),
		VIDEO_PLAY_PAUSE, _("Odtwórz / Pauza"), wxPoint(40, m_ToolBarHeight - 6), wxSize(26, 26));
	m_ButtonPlayLine = new BitmapButton(m_VideoPanel, CreateBitmapFromPngResource(L"playline"), CreateBitmapFromPngResource(L"playline1"),
		GLOBAL_PLAY_ACTUAL_LINE, _("Odtwórz aktywną linię"), wxPoint(75, m_ToolBarHeight - 6), wxSize(26, 26), GLOBAL_HOTKEY);
	m_ButtonStop = new BitmapButton(m_VideoPanel, CreateBitmapFromPngResource(L"stop"), CreateBitmapFromPngResource(L"stop1"),
		VIDEO_STOP, _("Zatrzymaj"), wxPoint(110, m_ToolBarHeight - 6), wxSize(26, 26));
	m_ButtonNextFile = new BitmapButton(m_VideoPanel, CreateBitmapFromPngResource(L"forward"), CreateBitmapFromPngResource(L"forward1"),
		VIDEO_NEXT_FILE, _("Następny plik"), wxPoint(145, m_ToolBarHeight - 6), wxSize(26, 26));

	m_VolumeSlider = new VolSlider(m_VideoPanel, ID_VOL, Options.GetInt(VIDEO_VOLUME), wxPoint(size.x - 110, m_ToolBarHeight - 5), wxSize(110, 25));
	m_TimesTextField = new KaiTextCtrl(m_VideoPanel, -1, L"", wxPoint(180, m_ToolBarHeight - 6), wxSize(360, 26), wxTE_READONLY);
	m_TimesTextField->SetWindowStyle(wxBORDER_NONE);
	m_TimesTextField->SetCursor(wxCURSOR_ARROW);
	m_TimesTextField->SetBackgroundColour(WINDOW_BACKGROUND);

	m_VideoToolbar = new VideoToolbar(m_VideoPanel, wxPoint(0, m_PanelHeight - m_ToolBarHeight), wxSize(-1, m_ToolBarHeight));
	Bind(wxEVT_COMMAND_MENU_SELECTED, &VideoCtrl::OnChangeVisual, this, ID_VIDEO_TOOLBAR_EVENT);

	Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &evt){
		renderer->VisualChangeTool(evt.GetInt());
	}, ID_VECTOR_TOOLBAR_EVENT);
	Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &evt){
		renderer->VisualChangeTool(evt.GetInt());
	}, ID_MOVE_TOOLBAR_EVENT);
	Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &evt){
		renderer->VisualChangeTool(evt.GetInt());
	}, ID_SCALE_ROTATE_TOOLBAR_EVENT);
	Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &evt){
		RefreshTime();
	}, ID_REFRESH_TIME);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &KainoteFrame::OnMenuSelected1, Kai, GLOBAL_PLAY_ACTUAL_LINE);
	Connect(VIDEO_PREVIOUS_FILE, VIDEO_NEXT_FILE, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&VideoCtrl::OnAccelerator);
	Connect(VIDEO_PLAY_PAUSE, VIDEO_STOP, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&VideoCtrl::OnAccelerator);
	Connect(ID_VOL, wxEVT_COMMAND_SLIDER_UPDATED, (wxObjectEventFunction)&VideoCtrl::OnVolume);

	m_VideoTimeTimer.SetOwner(this, ID_VIDEO_TIME);
	idletime.SetOwner(this, ID_IDLE);

}

VideoCtrl::~VideoCtrl()
{
	SAFE_DELETE(renderer);
}

bool VideoCtrl::Play()
{
	if (!renderer)
		return false;

	wxMutexLocker lock(vbmutex);
	if (!renderer->Play()){ return false; }
	int ms = (m_IsFullscreen && !m_FullScreenWindow->panel->IsShown()) ? 1000 : 100;
	m_VideoTimeTimer.Start(ms);
	ChangeButtonBMP(false);
	return true;
}

void VideoCtrl::PlayLine(int start, int end)
{
	if (!renderer)
		return;
	//wxMutexLocker lock(vbmutex);
	if (!renderer->PlayLine(start, end)){ return; }
	int ms = (m_IsFullscreen && !m_FullScreenWindow->panel->IsShown()) ? 1000 : 100;
	m_VideoTimeTimer.Start(ms);
	ChangeButtonBMP(false);
}

bool VideoCtrl::Pause(bool skipWhenOnEnd)
{
	wxMutexLocker lock(vbmutex);

	if (!renderer){
		MenuItem *index = Kai->Menubar->FindItem(GLOBAL_VIDEO_INDEXING);
		if (index->IsChecked() && index->IsEnabled()){
			EditBox *eb = tab->Edit;
			if (eb->ABox){
				eb->ABox->audioDisplay->Play(eb->line->Start.mstime, eb->line->End.mstime);
				return true;
			}
			return false;
		}
		LoadVideo(Kai->videorec[Kai->videorec.size() - 1], CLOSE_SUBTITLES);
		return true;
	}
	if (renderer->m_Time >= renderer->GetDuration() && skipWhenOnEnd){ return false; }
	if (!renderer->Pause()){ return false; }
	if (GetState() == Paused){
		m_VideoTimeTimer.Stop(); RefreshTime();
	}
	else if (GetState() == Playing){
		int ms = (m_IsFullscreen && !m_FullScreenWindow->panel->IsShown()) ? 1000 : 100;
		m_VideoTimeTimer.Start(ms);
	}
	ChangeButtonBMP(!(GetState() == Playing));

	return true;
}

bool VideoCtrl::Stop()
{
	if (!renderer)
		return false;

	wxMutexLocker lock(vbmutex);

	if (!renderer->Stop()){ return false; }
	m_VideoTimeTimer.Stop();
	Seek(0);
	RefreshTime();

	ChangeButtonBMP(true);

	return true;
}

bool VideoCtrl::LoadVideo(const wxString& fileName, int subsFlag, bool fulls /*= false*/, 
	bool changeAudio, int customFFMS2, bool dontPlayOnStart)
{
	prevchap = -1;
	bool curentFFMS2 = !m_IsDirectShow;
	bool byFFMS2;
	if (customFFMS2 == -1){
		MenuItem *index = Kai->Menubar->FindItem(GLOBAL_VIDEO_INDEXING);
		byFFMS2 = index->IsChecked() && index->IsEnabled() && !fulls/* && !isFullscreen*/;
	}
	else
		byFFMS2 = customFFMS2 == 1;

	if (byFFMS2 != curentFFMS2){
		SAFE_DELETE(renderer);
	}
	if (!renderer){
		if (byFFMS2)
			renderer = new RendererFFMS2(this);
		else
			renderer = new RendererDirectShow(this);
	}


	bool shown = true;
	renderer->m_BlockResize = true;
	if (!renderer->OpenFile(fileName, subsFlag, !tab->editor, changeAudio)){
		renderer->m_BlockResize = false;
		if (!byFFMS2){ KaiMessageBox(_("Plik nie jest poprawnym plikiem wideo albo jest uszkodzony,\nbądź brakuje kodeków czy też splittera"), _("Uwaga")); }
		SAFE_DELETE(renderer)
		return false;
	}
	m_IsDirectShow = !byFFMS2;
	if (fulls) { SetFullscreen(); }
	if (!(IsShown() || (m_FullScreenWindow && m_FullScreenWindow->IsShown()))){
		shown = false; Show();
	}

	m_ArrowEater = m_IsDirectShow;

	if (!m_IsFullscreen && !fulls){
		int sx, sy;
		//wyłączony edytor
		if (!tab->editor){
			if (!Kai->IsMaximized()){
				int sizex, sizey;
				Kai->GetClientSize(&sizex, &sizey);
				CalcSize(&sx, &sy, 0, 0, true, true);
				wxSize panelsize = m_VideoPanel->GetSize();
				sx += Kai->borders.left + Kai->borders.right;
				sy += (m_PanelHeight + Kai->borders.bottom + Kai->borders.top);
				if (sx == sizex && sy == sizey){
					renderer->UpdateVideoWindow();
				}
				else{
					Kai->SetClientSize(sx, sy);
					tab->MainSizer->Layout();
				}
			}
			//załączony edytor
		}
		else{
			int kw, kh;
			Options.GetCoords(VIDEO_WINDOW_SIZE, &kw, &kh);
			bool ischanged = CalcSize(&sx, &sy, kw, kh, true, true);
			if (ischanged || !shown){
				SetMinSize(wxSize(sx, sy + m_PanelHeight));
				tab->MainSizer->Layout();
			}
			Options.SetCoords(VIDEO_WINDOW_SIZE, sx, sy + m_PanelHeight);
		}

	}
	if (m_IsFullscreen){
		renderer->UpdateVideoWindow();
		wxSize size; 
		renderer->GetVideoSize(&size.x, &size.y);
		Kai->SetVideoResolution(size.x, size.y, !Options.GetBool(DONT_ASK_FOR_BAD_RESOLUTION));
	}
	//renderer->m_BlockResize = false;
	if (m_IsDirectShow){
		Play();
		if ((tab->editor && !m_IsFullscreen) || dontPlayOnStart){ Pause(); }
		if (!m_VolumeSlider->IsShown()){ 
			m_VolumeSlider->Show(); 
			m_TimesTextField->SetSize(m_VideoWindowLastSize.x - 290, -1); 
		}
	}
	else{
		renderer->m_BlockResize = false;
		if (m_VolumeSlider->IsShown()){
			m_VolumeSlider->Show(false);
			m_TimesTextField->SetSize(m_VideoWindowLastSize.x - 185, -1);
		}
		renderer->m_State = Paused;
		renderer->Render(true, false);
	}

	RefreshTime();
	if (m_IsDirectShow){
		int pos = Options.GetInt(VIDEO_VOLUME);
		renderer->SetVolume(-(pos * pos));
	}
	//SetFocus();
	tab->VideoPath = fileName;
	tab->VideoName = fileName.AfterLast(L'\\');
	Kai->SetStatusText(tab->VideoName, 8);
	if (m_FullScreenWindow){ m_FullScreenWindow->Videolabel->SetLabelText(tab->VideoName); }
	if (!tab->editor){ Kai->Label(0, true); }
	Kai->SetStatusText(getfloat(m_FPS) + L" FPS", 4);
	wxString tar;
	tar << m_AspectRatioX << L" : " << m_AspectRatioY;
	Kai->SetStatusText(tar, 6);
	STime duration;
	duration.mstime = renderer->GetDuration();
	Kai->SetStatusText(duration.raw(SRT), 3);
	Kai->SetRecent(1);

	if (tab->editor && (!m_IsFullscreen || IsShown()) &&
		tab->SubsPath != L"" && Options.GetBool(OPEN_VIDEO_AT_ACTIVE_LINE)){
		Seek(tab->Edit->line->Start.mstime);
	}
	if (Options.GetBool(EDITBOX_TIMES_TO_FRAMES_SWITCH)){
		tab->Edit->SetLine(tab->Grid->currentLine);
		tab->Grid->RefreshColumns(START | END);
	}
	SetScaleAndZoom();
	ChangeStream();
	return true;
}


PlaybackState VideoCtrl::GetState()
{
	if (!renderer)
		return None;

	return renderer->m_State;
}

bool VideoCtrl::Seek(int whre, bool starttime/*=true*/, bool disp/*=true*/, bool reloadSubs/*=true*/, bool correct /*= true*/, bool asynchonize /*= true*/)
{
	wxMutexLocker lock(vbmutex);
	if (!renderer){ return false; }
	renderer->SetPosition(whre, starttime, correct, asynchonize);
	return true;
}

int VideoCtrl::Tell()
{
	if (!renderer)
		return 0;

	return renderer->GetCurrentPosition();
}


void VideoCtrl::OnSize(wxSizeEvent& event)
{
	wxSize asize = GetClientSize();
	if (m_VideoWindowLastSize == asize){ return; }
	m_VideoWindowLastSize = asize;
	int oldToolbarHeight = m_ToolBarHeight;
	int fw;
	GetTextExtent(L"#TWFfGH", &fw, &m_ToolBarHeight);
	m_ToolBarHeight += 8;
	m_PanelHeight = 30 + (m_ToolBarHeight * 2) - 8;
	if (!m_VideoToolbar->IsShown())
		m_PanelHeight -= m_ToolBarHeight;

	m_VideoPanel->SetSize(0, asize.y - m_PanelHeight, asize.x, m_PanelHeight);
	int difSize = (m_VolumeSlider->IsShown()) ? 290 : 185;
	if (oldToolbarHeight == m_ToolBarHeight){
		m_ButtonPreviousFile->SetPosition(wxPoint(5, m_ToolBarHeight - 6));
		m_ButtonPause->SetPosition(wxPoint(40, m_ToolBarHeight - 6));
		m_ButtonPlayLine->SetPosition(wxPoint(75, m_ToolBarHeight - 6));
		m_ButtonStop->SetPosition(wxPoint(110, m_ToolBarHeight - 6));
		m_ButtonNextFile->SetPosition(wxPoint(145, m_ToolBarHeight - 6));

		m_TimesTextField->SetSize(180, m_ToolBarHeight - 6, asize.x - difSize, -1);
		m_VideoToolbar->SetSize(0, m_PanelHeight - m_ToolBarHeight, asize.x, m_ToolBarHeight);
	}
	else{
		m_TimesTextField->SetSize(asize.x - difSize, -1);
		m_VideoToolbar->SetSize(asize.x, m_ToolBarHeight);
	}
	m_VolumeSlider->SetPosition(wxPoint(asize.x - 110, m_ToolBarHeight - 5));
	m_SeekingSlider->SetSize(wxSize(asize.x, m_ToolBarHeight - 8));
	if (renderer){
		renderer->UpdateVideoWindow();
	}
	else{
		Refresh(false);
	}
}


void VideoCtrl::OnMouseEvent(wxMouseEvent& event)
{

	if (event.ButtonUp())
	{
		SetFocus();
		if (m_IsMenuShown){ 
			m_IsMenuShown = false; 
		}
	}
	if (GetState() == None){ return; }
	if (renderer->m_HasZoom){
		renderer->ZoomMouseHandle(event);
		return;
	}
	m_X = event.GetX();
	m_Y = event.GetY();
	int w, h;
	GetClientSize(&w, &h);
	bool onVideo = m_Y < h - m_PanelHeight;

	if (event.GetWheelRotation() != 0) {

		if (event.ControlDown() && !m_IsFullscreen){
			int step = event.GetWheelRotation() / event.GetWheelDelta();

			int mw, mh;
			GetParent()->GetClientSize(&mw, &mh);
			int newHeight = h + (step * 20);
			if (newHeight >= mh){ newHeight = mh - 3; }
			if (onVideo){
				if (h <= 350 && step < 0 || h == newHeight){ return; }
				tab->SetVideoWindowSizes(w, newHeight, event.ShiftDown());
			}
			return;
		}
		else if (!renderer->HasVisual(true)){
			if (!m_IsDirectShow){
				AudioBox *box = tab->Edit->ABox;
				if (box){
					int vol = box->GetVolume();
					int step = event.GetWheelRotation() / event.GetWheelDelta();
					vol += (step * 3);
					box->SetVolume(vol);
				}
			}
			else if (m_IsFullscreen){ m_FullScreenWindow->volslider->OnMouseEvent(event); }
			else{ m_VolumeSlider->OnMouseEvent(event); }
			return;
		}
	}


	if (renderer->HasVisual()){
		Visuals *visual = renderer->GetVisual();
		visual->OnMouseEvent(event);
		//only cross let for another click events
		//it's only uses moving
		if (visual->Visual != CROSS){
			return;
		}
	}

	if (event.RightUp()) {
		ContextMenu(event.GetPosition());
		SetCursor(wxCURSOR_ARROW);
		return;
	}

	if (event.LeftDClick() && event.GetModifiers() == 0){
		SetFullscreen();
		if (!m_IsFullscreen && tab->SubsPath != L"" && Options.GetBool(GRID_SET_VISIBLE_LINE_AFTER_FULL_SCREEN)){
			tab->Edit->Send(EDITBOX_LINE_EDITION, false);
			tab->Grid->SelVideoLine();
		}
		

		if (!onVideo){
			SetCursor(wxCURSOR_ARROW);
		}
		else{
			SetCursor(wxCURSOR_BLANK);
		}
		return;
	}

	if (m_IsFullscreen){
		if (m_ArrowEater && event.Moving() && !event.ButtonDown()){ 
			Sleep(200); 
			m_ArrowEater = false; 
			return; 
		}
		m_FullScreenWindow->GetClientSize(&w, &h);
		bool onFullVideo = m_Y < h - m_PanelHeight;
		if (!HasArrow() && !m_FullScreenWindow->showToolbar->GetValue()){ 
			SetCursor(wxCURSOR_ARROW); 
		}
		else if (HasArrow() && !m_IsMenuShown && m_FullScreenWindow->showToolbar->GetValue() && onFullVideo){
			SetCursor(wxCURSOR_BLANK);
		}

		if (!onFullVideo && !m_FullScreenWindow->panel->IsShown()){
			m_VideoTimeTimer.Start(100); 
			m_FullScreenWindow->panel->Show(); 
		}
		else if (onFullVideo && m_FullScreenWindow->panel->IsShown() && !m_PanelOnFullscreen){
			m_VideoTimeTimer.Start(1000); 
			m_FullScreenWindow->panel->Show(false); 
			SetFocus(); 
		}
		if (!m_FullScreenWindow->panel->IsShown() && !m_IsMenuShown){ 
			idletime.Start(1000, true); 
		}
	}
	else if (tab->editor){
		if (/*onVideo && */!m_IsMenuShown && HasArrow()){
			//KaiLog(wxString::Format(L"Vb blank %i", (int)m_IsMenuShown));
			SetCursor(wxCURSOR_BLANK);  
		}

	}
	else if (!HasArrow()){
		SetCursor(wxCURSOR_ARROW);  
	}



	if (Options.GetBool(VIDEO_PAUSE_ON_CLICK) && event.LeftUp() && !event.ControlDown()){
		Pause();
	}


	


}



void VideoCtrl::OnPlaytime(wxTimerEvent& event)
{
	RefreshTime();
}

void VideoCtrl::OnKeyPress(wxKeyEvent& event)
{
	int key = event.GetKeyCode();
	if (key == L'F'){ SetFullscreen(); }
	else if (key == WXK_WINDOWS_MENU){
		wxWindow *owner = (m_IsFullscreen && m_FullScreenWindow) ? (wxWindow *)m_FullScreenWindow : this;
		wxPoint poss = owner->ScreenToClient(wxGetMousePosition());
		ContextMenu(poss);
	}
	else if ((key == L'B' || key == WXK_ESCAPE) && m_IsFullscreen){
		SetFullscreen();
		if (tab->SubsPath != L"" && Options.GetBool(GRID_SET_VISIBLE_LINE_AFTER_FULL_SCREEN)){
			tab->Edit->Send(EDITBOX_LINE_EDITION, false);
			tab->Grid->SelVideoLine();
		}
		if (key == L'B'){
			if (GetState() == Playing){ Pause(); }
			ShowWindow(Kai->GetHWND(), SW_SHOWMINNOACTIVE);
		}
	}
	if (!renderer)
		return;

	if (key == WXK_RETURN && renderer->m_HasZoom){
		renderer->SetZoom();
	}
	else if (key == L'Z' && event.ControlDown() && event.ShiftDown()){
		renderer->ResetZoom();
	}
	else if (renderer->HasVisual()){
		renderer->GetVisual()->OnKeyPress(event);
	}
}


void VideoCtrl::OnIdle(wxTimerEvent& event)
{
	if (m_IsFullscreen && !m_FullScreenWindow->panel->IsShown() && !m_IsMenuShown){
		SetCursor(wxCURSOR_BLANK);
		m_ArrowEater = m_IsDirectShow;
	}
}



void VideoCtrl::NextFile(bool next)
{
	wxMutexLocker lock(nextmutex);
	wxString path;
	if (tab->VideoPath != L""){
		path = tab->VideoPath;
	}
	else{ path = Kai->videorec[Kai->videorec.size() - 1]; }
	wxString pathwn = path.BeforeLast(L'\\');
	wxDir kat(pathwn);
	if (kat.IsOpened()){
		files.Clear();
		kat.GetAllFiles(pathwn, &files, L"", wxDIR_FILES);
	}
	for (int j = 0; j < (int)files.GetCount(); j++)
	{
		if (files[j] == path){ actualFile = j; break; }
	}
	if (next && actualFile >= (int)files.GetCount() - 1){
		Seek(0);
		Pause(false);
		actualFile = files.GetCount() - 1;
		return;
	}
	else if (!next && actualFile <= 0){
		Seek(0);
		Pause(false);
		actualFile = 0;
		return;
	}

	int k = (next) ? actualFile + 1 : actualFile - 1;
	while ((next) ? k < (int)files.GetCount() : k >= 0)
	{

		wxString ext = files[k].AfterLast(L'.').Lower();
		if (ext == L"avi" || ext == L"mp4" || ext == L"mkv" || ext == L"ogm" || ext == L"wmv" ||
			ext == L"asf" || ext == L"rmvb" || ext == L"rm" || ext == L"3gp" ||//||ext==L"avs" przynajmniej do momentu dodania otwierania avs przy włączonym ffms2
			ext == L"ts" || ext == L"m2ts" || ext == L"mpg" || ext == L"mpeg"){


			bool isload = Kai->OpenFile(files[k]);
			if (isload){
				actualFile = k;
				if (m_IsFullscreen){ SetFocus(); }
				return;
			}
			else if (!m_IsDirectShow){ return; }
		}
		if (next){ k++; }
		else{ k--; }
	}
	Seek(0);
	Pause(false);
}

void VideoCtrl::SetFullscreen(int monitor)
{
	if (!renderer)
		return;
	//wxMutexLocker lock(vbmutex);
	m_IsFullscreen = !m_IsFullscreen;

	//turn off full screen
	if (!m_IsFullscreen){

		if (GetState() == Playing){ if (tab->editor){ Pause(); } else{ m_VideoTimeTimer.Start(100); } }
		if (m_FullScreenWindow->HasCapture()){ m_FullScreenWindow->ReleaseMouse(); }

		int sx, sy, sizex, sizey;

		if (!tab->editor){
			if (!Kai->IsMaximized()){
				Kai->GetSize(&sizex, &sizey);
				int yDiff = m_PanelHeight + Kai->borders.bottom + Kai->borders.top;
				int xDiff = Kai->borders.left + Kai->borders.right;
				CalcSize(&sx, &sy, sizex - xDiff, sizey - yDiff);
				Kai->SetSize(sx + xDiff, sy + yDiff);
			}
		}
		else{
			Options.GetCoords(VIDEO_WINDOW_SIZE, &sizex, &sizey);
			CalcSize(&sx, &sy, sizex, sizey);
			SetMinSize(wxSize(sx, sy + m_PanelHeight));
			tab->MainSizer->Layout();
		}
		m_VolumeSlider->SetValue(Options.GetInt(VIDEO_VOLUME));

		if (!IsShown()){
			m_IsOnAnotherMonitor = false;
			Show(); 
			GetParent()->Layout();
		}
		renderer->UpdateVideoWindow();
		renderer->m_BlockResize = true;
		renderer->Render();
		renderer->m_BlockResize = false;

		m_VideoToolbar->Synchronize(m_FullScreenWindow->vToolbar);
		RefreshTime();
		m_FullScreenWindow->Hide();
		SetCursor(wxCURSOR_ARROW);
	}
	//turn on fullscreen
	else{
		if (wxWindow::HasCapture()){ wxWindow::ReleaseMouse(); }
		wxRect rt = GetMonitorRect(monitor);
		if (!m_FullScreenWindow){
			m_FullScreenWindow = new Fullscreen(this, rt.GetPosition(), rt.GetSize());
			m_FullScreenWindow->Videolabel->SetLabelText(tab->VideoName);
		}
		else{
			m_FullScreenWindow->SetPosition(rt.GetPosition());
			m_FullScreenWindow->SetSize(rt.GetSize());
		}
		m_FullScreenWindow->OnSize();
		m_FullScreenWindow->volslider->SetValue(Options.GetInt(VIDEO_VOLUME));
		m_FullScreenWindow->vToolbar->Synchronize(m_VideoToolbar);
		if (!m_PanelOnFullscreen){ m_FullScreenWindow->panel->Hide(); }
		m_FullScreenWindow->Show();
		renderer->m_BlockResize = true;
		renderer->UpdateVideoWindow();
		renderer->m_BlockResize = false;
		RefreshTime();
		if (GetState() == Playing && !m_FullScreenWindow->panel->IsShown()){ m_VideoTimeTimer.Start(1000); }
		if (!tab->editor)
			m_FullScreenWindow->HideToolbar(true);
		if (monitor && tab->editor){
			Hide();
			GetParent()->Layout();
			m_IsOnAnotherMonitor = true;
		}
		else{
			SetFocus();
		}
	}
	ChangeButtonBMP(!(GetState() == Playing));


}

bool VideoCtrl::CalcSize(int *width, int *height, int wwidth, int wheight, bool setstatus, bool calcH)
{
	wxSize size;
	renderer->GetVideoSize(&size.x, &size.y);
	if (setstatus){
		Kai->SetVideoResolution(size.x, size.y, !Options.GetBool(DONT_ASK_FOR_BAD_RESOLUTION));
	}
	if (wheight == 0){//width can normally have 0 when editbox have 0
		GetClientSize(&wwidth, &wheight);
	}
	float precy = size.y, precx = size.x;
	if (!calcH){
		size.x = wwidth;
		if (m_AspectRatio <= 0){ size.y *= (wwidth / precx); }
		else{ size.y = size.x * m_AspectRatio; }
	}
	if (calcH || size.y > 700){
		wheight -= m_PanelHeight;
		size.y = wheight;
		if (m_AspectRatio > 0){ size.x = size.y / m_AspectRatio; }
		else{ size.x *= (wheight / precy); }
	}

	*width = size.x;
	*height = size.y;
	return !(size.x == wwidth && size.y == wheight);
}

void VideoCtrl::OnPrew()
{
	MenuItem *index = Kai->Menubar->FindItem(GLOBAL_VIDEO_INDEXING);
	if (index->IsChecked() && index->IsEnabled()/* && !isFullscreen*/){
		if (KaiMessageBox(_("Czy na pewno chcesz zindeksować poprzednie wideo?"), _("Potwierdzenie"), 
			wxYES_NO, (m_IsFullscreen) ? (wxWindow*)m_FullScreenWindow : Kai) == wxNO) return;
	}
	NextFile(false);
}


void VideoCtrl::OnNext()
{
	MenuItem *index = Kai->Menubar->FindItem(GLOBAL_VIDEO_INDEXING);
	if (index->IsChecked() && index->IsEnabled()/* && !isFullscreen*/){
		if (KaiMessageBox(_("Czy na pewno chcesz zindeksować następne wideo?"), _("Potwierdzenie"), 
			wxYES_NO, (m_IsFullscreen)? (wxWindow*)m_FullScreenWindow : Kai) == wxNO) return;
	}
	NextFile();
}

void VideoCtrl::OnVolume(wxScrollEvent& event)
{
	int pos = event.GetPosition();
	Options.SetInt(VIDEO_VOLUME, pos);
	renderer->SetVolume(-(pos * pos));
}

void VideoCtrl::ContextMenu(const wxPoint &pos)
{
	m_IsMenuShown = true;
	Menu* menu = new Menu(VIDEO_HOTKEY);
	bool editor = tab->editor;
	wxString txt = L"\t" + Hkeys.GetStringHotkey(VIDEO_PLAY_PAUSE);
	if (GetState() != Playing){ txt.Prepend(_("Odtwórz")); }
	else if (GetState() == Playing){ txt.Prepend(_("Pauza")); }
	if (!m_IsFullscreen && editor)
	{
		menu->SetAccMenu(VIDEO_COPY_COORDS, _("Kopiuj pozycję na wideo"));
	}
	menu->Append(VIDEO_PLAY_PAUSE, txt)->Enable(GetState() != None);
	menu->SetAccMenu(VIDEO_STOP, _("Zatrzymaj"))->Enable(GetState() == Playing);
	wxString txt1;
	if (!m_IsFullscreen){ txt1 = _("Pełny ekran\tF"); }
	else{ txt1 = _("Wyłącz pełny ekran\tEscape"); }
	MenuItem *Item = menu->Append(VIDEO_FULL_SCREEN, txt1);
	Item->Enable(GetState() != None);
	Item->DisableMapping();
	GetMonitorRect(-1);
	for (size_t i = 1; i < MonRects.size(); i++)
	{
		wxString txt2;
		if (m_IsFullscreen){ txt2 = wxString::Format(_("Przełącz pełny ekran na %i monitor"), (int)(i + 1)); }
		else{ txt2 = wxString::Format(_("Włącz pełny ekran na %i monitorze"), (int)(i + 1)); }
		menu->Append(MENU_MONITORS + i, txt2)->Enable(GetState() != None);
	}
	menu->SetWindow(GLOBAL_HOTKEY);
	menu->SetAccMenu(GLOBAL_EDITOR, _("Otwórz edytor"))->Enable(m_IsFullscreen);
	Menu* menu1 = new Menu();
	Menu* menu2 = new Menu();
	for (size_t i = 0; i < 20; i++)
	{
		if (i < Kai->subsrec.size()){
			if (!wxFileExists(Kai->subsrec[i])){ continue; }
			menu1->Append(30000 + i, Kai->subsrec[i].AfterLast(L'\\'));
		}
		if (i < Kai->videorec.size()){
			if (!wxFileExists(Kai->videorec[i])){ continue; }
			menu2->Append(30020 + i, Kai->videorec[i].AfterLast(L'\\'));
		}

	}
	menu->Append(ID_MRECSUBS, _("Ostatnio otwarte napisy"), menu1);
	menu->Append(ID_MRECVIDEO, _("Ostatnio otwarte wideo"), menu2);
	menu->SetAccMenu(GLOBAL_OPEN_VIDEO, _("Otwórz wideo"));

	menu->SetAccMenu(GLOBAL_OPEN_SUBS, _("Otwórz napisy"));
	menu->SetWindow(VIDEO_HOTKEY);
	menu->SetAccMenu(VIDEO_HIDE_PROGRESS_BAR, _("Ukryj / pokaż pasek postępu"))->Enable(m_IsFullscreen);
	menu->SetAccMenu(VIDEO_ASPECT_RATIO, _("Zmień proporcje wideo"));
	menu->SetAccMenu(VIDEO_SAVE_SUBBED_FRAME_TO_PNG, _("Zapisz klatkę z napisami jako PNG"))->Enable(GetState() == Paused);
	menu->SetAccMenu(VIDEO_COPY_SUBBED_FRAME_TO_CLIPBOARD, _("Kopiuj klatkę z napisami do schowka"))->Enable(GetState() == Paused);
	menu->SetAccMenu(VIDEO_SAVE_FRAME_TO_PNG, _("Zapisz klatkę jako PNG"))->Enable(GetState() == Paused && tab->editor);
	menu->SetAccMenu(VIDEO_COPY_FRAME_TO_CLIPBOARD, _("Kopiuj klatkę do schowka"))->Enable(GetState() == Paused && tab->editor);
	menu->AppendSeparator();

	menu->SetAccMenu(VIDEO_DELETE_FILE, _("Usuń plik wideo"))->Enable(GetState() != None);

	Menu* menu3 = NULL;
	int numfilters = 0;
	if (GetState() != None && m_IsDirectShow){
		menu3 = new Menu();
		renderer->EnumFilters(menu3);
		numfilters = menu3->GetMenuItemCount();
		menu->Append(23456, _("Filtry"), menu3, _("Wyświetla użyte filtry"));
	}


	wxArrayString streams = renderer->GetStreams();
	wxString prev;
	wxString name, enable;
	for (size_t i = 0; i < streams.size(); i++){
		wxString ident = streams[i].BeforeFirst(L':');
		name = streams[i].BeforeLast(L' ', &enable);
		if ((ident == L"S" || ident == L"s") && editor)
			break;
		if (ident != prev){ menu->AppendSeparator(); }
		menu->Append(MENU_STREAMS + i, name, L"", true, 0, 0, (enable == L"1") ? ITEM_RADIO : ITEM_NORMAL);//->Check(enable=="1");
		prev = ident;
	}
	STime timee;
	for (size_t j = 0; j < renderer->m_Chapters.size(); j++){
		if (j == 0){ menu->AppendSeparator(); }
		timee.NewTime(renderer->m_Chapters[j].time);
		int ntime = (j >= renderer->m_Chapters.size() - 1) ? INT_MAX : renderer->m_Chapters[(j + 1)].time;
		menu->Append(MENU_CHAPTERS + j, renderer->m_Chapters[j].name + L"\t[" + timee.raw() + L"]",
			L"", true, 0, 0, (ntime > renderer->m_Time) ? ITEM_RADIO : ITEM_NORMAL);
	}
	id = 0;
	int Modifiers = 0;
	menu->SetMaxVisible(40);
	//ismenu=true;
	if (m_IsFullscreen){
		id = menu->GetPopupMenuSelection(pos, m_FullScreenWindow, &Modifiers, true);
	}
	else{
		id = menu->GetPopupMenuSelection(pos, this, &Modifiers, true);
	}
	//ismenu=false;
	if ((Modifiers == wxMOD_SHIFT) && id < 2100 && id >= 2000){
		Hkeys.OnMapHkey(id, L"", this, VIDEO_HOTKEY);
		delete menu;
		m_IsMenuShown = false;
		return;
	}

	if (id == VIDEO_COPY_COORDS){ OnCopyCoords(pos); }
	else if (id > 1999 && id < MENU_STREAMS){
		wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, id);
		OnAccelerator(evt);
	}
	else if (id >= 30000 && id < 30020){ Kai->OpenFile(Kai->subsrec[id - 30000]); }
	else if (id >= 30020 && id < 30040){ Kai->OpenFile(Kai->videorec[id - 30020]); }
	else if (id >= MENU_STREAMS && id < MENU_STREAMS + (int)streams.size()){
		int wstream = id - MENU_STREAMS;
		renderer->EnableStream((long)wstream);
	}
	else if (id >= MENU_CHAPTERS && id < MENU_CHAPTERS + (int)renderer->m_Chapters.size()){
		Seek(renderer->m_Chapters[id - MENU_CHAPTERS].time);
	}
	else if (id >= 13000 && id < 13000 + numfilters && menu3){
		MenuItem *item = menu3->FindItem(id);
		renderer->FilterConfig(item->GetLabel(), id - 13000, pos);
	}
	else if (id > 15000 && id < 15000 + (int)MonRects.size()){
		m_IsFullscreen = false;
		SetFullscreen(id - 15000);
	}
	delete menu;
	m_IsMenuShown = false;
}


void VideoCtrl::OnHidePB()
{
	if (!renderer)
		return;

	bool pb = !Options.GetBool(VIDEO_PROGRESS_BAR);
	Options.SetBool(VIDEO_PROGRESS_BAR, pb);
	if (pb){ m_FullScreenProgressBar = true; RefreshTime(); }
	else{ m_FullScreenProgressBar = false; }
	if (GetState() == Paused){ renderer->Render(false); }
}

void VideoCtrl::OnDeleteVideo()
{
	wxString path = tab->VideoPath;
	if (path == L"" && KaiMessageBox(_("Czy na pewno chcesz przenieść wczytany plik wideo do kosza?"), _("Usuwanie"), wxYES_NO) == wxNO){ return; }
	NextFile();
	CRecycleFile x;
	x.Recycle(path.data());
}

void VideoCtrl::OnOpVideo()
{
	wxFileDialog* FileDialog2 = new wxFileDialog(this, _("Wybierz plik wideo"),
		(Kai->videorec.size() > 0) ? Kai->videorec[Kai->videorec.size() - 1].BeforeLast(L'\\') : L"",
		L"", _("Pliki wideo(*.avi),(*.mkv),(*.mp4),(*.ogm),(*.wmv),(*.asf),(*.rmvb),(*.rm),(*.3gp),(*.avs)|*.avi;*.mkv;*.mp4;*.ogm;*.wmv;*.asf;*.rmvb;*.rm;*.3gp;*.avs|Wszystkie pliki (*.*)|*.*"),
		wxFD_DEFAULT_STYLE);
	if (FileDialog2->ShowModal() == wxID_OK){
		Kai->OpenFile(FileDialog2->GetPath());
	}
	FileDialog2->Destroy();
}

void VideoCtrl::OnOpSubs()
{
	if (Kai->SavePrompt(2)){ return; }
	wxFileDialog* FileDialog2 = new wxFileDialog(Kai, _("Wybierz plik napisów"),
		(Kai->subsrec.size() > 0) ? Kai->subsrec[Kai->subsrec.size() - 1].BeforeLast(L'\\') : L"", L"",
		_("Pliki napisów (*.ass),(*.sub),(*.txt)|*.ass;*.sub;*.txt"),
		wxFD_DEFAULT_STYLE);

	if (FileDialog2->ShowModal() == wxID_OK){
		Kai->OpenFile(FileDialog2->GetPath());
	}
	FileDialog2->Destroy();
}

void VideoCtrl::OpenEditor(bool esc)
{

	if (m_IsFullscreen){
		if (GetState() == Playing){ Pause(); }
		Options.SetBool(EDITOR_ON, true);
		if (tab->SubsPath != L""){
			tab->Grid->SelVideoLine();
		}

		SetFullscreen();

		if (!esc){ ShowWindow(Kai->GetHWND(), SW_MINIMIZE); }
	}

}


void VideoCtrl::OnAccelerator(wxCommandEvent& event)
{

	id = event.GetId();
	if (Options.CheckLastKeyEvent(id, 50))
		return;

	if (id == VIDEO_PLAY_PAUSE){ Pause(); }
	else if (id == VIDEO_5_SECONDS_BACKWARD){ Seek(Tell() - 5000); }
	else if (id == VIDEO_5_SECONDS_FORWARD){ Seek(Tell() + 5000); }
	else if (id == VIDEO_MINUTE_BACKWARD){ Seek(Tell() - 60000); }
	else if (id == VIDEO_MINUTE_FORWARD){ Seek(Tell() + 60000); }
	else if (id == VIDEO_PREVIOUS_FILE){ OnPrew(); }
	else if (id == VIDEO_NEXT_FILE){ OnNext(); }
	else if (id == VIDEO_VOLUME_PLUS){ OnSPlus(); }
	else if (id == VIDEO_VOLUME_MINUS){ OnSMinus(); }
	else if (id == VIDEO_PREVIOUS_CHAPTER){ PrevChap(); }
	else if (id == VIDEO_NEXT_CHAPTER){ NextChap(); }
	else if (id == VIDEO_COPY_COORDS){ wxPoint pos = wxGetMousePosition(); pos = ScreenToClient(pos); OnCopyCoords(pos); }
	else if (id == VIDEO_STOP){
		if (!tab->editor){ Stop(); }
		else{
			if (GetState() == Playing){
				Pause();
			}
			if (m_IsDirectShow){ Seek(0); }
		}
	}
	else if (id == VIDEO_FULL_SCREEN){ SetFullscreen(); }
	else if (id == GLOBAL_EDITOR){ OpenEditor(); }
	else if (id == GLOBAL_OPEN_VIDEO){ OnOpVideo(); }
	else if (id == GLOBAL_OPEN_SUBS){ OnOpSubs(); }
	else if (id == VIDEO_HIDE_PROGRESS_BAR){ OnHidePB(); }
	else if (id == VIDEO_ASPECT_RATIO){
		AspectRatioDialog changear(this, m_AspectRatio);
		changear.ShowModal();
	}
	else if (id == VIDEO_DELETE_FILE){ OnDeleteVideo(); }
	else if (id >= VIDEO_SAVE_FRAME_TO_PNG && id <= VIDEO_COPY_SUBBED_FRAME_TO_CLIPBOARD && GetState() == Paused){
		CColorSpaceConverter conv(renderer->m_Format, renderer->m_Width, renderer->m_Height);
		bool del = false;
		byte *framebuf = renderer->GetFramewithSubs(id > VIDEO_COPY_FRAME_TO_CLIPBOARD, &del);
		if (id == VIDEO_SAVE_FRAME_TO_PNG || id == VIDEO_SAVE_SUBBED_FRAME_TO_PNG){
			wxString path;
			int num = 1;
			wxArrayString paths;
			wxString filespec;
			wxString dirpath = tab->VideoPath.BeforeLast(L'\\', &filespec);
			wxDir kat(dirpath);
			path = tab->VideoPath;
			if (kat.IsOpened()){
				kat.GetAllFiles(dirpath, &paths, filespec.BeforeLast(L'.') << L"_*_*.png", wxDIR_FILES);
			}
			for (wxString &file : paths){
				if (file.find(L"_" + std::to_string(num) + L"_") == wxNOT_FOUND){
					break;
				}
				num++;
			}
			path = tab->VideoPath.BeforeLast(L'.');
			STime currentTime;
			currentTime.mstime = renderer->m_Time;
			wxString timestring = currentTime.raw(SRT);
			timestring.Replace(L":", L";");
			//path.Replace(L",", L".");
			path << L"_" << num << L"_" << timestring << L".png";

			if (wxFileExists(path)){
				bool thisisbad = true;
			}
			conv.SavePNG(path, framebuf);
		}
		else{
			conv.SavetoClipboard(framebuf);
		}
		if (del){ delete framebuf; }
	}
}


void VideoCtrl::OnSMinus()
{
	if (!renderer)
		return;

	if (m_IsDirectShow){
		int pos = m_VolumeSlider->GetValue() - 2;
		if (pos > (-91)){
			renderer->SetVolume(-(pos * pos));
			m_VolumeSlider->SetValue(pos);
			if (m_FullScreenWindow){ m_FullScreenWindow->volslider->SetValue(pos); }
			Options.SetInt(VIDEO_VOLUME, pos);
		}
	}
	else{
		AudioBox *box = tab->Edit->ABox;
		if (box){
			int vol = box->GetVolume();
			vol -= 2;
			if (vol >= 0){
				box->SetVolume(vol);
			}
		}
	}
}

void VideoCtrl::OnSPlus()
{
	if (!renderer)
		return;

	if (m_IsDirectShow){
		int pos = m_VolumeSlider->GetValue() + 2;
		if (pos < 1){
			renderer->SetVolume(-(pos * pos));
			m_VolumeSlider->SetValue(pos);
			if (m_FullScreenWindow){ m_FullScreenWindow->volslider->SetValue(pos); }
			Options.SetInt(VIDEO_VOLUME, pos);
		}
	}
	else{

		AudioBox *box = tab->Edit->ABox;
		if (box){
			int vol = box->GetVolume();
			vol += 2;
			if (vol < 102){
				box->SetVolume(vol);
			}
		}

	}
}

void VideoCtrl::OnPaint(wxPaintEvent& event)
{
	if (renderer && !renderer->m_BlockResize && renderer->m_State == Paused){
		renderer->Render(true, false);
	}
	else if (GetState() == None){
		int x, y;
		GetClientSize(&x, &y);
		wxPaintDC dc(this);
		dc.SetBrush(wxBrush(L"#000000"));
		dc.SetPen(wxPen(L"#000000"));
		dc.DrawRectangle(0, 0, x, y);
		wxFont font1(72, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, L"Tahoma");
		dc.SetFont(font1);
		wxSize size = dc.GetTextExtent(L"KaiNote");
		dc.SetTextForeground(L"#2EA6E2");
		dc.DrawText(L"KaiNote", (x - size.x) / 2, (y - size.y - m_PanelHeight) / 2);
	}

}

void VideoCtrl::OnEndFile(wxCommandEvent &event)
{
	if ((!tab->editor || m_IsFullscreen) && m_IsDirectShow){ NextFile(); }
	else{ if (GetState() == Playing){ Pause(false); } }
}

void VideoCtrl::SetAspectRatio(float _AR)
{
	if (!renderer)
		return;

	m_AspectRatio = _AR;
	if (tab->editor && !m_IsFullscreen){
		int ww, hh;
		CalcSize(&ww, &hh, 0, 0, false, true);
		SetMinSize(wxSize(ww, hh + m_PanelHeight));
		tab->MainSizer->Layout();
	}
	renderer->UpdateVideoWindow();
	if (GetState() == Paused){ renderer->Render(false); }
}

void VideoCtrl::SetScaleAndZoom()
{
	if (!renderer)
		return;

	wxString scale;
	wxSize wsize = GetSize();
	scale << (int)((wsize.x / (float)renderer->m_Width) * 100) << L"%";
	Kai->SetStatusText(scale, 1);
	wxString zoom;
	zoom << (int)(renderer->m_ZoomParcent * 100) << L"%";
	Kai->SetStatusText(zoom, 2);
}

void VideoCtrl::ChangeOnScreenResolution(TabPanel *tab)
{
	if (renderer && renderer->HasVisual())
		return;

	Visuals *visual = renderer->GetVisual();
	if (visual->Visual != CROSS)
		return;

	visual->SetCurVisual();
}

void VideoCtrl::RefreshTime()
{
	if (!renderer)
		return;

	STime videoTime;
	videoTime.mstime = renderer->m_Time;
	float dur = renderer->GetDuration();
	float val = (dur > 0) ? videoTime.mstime / dur : 0.0;

	if (m_IsFullscreen){
		m_FullScreenWindow->vslider->SetValue(val);
		if (m_FullScreenWindow->panel->IsShown()){
			wxString times;
			times << videoTime.raw(SRT) << L";  ";
			if (!m_IsDirectShow){
				times << renderer->m_Frame << L";  ";
				if (renderer->HasFFMS2()){
					if (renderer->GetFFMS2()->KeyFrames.Index(renderer->m_Time) != -1){
						m_ShownKeyframe = true;
						m_FullScreenWindow->mstimes->SetForegroundColour(WINDOW_WARNING_ELEMENTS);
					}
					else if (m_ShownKeyframe){
						m_ShownKeyframe = false;
						m_FullScreenWindow->mstimes->SetForegroundColour(WINDOW_TEXT);
					}
				}
			}
			if (tab->editor){
				Dialogue *line = tab->Edit->line;
				int sdiff = videoTime.mstime - ZEROIT(line->Start.mstime);
				int ediff = videoTime.mstime - ZEROIT(line->End.mstime);
				times << sdiff << L" ms, " << ediff << L" ms";
			}
			m_FullScreenWindow->mstimes->SetValue(times);
			m_FullScreenWindow->mstimes->Update();
		}
		if (!m_FullScreenProgressBar){ return; }
		STime kkk1;
		kkk1.mstime = dur;
		renderer->m_ProgressBarTime = videoTime.raw(TMP) + L" / " + kkk1.raw(TMP);
		renderer->DrawProgBar();
	}
	else{
		m_SeekingSlider->SetValue(val);
		m_SeekingSlider->Update();
		wxString times;
		times << videoTime.raw(SRT) << L";  ";
		if (!m_IsDirectShow){
			times << renderer->m_Frame << L";  ";
			if (renderer->HasFFMS2()){
				if (renderer->GetFFMS2()->KeyFrames.Index(renderer->m_Time) != -1){
					m_ShownKeyframe = true;
					m_TimesTextField->SetForegroundColour(WINDOW_WARNING_ELEMENTS);
				}
				else if (m_ShownKeyframe){
					m_ShownKeyframe = false;
					m_TimesTextField->SetForegroundColour(WINDOW_TEXT);
				}
			}
		}
		if (tab->editor){
			Dialogue *line = tab->Edit->line;
			int sdiff = videoTime.mstime - ZEROIT(line->Start.mstime);
			int ediff = videoTime.mstime - ZEROIT(line->End.mstime);
			times << sdiff << L" ms, " << ediff << L" ms";
			tab->Grid->RefreshIfVisible(videoTime.mstime);
		}
		m_TimesTextField->SetValue(times);
		m_TimesTextField->Update();

	}

}

void VideoCtrl::OnCopyCoords(const wxPoint &pos)
{
	int w, h;
	GetClientSize(&w, &h);
	int nx = 0, ny = 0;
	tab->Grid->GetASSRes(&nx, &ny);
	float coeffX = (float)nx / (float)(w - 1);
	float coeffY = (float)ny / (float)(h - m_PanelHeight - 1);
	int posx = (float)pos.x * coeffX;
	int posy = (float)pos.y * coeffY;
	wxString poss;
	poss << posx << L", " << posy;
	if (wxTheClipboard->Open())
	{
		wxTheClipboard->SetData(new wxTextDataObject(poss));
		wxTheClipboard->Close();
	}
}

void VideoCtrl::ChangeButtonBMP(bool play)
{
	if (m_IsFullscreen){ m_FullScreenWindow->bpause->ChangeBitmap(play); }
	else{ m_ButtonPause->ChangeBitmap(play); }
}

void VideoCtrl::NextChap()
{
	if (!renderer)
		return;

	const auto &chapters = renderer->m_Chapters;
	if (chapters.size() < 1){ return; }
	int vrtime = Tell();
	for (int j = 0; j < (int)chapters.size(); j++){

		int ntime = (j >= (int)chapters.size() - 1) ? INT_MAX : chapters[(j + 1)].time;
		if (ntime > vrtime){

			int jj = (j >= (int)chapters.size() - 1 || (j == 0 && chapters[0].time >= vrtime)) ? 0 : j + 1;
			if (jj == prevchap/* && vrtime == chapters[jj].time*/){ 
				if (jj >= (int)chapters.size() - 1){ jj = 0; } 
				else{ jj++; } 
			}
			Seek(chapters[jj].time, true, true, true, false);

			prevchap = jj;
			break;
		}
	}
}

void VideoCtrl::PrevChap()
{
	if (!renderer)
		return;

	const auto &chapters = renderer->m_Chapters;
	if (chapters.size() < 1){ return; }
	int vrtime = Tell();
	for (int j = 0; j < (int)chapters.size(); j++){
		int ntime = (j >= (int)chapters.size() - 1) ? INT_MAX : chapters[(j + 1)].time;
		if (ntime > vrtime){
			int jj = (j < 1) ? 0 : j;
			if (jj == prevchap){
				if (jj < 1){ jj = chapters.size() - 1; } 
				else{ jj--; } 
			}
			Seek(chapters[jj].time, true, true, true, false);
			prevchap = jj;
			break;
		}
	}
}

void VideoCtrl::ConnectAcc(int id)
{
	Connect(id, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&VideoCtrl::OnAccelerator);
}

void VideoCtrl::ChangeStream()
{
	if (!m_IsDirectShow || !renderer){ return; }
	wxArrayString enabled;
	Options.GetTableFromString(ACCEPTED_AUDIO_STREAM, enabled, L";");
	if (enabled.size() < 1){ return; }
	wxArrayString streams = renderer->GetStreams();
	//int firstSubsStream = -1;
	int numofastreams = 0;
	for (int i = streams.size() - 1; i >= 0; i--){
		if (streams[i][0] == L'A'){
			streams[i] = streams[i].AfterFirst(L' ').Lower();
			numofastreams++;
			continue;
		}
		streams[i] = L"";
	}
	if (numofastreams > 1){
		long streamToChange = 0;
		int enabledSize = enabled.size();
		int lowestIndex = enabledSize;

		for (int i = 0; i < (int)streams.size(); i++){
			if (streams[i] == L""){ continue; }
			for (int j = 0; j < (int)enabledSize; j++){
				size_t result = streams[i].Lower().find(L"[" + enabled[j] + L"]");
				if (result != wxNOT_FOUND && j < lowestIndex){
					lowestIndex = j;
					streamToChange = i;
				}
			}
		}
		if (lowestIndex < enabledSize){
			if (streams[streamToChange].AfterLast(L' ') == L"0"){
				renderer->EnableStream(streamToChange);
			}
		}
	}
	
}

BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
	std::vector<RECT> * MonRects = (std::vector<RECT>*)dwData;
	WinStruct<MONITORINFO> monitorinfo;
	if (!GetMonitorInfo(hMonitor, &monitorinfo)){
		KaiLog(_("Nie można pobrać informacji o monitorze"));
		return TRUE;
	}
	// Main monitor have to be put as first
	if (monitorinfo.dwFlags == MONITORINFOF_PRIMARY){
		MonRects->insert(MonRects->begin(), monitorinfo.rcMonitor);
		return TRUE;
	}
	MonRects->push_back(monitorinfo.rcMonitor);
	return TRUE;
}

wxRect VideoCtrl::GetMonitorRect(int wmonitor){
	MonRects.clear();
	EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, (LPARAM)&MonRects);
	wxRect rt(MonRects[0].left, MonRects[0].top, abs(MonRects[0].right - MonRects[0].left), abs(MonRects[0].bottom - MonRects[0].top));
	if (wmonitor == -1 || MonRects.size() == 1){ return rt; }
	else if (wmonitor == 0){
		wxRect rect = Kai->GetRect();
		int x = (rect.width / 2) + rect.x;
		int y = (rect.height / 2) + rect.y;
		for (size_t i = 0; i < MonRects.size(); i++){
			if (MonRects[i].left <= x && x < MonRects[i].right && MonRects[i].top <= y && y < MonRects[i].bottom){
				return wxRect(MonRects[i].left, MonRects[i].top, abs(MonRects[i].right - MonRects[i].left), abs(MonRects[wmonitor].bottom - MonRects[wmonitor].top));
			}
		}
	}
	else{
		return wxRect(MonRects[wmonitor].left, MonRects[wmonitor].top, abs(MonRects[wmonitor].right - MonRects[wmonitor].left), abs(MonRects[wmonitor].bottom - MonRects[wmonitor].top));
	}
	return rt;
}

void VideoCtrl::OnChangeVisual(wxCommandEvent &evt)
{
	if (!renderer)
		return;

	EditBox *eb = tab->Edit;
	int vis = evt.GetInt();
	VideoToolbar *vTB = (m_IsFullscreen && m_FullScreenWindow) ? m_FullScreenWindow->vToolbar : m_VideoToolbar;

	if (vis == eb->Visual){ return; }
	if (vis == 0){
		renderer->RemoveVisual();
	}
	else if (vis != eb->Visual){
		if (renderer->m_HasZoom){ renderer->SetZoom(); }
		eb->Visual = vis;
		renderer->SetVisual();
		if (vis >= VECTORCLIP){ 
			renderer->VisualChangeTool(vTB->GetItemToggled()); }
		if (!HasArrow()){ SetCursor(wxCURSOR_ARROW); }
		SetFocus();
	}

}

bool VideoCtrl::SetBackgroundColour(const wxColour &col)
{
	m_VideoPanel->SetBackgroundColour(col);
	return true;
}

bool VideoCtrl::SetFont(const wxFont &font)
{
	wxWindow::SetFont(font);
	int fw;
	GetTextExtent(L"#TWFfGH", &fw, &m_ToolBarHeight);
	m_ToolBarHeight += 8;
	
	int oldPanelHeight = m_PanelHeight;
	m_PanelHeight = 30 + (m_ToolBarHeight * 2) - 8;
	wxSize size = m_VideoPanel->GetSize();
	size.y = m_PanelHeight;
	m_VideoPanel->SetSize(size);
	m_TimesTextField->SetFont(font);
	m_VideoToolbar->SetFont(font);
	m_VideoToolbar->SetHeight(m_ToolBarHeight);
	if (IsShown()){
		wxSize windowSize = GetSize();
		windowSize.y -= (oldPanelHeight - m_PanelHeight);
		tab->SetVideoWindowSizes(windowSize.x, windowSize.y, false);
	}

	if (m_FullScreenWindow){
		wxSize size1 = m_FullScreenWindow->panel->GetSize();
		size1.y = m_PanelHeight;
		m_FullScreenWindow->panel->SetSize(size1);
		m_FullScreenWindow->mstimes->SetFont(font);
		m_FullScreenWindow->Videolabel->SetFont(font);
		m_FullScreenWindow->showToolbar->SetFont(font);
		m_FullScreenWindow->vToolbar->SetFont(font);
		m_FullScreenWindow->vToolbar->SetHeight(m_ToolBarHeight);
	}
	
	return true;
}

void VideoCtrl::GetVideoSize(int *width, int *height)
{
	if (renderer){
		renderer->GetVideoSize(width, height);
	}
	else{
		*width = 0;
		*height = 0;
	}
}
wxSize VideoCtrl::GetVideoSize()
{
	wxSize size(0, 0);
	if (renderer){
		renderer->GetVideoSize(&size.x, &size.y);
	}
	return size;
}

void VideoCtrl::GetFPSAndAspectRatio(float *FPS, float *AspectRatio, int *AspectRatioX, int *AspectRatioY)
{
	if (renderer){
		if(FPS)
			*FPS = m_FPS;
		if (AspectRatio)
			*AspectRatio = m_AspectRatio;
		if (AspectRatioX)
			*AspectRatioX = m_AspectRatioX;
		if (AspectRatioX)
			*AspectRatioY = m_AspectRatioY;
	}
}

int VideoCtrl::GetDuration()
{
	if (renderer){
		return renderer->GetDuration();
	}
	return false;
}

bool VideoCtrl::OpenSubs(int flag, bool recreateFrame, bool refresh, bool resetParameters)
{
	if (renderer){
		bool success = renderer->OpenSubs(flag, recreateFrame, NULL, resetParameters);
		if (refresh){
			Render();
		}
		return success;
	}
	return false;
}
void VideoCtrl::Render(bool recreateFrame)
{
	if (renderer)
		renderer->Render(recreateFrame);
}
void VideoCtrl::UpdateVideoWindow()
{
	if (renderer)
		renderer->UpdateVideoWindow();
}
void VideoCtrl::ChangePositionByFrame(int cpos)
{
	if (renderer)
		renderer->ChangePositionByFrame(cpos);
}
bool VideoCtrl::RemoveVisual(bool noRefresh, bool disable)
{
	if (renderer)
		return renderer->RemoveVisual(noRefresh, disable);

	return false;
}
int VideoCtrl::GetFrameTime(bool start)
{
	if (renderer)
		return renderer->GetFrameTime(start);

	return 0;
}
int VideoCtrl::GetFrameTimeFromTime(int time, bool start)
{
	if (renderer)
		return renderer->GetFrameTimeFromTime(time, start);

	return 0;
}
void VideoCtrl::GetStartEndDelay(int startTime, int endTime, int *retStart, int *retEnd)
{
	if (renderer)
		renderer->GetStartEndDelay(startTime, endTime, retStart, retEnd);
}
int VideoCtrl::GetFrameTimeFromFrame(int frame, bool start)
{
	if (renderer)
		return renderer->GetFrameTimeFromFrame(frame, start);
	return 0;
}
void VideoCtrl::SetZoom()
{
	if (renderer)
		renderer->SetZoom();
}
void VideoCtrl::GoToNextKeyframe()
{
	if (renderer)
		renderer->GoToNextKeyframe();
}
void VideoCtrl::GoToPrevKeyframe()
{
	if (renderer)
		renderer->GoToPrevKeyframe();
}
void VideoCtrl::OpenKeyframes(const wxString &filename)
{
	if (renderer && renderer->HasFFMS2()) {
		renderer->GetFFMS2()->OpenKeyframes(filename);
		m_KeyframesFileName.Empty();
		return;
	}
	else if (tab->Edit->ABox) {
		// skip return when audio do not have own provider or file didn't have video for take timecodes.
		if (tab->Edit->ABox->OpenKeyframes(filename)) {
			m_KeyframesFileName.Empty();
			return;
		}
	}
	//if there is no FFMS2 or audiobox we store keyframes path;
	m_KeyframesFileName = filename;
}
void VideoCtrl::SetColorSpace(const wxString& matrix, bool render)
{
	if (renderer)
		renderer->SetColorSpace(matrix, render);
}
int VideoCtrl::GetPlayEndTime(int time)
{
	if (renderer)
		return renderer->GetPlayEndTime(time);
	return 0;
}
void VideoCtrl::DisableVisuals(bool disable)
{
	m_VideoToolbar->DisableVisuals(disable);
}
void VideoCtrl::DeleteAudioCache()
{
	if (renderer)
		renderer->DeleteAudioCache();
}
wxWindow *VideoCtrl::GetMessageWindowParent()
{
	return (m_IsFullscreen && m_FullScreenWindow) ? (wxWindow*)m_FullScreenWindow : Kai;
}
bool VideoCtrl::IsFullScreen()
{
	return m_IsFullscreen;
}
bool VideoCtrl::IsDirectShow()
{
	return m_IsDirectShow;
}
void VideoCtrl::GetVideoListsOptions(int *videoPlayAfter, int *videoSeekAfter)
{
	if (videoPlayAfter)
		*videoPlayAfter = m_VideoToolbar->videoPlayAfter->GetSelection();
	if (videoSeekAfter)
		*videoSeekAfter = m_VideoToolbar->videoSeekAfter->GetSelection();
}
void VideoCtrl::SetVisual(bool settext, bool noRefresh)
{
	if (renderer)
		renderer->SetVisual(settext, noRefresh);
}
void VideoCtrl::ResetVisual()
{
	if (renderer)
		renderer->ResetVisual();
}

bool VideoCtrl::HasFFMS2()
{
	return renderer && renderer->HasFFMS2();
}
VideoFfmpeg *VideoCtrl::GetFFMS2()
{
	if (renderer)
		return renderer->GetFFMS2();

	return NULL;
}

void VideoCtrl::SetVisualEdition(bool value)
{
	if(renderer)
		renderer->m_HasVisualEdition = value;
}

RendererVideo *VideoCtrl::GetRenderer()
{
	return renderer;
}

Fullscreen *VideoCtrl::GetFullScreenWindow()
{
	return m_FullScreenWindow;
}
VideoToolbar *VideoCtrl::GetVideoToolbar()
{
	return m_VideoToolbar;
}

void VideoCtrl::HideVideoToolbar()
{
	wxSize toolbarSize = m_VideoToolbar->GetSize();
	m_VideoToolbar->Hide();
	m_PanelHeight -= toolbarSize.y;
}

void VideoCtrl::ShowVideoToolbar()
{
	m_VideoToolbar->Show();
	wxSize toolbarSize = m_VideoToolbar->GetSize();
	m_PanelHeight += toolbarSize.y;
}

int VideoCtrl::GetPanelHeight()
{
	return m_PanelHeight;
};

//void VideoCtrl::SetPanelHeight(int panelHeight)
//{
//	m_PanelHeight = panelHeight;
//}

int VideoCtrl::GetCurrentFrame()
{
	if (renderer)
		return renderer->GetCurrentFrame();
	return 0;
}

void VideoCtrl::ChangeVobsub(bool vobsub)
{
	if (renderer)
		renderer->ChangeVobsub(vobsub);
}

void VideoCtrl::SetPanelOnFullScreen(bool value)
{
	m_PanelOnFullscreen = value;
}

void VideoCtrl::SetVideoWindowLastSize(const wxSize & size)
{
	m_VideoWindowLastSize = size;
}

bool VideoCtrl::IsOnAnotherMonitor()
{
	return m_IsOnAnotherMonitor;
}
void VideoCtrl::SaveVolume()
{
	Options.SetInt(VIDEO_VOLUME, m_VolumeSlider->GetValue());
}

bool VideoCtrl::IsMenuShown()
{
	return m_IsMenuShown;
}
const wxString & VideoCtrl::GetKeyFramesFileName()
{
	return m_KeyframesFileName;
}
void VideoCtrl::SetKeyFramesFileName(const wxString & fileName)
{
	m_KeyframesFileName = fileName;
}
BEGIN_EVENT_TABLE(VideoCtrl, wxWindow)
EVT_SIZE(VideoCtrl::OnSize)
EVT_MOUSE_EVENTS(VideoCtrl::OnMouseEvent)
EVT_KEY_DOWN(VideoCtrl::OnKeyPress)
EVT_PAINT(VideoCtrl::OnPaint)
EVT_TIMER(ID_VIDEO_TIME, VideoCtrl::OnPlaytime)
EVT_TIMER(ID_IDLE, VideoCtrl::OnIdle)
EVT_ERASE_BACKGROUND(VideoCtrl::OnErase)
EVT_BUTTON(ID_END_OF_STREAM, VideoCtrl::OnEndFile)
EVT_MOUSE_CAPTURE_LOST(VideoCtrl::OnLostCapture)
END_EVENT_TABLE()