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

#include "BidiConversion.h"
#include "LogHandler.h"
#include <unicode/utypes.h>
#include <unicode/ubidi.h>
#include <unicode/ubiditransform.h>
#include <unicode/uchar.h>
#include <unicode/localpointer.h>
#include <unicode/putil.h>
#include <unicode/ushape.h>
//#include <windows.h>

void ConvertToRTL(wxString* textin, wxString* textout)
{
	if (!textin)
		return;

	size_t len = textin->size();
	if (!len)
		return;

	wxString convertedText;
	wxString ltrText;
	wxString rtlText;
	bool block = false;
	for (int j = 0; j < len; j++) {
		const wxUniChar& ch = (*textin)[j];
		if (ch == L'{') {
			if (!rtlText.empty()) {
				BIDIConvert(&rtlText);
				convertedText << rtlText;
				rtlText.clear();
			}
			ltrText << ch;
			block = true;
		}
		else if (ch == L'}') {
			block = false;
			convertedText << ltrText << ch;
			ltrText.clear();
		}
		else if (block) {
			ltrText << ch;
		}
		else {
			rtlText << ch;
		}
		/*if (IsRTLCharacter(ch)) {
			if (!ltrText.empty()) {

				convertedText << ltrText;
				ltrText.clear();
			}
			rtlText << ch;
		}
		else {
			if (!rtlText.empty()) {
				BIDIConvert(&rtlText);
				convertedText << rtlText;
				rtlText.clear();
			}
			ltrText << ch;
		}*/

	}
	if (!ltrText.empty()) {
		convertedText << ltrText;
	}
	if (!rtlText.empty()) {
		BIDIConvert(&rtlText);
		convertedText << rtlText;
	}

	if (textout)
		*textout = convertedText;
	else
		*textin = convertedText;
}

void ConvertToRTLChars(wxString* textin, wxString* textout)
{
	if (!textin)
		return;

	size_t len = textin->size();
	if (!len) {
		if (textout)
			*textout = *textin;
		return;
	}

	wxString convertedText;
	wxString ltrText;
	wxString rtlText;
	wxString reverseRTL;
	bool block = false;
	for (int j = 0; j < len; j++) {
		const wxUniChar& ch = (*textin)[j];
		if (ch == L'{' && (!reverseRTL.empty() || !rtlText.empty())) {
			if (!rtlText.empty()) {
				if (NeedConversion(&rtlText)) {
					//if (!spellChecking)
						BIDIConvert(&rtlText);
				}
				else {
					/*if(spellChecking)
						BIDIReverseConvert(&rtlText);
					else*/
						SwitchRTLChars(&rtlText);
				}

				reverseRTL.insert(0, rtlText);
				rtlText.clear();
			}
			convertedText << reverseRTL;
			reverseRTL.clear();
			ltrText << ch;
			convertedText << ltrText;
			ltrText.clear();
		}
		else if (iswctype(wint_t(ch), _SPACE | _PUNCT) != 0 && 
			//0x61F = Arabic "?"
			(!rtlText.empty() || !reverseRTL.empty()) && ch != 0x61F && ch != L'\\'/* && ch != 0x60C*/) {
			if (rtlText.empty())
				reverseRTL.insert(0, ch);
			else {
				if (NeedConversion(&rtlText)) {
					//if (!spellChecking)
						BIDIConvert(&rtlText);
				}
				else {
					/*if (spellChecking)
						BIDIReverseConvert(&rtlText);
					else*/
						SwitchRTLChars(&rtlText);
				}
				const wxUniChar& nch = (j + 1 < len) ? (*textin)[j + 1] : L'A';
				bool isNextRTL = IsRTLCharacter(nch) || (iswctype(wint_t(nch), _SPACE | _PUNCT) != 0 && 
					nch != L'{' && nch != L'\\');
				reverseRTL.insert(0, isNextRTL ? ch + rtlText : rtlText);
				rtlText.clear();
				if (!isNextRTL)
					ltrText.insert(0, ch);
			}
		}
		else if (IsRTLCharacter(ch)) {
			if (!ltrText.empty()) {

				convertedText << ltrText;
				ltrText.clear();
				if (ch == 0x60C) {
					convertedText << ch;
					continue;
				}

			}
			rtlText << ch;
		}
		else {
			if (!reverseRTL.empty() || !rtlText.empty()) {
				if (!rtlText.empty()) {
					if (NeedConversion(&rtlText)) {
						//if (!spellChecking)
							BIDIConvert(&rtlText);
					}
					else {
						/*if (spellChecking)
							BIDIReverseConvert(&rtlText);
						else*/
							SwitchRTLChars(&rtlText);
					}

					reverseRTL.insert(0, rtlText);
					rtlText.clear();
				}
				convertedText << reverseRTL;
				reverseRTL.clear();
			}
			ltrText << ch;
		}

	}
	
	if (!rtlText.empty() || !reverseRTL.empty()) {
		if (!rtlText.empty()) {
			if (NeedConversion(&rtlText)) {
				//if (!spellChecking)
					BIDIConvert(&rtlText);
			}
			else {
				/*if (spellChecking)
					BIDIReverseConvert(&rtlText);
				else*/
					SwitchRTLChars(&rtlText);
			}

			reverseRTL.insert(0, rtlText);
		}
		convertedText << reverseRTL;
		reverseRTL.clear();
	}
	if (!ltrText.empty()) {
		convertedText << ltrText;
	}

	if (textout)
		*textout = convertedText;
	else
		*textin = convertedText;
}

