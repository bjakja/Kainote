#include <wx/dir.h>
#include "VideoFfmpeg.h"
#include "KainoteApp.h"
#include "Config.h"
#include "MKVWrap.h"
#include <objbase.h>
#include <algorithm>

#include "include\ffmscompat.h"



class listw : public wxDialog
{
public:
	listw(wxWindow *parent, wxArrayString suggest);
	virtual ~listw();
	wxListBox *disperrs;
private:

	void OnDoubleClick(wxCommandEvent& event);
};

listw::listw(wxWindow *parent, wxArrayString suggest)
	: wxDialog(parent,-1,_("Wybierz ścieżkę"))
{
	wxBoxSizer *sizer=new wxBoxSizer(wxHORIZONTAL);
	disperrs=new wxListBox(this,29886,wxDefaultPosition,wxDefaultSize,suggest);
	sizer->Add(disperrs,1,wxEXPAND|wxALL,2);
	sizer->SetMinSize(100,100);
	SetSizerAndFit(sizer);
	CenterOnParent();
	Connect(29886,wxEVT_COMMAND_LISTBOX_DOUBLECLICKED,(wxObjectEventFunction)&listw::OnDoubleClick);
}
listw::~listw()
{
}

void listw::OnDoubleClick(wxCommandEvent& event)
{
	EndModal(wxID_OK);
}

struct Win32KernelHandle {
	// HANDLE value being managed
	HANDLE handle;

	//Create with a managed handle
	//handle Win32 handle to manage
	Win32KernelHandle(HANDLE handle = 0)
		: handle(handle)
	{
	}

	//Destructor, closes the managed handle
	~Win32KernelHandle()
	{
		if (handle) CloseHandle(handle);
	}

	//Returns the managed handle
	operator HANDLE () const { return handle; }
};

VideoFfmpeg::VideoFfmpeg(const wxString &filename, VideoRend *renderer, bool *_success)
	: rend(renderer)
	, eventStartPlayback (CreateEvent(0, FALSE, FALSE, 0))
	, eventRefresh (CreateEvent(0, FALSE, FALSE, 0))
	, eventKillSelf (CreateEvent(0, FALSE, FALSE, 0))
	, eventEndInit (CreateEvent(0, FALSE, FALSE, 0))
	,blnum(0)
	,Cache(0)
	,Delay(0)
	,audiosource(0)
	,videosource(0)
	,progress(0)
{
	if(!Options.AudioOpts && !Options.LoadAudioOpts()){wxMessageBox(_("Dupa blada, opcje się nie wczytały, na audio nie podziałasz"), _("Błędny błąd"));}
	disccache = !Options.GetBool("Audio RAM Cache");
	
	success=false;
	fname = filename;
	thread = CreateThread( NULL, 0,  (LPTHREAD_START_ROUTINE)FFMS2Proc, this, 0, 0);

	WaitForSingleObject(eventEndInit, INFINITE);
	*_success = success;
	CloseHandle(eventEndInit);eventEndInit=NULL;
}
	
