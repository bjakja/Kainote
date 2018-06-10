// Copyright (c) 2004-2006, Rodrigo Braz Monteiro, Mike Matsnev
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of the Aegisub Group nor the names of its contributors
//     may be used to endorse or promote products derived from this software
//     without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// -----------------------------------------------------------------------------
//
// AEGISUB
//
// Website: http://aegisub.cellosoft.com
// Contact: mailto:zeratul@cellosoft.com
//


///////////
// Headers
#include "config.h"
#include <vector>
#include <algorithm>
#include <errno.h>
#include <stdint.h>
#include <wx/tokenzr.h>
#include <wx/choicdlg.h>
#include <wx/filename.h>
#include "ProgressDialog.h"
#include "MKVWrap.h"
#include "SubsGrid.h"
#include "SubsTime.h"
#include "KaraokeSplitting.h"
#include "OpenNWrite.h"
#include "KaiMessageBox.h"
#include "Stylelistbox.h"

////////////
// Instance
MatroskaWrapper MatroskaWrapper::wrapper;


///////////
// Defines
#define	CACHESIZE     65536


///////////////
// Constructor
MatroskaWrapper::MatroskaWrapper() {
	file = NULL;
}


//////////////
// Destructor
MatroskaWrapper::~MatroskaWrapper() {
	Close();
}


/////////////
// Open file
bool MatroskaWrapper::Open(const wxString &filename,bool parse) {
	// Make sure it's closed first
	Close();
	atts=NULL;
	count=0;
	// Open
	char err[2048];
	input = new MkvStdIO(filename);

	if (input->fp) {

		file = mkv_Open(input,err,sizeof(err));

		// Failed parsing
		if (!file) {
			delete input;
			//throw wxString("MatroskaParser error: " + wxString(err,wxConvUTF8)).c_str();
			wxLogStatus(_("Błąd MatroskaParsera: ") + wxString(err,wxConvUTF8));
			return false;
		}

		// Parse
		//if (parse) Parse();
	}

	// Failed opening
	else {
		delete input;
		//throw "Unable to open Matroska file for parsing.";
		wxLogStatus(_("Nie można otworzyć pliku Matroska."));
		return false;
	}
	return true;
}


//////////////
// Close file
void MatroskaWrapper::Close() {
	//if (atts){
	//delete[] atts;
	//atts=NULL;
	//}
	if (file) {
		mkv_Close(file);
		file = NULL;
		delete input;
	}
}



