//  Copyright (c) 2018, Marcin Drob

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


#include "FFMS2Player.h"
#include "kainoteApp.h"
#include "VideoFfmpeg.h"
#include "CsriMod.h"
#include "DshowRenderer.h"
#include "OpennWrite.h"

bool FFMS2Player::OpenFile(const wxString &fname, wxString *textsubs, bool vobsub, bool changeAudio)
{
	wxMutexLocker lock(mutexOpenFile);
	kainoteApp *Kaia = (kainoteApp*)wxTheApp;
	TabPanel *tab = ((TabPanel*)videoWindow->GetParent());
	VideoFfmpeg *tmpvff = NULL;
	if (vstate == Playing){ videoWindow->Stop(); }

	
	bool success;
	tmpvff = new VideoFfmpeg(fname, videoWindow, &success);
	//this is safe mode, when new video not load, 
	//the last opened will not be released
	if (!success || !tmpvff){
		SAFE_DELETE(tmpvff);
		return false;
	}
	//when loading only audio do not remove video
	if (tmpvff->width < 0 && tmpvff->GetSampleRate() > 0){
		VideoFfmpeg *tmp = VFF;
		VFF = tmpvff;
		Kaia->Frame->OpenAudioInTab(tab, 40000, fname);
		player = tab->Edit->ABox->audioDisplay;
		VFF = tmp;
		return false;
	}
	
	SAFE_DELETE(VFF);

	time = 0;
	numframe = 0;

	VFF = tmpvff;
	d3dformat = D3DFMT_X8R8G8B8;
	vformat = RGB32;
	vwidth = VFF->width;
	vheight = VFF->height;
	fps = VFF->fps;
	ax = VFF->arwidth;
	ay = VFF->arheight;
	if (vwidth % 2 != 0){ vwidth++; }
	pitch = vwidth * 4;
	if (changeAudio){
		if (VFF->GetSampleRate() > 0){
			Kaia->Frame->OpenAudioInTab(tab, 40000, fname);
			player = tab->Edit->ABox->audioDisplay;
		}
		else if (player){ Kaia->Frame->OpenAudioInTab(tab, CloseAudio, ""); }
	}
	if (!VFF || VFF->width < 0){
		return false;
	}
	
	frameDuration = (1000.0f / fps);
	if (ay == 0 || ax == 0){ AR = 0.0f; }
	else{ AR = (float)ay / (float)ax; }

	mainStreamRect.bottom = vheight;
	mainStreamRect.right = vwidth;
	mainStreamRect.left = 0;
	mainStreamRect.top = 0;
	if (datas){ delete[] datas; datas = NULL; }
	datas = new byte[vheight*pitch];

	if (!InitDX()){ return false; }
	UpdateRects();

	if (!framee){ framee = new csri_frame; }
	if (!format){ format = new csri_fmt; }
	for (int i = 1; i < 4; i++){
		framee->planes[i] = NULL;
		framee->strides[i] = NULL;
	}

	framee->pixfmt = (vformat == 5) ? CSRI_F_YV12A : (vformat == 3) ? CSRI_F_YV12 : (vformat == 2) ? CSRI_F_YUY2 : CSRI_F_BGR_;

	format->width = vwidth;
	format->height = vheight;
	format->pixfmt = framee->pixfmt;
	format->fps = 25.0f;

	if (!vobsub){
		OpenSubs(textsubs, false);
	}
	else{
		SAFE_DELETE(textsubs);
		OpenSubs(0, false);
	}
	vstate = (PlaybackState)Stopped;
	
	VFF->GetChapters(&chapters);

	if (Visual){
		Visual->SizeChanged(wxRect(backBufferRect.left, backBufferRect.top, backBufferRect.right, backBufferRect.bottom), lines, overlayFont, d3device);
	}
	return true;
}

bool FFMS2Player::Play(int end)
{
	SetThreadExecutionState(ES_DISPLAY_REQUIRED | ES_CONTINUOUS);
	VideoCtrl *vb = videoWindow;
	if (!(vb->IsShown() || (vb->TD && vb->TD->IsShown()))){ return false; }
	TabPanel* pan = (TabPanel*)vb->GetParent();
	if (hasVisualEdition){
		OpenSubs(pan->Grid->SaveText(), false, true);
		SAFE_DELETE(Visual->dummytext);
		hasVisualEdition = false;
	}
	else if (hasDummySubs && pan->editor){
		OpenSubs(pan->Grid->SaveText(), false, true);
	}

	if (end > 0){ playend = end; }
	else{ playend = GetDuration(); }
	
	vstate = (PlaybackState)Playing;

	time = VFF->Timecodes[numframe];
	lasttime = timeGetTime() - time;
	if (player){ player->Play(time, -1, false); }
	VFF->Play();

	return true;
}

