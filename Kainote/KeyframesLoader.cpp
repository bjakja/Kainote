//  Copyright (c) 2018 - 2020, Marcin Drob

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

//Kayframes formats loading methods taken from Aegisub



#include "KeyframesLoader.h"
#include <wx/file.h>
#include <wx/arrstr.h>
#include "OpennWrite.h"
#include "Provider.h"

KeyframeLoader::KeyframeLoader(const wxString &filename, wxArrayInt *_keyframes, Provider *_receiver)
	: keyframes(_keyframes)
	, receiver(_receiver)
{
	OpenWrite ow;
	wxString kftext;
	ow.FileOpen(filename, &kftext);
	if (kftext.empty())
		return;

	wxStringTokenizer kftokenizer(kftext, L"\n", wxTOKEN_STRTOK);
	wxString header = kftokenizer.GetNextToken();

	if (header == L"# keyframe format v1")
		OpenAegisubKeyframes(&kftokenizer);
	else if (header.StartsWith(L"# XviD 2pass stat file"))
		OpenOtherKeyframes(TYPE_XVID, &kftokenizer);
	else if (header.StartsWith(L"# ffmpeg 2-pass log file, using xvid codec"))
		OpenOtherKeyframes(TYPE_XVID, &kftokenizer);
	else if (header.StartsWith(L"# avconv 2-pass log file, using xvid codec"))
		OpenOtherKeyframes(TYPE_XVID, &kftokenizer);
	else if (header.StartsWith(L"##map version"))
		OpenOtherKeyframes(TYPE_DIVX, &kftokenizer);
	else if (header.StartsWith(L"#options:"))
		OpenOtherKeyframes(TYPE_X264, &kftokenizer);
}

void KeyframeLoader::OpenAegisubKeyframes(wxStringTokenizer *kftokenizer)
{
	while (kftokenizer->HasMoreTokens()){
		int keyframe = wxAtoi(kftokenizer->GetNextToken());
		keyframes->push_back(receiver->GetMSfromFrame(keyframe));
	}
}

void KeyframeLoader::OpenOtherKeyframes(int type, wxStringTokenizer *kftokenizer)
{
	wxUniChar frameType;
	wxString frameChars = L"IPB";
	size_t frameCounter = 0;
	while (kftokenizer->HasMoreTokens()){
		wxString token = kftokenizer->GetNextToken();
		if (type == TYPE_XVID)
			frameType = token[0];
		else if(type == TYPE_DIVX){
			frameType = L'#';
			for (size_t i = 0; i < 3; i++){
				size_t result = token.find(frameChars);
				if (result != -1){
					frameType = tolower(token[result]);
					break;
				}
			}
		}
		else{
			frameType = L'#';
			size_t result = token.find(L"type:");
			if (result != -1 && result + 5 < token.size())
				frameType = tolower(token[result + 5]);
		}
		if (frameType == L'i')
			keyframes->push_back(receiver->GetMSfromFrame(frameCounter++));
		else if (frameType == L'p' || frameType == L'b')
			frameCounter++;
	}
}
