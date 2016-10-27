// Copyright (c) 2005, Rodrigo Braz Monteiro
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of the Aegisub Group nor the names of its contributors
//     may be used to endorse or promote products derived from this software
//     without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// -----------------------------------------------------------------------------
//
// AEGISUB
//
// Website: http://aegisub.cellosoft.com
// Contact: mailto:zeratul@cellosoft.com
//


///////////
// Headers

#include "Config.h"

//#include <wx/tglbtn.h>
#include <wx/filename.h>
#include <math.h>
#include <vector>
#include "AudioDisplay.h"
#include "EditBox.h"

#include "Config.h"
#include "AudioBox.h"

#include "ColorSpace.h"
#include "Hotkeys.h"
#include "Grid.h"
#include "kainoteApp.h"


int64_t abs64(int64_t input) {
	if (input < 0) return -input;
	return input;
}




///////////////
// Constructor
AudioDisplay::AudioDisplay(wxWindow *parent)
	: wxWindow (parent, -1, wxDefaultPosition, wxSize(100,100), wxSUNKEN_BORDER | wxWANTS_CHARS , _T("Audio Display"))
{
	// Set variables
	origImage = NULL;
	spectrumDisplay = NULL;
	spectrumDisplaySelected = NULL;
	spectrumRenderer = NULL;
	ScrollBar = NULL;
	karaoke = NULL;
	peak = NULL;
	min = NULL;
	dialogue=NULL;
	cursorPaint=false;
	defCursor = true;
	karaAuto=Options.GetBool(_T("Audio Karaoke Split Mode"));
	hasKara =Options.GetBool(_T("Audio Karaoke"));
	if(hasKara){karaoke=new Karaoke(this);}
	hasSel = hasMark = false;
	diagUpdated = false;
	NeedCommit = false;
	loaded = false;
	blockUpdate = false;
	holding = false;
	draggingScale = false;
	inside=ownProvider=false;
	whichsyl=0;
	letter=-1;
	Grabbed=-1;
	Position = 0;
	PositionSample = 0;
	oldCurPos = 0;
	scale = 1.0f;
	provider = NULL;
	player = NULL;
	hold = 0;
	samples = 0;
	samplesPercent = 100;
	selMark=0;
	curMarkMS=0;
	hasFocus = (wxWindow::FindFocus() == this);
	needImageUpdate = false;
	needImageUpdateWeak = true;
	playingToEnd = false;

	// Init
	UpdateTimer.SetOwner(this,Audio_Update_Timer);
	GetClientSize(&w,&h);
	h -= 20;
	SetSamplesPercent((hasKara)? 30 : 50,false);

	// Set cursor
	//wxCursor cursor(wxCURSOR_BLANK);
	//SetCursor(cursor);


}


//////////////
// Destructor
AudioDisplay::~AudioDisplay() {
	if (player) {player->CloseStream();delete player;}
	if (ownProvider && provider) {delete provider;provider = NULL;}
	if (origImage) {delete origImage;}
	if (karaoke){delete karaoke;}
	if(spectrumRenderer){delete spectrumRenderer;};
	if(spectrumDisplay){delete spectrumDisplay;}
	if(spectrumDisplay){delete spectrumDisplaySelected;}
	if(peak){delete[] peak;
	delete[] min;}
	

	player = NULL;
	origImage = NULL;
	karaoke = NULL;
	spectrumRenderer = NULL;
	spectrumDisplay = NULL;
	spectrumDisplaySelected = NULL;
	peak = NULL;
	min = NULL;
}


/////////
// Reset
void AudioDisplay::Reset() {
	hasSel = false;
	diagUpdated = false;
	NeedCommit = false;
}


////////////////
// Update image
void AudioDisplay::UpdateImage(bool weak) {
	// Update samples
	UpdateSamples();

	// Set image as needing to be redrawn
	needImageUpdate = true;
	if (weak == false && needImageUpdateWeak == true) {
		needImageUpdateWeak = false;
	}
	Refresh(false);
}

