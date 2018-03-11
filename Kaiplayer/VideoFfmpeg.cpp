//  Copyright (c) 2016, Marcin Drob

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

#include <wx/dir.h>
#include "VideoFfmpeg.h"
#include "KainoteApp.h"
#include "Config.h"
#include "MKVWrap.h"
#include "KaiMessageBox.h"
#include <objbase.h>
#include <algorithm>
#include <process.h>
#include "include\ffmscompat.h"
#include "Stylelistbox.h"
#include <wx/file.h>
#include <thread>



VideoFfmpeg::VideoFfmpeg(const wxString &filename, VideoRend *renderer, bool *_success)
	: rend(renderer)
	, eventStartPlayback (CreateEvent(0, FALSE, FALSE, 0))
	, eventRefresh (CreateEvent(0, FALSE, FALSE, 0))
	, eventKillSelf (CreateEvent(0, FALSE, FALSE, 0))
	, eventComplete (CreateEvent(0, FALSE, FALSE, 0))
	, eventAudioComplete (CreateEvent(0, FALSE, FALSE, 0))
	,blnum(0)
	,Cache(0)
	,Delay(0)
	,audiosource(0)
	,videosource(0)
	,progress(0)
	,thread(0)
	,lastframe(-1)
	,width(-1)
	,fp(NULL)
	,index(NULL)
	,isBusy(false)
{
	if(!Options.AudioOpts && !Options.LoadAudioOpts()){KaiMessageBox(_("Nie można wczytać opcji audio"), _("Błąd"));}
	disccache = !Options.GetBool(AudioRAMCache);

	success=false;
	fname = filename;
	kainoteApp *Kaia=(kainoteApp*)wxTheApp;
	progress = new ProgressSink(Kaia->Frame,_("Indeksowanie pliku wideo"));
	//listw tracks(Kaia->Frame);
	if(renderer){

		thread = (HANDLE)_beginthreadex(0, 0, FFMS2Proc, this, 0, 0);//CreateThread( NULL, 0,  (LPTHREAD_START_ROUTINE)FFMS2Proc, this, 0, 0);
		SetThreadPriority(thread,THREAD_PRIORITY_TIME_CRITICAL);
		progress->ShowDialog();
		//HANDLE events[] = { eventComplete, eventAudioComplete };
		WaitForSingleObject(eventComplete, INFINITE);
		//ResetEvent(eventAudioComplete);
		ResetEvent(eventComplete);
		*_success = success;
	}else{
		progress->SetAndRunTask([=](){return Init();});
		progress->ShowDialog();
		*_success=((int)progress->Wait() == 1 );
	}
	SAFE_DELETE(progress);
	if(index){FFMS_DestroyIndex(index);}
	
}

unsigned int __stdcall VideoFfmpeg::FFMS2Proc(void* cls)
{
	((VideoFfmpeg*)cls)->Processing();
	return 0;
}

