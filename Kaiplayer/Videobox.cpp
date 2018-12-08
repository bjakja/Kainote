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

class bars1 : public KaiDialog
{
public:
	bars1(VideoCtrl *parent, float _AR);
	virtual ~bars1(){};

	KaiSlider *slider;
	KaiStaticText *actual;
	void OnSlider(wxCommandEvent &event);
	VideoCtrl *_parent;
	float AR = 1.f;
};

bars1::bars1(VideoCtrl *parent, float _AR)
	: KaiDialog((parent->isFullscreen) ? (wxWindow*)parent->TD : parent, -1, "", wxDefaultPosition, wxDefaultSize)
{
	AR = _AR;
	_parent = parent;
	DialogSizer *sizer = new DialogSizer(wxVERTICAL);
	actual = new KaiStaticText(this, -1, wxString::Format(_("Proporcje ekranu: %5.3f"), 1.f / AR));
	slider = new KaiSlider(this, 7767, AR * 700000, 100000, 1000000, wxDefaultPosition, wxSize(400, -1), wxHORIZONTAL | wxSL_INVERSE);
	Connect(7767, wxEVT_SCROLL_THUMBTRACK, (wxObjectEventFunction)&bars1::OnSlider);
	sizer->Add(actual, 0, wxALL, 3);
	sizer->Add(slider, 1, wxEXPAND | wxALL, 3);
	SetSizerAndFit(sizer);
	MoveToMousePosition(this);
}

void bars1::OnSlider(wxCommandEvent &event)
{
	_parent->SetAspectRatio(slider->GetValue() / 700000.0f);
	actual->SetLabelText(wxString::Format(_("Proporcje ekranu: %5.3f"), 1.f / AR));
}


VideoCtrl::VideoCtrl(wxWindow *parent, KainoteFrame *KaiParent, const wxSize &size)
	: wxWindow(parent, -1, wxDefaultPosition, size)
	, Kai(KaiParent)
	, hasArrow(true)
	, ismenu(false)
	, eater(false)
	, actualFile(0)
	, prevchap(-1)
	, coeffX(0.0f)
	, coeffY(0.0f)
	, TD(NULL)
	, blockpaint(false)
	, isOnAnotherMonitor(false)
	, shownKeyframe(false)
{

	panel = new wxWindow(this, -1, wxPoint(0, size.y - panelHeight), wxSize(size.x, panelHeight));
	panel->SetBackgroundColour(Options.GetColour(WindowBackground));

	vslider = new VideoSlider(panel, ID_SLIDER, wxPoint(0, 1), wxSize(size.x, 14));
	vslider->VB = this;
	bprev = new BitmapButton(panel, CreateBitmapFromPngResource("backward"), CreateBitmapFromPngResource("backward1"), PreviousVideo, _("Poprzedni plik"), wxPoint(5, 16), wxSize(26, 26));
	bpause = new BitmapButton(panel, CreateBitmapFromPngResource("play"), CreateBitmapFromPngResource("play1"), PlayPause, _("Odtwórz / Pauza"), wxPoint(40, 16), wxSize(26, 26));
	bpline = new BitmapButton(panel, CreateBitmapFromPngResource("playline"), CreateBitmapFromPngResource("playline1"), PlayActualLine, _("Odtwórz aktywną linię"), wxPoint(75, 16), wxSize(26, 26), GLOBAL_HOTKEY);
	bstop = new BitmapButton(panel, CreateBitmapFromPngResource("stop"), CreateBitmapFromPngResource("stop1"), StopPlayback, _("Zatrzymaj"), wxPoint(110, 16), wxSize(26, 26));
	bnext = new BitmapButton(panel, CreateBitmapFromPngResource("forward"), CreateBitmapFromPngResource("forward1"), NextVideo, _("Następny plik"), wxPoint(145, 16), wxSize(26, 26));

	volslider = new VolSlider(panel, ID_VOL, Options.GetInt(VideoVolume), wxPoint(size.x - 110, 17), wxSize(110, 25));
	mstimes = new KaiTextCtrl(panel, -1, "", wxPoint(180, 19), wxSize(360, -1), wxTE_READONLY);
	mstimes->SetWindowStyle(wxBORDER_NONE);
	mstimes->SetCursor(wxCURSOR_ARROW);
	mstimes->SetBackgroundColour(WindowBackground);

	vToolbar = new VideoToolbar(panel, wxPoint(0, panelHeight - 22));
	Bind(wxEVT_COMMAND_MENU_SELECTED, &VideoCtrl::OnChangeVisual, this, ID_VIDEO_TOOLBAR_EVENT);

	Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &evt){
		if (vplayer && vplayer->Visual)
			vplayer->Visual->ChangeTool(evt.GetInt());
	}, ID_VECTOR_TOOLBAR_EVENT);
	Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &evt){
		if (vplayer && vplayer->Visual)
			vplayer->Visual->ChangeTool(evt.GetInt());
	}, ID_MOVE_TOOLBAR_EVENT);
	Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &evt){
		if (vplayer && vplayer->Visual)
			vplayer->Visual->ChangeTool(evt.GetInt());
	}, ID_SCALE_ROTATE_TOOLBAR_EVENT);
	Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &evt){
		RefreshTime();
	}, 23334);

	Connect(PreviousVideo, NextVideo, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&VideoCtrl::OnAccelerator);
	Connect(PlayPause, StopPlayback, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&VideoCtrl::OnAccelerator);
	Connect(ID_VOL, wxEVT_COMMAND_SLIDER_UPDATED, (wxObjectEventFunction)&VideoCtrl::OnVolume);

	vtime.SetOwner(this, idvtime);
	idletime.SetOwner(this, ID_IDLE);

}

VideoCtrl::~VideoCtrl()
{
}

bool VideoCtrl::Play()
{
	wxMutexLocker lock(vbmutex);
	if (!vplayer && !vplayer->Play()){ return false; }
	int ms = (isFullscreen && !TD->panel->IsShown()) ? 1000 : 100;
	vtime.Start(ms);
	ChangeButtonBMP(false);
	return true;
}