DWORD VideoFfmpeg::FFMS2Proc(void* cls)
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

	FFMS_Init(0, 1);

	char errmsg[1024];
	errinfo.Buffer      = errmsg;
	errinfo.BufferSize  = sizeof(errmsg);
	errinfo.ErrorType   = FFMS_ERROR_SUCCESS;
	errinfo.SubType     = FFMS_ERROR_SUCCESS;

	FFMS_Indexer *Indexer = FFMS_CreateIndexer(fname.utf8_str(), &errinfo);

	int NumTracks = FFMS_GetNumTracksI(Indexer);
	int audiotrack=-1;
	//wxArrayInt audiotable;
	int videotrack=-1;

	for (int i=0; i<NumTracks; i++) {
		if (FFMS_GetTrackTypeI(Indexer, i) == FFMS_TYPE_VIDEO && videotrack==-1) {
			videotrack=i;
		}
		else if (FFMS_GetTrackTypeI(Indexer, i) == FFMS_TYPE_AUDIO && audiotrack==-1) {
			//audiotable.Add(i);
			audiotrack = i;
		}
		//else if(audiotrack!=-1 && videotrack !=-1)
		//{break;}
	}

	wxString path=Options.pathfull + "\\Indices\\" + fname.AfterLast('\\').BeforeLast('.') + wxString::Format("_%i.ffindex",audiotrack);

	FFMS_Index *index=NULL;

	if(wxFileExists(path)){
		index = FFMS_ReadIndex(path.utf8_str(), &errinfo);

		if(FFMS_IndexBelongsToFile(index, fname.utf8_str(), &errinfo))
		{
			FFMS_DestroyIndex(index);
			index=NULL;
		}

	}

	if(!index){
		FFMS_TrackIndexSettings(Indexer, audiotrack, 1, 0);
		//FFMS_SetProgressCallback(Indexer, UpdateProgress, (void*)progress);
		//FFMS_SetAudioNameCallback(Indexer, UpdateProgress, (void*)progress);
		//index =FFMS_DoIndexing(Indexer, (1<<audiotrack), 0, NULL, NULL, FFMS_IEH_IGNORE, UpdateProgress, (void*)progress, &errinfo);
		index =FFMS_DoIndexing2(Indexer, FFMS_IEH_IGNORE, &errinfo);
		//index =FFMS_MakeIndex(filen.utf8_str(), /*(1<<audiotrack)*/0, 0, NULL,NULL, FFMS_IEH_ABORT, NULL, NULL, &errinfo);
		//wxLogStatus("vidtrack %i, audtrack %i 2",videotrack,audiotrack);
		if (index == NULL) {
			if(wxString(errinfo.Buffer).StartsWith("Cancelled")){
				wxLogStatus(_("Indeksowanie anulowane przez użytkownika"));}
			else{
				wxLogStatus(wxString::Format(_("Wystąpił błąd indeksowania: %s"),errinfo.Buffer));}
			SetEvent(eventEndInit);
			return;
		}
		//wxLogStatus("write index "+path);
		if(!wxDir::Exists(path.BeforeLast('\\')))
		{
			wxDir::Make(path.BeforeLast('\\'));
		}
		//wxLogStatus("write index");
		if(FFMS_WriteIndex(path.utf8_str(), index, &errinfo))
		{
			wxLogStatus(_("Nie można zapisać indeksu, wystąpił błąd %s"), errinfo.Buffer);
			FFMS_DestroyIndex(index);
			SetEvent(eventEndInit);
			return;
		}
		//wxLogStatus("Index writed");

	}


	//wxLogStatus("video");
	if(videotrack!=-1){	
		//wxLogStatus("num of cores %i", (int)std::thread::hardware_concurrency());
		SYSTEM_INFO sysinfo;
		GetSystemInfo( &sysinfo );
		videosource = FFMS_CreateVideoSource(
			fname.utf8_str(), 
			videotrack, 
			index, 
			sysinfo.dwNumberOfProcessors,
			FFMS_SEEK_AGGRESSIVE, 
			&errinfo);// FFMS_SEEK_UNSAFE
		//Since the index is copied into the video source object upon its creation,
		//we can and should now destroy the index object. 

		if (videosource == NULL) {
			wxLogStatus(_("Dupa bada, videosource nie utworzył się."));
			SetEvent(eventEndInit);
			return;
		}



		//wxLogStatus("videoprops");
		const FFMS_VideoProperties *videoprops = FFMS_GetVideoProperties(videosource);
		//wxLogStatus("numframes");
		NumFrames = videoprops->NumFrames;
		Duration=videoprops->LastTime;
		//Delay = videoprops->FirstTime + (Options.GetInt("Audio Delay")/1000);
		fps=(float)videoprops->FPSNumerator/(float)videoprops->FPSDenominator;

		const FFMS_Frame *propframe = FFMS_GetFrame(videosource, 0, &errinfo);
		//wxLogStatus("propframe");
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
		pixfmt[0] = 25; //PIX_FMT_NV12 == 25  PIX_FMT_YUVJ420P;//PIX_FMT_YUV411P;//PIX_FMT_YUV420P; //PIX_FMT_YUYV422;//PIX_FMT_NV12;//FFMS_GetPixFmt("bgra");PIX_FMT_YUYV422;//
		pixfmt[1] = -1;

		if (FFMS_SetOutputFormatV2(videosource, pixfmt, width, height, FFMS_RESIZER_BILINEAR, &errinfo)) {
			wxLogStatus(_("Dupa bada, nie można przekonwertować wideo na NV12"));
			SetEvent(eventEndInit);
			return;
		}

		FFMS_Track *FrameData = FFMS_GetTrackFromVideo(videosource);
		if (FrameData == NULL){
			wxLogStatus(_("Dupa bada, nie można pobrać ścieżki wideo"));
			SetEvent(eventEndInit);
			return;}
		const FFMS_TrackTimeBase *TimeBase = FFMS_GetTimeBase(FrameData);
		if (TimeBase == NULL){
			wxLogStatus(_("Dupa bada, nie można pobrać informacji o wideo"));
			SetEvent(eventEndInit);
			return;}

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
	if(audiotrack==-1){FFMS_DestroyIndex(index); SampleRate=-1;}
	else{
		/* We now have enough information to create the audio source object */
		audiosource = FFMS_CreateAudioSource(fname.utf8_str(), audiotrack, index, FFMS_DELAY_FIRST_VIDEO_TRACK, &errinfo);//FFMS_DELAY_FIRST_VIDEO_TRACK
		FFMS_DestroyIndex(index);
		if (audiosource == NULL) {
			/* handle error (you should know what to do by now) */
			wxLogStatus(wxString::Format(_("Wystąpił błąd tworzenia źródła audio: %s"),errinfo.Buffer));
			SetEvent(eventEndInit);
			return;
		}


		FFMS_ResampleOptions *resopts=FFMS_CreateResampleOptions(audiosource);
		resopts->ChannelLayout=FFMS_CH_FRONT_CENTER;
		resopts->SampleFormat=FFMS_FMT_S16;

		if (FFMS_SetOutputFormatA(audiosource, resopts, &errinfo)){
			wxLogStatus(wxString::Format(_("Wystąpił błąd konwertowania audio: %s"),errinfo.Buffer));
			SetEvent(eventEndInit);
			return;
		}
		else{
			BytesPerSample=2;
			Channels=1;
		}
		FFMS_DestroyResampleOptions(resopts);
		const FFMS_AudioProperties *audioprops = FFMS_GetAudioProperties(audiosource);

		SampleRate=audioprops->SampleRate;
		//BytesPerSample=audioprops->BitsPerSample/8;
		//Channels=audioprops->Channels;
		Delay=(Options.GetInt("Audio Delay")/1000.0);
		NumSamples=audioprops->NumSamples;
		//audioprops = FFMS_GetAudioProperties(audiosource);
		if(Delay >= (SampleRate*NumSamples*BytesPerSample)){
			wxLogStatus(_("Z opóźnienia nici, przekracza czas trwania audio"));
			Delay=0;
		}

		if(disccache){
			diskCacheFilename="";
			diskCacheFilename << Options.pathfull << "\\AudioCache\\" << fname.AfterLast('\\').BeforeLast('.')<< "_track" << audiotrack << ".w64";
		
			if(!DiskCache()){SetEvent(eventEndInit);return;}
		}else{
			if(!CacheIt()){SetEvent(eventEndInit);return;}
		}
	}
	
	int pitch = width*1.5f;
	int fplane=height * width;
	int uvplane=fplane/2;
	
	int tdiff=0;

	success=true;
	SetEvent(eventEndInit);


	while(1){
		DWORD wait_result = WaitForMultipleObjects(sizeof(events_to_wait)/sizeof(HANDLE), events_to_wait, FALSE, INFINITE);

		if(wait_result == WAIT_OBJECT_0+0)
		{
			byte *buff = (byte*)rend->datas;
			while(1){

				if(rend->time>=rend->playend){
					wxCommandEvent *evt = new wxCommandEvent(wxEVT_COMMAND_BUTTON_CLICKED,23333);
					wxQueueEvent(rend, evt);
					break;
				}
				else if(rend->vstate!=Playing){break;}	

				fframe=FFMS_GetFrame(videosource, rend->lastframe, &errinfo);
		
				if(!fframe){continue;}
				memcpy(&buff[0],fframe->Data[0],fplane);
				memcpy(&buff[fplane],fframe->Data[1],uvplane);
				rend->DrawTexture(buff);
				rend->Render(false);
				rend->time= timeGetTime() - rend->lasttime;

				rend->lastframe++;

				while(true)
				{
					if(Timecodes[rend->lastframe]>=rend->time)
					{
						break;
					}
					else{rend->lastframe++;}
				}
				
				tdiff = Timecodes[rend->lastframe] - rend->time;
				rend->time = Timecodes[rend->lastframe];

				Sleep(tdiff);

			}
		}else if(wait_result == WAIT_OBJECT_0+1){
			byte *buff = (byte*)rend->datas;
			fframe=FFMS_GetFrame(videosource, rend->lastframe, &errinfo);
			if(!fframe){continue;}
			memcpy(&buff[0],fframe->Data[0],fplane);
			memcpy(&buff[fplane],fframe->Data[1],uvplane);
			rend->DrawTexture(buff);
			rend->Render(false);


		}
		else{
			break;
		}

	}
}


