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
#include "UtilsWindows.h"
#include "ProviderFFMS2.h"
#include "KaiMessageBox.h"
#include "Videobox.h"
#include "kainoteApp.h"
#include <wx/dir.h>
#include <wx/filename.h>


ProviderFFMS2::ProviderFFMS2(const wxString& filename, RendererVideo* renderer, wxWindow* progressSinkWindow, bool* _success)
	: Provider(filename, renderer)
	, m_eventAudioComplete(CreateEvent(0, FALSE, FALSE, 0))
{
	if (!Options.AudioOpts && !Options.LoadAudioOpts()) { KaiMessageBox(_("Nie można wczytać opcji audio"), _("Błąd")); }
	m_discCache = !Options.GetBool(AUDIO_RAM_CACHE);

	m_success = false;
	progress = new ProgressSink(progressSinkWindow, _("Indeksowanie pliku wideo"));

	if (renderer) {
		unsigned int threadid = 0;
		m_thread = (HANDLE)_beginthreadex(0, 0, FFMS2Proc, this, 0, &threadid);
		//CreateThread( NULL, 0,  (LPTHREAD_START_ROUTINE)FFMS2Proc, this, 0, 0);
		SetThreadPriority(m_thread, THREAD_PRIORITY_TIME_CRITICAL);
		SetThreadName(threadid, "VideoThread");
		progress->ShowDialog();
		WaitForSingleObject(m_eventComplete, INFINITE);
		ResetEvent(m_eventComplete);
		*_success = m_success;
	}
	else {
		progress->SetAndRunTask([=]() {return Init(); });
		progress->ShowDialog();
		*_success = ((int)progress->Wait() == 1);
	}
	SAFE_DELETE(progress);
	if (m_index) { FFMS_DestroyIndex(m_index); }

}

unsigned int __stdcall ProviderFFMS2::FFMS2Proc(void* cls)
{
	((ProviderFFMS2*)cls)->Processing();
	return 0;
}

void ProviderFFMS2::Processing()
{
	HANDLE events_to_wait[] = {
		m_eventStartPlayback,
		m_eventSetPosition,
		m_eventKillSelf
	};

	m_success = (Init() == 1);

	progress->EndModal();

	m_framePlane = m_height * m_width * 4;
	int tdiff = 0;

	SetEvent(m_eventComplete);
	if (m_width < 0) { return; }

	while (1) {
		DWORD wait_result = WaitForMultipleObjects(sizeof(events_to_wait) / sizeof(HANDLE), events_to_wait, FALSE, INFINITE);

		if (wait_result == WAIT_OBJECT_0 + 0)
		{
			byte* buff = (byte*)m_renderer->m_FrameBuffer;
			int acttime;
			while (1) {

				if (m_renderer->m_Frame != m_lastFrame) {
					m_renderer->m_Time = m_timecodes[m_renderer->m_Frame];
					m_lastFrame = m_renderer->m_Frame;
				}
				GetFFMSFrame();

				if (!m_FFMS2frame) {
					continue;
				}
				memcpy(&buff[0], m_FFMS2frame->Data[0], m_framePlane);

				m_renderer->DrawTexture(buff);
				m_renderer->Render(false);

				if (m_renderer->m_Time >= m_renderer->m_PlayEndTime || 
					m_renderer->m_Frame >= m_numFrames - 1) {
					wxCommandEvent* evt = new wxCommandEvent(wxEVT_COMMAND_BUTTON_CLICKED, ID_END_OF_STREAM);
					wxQueueEvent(m_renderer->videoControl, evt);
					break;
				}
				else if (m_renderer->m_State != Playing) {
					break;
				}
				acttime = timeGetTime() - m_renderer->m_LastTime;

				m_renderer->m_Frame++;
				m_renderer->m_Time = m_timecodes[m_renderer->m_Frame];

				tdiff = m_renderer->m_Time - acttime;

				if (tdiff > 0) { Sleep(tdiff); }
				else if (tdiff < -20) {
					while (1) {
						int frameTime = m_timecodes[m_renderer->m_Frame];
						if (frameTime >= acttime || frameTime >= m_renderer->m_PlayEndTime || 
							m_renderer->m_Frame >= m_numFrames) {
							if (m_renderer->m_Frame >= m_numFrames) {
								m_renderer->m_Frame = m_numFrames - 1;
								m_renderer->m_Time = m_renderer->m_PlayEndTime;
							}
							break;
						}
						else {
							m_renderer->m_Frame++;
						}
					}

				}

			}
		}
		else if (wait_result == WAIT_OBJECT_0 + 1) {
			//entire seeking have to be in this thread or subtitles will out of sync
			m_renderer->SetFFMS2Position(m_changedTime, m_isStartTime);
		}
		else {
			break;
		}

	}
}


