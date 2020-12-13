//  Copyright (c) 2017 - 2020, Marcin Drob

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


#include "Utils.h"
#include <ShlObj.h>
#include <wx/msw/private.h>
#include <wx/mstream.h>


void SelectInFolder(const wxString & filename)
{
	CoInitialize(0);
	ITEMIDLIST *pidl = ILCreateFromPathW(filename.wc_str());
	if (pidl) {
		SHOpenFolderAndSelectItems(pidl, 0, 0, 0);
		ILFree(pidl);
	}
	CoUninitialize();
}

void OpenInBrowser(const wxString &adress)
{
	WinStruct<SHELLEXECUTEINFO> sei;
	sei.lpFile = adress.c_str();
	sei.lpVerb = wxT("open");
	sei.nShow = SW_RESTORE;
	sei.fMask = SEE_MASK_FLAG_NO_UI; // we give error message ourselves
	ShellExecuteEx(&sei);
}

bool IsNumberFloat(const wxString &test) {
	bool isnumber = true;
	wxString testchars = L"0123456789.";
	for (size_t i = 0; i < test.length(); i++) {
		wxUniChar ch = test[i];
		if (testchars.Find(ch) == -1) { isnumber = false; break; }
	}
	return isnumber;
}

wxString getfloat(float num, const wxString &format, bool Truncate)
{
	wxString strnum = wxString::Format(L"%" + format, num);
	//if(strnum.find(L'.')!= -1){return strnum.Trim(false);}
	if (!Truncate || format.EndsWith(L".0f")) { return strnum.Trim(false); }
	int rmv = 0;
	bool trim = false;
	for (int i = strnum.Len() - 1; i > 0; i--)
	{
		if (strnum[i] == L'0') { rmv++; }//&&!trim
		//else if(strnum[i]==L'9'){rmv++;trim=true;}
		else if (strnum[i] == L'.') { rmv++; break; }//}if(!trim){
		else {/*if(trim){int tmpc=static_cast < int >(strnum.GetChar(i));tmpc++;strnum[i]=(wxUniChar)tmpc;}*/break; }
	}
	if (rmv) { strnum.RemoveLast(rmv); }
	return strnum.Trim(false);
}



bool LoadDataFromResource(char*& t_data, DWORD& t_dataSize, const wxString& t_name)
{
	bool     r_result = false;
	HGLOBAL  a_resHandle = 0;
	HRSRC    a_resource;

	a_resource = FindResource(0, t_name.wchar_str(), RT_RCDATA);

	if (0 != a_resource)
	{
		a_resHandle = LoadResource(NULL, a_resource);
		if (0 != a_resHandle)
		{
			t_data = (char*)LockResource(a_resHandle);
			t_dataSize = SizeofResource(NULL, a_resource);
			r_result = true;
		}
	}

	return r_result;
}


wxBitmap GetBitmapFromMemory(const char* t_data, const DWORD t_size)
{
	wxMemoryInputStream a_is(t_data, t_size);
	return wxBitmap(wxImage(a_is, wxBITMAP_TYPE_PNG, -1), -1);
}

wxBitmap CreateBitmapFromPngResource(const wxString& t_name)
{
	wxBitmap   r_bitmapPtr;

	char*       a_data = 0;
	DWORD       a_dataSize = 0;

	if (LoadDataFromResource(a_data, a_dataSize, t_name))
	{
		r_bitmapPtr = GetBitmapFromMemory(a_data, a_dataSize);
	}

	return r_bitmapPtr;
}

wxBitmap *CreateBitmapPointerFromPngResource(const wxString& t_name)
{
	wxBitmap  *r_bitmapPtr = NULL;

	char*       a_data = 0;
	DWORD       a_dataSize = 0;

	if (LoadDataFromResource(a_data, a_dataSize, t_name))
	{
		wxMemoryInputStream a_is(a_data, a_dataSize);
		r_bitmapPtr = new wxBitmap(wxImage(a_is, wxBITMAP_TYPE_PNG, -1), -1);
	}

	return r_bitmapPtr;
}