void VideoFfmpeg::Processing()
{
	HANDLE events_to_wait[] = {
		eventStartPlayback,
		eventRefresh,
		eventKillSelf
	};

	success=(Init()==1);
	
	progress->EndModal();

	int fplane=height * width * 4;
	int tdiff=0;
	//if (!success)
		//SetEvent(eventAudioComplete);
	SetEvent(eventComplete);
	if(width < 0){return;}

	while(1){
		DWORD wait_result = WaitForMultipleObjects(sizeof(events_to_wait)/sizeof(HANDLE), events_to_wait, FALSE, INFINITE);

		if(wait_result == WAIT_OBJECT_0+0)
		{
			byte *buff = (byte*)rend->datas;
			int acttime;
			isBusy = false;
			while(1){

				//rend->lastframe = GetFramefromMS(timeGetTime() - rend->lasttime, rend->lastframe);
				if(rend->lastframe != lastframe){
					rend->time = Timecodes[rend->lastframe];
					lastframe = rend->lastframe;
				}
				if (audioNotInitialized)
					GetFFMSFrame(rend->lastframe);
				else
					fframe=FFMS_GetFrame(videosource, rend->lastframe, &errinfo);
				
				if(!fframe){continue;}
				memcpy(&buff[0],fframe->Data[0],fplane);

				rend->DrawTexture(buff);
				rend->Render(false);
				
				if(rend->time>=rend->playend || rend->lastframe >= NumFrames-1){
					wxCommandEvent *evt = new wxCommandEvent(wxEVT_COMMAND_BUTTON_CLICKED,23333);
					wxQueueEvent(rend, evt);
					break;
				}
				else if(rend->vstate!=Playing){
					break;
				}	
				acttime = timeGetTime() - rend->lasttime; //rend->player->player->GetCurPositionMS();//
				//acttime = rend->lasttime + std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - rend->startTime).count();

				rend->lastframe++;
				rend->time = Timecodes[rend->lastframe];

				tdiff = rend->time - acttime;
				
				if(tdiff>0){Sleep(tdiff);}
				else{
					while(1){
						int frameTime = Timecodes[rend->lastframe];
						if(frameTime>=acttime || frameTime>=rend->playend || rend->lastframe>=NumFrames){
							if(rend->lastframe>=NumFrames){rend->lastframe = NumFrames-1; rend->time = rend->playend;}
							break;
						}else{
							rend->lastframe++;
						}
					}
					
				}

			}
		}else if(wait_result == WAIT_OBJECT_0+1){
			byte *buff = (byte*)rend->datas;
			if(rend->lastframe != lastframe){
				if (audioNotInitialized)
					GetFFMSFrame(rend->lastframe);
				else
					fframe = FFMS_GetFrame(videosource, rend->lastframe, &errinfo);
				lastframe = rend->lastframe;
			}
			if(!fframe){SetEvent(eventComplete);isBusy = false;continue;}
			memcpy(&buff[0],fframe->Data[0],fplane);
			rend->DrawTexture(buff);
			rend->Render(false);
			SetEvent(eventComplete);
			isBusy = false;
		}else{
			break;
		}

	}
}


