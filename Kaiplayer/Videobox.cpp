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
	int Recycle(const wchar_t *pszPath, BOOL bDelete=FALSE);
};

CRecycleFile::CRecycleFile()
{
	memset((SHFILEOPSTRUCT*)this,0,sizeof(SHFILEOPSTRUCT));
	fFlags |= FOF_SILENT;                
	fFlags |= FOF_NOERRORUI;            
	fFlags |= FOF_NOCONFIRMATION;        
}


int CRecycleFile::Recycle(const wchar_t *pszPath, BOOL bDelete)
{

	wchar_t buf[_MAX_PATH + 1]; 
	wcscpy(buf,pszPath);
	buf[wcslen(buf)+1]=0;   

	wFunc = FO_DELETE;                  
	pFrom = buf;                         
	pTo = NULL;                          
	if (bDelete) {                       
		fFlags &= ~FOF_ALLOWUNDO;         
	} else {                             
		fFlags |= FOF_ALLOWUNDO;          
	}
	return SHFileOperation(this);

}

class bars1 : public KaiDialog
{
public:
	bars1(VideoCtrl *parent);
	virtual ~bars1(){};

	KaiSlider *slider;
	KaiStaticText *actual;
	void OnSlider(wxCommandEvent &event);
	VideoCtrl *_parent;
};

bars1::bars1(VideoCtrl *parent)
	: KaiDialog((parent->isFullscreen)? (wxWindow*)parent->TD : parent, -1, "", wxDefaultPosition, wxDefaultSize)
{
	_parent=parent;
	DialogSizer *sizer= new DialogSizer(wxVERTICAL);
	actual= new KaiStaticText(this,-1,wxString::Format(_("Proporcje ekranu: %5.3f"), 1.f / parent->AR));
	slider= new KaiSlider(this, 7767, parent->AR*700000, 100000, 1000000, wxDefaultPosition, wxSize(400,-1),wxHORIZONTAL|wxSL_INVERSE);
	Connect(7767,wxEVT_SCROLL_THUMBTRACK,(wxObjectEventFunction)&bars1::OnSlider);
	sizer->Add(actual, 0, wxALL, 3);
	sizer->Add(slider, 1, wxEXPAND|wxALL, 3);
	SetSizerAndFit(sizer);
	MoveToMousePosition(this);
}

void bars1::OnSlider(wxCommandEvent &event)
{
	_parent->SetAspectRatio(slider->GetValue()/700000.0f);
	actual->SetLabelText(wxString::Format(_("Proporcje ekranu: %5.3f"), 1.f / _parent->AR));
}


VideoCtrl::VideoCtrl(wxWindow *parent, kainoteFrame *kfpar, const wxSize &size)
	: VideoRend(parent, size)
	,Kai(kfpar)
	,hasArrow(true)
	,seekfiles(true)
	,fullarrow(true)
	,ismenu(false)
	,eater(false)
	,actfile(0)
	,prevchap(-1)
	,wspx(0.0f)
	,wspy(0.0f)
	,TD(NULL)
	,blockpaint(false)
	,isOnAnotherMonitor(false)
	,shownKeyframe(false)
{

	panel=new wxWindow(this,-1,wxPoint(0,size.y-panelHeight),wxSize(size.x,panelHeight));
	panel->SetBackgroundColour(Options.GetColour(WindowBackground));

	vslider= new VideoSlider(panel, ID_SLIDER,wxPoint(0,1),wxSize(size.x,14));
	vslider->VB=this;
	bprev = new BitmapButton(panel,CreateBitmapFromPngResource("backward"),CreateBitmapFromPngResource("backward1"), PreviousVideo, _("Poprzedni plik"), wxPoint(5,16), wxSize(26,26));
	bpause = new BitmapButton(panel, CreateBitmapFromPngResource("play"),CreateBitmapFromPngResource("play1"),PlayPause, _("Odtwórz / Pauza"), wxPoint(40,16), wxSize(26,26));
	bpline = new BitmapButton(panel, CreateBitmapFromPngResource("playline"), CreateBitmapFromPngResource("playline1"),PlayActualLine,_("Odtwórz aktywną linię"), wxPoint(75,16), wxSize(26,26), GLOBAL_HOTKEY);
	bstop = new BitmapButton(panel, CreateBitmapFromPngResource("stop"),CreateBitmapFromPngResource("stop1"), StopPlayback, _("Zatrzymaj"),wxPoint(110,16), wxSize(26,26));
	bnext = new BitmapButton(panel, CreateBitmapFromPngResource("forward"), CreateBitmapFromPngResource("forward1"),NextVideo,_("Następny plik"), wxPoint(145,16), wxSize(26,26));

	volslider=new VolSlider(panel,ID_VOL,Options.GetInt(VideoVolume),wxPoint(size.x-110,17),wxSize(110,25));
	mstimes=new KaiTextCtrl(panel,-1,"",wxPoint(180,19),wxSize(360,-1),wxTE_READONLY);
	mstimes->SetWindowStyle(wxBORDER_NONE);
	mstimes->SetCursor(wxCURSOR_ARROW);
	mstimes->SetBackgroundColour(WindowBackground);

	vToolbar = new VideoToolbar(panel,wxPoint(0, panelHeight - 22));
	Bind(wxEVT_COMMAND_MENU_SELECTED, &VideoCtrl::OnChangeVisual, this, ID_VIDEO_TOOLBAR_EVENT);

	Bind(wxEVT_COMMAND_MENU_SELECTED,[=](wxCommandEvent &evt){
		Vclips->ChangeTool(evt.GetInt());
	},ID_VECTOR_TOOLBAR_EVENT);
	Bind(wxEVT_COMMAND_MENU_SELECTED,[=](wxCommandEvent &evt){
		Vclips->ChangeTool(evt.GetInt());
	},ID_MOVE_TOOLBAR_EVENT);
	Bind(wxEVT_COMMAND_MENU_SELECTED,[=](wxCommandEvent &evt){
		RefreshTime();
	},23334);

	Connect(PreviousVideo,NextVideo,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&VideoCtrl::OnAccelerator);
	Connect(PlayPause,StopPlayback,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&VideoCtrl::OnAccelerator);
	Connect(ID_VOL,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&VideoCtrl::OnVolume);

	vtime.SetOwner(this,idvtime);
	idletime.SetOwner(this,ID_IDLE);

}

VideoCtrl::~VideoCtrl()
{
}

bool VideoCtrl::Play()
{
	wxMutexLocker lock(vbmutex);
	if(time>=GetDuration()){return false;}
	if(!VideoRend::Play()){return false;}
	int ms=(isFullscreen)?1000:100;
	vtime.Start(ms);
	ChangeButtonBMP(false);
	return true;
}