wxImage CreateImageFromPngResource(const wxString& t_name)
{
	wxImage   image;

	char*       a_data = 0;
	DWORD       a_dataSize = 0;

	if (LoadDataFromResource(a_data, a_dataSize, t_name))
	{
		wxMemoryInputStream a_is(a_data, a_dataSize);
		image = wxImage(a_is, wxBITMAP_TYPE_PNG, -1);
	}

	return image;
}

void MoveToMousePosition(wxWindow *win)
{
	wxPoint mst = wxGetMousePosition();
	wxSize siz = win->GetSize();
	siz.x;
	wxRect rc = GetMonitorWorkArea(0, NULL, mst, true);
	mst.x -= (siz.x / 2);
	mst.x = MID(rc.x, mst.x, (rc.width + rc.x) - siz.x);
	mst.y += 15;
	if (mst.y + siz.y > rc.height + rc.y) {
		mst.y = mst.y - siz.y - 30;
		if (mst.y < rc.y) {
			mst.y = (rc.height + rc.y) - siz.y;
		}
	}
	win->Move(mst);
}

wxString MakePolishPlural(int num, const wxString &normal, const wxString &plural2to4, const wxString &pluralRest)
{
	wxString result;
	int div10mod = (num % 10);
	int div100mod = (num % 100);
	if (num == 1 || num == -1) { result = normal; }
	else if ((div10mod >= 2 && div10mod <= 4) && (div100mod < 10 || div100mod>20)) {
		result = plural2to4;
	}
	else {
		result = pluralRest;
	}
	wxString finalResult;
	return finalResult << num << " " << result;
}

BOOL CALLBACK MonitorEnumProc1(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
	std::pair<std::vector<RECT>, bool> *pair = (std::pair<std::vector<RECT>, bool> *)dwData;
	WinStruct<MONITORINFO> monitorinfo;

	if (!GetMonitorInfo(hMonitor, &monitorinfo)) {
		KaiLog(_("Nie mo¿na pobraæ informacji o monitorze"));
		return TRUE;
	}
	//podstawowy monitor ma byæ pierwszy w tablicy
	if (monitorinfo.dwFlags == MONITORINFOF_PRIMARY) {
		pair->first.insert(pair->first.begin(), (pair->second) ? monitorinfo.rcWork : monitorinfo.rcMonitor);
		return TRUE;
	}
	pair->first.push_back((pair->second) ? monitorinfo.rcWork : monitorinfo.rcMonitor);
	return TRUE;
}

wxRect GetMonitorWorkArea(int wmonitor, std::vector<tagRECT> *MonitorRects, const wxPoint &position, bool workArea) {
	std::vector<RECT> MonRects;
	std::pair<std::vector<RECT>, bool> *pair = new std::pair<std::vector<RECT>, bool>(MonRects, workArea);
	EnumDisplayMonitors(NULL, NULL, MonitorEnumProc1, (LPARAM)pair);
	MonRects = pair->first;
	delete pair;
	if (MonitorRects)
		*MonitorRects = MonRects;
	if (MonRects.size() == 0) {
		return { 0, 0, 1920, 1040 };
	}
	wxRect rt(MonRects[0].left, MonRects[0].top,
		abs(MonRects[0].right - MonRects[0].left), abs(MonRects[0].bottom - MonRects[0].top));
	if (wmonitor == -1 || MonRects.size() == 1) {
		return rt;
	}
	else if (wmonitor == 0) {
		int x = position.x;
		int y = position.y;
		for (size_t i = 0; i < MonRects.size(); i++) {
			if (MonRects[i].left <= x && x < MonRects[i].right && MonRects[i].top <= y && y < MonRects[i].bottom) {
				return wxRect(MonRects[i].left, MonRects[i].top,
					abs(MonRects[i].right - MonRects[i].left), abs(MonRects[i].bottom - MonRects[i].top));
			}
		}
	}
	else if (wmonitor < MonRects.size()) {
		return wxRect(MonRects[wmonitor].left, MonRects[wmonitor].top,
			abs(MonRects[wmonitor].right - MonRects[wmonitor].left), abs(MonRects[wmonitor].bottom - MonRects[wmonitor].top));
	}
	return rt;
}