void AudioDisplay::DoUpdateImage() {
	// Loaded?
	if (!loaded || !provider) return;

	// Needs updating?
	if (!needImageUpdate) return;
	bool weak = needImageUpdateWeak;

	// Prepare bitmap
	int timelineHeight = 20;
	int displayH = h+timelineHeight;
	if (origImage) {
		if (origImage->GetWidth() != w || origImage->GetHeight() != displayH) {
			delete origImage;
			origImage = NULL;
		}
	}

	// Options
	bool draw_boundary_lines = Options.GetBool(_T("Audio Draw Secondary Lines"));
	bool draw_selection_background = Options.GetBool(_T("Audio Draw Selection Background"));
	bool drawKeyframes = Options.GetBool(_T("Audio Draw Keyframes"));

	// Invalid dimensions
	if (w == 0 || displayH == 0) return;

	// New bitmap
	if (!origImage) origImage = new wxBitmap(w,displayH,-1);

	// Is spectrum?
	bool spectrum = false;
	if (provider && Options.GetBool(_T("Audio Spectrum"))) {
		spectrum = true;
	}

	// Draw image to be displayed
	wxMemoryDC dc;
	dc.SelectObject(*origImage);

	// Black background
	dc.SetPen(*wxTRANSPARENT_PEN);
	dc.SetBrush(wxBrush(Options.GetColour(_T("Audio Background"))));
	dc.DrawRectangle(0,0,w,h);

	// Selection position
	//hasSel = false;
	//hasKaraoke = karaoke->enabled;
	selStart = 0;
	selEnd = 0;
	lineStart = 0;
	lineEnd = 0;
	selStartCap = 0;
	selEndCap = 0;
	int64_t drawSelStart = 0;
	int64_t drawSelEnd = 0;

	GetDialoguePos(lineStart,lineEnd,false);
	hasSel = true;

	GetDialoguePos(selStartCap,selEndCap,true);
	selStart = lineStart;
	selEnd = lineEnd;
	drawSelStart = lineStart;
	drawSelEnd = lineEnd;

	// Draw selection bg
	if (hasSel && drawSelStart < drawSelEnd && draw_selection_background) {
		if (NeedCommit ) dc.SetBrush(wxBrush(Options.GetColour(_T("Audio Selection Background Modified"))));
		else dc.SetBrush(wxBrush(Options.GetColour(_T("Audio Selection Background"))));
		dc.DrawRectangle(drawSelStart,0,drawSelEnd-drawSelStart,h);
	}

	// Draw spectrum
	if (spectrum) {
		DrawSpectrum(dc,weak);
	}


	// Waveform
	else if (provider) {
		DrawWaveform(dc,weak);
	}

	// Nothing
	else {
		dc.DrawLine(0,h/2,w,h/2);
	}

	// Draw seconds boundaries
	if (draw_boundary_lines) {
		int64_t start = Position*samples;
		int rate = provider->GetSampleRate();
		int pixBounds = rate / samples;
		dc.SetPen(wxPen(Options.GetColour(_T("Audio Seconds Boundaries")),1,wxDOT));//
		if (pixBounds >= 8) {
			for (int x=0;x<w;x++) {
				if (((x*samples)+start) % rate < samples) {
					dc.DrawLine(x,0,x,h);
				}
			}
		}
	}

	// Draw previous line
	DrawInactiveLines(dc);



	if (hasSel) {
		// Draw boundaries
		//if (true) {
		// Draw start boundary
		int selWidth = Options.GetInt(_T("Audio Line Boundaries Thickness"));
		dc.SetPen(wxPen(Options.GetColour(_T("Audio Line Boundary Start"))));
		dc.SetBrush(wxBrush(Options.GetColour(_T("Audio Line Boundary Start"))));
		dc.DrawRectangle(lineStart-selWidth/2+1,0,selWidth,h);
		wxPoint points1[3] = { wxPoint(lineStart,0), wxPoint(lineStart+10,0), wxPoint(lineStart,10) };
		wxPoint points2[3] = { wxPoint(lineStart,h-1), wxPoint(lineStart+10,h-1), wxPoint(lineStart,h-11) };
		dc.DrawPolygon(3,points1);
		dc.DrawPolygon(3,points2);

		// Draw end boundary
		dc.SetPen(wxPen(Options.GetColour(_T("Audio Line Boundary End"))));
		dc.SetBrush(wxBrush(Options.GetColour(_T("Audio Line Boundary End"))));
		dc.DrawRectangle(lineEnd-selWidth/2+1,0,selWidth,h);
		wxPoint points3[3] = { wxPoint(lineEnd,0), wxPoint(lineEnd-10,0), wxPoint(lineEnd,10) };
		wxPoint points4[3] = { wxPoint(lineEnd,h-1), wxPoint(lineEnd-10,h-1), wxPoint(lineEnd,h-11) };
		dc.DrawPolygon(3,points3);
		dc.DrawPolygon(3,points4);
		//}

		// Draw karaoke
		if (hasKara) {
			dc.SetPen(wxPen(Options.GetColour("Audio Syllable Boundaries")));
			dc.SetTextForeground(Options.GetColour("Audio Syllable Text"));
			dc.SetTextBackground(Options.GetColour("Audio Syllable Boundaries"));
			dc.SetBackgroundMode(wxSOLID);

			int karstart=selStart;
			wxFont karafont(11,wxDEFAULT,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_BOLD,false,_T("Verdana"));
			dc.SetFont(karafont);
			wxString acsyl;
			for(size_t i=0; i<karaoke->syls.size(); i++)
			{
				acsyl=karaoke->syls[i];
				int fw, fh;
				dc.GetTextExtent(acsyl,&fw, &fh, 0, 0, &karafont);

				int XX=GetXAtMS(karaoke->syltimes[i]);
				if(XX>=0){dc.DrawLine(XX,0,XX,h);}
				int center=((XX-karstart)-fw)/2;
				dc.DrawText(acsyl,center+karstart,0);

				//obramowanie aktywynej sylaby
				if(i==whichsyl){
					dc.SetPen(Options.GetColour("Audio Syllable Text"));
					dc.SetBrush(*wxTRANSPARENT_BRUSH);
					dc.DrawRectangle(karstart+2,1,XX-karstart-2,h-2);
					dc.SetPen(wxPen(Options.GetColour("Audio Syllable Boundaries")));
				}

				karstart=XX;
			}
			dc.SetBackgroundMode(wxTRANSPARENT);
		}
	}

	// Modified text
	if (NeedCommit || selStart > selEnd) {
		dc.SetFont(wxFont(9,wxDEFAULT,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_BOLD,false,_T("Verdana"))); // FIXME: hardcoded font name
		dc.SetTextForeground(wxColour(255,0,0));
		if (selStart <= selEnd) {
			dc.DrawText(_T("Zmodyfikowano"),4,4);
		}
		else {
			dc.DrawText(_T("Czas ujemny"),4,4);
		}
	}

	DrawTimescale(dc);


	if(hasMark){
		selMark=GetXAtMS(curMarkMS);
		if(selMark>=0&&selMark<w){
			wxColour kol=Options.GetColour(_T("Audio Line Boundary Mark"));
			dc.SetPen(wxPen(kol));
			dc.SetBrush(wxBrush(kol));
			dc.DrawRectangle(selMark,0,2,h);
			dc.SetTextForeground(kol);
			STime time(curMarkMS);
			wxString text=time.raw();
			wxFont font(10,wxFONTFAMILY_DEFAULT,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_BOLD,false,_T("Verdana"));
			dc.SetFont(font);
			int dx,dy;
			dc.GetTextExtent(text,&dx, &dy, 0, 0, &font);
			dx=selMark-(dx/2);
			dy=h-dy-2;
			dc.DrawText(text,dx+1,dy-1);
			dc.DrawText(text,dx+1,dy+1);
			dc.DrawText(text,dx-1,dy-1);
			dc.DrawText(text,dx-1,dy+1);
			dc.SetTextForeground(wxColour(255,255,255));
			dc.DrawText(text,dx,dy);

		}
	}

	// Draw current frame
	if (Options.GetBool(_T("Audio Draw Video Position"))) {
		VideoCtrl *Video= Notebook::GetTab()->Video;
		if (Video->GetState()==Paused) {
			dc.SetPen(wxPen(Options.GetColour(_T("Audio Play Cursor")),2,wxLONG_DASH));
			int x = GetXAtMS(Video->Tell());
			//wxLogStatus("xpos %i", x);
			dc.DrawLine(x,0,x,h);
		}
	}

	// Draw keyframes
	if (drawKeyframes && provider->KeyFrames.size()>0) {
		DrawKeyframes(dc);
	}
	// Draw focus border
	if (hasFocus) {
		dc.SetPen(*wxGREEN_PEN);
		dc.SetBrush(*wxTRANSPARENT_BRUSH);
		dc.DrawRectangle(0,0,w,h);
	}

	// Done
	needImageUpdate = false;
	needImageUpdateWeak = true;
}


///////////////////////
// Draw Inactive Lines
void AudioDisplay::DrawInactiveLines(wxDC &dc) {
	// Check if there is anything to do
	int shadeType = Options.GetInt(_T("Audio Inactive Lines Display Mode"));
	if (shadeType == 0) return;

	// Spectrum?
	bool spectrum = false;
	if (provider && Options.GetBool(_T("Audio Spectrum"))) {
		spectrum = true;
	}

	// Set options
	dc.SetBrush(wxBrush(Options.GetColour(_T("Audio Line Boundary Inactive Line"))));
	int selWidth = Options.GetInt(_T("Audio Line Boundaries Thickness"));
	Dialogue *shade;
	int shadeX1,shadeX2;
	int shadeFrom,shadeTo;

	// Only previous
	if (shadeType == 1) {
		shadeFrom = this->line_n-1;
		shadeTo = shadeFrom+3;
	}

	// All
	else {
		shadeFrom = 0;
		shadeTo = grid->GetCount();
	}

	for (int j=shadeFrom;j<shadeTo;j++) {
		if (j == line_n) continue;
		if (j < 0 || j>= grid->GetCount() ) continue;
		shade = grid->GetDial(j);


		// Get coordinates
		shadeX1 = GetXAtMS(shade->Start.mstime);
		shadeX2 = GetXAtMS(shade->End.mstime);
		if (shadeX2 < 0 || shadeX1 > w) continue;

		// Draw over waveform
		if (!spectrum) {
			// Selection
			int selX1 = MAX(0,GetXAtMS(curStartMS));
			int selX2 = MIN(w,GetXAtMS(curEndMS));

			// Get ranges (x1->x2, x3->x4).
			int x1 = MAX(0,shadeX1);
			int x2 = MIN(w,shadeX2);
			int x3 = MAX(x1,selX2);
			int x4 = MAX(x2,selX2);

			// Clip first range
			x1 = MIN(x1,selX1);
			x2 = MIN(x2,selX1);

			// Set pen and draw
			dc.SetPen(wxPen(Options.GetColour(_T("Audio Waveform Inactive"))));
			for (int i=x1;i<x2;i++) dc.DrawLine(i,peak[i],i,min[i]-1);
			for (int i=x3;i<x4;i++) dc.DrawLine(i,peak[i],i,min[i]-1);

		}
		// Draw boundaries
		dc.SetPen(wxPen(Options.GetColour(_T("Audio Line Boundary Inactive Line"))));
		dc.DrawRectangle(shadeX1-selWidth/2+1,0,selWidth,h);
		dc.DrawRectangle(shadeX2-selWidth/2+1,0,selWidth,h);
	}

}