void VideoCtrl::PlayLine(int start, int end)
{
	//wxMutexLocker lock(vbmutex);
	if (!vplayer && !vplayer->PlayLine(start, end)){ return; }
	int ms = (isFullscreen && !TD->panel->IsShown()) ? 1000 : 100;
	vtime.Start(ms);
	ChangeButtonBMP(false);
}

bool VideoCtrl::Pause(bool skipWhenOnEnd)
{
	wxMutexLocker lock(vbmutex);

	if (!HasVideo()){
		MenuItem *index = Kai->Menubar->FindItem(VideoIndexing);
		if (index->IsChecked() && index->IsEnabled()){
			EditBox *eb = Kai->GetTab()->Edit;
			if (eb->ABox){
				eb->ABox->audioDisplay->Play(eb->line->Start.mstime, eb->line->End.mstime);
				return true;
			}
			return false;
		}
		LoadVideo(Kai->videorec[Kai->videorec.size() - 1], NULL);
		return true;
	}

	if (!vplayer->Pause(skipWhenOnEnd)){ return false; }
	if (GetState() == Paused){
		vtime.Stop(); RefreshTime();
	}
	else if (GetState() == Playing){
		int ms = (isFullscreen && !TD->panel->IsShown()) ? 1000 : 100;
		vtime.Start(ms);
	}
	ChangeButtonBMP(!(GetState() == Playing));

	return true;
}

bool VideoCtrl::Stop()
{
	wxMutexLocker lock(vbmutex);

	if (!vplayer && !vplayer->Stop()){ return false; }
	vtime.Stop();
	Seek(0);
	RefreshTime();

	ChangeButtonBMP(true);

	return true;
}