void VideoCtrl::PlayLine(int start, int end)
{
	//wxMutexLocker lock(vbmutex);
	if(!VideoRend::PlayLine(start, end)){return;}
	int ms=(isFullscreen)?1000:100;
	vtime.Start(ms);
	ChangeButtonBMP(false);
}

bool VideoCtrl::Pause(bool burstbl)
{
	wxMutexLocker lock(vbmutex);

	if(GetState()==None){
		MenuItem *index=Kai->Menubar->FindItem(VideoIndexing);
		if(index->IsChecked() && index->IsEnabled()){
			EditBox *eb = Kai->GetTab()->Edit;
			if(eb->ABox){
				eb->ABox->audioDisplay->Play(eb->line->Start.mstime, eb->line->End.mstime);
				return true;
			}
			return false;
		}
		LoadVideo(Kai->videorec[Kai->videorec.size()-1],NULL);
		return true;
	}
	if(time>=GetDuration()&&burstbl){return false;}
	if(!VideoRend::Pause()){return false;}
	if(GetState()==Paused){
		vtime.Stop();RefreshTime();}
	else if(GetState()==Playing){int ms=(isFullscreen)?1000:100;vtime.Start(ms);}
	ChangeButtonBMP(!(GetState()==Playing));

	return true;
}

bool VideoCtrl::Stop()
{
	wxMutexLocker lock(vbmutex);

	if(!VideoRend::Stop()){return false;}
	vtime.Stop();
	Seek(0);
	RefreshTime();

	ChangeButtonBMP(true);

	return true;
}

bool VideoCtrl::LoadVideo(const wxString& fileName, wxString *subsName,bool fulls /*= false*/, bool changeAudio)
{   
	if(fulls){SetFullscreen();}
	prevchap=-1;
	MenuItem *index=Kai->Menubar->FindItem(VideoIndexing);
	bool byFFMS2 = index->IsChecked() && index->IsEnabled() && !fulls && !isFullscreen;
	bool shown=true;
	block=true;
	if (!OpenFile(fileName, subsName, !byFFMS2, !Kai->GetTab()->editor, changeAudio)){
		delete subsName; block=false; return false;
	}
	if( !(IsShown() || (TD && TD->IsShown())) ){
		shown=false; Show();
	}

	eater=IsDshow;
	
	if(!isFullscreen&&!fulls){
		int sx,sy;
		//wyłączony edytor
		if(!Kai->GetTab()->editor){
			if(!Kai->IsMaximized()){
				int sizex,sizey;
				Kai->GetClientSize(&sizex,&sizey);
				CalcSize(&sx,&sy,0,0,true,true);
				sx+=iconsize;
				sy+= (panelHeight + Kai->Tabs->GetHeight() + Kai->Menubar->GetSize().y + Kai->StatusBar->GetSize().y);
				if(sx == sizex && sy == sizey){
					UpdateVideoWindow();
				}else{
					Kai->SetClientSize(sx, sy);
					Kai->GetTab()->BoxSizer1->Layout();
				}
			}
			//załączony edytor
		}else{
			int kw,kh;
			Options.GetCoords(VideoWindowSize,&kw,&kh);
			bool ischanged = CalcSize(&sx,&sy,kw,kh,true,true);
			if(ischanged||!shown){
				SetMinSize(wxSize(sx,sy+panelHeight));
				Kai->GetTab()->BoxSizer1->Layout();
			}
			Options.SetCoords(VideoWindowSize,sx,sy+panelHeight);
		}

	}
	if(isFullscreen){
		UpdateVideoWindow();
		wxSize size=GetVideoSize();
		wxString res;
		Kai->SetVideoResolution(size.x, size.y, !Options.GetBool(DontAskForBadResolution));
	}
	//block = false;
	if(IsDshow){
		Play();
		if (Kai->GetTab()->editor && !isFullscreen){ Pause(); }
		if (!volslider->IsShown()){ volslider->Show(); mstimes->SetSize(lastSize.x - 290, -1); }
	}else{
		block = false;
		if(volslider->IsShown()){
			volslider->Show(false); 
			mstimes->SetSize(lastSize.x-185,-1);
		}
		vstate = Paused;
		Render();
	}

	RefreshTime();
	if(IsDshow){
		int pos= Options.GetInt(VideoVolume);
		SetVolume(-(pos*pos));
	}
	//SetFocus();
	Kai->GetTab()->VideoPath=fileName;
	Kai->GetTab()->VideoName=Kai->GetTab()->VideoPath.AfterLast('\\');
	Kai->SetStatusText(Kai->GetTab()->VideoName,8);
	if(TD){TD->Videolabel->SetLabelText(Kai->GetTab()->VideoName);}
	if(!Kai->GetTab()->editor){Kai->Label(0,true);}
	Kai->SetStatusText(getfloat(fps)+" FPS",4);
	wxString tar;
	tar<<ax<<" : "<<ay;
	Kai->SetStatusText(tar,6);
	STime kkk1;
	kkk1.mstime=GetDuration();
	Kai->SetStatusText(kkk1.raw(SRT),3);
	Kai->SetRecent(1);
	if(Kai->GetTab()->editor && (!isFullscreen || IsShown()) && 
		Kai->GetTab()->SubsPath!="" && Options.GetBool(OpenVideoAtActiveLine)){
			Seek(Kai->GetTab()->Edit->line->Start.mstime);
	}
	SetScaleAndZoom();
	ChangeStream();
	return true;
}


PlaybackState VideoCtrl::GetState()
{
	return vstate; 
}

bool VideoCtrl::Seek(int whre,bool starttime, bool disp, bool reloadSubs)
{
	wxMutexLocker lock(vbmutex);
	if(GetState()==None){return false;}
	SetPosition(whre, starttime, true, reloadSubs);
	if(disp && !IsDshow){RefreshTime();}
	return true;
}

int VideoCtrl::Tell()
{
	return GetCurrentPosition();
}


void VideoCtrl::OnSize(wxSizeEvent& event)
{
	wxSize asize = GetClientSize();
	if(lastSize == asize){return;}
	lastSize = asize;
	panel->SetSize(0, asize.y-panelHeight, asize.x, panelHeight);
	vslider->SetSize(wxSize(asize.x,14));
	volslider->SetPosition(wxPoint(asize.x-110,17));
	int difSize = (volslider->IsShown())? 290 : 185;
	mstimes->SetSize(asize.x - difSize,-1);
	vToolbar->SetSize(asize.x, 22);
	if(vstate != None){
		UpdateVideoWindow();
	}else{
		Refresh(false);
	}
}


