//  Copyright (c) 2026, Marcin Drob

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


#include "ModificationChecker.h"
#include "LogHandler.h"

//first < second
bool LastModificationChecker::CheckDate(SYSTEMTIME* firstDate, SYSTEMTIME* secondDate)
{
	if (firstDate->wYear < secondDate->wYear)
		return true;
	else if (firstDate->wYear > secondDate->wYear)
		return false;

	if (firstDate->wMonth < secondDate->wMonth)
		return true;
	else if (firstDate->wMonth > secondDate->wMonth)
		return false;

	if (firstDate->wDay < secondDate->wDay)
		return true;
	else if (firstDate->wDay > secondDate->wDay)
		return false;

	if (firstDate->wHour < secondDate->wHour)
		return true;
	else if (firstDate->wHour > secondDate->wHour)
		return false;

	if (firstDate->wMinute < secondDate->wMinute)
		return true;
	else if (firstDate->wMinute > secondDate->wMinute)
		return false;

	if (firstDate->wSecond < secondDate->wSecond)
		return true;
	else if (firstDate->wSecond > secondDate->wSecond)
		return false;

	if (firstDate->wMilliseconds < secondDate->wMilliseconds)
		return true;

	return false;
}

LastModificationChecker::LastModificationChecker()
{
}

LastModificationChecker::~LastModificationChecker()
{
}

int LastModificationChecker::NeedReload(const wxString& fullpath, SYSTEMTIME* lastSaveTime)
{
	FILETIME ft{};
	HANDLE ffile = CreateFileW(fullpath.wc_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	//return -1 to show that file not exist;
	if (ffile == INVALID_HANDLE_VALUE)
		return -1;

	if (!GetFileTime(ffile, 0, 0, &ft)) {
		KaiLogSilent(L"Could not get subtitles modification time");
		CloseHandle(ffile);
		return 0;
	}
	CloseHandle(ffile);
	SYSTEMTIME st;
	if (!FileTimeToSystemTime(&ft, &st)) {
		KaiLogSilent(L"Could not convert time from file time to system time");
		return 0;
	}
	if (CheckDate(lastSaveTime, &st)) {
		return 1;
	}

	return 0;
}


LastModificationChecker ModifChecker;