void ConvertToRTLCharsSpellchecker(wxString* textin, wxString* textout)
{
	if (!textin)
		return;

	size_t len = textin->size();
	if (!len) {
		if (textout)
			*textout = *textin;
		return;
	}

	wxString convertedText;
	wxString ltrText;
	wxString rtlText;
	wxString reverseRTL;
	bool block = false;
	for (int j = 0; j < len; j++) {
		const wxUniChar& ch = (*textin)[j];
		if (ch == L'{' && (!reverseRTL.empty() || !rtlText.empty())) {
			if (!rtlText.empty()) {
				//BIDIReverseConvert(&rtlText);
				reverseRTL.insert(0, rtlText);
				rtlText.clear();
			}
			convertedText << reverseRTL;
			reverseRTL.clear();
			ltrText << ch;
			convertedText << ltrText;
			ltrText.clear();
		}
		else if (iswctype(wint_t(ch), _SPACE | _PUNCT) != 0 &&
			//0x61F = Arabic "?"
			(!rtlText.empty() || !reverseRTL.empty()) && ch != 0x61F && ch != L'\\') {
			if (rtlText.empty())
				reverseRTL.insert(0, ch);
			else {
				//BIDIReverseConvert(&rtlText);
				const wxUniChar& nch = (j + 1 < len) ? (*textin)[j + 1] : L'A';
				bool isNextRTL = IsRTLCharacter(nch) || (iswctype(wint_t(nch), _SPACE | _PUNCT) != 0 &&
					nch != L'{' && nch != L'\\');
				reverseRTL.insert(0, isNextRTL ? ch + rtlText : rtlText);
				rtlText.clear();
				if (!isNextRTL)
					ltrText.insert(0, ch);
			}
		}
		else if (ch == 0x61F) {
			rtlText.insert(0, ch);
		}
		else if (IsRTLCharacter(ch)) {
			if (!ltrText.empty()) {

				convertedText << ltrText;
				ltrText.clear();
				if (ch == 0x60C) {
					convertedText << ch;
					continue;
				}
			}
			rtlText << ch;
		}
		else {
			if (!reverseRTL.empty() || !rtlText.empty()) {
				if (!rtlText.empty()) {
					//BIDIReverseConvert(&rtlText);
					reverseRTL.insert(0, rtlText);
					rtlText.clear();
				}
				convertedText << reverseRTL;
				reverseRTL.clear();
			}
			ltrText << ch;
		}

	}

	if (!rtlText.empty() || !reverseRTL.empty()) {
		if (!rtlText.empty()) {
			//BIDIReverseConvert(&rtlText);
			reverseRTL.insert(0, rtlText);
		}
		convertedText << reverseRTL;
		reverseRTL.clear();
	}
	if (!ltrText.empty()) {
		convertedText << ltrText;
	}

	if (textout)
		*textout = convertedText;
	else
		*textin = convertedText;
}