void VideoCtrl::OnMouseEvent(wxMouseEvent& event)
{

	if(event.ButtonDown())
	{
		SetFocus();
		if(ismenu){ismenu=false;}
	}
	if(vstate==None){return;}
	if(hasZoom){
		ZoomMouseHandle(event);
		return;
	}
	int x=event.GetX(), y= event.GetY();
	if (event.GetWheelRotation() != 0 ) {

		if(event.ControlDown() && !isFullscreen){ 
			int step = event.GetWheelRotation() / event.GetWheelDelta();

			int w,h, mw, mh;
			GetClientSize(&w,&h);
			GetParent()->GetClientSize(&mw,&mh);
			int incr=h+(step*20);
			if(incr>=mh){incr=mh-3;}
			if( y < h-panelHeight){
				if(h<=350 && step<0 || h == incr){return;}
				int ww,hh;
				CalcSize(&ww,&hh,w,incr,false,true);
				SetMinSize(wxSize(ww,hh+panelHeight));
				Options.SetCoords(VideoWindowSize,ww,hh+panelHeight);
				Kai->GetTab()->BoxSizer1->Layout();
				if(event.ShiftDown()){
					Kai->GetTab()->SetVideoWindowSizes(w, incr);
				}
			}
			return;
		}else if(!Vclips){ 
			if(!IsDshow){
				AudioBox *box = Kai->GetTab()->Edit->ABox;
				if(box){
					int vol = box->GetVolume();
					int step = event.GetWheelRotation() / event.GetWheelDelta();
					vol+=(step*3);
					box->SetVolume(vol);
				}
			}
			else if(isFullscreen){TD->volslider->OnMouseEvent(event);}
			else{volslider->OnMouseEvent(event);}
			return;
		}
	}


	if(Vclips){
		Vclips->OnMouseEvent(event);if(!hasArrow){SetCursor(wxCURSOR_ARROW);hasArrow=true;}
		return;
	}//jak na razie 


	if(event.LeftDClick() && event.GetModifiers()==0){
		SetFullscreen();
		if(!isFullscreen && Kai->GetTab()->SubsPath!="" && Options.GetBool(SelectVisibleLineAfterFullscreen)){
			Kai->GetTab()->Edit->Send(EDITBOX_LINE_EDITION,false);
			Kai->GetTab()->Grid->SelVideoLine();
		}
		int w,h;
		GetClientSize(&w,&h);

		if(y>=h-panelHeight && !hasArrow){
			SetCursor(wxCURSOR_ARROW);hasArrow=true;
		}
		return;
	}

	if(isFullscreen){
		if(eater && event.Moving() && !event.ButtonDown()){Sleep(200);eater=false;return;}
		if(!fullarrow){TD->SetCursor(wxCURSOR_ARROW);fullarrow=true;}

		int w,h;
		TD->GetClientSize(&w, &h);
		if(y >= h - panelHeight && !TD->panel->IsShown()){TD->panel->Show();}
		else if(y < h - panelHeight && TD->panel->IsShown() && !panelOnFullscreen){TD->panel->Show(false);SetFocus();}
		if(!TD->panel->IsShown() && !ismenu){idletime.Start(1000, true);}
	}
	else if(Kai->GetTab()->editor){
		int w,h;
		GetClientSize(&w,&h);
		if(hasArrow && y<(h-panelHeight)){SetCursor(wxCURSOR_BLANK);hasArrow=false;}

		if(event.Leaving()){
			if(cross){
				cross=false;
				if(!hasArrow){SetCursor(wxCURSOR_ARROW);hasArrow=true;}
				if(GetState()==Paused && !block){Render(false);}
			}
			return;
		}

		if(event.Entering()){
			int nx=0, ny=0;
			Kai->GetTab()->Grid->GetASSRes(&nx,&ny);
			wspx=(float)nx/(float)(w - 1);
			wspy=(float)ny/(float)(h - panelHeight - 1);

		}
		int curX=event.GetX();
		int curY=event.GetY();
		int posx=(float)curX*wspx;
		int posy=(float)curY*wspy;
		coords="";
		coords<<posx<<", "<<posy;
		DrawLines(wxPoint(curX,curY));


	}else if(!hasArrow){SetCursor(wxCURSOR_ARROW);hasArrow=true;}



	if(Options.GetBool(VideoPauseOnClick) && event.LeftUp() && !event.ControlDown()){
		Pause();
	}


	if (event.RightDown()) {
		ContextMenu(event.GetPosition());
		return;}

	if (event.MiddleDown()||(event.LeftDown()&&event.ControlDown())){
		//OpenEditor();
		if(!isFullscreen)
		{
			Dialogue *aline=Kai->GetTab()->Edit->line;
			bool istl=(Kai->GetTab()->Grid->hasTLMode && aline->TextTl!="");
			wxString ltext=(istl)? aline->TextTl : aline->Text;
			wxRegEx posmov("\\\\(pos|move)([^\\\\}]+)",wxRE_ADVANCED);
			posmov.ReplaceAll(&ltext,"");

			wxString postxt;
			float posx=(float)event.GetX()*wspx;
			float posy=(float)event.GetY()*wspy;
			postxt= "\\pos("+ getfloat(posx) + "," + getfloat(posy) + ")";
			if(ltext.StartsWith("{")){
				ltext.insert(1,postxt);
			}else{
				ltext="{"+postxt+"}"+ltext;
			}
			if(istl){aline->TextTl=ltext;}else{aline->Text=ltext;}
			Kai->GetTab()->Grid->ChangeCell((istl)?TXTTL : TXT, Kai->GetTab()->Edit->ebrow, aline);
			Kai->GetTab()->Grid->Refresh(false);
			Kai->GetTab()->Grid->SetModified(VISUAL_POSITION);
		}
	}


}



void VideoCtrl::OnPlaytime(wxTimerEvent& event)
{
	RefreshTime();
}

void VideoCtrl::OnKeyPress(wxKeyEvent& event)
{
	int key = event.GetKeyCode();
	if(key=='F'){SetFullscreen();}
	else if(key == WXK_WINDOWS_MENU){
		wxWindow *owner = (isFullscreen && TD)? (wxWindow *)TD : this;
		wxPoint poss= owner->ScreenToClient(wxGetMousePosition());
		ContextMenu(poss);}
	else if((key=='B'||key==WXK_ESCAPE) && isFullscreen){
		//OpenEditor((key==WXK_ESCAPE));
		SetFullscreen();
		if(Kai->GetTab()->SubsPath!=""){
			Kai->GetTab()->Grid->SelVideoLine();}
		if(key=='B'){if(GetState()==Playing){Pause();}ShowWindow(Kai->GetHWND(),SW_SHOWMINNOACTIVE);}
	}
	else if(key=='S'&&event.m_controlDown){Kai->Save(false);}
	else if(key==WXK_RETURN && hasZoom){
		SetZoom();
	}
	else if(key=='Z' && event.ControlDown() && event.ShiftDown()){
		ResetZoom();
	}
	else if(event.ControlDown() && key=='A' && Vclips && (Vclips->Visual == VECTORCLIP || Vclips->Visual == VECTORDRAW)){
		((DrawingAndClip*)Vclips)->ChangeSelection(true);
		Render(false);
	}
}


