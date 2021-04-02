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

#pragma once
#include <wx/string.h>

//when textout is NULL result puts to textIn
void ConvertToRTLChars(wxString* textin, wxString* textout = NULL);
void ConvertToRTL(wxString* textin, wxString* textout = NULL);
void BIDIConvert(wxString* text);
void BIDIReverseConvert(wxString* text);
//when textout is NULL result puts to textIn
void ConvertToLTR(wxString* textin, wxString* textout = NULL);
void ConvertToLTRChars(wxString* textin, wxString* textout = NULL);
bool IsRTLCharacter(const wxUniChar& ch);
bool CheckRTL(const wxString* text);