void BIDIConvert(wxString* text)
{
	UErrorCode errorCode = U_ZERO_ERROR;

	// Open a new UBiDiTransform.

	UBiDiTransform* transform = ubiditransform_open(&errorCode);

	// Run a transformation.
	std::wstring wtext = text->ToStdWstring();

	std::u16string text1(wtext.begin(), wtext.end());
	size_t len = (text1.size() + 3) * 2;
	UChar* text2 = (UChar*)malloc(len);
	if (!text2) {
		KaiLog(L"Cannot allocate bidi conversion text");
		return;
	}

	ubiditransform_transform(transform, text1.data(), -1, text2, (len),
		UBIDI_RTL, UBIDI_LOGICAL, UBIDI_LTR, UBIDI_VISUAL,
		UBIDI_MIRRORING_OFF, U_SHAPE_LETTERS_SHAPE, &errorCode);

	ubiditransform_close(transform);
	std::u16string result16(text2);
	const wchar_t* result = reinterpret_cast<const wchar_t*>(result16.c_str());

	(*text) = wxString(result);
	free(text2);
}

void BIDIReverseConvert(wxString* text)
{
	UErrorCode errorCode = U_ZERO_ERROR;

	// Open a new UBiDiTransform.

	UBiDiTransform* transform = ubiditransform_open(&errorCode);

	// Run a transformation.
	std::wstring wtext = text->ToStdWstring();

	std::u16string text1(wtext.begin(), wtext.end());
	size_t len = (text1.size() + 3) * 2;
	UChar* text2 = (UChar*)malloc(len);
	if (!text2) {
		KaiLog(L"Cannot allocate bidi conversion text");
		return;
	}

	ubiditransform_transform(transform, text1.data(), -1, text2, (len),
		UBIDI_LTR, UBIDI_VISUAL, UBIDI_RTL, UBIDI_LOGICAL,
		UBIDI_MIRRORING_OFF, U_SHAPE_LETTERS_UNSHAPE, &errorCode);

	ubiditransform_close(transform);
	std::u16string result16(text2);
	const wchar_t* result = reinterpret_cast<const wchar_t*>(result16.c_str());

	(*text) = wxString(result);
	free(text2);
}

//put normal RTL text and get converted to LTR when text == NULL or len = 0 function fail
void ConvertToLTR(wxString* textin, wxString* textout)
{
	//if (!textin)
	//	return;

	//size_t len = textin->size();
	//if (!len)
	//	return;

	//wxString ltrText;
	//wxString rtlText;
	//wxString resultText;
	//bool block = false;
	//int lastBracketsPos = 0;

	//for (int j = 0; j < len; j++) {
	//	const wxUniChar& ch = (*textin)[j];
	//	if (ch == L'{') {
	//		if (!rtlText.empty()) {
	//			BIDIReverseConvert(&rtlText);
	//			resultText.insert(lastBracketsPos, rtlText);
	//			rtlText.clear();
	//		}
	//		if (!ltrText.empty()) {
	//			resultText.insert(lastBracketsPos, ltrText);
	//			ltrText.clear();
	//		}
	//		ltrText << ch;
	//		block = true;
	//		lastBracketsPos = j + 1;
	//	}
	//	else if (ch == L'}') {
	//		block = false;
	//		resultText << ltrText << ch;
	//		ltrText.clear();
	//		lastBracketsPos = j + 1;
	//	}
	//	else if (ch == L' ' && !rtlText.empty()) {
	//		BIDIReverseConvert(&rtlText);
	//		resultText.insert(lastBracketsPos, rtlText);
	//		rtlText.clear();
	//		ltrText << ch;
	//		resultText.insert(lastBracketsPos, ch);
	//	}
	//	else if (IsRTLCharacter(ch)) {
	//		if (!ltrText.empty()) {
	//			if (block) {
	//				resultText << ltrText;
	//				block = false;
	//			}
	//			else
	//				resultText.insert(0, ltrText);

	//			ltrText.clear();
	//		}
	//		rtlText << ch;
	//	}//when single { to avoid shit first check RTL character
	//	else if (block) {
	//		ltrText << ch;
	//		lastBracketsPos = j + 1;
	//	}
	//	else {
	//		if (!rtlText.empty()) {
	//			BIDIReverseConvert(&rtlText);
	//			resultText.insert(lastBracketsPos, rtlText);
	//			rtlText.clear();
	//		}
	//		ltrText << ch;
	//	}

	//}
	//if (!ltrText.empty()) {
	//	if (block)
	//		resultText << ltrText;
	//	else
	//		resultText.insert(0, ltrText);
	//}
	//if (!rtlText.empty()) {
	//	BIDIReverseConvert(&rtlText);
	//	resultText.insert(lastBracketsPos, rtlText);
	//}

	//if (textout)
	//	*textout = resultText;
	//else
	//	*textin = resultText;

	if (!textin)
		return;

	size_t len = textin->size();
	if (!len)
		return;

	wxString ltrText;
	wxString rtlText;
	wxString rtlWord;
	wxString resultText;

	for (int j = 0; j < len; j++) {
		const wxUniChar& ch = (*textin)[j];
		if (ch == L'{'/* || ch == L'}'*/) {
			if (!ltrText.empty()) {
				resultText << ltrText;
				ltrText.clear();
			}
			if (!rtlText.empty()) {
				BIDIReverseConvert(&rtlText);
				resultText << rtlText;
				rtlText.clear();
			}
			ltrText << ch;
		}
		else if (IsRTLCharacter(ch)) {
			if (!ltrText.empty()) {

				resultText << ltrText;
				ltrText.clear();
			}
			rtlText << ch;
		}
		else {
			if (!rtlText.empty()) {
				BIDIReverseConvert(&rtlText);
				resultText << rtlText;
				rtlText.clear();
			}
			ltrText << ch;
		}

	}
	if (!ltrText.empty()) {
		resultText << ltrText;
	}
	if (!rtlText.empty()) {
		BIDIReverseConvert(&rtlText);
		resultText << rtlText;
	}

	if (textout)
		*textout = resultText;
	else
		*textin = resultText;
}