int VideoFfmpeg::Init()
{

	FFMS_Init(0, 1);

	char errmsg[1024];
	errinfo.Buffer      = errmsg;
	errinfo.BufferSize  = sizeof(errmsg);
	errinfo.ErrorType   = FFMS_ERROR_SUCCESS;
	errinfo.SubType     = FFMS_ERROR_SUCCESS;

	FFMS_Indexer *Indexer = FFMS_CreateIndexer(fname.utf8_str(), &errinfo);
	if(!Indexer){
		wxLogStatus(_("Wystąpił błąd indeksowania: %s"), errinfo.Buffer); return 0;
	}

	int NumTracks = FFMS_GetNumTracksI(Indexer);
	int audiotrack=-1;
	wxArrayInt audiotable;
	int videotrack=-1;

	for (int i=0; i<NumTracks; i++) {
		if (FFMS_GetTrackTypeI(Indexer, i) == FFMS_TYPE_VIDEO && videotrack==-1) {
			videotrack=i;
		}
		else if (FFMS_GetTrackTypeI(Indexer, i) == FFMS_TYPE_AUDIO) {
			audiotable.Add(i);
		}
		//else if(audiotrack!=-1 && videotrack !=-1)
		//{break;}
	}
	bool ismkv=(fname.AfterLast('.').Lower()=="mkv");

	if(audiotable.size()>1 || ismkv){

		wxArrayString tracks;

		if(ismkv){
			MatroskaWrapper mw;
			if(mw.Open(fname,false)){
				if(audiotable.size()>1){
					for (size_t j=0;j<audiotable.size();j++){
						TrackInfo* ti=mkv_GetTrackInfo(mw.file,audiotable[j]);

						wxString all;
						char *opis = (ti->Name)? ti->Name : ti->Language;
						all<<audiotable[j]<<": "<<opis;
						tracks.Add(all);
					}
				}
				Chapter *chap=NULL;
				UINT nchap=0;
				mkv_GetChapters(mw.file,&chap,&nchap);
				
				if(chap && nchap && rend){
					for(int i=0; i<(int)chap->nChildren; i++){
						chapter ch;
						ch.name=wxString(chap->Children[i].Display->String);
						ch.time=(int)(chap->Children[i].Start/1000000.0);
						rend->chaps.push_back(ch);
					}
				}
				mw.Close();
			}
			if(audiotable.size()<2){audiotrack= (audiotable.size()>0)? audiotable[0] : -1; goto done;}
		}else{
			for (size_t j=0;j<audiotable.size();j++){
				wxString CodecName(FFMS_GetCodecNameI(Indexer, audiotable[j]), wxConvUTF8);
				wxString all;
				all<<audiotable[j]<<": "<<CodecName;
				tracks.Add(all);
			}
		}
		audiotrack = progress->ShowSecondaryDialog([=](){
			kainoteApp *Kaia=(kainoteApp*)wxTheApp;

			KaiListBox tracks(Kaia->Frame, tracks, _("Wybierz ścieżkę"),true);
			if(tracks.ShowModal()==wxID_OK){
				int result=wxAtoi(tracks.GetSelection().BeforeFirst(':'));
				return result;
			}
			return -1;
		});

		if(audiotrack == -1){
			FFMS_CancelIndexing(Indexer);
			return 0;
		}

	}else if(audiotable.size()>0){
		audiotrack=audiotable[0];
	}
done:

	indexPath=Options.pathfull + "\\Indices\\" + fname.AfterLast('\\').BeforeLast('.') + wxString::Format("_%i.ffindex",audiotrack);

	if (wxFileExists(indexPath)){
		index = FFMS_ReadIndex(indexPath.utf8_str(), &errinfo);

		if(FFMS_IndexBelongsToFile(index, fname.utf8_str(), &errinfo))
		{
			FFMS_DestroyIndex(index);
			index=NULL;
		}else{
			FFMS_CancelIndexing(Indexer);
		}

	}
	bool newIndex=false;
	if(!index){
		FFMS_TrackIndexSettings(Indexer, audiotrack, 1, 0);
		FFMS_SetProgressCallback(Indexer, UpdateProgress, (void*)progress);
		index = FFMS_DoIndexing2(Indexer, FFMS_IEH_IGNORE, &errinfo);
		//in this moment indexer was released, there no need to release it
		if (index == NULL) {
			if(wxString(errinfo.Buffer).StartsWith("Cancelled")){
				wxLogStatus(_("Indeksowanie anulowane przez użytkownika"));
			}
			else{
				wxLogStatus(_("Wystąpił błąd indeksowania: %s"), errinfo.Buffer);
			}
			//FFMS_CancelIndexing(Indexer);
			return 0;
		}
		if (!wxDir::Exists(indexPath.BeforeLast('\\')))
		{
			wxDir::Make(indexPath.BeforeLast('\\'));
		}
		if (FFMS_WriteIndex(indexPath.utf8_str(), index, &errinfo))
		{
			wxLogStatus(_("Nie można zapisać indeksu, wystąpił błąd %s"), errinfo.Buffer);
			//FFMS_DestroyIndex(index);
			//FFMS_CancelIndexing(Indexer);
			//return 0;
		}
		newIndex=true;
	}
	
	
	if(videotrack!=-1 && rend){	
		SYSTEM_INFO sysinfo;
		GetSystemInfo( &sysinfo );
		videosource = FFMS_CreateVideoSource(
			fname.utf8_str(), 
			videotrack, 
			index, 
			sysinfo.dwNumberOfProcessors*2,
			Options.GetInt(FFMS2VideoSeeking),//FFMS_SEEK_NORMAL, // FFMS_SEEK_UNSAFE/*FFMS_SEEK_AGGRESSIVE*/
			&errinfo);
		//Since the index is copied into the video source object upon its creation,
		//we can and should now destroy the index object. 

		if (videosource == NULL) {
			wxLogStatus(_("Nie można utworzyć VideoSource."));
			return 0;
		}

		const FFMS_VideoProperties *videoprops = FFMS_GetVideoProperties(videosource);

		NumFrames = videoprops->NumFrames;
		Duration=videoprops->LastTime;
		//Delay = videoprops->FirstTime + (Options.GetInt("Audio Delay")/1000);
		fps=(float)videoprops->FPSNumerator/(float)videoprops->FPSDenominator;

		const FFMS_Frame *propframe = FFMS_GetFrame(videosource, 0, &errinfo);

		width=propframe->EncodedWidth;
		height=propframe->EncodedHeight; 
		arwidth=(videoprops->SARNum==0)? width : (float)width*((float)videoprops->SARNum/(float)videoprops->SARDen);
		arheight= height;
		while(1){
			bool divided=false;
			for (int i = 10; i>1; i--){
				if((arwidth % i)==0 && (arheight % i)==0){
					arwidth/=i; arheight /=i;
					divided=true;
					break;
				}
			}
			if(!divided){break;}
		}

		int pixfmt[2];
		pixfmt[0] = FFMS_GetPixFmt("bgra");//PIX_FMT_YUYV422; //PIX_FMT_NV12 == 25  PIX_FMT_YUVJ420P;//PIX_FMT_YUV411P;//PIX_FMT_YUV420P; //PIX_FMT_YUYV422;//PIX_FMT_NV12;//FFMS_GetPixFmt("bgra");PIX_FMT_YUYV422;//
		pixfmt[1] = -1;

		if (FFMS_SetOutputFormatV2(videosource, pixfmt, width, height, FFMS_RESIZER_BILINEAR, &errinfo)) {
			wxLogStatus(_("Nie można przekonwertować wideo na RGBA"));
			return 0;
		}

		CS = propframe->ColorSpace;
		CR = propframe->ColorRange;

		if (CS == FFMS_CS_UNSPECIFIED)
			CS = width > 1024 || height >= 600 ? FFMS_CS_BT709 : FFMS_CS_BT470BG;
		ColorSpace = RealColorSpace = ColorCatrixDescription(CS, CR);
		SubsGrid *grid = ((TabPanel*)rend->GetParent())->Grid;
		const wxString &colormatrix = grid->GetSInfo("YCbCr Matrix");
		if ((CS == FFMS_CS_BT709 && colormatrix == "TV.601") || (ColorSpace != colormatrix && CS == FFMS_CS_BT470BG)) {
			if (FFMS_SetInputFormatV(videosource, CS == FFMS_CS_BT709 ? FFMS_CS_BT470BG : FFMS_CS_BT470BG, CR, FFMS_GetPixFmt(""), &errinfo)){
				wxLogStatus(_("Nie można zmienić macierzy YCbCr"));
			}
		}
		if(colormatrix == "TV.601"){
			ColorSpace = ColorCatrixDescription(FFMS_CS_BT470BG, CR);
		}
		else if (colormatrix == "TV.709"){
			ColorSpace = ColorCatrixDescription(FFMS_CS_BT709, CR);
		}

		FFMS_Track *FrameData = FFMS_GetTrackFromVideo(videosource);
		if (FrameData == NULL){
			wxLogStatus(_("Nie można pobrać ścieżki wideo"));
			return 0;}
		const FFMS_TrackTimeBase *TimeBase = FFMS_GetTimeBase(FrameData);
		if (TimeBase == NULL){
			wxLogStatus(_("Nie można pobrać informacji o wideo"));
			return 0;}

		const FFMS_FrameInfo *CurFrameData;


		// build list of keyframes and timecodes
		for (int CurFrameNum = 0; CurFrameNum < videoprops->NumFrames; CurFrameNum++) {
			CurFrameData = FFMS_GetFrameInfo(FrameData, CurFrameNum);
			if (CurFrameData == NULL) {
				continue;
			}

			// keyframe?

			int Timestamp = ((CurFrameData->PTS * TimeBase->Num) / TimeBase->Den);
			if (CurFrameData->KeyFrame){KeyFrames.Add(Timestamp);}
			Timecodes.push_back(Timestamp);

		}

	}
	//if(audiotrack==-1){ SampleRate=-1; return 1;}
	if (audiotrack != -1){
		audiosource = FFMS_CreateAudioSource(fname.utf8_str(), audiotrack, index, FFMS_DELAY_FIRST_VIDEO_TRACK, &errinfo);
		if (audiosource == NULL) {
			wxLogStatus(_("Wystąpił błąd tworzenia źródła audio: %s"), errinfo.Buffer);
			return false;
		}

		FFMS_ResampleOptions *resopts = FFMS_CreateResampleOptions(audiosource);
		resopts->ChannelLayout = FFMS_CH_FRONT_CENTER;
		resopts->SampleFormat = FFMS_FMT_S16;

		if (FFMS_SetOutputFormatA(audiosource, resopts, &errinfo)){
			wxLogStatus(_("Wystąpił błąd konwertowania audio: %s"), errinfo.Buffer);
		}
		else{
			BytesPerSample = 2;
			Channels = 1;
		}
		FFMS_DestroyResampleOptions(resopts);
		const FFMS_AudioProperties *audioprops = FFMS_GetAudioProperties(audiosource);

		SampleRate = audioprops->SampleRate;
		Delay = (Options.GetInt(AudioDelay) / 1000);
		NumSamples = audioprops->NumSamples;

		if (abs(Delay) >= (SampleRate * NumSamples * BytesPerSample)){
			wxLogStatus(_("Nie można ustawić opóźnienia, przekracza czas trwania audio"));
			Delay = 0;
		}
		audioLoadThread = new std::thread(AudioLoad, this, newIndex, audiotrack);
		audioLoadThread->detach();
	}
	return 1;
}