VideoFfmpeg::VideoFfmpeg(const wxString &filename, bool *success)
{
	//com_inited = false;
	if(!Options.AudioOpts && !Options.LoadAudioOpts()){wxMessageBox(_("Dupa blada, opcje się nie wczytały, na audio nie podziałasz"), _("Błędny błąd"));}
	disccache = !Options.GetBool("Audio RAM Cache");
	blnum=0;
	Cache=0;
	lastframe=-1;
	lasttime=-1;
	Delay=0;
	audiosource=0;
	videosource=0;
	progress=0;
	thread=0;
	//if (SUCCEEDED(CoInitializeEx(0,0))) 
	//{}	//com_inited = true;
	//else{
	//*success=false;
	//wxLogStatus("Dupa blada COM się nie zainicjalizował");
	//}
	//CoInitializeEx(0,COINIT_APARTMENTTHREADED);
	FFMS_Init(0, 1);

	char errmsg[1024];
	errinfo.Buffer      = errmsg;
	errinfo.BufferSize  = sizeof(errmsg);
	errinfo.ErrorType   = FFMS_ERROR_SUCCESS;
	errinfo.SubType     = FFMS_ERROR_SUCCESS;

	*success=(Init(filename)==1);
	if(progress){
		progress->Destroy();
		progress=0;}
	if(audiosource){FFMS_DestroyAudioSource(audiosource);audiosource=0;}
}