void ConvertToLTRChars(wxString* textin, wxString* textout)
{
	if (!textin)
		return;

	size_t len = textin->size();
	if (!len) {
		if (textout)
			*textout = *textin;
		return;
	}

	wxString convertedText;
	wxString ltrText;
	wxString rtlText;
	wxString reverseRTL;
	bool block = false;
	int lastBracket = 0;
	for (int j = 0; j < len; j++) {
		const wxUniChar& ch = (*textin)[j];
		if (ch == L'{' && (!reverseRTL.empty() || !rtlText.empty())) {
			if (!rtlText.empty()) {
				BIDIReverseConvert(&rtlText);
				//SwitchRTLChars(&rtlText);
				reverseRTL.insert(0, rtlText);
				rtlText.clear();
			}
			convertedText << reverseRTL;
			reverseRTL.clear();
			ltrText << ch;
			convertedText << ltrText;
			ltrText.clear();
		}
		else if (iswctype(wint_t(ch), _SPACE | _PUNCT) != 0 && 
			(!rtlText.empty() || !reverseRTL.empty()) && ch != 0x61F && ch != L'\\') {
			if (rtlText.empty())
				reverseRTL.insert(0, ch);
			else {
				BIDIReverseConvert(&rtlText);
				//SwitchRTLChars(&rtlText);
				const wxUniChar& nch = (j + 1 < len) ? (*textin)[j + 1] : L'A';
				bool isNextRTL = IsRTLCharacter(nch) || (iswctype(wint_t(nch), _SPACE | _PUNCT) != 0 && 
					nch != L'{' && nch != L'\\');
				reverseRTL.insert(0, isNextRTL ? ch + rtlText : rtlText);
				rtlText.clear();
				if (!isNextRTL)
					ltrText.insert(0, ch);
			}
		}
		else if (IsRTLCharacter(ch)) {
			if (!ltrText.empty()) {

				convertedText << ltrText;
				ltrText.clear();
				if (ch == 0x60C) {
					convertedText << ch;
					continue;
				}
			}
			rtlText << ch;
		}
		else {
			if (!reverseRTL.empty() || !rtlText.empty()) {
				if (!rtlText.empty()) {
					BIDIReverseConvert(&rtlText);
					reverseRTL.insert(0, rtlText);
					rtlText.clear();
				}
				convertedText << reverseRTL;
				reverseRTL.clear();
			}
			ltrText << ch;
		}

	}
	if (!rtlText.empty() || !reverseRTL.empty()) {
		if (!rtlText.empty()) {
			BIDIReverseConvert(&rtlText);
			reverseRTL.insert(0, rtlText);
		}
		//SwitchRTLChars(&rtlText);
		convertedText << reverseRTL;
	}
	if (!ltrText.empty()) {
		convertedText << ltrText;
	}
	

	if (textout)
		*textout = convertedText;
	else
		*textin = convertedText;
}