/////////////////
// Get subtitles
bool MatroskaWrapper::GetSubtitles(SubsGrid *target) {
	// Get info

	int tracks = mkv_GetNumTracks(file);
	TrackInfo *trackInfo;
	//SegmentInfo *segInfo = mkv_GetFileInfo(file);
	wxArrayInt tracksFound;
	wxArrayString tracksNames;
	int trackToRead = -1;


	// Find tracks
	for (int track=0;track<tracks;track++) {
		trackInfo = mkv_GetTrackInfo(file,track);

		// Subtitle track
		if (trackInfo->Type == 0x11) {
			wxString CodecID = wxString(trackInfo->CodecID,*wxConvCurrent);
			wxString TrackName = wxString(trackInfo->Name,*wxConvCurrent);
			wxString TrackLanguage = wxString(trackInfo->Language,*wxConvCurrent);

			// Known subtitle format
			if (CodecID == "S_TEXT/SSA" || CodecID == "S_TEXT/ASS" || CodecID == "S_TEXT/UTF8") {
				tracksFound.Add(track);
				tracksNames.Add(wxString::Format("%i (",track) + CodecID + " " + TrackLanguage + "): " + TrackName);
			}
		}
	}


	// No tracks found
	if (tracksFound.Count() == 0) {
		Close();
		KaiMessageBox(_("Plik nie ma żadnej ścieżki z napisami."));
		return false;
	}

	// Only one track found
	else if (tracksFound.Count() == 1) {
		trackToRead = tracksFound[0];
	}

	// Pick a track
	else {
		KaiListBox tracks(target->GetParent(), tracksNames, _("Wybierz ścieżkę napisów"),true);
		//int choice = wxGetSingleChoiceIndex(_("Wybierz ścieżkę do wczytania:"), _("Znaleziono kilka ścieżek z napisami"), tracksNames);
		if (tracks.ShowModal() != wxID_OK) {
			Close();
			KaiMessageBox(_("Anulowano."));
			return false;
		}
		trackToRead = tracksFound[tracks.GetIntSelection()];
	}

	// Picked track
	if (trackToRead != -1) {

		// Get codec type (0 = ASS/SSA, 1 = SRT)
		trackInfo = mkv_GetTrackInfo(file,trackToRead);
		//wxLogStatus("track infos %i %i", (int)trackInfo->CompEnabled, trackInfo->CompMethod);
		CompressedStream *cs=NULL;
		if(trackInfo->CompEnabled && trackInfo->CompMethod==0){
			char msg[201];
			msg[200]=0;
			cs=cs_Create(file,trackToRead,msg,200);
			if(!cs){wxLogStatus(_("Błąd zlib: %s"), msg);}
		}
		wxString CodecID = wxString(trackInfo->CodecID,*wxConvCurrent);
		int codecType = 0;
		if (CodecID == "S_TEXT/UTF8") codecType = 1;



		// Read timecode scale
		SegmentInfo *segInfo = mkv_GetFileInfo(file);

		longlong timecodeScale = mkv_TruncFloat(trackInfo->TimecodeScale) * segInfo->TimecodeScale;


		ProgressSink *progress = new ProgressSink(target->GetParent(),_("Odczyt napisów z pliku Matroska."));
		progress->SetAndRunTask([=](){

			char form=1;
			// Haali's library variables
			ulonglong startTime, endTime, filePos;
			unsigned int rt, frameSize, frameFlags;
			// Prepare STD vector to get lines inserted
			std::vector<wxString> subList;
			long int order = -1;

			// Progress bar
			int totalTime = int(double(segInfo->Duration) / timecodeScale);

			// Load blocks
			// Mask is unsigned int works wrong with files with num of tracks above 32
			mkv_SetTrackMask(file, ~(1 << trackToRead));
			while (mkv_ReadFrame(file, 0, &rt, &startTime, &endTime, &filePos, &frameSize, &frameFlags) != EOF) {
				//This check prevents loading bad tracks when num of tracks is above 32
				if (trackToRead != rt)
					continue;
				// Canceled			
				if (progress->WasCancelled()) {
					subList.clear();
					Close();
					return 0;
				}


				// Read to temp
				char *tmp;
				if(cs){
					int oscfs=frameSize*10;
					tmp=new char[oscfs+1];
					cs_NextFrame(cs,filePos,frameSize);
					int rdata= cs_ReadData(cs,tmp,oscfs);
					tmp[rdata] = 0;
				}else{
					tmp=new char[frameSize+1];
					_fseeki64(input->fp, filePos, SEEK_SET);
					fread(tmp,1,frameSize,input->fp);
					tmp[frameSize] = 0;
				}

				wxString blockString(tmp,wxConvUTF8);
				delete[] tmp;

				// Get start and end times
				//longlong timecodeScaleLow = timecodeScale / 100;
				longlong timecodeScaleLow = 1000000;
				STime subStart,subEnd;
				startTime /=timecodeScaleLow;
				endTime /=timecodeScaleLow;
				if (codecType == 0) { startTime=ZEROIT(startTime); endTime=ZEROIT(endTime);}
				subStart.NewTime(startTime);
				subEnd.NewTime(endTime);
				//wxLogMessage(subStart.GetASSFormated() + "-" + subEnd.GetASSFormated() + ": " + blockString);

				// Process SSA/ASS
				if (codecType == 0) {
					// Get order number
					int pos = blockString.Find(",");
					wxString orderString = blockString.Left(pos);
					orderString.ToLong(&order);
					blockString = blockString.Mid(pos+1);

					// Get layer number
					pos = blockString.Find(",");
					long int layer = 0;
					if (pos) {
						wxString layerString = blockString.Left(pos);
						layerString.ToLong(&layer);
						blockString = blockString.Mid(pos+1);
					}
					if(blockString==""){blockString<<"Default,,0000,0000,0000,,";}
					// Assemble final
					if(!blockString.StartsWith(",")){blockString.Prepend(",");}
					blockString = wxString::Format("Dialogue: %li,",layer) + subStart.raw() + "," + subEnd.raw() + blockString;

				}

				// Process SRT
				else {

					blockString = subStart.raw(SRT) + " --> " + subEnd.raw(SRT) + "\r\n" + blockString;

					order++;
				}

				// Insert into vector
				subList.push_back(blockString);
				//if (subList.size() == (unsigned int)order) subList.push_back(blockString);
				//else {
				//if ((signed)(subList.size()) < order+1) subList.resize(order+1);
				//subList[order] = blockString;
				//}

				// Update progress bar
				//progress->SetProgress(int(double(startTime) / 1000000.0),totalTime);
				int prog=((double(startTime))/double(totalTime))*100;
				progress->Progress(prog);
			}

			if (!subList.size())
				return 0;

			target->Clearing();
			target->file=new SubsFile();

			// Read private data if it's ASS/SSA
			if (codecType == 0) {
				// Read raw data
				TrackInfo *trackInfo = mkv_GetTrackInfo(file,trackToRead);
				unsigned int privSize = trackInfo->CodecPrivateSize;
				char *privData = new char[privSize+1];
				memcpy(privData,trackInfo->CodecPrivate,privSize);
				privData[privSize] = 0;
				wxString privString(privData,wxConvUTF8);
				delete[] privData;

				// Load into file
				wxString group = "[Script Info]";
				if (CodecID == "S_TEXT/SSA") form = 2;
				wxStringTokenizer token(privString,"\r\n",wxTOKEN_STRTOK);
				while (token.HasMoreTokens()) {
					wxString next = token.GetNextToken();
					if (next[0] == '[') group = next;
					if(group=="[Script Info]"&&!next.StartsWith(";")){
						target->AddSInfo(next);
					}
					else if(next.StartsWith("Style:")){
						target->AddStyle(new Styles(next,form));}
					else if(next.StartsWith("Comment")){
						target->AddLine(new Dialogue(next));}
				}


			}

			//progress->Update(99,"Wstawianie napisów do okna");
			for (unsigned int i=0;i<subList.size();i++) {
				target->AddLine(new Dialogue(subList[i]));
			}
			const wxString &matrix = target->GetSInfo("YCbCr Matrix");
			if ((matrix == "" || matrix == "None") && codecType < 1){ target->AddSInfo("YCbCr Matrix", "TV.601"); }
			target->file->EndLoad(OPEN_SUBTITLES, 0, true);
			subList.clear();
			return 1;
		});
		progress->ShowDialog();
		bool isgood =((int)progress->Wait() == 1 );
		delete progress; progress=0;
		if(cs){cs_Destroy(cs);}
		return isgood;
	}

	// No track to load
	return false;
}