int ProviderFFMS2::Init()
{

	FFMS_Init(0, 1);

	m_errInfo.Buffer = m_errmsg;
	m_errInfo.BufferSize = sizeof(m_errmsg);
	m_errInfo.ErrorType = FFMS_ERROR_SUCCESS;
	m_errInfo.SubType = FFMS_ERROR_SUCCESS;

	FFMS_Indexer* Indexer = FFMS_CreateIndexer(m_filename.utf8_str(), &m_errInfo);
	if (!Indexer) {
		KaiLogDebug(wxString::Format(_("Wystąpił błąd indeksowania: %s"), m_errInfo.Buffer)); return 0;
	}

	int NumTracks = FFMS_GetNumTracksI(Indexer);
	int audiotrack = -1;
	wxArrayInt audiotable;
	int videotrack = -1;

	for (int i = 0; i < NumTracks; i++) {
		if (FFMS_GetTrackTypeI(Indexer, i) == FFMS_TYPE_VIDEO && videotrack == -1) {
			videotrack = i;
		}
		else if (FFMS_GetTrackTypeI(Indexer, i) == FFMS_TYPE_AUDIO) {
			audiotable.Add(i);
		}
	}
	wxString ext = m_filename.AfterLast('.').Lower();
	bool ismkv = (ext == L"mkv");
	bool hasMoreAudioTracks = audiotable.size() > 1;

	FFMS_Chapters* chapters = FFMS_GetChapters(Indexer);
	if (chapters && m_renderer) {
		for (int j = 0; j < chapters->NumOfChapters; j++) {
			chapter ch;
			ch.name = wxString(chapters->Chapters[j].Title, wxConvUTF8);
			ch.time = (int)(chapters->Chapters[j].Start);
			m_chapters.push_back(ch);
		}
		FFMS_FreeChapters(&chapters);
	}

	if (hasMoreAudioTracks) {

		wxArrayString tracks;
		wxArrayString enabled;
		Options.GetTableFromString(ACCEPTED_AUDIO_STREAM, enabled, L";");
		int enabledSize = enabled.GetCount();
		int lowestIndex = enabledSize;
		for (size_t j = 0; j < audiotable.GetCount(); j++) {
			const char* namec = FFMS_GetTrackName(Indexer, audiotable[j]);
			const char* languagec = FFMS_GetTrackLanguage(Indexer, audiotable[j]);
			wxString name = (namec) ? wxString(namec, wxConvUTF8) : emptyString;
			wxString language = (languagec) ? wxString(languagec, wxConvUTF8) : emptyString;
			if (languagec) {
				if (language.Find(L'[', true) != -1 && language.Find(L']', true) != -1) {
					size_t startBracket = language.Find(L'[', true);
					size_t endBracket = language.Find(L']', true);
					if (name.empty() && startBracket > 1) {
						name = language.Mid(0, startBracket);
					}
					language = language.Mid(startBracket + 1, endBracket - (startBracket + 1));
				}
				if (enabledSize) {
					int index = enabled.Index(language, false);
					if (index > -1 && index < lowestIndex) {
						lowestIndex = index;
						audiotrack = audiotable[j];
						continue;
					}
				}
			}
			wxString description;
			if (namec) {
				description = name;
			}
			if (languagec) {
				if (namec)
					description << L" [";
				description << language;
				if (namec)
					description << L"]";
			}
			if (description.empty())
				description = _("Bez nazwy");

			wxString all;
			wxString codecName(FFMS_GetCodecNameI(Indexer, audiotable[j]), wxConvUTF8);
			all << audiotable[j] << L": " << description <<
				L" (" << codecName << L")";
			tracks.Add(all);
		}
		if (lowestIndex < enabledSize) {
			tracks.Clear();
			hasMoreAudioTracks = false;
			goto done;
		}
		
		audiotrack = progress->ShowSecondaryDialog([=]() {
			kainoteApp* Kaia = (kainoteApp*)wxTheApp;

			KaiListBox tracks1(Kaia->Frame, tracks, _("Wybierz ścieżkę"), true);
			if (tracks1.ShowModal() == wxID_OK) {
				int result = wxAtoi(tracks1.GetSelection().BeforeFirst(':'));
				return result;
			}
			return -1;
			});

		if (audiotrack == -1) {
			FFMS_CancelIndexing(Indexer);
			return 0;
		}

	}
	else if (audiotable.size() > 0) {
		audiotrack = audiotable[0];
	}
done:

	m_indexPath = Options.pathfull + L"\\Indices\\" + m_filename.AfterLast(L'\\').BeforeLast(L'.') +
		wxString::Format(L"_%i.ffindex", audiotrack);

	if (wxFileExists(m_indexPath)) {
		m_index = FFMS_ReadIndex(m_indexPath.utf8_str(), &m_errInfo);
		if (!m_index) {/*do nothing to skip*/ }
		else if (FFMS_IndexBelongsToFile(m_index, m_filename.utf8_str(), &m_errInfo))
		{
			FFMS_DestroyIndex(m_index);
			m_index = NULL;
		}
		else {
			FFMS_CancelIndexing(Indexer);
		}

	}
	bool newIndex = false;
	if (!m_index) {
		FFMS_TrackIndexSettings(Indexer, audiotrack, 1, 0);
		FFMS_SetProgressCallback(Indexer, UpdateProgress, (void*)progress);
		m_index = FFMS_DoIndexing2(Indexer, FFMS_IEH_IGNORE, &m_errInfo);
		//in this moment indexer was released, there no need to release it
		if (m_index == NULL) {
			if (wxString(m_errInfo.Buffer).StartsWith(L"Cancelled")) {
				//No need spam user that he clicked cancel button
				//KaiLog(_("Indeksowanie anulowane przez użytkownika"));
			}
			else {
				KaiLog(wxString::Format(_("Wystąpił błąd indeksowania: %s"), m_errInfo.Buffer));
			}
			//FFMS_CancelIndexing(Indexer);
			return 0;
		}
		if (!wxDir::Exists(m_indexPath.BeforeLast(L'\\')))
		{
			wxDir::Make(m_indexPath.BeforeLast(L'\\'));
		}
		if (FFMS_WriteIndex(m_indexPath.utf8_str(), m_index, &m_errInfo))
		{
			KaiLogDebug(wxString::Format(_("Nie można zapisać indeksu, wystąpił błąd %s"), m_errInfo.Buffer));
			//FFMS_DestroyIndex(index);
			//FFMS_CancelIndexing(Indexer);
			//return 0;
		}
		newIndex = true;
	}


	if (videotrack != -1) {
		SYSTEM_INFO sysinfo;
		GetSystemInfo(&sysinfo);
		try {
			m_videoSource = FFMS_CreateVideoSource(
				m_filename.utf8_str(),
				videotrack,
				m_index,
				sysinfo.dwNumberOfProcessors,
				Options.GetInt(FFMS2_VIDEO_SEEKING),
				&m_errInfo);
		}
		catch (...) {}
		//Since the index is copied into the video source object upon its creation,
		//we can and should now destroy the index object. 

		if (m_videoSource == NULL) {
			if (audiotrack == -1) {
				KaiLog(_("Nie można utworzyć VideoSource."));
				return 0;
			}
			else
				goto audio;
		}

		const FFMS_VideoProperties* videoprops = FFMS_GetVideoProperties(m_videoSource);

		m_numFrames = videoprops->NumFrames;
		m_duration = videoprops->LastTime;
		//Delay = videoprops->FirstTime + (Options.GetInt("Audio Delay")/1000);
		m_FPS = (float)videoprops->FPSNumerator / (float)videoprops->FPSDenominator;

		const FFMS_Frame* propframe = FFMS_GetFrame(m_videoSource, 0, &m_errInfo);

		m_width = propframe->EncodedWidth;
		m_height = propframe->EncodedHeight;
		m_arwidth = (videoprops->SARNum == 0) ? m_width : (float)m_width * ((float)videoprops->SARNum / (float)videoprops->SARDen);
		m_arheight = m_height;
		m_CS = propframe->ColorSpace;
		m_CR = propframe->ColorRange;
		while (1) {
			bool divided = false;
			for (int i = 10; i > 1; i--) {
				if ((m_arwidth % i) == 0 && (m_arheight % i) == 0) {
					m_arwidth /= i; m_arheight /= i;
					divided = true;
					break;
				}
			}
			if (!divided) { break; }
		}

		int pixfmt[2];
		pixfmt[0] = FFMS_GetPixFmt("bgra");//PIX_FMT_YUYV422; //PIX_FMT_NV12 == 25  PIX_FMT_YUVJ420P;//PIX_FMT_YUV411P;//PIX_FMT_YUV420P; //PIX_FMT_YUYV422;//PIX_FMT_NV12;//FFMS_GetPixFmt("bgra");PIX_FMT_YUYV422;//
		pixfmt[1] = -1;

		if (FFMS_SetOutputFormatV2(m_videoSource, pixfmt, m_width, m_height, FFMS_RESIZER_BILINEAR, &m_errInfo)) {
			KaiLog(_("Nie można przekonwertować wideo na RGBA"));
			return 0;
		}

		if (m_renderer) {
			SubsGrid* grid = ((TabPanel*)m_renderer->videoControl->GetParent())->Grid;
			const wxString& colormatrix = grid->GetSInfo(L"YCbCr Matrix");
			bool changeMatrix = false;
			if (m_CS == FFMS_CS_UNSPECIFIED) {
				m_CS = m_width > 1024 || m_height >= 600 ? FFMS_CS_BT709 : FFMS_CS_BT470BG;
			}
			m_colorSpace = m_realColorSpace = ColorMatrixDescription(m_CS, m_CR);
			if (m_CS == FFMS_CS_BT709 && colormatrix == L"TV.709") {
				if (FFMS_SetInputFormatV(m_videoSource, FFMS_CS_BT709, m_CR, FFMS_GetPixFmt(""), &m_errInfo)) {
					KaiLog(_("Nie można zmienić macierzy YCbCr"));
				}
			}
			if (colormatrix == L"TV.601") {
				m_colorSpace = ColorMatrixDescription(FFMS_CS_BT470BG, m_CR);
				if (FFMS_SetInputFormatV(m_videoSource, FFMS_CS_BT470BG, m_CR, FFMS_GetPixFmt(""), &m_errInfo)) {
					KaiLog(_("Nie można zmienić macierzy YCbCr"));
				}
			}
			else if (colormatrix == L"TV.709") {
				m_colorSpace = ColorMatrixDescription(FFMS_CS_BT709, m_CR);
			}
		}

		FFMS_Track* FrameData = FFMS_GetTrackFromVideo(m_videoSource);
		if (FrameData == NULL) {
			KaiLog(_("Nie można pobrać ścieżki wideo"));
			return 0;
		}
		const FFMS_TrackTimeBase* TimeBase = FFMS_GetTimeBase(FrameData);
		if (TimeBase == NULL) {
			KaiLog(_("Nie można pobrać informacji o wideo"));
			return 0;
		}

		const FFMS_FrameInfo* CurFrameData;


		// build list of keyframes and timecodes
		for (int CurFrameNum = 0; CurFrameNum < videoprops->NumFrames; CurFrameNum++) {
			CurFrameData = FFMS_GetFrameInfo(FrameData, CurFrameNum);
			if (CurFrameData == NULL) {
				continue;
			}

			int Timestamp = ((CurFrameData->PTS * TimeBase->Num) / TimeBase->Den);
			// keyframe?
			if (CurFrameData->KeyFrame) { m_keyFrames.Add(Timestamp); }
			m_timecodes.push_back(Timestamp);

		}
		if (m_renderer && !m_renderer->videoControl->GetKeyFramesFileName().empty()) {
			OpenKeyframes(m_renderer->videoControl->GetKeyFramesFileName());
			m_renderer->videoControl->SetKeyFramesFileName(emptyString);
		}
	}
audio:

	if (audiotrack != -1) {
		m_audioSource = FFMS_CreateAudioSource(m_filename.utf8_str(), audiotrack, m_index, FFMS_DELAY_FIRST_VIDEO_TRACK, &m_errInfo);
		if (m_audioSource == NULL) {
			KaiLog(wxString::Format(_("Wystąpił błąd tworzenia źródła audio: %s"), m_errInfo.Buffer));
			return 0;
		}

		FFMS_ResampleOptions* resopts = FFMS_CreateResampleOptions(m_audioSource);
		resopts->ChannelLayout = FFMS_CH_FRONT_CENTER;
		resopts->SampleFormat = FFMS_FMT_S16;

		if (FFMS_SetOutputFormatA(m_audioSource, resopts, &m_errInfo)) {
			KaiLog(wxString::Format(_("Wystąpił błąd konwertowania audio: %s"), m_errInfo.Buffer));
			return 1;
		}
		FFMS_DestroyResampleOptions(resopts);
		const FFMS_AudioProperties* audioprops = FFMS_GetAudioProperties(m_audioSource);

		m_sampleRate = audioprops->SampleRate;
		m_delay = (Options.GetInt(AUDIO_DELAY) / 1000);
		m_numSamples = audioprops->NumSamples;
		m_bytesPerSample = 2;
		m_channels = 1;

		if (abs(m_delay) >= (m_sampleRate * m_numSamples * m_bytesPerSample)) {
			KaiLog(_("Nie można ustawić opóźnienia, przekracza czas trwania audio"));
			m_delay = 0;
		}
		m_audioLoadThread = new std::thread(AudioLoad, this, newIndex, audiotrack);
		m_audioLoadThread->detach();
	}
	return 1;
}