//////////////////
// Draw timescale
void AudioDisplay::DrawTimescale(wxDC &dc) {
	// Set size
	int timelineHeight = 20;

	// Set colours
	dc.SetBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
	dc.SetPen(*wxTRANSPARENT_PEN);
	dc.DrawRectangle(0,h,w,timelineHeight);
	dc.SetPen(wxSystemSettings::GetColour(wxSYS_COLOUR_3DLIGHT));
	dc.DrawLine(0,h,w,h);
	dc.SetPen(wxSystemSettings::GetColour(wxSYS_COLOUR_3DHIGHLIGHT));
	dc.DrawLine(0,h+1,w,h+1);
	dc.SetPen(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT));
	dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT));
	wxFont scaleFont;
	scaleFont.SetFaceName(_T("Tahoma")); // FIXME: hardcoded font name
	if (!scaleFont.IsOk())
		scaleFont.SetFamily(wxFONTFAMILY_SWISS );
	scaleFont.SetPointSize(8);
	dc.SetFont(scaleFont);

	// Timescale ticks
	int64_t start = Position*samples;
	int rate = provider->GetSampleRate();
	for (int i=1;i<32;i*=2) {
		int pixBounds = rate / (samples * 4 / i);
		if (pixBounds >= 8) {
			for (int x=0;x<w;x++) {
				int64_t pos = (x*samples)+start;
				// Second boundary
				if (pos % rate < samples) {
					dc.DrawLine(x,h+2,x,h+8);

					// Draw text
					wxCoord textW,textH;
					int hr = 0;
					int m = 0;
					int s = pos/rate;
					while (s >= 3600) {
						s -= 3600;
						hr++;
					}
					while (s >= 60) {
						s -= 60;
						m++;
					}
					wxString text;
					if (hr) text = wxString::Format(_T("%i:%02i:%02i"),hr,m,s);
					else if (m) text = wxString::Format(_T("%i:%02i"),m,s);
					else text = wxString::Format(_T("%i"),s);
					dc.GetTextExtent(text,&textW,&textH,NULL,NULL,&scaleFont);
					dc.DrawText(text,MAX(0,x-textW/2)+1,h+8);
				}

				// Other
				else if (pos % (rate / 4 * i) < samples) {
					dc.DrawLine(x,h+2,x,h+5);
				}
			}
			break;
		}
	}
}


////////////
// Waveform
void AudioDisplay::DrawWaveform(wxDC &dc,bool weak) {
	// Prepare Waveform
	if (!weak || peak == NULL || min == NULL) {
		if (peak) delete[] peak;
		if (min) delete[] min;
		peak = new int[w];
		min = new int[w];
	}

	// Get waveform
	if (!weak) {
		provider->GetWaveForm(min,peak,Position*samples,w,h,samples,scale);
	}

	// Draw pre-selection
	if (!hasSel) selStartCap = w;
	dc.SetPen(wxPen(Options.GetColour(_T("Audio Waveform"))));//Options.GetColour(_T("Audio Waveform"))
	for (int64_t i=0;i<selStartCap;i++) {
		dc.DrawLine(i,peak[i],i,min[i]-1);
	}

	if (hasSel) {
		// Draw selection
		if (Options.GetBool(_T("Audio Draw Selection Background"))) {
			if (NeedCommit) dc.SetPen(wxPen(Options.GetColour(_T("Audio Waveform Modified"))));
			else dc.SetPen(wxPen(Options.GetColour(_T("Audio Waveform Selected"))));
		}
		for (int64_t i=selStartCap;i<selEndCap;i++) {
			dc.DrawLine(i,peak[i],i,min[i]-1);
		}

		// Draw post-selection
		dc.SetPen(wxPen(Options.GetColour(_T("Audio Waveform"))));
		for (int64_t i=selEndCap;i<w;i++) {
			dc.DrawLine(i,peak[i],i,min[i]-1);
		}
	}
}


//////////////////////////
// Draw spectrum analyzer
void AudioDisplay::DrawSpectrum(wxDC &finaldc, bool weak) {
	if (!weak || !spectrumDisplay || spectrumDisplay->GetWidth() != w || spectrumDisplay->GetHeight() != h) {
		if (spectrumDisplay) {
			delete spectrumDisplay;
			if(spectrumDisplaySelected){delete spectrumDisplaySelected;
			spectrumDisplaySelected = 0;}
			spectrumDisplay = 0;

		}
		weak = false;
	}

	if (!weak) {
		if (!spectrumRenderer)
			spectrumRenderer = new AudioSpectrum(provider);
		spectrumRenderer->SetScaling(scale);

		unsigned char *img = (unsigned char *)malloc(h*w*3); // wxImage requires using malloc

		// Use a slightly slower, but simple way
		// Always draw the spectrum for the entire width
		// Hack: without those divs by 2 the display is horizontally compressed
		spectrumRenderer->RenderRange(Position*samples, (Position+w)*samples, false, img, w, h, samplesPercent);

		// The spectrum bitmap will have been deleted above already, so just make a new one
		wxImage imgobj(w, h, img, false);
		spectrumDisplay = new wxBitmap(imgobj);
	}

	if (hasSel && selStartCap < selEndCap && !spectrumDisplaySelected) {
		// There is a visible selection and we don't have a rendered one
		// This should be done regardless whether we're "weak" or not
		// Assume a few things were already set up when things were first rendered though
		unsigned char *img = (unsigned char *)malloc(h*w*3);
		spectrumRenderer->RenderRange(Position*samples, (Position+w)*samples, true, img, w, h, samplesPercent);
		wxImage imgobj(w, h, img, false);
		spectrumDisplaySelected = new wxBitmap(imgobj);
	}

	// Draw
	wxMemoryDC dc;
	dc.SelectObject(*spectrumDisplay);
	finaldc.Blit(0,0,w,h,&dc,0,0);

	if (hasSel && spectrumDisplaySelected && selStartCap < selEndCap) {
		dc.SelectObject(*spectrumDisplaySelected);
		finaldc.Blit(selStartCap, 0, selEndCap-selStartCap, h, &dc, selStartCap, 0);
	}
}

//////////////////////////
// Get selection position
void AudioDisplay::GetDialoguePos(int64_t &selStart,int64_t &selEnd, bool cap) {
	selStart = GetXAtMS(curStartMS);
	selEnd = GetXAtMS(curEndMS);

	if (cap) {
		if (selStart < 0) selStart = 0;
		if (selEnd < 0) selEnd = 0;
		if (selStart >= w) selStart = w-1;
		if (selEnd >= w) selEnd = w-1;
	}
}




//////////
// Update
void AudioDisplay::Update() {
	if (blockUpdate) return;
	if (loaded) {
		if (Options.GetBool(_T("Audio Autoscroll")))
			MakeDialogueVisible();
		else
			UpdateImage(true);
	}
}