VideoFfmpeg::~VideoFfmpeg()
{
	if(thread){ 
		SetEvent(eventKillSelf);
		WaitForSingleObject(thread,2000);
		CloseHandle(thread);
		CloseHandle(eventStartPlayback);
		CloseHandle(eventRefresh);
		CloseHandle(eventKillSelf);
	}

	if (audioLoadThread){ 
		stopLoadingAudio = true;
		WaitForSingleObject(eventAudioComplete, INFINITE);
		CloseHandle(eventAudioComplete);
	}
	KeyFrames.Clear();
	Timecodes.clear();

	if(videosource){
		FFMS_DestroyVideoSource(videosource);videosource=NULL;
	}

	if(disccache){Cleardiskc();}else{Clearcache();}
}



int FFMS_CC VideoFfmpeg::UpdateProgress(int64_t Current, int64_t Total, void *ICPrivate)
{
	ProgressSink *progress= (ProgressSink*)ICPrivate;
	progress->Progress(((double)Current/(double)Total)*100);
	return progress->WasCancelled();
}

void VideoFfmpeg::AudioLoad(VideoFfmpeg *vf, bool newIndex, int audiotrack)
{
	if (vf->disccache){
		vf->diskCacheFilename = "";
		vf->diskCacheFilename << Options.pathfull << "\\AudioCache\\" << vf->fname.AfterLast('\\').BeforeLast('.') << "_track" << audiotrack << ".w64";
		if (!vf->DiskCache(newIndex)){ goto done; }
	}
	else{
		if (!vf->RAMCache()){ goto done; }
	}
	vf->audioNotInitialized = false;
done:
	if (vf->audiosource){ FFMS_DestroyAudioSource(vf->audiosource); vf->audiosource = NULL; }
	SetEvent(vf->eventAudioComplete);
	if (vf->audioLoadThread){ delete vf->audioLoadThread; vf->audioLoadThread = NULL; }
	
}