ProviderFFMS2::~ProviderFFMS2()
{
	if (m_thread) {
		SetEvent(m_eventKillSelf);
		WaitForSingleObject(m_thread, 20000);
		CloseHandle(m_thread);
		CloseHandle(m_eventStartPlayback);
		CloseHandle(m_eventKillSelf);
	}

	if (m_audioLoadThread) {
		m_stopLoadingAudio = true;
		WaitForSingleObject(m_eventAudioComplete, INFINITE);
		CloseHandle(m_eventAudioComplete);
	}
	m_keyFrames.Clear();
	m_timecodes.clear();

	if (m_videoSource) {
		FFMS_DestroyVideoSource(m_videoSource); m_videoSource = NULL;
	}

	if (m_discCache) { ClearDiskCache(); }
	else { ClearRAMCache(); }
	if (!m_stopLoadingAudio && m_discCache && m_diskCacheFilename.EndsWith(L".part")) {
		wxString discCacheNameWithGoodExt = m_diskCacheFilename;
		discCacheNameWithGoodExt.RemoveLast(5);
		_wrename(m_diskCacheFilename.wc_str(), discCacheNameWithGoodExt.wc_str());
	}
}



int FFMS_CC ProviderFFMS2::UpdateProgress(int64_t Current, int64_t Total, void* ICPrivate)
{
	ProgressSink* progress = (ProgressSink*)ICPrivate;
	progress->Progress(((double)Current / (double)Total) * 100);
	return progress->WasCancelled();
}