std::map<int, wxString> MatroskaWrapper::GetFontList()
{
	std::map<int, wxString> attsname;
	mkv_GetAttachments(file, &atts, &count);
	if(!atts || count==0){return attsname;}

	for(size_t i=0; i<count; i++)
	{
		wxString mimetype(atts[i].MimeType,wxConvUTF8);
		if(mimetype== "application/x-truetype-font" || mimetype== "application/vnd.ms-opentype"){
			attsname[i]=(wxString(atts[i].Name,wxConvUTF8));}
	}
	return attsname;
}

bool MatroskaWrapper::SaveFont(int id,const wxString &path, wxZipOutputStream *zip)
{

	char *tmp = new char[atts[id].Length];
	_fseeki64(input->fp, atts[id].Position, SEEK_SET);
	fread(tmp,1,atts[id].Length,input->fp);
	bool isgood=true;

	if(zip){
		wxString fn = path.AfterLast('\\');
		try{
			isgood=zip->PutNextEntry(fn);
			zip->Write((void*)tmp,atts[id].Length);
		}
		catch(...)
		{
			isgood=false;
		}
	}
	else
	{
		wxFile file;
		file.Create(path,true,wxS_DEFAULT);
		if (file.IsOpened()){
			file.Write(tmp, atts[id].Length);
			file.Close();
		}else{isgood=false;}

	}
	delete[] tmp;
	return isgood;
}



