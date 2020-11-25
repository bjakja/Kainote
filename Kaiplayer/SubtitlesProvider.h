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

#include "SubtitlesProviderManager.h"
#ifdef subsProvider

#include <wx/wx.h>
#include <wx/arrstr.h>
#include <atomic>
#include "CsriMod.h"
extern "C" {
#include <libass/ass.h>
}



class SubtitlesProvider
{
public:
	virtual ~SubtitlesProvider(){};
	SubtitlesProvider(){};
	virtual void Draw(unsigned char* buffer, int time){};
	virtual bool Open(TabPanel *tab, int flag, wxString *text){ return false; };
	//for styles preview
	virtual bool OpenString(wxString *text){ return false; };
	virtual void SetVideoParameters(const wxSize& size, unsigned char format, bool isSwapped) {};
	virtual void ReloadLibraries(bool destroyExisted = false) { };
	virtual bool IsLibass() { return false; }
	//implementation in subtitlesVsfilter
	static void DestroySubtitlesProvider();
	static ASS_Renderer *m_Libass;
	static ASS_Library *m_Library;
private:
	SubtitlesProvider(const SubtitlesProvider &copy) = delete;
protected:
	wxSize m_VideoSize;
	unsigned char m_Format;
	bool m_IsSwapped;
	bool m_HasParameters = false;
	char m_BytesPerColor = 4;
	static csri_rend *m_CsriRenderer;
};


class SubtitlesVSFilter : public SubtitlesProvider
{
public:
	SubtitlesVSFilter();
	virtual ~SubtitlesVSFilter();
	void Draw(unsigned char* buffer, int time);
	bool Open(TabPanel *tab, int flag, wxString *text);
	bool OpenString(wxString *text);
	static void GetProviders(wxArrayString *providerList);
	void SetVideoParameters(const wxSize& size, unsigned char format, bool isSwapped);
private:
	csri_frame *m_CsriFrame = NULL;
	csri_fmt *m_CsriFormat = NULL;
	csri_inst *m_CsriInstance = NULL;
	csri_rend *GetVSFilter();
};

class SubtitlesLibass : public SubtitlesProvider
{
public:
	SubtitlesLibass();
	virtual ~SubtitlesLibass();
	void Draw(unsigned char* buffer, int time);
	bool Open(TabPanel *tab, int flag, wxString *text);
	bool OpenString(wxString *text);
	void SetVideoParameters(const wxSize& size, unsigned char format, bool isSwapped);
	void ReloadLibraries(bool destroyExisted = false) override;
	bool IsLibass() { return true; }
	ASS_Track *m_AssTrack = NULL;
	static std::atomic<bool> m_IsReady;
	HANDLE thread = NULL;
	wxSize m_VideoSize;
	volatile bool m_SubsSkipped = false;
	static wxMutex openMutex;
};

#endif