void ProviderFFMS2::AudioLoad(ProviderFFMS2* vf, bool newIndex, int audiotrack)
{
	if (vf->m_discCache) {
		vf->m_diskCacheFilename = emptyString;
		vf->m_diskCacheFilename << Options.pathfull << L"\\AudioCache\\" <<
			vf->m_filename.AfterLast(L'\\').BeforeLast(L'.') << L"_track" << audiotrack << L".w64";
		if (!vf->DiskCache(newIndex)) { goto done; }
	}
	else {
		if (!vf->RAMCache()) { goto done; }
	}
	vf->audioNotInitialized = false;
done:
	if (vf->m_audioSource) { FFMS_DestroyAudioSource(vf->m_audioSource); vf->m_audioSource = NULL; }
	vf->m_lockGetFrame = false;
	SetEvent(vf->m_eventAudioComplete);
	if (vf->m_audioLoadThread) { delete vf->m_audioLoadThread; vf->m_audioLoadThread = NULL; }

}

void ProviderFFMS2::GetFrame(int ttime, byte* buff)
{
	byte* cpy = (byte*)m_FFMS2frame->Data[0];
	memcpy(&buff[0], cpy, m_framePlane);

}

void ProviderFFMS2::GetFFMSFrame()
{
	wxCriticalSectionLocker lock(m_blockFrame);
	m_FFMS2frame = FFMS_GetFrame(m_videoSource, m_renderer->m_Frame, &m_errInfo);
}

