//  Copyright (c) 2018 - 2026, Marcin Drob

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

#pragma once
//#include <wx/dynarray.h>
#include <wx/tokenzr.h>
class Provider;
class wxArrayInt;


class KeyframeLoader
{
public:
	//load keyframes after loading video, it need timecodes
	KeyframeLoader(const wxString &filename, wxArrayInt *keyframes, Provider *receiver);
	//for renderers that have no provider with timecodes (GStreamer on Linux),
	//frame times are counted from fps, so it's exact only on CFR video
	KeyframeLoader(const wxString &filename, wxArrayInt *keyframes, float fps);
private:
	void LoadFile(const wxString &filename);
	int GetMSfromFrame(int frame);
	void OpenAegisubKeyframes(wxStringTokenizer *kftokenizer);
	void OpenOtherKeyframes(int type, wxStringTokenizer *kftokenizer);

	wxArrayInt *keyframes;
	Provider *receiver;
	float fps;
};

enum{
	TYPE_XVID,
	TYPE_DIVX,
	TYPE_X264,
};