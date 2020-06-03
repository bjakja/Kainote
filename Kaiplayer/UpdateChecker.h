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

#pragma once

#include <thread>
#include <wx/string.h>

class UpdateChecker
{
public:
	UpdateChecker();
	~UpdateChecker();
	//int CheckForUpdate();
private:
	static int CheckAsynchronously(UpdateChecker *checker, bool closeProgram = true);
	int Downloader(const wchar_t *server, const wchar_t *page, const wchar_t *filename, std::string *output);
	int DownloadZip();
	void Update(bool closeProgram = true);
	bool CheckForUpdate();
	bool updateStable = false;
	bool checkOnClose = false;
	bool dontAskForUpdate = false;
	bool updateOnClose = false;
	int checkIntensity = 0;
	wxString server;
	wxString page;
	wxString savePath;
	std::thread *thread = NULL;
};