void VideoCtrl::OnIdle(wxTimerEvent& event)
{
	if(isFullscreen && !TD->panel->IsShown()&&!ismenu){
		TD->SetCursor(wxCURSOR_BLANK);eater=IsDshow;fullarrow=false;
	}
}



void VideoCtrl::NextFile(bool next)
{
	wxMutexLocker lock(nextmutex);
	//if(vstate==Playing){Pause(false);}
	wxString path;
	if(Kai->GetTab()->VideoPath!=""){
		path=Kai->GetTab()->VideoPath;}
	else{path=Kai->videorec[Kai->videorec.size()-1];}
	wxString pathwn=path.BeforeLast('\\');
	//if(pathwn!=oldpath){
	wxDir kat(pathwn);
	if(kat.IsOpened()){
		pliki.Clear();
		kat.GetAllFiles(pathwn,&pliki,"", wxDIR_FILES);
	}
	//oldpath=pathwn;
	//}
	for(int j=0;j<(int)pliki.GetCount();j++)
	{
		if(pliki[j]==path){actfile=j;break;}
	}
	if(next && actfile>=(int)pliki.GetCount()-1){Seek(0);Pause(false);actfile=pliki.GetCount()-1;return;}
	else if(!next && actfile<=0){Seek(0);Pause(false);actfile=0;return;}

	int k= (next)? actfile+1 : actfile-1;
	while((next)? k<(int)pliki.GetCount() : k>=0)
	{

		wxString ext=pliki[k].AfterLast('.').Lower();
		if(ext=="avi"||ext=="mp4"||ext=="mkv"||ext=="ogm"||ext=="wmv"||
			ext=="asf"||ext=="rmvb"||ext=="rm"||ext=="3gp"||//||ext=="avs" przynajmniej do momentu dodania otwierania avs przy włączonym ffms2
			ext=="ts"||ext=="m2ts"||ext=="mpg"||ext=="mpeg"){


				bool isload=Kai->OpenFile(pliki[k]);
				if(isload){
					actfile=k; 
					seekfiles=false;
					if(isFullscreen){SetFocus();}
					return;
				}
				else if(!IsDshow){return;}
		}
		if(next){k++;}else{k--;}
	}
	Seek(0);
	Pause(false);
}

void VideoCtrl::SetFullscreen(int monitor)
{
	//wxMutexLocker lock(vbmutex);
	isFullscreen = !isFullscreen;

	//wyjście z fullskreena
	if(!isFullscreen){

		if(GetState()==Playing){if(Kai->GetTab()->editor){Pause();}else{vtime.Start(100);}}
		if(TD->HasCapture()){TD->ReleaseMouse();}

		int sx,sy,sizex,sizey;

		if(!Kai->GetTab()->editor){
			if(!Kai->IsMaximized()){
				Kai->GetClientSize(&sizex,&sizey);
				int yDiff = panelHeight + Kai->Tabs->GetHeight() + Kai->Menubar->GetSize().y + Kai->StatusBar->GetSize().y;
				CalcSize(&sx,&sy,sizex - iconsize,sizey - yDiff);
				Kai->SetClientSize(sx + iconsize, sy + yDiff);
			}
		}else{
			Options.GetCoords(VideoWindowSize,&sizex,&sizey);
			CalcSize(&sx,&sy,sizex,sizey);
			SetMinSize(wxSize(sx,sy+panelHeight));
			Kai->GetTab()->BoxSizer1->Layout();
		}
		volslider->SetValue(Options.GetInt(VideoVolume));

		if(!IsShown()){
			isOnAnotherMonitor=false;
			Show();GetParent()->Layout();
		}
		UpdateVideoWindow();
		block=true;
		Render();
		block=false;

		vToolbar->Synchronize(TD->vToolbar);
		RefreshTime();
		TD->Hide();
		SetCursor(wxCURSOR_ARROW); hasArrow=true;
	}
	//przejście na fullskreena
	else{
		if(wxWindow::HasCapture()){wxWindow::ReleaseMouse();}
		wxRect rt = GetMonitorRect(monitor);
		if(!TD){
			TD=new Fullscreen(this,rt.GetPosition(),rt.GetSize());
			TD->Videolabel->SetLabelText(Kai->GetTab()->VideoName);
		}else{
			TD->SetPosition(rt.GetPosition());
			TD->SetSize(rt.GetSize());
		}
		TD->OnSize();
		TD->volslider->SetValue(Options.GetInt(VideoVolume));
		TD->vToolbar->Synchronize(vToolbar);
		if(!panelOnFullscreen){TD->panel->Hide();}
		TD->Show();
		block = true;
		UpdateVideoWindow();
		block = false;
		RefreshTime();
		if(GetState()==Playing){vtime.Start(1000);}
		//if(GetState()==Paused){Kp->Render();}
		if(monitor && Kai->GetTab()->editor){
			Hide();
			GetParent()->Layout();
			isOnAnotherMonitor=true;
		}else{
			SetFocus();
		}
	}
	ChangeButtonBMP(!(GetState()==Playing));


}

bool VideoCtrl::CalcSize(int *width, int *height,int wwidth,int wheight,bool setstatus,bool calcH)
{
	wxSize size=GetVideoSize();
	if(setstatus){
		Kai->SetVideoResolution(size.x, size.y, !Options.GetBool(DontAskForBadResolution));}
	if(wwidth==0){
		GetClientSize(&wwidth,&wheight);}
	float precy=size.y,precx=size.x;
	if(!calcH){
		size.x=wwidth;
		if(AR<=0){size.y*=(wwidth/precx);}
		else{size.y=size.x*AR;}
	}
	if(calcH||size.y>700){
		wheight-=panelHeight;
		size.y=wheight;
		if(AR>0){size.x=size.y/AR;}
		else{size.x*=(wheight/precy);}
	}
	*width=size.x;
	*height=size.y;
	return !(size.x == wwidth && size.y == wheight);
}