void ProviderFFMS2::GetAudio(void* buf, int64_t start, int64_t count)
{

	if (count == 0 || !m_audioSource) return;
	if (start + count > m_numSamples) {
		int64_t oldcount = count;
		count = m_numSamples - start;
		if (count < 0) count = 0;

		// Fill beyond with zero

		short* temp = (short*)buf;
		for (int64_t i = count; i < oldcount; i++) {
			temp[i] = 0;
		}

	}
	wxCriticalSectionLocker lock(m_blockAudio);
	if (FFMS_GetAudio(m_audioSource, buf, start, count, &m_errInfo)) {
		KaiLogDebug(L"error audio" + wxString(m_errInfo.Buffer));
	}

}

void ProviderFFMS2::GetBuffer(void* buf, int64_t start, int64_t count, double volume)
{
	if (audioNotInitialized) { return; }

	if (start + count > m_numSamples) {
		int64_t oldcount = count;
		count = m_numSamples - start;
		if (count < 0) count = 0;


		short* temp = (short*)buf;
		for (int i = count; i < oldcount; i++) {
			temp[i] = 0;
		}
	}

	if (count) {
		if (m_discCache) {
			if (m_fp) {
				wxCriticalSectionLocker lock(m_blockAudio);
				_int64 pos = start * m_bytesPerSample;
				_fseeki64(m_fp, pos, SEEK_SET);
				fread(buf, 1, count * m_bytesPerSample, m_fp);
			}
		}
		else {
			if (!m_cache) { return; }
			char* tmpbuf = (char*)buf;
			int i = (start * m_bytesPerSample) >> 22;
			int blsize = (1 << 22);
			int offset = (start * m_bytesPerSample) & (blsize - 1);
			int64_t remaining = count * m_bytesPerSample;
			int readsize = remaining;

			while (remaining) {
				readsize = MIN(remaining, blsize - offset);

				memcpy(tmpbuf, (char*)(m_cache[i++] + offset), readsize);
				tmpbuf += readsize;
				offset = 0;
				remaining -= readsize;
			}
		}
		if (volume == 1.0) return;


		// Read raw samples
		short* buffer = (short*)buf;
		int value;

		// Modify
		for (int64_t i = 0; i < count; i++) {
			value = (int)(buffer[i] * volume + 0.5);
			if (value < -0x8000) value = -0x8000;
			if (value > 0x7FFF) value = 0x7FFF;
			buffer[i] = value;
		}

	}
}