//////////////////////
// Recreate the image
void AudioDisplay::RecreateImage() {
	GetClientSize(&w,&h);
	h -= 20;
	delete origImage;
	origImage = NULL;
	UpdateImage(false);
}


/////////////////////////
// Make dialogue visible
void AudioDisplay::MakeDialogueVisible(bool force) {
	// Variables
	int startShow=0, endShow=0;
	// In karaoke mode the syllable and as much as possible towards the end of the line should be shown

	GetTimesSelection(startShow,endShow);

	int startPos = GetSampleAtMS(startShow);
	int endPos = GetSampleAtMS(endShow);
	int startX = GetXAtMS(startShow);
	int endX = GetXAtMS(endShow);
	if(hasKara){
		if (startX < 50 || endX > (w-200)) {
			UpdatePosition((startPos+endPos-w*samples)/2,true);
			//wxLogMessage("pos %i",endPos - 100*samples);
		}
	}
	else if (force || (startX < 50 && endX < w) || (endX > w-50 && startX > 0 )) {
		if ((startX < 50) || (endX >= w-50 )) {
			// Make sure the left edge of the selection is at least 50 pixels from the edge of the display
			UpdatePosition(startPos - 50*samples, true);
		} else {
			// Otherwise center the selection in display
			UpdatePosition((startPos+endPos-w*samples)/2,true);
		}
	}

	// Update
	UpdateImage();
}


////////////////
// Set position
void AudioDisplay::SetPosition(int pos) {
	Position = pos;
	PositionSample = pos * samples;
	UpdateImage();
}


///////////////////
// Update position
void AudioDisplay::UpdatePosition (int pos,bool IsSample) {
	// Safeguards
	if (!provider) return;
	if (IsSample) pos /= samples;
	int len = provider->GetNumSamples() / samples;
	if (pos < 0) pos = 0;
	if (pos >= len) pos = len-1;

	// Set
	Position = pos;
	PositionSample = pos*samples;
	UpdateScrollbar();
}


/////////////////////////////
// Set samples in percentage
// Note: aka Horizontal Zoom
void AudioDisplay::SetSamplesPercent(int percent,bool update,float pivot) {
	// Calculate
	if (percent < 1) percent = 1;
	if (percent > 100) percent = 100;
	if (samplesPercent == percent) return;
	samplesPercent = percent;

	// Update
	if (update) {
		// Center scroll
		int oldSamples = samples;
		UpdateSamples();
		PositionSample += int64_t((oldSamples-samples)*w*pivot);
		if (PositionSample < 0) PositionSample = 0;

		// Update
		//UpdateSamples();
		UpdateImage();
		UpdateScrollbar();

		//Refresh(false);
	}
}


//////////////////
// Update samples
void AudioDisplay::UpdateSamples() {
	// Set samples
	if (!provider) return;
	if (w) {
		int64_t totalSamples = provider->GetNumSamples();
		int total = totalSamples / w;
		int max = 5760000 / w;	// 2 minutes at 48 kHz maximum
		if (total > max) total = max;
		int min = 8;
		if (total < min) total = min;
		int range = total-min;
		samples = int(range*pow(samplesPercent/100.0,3)+min);

		// Set position
		int length = w * samples;
		if (PositionSample + length > totalSamples) {
			PositionSample = totalSamples - length;
			if (PositionSample < 0) PositionSample = 0;
			if (samples) Position = PositionSample / samples;
		}
	}
}


/////////////
// Set scale
void AudioDisplay::SetScale(float _scale) {
	if (scale == _scale) return;
	scale = _scale;
	UpdateImage();
}


//////////////////
// Load from file
void AudioDisplay::SetFile(wxString file, bool fromvideo) {
	// Unload
	if (player) {try {
		try {
			player->CloseStream();
		}
		catch (const wxChar *e) {
			wxLogError(e);
		}
		if(ownProvider && provider){delete provider;provider = NULL;}
		delete player;
		if(spectrumRenderer){delete spectrumRenderer; spectrumRenderer = NULL;}

		player = NULL;
		Reset();
		loaded = false;
	}
	catch (wxString e) {
		wxLogError(e);
	}
	catch (const wxChar *e) {
		wxLogError(e);
	}
	catch (...) {
		wxLogError(_T("Unknown error unloading audio"));
	}
	}
	// Load
	if(!file.IsEmpty()) {
		try {
			// Get provider
			TabPanel *pan=((TabPanel*)box->GetGrandParent());
			VideoCtrl *vb=pan->Video;
			bool success=true;
			if(vb->VFF && fromvideo){
				provider = vb->VFF; ownProvider=false;
			}else{
				provider = new VideoFfmpeg(file, 0, &success);
				if (!success || provider->SampleRate < 0) {
					delete provider; provider = 0; 
					loaded= false; return;
				}
				vb->player=this; ownProvider=true;
			}


			// Get player
			player = new DirectSoundPlayer2();
			player->SetProvider(provider);
			player->OpenStream();
			loaded = true;

			UpdateImage();
		}
		catch (const wxChar *e) {
			if (player) { delete player; player = 0; }
			if (provider) { delete provider; provider = 0; }
			wxLogError(e);
		}
		catch (wxString &err) {
			if (player) { delete player; player = 0; }
			if (provider) { delete provider; provider = 0; }
			wxMessageBox(err,_T("Error loading audio"),wxICON_ERROR | wxOK);
		}
		catch (...) {
			if (player) { delete player; player = 0; }
			if (provider) { delete provider; provider = 0; }
			wxLogError(_T("Unknown error loading audio"));
		}
	}

	if (!loaded) return;

	assert(loaded == (provider != NULL));

	// Set default selection
	int n = Edit->ebrow;
	SetDialogue(grid->GetDial(n),n);
}



////////////////////
// Update scrollbar
void AudioDisplay::UpdateScrollbar() {
	if (!provider) return;
	int page = w/12;
	int len = provider->GetNumSamples() / samples / 12;
	Position = PositionSample / samples;
	ScrollBar->SetScrollbar(Position/12,page,len,int(page*0.7),true);
}


//////////////////////////////////////////////
// Gets the sample number at the x coordinate
int64_t AudioDisplay::GetSampleAtX(int x) {
	return (x+Position)*samples;
}


/////////////////////////////////////////////////
// Gets the x coordinate corresponding to sample
int AudioDisplay::GetXAtSample(int64_t n) {
	return samples ? (n/samples)-Position : 0;
}


/////////////////
// Get MS from X
int AudioDisplay::GetMSAtX(int64_t x) {
	return (PositionSample+(x*samples)) * 1000 / provider->GetSampleRate();
}


/////////////////
// Get X from MS
int AudioDisplay::GetXAtMS(int64_t ms) {
	return ((ms * provider->GetSampleRate() / 1000)-PositionSample)/samples;
}


////////////////////
// Get MS At sample
int AudioDisplay::GetMSAtSample(int64_t x) {
	return x * 1000 / provider->GetSampleRate();
}


////////////////////
// Get Sample at MS
int64_t AudioDisplay::GetSampleAtMS(int64_t ms) {
	return ms * provider->GetSampleRate() / 1000;
}


