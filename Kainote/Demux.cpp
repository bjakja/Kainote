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

#include "Demux.h"
#include "LogHandler.h"
#include "KaiMessageBox.h"
#include "StyleListbox.h"
#include "SubsGrid.h"
#include "ProgressDialog.h"


Demux::~Demux()
{
	if (indexer)
		Close();
}

bool Demux::Open(const wxString& filename)
{
	FFMS_Init(0, 1);
	char errmsg[1024];
	FFMS_ErrorInfo errInfo;
	errInfo.Buffer = errmsg;
	errInfo.BufferSize = sizeof(errmsg);
	errInfo.ErrorType = FFMS_ERROR_SUCCESS;
	errInfo.SubType = FFMS_ERROR_SUCCESS;

	indexer = FFMS_CreateIndexer(filename.utf8_str(), &errInfo);
	if (!indexer) {
		KaiLog(wxString::Format(_("Wystąpił błąd indeksowania: %s"), errInfo.Buffer)); return false;
	}
	return true;
}

void Demux::Close()
{
	if (attachments.size()) {
		for (auto attachment : attachments) {
			FFMS_FreeAttachment(&attachment);
		}
		attachments.clear();
	}

	if (indexer) {
		FFMS_CancelIndexing(indexer);
		indexer = nullptr;
	}
}

bool Demux::GetSubtitles(SubsGrid* target)
{
	int numTracks = FFMS_GetNumTracksI(indexer);
	int trackToRead = -1;
	wxArrayString trackNameList;
	wxArrayInt trackList;
	for (int i = 0; i < numTracks; i++){
		if (FFMS_GetTrackTypeI(indexer, i) == FFMS_TYPE_SUBTITLE) {
			wxString codecName = wxString(FFMS_GetSubtitleFormat(indexer, i), wxConvUTF8);
			wxString trackName = wxString(FFMS_GetTrackName(indexer, i), wxConvUTF8);
			wxString trackLanguage = wxString(FFMS_GetTrackLanguage(indexer, i), wxConvUTF8);
			if (codecName == L"ass" || codecName == L"ssa" || codecName == L"subrip" || 
				codecName == L"srt" || codecName == L"text") {
				trackList.Add(i);
				trackNameList.Add(wxString::Format(L"%i ", i) + trackName + L" (" + trackLanguage + L", " + codecName + L")");
			}
		}
	}
	// No tracks found
	if (trackList.Count() == 0) {
		Close();
		KaiMessageBox(_("Plik nie ma żadnej ścieżki z napisami."));
		return false;
	}

	// Only one track found
	else if (trackList.Count() == 1) {
		trackToRead = trackList[0];
	}

	// Pick a track
	else {
		KaiListBox tracks(target->GetParent(), trackNameList, _("Wybierz ścieżkę napisów"), true);
		//int choice = wxGetSingleChoiceIndex(_("Wybierz ścieżkę do wczytania:"), _("Znaleziono kilka ścieżek z napisami"), tracksNames);
		if (tracks.ShowModal() != wxID_OK) {
			Close();
			return false;
		}
		trackToRead = trackList[tracks.GetIntSelection()];
	}

	// Picked track
	if (trackToRead != -1) {
		// to force saving to show choose name dialog
		target->originalFormat = -1;
		wxString codecName = wxString(FFMS_GetSubtitleFormat(indexer, trackToRead), wxConvUTF8);
		if (codecName == L"ass")
			codecType = 0;
		else if (codecName == L"ssa")
			codecType = 1;
		else
			codecType = 2;

		progress = new ProgressSink(target->GetParent(), _("Odczyt napisów z pliku Matroska."));
		progress->SetAndRunTask([=]() {
			FFMS_GetSubtitles(indexer, trackToRead, GetSubtitles, (void*)this);
			if (progress->WasCancelled()) {
				subtitleList.clear();
				return 0;
			}
			target->Clearing();
			target->file = new SubsFile(&target->GetMutex());

			// Read private data if it's ASS/SSA
			if (codecType < 2) {
				// Read raw data
				const char* privData = FFMS_GetSubtitleExtradata(indexer, trackToRead);
				wxString privString(privData, wxConvUTF8);

				// Load into file
				int type = 0;
				wxStringTokenizer token(privString, L"\r\n", wxTOKEN_STRTOK);
				while (token.HasMoreTokens()) {
					wxString next = token.GetNextToken();
					if (next.StartsWith(L"Style:")) {
						//format 2 for SSA
						target->AddStyle(new Styles(next, codecType == 1? 2 : 1));
						type = 1;
					}
					else if (next.StartsWith(L"Comment:")) {
						target->AddLine(new Dialogue(next));
						type = 2;
					}
					else if (type == 0 && !next.StartsWith(L";") && !next.StartsWith(L"[") && !next.StartsWith(L"Format:")) {
						target->AddSInfo(next);
					}
				}


			}


			for (unsigned int i = 0; i < subtitleList.size(); i++) {
				target->AddLine(new Dialogue(subtitleList[i]));
			}
			const wxString& matrix = target->GetSInfo(L"YCbCr Matrix");
			if ((matrix == emptyString || matrix == L"None") && codecType < 1) 
				target->AddSInfo(L"YCbCr Matrix", L"TV.601");

			target->file->EndLoad(OPEN_SUBTITLES, 0, true);
			subtitleList.clear();

			return 1;
		});
		progress->ShowDialog();
		bool isgood = ((int)progress->Wait() == 1);
		delete progress; 
		progress = nullptr;
		return isgood;
	}
	return false;
}