bool FFMS2Player::Pause()
{
	if (vstate == Playing){
		SetThreadExecutionState(ES_CONTINUOUS);
		vstate = (PlaybackState)Paused;
		if (player){ player->Stop(false); }
	}
	else{
		Play();
	}
	return true;
}

bool FFMS2Player::Stop()
{
	if (vstate == Playing){
		SetThreadExecutionState(ES_CONTINUOUS);
		vstate = (PlaybackState)Stopped;
		if (player){
			player->Stop();
			playend = GetDuration();
		}
		time = 0;
		return true;
	}
	return false;
}

void FFMS2Player::SetPosition(int _time, bool starttime/*=true*/, bool corect/*=true*/)
{
	if (vstate == Paused)
		VFF->SetPosition(_time, starttime);
	else
		SetFFMS2Position(_time, starttime);
}

void FFMS2Player::SetFFMS2Position(int _time, bool starttime){
	TabPanel* tab = (TabPanel*)videoWindow->GetParent();
	bool playing = vstate == Playing;
	numframe = VFF->GetFramefromMS(_time, (time > _time) ? 0 : numframe);
	time = VFF->Timecodes[numframe];
	if (!starttime){
		numframe--;
		if (time >= _time){ numframe--; time = VFF->Timecodes[numframe]; }
	}

	lasttime = timeGetTime() - time;
	playend = GetDuration();

	if (hasVisualEdition){
		SAFE_DELETE(Visual->dummytext);
		if (Visual->Visual == VECTORCLIP){
			Visual->SetClip(Visual->GetVisual(), true, false, false);
		}
		else{
			OpenSubs((playing) ? tab->Grid->SaveText() : tab->Grid->GetVisible(), true, playing);
			if (playing){ hasVisualEdition = false; }
		}
	}
	else if (hasDummySubs){
		OpenSubs((playing) ? tab->Grid->SaveText() : tab->Grid->GetVisible(), true, playing);
	}
	if (vstate == Playing){
		if (player){
			player->player->SetCurrentPosition(player->GetSampleAtMS(time));
		}
	}
	else{
		if (player){ player->UpdateImage(true, true); }
		//Render(true, false);
		VFF->Render();
		videoWindow->RefreshTime();
	}
}

bool FFMS2Player::OpenSubs(wxString *textsubs, bool redraw, bool fromFile)
{
	wxCriticalSectionLocker lock(mutexRender);
	if (instance) csri_close(instance);
	instance = NULL;

	if (!textsubs) {
		hasDummySubs = true;
		return true;
	}

	if (hasVisualEdition && Visual->Visual == VECTORCLIP && Visual->dummytext){
		wxString toAppend = Visual->dummytext->Trim().AfterLast(L'\n');
		if (fromFile){
			OpenWrite ow(*textsubs, false);
			ow.PartFileWrite(toAppend);
			ow.CloseFile();
		}
		else{
			(*textsubs) << toAppend;
		}
	}

	hasDummySubs = !fromFile;

	wxScopedCharBuffer buffer = textsubs->mb_str(wxConvUTF8);
	int size = strlen(buffer);


	// Select renderer
	vobsub = csri_renderer_default();
	if (!vobsub){ KaiLog(_("CSRI odmówi³o pos³uszeñstwa.")); delete textsubs; return false; }

	instance = (fromFile) ? csri_open_file(vobsub, buffer, NULL) : csri_open_mem(vobsub, buffer, size, NULL);
	if (!instance){ KaiLog(_("Instancja VobSuba nie utworzy³a siê.")); delete textsubs; return false; }

	if (!format || csri_request_fmt(instance, format)){
		KaiLog(_("CSRI nie obs³uguje tego formatu."));
		csri_close(instance);
		instance = NULL;
		delete textsubs; return false;
	}

	delete textsubs;
	return true;
}