////////
// Play
void AudioDisplay::Play(int start,int end,bool pause) {
	//Stop();
	if(pause && Notebook::GetTab()->Video->GetState()==Playing){Notebook::GetTab()->Video->Pause();}

	// Check provider
	if (!provider) {
		return;
	}

	// Set defaults
	playingToEnd = end < 0;
	int64_t num_samples = provider->GetNumSamples();
	start = GetSampleAtMS(start);
	if (end != -1) end = GetSampleAtMS(end);
	else end = num_samples-1;

	// Sanity checking
	if (start < 0) start = 0;
	if (start >= num_samples) start = num_samples-1;
	if (end >= num_samples) end = num_samples-1;
	if (end < start) end = start;

	// Redraw the image to avoid any junk left over from mouse movements etc
	// See issue #598
	UpdateImage(true);

	// Call play
	player->Play(start,end-start);
	if (!UpdateTimer.IsRunning()) UpdateTimer.Start(30);
}


////////
// Stop
void AudioDisplay::Stop() {
	if(Notebook::GetTab()->Video->GetState()==Playing){Notebook::GetTab()->Video->Pause();}
	else if (player) {
		player->Stop();
		if (UpdateTimer.IsRunning()) UpdateTimer.Stop();
	}

}


///////////////////////////
// Get samples of dialogue
void AudioDisplay::GetTimesDialogue(int &start,int &end) {
	start = dialogue->Start.mstime;
	end = dialogue->End.mstime;
}


////////////////////////////
// Get samples of selection
void AudioDisplay::GetTimesSelection(int &start,int &end) {
	if(hasKara){
		whichsyl = MID(0,whichsyl,(int)karaoke->syls.size()-1);
		karaoke->GetSylTimes(whichsyl, start, end);
	}else{
		start = curStartMS;
		end = curEndMS;
	}
}


/////////////////////////////
// Set the current selection
void AudioDisplay::SetSelection(int start, int end) {
	curStartMS = start;
	curEndMS = end;
	Update();
}


////////////////
// Set dialogue
void AudioDisplay::SetDialogue(Dialogue *diag,int n) {
	// Actual parameters
	// Set variables
	line_n = n;
	//dialog jest tylko odczytywany, jest on własnością editboxa, usuwać go nie można.
	dialogue = diag;
	NeedCommit=false;
	whichsyl=0;
	// Set flags
	// Set times
	if (Options.GetBool(_T("Audio Grab Times On Select"))) {
		curStartMS = dialogue->Start.mstime;
		curEndMS = dialogue->End.mstime;

		// Never do it for 0:00:00.00->0:00:00.00 lines
		/*if (s != 0 || e != 0) {
			curStartMS = s;
			curEndMS = e;
		}*/
	}


	// Reset karaoke pos

	if(hasKara){
		//whichsyl=0;
		karaoke->Split();

	}

	// Update	
	Update();
}


//////////////////
// Commit changes
void AudioDisplay::CommitChanges (bool nextLine) {
	// Loaded?
	if (!loaded) return;


	NeedCommit = false;

	// Update dialogues
	blockUpdate = true;
	STime gtime;
	gtime.NewTime(curStartMS);
	Edit->StartEdit->SetTime(gtime);
	gtime.NewTime(curEndMS);
	Edit->EndEdit->SetTime(gtime);
	gtime.NewTime(curEndMS - curStartMS);
	Edit->DurEdit->SetTime(gtime);
	Edit->StartEdit->SetModified(true);
	Edit->EndEdit->SetModified(true);
	Edit->Send(nextLine);
	if(!nextLine){Edit->UpdateChars(Edit->TextEdit->GetValue());}
	Edit->StartEdit->SetModified(false);
	Edit->EndEdit->SetModified(false);
	blockUpdate = false;



	Update();
}


////////////
// Add lead
void AudioDisplay::AddLead(bool in,bool out) {
	// Lead in
	if (in) {
		curStartMS -= Options.GetInt(_T("Audio Lead In"));
		if (curStartMS < 0) curStartMS = 0;
	}

	// Lead out
	if (out) {
		curEndMS += Options.GetInt(_T("Audio Lead Out"));
	}

	// Set changes
	UpdateTimeEditCtrls();
	NeedCommit = true;
	if (Options.GetBool(_T("Audio Autocommit"))) CommitChanges();
	Update();
}




/////////
// Paint
void AudioDisplay::OnPaint(wxPaintEvent& event) {
	if (w == 0 || h == 0) return;
	DoUpdateImage();

	wxPaintDC dc(this);
	if (origImage) dc.DrawBitmap(*origImage,0,0);

}


