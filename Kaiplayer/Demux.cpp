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

	FFMS_Indexer* Indexer = FFMS_CreateIndexer(filename.utf8_str(), &errInfo);
	if (!Indexer) {
		KaiLog(wxString::Format(_("Wyst¹pi³ b³¹d indeksowania: %s"), errInfo.Buffer)); return false;
	}
	return true;
}

void Demux::Close()
{
	if (attachments.size()) {
		for (auto attachment : attachments) {
			FFMS_FreeAttachment(&attachment);
		}
	}

	if (indexer)
		FFMS_CancelIndexing(indexer);
}

bool Demux::GetSubtitles(SubsGrid* target)
{
	return false;
}

void Demux::GetFontList(wxArrayString* list)
{
	int numTracks = FFMS_GetNumTracksI(indexer);
	for (size_t i = 0; i < numTracks; i++)
	{
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
