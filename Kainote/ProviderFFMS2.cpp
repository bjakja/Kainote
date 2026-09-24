//  Copyright (c) 2021 - 2026, Marcin Drob

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



#include "ProviderFFMS2.h"
#include "RendererFFMS2.h"
#include "LogHandler.h"
#include "VideoBox.h"
#include "kainoteApp.h"
#include "ListControls.h"
#include "Stylelistbox.h"
#include "SubsGrid.h"
#include <wx/dir.h>
#include <wx/filename.h>
#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <cmath>
#include <limits>
#include <vector>
#include "UtilsWindows.h"
#include "Provider.h"

namespace
{
	void CopyBgraFrameToBuffer(const FFMS_Frame* frame, unsigned char* dst, int width, int height)
	{
		if (!frame || !dst || width <= 0 || height <= 0 || !frame->Data[0])
			return;

		const int dstPitch = width * 4;
		const int srcPitch = frame->Linesize[0];
		const unsigned char* src = frame->Data[0];
		if (srcPitch < 0)
			src += static_cast<size_t>(height - 1) * static_cast<size_t>(-srcPitch);

		const int srcRowBytes = std::abs(srcPitch);
		const int copyBytes = std::min(dstPitch, srcRowBytes);
		for (int y = 0; y < height; ++y) {
			const unsigned char* row = src + static_cast<ptrdiff_t>(y) * srcPitch;
			memcpy(dst + static_cast<size_t>(y) * dstPitch, row, copyBytes);
			if (copyBytes < dstPitch)
				memset(dst + static_cast<size_t>(y) * dstPitch + copyBytes, 0, dstPitch - copyBytes);
		}
	}
}

ProviderFFMS2::ProviderFFMS2(const wxString& filename, RendererFFMS2* renderer, 
	wxWindow* progressSinkWindow, bool* _success)
	: Provider(filename, renderer)
	, m_eventAudioComplete(CreateEvent(0, FALSE, FALSE, 0))
{
	if (!Options.AudioOpts && !Options.LoadAudioOpts()) 
	{ 
		KaiLogSilent(_("Cannot load audio configuration"));
	}

	m_discCache = !Options.GetBool(AUDIO_RAM_CACHE);

	m_success = false;
	progress = new ProgressSink(progressSinkWindow, _("Indexing video"));

	if (renderer) {
		unsigned int threadid = 0;
		m_thread = (HANDLE)_beginthreadex(0, 0, FFMS2Proc, this, 0, &threadid);
		//CreateThread( nullptr, 0,  (LPTHREAD_START_ROUTINE)FFMS2Proc, this, 0, 0);
		SetThreadName(threadid, "VideoThread");
		progress->ShowDialog();
		WaitForSingleObject(m_eventComplete, INFINITE);
		ResetEvent(m_eventComplete);
		*_success = m_success;
	}
	else {
		progress->SetAndRunTask([&]() {return Init(); });
		progress->ShowDialog();
		*_success = ((long long)progress->Wait() == 1);
	}
	SAFE_DELETE(progress);
	if (m_index) { FFMS_DestroyIndex(m_index); }

}

bool ProviderFFMS2::FetchPlaybackFrame(int frame, unsigned char* buffer)
{
	if (CopyFrame(frame, buffer, true))
		return true;
	KaiLogDebug(wxString::Format(_("Cannot get frame %i: %s"),
		frame, wxString::FromUTF8(m_errInfo.Buffer)));
	return false;
}

unsigned int __stdcall ProviderFFMS2::FFMS2Proc(void* cls)
{
	((ProviderFFMS2*)cls)->Processing();
	return 0;
}

