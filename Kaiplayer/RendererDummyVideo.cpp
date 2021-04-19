//  Copyright (c) 2021, Marcin Drob

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

#include "RendererDummyVideo.h"
#include <wx/tokenzr.h>

RendererDummyVideo::RendererDummyVideo(VideoCtrl* control, bool visualDisabled)
	: RendererVideo(control, visualDisabled)
{
}

RendererDummyVideo::~RendererDummyVideo()
{
}

bool RendererDummyVideo::OpenFile(const wxString& fname, int subsFlag, bool vobsub, bool changeAudio)
{
	return false;
}

bool RendererDummyVideo::OpenSubs(int flag, bool redraw, wxString* text, bool resetParameters)
{
	return false;
}

bool RendererDummyVideo::Play(int end)
{
	return false;
}

bool RendererDummyVideo::Pause()
{
	return false;
}

bool RendererDummyVideo::Stop()
{
	return false;
}

void RendererDummyVideo::SetPosition(int _time, bool starttime, bool corect, bool async)
{
}

void RendererDummyVideo::SetFFMS2Position(int time, bool starttime)
{
}

void RendererDummyVideo::GoToNextKeyframe()
{
}

void RendererDummyVideo::GoToPrevKeyframe()
{
}

int RendererDummyVideo::GetFrameTime(bool start)
{
	return 0;
}

void RendererDummyVideo::GetStartEndDelay(int startTime, int endTime, int* retStart, int* retEnd)
{
}

int RendererDummyVideo::GetFrameTimeFromTime(int time, bool start)
{
	return 0;
}

int RendererDummyVideo::GetFrameTimeFromFrame(int frame, bool start)
{
	return 0;
}

int RendererDummyVideo::GetPlayEndTime(int time)
{
	return 0;
}

int RendererDummyVideo::GetDuration()
{
	return 0;
}

int RendererDummyVideo::GetVolume()
{
	return 0;
}

void RendererDummyVideo::GetVideoSize(int* width, int* height)
{
}

void RendererDummyVideo::GetFpsnRatio(float* fps, long* arx, long* ary)
{
}

void RendererDummyVideo::SetVolume(int vol)
{
}

bool RendererDummyVideo::DrawTexture(byte* nframe, bool copy)
{
	return false;
}

void RendererDummyVideo::Render(bool RecreateFrame, bool wait)
{
}

void RendererDummyVideo::ChangePositionByFrame(int cpos)
{
}

byte* RendererDummyVideo::GetFramewithSubs(bool subs, bool* del, void* converter)
{
	return nullptr;
}

void RendererDummyVideo::OpenKeyframes(const wxString& filename)
{
}

bool RendererDummyVideo::ParseDummyData(const wxString& data)
{
	wxStringTokenizer tokenzr(data, L":", wxTOKEN_RET_EMPTY_ALL);
	
	tokenzr.GetNextToken();
	if (!tokenzr.HasMoreTokens())
		return false;
	wxString strfps = tokenzr.GetNextToken();
	if (!tokenzr.HasMoreTokens())
		return false;
	wxString strduration = tokenzr.GetNextToken();
	if (!tokenzr.HasMoreTokens())
		return false;
	wxString strwidth = tokenzr.GetNextToken();
	if (!tokenzr.HasMoreTokens())
		return false;
	wxString strheight = tokenzr.GetNextToken();
	if (!tokenzr.HasMoreTokens())
		return false;
	wxString strcolorr = tokenzr.GetNextToken();
	if (!tokenzr.HasMoreTokens())
		return false;
	wxString strcolorg = tokenzr.GetNextToken();
	if (!tokenzr.HasMoreTokens())
		return false;
	wxString strcolorb = tokenzr.GetNextToken();
	if (!tokenzr.HasMoreTokens())
		return false;
	wxString strpattern = tokenzr.GetNextToken();

	double dfps = 0;
	if (!strfps.ToCDouble(&dfps))
		return false;
	FPS = dfps;
}