///////////////
// Mouse event
void AudioDisplay::OnMouseEvent(wxMouseEvent& event) {
	// Get x,y
	int64_t x = event.GetX();
	int64_t y = event.GetY();

	bool shiftDown = event.m_shiftDown;
	int timelineHeight = 20;
	if(box->arrows){box->SetCursor(wxCURSOR_ARROW); box->arrows=false;}
	// Leaving event
	if (event.Leaving()) {
		UpdateImage(true);
		return;
	}

	if (!player || !provider) {
		return;
	}

	// Is inside?

	bool onScale = false;
	if (x >= 0 && y >= 0 && x < w) {
		if (y < h) {
			inside = true;

			// Get focus
			if (wxWindow::FindFocus() != this && Options.GetBool(_T("Audio Autofocus"))) SetFocus();
		}
		else if (y < h+timelineHeight) onScale = true;
		if(inside && onScale){UpdateImage(true); inside=false;}
	}else{inside = false;}

	// Buttons
	bool leftDown = event.LeftDown();
	bool rightDown = event.RightDown();
	bool buttonDown = leftDown || rightDown;
	bool buttonUP= event.LeftUp() || event.RightUp();
	bool middleDown = event.MiddleDown();
	bool updated = false;
	int syll=-1;
	// Click type

	if (buttonUP && holding) {
		//wxLogStatus("left or right up");
		holding = false;
		if (HasCapture()) ReleaseMouse();
	}


	if(leftDown&&event.ControlDown()&&!event.AltDown()&&!onScale)
	{
		int pos=GetMSAtX(x);
		Notebook::GetTab()->Video->Seek(pos);
		UpdateImage(true);

	}

	if (buttonDown && !holding) {
		holding = true;
		CaptureMouse();
	}


	// Mouse wheel
	if (event.GetWheelRotation() != 0) {
		// Zoom or scroll?
		bool zoom = shiftDown;
		if (Options.GetBool(_T("Audio Wheel Default To Zoom"))) zoom = !zoom;

		// Zoom
		if (zoom) {

			int step = event.GetWheelRotation() / event.GetWheelDelta();

			int value = box->HorizontalZoom->GetValue()+step;
			box->HorizontalZoom->SetValue(value);
			SetSamplesPercent(value,true,float(x)/float(w));
		}

		// Scroll
		else {
			int step = -event.GetWheelRotation() * w / 360;
			UpdatePosition(Position+step,false);
			UpdateImage();
		}
	}


	// Scale dragging
	if ((hold == 0 && onScale) || draggingScale) {
		SetCursor(wxNullCursor);
		if(rightDown)
		{
			curMarkMS=GetMSAtX(x);
			if(!hasMark){
				hasMark=true;
				TabPanel *panel=(TabPanel*)grid->GetParent();
				panel->CTime->Contents();
			}
			hasMark=true;
			UpdateImage(true);
			return;
		}
		if (leftDown) {
			lastDragX = x;
			draggingScale = true;
		}
		else if (holding) {
			int delta = lastDragX - x;
			lastDragX = x;
			UpdatePosition(Position + delta);
			UpdateImage();
			Refresh(false);
			return;
		}
		else draggingScale = false;
	}

	// Outside
	if (!inside && hold == 0) return;

	// Left click - focos trzeba nadawać wszystkimi przyciskami
	if (event.ButtonDown()) {
		SetFocus();
	}


	// Timing
	if (hasSel && !(event.ControlDown() && !event.AltDown() && hold==0)) {

		letter=-1;
		// znacznik
		if (hold == 0) {
			if (hasMark && abs64 (x - selMark) < 6 ) {
				wxCursor cursor(wxCURSOR_SIZEWE);
				SetCursor(cursor);
				defCursor=false;
				if (buttonDown) {
					hold = 4;
				}
			}





			//start
			else if (abs64 (x - selStart) < 6 ) {
				wxCursor cursor(wxCURSOR_SIZEWE);
				SetCursor(cursor);
				defCursor=false;
				if (buttonDown) {
					hold = 1;
				}
			}


			//żółte linie karaoke
			else if(hasKara && y>20)
			{
				if(!karaoke->CheckIfOver(x, &Grabbed)){
					int tmpsyl = -1;
					bool hasSyl = karaoke->GetSylAtX(x,&tmpsyl);
					if(Options.GetBool("Audio Karaoke Move On Click") && hasSyl && !(tmpsyl<whichsyl-1||tmpsyl>whichsyl+1) && (leftDown||rightDown)){
						Grabbed=(tmpsyl<whichsyl)? whichsyl-1 : whichsyl;
						hold=5;
					}
					else if(leftDown && tmpsyl>=0){
						whichsyl=tmpsyl;
						updated=true;
					}
					if(!defCursor){SetCursor(wxNullCursor);defCursor=true;}
				}else{

					wxCursor cursor(wxCURSOR_SIZEWE);
					SetCursor(cursor);
					defCursor=false;
					if(middleDown){
						karaoke->Join(Grabbed);
						Commit();
						return;
					}

					if(buttonDown){hold=5;}
				}



			}
			// litery sylab
			else if(hasKara && karaoke->GetLetterAtX(x,&syll,&letter))
			{
				if (leftDown){
					karaoke->SplitSyl(syll, letter);
					whichsyl=syll;
					Commit();
				}


				if(!defCursor){SetCursor(wxNullCursor);defCursor=true;}
			}

			// Grab end
			else if (abs64 (x - selEnd) < 6 ) {
				wxCursor cursor(wxCURSOR_SIZEWE);
				SetCursor(cursor);
				defCursor=false;
				if (buttonDown) {
					hold = 2;
				}
			}

			// Dragging nothing, time from scratch
			else if (buttonDown && !hasKara) {
				if (leftDown) hold = 3;
				else hold = 2;
				lastX = x;

			}

			// restoring cursor
			else if(!defCursor){SetCursor(wxNullCursor);defCursor=true;}

		}

		// Drag start/end
		if(hold!=0){

			// Dragging
			// Release
			if(buttonUP) {
				// Prevent negative times
				//wxLogStatus(" hold1 %i", Grabbed);
				if(Grabbed==-1)
				{
					curStartMS = MAX(0, curStartMS);
					curEndMS = MAX(0, curEndMS);
					curStartMS = ZEROIT(curStartMS);
					curEndMS = ZEROIT(curEndMS);
					selStart = MAX(0, selStart);
					selEnd = MAX(0, selEnd);
					int nn = Edit->ebrow;
					//automatyczne ustawianie czasów następnej linijki (Chwyt myszą end + ctrl)
					//wxLogStatus(" hold %i %i %i %i", hold, nn, (int)event.ControlDown(), (int)event.AltDown());
					if(hold==2 && nn<grid->GetCount()-1 && event.ControlDown() && event.AltDown())
					{
						//wxLogStatus(" hold1 %i %i %i %i", hold, nn, (int)event.ControlDown(), (int)event.AltDown());
						Dialogue *dialc=grid->CopyDial(nn+1);
						dialc->Start.NewTime(curEndMS);
						dialc->End.NewTime(curEndMS+5000);
					}
				}


				if(hasKara && Grabbed!=-1)
				{
					int newpos=ZEROIT(GetMSAtX(x));
					int prev=(Grabbed==0)? curStartMS : karaoke->syltimes[Grabbed-1];
					int next=(Grabbed==(int)karaoke->syls.size()-1)? curEndMS : karaoke->syltimes[Grabbed+1];
					karaoke->syltimes[Grabbed]=MID(prev,newpos,next);
					whichsyl=Grabbed;
				}
				if(hold!=4){
					Commit();}

				// Update stuff

				hold = 0;

				return;
			}
			else {
				//drag timing change cursor
				if (hold == 4) {

					curMarkMS = GetMSAtX(x);
					updated = true;	
				}
				// Drag from nothing or straight timing
				if (hold == 3) {

					if (leftDown) curStartMS = GetBoundarySnap(GetMSAtX(x),16,event.ShiftDown(),true);
					else curEndMS = GetMSAtX(x);
					updated = true;
					NeedCommit = true;

					if (leftDown && abs((long)(x-lastX)) > Options.GetInt(_T("Audio Start Drag Sensitivity"))) {
						selStart = lastX;
						selEnd = x;
						curStartMS = GetBoundarySnap(GetMSAtX(lastX),16,event.ShiftDown(),true);
						curEndMS = GetMSAtX(x);
						hold = 2;
					}

				}

				// Drag start
				if (hold == 1) {
					// Set new value
					if (x != selStart) {
						int snapped = GetBoundarySnap(GetMSAtX(x),16,event.ShiftDown(),true);
						selStart = GetXAtMS(snapped);
						/*if (selStart > selEnd) {
						int temp = selStart;
						selStart = selEnd;
						selEnd = temp;
						hold = 2;
						curEndMS = snapped;
						snapped = GetMSAtX(selStart);
						}*/

						if(hasKara && (event.RightIsDown()||rightDown)){
							int sizes=karaoke->syls.size();
							int addtime = snapped - curStartMS;
							for(int i= 0; i<sizes; i++)
							{
								int time=karaoke->syltimes[i];
								time+=addtime;
								time=ZEROIT(time);
								karaoke->syltimes[i]=time;
							}
							curEndMS=karaoke->syltimes[sizes-1];
						}
						curStartMS = snapped;
						updated = true;
						NeedCommit = true;
					}
				}

				// Drag end
				if (hold == 2) {
					// Set new value
					if (x != selEnd) {
						int snapped = GetBoundarySnap(GetMSAtX(x),16,event.ShiftDown(),false);
						selEnd = GetXAtMS(snapped);
						//selEnd = GetBoundarySnap(x,event.ShiftDown()?0:10,false);
						/*if (selStart > selEnd) {
						int temp = selStart;
						selStart = selEnd;
						selEnd = temp;
						hold = 1;
						curStartMS = snapped;
						snapped = GetMSAtX(selEnd);
						}*/
						curEndMS = snapped;

						updated = true;
						NeedCommit = true;
					}
				}
				//drag karaoke
				if (hold==5 && Grabbed!=-1){
					//if(){
					int newpos=ZEROIT(GetMSAtX(x));
					int sizes=karaoke->syls.size()-1;
					int prev=(Grabbed==0)? curStartMS : karaoke->syltimes[Grabbed-1];
					int next=(Grabbed==sizes)? 2147483646 : karaoke->syltimes[Grabbed+1];
					int prevpos=karaoke->syltimes[Grabbed];
					karaoke->syltimes[Grabbed]=MID(prev,newpos,next);
					if(Grabbed==sizes){curEndMS=karaoke->syltimes[Grabbed];}
					//prawy przycisk myszy
					if(rightDown||event.RightIsDown()){
						int addtime=karaoke->syltimes[Grabbed]-prevpos;
						//int lines=sizes-(Grabbed+1);
						//wxLogMessage("lines %i, addtimes %i", lines, addtime);
						//lines+=10;
						//if(addtime>0){addtime=lines;}else if(addtime<0){addtime= (-lines);}


						for(int i= Grabbed+1;i<(int)karaoke->syls.size()-1;i++)
						{
							int time=karaoke->syltimes[i];
							time+=addtime;
							time=ZEROIT(time);
							karaoke->syltimes[i]=time;
							if(karaoke->syltimes[i]>karaoke->syltimes[i+1]){
								karaoke->syltimes[i]=karaoke->syltimes[i+1];}
							//if(addtime==0){break;}
							//if(addtime>0){addtime--;}else{addtime++;}
						}

						curEndMS=karaoke->syltimes[sizes];
					}
					updated=true;
					//}
				}


			}

		}
		// Update stuff
		if (updated) {

			if (!playingToEnd) {
				int64_t slend;
				if(hasKara && Grabbed>=0){slend=GetSampleAtMS(karaoke->syltimes[Grabbed]);
				//wxLogStatus("cur %i end %i", (int)player->GetCurrentPosition(), (int)slend);
				}else{
					slend=GetSampleAtX(selEnd);}
				player->SetEndPosition(slend);
			}

			UpdateImage(true);

			return;
		}

	}

	// Not holding
	else {
		hold = 0;
	}

	// Right click
	if (rightDown && hasKara) {
		SetFocus();
		int syl;
		if (karaoke->GetSylAtX(x, &syl)) {
			int start,end;
			karaoke->GetSylTimes(syl,start,end);
			Play(start, end);
			whichsyl=syl;	
		}

	}

	// Middle click
	if (middleDown) {
		SetFocus();
		int start=0,end=0;
		GetTimesSelection(start,end);
		Play(start,end);
	}

	// Cursor drawing
	if (player && !player->IsPlaying() && event.Moving() && origImage) {
		// Draw bg
		wxClientDC cdc(this);
		wxMemoryDC dc;
		if(w > origImage->GetWidth() || h > origImage->GetHeight()){return;}
		dc.SelectObject(origImage->GetSubBitmap(wxRect(0,0,w,h)));

		if(inside){
			int fw, fh=18;
			if(hasKara && letter!=-1){
				int start,end;
				//wxLogStatus("syll %i", syll);
				wxString syl=karaoke->syls[syll];
				//wxLogStatus(syl);
				wxFont karafont(11,wxDEFAULT,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_BOLD,false,_T("Verdana"));
				dc.SetFont(karafont);
				dc.GetTextExtent(syl,&fw, &fh, 0, 0, &karafont);
				//wxLogStatus("get syl times");
				karaoke->GetSylTimes(syll,start,end);
				//wxLogStatus("got syl times");
				start=GetXAtMS(start);
				end=GetXAtMS(end);
				int center=start+((end-start-fw)/2);
				if(letter==0){fw=0;}
				else{dc.GetTextExtent(syl.Mid(0,letter),&fw, &fh, 0, 0, &karafont);}
				dc.SetPen(wxPen(wxColour("#FF0000")));

				dc.DrawLine(center+fw,0,center+fw,fh);

				goto done;
			}

			// Draw cursor
			dc.SetLogicalFunction(wxINVERT);

			dc.DrawLine(x,0,x,h);


			// Time
			// Time string
			STime time;
			time.NewTime(GetMSAtX(x));
			wxString text = time.GetFormatted(ASS);

			// Calculate metrics
			// FIXME: Hardcoded font name
			wxFont font(10,wxFONTFAMILY_DEFAULT,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_BOLD,false,_T("Verdana"));
			dc.SetFont(font);
			int tw,th;
			GetTextExtent(text,&tw,&th,NULL,NULL,&font);

			// Text coordinates
			int dx;
			dx = x - tw/2;
			if (dx < 4) dx = 4;
			int max = w - tw - 4;
			if (dx > max) dx = max;
			int dy = 4;
			if (hasKara) dy += th;

			// Draw text
			dc.SetTextForeground(wxColour(64,64,64));
			dc.DrawText(text,dx+1,dy-1);
			dc.DrawText(text,dx+1,dy+1);
			dc.DrawText(text,dx-1,dy-1);
			dc.DrawText(text,dx-1,dy+1);
			dc.SetTextForeground(wxColour(255,255,255));
			dc.DrawText(text,dx,dy);
		}
done:

		cdc.Blit(0,0,w,h,&dc,0,0);
	}
}