void VideoCtrl::OnPrew()
{
	MenuItem *index=Kai->Menubar->FindItem(VideoIndexing);
	if(index->IsChecked()&&index->IsEnabled()){
		if(KaiMessageBox(_("Czy na pewno chcesz zindeksować poprzednie wideo?"),_("Potwierdzenie"),wxYES_NO)==wxNO)return;}
	NextFile(false);
}


void VideoCtrl::OnNext()
{
	MenuItem *index=Kai->Menubar->FindItem(VideoIndexing);
	if(index->IsChecked()&&index->IsEnabled()){
		if(KaiMessageBox(_("Czy na pewno chcesz zindeksować następne wideo?"),_("Potwierdzenie"),wxYES_NO)==wxNO)return;}
	NextFile();
}

//void VideoCtrl::OnVButton(wxCommandEvent& event)
//{
//	int id=event.GetId();
//	wxLogStatus("id %i", id);
//	if(id==ID_BPAUSE){Pause();}
//	else if(id==ID_BSTOP){
//		if(!Kai->GetTab()->editor){Stop();}
//		else{
//			if(GetState()==Playing){Pause();}
//			if(IsDshow){Seek(0);}
//		}
//	}
//	else if(id==ID_BPREV){OnPrew();}
//	else if(id==ID_BNEXT){OnNext();}
//	else if(id==ID_BPLINE){
//		EditBox *EB = Notebook::GetTab()->Edit;
//		EB->TextEdit->SetFocus();
//		PlayLine(EB->line->Start.mstime,EB->line->End.mstime - avtpf);
//	}
//}

void VideoCtrl::OnVolume(wxScrollEvent& event)
{
	int pos=event.GetPosition();
	Options.SetInt(VideoVolume,pos);
	SetVolume(-(pos*pos));
}

void VideoCtrl::ContextMenu(const wxPoint &pos, bool dummy)
{
	ismenu=true;
	Menu* menu=new Menu();
	wxString txt;
	if(GetState()!=Playing){txt=_("Odtwórz\t")+Hkeys.GetMenuH(PlayPause);}
	else if(GetState()==Playing){txt=_("Pauza\t")+Hkeys.GetMenuH(PlayPause);}
	if(!isFullscreen && ((TabPanel*)GetParent())->editor)
	{
		menu->SetAccMenu(CopyCoords,_("Kopiuj pozycję na wideo"));
	}
	menu->Append(PlayPause,txt)->Enable(GetState()!=None);
	menu->SetAccMenu(StopPlayback,_("Zatrzymaj"))->Enable(GetState()==Playing);
	wxString txt1;
	if(!isFullscreen){txt1=_("Pełny ekran\tF");}
	else{txt1=_("Wyłącz pełny ekran\tEscape");}
	menu->SetAccMenu(FullScreen,txt1)->Enable(GetState()!=None);

	GetMonitorRect(-1);
	for(size_t i=1; i<MonRects.size(); i++)
	{
		wxString txt2;
		if(isFullscreen){txt2 = wxString::Format(_("Przełącz pełny ekran na %i monitor"), (int)(i+1));}
		else{txt2 = wxString::Format(_("Włącz pełny ekran na %i monitorze"), (int)(i+1));}
		menu->SetAccMenu(MENU_MONITORS+i,txt2)->Enable(GetState()!=None);
	}

	menu->SetAccMenu(Editor,_("Otwórz edytor"))->Enable(isFullscreen);
	Menu* menu1=new Menu();
	Menu* menu2=new Menu();
	for(size_t i=0;i<20;i++)
	{
		if(i<Kai->subsrec.size()){
			if(!wxFileExists(Kai->subsrec[i])){continue;}
			menu1->Append(30000+i, Kai->subsrec[i].AfterLast('\\'));
		}
		if(i<Kai->videorec.size()){
			if(!wxFileExists(Kai->videorec[i])){continue;}
			menu2->Append(30020+i, Kai->videorec[i].AfterLast('\\'));
		}

	}
	menu->Append(ID_MRECSUBS, _("Ostatnio otwarte napisy"), menu1);
	menu->Append(ID_MRECVIDEO, _("Ostatnio otwarte wideo"), menu2);
	menu->SetAccMenu(OpenVideo,_("Otwórz wideo"));

	menu->SetAccMenu(Id::OpenSubs, _("Otwórz napisy"));
	menu->SetAccMenu(HideProgressBar,_("Ukryj / pokaż pasek postępu"))->Enable(isFullscreen);
	menu->SetAccMenu(AspectRatio,_("Zmień proporcje wideo"));
	menu->SetAccMenu(SubbedFrameToPNG,_("Zapisz klatkę z napisami jako PNG"))->Enable(GetState()==Paused);
	menu->SetAccMenu(SubbedFrameToClipboard,_("Kopiuj klatkę z napisami do schowka"))->Enable(GetState()==Paused);
	menu->SetAccMenu(FrameToPNG,_("Zapisz klatkę jako PNG"))->Enable(GetState()==Paused && ((TabPanel*)GetParent())->editor);
	menu->SetAccMenu(FrameToClipboard,_("Kopiuj klatkę do schowka"))->Enable(GetState()==Paused && ((TabPanel*)GetParent())->editor);
	menu->AppendSeparator();

	menu->SetAccMenu(DeleteVideo,_("Usuń plik wideo"))->Enable(GetState()!=None);
	if(dummy){
		delete menu;
		ismenu=false;
		return;
	}

	Menu* menu3=NULL;
	int numfilters=0;
	if(GetState()!=None && IsDshow){
		menu3=new Menu();
		EnumFilters(menu3);
		numfilters=menu3->GetMenuItemCount();
		menu->Append(23456,_("Filtry"),menu3,_("Wyświetla użyte filtry"));
	}


	wxArrayString streams=GetStreams();
	wxString prev;
	wxString name, enable;
	for(size_t i=0; i<streams.size(); i++){
		wxString ident= streams[i].BeforeFirst(':');
		name=streams[i].BeforeLast(' ', &enable);
		if(ident!=prev){menu->AppendSeparator();}
		menu->Append(MENU_STREAMS+i,name,"",true,0,0,(enable=="1")?ITEM_RADIO : ITEM_NORMAL);//->Check(enable=="1");
		prev=ident;
	}
	STime timee;
	for(size_t j=0; j<chaps.size(); j++){
		if(j==0){menu->AppendSeparator();}
		timee.NewTime(chaps[j].time);
		int ntime= (j>=chaps.size()-1)? INT_MAX : chaps[(j+1)].time;
		menu->Append(MENU_CHAPTERS+j,chaps[j].name+"\t["+timee.raw()+"]","",true,0,0,(ntime>=time)?ITEM_RADIO : ITEM_NORMAL);
	}
	id=0;
	int Modifiers=0;
	menu->SetMaxVisible(50);
	//ismenu=true;
	if(isFullscreen){
		id=menu->GetPopupMenuSelection(pos, TD, &Modifiers, true);
		//wxLogStatus("fulscreen menu %i", id);
	}else{
		id=menu->GetPopupMenuSelection(pos, this, &Modifiers, true);
	}
	//ismenu=false;
	if((Modifiers == wxMOD_SHIFT) && id<2100 && id>=2000){
		MenuItem *item=menu->FindItem(id);
		int ret=-1;
		wxString name=item->GetLabelText();
		ret=Hkeys.OnMapHkey(id, name, this, VIDEO_HOTKEY);
		if(ret!=-2){
			Hkeys.SetAccels(true);
			Hkeys.SaveHkeys();
		}
		delete menu;
		ismenu=false;
		return;
	}

	if(id==CopyCoords){OnCopyCoords(pos);}
	else if(id>1999 &&id<MENU_STREAMS){
		wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED,id);
		OnAccelerator(evt);
	}
	else if(id>=30000 && id<30020){Kai->OpenFile(Kai->subsrec[id-30000]);}
	else if(id>=30020 && id<30040){Kai->OpenFile(Kai->videorec[id-30020]);}
	else if(id>=MENU_STREAMS && id < MENU_STREAMS+(int)streams.size()){
		int wstream=id - MENU_STREAMS;
		EnableStream((long)wstream);
	}
	else if(id>=MENU_CHAPTERS && id< MENU_CHAPTERS+(int)chaps.size()){
		Seek(chaps[id-MENU_CHAPTERS].time);
	}
	else if(id>=13000 && id<13000+numfilters && menu3){
		MenuItem *item = menu3->FindItem(id);
		FilterConfig(item->GetLabel(), id-13000,pos);
	}else if(id>15000 && id<15000+(int)MonRects.size()){
		isFullscreen=false;
		SetFullscreen(id - 15000);
	}
	delete menu;
	ismenu=false;
}