//taken from https://stackoverflow.com/questions/4330951/how-to-detect-whether-a-character-belongs-to-a-right-to-left-language
bool IsRTLCharacter(const wxUniChar& ch)
{
	unsigned int c = ch.GetValue();
	if (c >= 0x5BE && c <= 0x10B7F)
	{
		if (c <= 0x85E)
		{
			if (c == 0x5BE)                        return true;
			else if (c == 0x5C0)                   return true;
			else if (c == 0x5C3)                   return true;
			else if (c == 0x5C6)                   return true;
			else if (0x5D0 <= c && c <= 0x5EA)     return true;
			else if (0x5F0 <= c && c <= 0x5F4)     return true;
			else if (c == 0x608)                   return true;
			else if (c == 0x60B)                   return true;
			else if (c == 0x60C)                   return true;
			else if (c == 0x60D)                   return true;
			else if (c == 0x61B)                   return true;
			else if (0x61E <= c && c <= 0x64A)     return true;
			else if (c >= 0x64B && c <= 0x65F)     return true;
			else if (0x66D <= c && c <= 0x66F)     return true;
			else if (0x671 <= c && c <= 0x6D5)     return true;
			else if (0x6E5 <= c && c <= 0x6E6)     return true;
			else if (0x6EE <= c && c <= 0x6EF)     return true;
			else if (0x6FA <= c && c <= 0x70D)     return true;
			else if (c == 0x710)                   return true;
			else if (0x712 <= c && c <= 0x72F)     return true;
			else if (0x74D <= c && c <= 0x7A5)     return true;
			else if (c == 0x7B1)                   return true;
			else if (0x7C0 <= c && c <= 0x7EA)     return true;
			else if (0x7F4 <= c && c <= 0x7F5)     return true;
			else if (c == 0x7FA)                   return true;
			else if (0x800 <= c && c <= 0x815)     return true;
			else if (c == 0x81A)                   return true;
			else if (c == 0x824)                   return true;
			else if (c == 0x828)                   return true;
			else if (0x830 <= c && c <= 0x83E)     return true;
			else if (0x840 <= c && c <= 0x858)     return true;
			else if (c == 0x85E)                   return true;
		}
		else if (c == 0x200F)                      return true;
		else if (c >= 0xFB1D)
		{
			if (c == 0xFB1D)                       return true;
			else if (0xFB1F <= c && c <= 0xFB28)   return true;
			else if (0xFB2A <= c && c <= 0xFB36)   return true;
			else if (0xFB38 <= c && c <= 0xFB3C)   return true;
			else if (c == 0xFB3E)                  return true;
			else if (0xFB40 <= c && c <= 0xFB41)   return true;
			else if (0xFB43 <= c && c <= 0xFB44)   return true;
			else if (0xFB46 <= c && c <= 0xFBC1)   return true;
			else if (0xFBD3 <= c && c <= 0xFD3D)   return true;
			else if (0xFD50 <= c && c <= 0xFD8F)   return true;
			else if (0xFD92 <= c && c <= 0xFDC7)   return true;
			else if (0xFDF0 <= c && c <= 0xFDFC)   return true;
			else if (0xFE70 <= c && c <= 0xFE74)   return true;
			else if (0xFE76 <= c && c <= 0xFEFC)   return true;
			else if (0x10800 <= c && c <= 0x10805) return true;
			else if (c == 0x10808)                 return true;
			else if (0x1080A <= c && c <= 0x10835) return true;
			else if (0x10837 <= c && c <= 0x10838) return true;
			else if (c == 0x1083C)                 return true;
			else if (0x1083F <= c && c <= 0x10855) return true;
			else if (0x10857 <= c && c <= 0x1085F) return true;
			else if (0x10900 <= c && c <= 0x1091B) return true;
			else if (0x10920 <= c && c <= 0x10939) return true;
			else if (c == 0x1093F)                 return true;
			else if (c == 0x10A00)                 return true;
			else if (0x10A10 <= c && c <= 0x10A13) return true;
			else if (0x10A15 <= c && c <= 0x10A17) return true;
			else if (0x10A19 <= c && c <= 0x10A33) return true;
			else if (0x10A40 <= c && c <= 0x10A47) return true;
			else if (0x10A50 <= c && c <= 0x10A58) return true;
			else if (0x10A60 <= c && c <= 0x10A7F) return true;
			else if (0x10B00 <= c && c <= 0x10B35) return true;
			else if (0x10B40 <= c && c <= 0x10B55) return true;
			else if (0x10B58 <= c && c <= 0x10B72) return true;
			else if (0x10B78 <= c && c <= 0x10B7F) return true;
		}
	}
	return false;
}