////////////////////////////// LOTS OF HAALI C CODE DOWN HERE ///////////////////////////////////////

#ifdef __VISUALC__
#define std_fread fread
#define std_fseek _fseeki64
#define std_ftell _ftelli64
#else
#define std_fread fread
#define std_fseek fseeko
#define std_ftell ftello
#endif

///////////////
// STDIO class
int StdIoRead(InputStream *_st, ulonglong pos, void *buffer, int count) {
	MkvStdIO *st = (MkvStdIO *) _st;
	size_t  rd;
	if (std_fseek(st->fp, pos, SEEK_SET)) {
		st->error = errno;
		return -1;
	}
	rd = std_fread(buffer, 1, count, st->fp);
	if (rd == 0) {
		if (feof(st->fp))
			return 0;
		st->error = errno;
		return -1;
	}
	return rd;
}

/* scan for a signature sig(big-endian) starting at file position pos
* return position of the first byte of signature or -1 if error/not found
*/
longlong StdIoScan(InputStream *_st, ulonglong start, unsigned signature) {
	MkvStdIO *st = (MkvStdIO *) _st;
	int	      c;
	unsigned    cmp = 0;
	FILE	      *fp = st->fp;

	if (std_fseek(fp, start, SEEK_SET))
		return -1;

	while ((c = getc(fp)) != EOF) {
		cmp = ((cmp << 8) | c) & 0xffffffff;
		if (cmp == signature)
			return std_ftell(fp) - 4;
	}

	return -1;
}

/* return cache size, this is used to limit readahead */
unsigned StdIoGetCacheSize(InputStream *_st) {
	return CACHESIZE;
}

/* return last error message */
const char *StdIoGetLastError(InputStream *_st) {
	MkvStdIO *st = (MkvStdIO *) _st;
	return strerror(st->error);
}

/* memory allocation, this is done via stdlib */
void  *StdIoMalloc(InputStream *_st, size_t size) {
	return malloc(size);
}

void  *StdIoRealloc(InputStream *_st, void *mem, size_t size) {
	return realloc(mem,size);
}

void  StdIoFree(InputStream *_st, void *mem) {
	free(mem);
}

int   StdIoProgress(InputStream *_st, ulonglong cur, ulonglong max) {
	return 1;
}

longlong StdIoGetFileSize(InputStream *_st) {
	MkvStdIO *st = (MkvStdIO *) _st;
	longlong epos = 0;
	longlong cpos = std_ftell(st->fp);
	std_fseek(st->fp, 0, SEEK_END);
	epos = std_ftell(st->fp);
	std_fseek(st->fp, cpos, SEEK_SET);
	return epos;
}

MkvStdIO::MkvStdIO(const wxString &filename) {
	read = StdIoRead;
	scan = StdIoScan;
	getcachesize = StdIoGetCacheSize;
	geterror = StdIoGetLastError;
	memalloc = StdIoMalloc;
	memrealloc = StdIoRealloc;
	memfree = StdIoFree;
	progress = StdIoProgress;
	getfilesize = StdIoGetFileSize;
	fp = _wfopen(filename.wc_str (),L"rb");
	if (fp) {
		setvbuf(fp, NULL, _IOFBF, CACHESIZE);
	}
}