void Demux::GetFontList(wxArrayString* list)
{
	int numTracks = FFMS_GetNumTracksI(indexer);
	for (size_t i = 0; i < numTracks; i++){
		if (FFMS_GetTrackTypeI(indexer, i) == FFMS_TYPE_ATTACHMENT) {
			FFMS_Attachment* attachment = FFMS_GetAttachment(indexer, i);
			wxString mimetype(attachment->Mimetype, wxConvUTF8);
			if (mimetype == L"font/ttf" || mimetype == L"font/otf" ||
				mimetype == L"application/x-truetype-font" || mimetype == L"application/vnd.ms-opentype") {
				list->Add(wxString(attachment->Filename, wxConvUTF8));
				attachments.push_back(attachment);
			}
			else {
				FFMS_FreeAttachment(&attachment);
			}
		}
	}
}

bool Demux::SaveFont(int i, const wxString& path, wxZipOutputStream* zip)
{
	if (i >= attachments.size())
		return false;

	FFMS_Attachment* attachment = attachments[i];
	bool isgood = true;

	if (zip) {
		wxString fn = path.AfterLast(L'\\');
		try {
			isgood = zip->PutNextEntry(fn);
			zip->Write((void*)attachment->Data, attachment->DataSize);
		}
		catch (...)
		{
			isgood = false;
		}
	}
	else
	{
		wxFile file;
		file.Create(path, true, wxS_DEFAULT);
		if (file.IsOpened()) {
			file.Write((void*)attachment->Data, attachment->DataSize);
			file.Close();
		}
		else { isgood = false; }

	}
	return isgood;
}

int __stdcall Demux::GetSubtitles(long long Start, long long Duration, long long Total, const char* Line, void* ICPrivate)
{
	Demux* demux = (Demux*)ICPrivate;
	wxString blockString(Line, wxConvUTF8);

	// Get start and end times
	SubsTime subStart, subEnd;
	int startTime = Start;
	int endTime = startTime + Duration;
	if (demux->codecType < 2) {
		startTime += 5;
		endTime += 5;
		startTime = ZEROIT(startTime);
		endTime = ZEROIT(endTime);
	}
	subStart.NewTime(startTime);
	subEnd.NewTime(endTime);
	//wxLogMessage(subStart.GetASSFormated() + "-" + subEnd.GetASSFormated() + ": " + blockString);

	// Process SSA/ASS
	if (demux->codecType < 2) {
		// Get order number
		int pos = blockString.Find(L",");
		wxString orderString = blockString.Left(pos);
		blockString = blockString.Mid(pos + 1);

		// Get layer number
		pos = blockString.Find(L",");
		long int layer = 0;
		if (pos) {
			wxString layerString = blockString.Left(pos);
			layerString.ToLong(&layer);
			blockString = blockString.Mid(pos + 1);
		}
		if (blockString == emptyString) { blockString << L"Default,,0000,0000,0000,,"; }
		// Assemble final
		if (!blockString.StartsWith(L",")) { blockString.Prepend(L","); }
		blockString = wxString::Format(L"Dialogue: %li,", layer) + subStart.raw() +
			L"," + subEnd.raw() + blockString;

	}

	// Process SRT
	else {
		blockString = subStart.raw(SRT) + L" --> " + subEnd.raw(SRT) + L"\r\n" + blockString;
	}

	// Insert into vector
	demux->subtitleList.push_back(blockString);


	int prog = ((double(Start)) / double(Total)) * 100;
	demux->progress->Progress(prog);
	return demux->progress->WasCancelled();
}