bool ProviderFFMS2::RAMCache()
{
	//progress->Title(_("Zapisywanie do pamięci RAM"));
	m_audioProgress = 0;
	int64_t end = m_numSamples * m_bytesPerSample;

	int blsize = (1 << 22);
	m_blockNum = ((float)end / (float)blsize) + 1;
	m_cache = NULL;
	m_cache = new char* [m_blockNum];
	if (m_cache == NULL) { KaiMessageBox(_("Za mało pamięci RAM")); return false; }

	int64_t pos = (m_delay < 0) ? -(m_sampleRate * m_delay * m_bytesPerSample) : 0;
	int halfsize = (blsize / m_bytesPerSample);


	for (int i = 0; i < m_blockNum; i++)
	{
		if (i >= m_blockNum - 1) { blsize = end - pos; halfsize = (blsize / m_bytesPerSample); }
		m_cache[i] = new char[blsize];
		if (m_delay > 0 && i == 0) {
			int delaysize = m_sampleRate * m_delay * m_bytesPerSample;
			if (delaysize % 2 == 1) { delaysize++; }
			int halfdiff = halfsize - (delaysize / m_bytesPerSample);
			memset(m_cache[i], 0, delaysize);
			GetAudio(&m_cache[i][delaysize], 0, halfdiff);
			pos += halfdiff;
		}
		else {
			GetAudio(m_cache[i], pos, halfsize);
			pos += halfsize;
		}
		m_audioProgress = ((float)i / (float)(m_blockNum - 1));
		if (m_stopLoadingAudio) {
			m_blockNum = i + 1;
			break;
		}
	}
	if (m_delay < 0) { m_numSamples += (m_sampleRate * m_delay * m_bytesPerSample); }
	m_audioProgress = 1.f;
	return true;
}