void VideoCtrl::OnHidePB()
{
	bool pb=!Options.GetBool(VideoProgressBar);
	Options.SetBool(VideoProgressBar,pb);
	if(pb){pbar=true; RefreshTime();}else{pbar=false;}
	if(GetState()==Paused){Render(false);}
}

void VideoCtrl::OnDeleteVideo()
{
	if(KaiMessageBox(_("Czy na pewno chcesz przenieść wczytany plik wideo do kosza?"), _("Usuwanie"), wxYES_NO)==wxNO){return;}
	wxString path=Kai->GetTab()->VideoPath;
	NextFile();
	CRecycleFile x;
	x.Recycle(path.data());
}

void VideoCtrl::OnOpVideo()
{
	wxFileDialog* FileDialog2 = new wxFileDialog(this, _("Wybierz plik wideo"),
		(Kai->videorec.size()>0)?Kai->videorec[Kai->videorec.size()-1].BeforeLast('\\'):"",
		"", _("Pliki wideo(*.avi),(*.mkv),(*.mp4),(*.ogm),(*.wmv),(*.asf),(*.rmvb),(*.rm),(*.3gp),(*.avs)|*.avi;*.mkv;*.mp4;*.ogm;*.wmv;*.asf;*.rmvb;*.rm;*.3gp;*.avs|Wszystkie pliki (*.*)|*.*"),
		wxFD_DEFAULT_STYLE, wxDefaultPosition, wxDefaultSize, "wxFileDialog");
	if (FileDialog2->ShowModal() == wxID_OK){
		Kai->OpenFile(FileDialog2->GetPath());
	}
	FileDialog2->Destroy();
}

void VideoCtrl::OnOpSubs()
{
	if(Kai->SavePrompt(2)){return;}
	wxFileDialog* FileDialog2 = new wxFileDialog(Kai, _("Wybierz plik napisów"), 
		(Kai->subsrec.size()>0)?Kai->subsrec[Kai->subsrec.size()-1].BeforeLast('\\'):"", "", 
		_("Pliki napisów (*.ass),(*.sub),(*.txt)|*.ass;*.sub;*.txt"), 
		wxFD_DEFAULT_STYLE, wxDefaultPosition, wxDefaultSize, "wxFileDialog");

	if (FileDialog2->ShowModal() == wxID_OK){
		Kai->OpenFile(FileDialog2->GetPath());
	}
	FileDialog2->Destroy();
}

void VideoCtrl::OpenEditor(bool esc)
{

	if(isFullscreen){
		if(GetState()==Playing){Pause();}	
		//if(Kai->GetTab()->SubsPath.BeforeLast('.')!=Kai->GetTab()->VideoPath.BeforeLast('.'))
		//{wxString fn=Kai->FindFile(Kai->GetTab()->VideoPath,false,false);
		//bool isgood=false;
		//if(fn!=""){bool isgood=Kai->OpenFile(fn);}
		//}
		Options.SetBool(EditorOn, true);
		if(Kai->GetTab()->SubsPath!=""){
			Kai->GetTab()->Grid->SelVideoLine();}


		SetFullscreen();

		if(!esc){ShowWindow(Kai->GetHWND(),SW_MINIMIZE);}
	}

}


void VideoCtrl::OnAccelerator(wxCommandEvent& event)
{

	id=event.GetId();
	if(id==PlayPause){Pause();}
	else if(id==Minus5Second){Seek(Tell()-5000);}
	else if(id==Plus5Second){Seek(Tell()+5000);}
	else if(id==MinusMinute){Seek(Tell()-60000);}
	else if(id==PlusMinute){Seek(Tell()+60000);}
	else if(id==PreviousVideo){OnPrew();}
	else if(id==NextVideo){OnNext();}
	else if(id==VolumePlus){OnSPlus();}
	else if(id==VolumeMinus){OnSMinus();}
	else if(id==PreviousChapter){PrevChap();}
	else if(id==NextChapter){NextChap();}
	else if(id==CopyCoords){wxPoint pos=wxGetMousePosition();pos=ScreenToClient(pos);OnCopyCoords(pos);}
	else if(id==StopPlayback){
		if(!Kai->GetTab()->editor){Stop();}
		else{
			if(GetState()==Playing){
				Pause();
			}
			if(IsDshow){Seek(0);}
		}
	}
	else if(id==FullScreen){SetFullscreen();}
	else if(id==Editor){OpenEditor();}
	else if(id==OpenVideo){OnOpVideo();}
	else if(id==Id::OpenSubs){OnOpSubs();}
	else if(id==HideProgressBar){OnHidePB();}
	else if(id==AspectRatio){
		bars1 changear(this);
		changear.ShowModal();
	}
	else if(id==DeleteVideo){OnDeleteVideo();}
	else if(id>=FrameToPNG && id<=SubbedFrameToClipboard && GetState()==Paused){
		CColorSpaceConverter conv(vformat,vwidth,vheight);
		bool del=false;
		byte *framebuf =  GetFramewithSubs(id>FrameToClipboard, &del);
		if(id==FrameToPNG || id==SubbedFrameToPNG){
			TabPanel *pan=Notebook::GetTab();
			wxString path;
			int num=0;
			do{
				path= pan->VideoPath;
				path<<num<<".png";
				num++;
			}while(wxFileExists(path));
			conv.SavePNG(path,framebuf);
		}else{
			conv.SavetoClipboard(framebuf);
		}
		if(del){delete framebuf;}
	}
}