bool VideoCtrl::LoadVideo(const wxString& fileName, wxString *subsName, bool fulls /*= false*/, bool changeAudio, int customFFMS2)
{
	if (fulls){ SetFullscreen(); }
	prevchap = -1;
	bool byFFMS2;
	if (customFFMS2 == -1){
		MenuItem *index = Kai->Menubar->FindItem(VideoIndexing);
		byFFMS2 = index->IsChecked() && index->IsEnabled() && !fulls && !isFullscreen;
	}
	else
		byFFMS2 = customFFMS2 == 1;


	bool shown = true;
	// removed block, check if it not crash or something
	// here we need to set vplayer class
	if (!VideoPlayer::Get(&vplayer, this, fileName, subsName, byFFMS2, !Kai->GetTab()->editor, changeAudio)){
		delete subsName;
		if (!byFFMS2){ KaiMessageBox(_("Plik nie jest poprawnym plikiem wideo albo jest uszkodzony,\nbądź brakuje kodeków czy też splittera"), _("Uwaga")); }
		return false;
	}
	if (!(IsShown() || (TD && TD->IsShown()))){
		shown = false; Show();
	}

	eater = IsDshow;

	TabPanel *tab = (TabPanel*)GetParent();
	if (!isFullscreen&&!fulls){
		int sx, sy;
		//wyłączony edytor
		if (!tab->editor){
			if (!Kai->IsMaximized()){
				int sizex, sizey;
				Kai->GetSize(&sizex, &sizey);
				CalcSize(&sx, &sy, 0, 0, true, true);
				sx += Kai->borders.left + Kai->borders.right;
				sy += (panelHeight + Kai->borders.bottom + Kai->borders.top);
				if (sx == sizex && sy == sizey){
					vplayer->UpdateVideoWindow();
				}
				else{
					Kai->SetClientSize(sx, sy);
					tab->BoxSizer1->Layout();
				}
			}
			//załączony edytor
		}
		else{
			int kw, kh;
			Options.GetCoords(VideoWindowSize, &kw, &kh);
			bool ischanged = CalcSize(&sx, &sy, kw, kh, true, true);
			if (ischanged || !shown){
				SetMinSize(wxSize(sx, sy + panelHeight));
				tab->BoxSizer1->Layout();
			}
			Options.SetCoords(VideoWindowSize, sx, sy + panelHeight);
		}

	}
	if (isFullscreen){
		vplayer->UpdateVideoWindow();
		wxSize size = vplayer->GetVideoSize();
		Kai->SetVideoResolution(size.x, size.y, !Options.GetBool(DontAskForBadResolution));
	}
	//block = false;
	if (IsDshow){
		Play();
		if (tab->editor && !isFullscreen){ Pause(); }
		if (!volslider->IsShown()){ volslider->Show(); mstimes->SetSize(lastSize.x - 290, -1); }
	}
	else{
		//block = false;
		if (volslider->IsShown()){
			volslider->Show(false);
			mstimes->SetSize(lastSize.x - 185, -1);
		}
		vplayer->Render(true);
	}

	RefreshTime();
	if (IsDshow){
		int pos = Options.GetInt(VideoVolume);
		vplayer->SetVolume(-(pos*pos));
	}
	//SetFocus();
	tab->VideoPath = fileName;
	tab->VideoName = fileName.AfterLast(L'\\');
	Kai->SetStatusText(tab->VideoName, 8);
	if (TD){ TD->Videolabel->SetLabelText(tab->VideoName); }
	if (!tab->editor){ Kai->Label(0, true); }
	Kai->SetStatusText(getfloat(vplayer->fps) + L" FPS", 4);
	wxString tar;
	tar << vplayer->ax << L" : " << vplayer->ay;
	Kai->SetStatusText(tar, 6);
	STime duration;
	duration.mstime = vplayer->GetDuration();
	Kai->SetStatusText(duration.raw(SRT), 3);
	Kai->SetRecent(1);
	
	if (tab->editor && (!isFullscreen || IsShown()) &&
		tab->SubsPath != L"" && Options.GetBool(OpenVideoAtActiveLine)){
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
	return (vplayer)? vplayer->vstate : None;
}

bool VideoCtrl::Seek(int whre, bool starttime/*=true*/, bool disp/*=true*/, bool reloadSubs/*=true*/, bool correct /*= true*/)
{
	wxMutexLocker lock(vbmutex);
	if (!HasVideo()){ return false; }
	vplayer->SetPosition(whre, starttime, correct);
	return true;
}

int VideoCtrl::Tell()
{
	return (vplayer)? vplayer->GetCurrentPosition() : 0;
}


void VideoCtrl::OnSize(wxSizeEvent& event)
{
	wxSize asize = GetClientSize();
	if (lastSize == asize){ return; }
	lastSize = asize;
	panel->SetSize(0, asize.y - panelHeight, asize.x, panelHeight);
	vslider->SetSize(wxSize(asize.x, 14));
	volslider->SetPosition(wxPoint(asize.x - 110, 17));
	int difSize = (volslider->IsShown()) ? 290 : 185;
	mstimes->SetSize(asize.x - difSize, -1);
	vToolbar->SetSize(asize.x, 22);
	if (vplayer){
		vplayer->UpdateVideoWindow();
	}
	else{
		Refresh(false);
	}
}


void VideoCtrl::OnMouseEvent(wxMouseEvent& event)
{

	if (event.ButtonDown())
	{
		SetFocus();
		if (ismenu){ ismenu = false; }
	}
	if (!vplayer){ return; }
	if (vplayer->hasZoom){
		vplayer->ZoomMouseHandle(event);
		return;
	}
	x = event.GetX(); 
	y = event.GetY();
	TabPanel *tab = Kai->GetTab();
	if (event.GetWheelRotation() != 0) {

		if (event.ControlDown() && !isFullscreen){
			int step = event.GetWheelRotation() / event.GetWheelDelta();

			int w, h, mw, mh;
			GetClientSize(&w, &h);
			GetParent()->GetClientSize(&mw, &mh);
			int incr = h + (step * 20);
			if (incr >= mh){ incr = mh - 3; }
			if (y < h - panelHeight){
				if (h <= 350 && step < 0 || h == incr){ return; }
				int ww, hh;
				CalcSize(&ww, &hh, w, incr, false, true);
				SetMinSize(wxSize(ww, hh + panelHeight));
				tab->Edit->SetMinSize(wxSize(-1, hh + panelHeight));
				Options.SetCoords(VideoWindowSize, ww, hh + panelHeight);
				tab->BoxSizer1->Layout();
				if (event.ShiftDown()){
					tab->SetVideoWindowSizes(w, incr);
				}
			}
			return;
		}
		else if (!vplayer->Visual){
			if (!IsDshow){
				AudioBox *box = tab->Edit->ABox;
				if (box){
					int vol = box->GetVolume();
					int step = event.GetWheelRotation() / event.GetWheelDelta();
					vol += (step * 3);
					box->SetVolume(vol);
				}
			}
			else if (isFullscreen){ TD->volslider->OnMouseEvent(event); }
			else{ volslider->OnMouseEvent(event); }
			return;
		}
	}


	if (vplayer->Visual){
		vplayer->Visual->OnMouseEvent(event); 
		if (!hasArrow){ SetCursor(wxCURSOR_ARROW); hasArrow = true; }
		return;
	}//jak na razie 


	if (event.LeftDClick() && event.GetModifiers() == 0){
		SetFullscreen();
		if (!isFullscreen && tab->SubsPath != "" && Options.GetBool(SelectVisibleLineAfterFullscreen)){
			tab->Edit->Send(EDITBOX_LINE_EDITION, false);
			tab->Grid->SelVideoLine();
		}
		int w, h;
		GetClientSize(&w, &h);

		if (y >= h - panelHeight && !hasArrow){
			SetCursor(wxCURSOR_ARROW); hasArrow = true;
		}
		return;
	}

	if (isFullscreen){
		if (eater && event.Moving() && !event.ButtonDown()){ Sleep(200); eater = false; return; }
		if (!hasArrow){ TD->SetCursor(wxCURSOR_ARROW); hasArrow = true; }

		int w, h;
		TD->GetClientSize(&w, &h);
		if (y >= h - panelHeight && !TD->panel->IsShown()){ vtime.Start(100); TD->panel->Show(); }
		else if (y < h - panelHeight && TD->panel->IsShown() && !panelOnFullscreen){ 
			vtime.Start(1000); 
			TD->panel->Show(false); 
			SetFocus(); }
		if (!TD->panel->IsShown() && !ismenu){ idletime.Start(1000, true); }
	}
	else if (tab->editor){
		int w, h;
		GetClientSize(&w, &h);
		if (hasArrow && y < (h - panelHeight)){ SetCursor(wxCURSOR_BLANK); hasArrow = false; }

		if (event.Leaving()){
			if (vplayer->cross){
				vplayer->cross = false;
				if (!hasArrow){ SetCursor(wxCURSOR_ARROW); hasArrow = true; }
				if (GetState() == Paused && !vplayer->block){ vplayer->Render(false); }
			}
			return;
		}

		if (event.Entering()){
			int nx = 0, ny = 0;
			tab->Grid->GetASSRes(&nx, &ny);
			coeffX = (float)nx / (float)(w - 1);
			coeffY = (float)ny / (float)(h - panelHeight - 1);

		}
		int posx = (float)x * coeffX;
		int posy = (float)y * coeffY;
		vplayer->coords = "";
		vplayer->coords << posx << ", " << posy;
		vplayer->DrawLines(wxPoint(x, y));
	}
	else if (!hasArrow){ SetCursor(wxCURSOR_ARROW); hasArrow = true; }



	if (Options.GetBool(VideoPauseOnClick) && event.LeftUp() && !event.ControlDown()){
		Pause();
	}


	if (event.RightUp()) {
		ContextMenu(event.GetPosition());
		return;
	}

	if (event.MiddleDown() || (event.LeftDown() && event.ControlDown())){
		//OpenEditor();
		if (!isFullscreen)
		{
			Dialogue *aline = tab->Edit->line;
			bool istl = (tab->Grid->hasTLMode && aline->TextTl != L"");
			wxString ltext = (istl) ? aline->TextTl : aline->Text;
			wxRegEx posmov(L"\\\\(pos|move)([^\\\\}]+)", wxRE_ADVANCED);
			posmov.ReplaceAll(&ltext, L"");

			wxString postxt;
			float posx = (float)x * coeffX;
			float posy = (float)y * coeffY;
			postxt = L"\\pos(" + getfloat(posx) + L"," + getfloat(posy) + L")";
			if (ltext.StartsWith(L"{")){
				ltext.insert(1, postxt);
			}
			else{
				ltext = L"{" + postxt + L"}" + ltext;
			}
			if (istl){ aline->TextTl = ltext; }
			else{ aline->Text = ltext; }
			tab->Grid->ChangeCell((istl) ? TXTTL : TXT, tab->Grid->currentLine, aline);
			tab->Grid->Refresh(false);
			tab->Grid->SetModified(VISUAL_POSITION);
		}
	}


}



void VideoCtrl::OnPlaytime(wxTimerEvent& event)
{
	RefreshTime();
}

void VideoCtrl::OnKeyPress(wxKeyEvent& event)
{
	if (!vplayer)
		return;

	int key = event.GetKeyCode();
	if (key == L'F'){ SetFullscreen(); }
	else if (key == WXK_WINDOWS_MENU){
		wxWindow *owner = (isFullscreen && TD) ? (wxWindow *)TD : this;
		wxPoint poss = owner->ScreenToClient(wxGetMousePosition());
		ContextMenu(poss);
	}
	else if ((key == L'B' || key == WXK_ESCAPE) && isFullscreen){
		SetFullscreen();
		//if(Kai->GetTab()->SubsPath!=L""){
		//Kai->GetTab()->Grid->SelVideoLine();}
		if (Kai->GetTab()->SubsPath != L"" && Options.GetBool(SelectVisibleLineAfterFullscreen)){
			Kai->GetTab()->Edit->Send(EDITBOX_LINE_EDITION, false);
			Kai->GetTab()->Grid->SelVideoLine();
		}
		if (key == L'B'){ 
			if (GetState() == Playing){ Pause(); }
			ShowWindow(Kai->GetHWND(), SW_SHOWMINNOACTIVE); 
		}
	}
	//probably made for fullscreen
	else if (key == L'S' && event.m_controlDown){ Kai->Save(false); }
	else if (key == WXK_RETURN && vplayer->hasZoom){
		vplayer->SetZoom();
	}
	else if (key == L'Z' && event.ControlDown() && event.ShiftDown()){
		vplayer->ResetZoom();
	}
	else if (vplayer->Visual){
		vplayer->Visual->OnKeyPress(event);
	}
}


void VideoCtrl::OnIdle(wxTimerEvent& event)
{
	if (isFullscreen && !TD->panel->IsShown() && !ismenu){
		TD->SetCursor(wxCURSOR_BLANK);
		eater = IsDshow;
		hasArrow = false;
	}
}



void VideoCtrl::NextFile(bool next)
{
	wxMutexLocker lock(nextmutex);
	wxString path;
	if (Kai->GetTab()->VideoPath != L""){
		path = Kai->GetTab()->VideoPath;
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
	if (next && actualFile >= (int)files.GetCount() - 1){ Seek(0); Pause(false); actualFile = files.GetCount() - 1; return; }
	else if (!next && actualFile <= 0){ Seek(0); Pause(false); actualFile = 0; return; }

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
				if (isFullscreen){ SetFocus(); }
				return;
			}
			else if (!IsDshow){ return; }
		}
		if (next){ k++; }
		else{ k--; }
	}
	Seek(0);
	Pause(false);
}