void VideoFfmpeg::GetFrame(int ttime, byte *buff)
{
	//if(lastframe!=ttime){fframe=FFMS_GetFrame(videosource, ttime, &errinfo);}//fframe=FFMS_GetFrameByTime(videosource, (double)ttime/1000.0, &errinfo);}
	//lastframe=ttime;
	byte* cpy= (byte *)fframe->Data[0];
	memcpy(&buff[0],cpy,height*width*4);
	
}

void VideoFfmpeg::GetFFMSFrame(int numframe)
{
	wxMutexLocker lock(blockaudio);
	fframe = FFMS_GetFrame(videosource, rend->lastframe, &errinfo);
}

void VideoFfmpeg::GetAudio(void *buf, int64_t start, int64_t count)
{
	wxMutexLocker lock(blockaudio);
	//wxLogStatus(_("weszło"));

	if (count == 0 || !audiosource) return;
	if (start+count > NumSamples) {
		int64_t oldcount = count;
		count = NumSamples-start;
		if (count < 0) count = 0;

		// Fill beyond with zero

		short *temp = (short *) buf;
		for (int64_t i=count;i<oldcount;i++) {
			temp[i] = 0;
		}

	}

	if (FFMS_GetAudio(audiosource, buf, start, count, &errinfo)){ 
		//sprawdzić co z tym kraszem
		wxLogStatus("error audio" + wxString(errinfo.Buffer)); 
	}

}