void ProviderFFMS2::ClearRAMCache()
{
	if (!m_cache) { return; }
	for (int i = 0; i < m_blockNum; i++)
	{
		delete[] m_cache[i];
	}
	delete[] m_cache;
	m_cache = 0;
	m_blockNum = 0;
}

bool ProviderFFMS2::DiskCache(bool newIndex)
{
	m_audioProgress = 0;

	bool good = true;
	wxFileName discCacheFile;
	discCacheFile.Assign(m_diskCacheFilename);
	if (!discCacheFile.DirExists()) { wxMkdir(m_diskCacheFilename.BeforeLast(L'\\')); }
	bool fileExists = discCacheFile.FileExists();
	if (!newIndex && fileExists) {
		m_fp = _wfopen(m_diskCacheFilename.wc_str(), L"rb");
		if (m_fp)
			return true;
		else
			return false;
	}
	else {
		if (fileExists) {
			_wremove(m_diskCacheFilename.wc_str());
		}
		m_diskCacheFilename << L".part";
		m_fp = _wfopen(m_diskCacheFilename.wc_str(), L"w+b");
		if (!m_fp)
			return false;
	}
	int block = 332768;
	if (m_delay > 0) {

		int size = (m_sampleRate * m_delay * m_bytesPerSample);
		if (size % 2 == 1) { size++; }
		char* silence = new char[size];
		memset(silence, 0, size);
		fwrite(silence, 1, size, m_fp);
		delete[] silence;
	}
	try {
		char* data = new char[block * m_bytesPerSample];
		int all = (m_numSamples / block) + 1;
		//int64_t pos=0;
		int64_t pos = (m_delay < 0) ? -(m_sampleRate * m_delay * m_bytesPerSample) : 0;
		for (int i = 0; i < all; i++) {
			if (block + pos > m_numSamples) block = m_numSamples - pos;
			GetAudio(data, pos, block);
			fwrite(data, 1, block * m_bytesPerSample, m_fp);
			pos += block;
			m_audioProgress = ((float)pos / (float)(m_numSamples));
			if (m_stopLoadingAudio) break;
		}
		delete[] data;

		rewind(m_fp);
		if (m_delay < 0) { m_numSamples += (m_sampleRate * m_delay * m_bytesPerSample); }
	}
	catch (...) {
		good = false;
	}

	if (!good) { ClearDiskCache(); }
	else { m_audioProgress = 1.f; }
	return good;
}