void VideoCtrl::SetFullscreen(int monitor)
{
	//wxMutexLocker lock(vbmutex);
	isFullscreen = !isFullscreen;

	//wyjście z fullskreena
	if (!isFullscreen){

		if (GetState() == Playing){ if (Kai->GetTab()->editor){ Pause(); } else{ vtime.Start(100); } }
		if (TD->HasCapture()){ TD->ReleaseMouse(); }

		int sx, sy, sizex, sizey;

		if (!Kai->GetTab()->editor){
			if (!Kai->IsMaximized()){
				Kai->GetClientSize(&sizex, &sizey);
				int yDiff = panelHeight + Kai->borders.bottom + Kai->borders.top;
				int xDiff = Kai->borders.left + Kai->borders.right;
				CalcSize(&sx, &sy, sizex - xDiff, sizey - yDiff);
				Kai->SetClientSize(sx + xDiff, sy + yDiff);
			}
		}
		else{
			Options.GetCoords(VideoWindowSize, &sizex, &sizey);
			CalcSize(&sx, &sy, sizex, sizey);
			SetMinSize(wxSize(sx, sy + panelHeight));
			Kai->GetTab()->BoxSizer1->Layout();
		}
		volslider->SetValue(Options.GetInt(VideoVolume));

		if (!IsShown()){
			isOnAnotherMonitor = false;
			Show(); GetParent()->Layout();
		}
		vplayer->UpdateVideoWindow();
		vplayer->block = true;
		vplayer->Render();
		vplayer->block = false;

		vToolbar->Synchronize(TD->vToolbar);
		RefreshTime();
		TD->Hide();
		SetCursor(wxCURSOR_ARROW); hasArrow = true;
	}
	//przejście na fullskreena
	else{
		if (wxWindow::HasCapture()){ wxWindow::ReleaseMouse(); }
		wxRect rt = GetMonitorRect(monitor);
		if (!TD){
			TD = new Fullscreen(this, rt.GetPosition(), rt.GetSize());
			TD->Videolabel->SetLabelText(Kai->GetTab()->VideoName);
		}
		else{
			TD->SetPosition(rt.GetPosition());
			TD->SetSize(rt.GetSize());
		}
		TD->OnSize();
		TD->volslider->SetValue(Options.GetInt(VideoVolume));
		TD->vToolbar->Synchronize(vToolbar);
		if (!panelOnFullscreen){ TD->panel->Hide(); }
		TD->Show();
		vplayer->block = true;
		vplayer->UpdateVideoWindow();
		vplayer->block = false;
		RefreshTime();
		if (GetState() == Playing && !TD->panel->IsShown()){ vtime.Start(1000); }
		if (!Kai->GetTab()->editor)
			TD->HideToolbar(true);
		if (monitor && Kai->GetTab()->editor){
			Hide();
			GetParent()->Layout();
			isOnAnotherMonitor = true;
		}
		else{
			SetFocus();
		}
	}
	ChangeButtonBMP(!(GetState() == Playing));


}

