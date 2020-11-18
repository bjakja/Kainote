//  Copyright (c) 2020, Marcin Drob

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

#pragma  once

#define subsProvider 0

#ifdef subsProvider
#include "wx/arrstr.h"
#include "wx/window.h"
#include <vector>

enum
{
	//set close subtitles to 0 to work with NULL
	CLOSE_SUBTITLES,
	OPEN_DUMMY,
	OPEN_WHOLE_SUBTITLES,
	OPEN_HAS_OWN_TEXT
};

class TabPanel;
class SubtitlesProvider;

class SubtitlesProviderManager
{
public:
	static SubtitlesProviderManager *Get();
	void Release();
	//set parameters first
	void Draw(unsigned char* buffer, int time);
	//set parameters first
	bool Open(TabPanel *tab, int flag, wxString *text);
	//set parameters first
	//for styles preview and visuals
	bool OpenString(wxString *text);
	void SetVideoParameters(const wxSize& size, unsigned char format, bool isSwapped);
	bool IsLibass();
	static void ReloadLibraries();
	static void GetProviders(wxArrayString *providerList);
	static void DestroyProviders();
	static void DestroySubsProvider();
private:
	~SubtitlesProviderManager();
	SubtitlesProviderManager() {};
	SubtitlesProviderManager(const SubtitlesProviderManager &copy) = delete;
	SubtitlesProvider *SP = NULL;
	SubtitlesProvider *GetProvider();
	static std::vector< SubtitlesProviderManager*> gs_Base;
};

#endif