void ProviderFFMS2::ClearDiskCache()
{
	if (m_fp) { fclose(m_fp); m_fp = NULL; }
}

void ProviderFFMS2::DeleteOldAudioCache()
{
	wxString path = Options.pathfull + L"\\AudioCache";
	size_t maxAudio = Options.GetInt(AUDIO_CACHE_FILES_LIMIT);
	if (maxAudio < 1)
		return;

	wxDir kat(path);
	wxArrayString audioCaches;
	if (kat.IsOpened()) {
		kat.GetAllFiles(path, &audioCaches, emptyString, wxDIR_FILES);
	}
	if (audioCaches.size() <= maxAudio) { return; }
	FILETIME ft;
	SYSTEMTIME st;
	std::map<unsigned __int64, int> dates;
	unsigned __int64 datetime;
	for (size_t i = 0; i < audioCaches.size(); i++) {
		HANDLE ffile = CreateFile(audioCaches[i].wc_str(), GENERIC_READ, FILE_SHARE_DELETE, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
		if (ffile != INVALID_HANDLE_VALUE) {
			GetFileTime(ffile, 0, &ft, 0);
			CloseHandle(ffile);
			FileTimeToSystemTime(&ft, &st);
			if (st.wYear > 3000) { st.wYear = 3000; }
			datetime = (st.wYear * 980294400000) + (st.wMonth * 2678400000) + (st.wDay * 86400000) + (st.wHour * 3600000) + (st.wMinute * 60000) + (st.wSecond * 1000) + st.wMilliseconds;
			dates[datetime] = i;
		}

	}
	int count = 0;
	int diff = audioCaches.size() - maxAudio;
	for (auto cur = dates.begin(); cur != dates.end(); cur++) {
		if (count >= diff) { break; }
		int isgood = _wremove(audioCaches[cur->second].wchar_str());
		count++;
	}

}

void ProviderFFMS2::GetFrameBuffer(byte** buffer)
{
	if (m_renderer->m_Frame != m_lastFrame) {
		GetFFMSFrame();
		m_lastFrame = m_renderer->m_Frame;
	}
	if (!m_FFMS2frame) {
		return;
	}
	memcpy(*buffer, m_FFMS2frame->Data[0], m_framePlane);
}

wxString ProviderFFMS2::ColorMatrixDescription(int cs, int cr) {
	// Assuming TV for unspecified
	wxString str = cr == FFMS_CR_JPEG ? L"PC" : L"TV";

	switch (cs) {
	case FFMS_CS_RGB:
		return _("Brak");
	case FFMS_CS_BT709:
		return str + L".709";
	case FFMS_CS_FCC:
		return str + L".FCC";
	case FFMS_CS_BT470BG:
	case FFMS_CS_SMPTE170M:
		return str + L".601";
	case FFMS_CS_SMPTE240M:
		return str + L".240M";
	default:
		return _("Brak");
	}
}

void ProviderFFMS2::SetColorSpace(const wxString& matrix)
{
	wxCriticalSectionLocker lock(m_blockFrame);
	if (matrix == m_colorSpace) return;
	//lockGetFrame = true;
	if (matrix == m_realColorSpace || (matrix != L"TV.601" && matrix != L"TV.709"))
		FFMS_SetInputFormatV(m_videoSource, m_CS, m_CR, FFMS_GetPixFmt(""), nullptr);
	else if (matrix == L"TV.601")
		FFMS_SetInputFormatV(m_videoSource, FFMS_CS_BT470BG, m_CR, FFMS_GetPixFmt(""), nullptr);
	else {
		//lockGetFrame = false;
		return;
	}
	//lockGetFrame = false;
	m_colorSpace = matrix;

}

bool ProviderFFMS2::HasVideo()
{
	return m_videoSource != NULL;
}
