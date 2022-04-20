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

#pragma once
#include "Provider.h"
#include "ProgressDialog.h"


class ProviderFFMS2 : public Provider
{
public:
	ProviderFFMS2(const wxString& filename, RendererVideo* renderer, wxWindow* progressSinkWindow, bool* success);
	virtual ~ProviderFFMS2();
	void GetFrameBuffer(unsigned char** buffer) override;
	void GetFrame(int frame, unsigned char* buff) override;
	void GetBuffer(void* buf, long long start, long long count, double vol = 1.0) override;
	bool RAMCache();
	int Init();
	void GetChapters(std::vector<chapter>* _chapters) override {
		if (_chapters) {
			*_chapters = m_chapters;
		}
	};

	ProgressSink* progress = nullptr;
	static int __stdcall UpdateProgress(long long Current, long long Total, void* ICPrivate);
	static void AudioLoad(ProviderFFMS2* parent, bool newIndex, int audiotrack);
	void ClearRAMCache();
	bool DiskCache(bool newIndex);
	void ClearDiskCache();
	void DeleteOldAudioCache();
	wxString ColorMatrixDescription(int cs, int cr);
	void SetColorSpace(const wxString& matrix);
	bool HasVideo();

	bool m_discCache;
	volatile bool m_success;
	volatile bool m_lockGetFrame = true;
	int m_CR;
	int m_CS;
	double m_delay = 0;
	HANDLE m_eventAudioComplete = nullptr;
	wxString m_diskCacheFilename;
	wxString m_colorSpace;
	wxString m_realColorSpace;
	wxString m_indexPath;
	FILE* m_fp = nullptr;
	std::vector<chapter> m_chapters;
	std::thread* m_audioLoadThread = nullptr;
private:
	char m_errmsg[1024];
	char** m_cache = nullptr;
	int m_blockNum = 0;
	void GetAudio(void* buf, long long start, long long count);
	void GetFFMSFrame();
	static unsigned int __stdcall FFMS2Proc(void* cls);
	void Processing();
	volatile bool m_stopLoadingAudio = false;
	wxCriticalSection m_blockAudio;
	wxCriticalSection m_blockFrame;
	FFMS_VideoSource* m_videoSource = nullptr;
	FFMS_AudioSource* m_audioSource = nullptr;
	FFMS_ErrorInfo m_errInfo;
	FFMS_Index* m_index = nullptr;
	const FFMS_Frame* m_FFMS2frame = nullptr;
};