////////////////////////
// Get snap to boundary
int AudioDisplay::GetBoundarySnap(int ms,int rangeX,bool shiftHeld,bool start, bool keysnap) {
	// Range?
	if (rangeX <= 0) return ms;

	// Convert range into miliseconds
	int rangeMS = rangeX*samples*1000 / provider->GetSampleRate();
	int halfframe=Notebook::GetTab()->Video->avtpf/2;
	// Keyframe boundaries
	wxArrayInt boundaries;

	bool snapKey = Options.GetBool(_T("Audio Snap To Keyframes"));
	if (shiftHeld) snapKey = !snapKey;
	if (snapKey && provider->KeyFrames.size()>0 && Options.GetBool(_T("Audio Draw Keyframes"))) {
		int64_t keyMS;
		//int diff=(start)? -halfframe : - halfframe;
		for (unsigned int i=0;i<provider->KeyFrames.Count();i++) {
			keyMS = provider->KeyFrames[i];
			int keyX=GetXAtMS(keyMS);
			if (keyX >= 0 && keyX < w) {boundaries.Add(ZEROIT(keyMS-halfframe));}
		}
	}

	// Other subtitles' boundaries
	int inactiveType = Options.GetInt(_T("Audio Inactive Lines Display Mode"));
	bool snapLines = Options.GetBool(_T("Audio Snap To Other Lines"));
	if (shiftHeld) snapLines = !snapLines;
	if (snapLines && (inactiveType == 1 || inactiveType == 2)) {
		Dialogue *shade;
		int shadeX1,shadeX2;
		int shadeFrom,shadeTo;

		// Get range
		if (inactiveType == 1) {
			shadeFrom = MAX(0,this->line_n-1);
			shadeTo = MIN(shadeFrom+3,grid->GetCount());
		}
		else {
			shadeFrom = 0;
			shadeTo = grid->GetCount();
		}

		for (int j=shadeFrom;j<shadeTo;j++) {
			if (j == line_n) continue;
			shade = grid->GetDial(j);


			// Get coordinates
			shadeX1 = GetXAtMS(shade->Start.mstime);
			shadeX2 = GetXAtMS(shade->End.mstime);
			if (shadeX1 >= 0 && shadeX1 < w) boundaries.Add(shade->Start.mstime);
			if (shadeX2 >= 0 && shadeX2 < w) boundaries.Add(shade->End.mstime);

		}
	}

	// See if ms falls within range of any of them
	int minDist = rangeMS+1;
	int adist = minDist;
	int bestMS = ms;
	for (unsigned int i=0;i<boundaries.Count();i++) {
		adist=abs(ms-boundaries[i]);
		if (adist < minDist) {
			if(keysnap && adist<10){continue;}
			bestMS = boundaries[i];
			minDist = adist;
		}
	}

	// Return best match
	return bestMS;
}