void VideoFfmpeg::GetBuffer(void *buf, int64_t start, int64_t count, double volume)
{
	if (audioNotInitialized){ return; }
	
	if (start+count > NumSamples) {
		int64_t oldcount = count;
		count = NumSamples-start;
		if (count < 0) count = 0;


		short *temp = (short *) buf;
		for (int i=count;i<oldcount;i++) {
			temp[i] = 0;
		}
	}

	if (count) {
		if(disccache){
			if(fp){
				wxMutexLocker lock(blockaudio);
				_int64 pos = start* BytesPerSample;
				_fseeki64(fp, pos, SEEK_SET);
				fread(buf, 1, count* BytesPerSample, fp);
			}
		}
		else{
			if(!Cache){return;}
			char *tmpbuf = (char *)buf;
			int i = (start* BytesPerSample) >> 22;
			int blsize=(1<<22);
			int offset = (start* BytesPerSample) & (blsize-1);
			int64_t remaining = count* BytesPerSample;
			int readsize=remaining;

			while(remaining){
				readsize = MIN(remaining,blsize - offset);

				memcpy(tmpbuf,(char *)(Cache[i++]+offset),readsize);
				tmpbuf+=readsize;
				offset=0;
				remaining-=readsize;
			}
		}
		if (volume == 1.0) return;


		// Read raw samples
		short *buffer = (short*) buf;
		int value;

		// Modify
		for (int64_t i=0;i<count;i++) {
			value = (int)(buffer[i]*volume+0.5);
			if (value < -0x8000) value = -0x8000;
			if (value > 0x7FFF) value = 0x7FFF;
			buffer[i] = value;
		}

	}
}


void VideoFfmpeg::GetWaveForm(int *min,int *peak,int64_t start,int w,int h,int samples,float scale)
{
	if (audioNotInitialized){ return; }
	wxMutexLocker lock(blockaudio);
	int n = w * samples;
	for (int i=0;i<w;i++) {
		peak[i] = 0;
		min[i] = h;
	}

	// Prepare waveform
	int cur;
	int curvalue;

	// Prepare buffers
	int needLen = n * BytesPerSample;

	void *raw;
	raw = new char[needLen];


	short *raw_short = (short*) raw;
	GetBuffer(raw,start,n);
	int half_h = h/2;
	int half_amplitude = int(half_h * scale);
	//wxLogStatus("before for");
	// Calculate waveform
	for (int i=0;i<n;i++) {
		cur = i/samples;
		curvalue = half_h - (int(raw_short[i])*half_amplitude)/0x8000;
		if (curvalue > h) curvalue = h;
		if (curvalue < 0) curvalue = 0;
		if (curvalue < min[cur]) min[cur] = curvalue;
		if (curvalue > peak[cur]) peak[cur] = curvalue;
	}
	//wxLogStatus("after for");

	delete[] raw;
	//wxLogStatus("del");

}

int VideoFfmpeg::GetSampleRate()
{
	return SampleRate;
}

int VideoFfmpeg::GetBytesPerSample()
{
	return BytesPerSample;
}

int VideoFfmpeg::GetChannels()
{
	return 1;
}

int64_t VideoFfmpeg::GetNumSamples()
{
	return NumSamples;
}