bool VideoCtrl::CalcSize(int *width, int *height, int wwidth, int wheight, bool setstatus, bool calcH)
{
	if (!vplayer)
		return;

	wxSize size = vplayer->GetVideoSize();
	if (setstatus){
		Kai->SetVideoResolution(size.x, size.y, !Options.GetBool(DontAskForBadResolution));
	}
	if (wwidth == 0){
		GetClientSize(&wwidth, &wheight);
	}
	float precy = size.y, precx = size.x;
	if (!calcH){
		size.x = wwidth;
		if (vplayer->AR <= 0){ size.y *= (wwidth / precx); }
		else{ size.y = size.x * vplayer->AR; }
	}
	if (calcH || size.y > 700){
		wheight -= panelHeight;
		size.y = wheight;
		if (vplayer->AR > 0){ size.x = size.y / vplayer->AR; }
		else{ size.x *= (wheight / precy); }
	}
	*width = size.x;
	*height = size.y;
	return !(size.x == wwidth && size.y == wheight);
}

void VideoCtrl::OnPrew()
{
	MenuItem *index = Kai->Menubar->FindItem(VideoIndexing);
	if (index->IsChecked() && index->IsEnabled() && !isFullscreen){
		if (KaiMessageBox(_("Czy na pewno chcesz zindeksować poprzednie wideo?"), _("Potwierdzenie"), wxYES_NO) == wxNO)return;
	}
	NextFile(false);
}


void VideoCtrl::OnNext()
{
	MenuItem *index = Kai->Menubar->FindItem(VideoIndexing);
	if (index->IsChecked() && index->IsEnabled() && !isFullscreen){
		if (KaiMessageBox(_("Czy na pewno chcesz zindeksować następne wideo?"), _("Potwierdzenie"), wxYES_NO) == wxNO)return;
	}
	NextFile();
}

void VideoCtrl::OnVolume(wxScrollEvent& event)
{
	int pos = event.GetPosition();
	Options.SetInt(VideoVolume, pos);
	vplayer->SetVolume(-(pos*pos));
}

void VideoCtrl::ContextMenu(const wxPoint &pos)
{
	ismenu = true;
	Menu* menu = new Menu();
	bool editor = ((TabPanel*)GetParent())->editor;
	wxString txt = L"\t" + Hkeys.GetStringHotkey(PlayPause);
	if (GetState() != Playing){ txt.Prepend(_("Odtwórz")); }
	else if (GetState() == Playing){ txt.Prepend(_("Pauza")); }
	if (!isFullscreen && editor)
	{
		menu->SetAccMenu(CopyCoords, _("Kopiuj pozycję na wideo"));
	}
	menu->Append(PlayPause, txt)->Enable(vplayer);
	menu->SetAccMenu(StopPlayback, _("Zatrzymaj"))->Enable(GetState() == Playing);
	wxString txt1;
	if (!isFullscreen){ txt1 = _("Pełny ekran\tF"); }
	else{ txt1 = _("Wyłącz pełny ekran\tEscape"); }
	MenuItem *Item = menu->Append(FullScreen, txt1);
	Item->Enable(vplayer);
	Item->DisableMapping();
	GetMonitorRect(-1);
	for (size_t i = 1; i < MonRects.size(); i++)
	{
		wxString txt2;
		if (isFullscreen){ txt2 = wxString::Format(_("Przełącz pełny ekran na %i monitor"), (int)(i + 1)); }
		else{ txt2 = wxString::Format(_("Włącz pełny ekran na %i monitorze"), (int)(i + 1)); }
		menu->Append(MENU_MONITORS + i, txt2)->Enable(vplayer);
	}

	menu->SetAccMenu(Editor, _("Otwórz edytor"))->Enable(isFullscreen);
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
	menu->SetAccMenu(OpenVideo, _("Otwórz wideo"));

	menu->SetAccMenu(Id::OpenSubs, _("Otwórz napisy"));
	menu->SetAccMenu(HideProgressBar, _("Ukryj / pokaż pasek postępu"))->Enable(isFullscreen);
	menu->SetAccMenu(AspectRatio, _("Zmień proporcje wideo"));
	menu->SetAccMenu(SubbedFrameToPNG, _("Zapisz klatkę z napisami jako PNG"))->Enable(GetState() == Paused);
	menu->SetAccMenu(SubbedFrameToClipboard, _("Kopiuj klatkę z napisami do schowka"))->Enable(GetState() == Paused);
	menu->SetAccMenu(FrameToPNG, _("Zapisz klatkę jako PNG"))->Enable(GetState() == Paused && ((TabPanel*)GetParent())->editor);
	menu->SetAccMenu(FrameToClipboard, _("Kopiuj klatkę do schowka"))->Enable(GetState() == Paused && ((TabPanel*)GetParent())->editor);
	menu->AppendSeparator();

	menu->SetAccMenu(DeleteVideo, _("Usuń plik wideo"))->Enable(GetState() != None);

	Menu* menu3 = NULL;
	int numfilters = 0;
	wxArrayString streams;
	if (vplayer && IsDshow){
		menu3 = new Menu();
		vplayer->EnumFilters(menu3);
		numfilters = menu3->GetMenuItemCount();
		menu->Append(23456, _("Filtry"), menu3, _("Wyświetla użyte filtry"));



		streams = vplayer->GetStreams();
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
		for (size_t j = 0; j < chapters.size(); j++){
			if (j == 0){ menu->AppendSeparator(); }
			timee.NewTime(chapters[j].time);
			int ntime = (j >= chapters.size() - 1) ? INT_MAX : chapters[(j + 1)].time;
			menu->Append(MENU_CHAPTERS + j, chapters[j].name + L"\t[" + timee.raw() + L"]", L"", true, 0, 0, (ntime >= Tell()) ? ITEM_RADIO : ITEM_NORMAL);
		}
	}
	id = 0;
	int Modifiers = 0;
	menu->SetMaxVisible(40);
	//ismenu=true;
	if (isFullscreen){
		id = menu->GetPopupMenuSelection(pos, TD, &Modifiers, true);
	}
	else{
		id = menu->GetPopupMenuSelection(pos, this, &Modifiers, true);
	}
	//ismenu=false;
	if ((Modifiers == wxMOD_SHIFT) && id < 2100 && id >= 2000){
		Hkeys.OnMapHkey(id, L"", this, VIDEO_HOTKEY);
		delete menu;
		ismenu = false;
		return;
	}

	if (id == CopyCoords){ OnCopyCoords(pos); }
	else if (id > 1999 && id < MENU_STREAMS){
		wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, id);
		OnAccelerator(evt);
	}
	else if (id >= 30000 && id < 30020){ Kai->OpenFile(Kai->subsrec[id - 30000]); }
	else if (id >= 30020 && id < 30040){ Kai->OpenFile(Kai->videorec[id - 30020]); }
	else if (id >= MENU_STREAMS && id < MENU_STREAMS + (int)streams.size()){
		int wstream = id - MENU_STREAMS;
		vplayer->EnableStream((long)wstream);
	}
	else if (id >= MENU_CHAPTERS && id < MENU_CHAPTERS + (int)chapters.size()){
		Seek(chapters[id - MENU_CHAPTERS].time);
	}
	else if (id >= 13000 && id < 13000 + numfilters && menu3){
		MenuItem *item = menu3->FindItem(id);
		vplayer->FilterConfig(item->GetLabel(), id - 13000, pos);
	}
	else if (id > 15000 && id < 15000 + (int)MonRects.size()){
		isFullscreen = false;
		SetFullscreen(id - 15000);
	}
	delete menu;
	ismenu = false;
}