void VideoCtrl::OnSMinus()
{
	if(IsDshow){
		int pos=volslider->GetValue()-2;
		if(pos>(-91)){
			SetVolume(-(pos*pos));
			volslider->SetValue(pos);
			if(TD){TD->volslider->SetValue(pos);}
			Options.SetInt(VideoVolume,pos);
		}
	}else{
		AudioBox *box = Kai->GetTab()->Edit->ABox;
		if(box){
			int vol = box->GetVolume();
			vol -= 2;
			if(vol>=0){
				box->SetVolume(vol);
			}
		}
	}
}

void VideoCtrl::OnSPlus()
{
	if(IsDshow){
		int pos=volslider->GetValue()+2;
		if(pos<1){
			SetVolume(-(pos*pos));
			volslider->SetValue(pos);
			if(TD){TD->volslider->SetValue(pos);}
			Options.SetInt(VideoVolume,pos);
		}
	}else{

		AudioBox *box = Kai->GetTab()->Edit->ABox;
		if(box){
			int vol = box->GetVolume();
			vol += 2;
			if(vol<102){
				box->SetVolume(vol);
			}
		}

	}
}

void VideoCtrl::OnPaint(wxPaintEvent& event)
{
	if( !block /*&& !blockpaint*/&& vstate==Paused ){
		Render(true);
	}
	else if(vstate==None){
		int x, y;
		GetClientSize(&x,&y);
		wxPaintDC dc(this);
		dc.SetBrush(wxBrush("#000000"));
		dc.SetPen(wxPen("#000000"));
		dc.DrawRectangle(0,0,x,y);
		wxFont font1(72,wxSWISS,wxFONTSTYLE_NORMAL,wxNORMAL,false,"Tahoma");
		dc.SetFont(font1);
		wxSize size = dc.GetTextExtent("KaiNote");
		dc.SetTextForeground("#2EA6E2");
		dc.DrawText("KaiNote", (x-size.x)/2, (y - size.y - panelHeight)/2);
	}

}

void VideoCtrl::OnEndFile(wxCommandEvent &event)
{
	if((!Kai->GetTab()->editor||isFullscreen) && IsDshow){ NextFile();}
	else{if(vstate==Playing){Pause(false);}}
}

void VideoCtrl::SetAspectRatio(float _AR)
{
	AR=_AR;
	TabPanel *tab = ((TabPanel*)GetParent());
	if(tab->editor && !isFullscreen){
		int ww,hh;
		CalcSize(&ww,&hh,0,0,false,true);
		SetMinSize(wxSize(ww,hh+panelHeight));
		tab->BoxSizer1->Layout();
	}
	UpdateVideoWindow();
	if(GetState()==Paused){Render(false);}
}

void VideoCtrl::SetScaleAndZoom()
{
	wxString scale;
	wxSize wsize = GetSize();
	scale<<(int)((wsize.x/(float)vwidth)*100)<<"%";
	Kai->SetStatusText(scale,1);
	wxString zoom;
	zoom<<(int)(zoomParcent*100)<<"%";
	Kai->SetStatusText(zoom,2);
}

void VideoCtrl::RefreshTime()
{
	STime kkk;
	kkk.mstime = time;
	float dur=GetDuration();
	float val=(dur>0)? kkk.mstime/dur : 0.0;



	if(isFullscreen){
		TD->vslider->SetValue(val);
		if(!pbar){return;}
		STime kkk1;
		kkk1.mstime=dur;
		pbtime=kkk.raw(TMP)+" / "+kkk1.raw(TMP);
		DrawProgBar();
	}
	else{
		vslider->SetValue(val);
		vslider->Update();
		wxString times;
		times<<kkk.raw(SRT)<<";  ";
		TabPanel *pan=(TabPanel*)GetParent();
		if(!IsDshow){
			times<<lastframe<<";  ";
			if(VFF){
				if(VFF->KeyFrames.Index(time) != -1){
					shownKeyframe=true;
					mstimes->SetForegroundColour(WindowWarningElements);
				}else if(shownKeyframe){
					shownKeyframe=false;
					mstimes->SetForegroundColour(WindowText);
				}
			}
		}
		if(pan->editor){
			Dialogue *line=pan->Edit->line;
			int sdiff=kkk.mstime - ZEROIT(line->Start.mstime);
			int ediff=kkk.mstime - ZEROIT(line->End.mstime);
			times<<sdiff<<" ms, "<<ediff<<" ms";
			pan->Grid->RefreshIfVisible(kkk.mstime);
		}
		mstimes->SetValue(times);
		mstimes->Update();
		
	}

}

void VideoCtrl::OnCopyCoords(const wxPoint &pos)
{
	int w, h;
	GetClientSize(&w,&h);
	int nx=0, ny=0;
	Kai->GetTab()->Grid->GetASSRes(&nx,&ny);
	wspx=(float)nx/(float)(w-1);
	wspy=(float)ny/(float)(h - panelHeight - 1);
	int posx=(float)pos.x*wspx;
	int posy=(float)pos.y*wspy;
	wxString poss;
	poss<<posx<<", "<<posy;
	if (wxTheClipboard->Open())
	{
		wxTheClipboard->SetData( new wxTextDataObject(poss) );
		wxTheClipboard->Close();
	}
}

void VideoCtrl::ChangeButtonBMP(bool play)
{
	if(isFullscreen){TD->bpause->ChangeBitmap(play);}
	else{bpause->ChangeBitmap(play);}
}

void VideoCtrl::NextChap()
{
	if(chaps.size()<1){return;}
	int vrtime=Tell();
	for(int j=0; j<(int)chaps.size(); j++){

		int ntime= (j>=(int)chaps.size()-1)? INT_MAX : chaps[(j+1)].time;
		if(ntime>=vrtime){

			int jj=(j>=(int)chaps.size()-1 || (j==0 && chaps[0].time>=vrtime))? 0 : j+1;
			if(jj==prevchap){if(jj>=(int)chaps.size()-1){jj=0;}else{jj++;}}
			Seek(chaps[jj].time,false);

			prevchap=jj;
			break;
		}
	}
}