bool IsRTLConvertedCharacter(const wxUniChar& ch)
{
	unsigned int c = ch.GetValue();
	if (c >= 0xFB1D)
	{
		if (c == 0xFB1D)                       return true;
		else if (0xFB1F <= c && c <= 0xFB28)   return true;
		else if (0xFB2A <= c && c <= 0xFB36)   return true;
		else if (0xFB38 <= c && c <= 0xFB3C)   return true;
		else if (c == 0xFB3E)                  return true;
		else if (0xFB40 <= c && c <= 0xFB41)   return true;
		else if (0xFB43 <= c && c <= 0xFB44)   return true;
		else if (0xFB46 <= c && c <= 0xFBC1)   return true;
		else if (0xFBD3 <= c && c <= 0xFD3D)   return true;
		else if (0xFD50 <= c && c <= 0xFD8F)   return true;
		else if (0xFD92 <= c && c <= 0xFDC7)   return true;
		else if (0xFDF0 <= c && c <= 0xFDFC)   return true;
		else if (0xFE70 <= c && c <= 0xFE74)   return true;
		else if (0xFE76 <= c && c <= 0xFEFC)   return true;
		else if (0x10800 <= c && c <= 0x10805) return true;
		else if (c == 0x10808)                 return true;
		else if (0x1080A <= c && c <= 0x10835) return true;
		else if (0x10837 <= c && c <= 0x10838) return true;
		else if (c == 0x1083C)                 return true;
		else if (0x1083F <= c && c <= 0x10855) return true;
		else if (0x10857 <= c && c <= 0x1085F) return true;
		else if (0x10900 <= c && c <= 0x1091B) return true;
		else if (0x10920 <= c && c <= 0x10939) return true;
		else if (c == 0x1093F)                 return true;
		else if (c == 0x10A00)                 return true;
		else if (0x10A10 <= c && c <= 0x10A13) return true;
		else if (0x10A15 <= c && c <= 0x10A17) return true;
		else if (0x10A19 <= c && c <= 0x10A33) return true;
		else if (0x10A40 <= c && c <= 0x10A47) return true;
		else if (0x10A50 <= c && c <= 0x10A58) return true;
		else if (0x10A60 <= c && c <= 0x10A7F) return true;
		else if (0x10B00 <= c && c <= 0x10B35) return true;
		else if (0x10B40 <= c && c <= 0x10B55) return true;
		else if (0x10B58 <= c && c <= 0x10B72) return true;
		else if (0x10B78 <= c && c <= 0x10B7F) return true;
	}
	return false;
}

bool CheckRTL(const wxString* text)
{
	for (size_t i = 0; i < text->size(); i++) {
		const wxUniChar& ch = (*text)[i];
		if (IsRTLCharacter(ch)) {
			return true;
		}
	}
	return false;
}

bool CheckRTLConverted(const wxString* text)
{
	for (size_t i = 0; i < text->size(); i++) {
		const wxUniChar& ch = (*text)[i];
		if (IsRTLConvertedCharacter(ch)) {
			return true;
		}
	}
	return false;
}

void SwitchRTLChars(wxString* text)
{
	wxString result;
	for (size_t i = 0; i < text->size(); i++) {
		result.insert(0, (*text)[i]);
	}
	*text = result;
}

bool NeedConversion(const wxString* text)
{
	//bool result;
	for (size_t i = 0; i < text->size(); i++) {
		const wxUniChar& ch = (*text)[i];
		if (ch.GetValue() >= 0xFB1D)
			return false;
	}
	return true;
}