bool VideoFfmpeg::RAMCache()
{
	//progress->Title(_("Zapisywanie do pamięci RAM"));
	audioProgress = 0;
	int64_t end=NumSamples*BytesPerSample;

	int blsize=(1<<22);
	blnum=((float)end/(float)blsize)+1;
	Cache=NULL;
	Cache=new char*[blnum];
	if(Cache==NULL){KaiMessageBox(_("Za mało pamięci RAM"));return false;}

	int64_t pos= (Delay<0)? -(SampleRate * Delay * BytesPerSample) : 0;
	int halfsize=(blsize/BytesPerSample);


	for(int i = 0; i< blnum; i++)
	{
		if(i >= blnum-1){blsize=end-pos; halfsize=(blsize/BytesPerSample);}
		Cache[i]= new char[blsize];
		if(Delay>0 && i == 0){
			int delaysize=SampleRate*Delay*BytesPerSample;
			if(delaysize%2==1){delaysize++;}
			int halfdiff= halfsize - (delaysize/BytesPerSample);
			memset(Cache[i],0,delaysize);
			GetAudio(&Cache[i][delaysize], 0, halfdiff);
			pos+=halfdiff;
		}else{
			GetAudio(Cache[i], pos, halfsize);
			pos+=halfsize;
		}
		audioProgress = ((float)i / (float)(blnum - 1));
		if (stopLoadingAudio) { 
			blnum = i + 1;
			break; 
		}
	}
	if(Delay<0){NumSamples += (SampleRate * Delay * BytesPerSample);}
	audioProgress = 1.f;
	return true;
}



void VideoFfmpeg::Clearcache()
{
	if(!Cache){return;}
	for(int i=0; i<blnum; i++)
	{
		delete[ ] Cache[i];
	}
	delete[ ] Cache;
	Cache=0;
	blnum=0;
}

int VideoFfmpeg::TimefromFrame(int nframe)
{
	if(nframe<0){nframe=0;}
	if(nframe>=NumFrames){nframe=NumFrames-1;}
	return Timecodes[nframe];
}

int VideoFfmpeg::FramefromTime(int time)
{
	if(time<=0){return 0;}
	int start=lastframe;
	if(lasttime>time)
	{
		start=0;
	}
	int wframe=NumFrames-1;	
	for(int i=start;i<NumFrames-1;i++)
	{
		if(Timecodes[i]>=time && time<Timecodes[i+1])
		{
			wframe= i;
			break;
		}
	}
	//if(lastframe==wframe){return-1;}
	lastframe=wframe;
	lasttime=time;	
	return lastframe;
}

bool VideoFfmpeg::DiskCache(bool newIndex)
{
	audioProgress = 0;

	bool good=true;
	wxFileName discCacheFile;
	discCacheFile.Assign(diskCacheFilename);
	if(!discCacheFile.DirExists()){wxMkdir(diskCacheFilename.BeforeLast('\\'));}
	if (!newIndex && discCacheFile.FileExists()){
		fp = _wfopen(diskCacheFilename.wc_str(), L"rb");
		if (fp)
			return true;
		else 
			return false;
	}else{
		fp = _wfopen(diskCacheFilename.wc_str(), L"w+b");
		if(!fp || !audiosource)
			return false;
	}
	int block = 332768;
	if(Delay>0){

		int size=(SampleRate*Delay*BytesPerSample);
		if(size%2==1){size++;}
		char *silence=new char[size];
		memset(silence,0,size);
		fwrite(silence, 1 ,size, fp);
		delete[] silence;
	}
	try {
		char *data= new char[block*BytesPerSample];
		int all=(NumSamples/block)+1;
		//int64_t pos=0;
		int64_t pos= (Delay<0)? -(SampleRate * Delay * BytesPerSample) : 0;
		for (int i=0;i<all; i++) {
			if (block+pos > NumSamples) block = NumSamples - pos;
			GetAudio(data,pos,block);
			fwrite(data, 1 ,block * BytesPerSample, fp);
			pos+=block;
			audioProgress = ((float)pos / (float)(NumSamples));
			if (stopLoadingAudio) break;
		}
		delete[] data;
		
		rewind(fp);
		if(Delay<0){NumSamples += (SampleRate * Delay * BytesPerSample);}
	}
	catch (...) {
		good=false;
	}

	if(!good){Cleardiskc();}
	else{ audioProgress = 1.f; }
	return good;
}