void VideoCtrl::PrevChap()
{
	if(chaps.size()<1){return;}
	int vrtime=Tell();
	for(int j=0; j<(int)chaps.size(); j++){
		int ntime= (j>=(int)chaps.size()-1)? INT_MAX : chaps[(j+1)].time;
		if(ntime>=vrtime){
			int jj=(j<1)? 0 : j-1;
			if(jj==prevchap){if(jj<1){jj=chaps.size()-1;}else{jj--;}}
			Seek(chaps[jj].time,false);
			prevchap=jj;
			break;
		}
	}
}

void VideoCtrl::ConnectAcc(int id)
{
	Connect(id,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&VideoCtrl::OnAccelerator);
}

void VideoCtrl::ChangeStream()
{
	if(!IsDshow){return;}
	wxArrayString enabled;
	Options.GetTable(AcceptedAudioStream,enabled,";");
	if(enabled.size()<1){return;}
	wxArrayString streams=GetStreams();
	//int firstSubsStream = -1;
	int numofastreams=0;
	for(int i=streams.size()-1; i>=0; i--){
		if(streams[i][0]=='A' && streams[i][0]=='a'){
			streams[i] = streams[i].AfterFirst(' ').Lower();
			numofastreams++;
			continue;
		}
		/*else if (firstSubsStream == -1 && streams[i][0] == 'S' || streams[i][0] == 's'){
			firstSubsStream = i;
		}*/
		streams[i] = "";
	}
	if(numofastreams>1){
		for(int i=0; i<(int)streams.size(); i++){
			if(streams[i]==""){continue;}
			for(int j=0; j<(int)enabled.size(); j++){
				if(streams[i].Lower().find(enabled[j])!=-1){
					if(streams[i].AfterLast(' ')=="0"){
						EnableStream((long)i);
					}
					return;
				}
			}
		}
	}
	/*if (firstSubsStream != -1){
		TabPanel *tab = Kai->GetTab();
		if (!tab->editor && Kai->FindFile(tab->VideoPath, false, false).empty()){
		EnableStream((long)firstSubsStream);
		}
		}*/
}

BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor,HDC hdcMonitor,LPRECT lprcMonitor,LPARAM dwData)
{
	VideoCtrl *vb=(VideoCtrl *) dwData;
	WinStruct<MONITORINFO> monitorinfo;
	if(!GetMonitorInfo(hMonitor, &monitorinfo)){
		wxLogStatus(_("Nie można pobrać informacji o monitorze"));
		return TRUE;
	}
	//podstawowy monitor ma być pierwszy w tablicy
	if(monitorinfo.dwFlags==MONITORINFOF_PRIMARY){
		vb->MonRects.insert(vb->MonRects.begin(), monitorinfo.rcMonitor);
		return TRUE;
	}
	vb->MonRects.push_back(monitorinfo.rcMonitor);
	return TRUE;
}

wxRect VideoCtrl::GetMonitorRect(int wmonitor){
	MonRects.clear();
	EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, (LPARAM)this);
	wxRect rt(MonRects[0].left, MonRects[0].top,  abs(MonRects[0].right - MonRects[0].left), abs(MonRects[0].bottom - MonRects[0].top));
	if(wmonitor==-1||MonRects.size()==1){return rt;}
	else if(wmonitor==0){
		wxRect rect=Kai->GetRect();
		int x= (rect.width/2)+rect.x;
		int y= (rect.height/2)+rect.y;
		for(size_t i=0; i<MonRects.size(); i++){
			if(MonRects[i].left <= x && x < MonRects[i].right && MonRects[i].top <= y && y < MonRects[i].bottom){
				return wxRect(MonRects[i].left, MonRects[i].top,  abs(MonRects[i].right - MonRects[i].left), abs(MonRects[wmonitor].bottom - MonRects[wmonitor].top));
			}
		}
	}else{
		return wxRect(MonRects[wmonitor].left, MonRects[wmonitor].top, abs(MonRects[wmonitor].right - MonRects[wmonitor].left), abs(MonRects[wmonitor].bottom - MonRects[wmonitor].top));
	}
	return rt;
}

void VideoCtrl::OnChangeVisual(wxCommandEvent &evt)
{
	EditBox *eb = Kai->GetTab()->Edit;
	int vis = evt.GetInt();

	if(vis==eb->Visual){return;}
	if(Vclips && vis == 0){ 
		SetVisual(true); 
		if(vToolbar->ClipToolsShown()||vToolbar->MoveToolsShown()){vToolbar->ShowTools(false,vToolbar->ClipToolsShown());}
	}else if( vis != eb->Visual ){
		if(vis==VECTORCLIP || vis==VECTORDRAW || eb->Visual==VECTORCLIP 
			|| eb->Visual==VECTORDRAW || vis==MOVEALL || eb->Visual==MOVEALL){
				bool vector = (vis==VECTORCLIP || vis==VECTORDRAW || vis==MOVEALL);
				bool isMove =  (vis==MOVEALL || (eb->Visual==MOVEALL && vis!=VECTORCLIP && vis!=VECTORDRAW));
				if(isFullscreen && TD){TD->vToolbar->ShowTools(vector, !isMove);}
				else{vToolbar->ShowTools(vector, !isMove) ;}
		}
		eb->Visual = vis;
		SetVisual();
		if(vis==MOVEALL){Vclips->ChangeTool(vToolbar->GetMoveToggled());}
		if(!hasArrow){SetCursor(wxCURSOR_ARROW);hasArrow=true;}
	}

}

bool VideoCtrl::SetBackgroundColour(const wxColour &col)
{
	panel->SetBackgroundColour(col);
	return true;
}

BEGIN_EVENT_TABLE(VideoCtrl,wxWindow)
	EVT_SIZE(VideoCtrl::OnSize)
	EVT_MOUSE_EVENTS(VideoCtrl::OnMouseEvent)
	EVT_KEY_DOWN(VideoCtrl::OnKeyPress)
	EVT_PAINT(VideoCtrl::OnPaint)
	EVT_TIMER(idvtime, VideoCtrl::OnPlaytime)
	EVT_TIMER(ID_IDLE, VideoCtrl::OnIdle)
	EVT_ERASE_BACKGROUND(VideoCtrl::OnErase)
	EVT_BUTTON(23333,VideoCtrl::OnEndFile)
	EVT_MOUSE_CAPTURE_LOST(VideoCtrl::OnLostCapture)
	END_EVENT_TABLE()