//////////////
// Size event
void AudioDisplay::OnSize(wxSizeEvent &event) {
	// Set size
	GetClientSize(&w,&h);
	h -= 20;

	// Update image
	UpdateSamples();
	if (samples) {
		UpdatePosition(PositionSample / samples);
	}
	UpdateImage();

	// Update scrollbar
	UpdateScrollbar();
}


///////////////
// Timer event
void AudioDisplay::OnUpdateTimer(wxTimerEvent &event) {

	if (!origImage)
		return;

	//wxLogStatus("Isplaying %i", (int)player->IsPlaying());	
	/*if (!player->IsPlaying()){
	if(cursorPaint){
	needImageUpdate=false;
	Refresh(false);
	cursorPaint=false;
	}
	return;
	}*/
	wxMutexLocker lock(mutex);

	// Get DCs
	wxClientDC dc(this);

	// Draw cursor
	int curpos = -1;
	if (player->IsPlaying()) {
		cursorPaint=true;
		int64_t curPos = player->GetCurrentPosition();
		//wxLogStatus("Cur %i", curPos);
		if (curPos > player->GetStartPosition() && curPos < player->GetEndPosition()) {
			// Scroll if needed
			int posX = GetXAtSample(curPos);
			bool fullDraw = false;
			bool centerLock = false;
			bool scrollToCursor = Options.GetBool(_T("Audio Lock Scroll On Cursor"));
			if (centerLock) {
				int goTo = MAX(0,curPos - w*samples/2);
				if (goTo >= 0) {
					UpdatePosition(goTo,true);
					UpdateImage();
					fullDraw = true;
				}
			}
			else {
				if (scrollToCursor) {
					if (posX < 80 || posX > w-80) {
						int goTo = MAX(0,curPos - 80*samples);
						if (goTo >= 0) {
							UpdatePosition(goTo,true);
							UpdateImage();
							fullDraw = true;
						}
					}
				}
			}

			// Draw cursor
			wxMemoryDC src;
			curpos = GetXAtSample(curPos);
			if (curpos >= 0 && curpos < GetClientSize().GetWidth()) {

				dc.SetPen(wxPen(Options.GetColour(_T("Audio Play Cursor")),2));

				if (fullDraw) {
					//dc.Blit(0,0,w,h,&src,0,0);
					dc.DrawLine(curpos,0,curpos,h);
					//dc.Blit(0,0,curpos-10,h,&src,0,0);
					//dc.Blit(curpos+10,0,w-curpos-10,h,&src,curpos+10,0);
				}
				else {
					src.SelectObject(*origImage);
					dc.Blit(oldCurPos-1,0,2,h,&src,oldCurPos-1,0);
					dc.DrawLine(curpos,0,curpos,h);
				}

			}
		}
		else {
			if (curPos > player->GetEndPosition() + 8192) {
				player->Stop();
			}

			if(cursorPaint){
				wxMemoryDC src;
				src.SelectObject(*origImage);
				dc.Blit(oldCurPos-1,0,2,h,&src,oldCurPos-1,0);
				cursorPaint=false;
			}
		}
	}

	// Restore background
	else {

		//wxMemoryDC src;
		//src.SelectObject(*origImage);
		//dc.Blit(oldCurPos-1,0,2,h,&src,oldCurPos-1,0);
		needImageUpdate=true;
		Refresh(false);
		if (UpdateTimer.IsRunning()) UpdateTimer.Stop();
	}
	oldCurPos = curpos;
	if (oldCurPos < 0) oldCurPos = 0;
}




///////////////
// Change line
void AudioDisplay::ChangeLine(int delta, bool block) {

	// Get next line number and make sure it's within bounds

	if (line_n==0 && delta<0 || line_n== grid->GetCount()-1 && delta>0 ) {return;}
	int next = line_n+delta;
	// Set stuff
	grid->SelectRow(next);
	grid->ScrollTo(next-4);
	Edit->SetIt(next);



}


////////
// Next
void AudioDisplay::Next(bool play) {
	// Karaoke
	if(hasKara){
		whichsyl++;
		if(whichsyl>=(int)karaoke->syls.size()){whichsyl=0;ChangeLine(1);}
	}else{
		//if(Notebook::GetTab()->Video->GetState()==Playing){Notebook::GetTab()->Video->Pause();}
		ChangeLine(1);}

	if (play){
		int start=0,end=0;
		GetTimesSelection(start,end);
		Play(start,end);}


}


////////////
// Previous
void AudioDisplay::Prev(bool play) {
	// Karaoke
	if(hasKara&&play){
		whichsyl--;
		if(whichsyl<0){whichsyl=karaoke->syls.size()-1;ChangeLine(-1);}

	}else{
		//if(Notebook::GetTab()->Video->GetState()==Playing){Notebook::GetTab()->Video->Pause();}
		ChangeLine(-1);}

	if (play) {
		int start=0,end=0;
		GetTimesSelection(start,end);
		Play(start,end);}
}



////////////////
// Focus events
void AudioDisplay::OnGetFocus(wxFocusEvent &event) {
	if (!hasFocus) {
		hasFocus = true;
		UpdateImage(true);
	}
}

void AudioDisplay::OnLoseFocus(wxFocusEvent &event) {
	//if(HasCapture()){ReleaseMouse();}
	if (hasFocus && loaded) {
		hasFocus = false;
		UpdateImage(true);
		Refresh(false);
	}
}


//////////////////////////////
// Update time edit controls
bool AudioDisplay::UpdateTimeEditCtrls() {
	// Make sure this does NOT get short-circuit evaluation,
	// this is why binary OR instead of logical OR is used.
	// All three time edits must always be updated.

	Edit->StartEdit->SetTime(curStartMS);
	Edit->EndEdit->SetTime(curEndMS);
	return true;
}

void AudioDisplay::Commit()
{
	bool autocommit=Options.GetBool(_T("Audio Autocommit"));
	if(hasKara) 
	{Edit->TextEdit->SetTextS(karaoke->GetText(),true);}

	if (autocommit) 
	{CommitChanges();}
	else
	{UpdateImage(true);}
}

//////////////////
// Draw keyframes
void AudioDisplay::DrawKeyframes(wxDC &dc) {
	dc.SetPen(wxPen(Options.GetColour("Audio Keyframes"),1));

	// Get min and max frames to care about
	int mintime = GetMSAtX(0);
	int maxtime = GetMSAtX(w);

	// Scan list
	for (size_t i=0;i<provider->KeyFrames.size();i++) {
		int cur = ((provider->KeyFrames[i]-5)/10)*10;
		if(cur>=mintime && cur<=maxtime)
		{
			int x = GetXAtMS(cur);
			dc.DrawLine(x,0,x,h);
		}
		if(cur>maxtime){break;}

	}
}
/*
void AudioDisplay::OnLostCapture(wxMouseCaptureLostEvent &event)
{
wxLogStatus("lost capture");
ReleaseMouse();

}
*/

///////////////
// Event table
BEGIN_EVENT_TABLE(AudioDisplay, wxWindow)
	EVT_MOUSE_EVENTS(AudioDisplay::OnMouseEvent)
	EVT_PAINT(AudioDisplay::OnPaint)
	EVT_SIZE(AudioDisplay::OnSize)
	EVT_TIMER(Audio_Update_Timer,AudioDisplay::OnUpdateTimer)
	EVT_SET_FOCUS(AudioDisplay::OnGetFocus)
	EVT_KILL_FOCUS(AudioDisplay::OnLoseFocus)
	//EVT_MOUSE_CAPTURE_LOST(AudioDisplay::OnLostCapture)
	END_EVENT_TABLE()