void VideoCtrl::OnHideProgressBar()
{
	bool pb = !Options.GetBool(VideoProgressBar);
	Options.SetBool(VideoProgressBar, pb);
	if (pb){ vplayer->pbar = true; RefreshTime(); }
	else{ vplayer->pbar = false; }
	if (GetState() == Paused){ vplayer->Render(false); }
}

void VideoCtrl::OnDeleteVideo()
{
	if (KaiMessageBox(_("Czy na pewno chcesz przenieść wczytany plik wideo do kosza?"), _("Usuwanie"), wxYES_NO) == wxNO){ return; }
	wxString path = Kai->GetTab()->VideoPath;
	NextFile();
	CRecycleFile x;
	x.Recycle(path.data());
}

void VideoCtrl::OnOpenVideo()
{
	wxFileDialog* FileDialog2 = new wxFileDialog(this, _("Wybierz plik wideo"),
		(Kai->videorec.size() > 0) ? Kai->videorec[Kai->videorec.size() - 1].BeforeLast(L'\\') : L"",
		L"", _("Pliki wideo(*.avi),(*.mkv),(*.mp4),(*.ogm),(*.wmv),(*.asf),(*.rmvb),(*.rm),(*.3gp),(*.avs)|*.avi;*.mkv;*.mp4;*.ogm;*.wmv;*.asf;*.rmvb;*.rm;*.3gp;*.avs|Wszystkie pliki (*.*)|*.*"),
		wxFD_DEFAULT_STYLE, wxDefaultPosition, wxDefaultSize, L"wxFileDialog");
	if (FileDialog2->ShowModal() == wxID_OK){
		Kai->OpenFile(FileDialog2->GetPath());
	}
	FileDialog2->Destroy();
}

void VideoCtrl::OnOpenSubs()
{
	if (Kai->SavePrompt(2)){ return; }
	wxFileDialog* FileDialog2 = new wxFileDialog(Kai, _("Wybierz plik napisów"),
		(Kai->subsrec.size() > 0) ? Kai->subsrec[Kai->subsrec.size() - 1].BeforeLast(L'\\') : L"", L"",
		_("Pliki napisów (*.ass),(*.sub),(*.txt)|*.ass;*.sub;*.txt"),
		wxFD_DEFAULT_STYLE, wxDefaultPosition, wxDefaultSize, L"wxFileDialog");

	if (FileDialog2->ShowModal() == wxID_OK){
		Kai->OpenFile(FileDialog2->GetPath());
	}
	FileDialog2->Destroy();
}

void VideoCtrl::OpenEditor(bool esc)
{

	if (isFullscreen){
		if (GetState() == Playing){ Pause(); }
		//if(Kai->GetTab()->SubsPath.BeforeLast(L'.')!=Kai->GetTab()->VideoPath.BeforeLast(L'.'))
		//{wxString fn=Kai->FindFile(Kai->GetTab()->VideoPath,false,false);
		//bool isgood=false;
		//if(fn!=""){bool isgood=Kai->OpenFile(fn);}
		//}
		Options.SetBool(EditorOn, true);
		if (Kai->GetTab()->SubsPath != L""){
			Kai->GetTab()->Grid->SelVideoLine();
		}


		SetFullscreen();

		if (!esc){ ShowWindow(Kai->GetHWND(), SW_MINIMIZE); }
	}

}