void VideoFfmpeg::Cleardiskc()
{
	if(fp){fclose(fp);fp=NULL;}
}

int VideoFfmpeg::GetMSfromFrame(int frame)
{
	if(frame >= NumFrames){return frame * (1000.f / fps);}
	else if(frame < 0){return 0;}
	return Timecodes[frame];
}

int VideoFfmpeg::GetFramefromMS(int MS, int seekfrom)
{
	if (MS<=0) return 0;
	//else if(MS>=Duration) return NumFrames-1;
	int result=NumFrames-1;
	for(int i=seekfrom; i<NumFrames; i++)
	{
		if(Timecodes[i]>=MS)
		{
			result = i;
			break;
		}
	}
	return result;
}

void VideoFfmpeg::DeleteOldAudioCache()
{
	wxString path = Options.pathfull + "\\AudioCache";
	size_t tabsSize = Notebook::GetTabs()->Size();
	size_t maxAudio = (tabsSize < 10)? 10 : tabsSize;
	wxDir kat(path);
	wxArrayString audioCaches;
	if(kat.IsOpened()){
		kat.GetAllFiles(path, &audioCaches, "*.w64", wxDIR_FILES);
	}
	if(audioCaches.size()<=maxAudio){return;}
	FILETIME ft;
	SYSTEMTIME st;
	std::map<unsigned __int64, int> dates;
	unsigned __int64 datetime;
	for(size_t i = 0; i< audioCaches.size(); i++){
		HANDLE ffile = CreateFile(audioCaches[i].wc_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

		GetFileTime(ffile,0,&ft,0);
		CloseHandle(ffile);
		FileTimeToSystemTime(&ft, &st);
		if(st.wYear>3000){st.wYear=3000;}
		datetime= (st.wYear *360000000000000) + (st.wMonth *36000000000) + (st.wDay *360000000) + (st.wHour*3600000)+(st.wMinute*60000)+(st.wSecond*1000)+st.wMilliseconds;
		dates[datetime]=i;

	}
	int count = 0;
	int diff = audioCaches.size() - maxAudio;
	for(auto cur = dates.begin(); cur != dates.end(); cur++){
		if(count >= diff){break;}
		int isgood = _wremove(audioCaches[cur->second].wchar_str());
		//wxLogStatus("usuwa plik %i "+audioCaches[cur->second], isgood);
		count++;
	}

}

void VideoFfmpeg::Refresh(bool wait){
	if (isBusy) return;
	isBusy = true;
	ResetEvent(eventComplete);
	SetEvent(eventRefresh);
	if(rend->vstate==Paused && wait){
		WaitForSingleObject(eventComplete, 4000);
	}
};

wxString VideoFfmpeg::ColorCatrixDescription(int cs, int cr) {
	// Assuming TV for unspecified
	wxString str = cr == FFMS_CR_JPEG ? "PC" : "TV";

	switch (cs) {
		case FFMS_CS_RGB:
			return _("Brak");
		case FFMS_CS_BT709:
			return str + ".709";
		case FFMS_CS_FCC:
			return str + ".FCC";
		case FFMS_CS_BT470BG:
		case FFMS_CS_SMPTE170M:
			return str + ".601";
		case FFMS_CS_SMPTE240M:
			return str + ".240M";
		default:
			return _("Brak");
	}
}

void VideoFfmpeg::SetColorSpace(const wxString& matrix){

		if (matrix == ColorSpace) return;
		if (matrix == RealColorSpace || (matrix != "TV.601" && matrix != "TV.709"))
			FFMS_SetInputFormatV(videosource, CS, CR, FFMS_GetPixFmt(""), nullptr);
		else if (matrix == "TV.601")
			FFMS_SetInputFormatV(videosource, FFMS_CS_BT470BG, CR, FFMS_GetPixFmt(""), nullptr);
		else
			return;
		ColorSpace = matrix;

}