int VideoFfmpeg::Init(const wxString &filename)
{

	kainoteApp *Kaia=(kainoteApp*)wxTheApp;


	FFMS_Indexer *Indexer = FFMS_CreateIndexer(filename.utf8_str(), &errinfo);

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
	bool ismkv=(filename.AfterLast('.').Lower()=="mkv");

	if(audiotable.size()>1 || ismkv){

		wxArrayString tracks;

		if(ismkv){
			MatroskaWrapper mw;mw.Open(filename,false);
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
			VideoRend *rend=Notebook::GetTab()->Video;
			//wxLogStatus("chap %i, %i", (int)chap, (int)nchap);
			if(chap && nchap){
				for(int i=0; i<(int)chap->nChildren; i++){
					chapter ch;
					ch.name=wxString(chap->Children[i].Display->String);
					ch.time=(int)(chap->Children[i].Start/1000000.0);
					rend->chaps.push_back(ch);
				}
			}
			mw.Close();
			if(audiotable.size()<2){audiotrack= (audiotable.size()>0)? audiotable[0] : -1; goto done;}
		}else{
			for (size_t j=0;j<audiotable.size();j++){
				wxString CodecName(FFMS_GetCodecNameI(Indexer, audiotable[j]), wxConvUTF8);
				wxString all;
				all<<audiotable[j]<<": "<<CodecName;
				tracks.Add(all);
			}
		}
		listw lwind(Kaia->Frame,tracks);

		if(lwind.ShowModal()==wxID_OK)
		{
			//wxLogStatus("weszło");
			wxString result=lwind.disperrs->GetString(lwind.disperrs->GetSelection());
			//wxLogStatus(result);
			audiotrack=wxAtoi(result.BeforeFirst(':'));

		}else{return 0;}

	}else if(audiotable.size()>0){
		audiotrack=audiotable[0];
	}
done:

	wxString path=Options.pathfull + "\\Indices\\" + filename.AfterLast('\\').BeforeLast('.') + wxString::Format("_%i.ffindex",audiotrack);

	FFMS_Index *index=NULL;


	progress = new ProgresDialog(Kaia->Frame,_("Indeksowanie pliku wideo"));

	if(wxFileExists(path)){
		index = FFMS_ReadIndex(path.utf8_str(), &errinfo);

		if(FFMS_IndexBelongsToFile(index, filename.utf8_str(), &errinfo))
		{
			FFMS_DestroyIndex(index);
			index=NULL;
		}

	}

	if(!index){
		FFMS_TrackIndexSettings(Indexer, audiotrack, 1, 0);
		FFMS_SetProgressCallback(Indexer, UpdateProgress, (void*)progress);
		index =FFMS_DoIndexing2(Indexer, FFMS_IEH_IGNORE, &errinfo);
		//index =FFMS_DoIndexing(Indexer, (1<<audiotrack), 0, NULL, NULL, FFMS_IEH_IGNORE,
			//UpdateProgress, (void*)progress, &errinfo);
		
		if (index == NULL) {
			if(wxString(errinfo.Buffer).StartsWith("Cancelled")){wxMessageBox(_("Indeksowanie anulowane przez użytkownika"),_("Uwaga"),5,Kaia->Frame);}
			else{
				wxMessageBox(wxString::Format(_("Wystąpił błąd indeksowania: %s"),errinfo.Buffer), _("Błąd"),5,Kaia->Frame);}
			//FFMS_CancelIndexing(Indexer);
			return 0;
		}
		//wxLogStatus("write index "+path);
		if(!wxDir::Exists(path.BeforeLast('\\')))
		{
			wxDir::Make(path.BeforeLast('\\'));
		}
		//wxLogStatus("write index");
		if(FFMS_WriteIndex(path.utf8_str(), index, &errinfo))
		{
			wxLogStatus(_("Nie można zapisać indeksu, wystąpił błąd %s"), errinfo.Buffer);
			FFMS_DestroyIndex(index);
			return 0;
		}
		//wxLogStatus("Index writed");

	}


	//wxLogStatus("video");
	if(videotrack!=-1){	
		//wxLogStatus("num of cores %i", (int)std::thread::hardware_concurrency());
		SYSTEM_INFO sysinfo;
		GetSystemInfo( &sysinfo );
		videosource = FFMS_CreateVideoSource(
			filename.utf8_str(), 
			videotrack, 
			index, 
			sysinfo.dwNumberOfProcessors,
			FFMS_SEEK_AGGRESSIVE, 
			&errinfo);// FFMS_SEEK_UNSAFE
		//Since the index is copied into the video source object upon its creation,
		//we can and should now destroy the index object. 

		if (videosource == NULL) {
			wxLogStatus(_("Dupa bada, videosource nie utworzył się."));
			return 0;
		}



		//wxLogStatus("videoprops");
		const FFMS_VideoProperties *videoprops = FFMS_GetVideoProperties(videosource);
		//wxLogStatus("numframes");
		NumFrames = videoprops->NumFrames;
		Duration=videoprops->LastTime;
		//Delay = videoprops->FirstTime + (Options.GetInt("Audio Delay")/1000);
		fps=(float)videoprops->FPSNumerator/(float)videoprops->FPSDenominator;

		const FFMS_Frame *propframe = FFMS_GetFrame(videosource, 0, &errinfo);
		//wxLogStatus("propframe");
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
		pixfmt[0] = 25; //PIX_FMT_NV12 == 25  PIX_FMT_YUVJ420P;//PIX_FMT_YUV411P;//PIX_FMT_YUV420P; //PIX_FMT_YUYV422;//PIX_FMT_NV12;//FFMS_GetPixFmt("bgra");PIX_FMT_YUYV422;//
		pixfmt[1] = -1;

		//wxLogStatus("Set format %i %i",width, height);
		

		/*if (FFMS_SetInputFormatV(videosource, FFMS_CS_BT709, FFMS_CR_JPEG, PIX_FMT_NV12, &errinfo)) {
			wxLogStatus(_("Dupa bada, nie można przekonwertować klatki na fullrange"));
			return 0;
		}*/
		if (FFMS_SetOutputFormatV2(videosource, pixfmt, width, height, FFMS_RESIZER_BILINEAR, &errinfo)) {
			wxLogStatus(_("Dupa bada, nie można przekonwertować wideo na NV12"));
			return 0;
		}

		FFMS_Track *FrameData = FFMS_GetTrackFromVideo(videosource);
		if (FrameData == NULL){
			wxLogStatus(_("Dupa bada, nie można pobrać ścieżki wideo"));
			return 0;}
		const FFMS_TrackTimeBase *TimeBase = FFMS_GetTimeBase(FrameData);
		if (TimeBase == NULL){
			wxLogStatus(_("Dupa bada, nie można pobrać informacji o wideo"));
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
	/* Retrieve the track number of the first audio track */
	//int trackn = FFMS_GetFirstTrackOfType(index, FFMS_TYPE_AUDIO, &errinfo);
	//wxLogStatus("audio");
	if(audiotrack==-1){FFMS_DestroyIndex(index); SampleRate=-1; return 1;}
	/* We now have enough information to create the audio source object */
	audiosource = FFMS_CreateAudioSource(filename.utf8_str(), audiotrack, index, FFMS_DELAY_FIRST_VIDEO_TRACK, &errinfo);//FFMS_DELAY_FIRST_VIDEO_TRACK
	if (audiosource == NULL) {
		/* handle error (you should know what to do by now) */
		wxMessageBox(wxString::Format(_("Wystąpił błąd tworzenia źródła audio: %s"),errinfo.Buffer),_("Błąd"),5,Kaia->Frame);
		FFMS_DestroyIndex(index);
		return 0;
	}

	FFMS_DestroyIndex(index);
	//wxLogStatus("index destroyed");

	FFMS_ResampleOptions *resopts=FFMS_CreateResampleOptions(audiosource);
	resopts->ChannelLayout=FFMS_CH_FRONT_CENTER;
	resopts->SampleFormat=FFMS_FMT_S16;

	if (FFMS_SetOutputFormatA(audiosource, resopts, &errinfo)){
		wxMessageBox(wxString::Format(_("Wystąpił błąd konwertowania audio: %s"),errinfo.Buffer),_("Błąd"),5,Kaia->Frame);
	}
	else{
		BytesPerSample=2;
		Channels=1;
	}
	FFMS_DestroyResampleOptions(resopts);
	const FFMS_AudioProperties *audioprops = FFMS_GetAudioProperties(audiosource);

	SampleRate=audioprops->SampleRate;
	//BytesPerSample=audioprops->BitsPerSample/8;
	//Channels=audioprops->Channels;
	Delay=(Options.GetInt("Audio Delay")/1000);
	NumSamples=audioprops->NumSamples;
	//audioprops = FFMS_GetAudioProperties(audiosource);
	if(Delay >= (SampleRate*NumSamples*BytesPerSample)){
		wxMessageBox(_("Z opóźnienia nici, przekracza czas trwania audio"),_("Błąd głupoty użyszkodnika"),5,Kaia->Frame);
		Delay=0;
	}

	if(disccache){
		diskCacheFilename="";
		diskCacheFilename << Options.pathfull << "\\AudioCache\\" << filename.AfterLast('\\').BeforeLast('.')<< "_track" << audiotrack << ".w64";
		
		if(!DiskCache()){return 0;}
	}else{
		if(!CacheIt()){return 0;}
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
	KeyFrames.Clear();
	Timecodes.clear();

	if(videosource){FFMS_DestroyVideoSource(videosource);videosource=NULL;}
	//if (com_inited){
	//CoUninitialize();//}

	if(disccache){Cleardiskc();}else{Clearcache();}
}



int FFMS_CC VideoFfmpeg::UpdateProgress(int64_t Current, int64_t Total, void *ICPrivate)
{
	ProgresDialog *progress= (ProgresDialog*)ICPrivate;
	progress->Progress(((double)Current/(double)Total)*100);
	return progress->WasCancelled();
}

void VideoFfmpeg::GetFrame(int ttime, byte *buff)
{
	if(lastframe!=ttime){fframe=FFMS_GetFrame(videosource, ttime, &errinfo);}//fframe=FFMS_GetFrameByTime(videosource, (double)ttime/1000.0, &errinfo);}
	lastframe=ttime;
	int fplane=height*width;
	byte* cpy= (byte *)fframe->Data[0];
	memcpy(&buff[0],cpy,fplane);
	cpy= (byte *)fframe->Data[1];
	int uvplane=fplane/2;
	//wxLogMessage("%i, %i",vheight*pitch,fplane+uvplane);
	memcpy(&buff[fplane],cpy,uvplane);
}

void VideoFfmpeg::GetAudio(void *buf, int64_t start, int64_t count)
{
	//wxMutexLocker lock(blockaudio);
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

	FFMS_GetAudio(audiosource, buf, start, count, &errinfo);

}

void VideoFfmpeg::GetBuffer(void *buf, int64_t start, int64_t count, double volume)
{
	wxMutexLocker lock(blockaudio);

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
			if(file_cache.IsOpened()){
				file_cache.Seek(start* BytesPerSample);
				file_cache.Read((char*)buf,count* BytesPerSample);}
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
				//wxLogStatus(_("i %i, readsize %i, end %i"), i, readsize, end);
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

bool VideoFfmpeg::CacheIt()
{
	/*progress->Title(_("Zapisywanie do pamięci RAM"));*/
	//progress->cancel->Enable(false);
	int64_t end=NumSamples*BytesPerSample;

	int blsize=(1<<22);
	blnum=((float)end/(float)blsize)+1;
	Cache=NULL;
	Cache=new char*[blnum];
	if(Cache==NULL){wxMessageBox(_("Za mało pamięci RAM"));return false;}

	//int64_t pos=0;
	int64_t pos= (Delay<0)? -(SampleRate * Delay * BytesPerSample) : 0;
	int halfsize=(blsize/BytesPerSample);
	
	
	for(int i = 0; i< blnum; i++)
	{
		if(i >= blnum-1){blsize=end-pos; halfsize=(blsize/BytesPerSample);}
		Cache[i]= new char[blsize];
		//wxLogStatus("pos %i, size %i, end %i, pos+size %i",(int)pos, blsize, (int)end, ((int)pos+blsize));
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
		
		/*progress->Progress(((float)i/(float)(blnum-1))*100);
		if(progress->WasCancelled()){blnum=i+1;Clearcache();return false;}*/
	}
	if(Delay<0){NumSamples += (SampleRate * Delay * BytesPerSample);}
	return true;
}



void VideoFfmpeg::Clearcache()
{
	if(!Cache){return;}
	for(int i=0; i<blnum; i++)
	{
		//wxLogStatus("i %i",i);
		delete[ ] Cache[i];
	}
	//wxLogStatus("del cache");
	delete[ ] Cache;
	//wxLogStatus("deleted");
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

bool VideoFfmpeg::DiskCache()
{
	/*progress->Title(_("Zapisywanie na dysk twardy"));
	
	progress->Progress(0);*/
	
	bool good=true;
	wxFileName fname;
	fname.Assign(diskCacheFilename);
	if(!fname.DirExists()){wxMkdir(diskCacheFilename.BeforeLast('\\'));}
	if(wxFileExists(diskCacheFilename)){
		file_cache.Open(diskCacheFilename,wxFile::read);
		return true;
	}else{
		DeleteOldAudioCache();
		file_cache.Create(diskCacheFilename,true,wxS_DEFAULT);
		file_cache.Open(diskCacheFilename,wxFile::read_write);
	}
	int block = 332768;
	//int block2=block*2
	if(Delay>0){
		
		int size=(SampleRate*Delay*BytesPerSample);
		if(size%2==1){size++;}
		char *silence=new char[size];
		memset(silence,0,size);
		int wr= file_cache.Write(silence,size); 
		delete[] silence;
	}
	try {
		char *data= new char[block*BytesPerSample];
		int all=(NumSamples/block)+1;
		//int64_t pos=0;
		int64_t pos= (Delay<0)? -(SampleRate * Delay * BytesPerSample) : 0;
		for (int i=0;i<all; i++) {
			if (block+pos > NumSamples) block = NumSamples - pos;
			//wxLogStatus("i %i block %i nums %i", (int)pos, block, (int)NumSamples);
			GetAudio(data,pos,block);
			//wxLogStatus("write");
			file_cache.Write(data,block*BytesPerSample);
			//wxLogStatus("Progress");
			pos+=block;
			/*progress->Progress(((float)pos/(float)(NumSamples))*100);
			if(progress->WasCancelled()){
				file_cache.Close();
				wxRemoveFile(diskCacheFilename);
				good=false;
				delete[] data;
				return false;
			}*/
		}
		delete[] data;
		file_cache.Seek(0);
		if(Delay<0){NumSamples += (SampleRate * Delay * BytesPerSample);}
	}
	catch (...) {
		good=false;
	}

	if(!good){Cleardiskc();}

	return good;
}

void VideoFfmpeg::Cleardiskc()
{
	file_cache.Close();

	//wxRemoveFile(diskCacheFilename);
}

int VideoFfmpeg::GetMSfromFrame(int frame)
{
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
	wxDir kat(path);
	wxArrayString audioCaches;
	if(kat.IsOpened()){
		kat.GetAllFiles(path, &audioCaches, "*.w64", wxDIR_FILES);
	}
	if(audioCaches.size()<=10){return;}
	FILETIME ft;
	SYSTEMTIME st;
	std::map<unsigned __int64, int> dates;
	unsigned __int64 datetime;
	for(size_t i = 0; i< audioCaches.size(); i++){
		HANDLE ffile = CreateFile(audioCaches[i].wc_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

		GetFileTime(ffile,0,&ft,0);
		CloseHandle(ffile);
		FileTimeToSystemTime(&ft, &st);
		
		datetime= (st.wYear *360000000000000) + (st.wMonth *36000000000) + (st.wDay *360000000) + (st.wHour*3600000)+(st.wMinute*60000)+(st.wSecond*1000)+st.wMilliseconds;
		//wxLogStatus("date %llu %i, %i, %i, %i, %i, %i, %i, %s", datetime, (int)st.wYear, (int)st.wMonth, (int)st.wDay, (int)st.wHour, (int)st.wMinute, (int)st.wSecond, (int)st.wMilliseconds, audioCaches[i]);
		dates[datetime]=i;
		
	}
	std::map<unsigned __int64, int>::iterator cur = dates.begin();
	for(size_t i = 0; i <= audioCaches.size()-10; i++){
		wxRemoveFile(audioCaches[cur->second]);
		cur++;
	}

}