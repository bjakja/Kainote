//  Copyright (c) 2020 - 2026, Marcin Drob

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
#include "WinUndef.h"
#include "wx/arrstr.h"
#include "wx/window.h"
#include "wx/gdicmn.h"
#include <vector>


enum
{
	//set close subtitles to 0 to work with nullptr
	CLOSE_SUBTITLES,
	OPEN_DUMMY,
	OPEN_WHOLE_SUBTITLES,
	OPEN_HAS_OWN_TEXT
};

class SubtitlesProvider;
class TabPanel;

class SubtitlesProviderManager
{
public:
	static SubtitlesProviderManager *Get();
	void Release();
	//set parameters first
	void Draw(unsigned char* buffer, int time);
	//set parameters first; see SubtitlesProvider::DrawOverlay
	bool DrawOverlay(unsigned char* overlay, int time, wxRect* dirty);
	//set parameters first; takes the text, nullptr closes the subtitles
	bool Open(int flag, wxString *text);
	// false after dummy or own text was shown, or after edits it doesn't show yet
	bool ShowsWholeSubtitles() const { return m_ShowsWholeSubtitles; }
	void MarkOutdated() { m_ShowsWholeSubtitles = false; }
	//set parameters first
	//for styles preview and visuals
	bool OpenString(wxString *text);
	void SetVideoParameters(const wxSize& size, unsigned char format, bool isSwapped);
	bool IsLibass();
	bool CanPrepare();
	// see SubtitlesProvider::Prepare; takes the text
	void Prepare(wxString *text);
	static bool ReloadLibraries();
	static void GetProviders(wxArrayString *providerList);
	static void DestroyProviders();
	static void DestroySubsProvider();
private:
	~SubtitlesProviderManager();
	SubtitlesProviderManager() {};
	SubtitlesProviderManager(const SubtitlesProviderManager &copy) = delete;
	SubtitlesProvider *SP = nullptr;
	bool m_ShowsWholeSubtitles = false;
	SubtitlesProvider *GetProvider();
	static std::vector< SubtitlesProviderManager*> gs_Base;
};

