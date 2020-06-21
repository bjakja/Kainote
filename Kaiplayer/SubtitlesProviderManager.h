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

#ifdef subsProvider
#include "wx/arrstr.h"
#include "wx/window.h"

enum
{
	OPEN_DUMMY,
	OPEN_TO_END,
	OPEN_WHOLE_SUBTITLES,
	CLOSE_SUBTITLES
};

class TabPanel;
class SubtitlesProvider;

class SubtitlesProviderManager
{
public:
	~SubtitlesProviderManager();
	SubtitlesProviderManager();
	void Draw(unsigned char* buffer, int time);
	bool Open(TabPanel *tab, int flag);
	//for styles preview
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