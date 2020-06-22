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
	~SubtitlesProviderManager();
	SubtitlesProviderManager();
	//set parameters first
	void Draw(unsigned char* buffer, int time);
	//set parameters first
	bool Open(TabPanel *tab, int flag, wxString *text);
	//set parameters first
	//for styles preview and visuals
	bool OpenString(wxString *text);
	void SetVideoParameters(const wxSize& size, char bytesPerColor, bool isSwapped);
	static void GetProviders(wxArrayString *providerList);
	static void SetProvider(const wxString &provider);
	static void DestroySubsProvider();
private:
	SubtitlesProviderManager(const SubtitlesProviderManager &copy) = delete;
	static SubtitlesProvider *SP;
	SubtitlesProvider *GetProvider();
};

#endif