wxRect GetMonitorRect1(int wmonitor, std::vector<tagRECT> *MonitorRects, const wxRect &programRect) {
	std::vector<RECT> MonRects;
	std::pair<std::vector<RECT>, bool> *pair = new std::pair<std::vector<RECT>, bool>(MonRects, false);
	EnumDisplayMonitors(NULL, NULL, MonitorEnumProc1, (LPARAM)pair);
	MonRects = pair->first;
	delete pair;
	if (MonitorRects)
		*MonitorRects = MonRects;
	if (MonRects.size() == 0) {
		return { 0, 0, 1920, 1080 };
	}
	wxRect rt(MonRects[0].left, MonRects[0].top, abs(MonRects[0].right - MonRects[0].left), abs(MonRects[0].bottom - MonRects[0].top));
	if (wmonitor == -1 || MonRects.size() == 1) { return rt; }
	else if (wmonitor == 0) {
		int x = (programRect.width / 2) + programRect.x;
		int y = (programRect.height / 2) + programRect.y;
		for (size_t i = 0; i < MonRects.size(); i++) {
			if (MonRects[i].left <= x && x < MonRects[i].right && MonRects[i].top <= y && y < MonRects[i].bottom) {
				return wxRect(MonRects[i].left, MonRects[i].top, abs(MonRects[i].right - MonRects[i].left), abs(MonRects[i].bottom - MonRects[i].top));
			}
		}
	}
	else {
		return wxRect(MonRects[wmonitor].left, MonRects[wmonitor].top, abs(MonRects[wmonitor].right - MonRects[wmonitor].left), abs(MonRects[wmonitor].bottom - MonRects[wmonitor].top));
	}
	return rt;
}

bool IsNumber(const wxString &test) {
	bool isnumber = true;
	wxString testchars = L"0123456789";
	for (size_t i = 0; i < test.length(); i++) {
		wxUniChar ch = test[i];
		if (testchars.Find(ch) == -1) { isnumber = false; break; }
	}
	return isnumber;
}

#ifdef _M_IX86

typedef struct tagTHREADNAME_INFO
{
	DWORD dwType;
	LPCSTR szName;
	DWORD dwThreadID;
	DWORD dwFlags;
} THREADNAME_INFO;

void SetThreadName(DWORD dwThreadID, LPCSTR szThreadName)
{
	THREADNAME_INFO info;
	info.dwType = 0x1000;
	info.szName = szThreadName;
	info.dwThreadID = dwThreadID;
	info.dwFlags = 0;

	__try
	{
		::RaiseException(0x406D1388, 0, sizeof(info) / sizeof(DWORD), (DWORD *)& info);
	}
	__except (EXCEPTION_CONTINUE_EXECUTION)
	{
	}
}
#else
typedef struct tagTHREADNAME_INFO
{
	size_t dwType;
	LPCSTR szName;
	size_t dwThreadID;
	size_t dwFlags;
} THREADNAME_INFO;

void SetThreadName(size_t dwThreadID, LPCSTR szThreadName)
{
	THREADNAME_INFO info;
	info.dwType = 0x1000;
	info.szName = szThreadName;
	info.dwThreadID = dwThreadID;
	info.dwFlags = 0;

	__try
	{
		::RaiseException(0x406D1388, 0, sizeof(info) / sizeof(size_t), (size_t *)& info);
	}
	__except (EXCEPTION_CONTINUE_EXECUTION)
	{
	}
}
#endif