void ProviderFFMS2::Processing()
{
	m_success = (Init() == 1);

	progress->EndModal();

	if (!m_success || m_width <= 0 || m_height <= 0) {
		SetEvent(m_eventComplete);
		return;
	}
	const long long framePlane = static_cast<long long>(m_height) * static_cast<long long>(m_width) * 4;
	if (framePlane > std::numeric_limits<int>::max()) {
		KaiLog(_("Video frame size is too large"));
		m_success = false;
		SetEvent(m_eventComplete);
		return;
	}
	m_framePlane = static_cast<int>(framePlane);
	SetEvent(m_eventComplete);
#ifdef _WIN32
	RunPlaybackThread();
#endif
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
		KaiLogDebug(wxString::Format(_("Indexing error occurred: %s"), wxString::FromUTF8(m_errInfo.Buffer))); return 0;
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
				description = _("Untitled");

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
			KaiListBox tracks1(KainoteFrame::Get(), tracks, _("Choose the track"), true);
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

	wxString sep = wxFileName::GetPathSeparator();
	wxString baseName = wxFileName(m_filename).GetName();
	m_indexPath = Options.pathfull + sep + L"Indices" + sep + baseName +
		wxString::Format(L"_%i.ffindex", audiotrack);

	if (wxFileExists(m_indexPath)) {
		m_index = FFMS_ReadIndex(m_indexPath.utf8_str(), &m_errInfo);
		if (!m_index) {/*do nothing to skip*/ }
		else if (FFMS_IndexBelongsToFile(m_index, m_filename.utf8_str(), &m_errInfo))
		{
			FFMS_DestroyIndex(m_index);
			m_index = nullptr;
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
		if (m_index == nullptr) {
			if (wxString::FromUTF8(m_errInfo.Buffer).StartsWith(L"Cancelled")) {
				//No need spam user that he clicked cancel button
				//KaiLog(_("Indeksowanie anulowane przez użytkownika"));
			}
			else {
				KaiLog(wxString::Format(_("Indexing error occurred: %s"), wxString::FromUTF8(m_errInfo.Buffer)));
			}
			//FFMS_CancelIndexing(Indexer);
			return 0;
		}
		wxFileName indexFile(m_indexPath);
		if (!indexFile.DirExists())
		{
			indexFile.Mkdir(wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
		}
		if (FFMS_WriteIndex(m_indexPath.utf8_str(), m_index, &m_errInfo))
		{
			KaiLogDebug(wxString::Format(_("Cannot save index, error %s occurred"), wxString::FromUTF8(m_errInfo.Buffer)));
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

		if (m_videoSource == nullptr) {
			if (audiotrack == -1) {
				KaiLog(_("Cannot create VideoSource."));
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
		pixfmt[0] = FFMS_GetPixFmt("bgra");
		pixfmt[1] = -1;

		if (FFMS_SetOutputFormatV2(m_videoSource, pixfmt, m_width, m_height, FFMS_RESIZER_BILINEAR, &m_errInfo)) {
			KaiLog(_("Cannot convert video to RGBA"));
			return 0;
		}

		if (m_renderer) {
			SubsGrid* grid = ((TabPanel*)m_renderer->videoControl->GetParent())->grid;
			const wxString& colormatrix = grid->file->GetSInfo(L"YCbCr Matrix");
			bool changeMatrix = false;
			if (m_CS == FFMS_CS_UNSPECIFIED) {
				m_CS = m_width > 1024 || m_height >= 600 ? FFMS_CS_BT709 : FFMS_CS_BT470BG;
			}
			m_colorSpace = m_realColorSpace = ColorMatrixDescription(m_CS, m_CR);
			if (m_CS == FFMS_CS_BT709 && colormatrix == L"TV.709") {
				if (FFMS_SetInputFormatV(m_videoSource, FFMS_CS_BT709, m_CR, FFMS_GetPixFmt(""), &m_errInfo)) {
					KaiLog(_("Cannot change YCbCr matrix"));
				}
			}
			if (colormatrix == L"TV.601") {
				m_colorSpace = ColorMatrixDescription(FFMS_CS_BT470BG, m_CR);
				if (FFMS_SetInputFormatV(m_videoSource, FFMS_CS_BT470BG, m_CR, FFMS_GetPixFmt(""), &m_errInfo)) {
					KaiLog(_("Cannot change YCbCr matrix"));
				}
			}
			else if (colormatrix == L"TV.709") {
				m_colorSpace = ColorMatrixDescription(FFMS_CS_BT709, m_CR);
			}
		}

		FFMS_Track* FrameData = FFMS_GetTrackFromVideo(m_videoSource);
		if (FrameData == nullptr) {
			KaiLog(_("You cannot load the video track"));
			return 0;
		}
		const FFMS_TrackTimeBase* TimeBase = FFMS_GetTimeBase(FrameData);
		if (TimeBase == nullptr) {
			KaiLog(_("You cannot get information about the video"));
			return 0;
		}

		const FFMS_FrameInfo* CurFrameData;


		// build list of keyframes and timecodes
		std::vector<int> timecodes;
		std::vector<int> keyframes;
		for (int CurFrameNum = 0; CurFrameNum < videoprops->NumFrames; CurFrameNum++) {
			CurFrameData = FFMS_GetFrameInfo(FrameData, CurFrameNum);
			if (CurFrameData == nullptr) {
				continue;
			}

			int Timestamp = ((CurFrameData->PTS * TimeBase->Num) / TimeBase->Den);
			// keyframe?
			if (CurFrameData->KeyFrame) { keyframes.push_back(Timestamp); }
			timecodes.push_back(Timestamp);

		}
		m_timebase = Timebase::FromTimecodes(std::move(timecodes), m_FPS);
		m_timebase.SetKeyframes(std::move(keyframes));
	}
audio:

	if (audiotrack != -1) {
		m_audioSource = FFMS_CreateAudioSource(m_filename.utf8_str(), audiotrack, m_index, FFMS_DELAY_FIRST_VIDEO_TRACK, &m_errInfo);
		if (m_audioSource == nullptr) {
			KaiLog(wxString::Format(_("An error occurred when creating audio source: %s"), wxString::FromUTF8(m_errInfo.Buffer)));
			return 0;
		}

		// stereo sources play in stereo; the waveform and spectrum read a downmix
		bool stereo = FFMS_GetAudioProperties(m_audioSource)->Channels > 1;
		FFMS_ResampleOptions* resopts = FFMS_CreateResampleOptions(m_audioSource);
		resopts->ChannelLayout = stereo ? (FFMS_CH_FRONT_LEFT | FFMS_CH_FRONT_RIGHT) : FFMS_CH_FRONT_CENTER;
		resopts->SampleFormat = FFMS_FMT_S16;

		if (FFMS_SetOutputFormatA(m_audioSource, resopts, &m_errInfo)) {
			KaiLog(wxString::Format(_("An error occurred when converting audio: %s"), wxString::FromUTF8(m_errInfo.Buffer)));
			FFMS_DestroyResampleOptions(resopts);
			FFMS_DestroyAudioSource(m_audioSource);
			m_audioSource = nullptr;
			return 1;
		}
		FFMS_DestroyResampleOptions(resopts);
		const FFMS_AudioProperties* audioprops = FFMS_GetAudioProperties(m_audioSource);

		m_sampleRate = audioprops->SampleRate;
		m_delayFrames = llround(m_sampleRate * (Options.GetInt(AUDIO_DELAY) / 1000.0));
		m_numSamples = audioprops->NumSamples;
		m_bytesPerSample = 2;
		m_channels = stereo ? 2 : 1;

		if (llabs(m_delayFrames) >= m_numSamples) {
			KaiLog(_("Delay failed, it's longer than audio duration time"));
			m_delayFrames = 0;
		}
		m_audioLoadThread = new std::thread(AudioLoad, this, newIndex, audiotrack);
	}
	return 1;
}



ProviderFFMS2::~ProviderFFMS2()
{
	if (m_thread) {
		SetEvent(m_eventKillSelf);
		WaitForSingleObject(m_thread, INFINITE);
		CloseHandle(m_thread);
		m_thread = nullptr;
	}

	if (m_audioLoadThread) {
		m_stopLoadingAudio = true;
		if (m_audioLoadThread->joinable())
			m_audioLoadThread->join();
		delete m_audioLoadThread;
		m_audioLoadThread = nullptr;
	}
	if (m_audioSource) {
		FFMS_DestroyAudioSource(m_audioSource);
		m_audioSource = nullptr;
	}
	if (m_eventAudioComplete) {
		CloseHandle(m_eventAudioComplete);
		m_eventAudioComplete = nullptr;
	}
	m_timebase = Timebase();

	if (m_videoSource) {
		FFMS_DestroyVideoSource(m_videoSource); m_videoSource = nullptr;
	}

	if (m_discCache) { ClearDiskCache(); }
	else { ClearRAMCache(); }
	// m_stopLoadingAudio is set above whenever audio was loaded, so it cannot tell
	if (m_discCache && m_diskCacheFilename.EndsWith(L".part")) {
		if (m_diskCacheComplete) {
			wxString discCacheNameWithGoodExt = m_diskCacheFilename;
			discCacheNameWithGoodExt.RemoveLast(5);
			_wrename(m_diskCacheFilename.wc_str(), discCacheNameWithGoodExt.wc_str());
		}
		else {
			_wremove(m_diskCacheFilename.wc_str());
		}
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
		wxString sep = wxFileName::GetPathSeparator();
		wxString baseName = wxFileName(vf->m_filename).GetName();
		// a cache made with other channels or another delay holds other data
		vf->m_diskCacheFilename << Options.pathfull << sep << L"AudioCache" << sep <<
			baseName << L"_track" << audiotrack << L"_" << vf->m_channels << L"ch_" <<
			vf->m_delayFrames << L".w64";
		if (!vf->DiskCache(newIndex)) { goto done; }
	}
	else {
		if (!vf->RAMCache()) { goto done; }
	}
	vf->audioNotInitialized = false;
done:
	if (vf->m_audioSource) { FFMS_DestroyAudioSource(vf->m_audioSource); vf->m_audioSource = nullptr; }
	vf->m_lockGetFrame = false;
	SetEvent(vf->m_eventAudioComplete);
	if (!vf->audioNotInitialized)
		vf->BuildPeaks(vf->m_stopLoadingAudio);
}

void ProviderFFMS2::GetFrame(int frame, unsigned char* buff)
{
	wxCriticalSectionLocker lock(m_blockFrame);
	const FFMS_Frame *ffmsframe = FFMS_GetFrame(m_videoSource, frame, &m_errInfo);
	CopyBgraFrameToBuffer(ffmsframe, buff, m_width, m_height);
	m_refreshFrame = true;
}

bool ProviderFFMS2::CopyFrame(int frame, unsigned char* buffer, bool forceFetch)
{
	//FFMS owns the frame memory and FFMS_SetInputFormatV hands it back
	//reallocated when the colour matrix changes, so the fetch and the read
	//have to share one lock. Copying after the lock was released is what
	//crashed playback on a colorspace switch (issue #39).
	wxCriticalSectionLocker lock(m_blockFrame);
	if (forceFetch || !m_FFMS2frame || frame != m_lastFrame || m_refreshFrame) {
		m_FFMS2frame = FFMS_GetFrame(m_videoSource, frame, &m_errInfo);
		m_lastFrame = frame;
		m_refreshFrame = false;
	}
	if (!m_FFMS2frame) {
		return false;
	}
	CopyBgraFrameToBuffer(m_FFMS2frame, buffer, m_width, m_height);
	return true;
}

void ProviderFFMS2::GetAudio(void* buf, long long start, long long count)
{

	if (count == 0 || !m_audioSource) return;
	if (start + count > m_numSamples) {
		long long oldcount = count;
		count = m_numSamples - start;
		if (count < 0) count = 0;

		// Fill beyond with zero

		short* temp = (short*)buf;
		for (long long i = count; i < oldcount; i++) {
			temp[i] = 0;
		}

	}
	wxCriticalSectionLocker lock(m_blockAudio);
	if (FFMS_GetAudio(m_audioSource, buf, start, count, &m_errInfo)) {
		KaiLogDebug(L"error audio" + wxString::FromUTF8(m_errInfo.Buffer));
	}

}

void ProviderFFMS2::ReadCache(void* buf, long long start, long long count)
{
	const int frameBytes = FrameBytes();
	if (start + count > m_numSamples) {
		long long valid = std::max(0LL, m_numSamples - start);
		memset((char*)buf + valid * frameBytes, 0, (count - valid) * frameBytes);
		count = valid;
	}
	if (count <= 0)
		return;

	if (m_discCache) {
		if (m_fp) {
			wxCriticalSectionLocker lock(m_blockAudio);
			_fseeki64(m_fp, start * frameBytes, SEEK_SET);
			fread(buf, 1, count * frameBytes, m_fp);
		}
		return;
	}
	if (!m_cache)
		return;
	char* tmpbuf = (char*)buf;
	const int blsize = (1 << 22);
	long long byte = start * frameBytes;
	int i = (int)(byte >> 22);
	int offset = (int)(byte & (blsize - 1));
	long long remaining = count * frameBytes;
	while (remaining) {
		int readsize = (int)MIN(remaining, blsize - offset);
		memcpy(tmpbuf, m_cache[i++] + offset, readsize);
		tmpbuf += readsize;
		offset = 0;
		remaining -= readsize;
	}
}

static void ApplyVolume(short* samples, long long count, double volume)
{
	if (volume == 1.0)
		return;
	for (long long i = 0; i < count; i++) {
		int value = (int)(samples[i] * volume + 0.5);
		if (value < -0x8000) value = -0x8000;
		if (value > 0x7FFF) value = 0x7FFF;
		samples[i] = value;
	}
}

void ProviderFFMS2::GetPlaybackBuffer(void* buf, long long start, long long count, double volume)
{
	if (audioNotInitialized) { return; }
	ReadCache(buf, start, count);
	ApplyVolume((short*)buf, count * m_channels, volume);
}

void ProviderFFMS2::GetBuffer(void* buf, long long start, long long count, double volume)
{
	if (audioNotInitialized) { return; }
	if (m_channels == 1) {
		GetPlaybackBuffer(buf, start, count, volume);
		return;
	}
	std::vector<short> frames((size_t)std::max(0LL, count) * m_channels);
	ReadCache(frames.data(), start, count);
	short* mono = (short*)buf;
	for (long long i = 0; i < count; i++) {
		int sum = 0;
		for (int c = 0; c < m_channels; c++)
			sum += frames[i * m_channels + c];
		mono[i] = (short)(sum / m_channels);
	}
	ApplyVolume(mono, count, volume);
}

bool ProviderFFMS2::RAMCache()
{
	m_audioProgress = 0;
	const int frameBytes = FrameBytes();
	// a positive delay starts with silence, a negative one skips the start
	long long silence = std::max(0LL, m_delayFrames);
	long long sourceFrame = std::max(0LL, -m_delayFrames);
	m_numSamples -= sourceFrame;
	long long end = m_numSamples * frameBytes;

	const long long blsize = (1 << 22);
	m_blockNum = (int)(end / blsize) + 1;
	m_cache = new char* [m_blockNum];

	long long written = 0;
	for (int i = 0; i < m_blockNum; i++)
	{
		long long size = std::min(blsize, end - written);
		m_cache[i] = new char[std::max(1LL, size)];
		long long frames = size / frameBytes;
		long long silentFrames = std::clamp(silence - written / frameBytes, 0LL, frames);
		memset(m_cache[i], 0, silentFrames * frameBytes);
		if (frames > silentFrames) {
			GetAudio(m_cache[i] + silentFrames * frameBytes, sourceFrame, frames - silentFrames);
			sourceFrame += frames - silentFrames;
		}
		written += size;
		m_audioProgress = (m_blockNum > 1) ? ((float)i / (float)(m_blockNum - 1)) : 1.f;
		if (m_stopLoadingAudio) {
			m_blockNum = i + 1;
			break;
		}
	}
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
	m_numSamples -= std::max(0LL, -m_delayFrames);

	bool good = true;
	wxFileName discCacheFile;
	discCacheFile.Assign(m_diskCacheFilename);
	if (!discCacheFile.DirExists()) { discCacheFile.Mkdir(wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL); }
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
	const int frameBytes = FrameBytes();
	long long block = 332768;
	long long sourceFrame = std::max(0LL, -m_delayFrames);
	// the frames read are counted without the skipped start
	long long sourceEnd = m_numSamples + sourceFrame;
	try {
		if (m_delayFrames > 0) {
			std::vector<char> silence((size_t)(m_delayFrames * frameBytes));
			fwrite(silence.data(), 1, silence.size(), m_fp);
		}
		std::vector<char> data((size_t)(block * frameBytes));
		while (sourceFrame < sourceEnd) {
			long long frames = std::min(block, sourceEnd - sourceFrame);
			GetAudio(data.data(), sourceFrame, frames);
			fwrite(data.data(), 1, frames * frameBytes, m_fp);
			sourceFrame += frames;
			m_audioProgress = ((float)sourceFrame / (float)sourceEnd);
			if (m_stopLoadingAudio) break;
		}
		m_diskCacheComplete = sourceFrame >= sourceEnd;
		rewind(m_fp);
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
	if (m_fp) { fclose(m_fp); m_fp = nullptr; }
}

void ProviderFFMS2::DeleteOldAudioCache()
{
	wxString path = Options.pathfull + wxFileName::GetPathSeparator() + L"AudioCache";
	size_t maxAudio = Options.GetInt(AUDIO_CACHE_FILES_LIMIT);
	if (maxAudio < 1)
		return;

	wxDir kat(path);
	wxArrayString audioCaches;
	if (kat.IsOpened()) {
		kat.GetAllFiles(path, &audioCaches, emptyString, wxDIR_FILES);
	}
	if (audioCaches.size() <= maxAudio) { return; }
	std::multimap<unsigned __int64, size_t> dates;
	for (size_t i = 0; i < audioCaches.size(); i++) {
		HANDLE ffile = CreateFile(audioCaches[i].wc_str(), GENERIC_READ, FILE_SHARE_DELETE, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
		if (ffile != INVALID_HANDLE_VALUE) {
			FILETIME ft{};
			SYSTEMTIME st{};
			const bool haveTimestamp = GetFileTime(ffile, 0, &ft, 0) &&
				FileTimeToSystemTime(&ft, &st);
			CloseHandle(ffile);
			if (!haveTimestamp)
				continue;
			if (st.wYear > 3000) { st.wYear = 3000; }
			const unsigned __int64 datetime = (st.wYear * 980294400000) + (st.wMonth * 2678400000) + (st.wDay * 86400000) + (st.wHour * 3600000) + (st.wMinute * 60000) + (st.wSecond * 1000) + st.wMilliseconds;
			dates.emplace(datetime, i);
		}

	}
	size_t count = 0;
	const size_t diff = audioCaches.size() - maxAudio;
	for (auto cur = dates.begin(); cur != dates.end(); cur++) {
		if (count >= diff) { break; }
		if (_wremove(audioCaches[cur->second].wchar_str()) == 0)
			count++;
	}

}

void ProviderFFMS2::PrefetchFrame(int frame)
{
	wxCriticalSectionLocker lock(m_blockFrame);
	if (!m_FFMS2frame || frame != m_lastFrame || m_refreshFrame) {
		m_FFMS2frame = FFMS_GetFrame(m_videoSource, frame, &m_errInfo);
		m_lastFrame = frame;
		m_refreshFrame = false;
	}
}

void ProviderFFMS2::GetFrameBuffer(int frame, unsigned char** buffer)
{
	CopyFrame(frame, *buffer, false);
}

wxString ProviderFFMS2::ColorMatrixDescription(int cs, int cr) {
	// Assuming TV for unspecified
	wxString str = cr == FFMS_CR_JPEG ? L"PC" : L"TV";

	switch (cs) {
	case FFMS_CS_RGB:
		return _("None");
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
		return _("None");
	}
}

void ProviderFFMS2::SetColorSpace(const wxString& matrix)
{
	int failed = 0;
	{
		wxCriticalSectionLocker lock(m_blockFrame);
		if (matrix == m_colorSpace) return;
		//lockGetFrame = true;
		if (matrix == m_realColorSpace || (matrix != L"TV.601" && matrix != L"TV.709"))
			failed = FFMS_SetInputFormatV(m_videoSource, m_CS, m_CR, FFMS_GetPixFmt(""), &m_errInfo);
		else if (matrix == L"TV.601")
			failed = FFMS_SetInputFormatV(m_videoSource, FFMS_CS_BT470BG, m_CR, FFMS_GetPixFmt(""), &m_errInfo);
		else {
			//lockGetFrame = false;
			return;
		}
		//lockGetFrame = false;
		//the cached frame was produced by the old format and does not survive
		//the reconfiguration above, a failed reconfiguration can even free it
		//half way through, so force the next read to fetch again
		m_FFMS2frame = nullptr;
		m_refreshFrame = true;
		//keep the old matrix when nothing changed or the next call asking for it
		//would be dropped as a no-op and the video would stay unconverted
		if (!failed)
			m_colorSpace = matrix;
	}
	if (failed)
		KaiLog(_("Cannot change YCbCr matrix"));

}

bool ProviderFFMS2::HasVideo()
{
	return m_videoSource != nullptr;
}