void VideoCtrl::OnAccelerator(wxCommandEvent& event)
{

	id = event.GetId();
	if (id == PlayPause){ Pause(); }
	else if (id == Minus5Second){ Seek(Tell() - 5000); }
	else if (id == Plus5Second){ Seek(Tell() + 5000); }
	else if (id == MinusMinute){ Seek(Tell() - 60000); }
	else if (id == PlusMinute){ Seek(Tell() + 60000); }
	else if (id == PreviousVideo){ OnPrew(); }
	else if (id == NextVideo){ OnNext(); }
	else if (id == VolumePlus){ OnVolumePlus(); }
	else if (id == VolumeMinus){ OnVolumeMinus(); }
	else if (id == PreviousChapter){ PrevChap(); }
	else if (id == NextChapter){ NextChap(); }
	else if (id == CopyCoords){ wxPoint pos = wxGetMousePosition(); pos = ScreenToClient(pos); OnCopyCoords(pos); }
	else if (id == StopPlayback){
		if (!Kai->GetTab()->editor){ Stop(); }
		else{
			if (GetState() == Playing){
				Pause();
			}
			if (IsDshow){ Seek(0); }
		}
	}
	else if (id == FullScreen){ SetFullscreen(); }
	else if (id == Editor){ OpenEditor(); }
	else if (id == OpenVideo){ OnOpenVideo(); }
	else if (id == Id::OpenSubs){ OnOpenSubs(); }
	else if (id == HideProgressBar){ OnHideProgressBar(); }
	else if (id == AspectRatio){
		bars1 changear(this, vplayer->AR);
		changear.ShowModal();
	}
	else if (id == DeleteVideo){ OnDeleteVideo(); }
	else if (id >= FrameToPNG && id <= SubbedFrameToClipboard && GetState() == Paused){
		CColorSpaceConverter conv(vplayer->vformat, vplayer->vwidth, vplayer->vheight);
		bool del = false;
		byte *framebuf = vplayer->GetFramewithSubs(id > FrameToClipboard, &del);
		if (id == FrameToPNG || id == SubbedFrameToPNG){
			TabPanel *pan = Notebook::GetTab();
			wxString path;
			int num = 1;
			wxArrayString paths;
			wxString filespec;
			wxString dirpath = pan->VideoPath.BeforeLast(L'\\', &filespec);
			wxDir kat(dirpath);
			path = pan->VideoPath;
			if (kat.IsOpened()){
				kat.GetAllFiles(dirpath, &paths, filespec.BeforeLast(L'.') << L"_*_*.png", wxDIR_FILES);
			}
			for (wxString &file : paths){
				if (file.find(L"_" + std::to_string(num) + L"_") == wxNOT_FOUND){
					break;
				}
				num++;
			}
			path = pan->VideoPath.BeforeLast(L'.');
			STime currentTime;
			currentTime.mstime = vplayer->time;
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


void VideoCtrl::OnVolumeMinus()
{
	if (IsDshow){
		int pos = volslider->GetValue() - 2;
		if (pos > (-91)){
			vplayer->SetVolume(-(pos*pos));
			volslider->SetValue(pos);
			if (TD){ TD->volslider->SetValue(pos); }
			Options.SetInt(VideoVolume, pos);
		}
	}
	else{
		AudioBox *box = Kai->GetTab()->Edit->ABox;
		if (box){
			int vol = box->GetVolume();
			vol -= 2;
			if (vol >= 0){
				box->SetVolume(vol);
			}
		}
	}
}

void VideoCtrl::OnVolumePlus()
{
	if (!vplayer)
		return;
	if (IsDshow){
		int pos = volslider->GetValue() + 2;
		if (pos < 1){
			vplayer->SetVolume(-(pos * pos));
			volslider->SetValue(pos);
			if (TD){ TD->volslider->SetValue(pos); }
			Options.SetInt(VideoVolume, pos);
		}
	}
	else{

		AudioBox *box = Kai->GetTab()->Edit->ABox;
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
	if (!vplayer){
		int x, y;
		GetClientSize(&x, &y);
		wxPaintDC dc(this);
		dc.SetBrush(wxBrush("#000000"));
		dc.SetPen(wxPen("#000000"));
		dc.DrawRectangle(0, 0, x, y);
		wxFont font1(72, wxSWISS, wxFONTSTYLE_NORMAL, wxNORMAL, false, L"Tahoma");
		dc.SetFont(font1);
		wxSize size = dc.GetTextExtent(L"KaiNote");
		dc.SetTextForeground("#2EA6E2");
		dc.DrawText(L"KaiNote", (x - size.x) / 2, (y - size.y - panelHeight) / 2);
	}else if (!vplayer->block && vplayer->vstate == Paused){
		vplayer->Render(true);
	}

}

void VideoCtrl::OnEndFile()
{
	if ((!Kai->GetTab()->editor || isFullscreen) && IsDshow){ NextFile(); }
	else if (vplayer && vplayer->vstate == Playing){ Pause(false); }
}

void VideoCtrl::SetAspectRatio(float _AR)
{
	vplayer->AR = _AR;
	TabPanel *tab = ((TabPanel*)GetParent());
	if (tab->editor && !isFullscreen){
		int ww, hh;
		CalcSize(&ww, &hh, 0, 0, false, true);
		SetMinSize(wxSize(ww, hh + panelHeight));
		tab->BoxSizer1->Layout();
	}
	vplayer->UpdateVideoWindow();
	if (GetState() == Paused){ vplayer->Render(false); }
}

void VideoCtrl::SetScaleAndZoom()
{
	if (!vplayer)
		return;

	wxString scale;
	wxSize wsize = GetSize();
	scale << (int)((wsize.x / (float)vplayer->vwidth) * 100) << L"%";
	Kai->SetStatusText(scale, 1);
	wxString zoom;
	zoom << (int)(vplayer->zoomParcent * 100) << L"%";
	Kai->SetStatusText(zoom, 2);
}

void VideoCtrl::ChangeOnScreenResolution(TabPanel *tab)
{
	if (!vplayer || !vplayer->cross)
		return;

	int nx = 0, ny = 0;
	tab->Grid->GetASSRes(&nx, &ny);
	int w = 0, h = 0;
	GetClientSize(&w, &h);
	coeffX = (float)nx / (float)(w - 1);
	coeffY = (float)ny / (float)(h - panelHeight - 1);
	int posx = (float)x * coeffX;
	int posy = (float)y * coeffY;
	vplayer->coords = L"";
	vplayer->coords << posx << L", " << posy;
	vplayer->DrawLines(wxPoint(x, y));
}

void VideoCtrl::RefreshTime()
{
	if (!vplayer)
		return;

	STime videoTime;
	videoTime.mstime = vplayer->time;
	float dur = vplayer->GetDuration();
	float val = (dur > 0) ? videoTime.mstime / dur : 0.0;

	if (isFullscreen){
		TD->vslider->SetValue(val);
		if (TD->panel->IsShown()){
			wxString times;
			times << videoTime.raw(SRT) << L";  ";
			TabPanel *pan = (TabPanel*)GetParent();
			if (!IsDshow){
				times << vplayer->numframe << L";  ";
				if (VFF){
					if (VFF->KeyFrames.Index(time) != -1){
						shownKeyframe = true;
						TD->mstimes->SetForegroundColour(WindowWarningElements);
					}
					else if (shownKeyframe){
						shownKeyframe = false;
						TD->mstimes->SetForegroundColour(WindowText);
					}
				}
			}
			if (pan->editor){
				Dialogue *line = pan->Edit->line;
				int sdiff = videoTime.mstime - ZEROIT(line->Start.mstime);
				int ediff = videoTime.mstime - ZEROIT(line->End.mstime);
				times << sdiff << L" ms, " << ediff << L" ms";
			}
			TD->mstimes->SetValue(times);
			TD->mstimes->Update();
		}
		if (!vplayer->pbar){ return; }
		STime kkk1;
		kkk1.mstime = dur;
		vplayer->pbtime = videoTime.raw(TMP) + L" / " + kkk1.raw(TMP);
		vplayer->DrawProgressBar();
	}
	else{
		vslider->SetValue(val);
		vslider->Update();
		wxString times;
		times << videoTime.raw(SRT) << L";  ";
		TabPanel *pan = (TabPanel*)GetParent();
		if (!IsDshow){
			times << vplayer->numframe << L";  ";
			if (VFF){
				if (VFF->KeyFrames.Index(time) != -1){
					shownKeyframe = true;
					mstimes->SetForegroundColour(WindowWarningElements);
				}
				else if (shownKeyframe){
					shownKeyframe = false;
					mstimes->SetForegroundColour(WindowText);
				}
			}
		}
		if (pan->editor){
			Dialogue *line = pan->Edit->line;
			int sdiff = videoTime.mstime - ZEROIT(line->Start.mstime);
			int ediff = videoTime.mstime - ZEROIT(line->End.mstime);
			times << sdiff << L" ms, " << ediff << L" ms";
			pan->Grid->RefreshIfVisible(videoTime.mstime);
		}
		mstimes->SetValue(times);
		mstimes->Update();

	}

}

void VideoCtrl::OnCopyCoords(const wxPoint &pos)
{
	int w, h;
	GetClientSize(&w, &h);
	int nx = 0, ny = 0;
	Kai->GetTab()->Grid->GetASSRes(&nx, &ny);
	coeffX = (float)nx / (float)(w - 1);
	coeffY = (float)ny / (float)(h - panelHeight - 1);
	int posx = (float)pos.x*coeffX;
	int posy = (float)pos.y*coeffY;
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
	if (isFullscreen){ TD->bpause->ChangeBitmap(play); }
	else{ bpause->ChangeBitmap(play); }
}

void VideoCtrl::NextChap()
{
	if (chapters.size() < 1){ return; }
	int vrtime = Tell();
	for (int j = 0; j < (int)chapters.size(); j++){

		int ntime = (j >= (int)chapters.size() - 1) ? INT_MAX : chapters[(j + 1)].time;
		if (ntime >= vrtime){

			int jj = (j >= (int)chapters.size() - 1 || (j == 0 && chapters[0].time >= vrtime)) ? 0 : j + 1;
			if (jj == prevchap){ if (jj >= (int)chapters.size() - 1){ jj = 0; } else{ jj++; } }
			Seek(chapters[jj].time, true, true, true, false);

			prevchap = jj;
			break;
		}
	}
}

void VideoCtrl::PrevChap()
{
	if (chapters.size() < 1){ return; }
	int vrtime = Tell();
	for (int j = 0; j < (int)chapters.size(); j++){
		int ntime = (j >= (int)chapters.size() - 1) ? INT_MAX : chapters[(j + 1)].time;
		if (ntime >= vrtime){
			int jj = (j < 1) ? 0 : j - 1;
			if (jj == prevchap){ if (jj < 1){ jj = chapters.size() - 1; } else{ jj--; } }
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
	if (!IsDshow || !vplayer){ return; }
	wxArrayString enabled;
	Options.GetTable(AcceptedAudioStream, enabled, L";");
	if (enabled.size() < 1){ return; }
	wxArrayString streams = vplayer->GetStreams();
	//int firstSubsStream = -1;
	int numofastreams = 0;
	for (int i = streams.size() - 1; i >= 0; i--){
		if (streams[i][0] == L'A'){
			streams[i] = streams[i].AfterFirst(L' ').Lower();
			numofastreams++;
			continue;
		}
		/*else if (firstSubsStream == -1 && streams[i][0] == L'S' || streams[i][0] == L's'){
			firstSubsStream = i;
			}*/
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
				vplayer->EnableStream(streamToChange);
			}
		}
	}
	
}

BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
	VideoCtrl *vb = (VideoCtrl *)dwData;
	WinStruct<MONITORINFO> monitorinfo;
	if (!GetMonitorInfo(hMonitor, &monitorinfo)){
		KaiLog(_("Nie można pobrać informacji o monitorze"));
		return TRUE;
	}
	//podstawowy monitor ma być pierwszy w tablicy
	if (monitorinfo.dwFlags == MONITORINFOF_PRIMARY){
		vb->MonRects.insert(vb->MonRects.begin(), monitorinfo.rcMonitor);
		return TRUE;
	}
	vb->MonRects.push_back(monitorinfo.rcMonitor);
	return TRUE;
}

wxRect VideoCtrl::GetMonitorRect(int wmonitor){
	MonRects.clear();
	EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, (LPARAM)this);
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
	EditBox *eb = Kai->GetTab()->Edit;
	int vis = evt.GetInt();
	VideoToolbar *vTB = (isFullscreen && TD) ? TD->vToolbar : vToolbar;

	if (vis == eb->Visual){ return; }
	if (vplayer->Visual && vis == 0){
		vplayer->SetVisual(true);
	}
	else if (vis != eb->Visual){
		if (vplayer->hasZoom){ vplayer->SetZoom(); }
		eb->Visual = vis;
		vplayer->SetVisual();
		if (vis >= VECTORCLIP){ vplayer->Visual->ChangeTool(vTB->GetItemToggled()); }
		if (!hasArrow){ SetCursor(wxCURSOR_ARROW); hasArrow = true; }
		SetFocus();
	}
	
}

bool VideoCtrl::SetBackgroundColour(const wxColour &col)
{
	panel->SetBackgroundColour(col);
	return true;
}

BEGIN_EVENT_TABLE(VideoCtrl, wxWindow)
EVT_SIZE(VideoCtrl::OnSize)
EVT_MOUSE_EVENTS(VideoCtrl::OnMouseEvent)
EVT_KEY_DOWN(VideoCtrl::OnKeyPress)
EVT_PAINT(VideoCtrl::OnPaint)
EVT_TIMER(idvtime, VideoCtrl::OnPlaytime)
EVT_TIMER(ID_IDLE, VideoCtrl::OnIdle)
EVT_ERASE_BACKGROUND(VideoCtrl::OnErase)
EVT_MOUSE_CAPTURE_LOST(VideoCtrl::OnLostCapture)
END_EVENT_TABLE()