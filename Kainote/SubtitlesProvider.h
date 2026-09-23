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
#include "SubtitlesProviderManager.h"
#include "CsriMod.h"
#include "WinUndef.h"
#include <wx/window.h>
#include <wx/arrstr.h>
#include <atomic>
#include <string_view>
#include <vector>

extern "C" {
#ifdef __WXMSW__
#include <libass/ass.h> // the submodule keeps its headers under libass/
#else
#include <ass/ass.h>
#endif
}



// The last two scripts parsed, by a hash of their text: editing switches
// between the whole script and one of the edited line, and switching back to
// one that has not changed then needs no parse.
template <typename T>
class ParsedScripts
{
public:
	explicit ParsedScripts(void (*release)(T*)) : m_release(release) {}
	~ParsedScripts() { Clear(); }
	static size_t Hash(const wxString &text)
	{
		return std::hash<std::wstring_view>()(std::wstring_view(text.wc_str(), text.length()));
	}
	T *Find(size_t hash)
	{
		for (size_t i = 0; i < m_items.size(); i++) {
			if (m_items[i].hash == hash) {
				Entry found = m_items[i];
				m_items.erase(m_items.begin() + i);
				m_items.insert(m_items.begin(), found);
				return found.item;
			}
		}
		return nullptr;
	}
	// the one added last is the one in use, so only an older one is released
	void Add(size_t hash, T *item)
	{
		m_items.insert(m_items.begin(), Entry{ hash, item });
		if (m_items.size() > 2) {
			m_release(m_items.back().item);
			m_items.pop_back();
		}
	}
	void Remove(T *item)
	{
		for (size_t i = 0; i < m_items.size(); i++) {
			if (m_items[i].item == item) {
				m_release(item);
				m_items.erase(m_items.begin() + i);
				return;
			}
		}
	}
	void Clear()
	{
		for (Entry &entry : m_items)
			m_release(entry.item);
		m_items.clear();
	}
private:
	struct Entry { size_t hash; T *item; };
	std::vector<Entry> m_items;
	void (*m_release)(T*);
};

class SubtitlesProvider
{
public:
	virtual ~SubtitlesProvider(){};
	SubtitlesProvider(){};
	virtual void Draw(unsigned char* buffer, int time){};
	// Draws into a transparent ARGB overlay the size of the video, which the
	// caller keeps between calls. Returns false when the subtitles look the
	// same as last time; otherwise dirty covers everything that changed.
	virtual bool DrawOverlay(unsigned char* overlay, int time, wxRect* dirty);
	// takes the text; nullptr closes the subtitles
	virtual bool Open(wxString *text){ return false; };
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
	unsigned char m_Format = 0;
	bool m_IsSwapped = false;
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
	bool Open(wxString *text);
	bool OpenString(wxString *text);
	static void GetProviders(wxArrayString *providerList);
	void SetVideoParameters(const wxSize& size, unsigned char format, bool isSwapped);
private:
	bool OpenInstance(wxString *text);
	bool OpenCached(wxString *text);
	csri_frame *m_CsriFrame = nullptr;
	csri_fmt *m_CsriFormat = nullptr;
	// owned by m_Instances
	csri_inst *m_CsriInstance = nullptr;
	ParsedScripts<csri_inst> m_Instances{ csri_close };
	csri_rend *GetVSFilter();
};

class SubtitlesLibass : public SubtitlesProvider
{
public:
	SubtitlesLibass();
	virtual ~SubtitlesLibass();
	void Draw(unsigned char* buffer, int time);
	bool DrawOverlay(unsigned char* overlay, int time, wxRect* dirty) override;
	bool Open(wxString *text);
	bool OpenString(wxString *text);
	void SetVideoParameters(const wxSize& size, unsigned char format, bool isSwapped);
	void ReloadLibraries(bool destroyExisted = false) override;
	bool IsLibass() { return true; }
	ASS_Track *m_AssTrack = nullptr;
	static std::atomic<bool> m_IsReady;
	HANDLE thread = nullptr;
	wxSize m_VideoSize;
	volatile bool m_SubsSkipped = false;
	static wxMutex openMutex;
private:
	// Blends a libass image list onto an ARGB (premultiplied) overlay buffer.
	void BlendImages(ASS_Image* img, unsigned char* buffer);
	ASS_Image* RenderFrame(int time, int* change);
	// call with openMutex locked; takes the text
	bool ReadTrack(wxString* text);
	// owns m_AssTrack
	ParsedScripts<ASS_Track> m_Tracks{ ass_free_track };
	bool m_HasRendered = false;
	// what the last DrawOverlay drew, cleared before the next one draws
	wxRect m_OverlayDrawn;
	// all tabs share m_Libass, whose change detection compares with whatever it rendered last
	static SubtitlesLibass* m_LastRenderer;
};

