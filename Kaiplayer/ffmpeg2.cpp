
#pragma comment(lib, "ffms2.lib")

#include "ffmpeg2.h"
#include "config.h"


#ifdef _WIN32
#include <objbase.h>
#endif

//static 
	wxProgressDialog *progress=NULL;
bool kkk=true;

int FFMS_CC UpdateProgress(int64_t Current, int64_t Total, void *ICPrivate)
	{
	//static
	
	if(kkk){
	progress = new wxProgressDialog(_("Indeksowanie"),_("Indeksowanie pliku audio"),100,0,wxPD_ELAPSED_TIME|wxPD_AUTO_HIDE|wxPD_CAN_ABORT);
	progress->Show();
	kkk=false;
		}
	progress->Update(((double)Current/(double)Total)*110);
	if(progress->WasCancelled())
		{progress->Destroy();progress=NULL; kkk=true;return 1;}
	return progress->WasCancelled();
	}



ffmpeg2::ffmpeg2(wxString filename, int tab, bool *success){
	/* If you are on Windows you should first initialize COM, or all MPEG-TS/PS and OGM
	files may return an error when you try to open them (if the library was built
	with HAALISOURCE defined). All other formats will work normally. */
#ifdef _WIN32
	com_inited = false;
	HRESULT res = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if (SUCCEEDED(res)) 
		com_inited = true;
	else if (res != RPC_E_CHANGED_MODE) {
		/* com initialization failed, handle error */
		wxMessageBox(_("com not initialized"));
	}
#endif

	audiosource = NULL;
	/* Initialize the library itself.
	The cpu caps are autodetected almost all places nowadays. Passing 0 should have close to no performance
	impact with a recent FFmpeg. If you want you can pass something like
	FFMS_CPU_CAPS_MMX | FFMS_CPU_CAPS_MMX2. */

	FFMS_Init(FFMS_CPU_CAPS_MMX | FFMS_CPU_CAPS_MMX2, 1);

	/* Index the source file. Note that this example does not index any audio tracks. */
	char errmsg[1024];
	errinfo.Buffer      = errmsg;
	errinfo.BufferSize  = sizeof(errmsg);
	errinfo.ErrorType   = FFMS_ERROR_SUCCESS;
	errinfo.SubType     = FFMS_ERROR_SUCCESS;

	*success=GetSource(filename, tab);
	}


bool ffmpeg2::GetSource(wxString filename, int tab){
	wxString path;
	path<<Options.pathfull<<_("\\")<<tab<<_("_audiocache.w64");
	const char* cpath=static_cast<const char*>(path);


	FFMS_Index *index = FFMS_MakeIndex(filename.utf8_str(), -1, -1, FFMS_DefaultAudioFilename, (void*)cpath, FFMS_IEH_ABORT, UpdateProgress, NULL, &errinfo);
	if(progress){progress->Destroy(); progress=NULL;kkk=true;}
	if (index == NULL) {
		/* handle error (print errinfo.Buffer somewhere) */
		if(wxString(errinfo.Buffer).StartsWith(_("Cancelled"))){wxMessageBox(_("Indeksowanie anulowane przez u¿ytkownika"));}
		else{
			wxMessageBox(wxString::Format(_("Wyst¹pi³ b³¹d indeksowania: %s"),errinfo.Buffer));}
		return false;
	}


	/* Retrieve the track number of the first audio track */
	int trackno = FFMS_GetFirstTrackOfType(index, FFMS_TYPE_AUDIO, &errinfo);
	if (trackno < 0) {
		wxMessageBox(wxString::Format(_("Wyst¹pi³ b³¹d szukania œcie¿ki: %s"),errinfo.Buffer));
		return false;
	}
	
	/* We now have enough information to create the audio source object */
	audiosource = FFMS_CreateAudioSource(filename.utf8_str(), trackno, index, FFMS_DELAY_FIRST_VIDEO_TRACK, &errinfo);
	if (audiosource == NULL) {
		/* handle error (you should know what to do by now) */
		wxMessageBox(wxString::Format(_("Wyst¹pi³ b³¹d tworzenia Ÿród³a audio: %s"),errinfo.Buffer));
		FFMS_DestroyIndex(index);
		return false;
	}
	
	FFMS_DestroyIndex(index);

	/* Retrieve video properties so we know what we're getting.
	As the lack of the errmsg parameter indicates, this function cannot fail. */
	const FFMS_AudioProperties *audioprops = FFMS_GetAudioProperties(audiosource);

	SampleRate=audioprops->SampleRate;
	BytesPerSample=audioprops->BitsPerSample/8;
	Channels=audioprops->Channels;
	NumSamples=audioprops->NumSamples;
	wxLogStatus("sr %i, bps %i, chan %i, nsampl %i", SampleRate, BytesPerSample, Channels, NumSamples);
	return true;
}


ffmpeg2::~ffmpeg2()
	{
	if(audiosource){FFMS_DestroyAudioSource(audiosource);audiosource=0;}
#ifdef _WIN32
	if (com_inited)
		CoUninitialize();
#endif
	}

void ffmpeg2::GetBuffer(void *buf, int64_t Start, int64_t Count, bool player)
	{
	wxCriticalSectionLocker locker(CritSec);
	try{
		if(player && Channels>2){
			DownMix(buf, Start, Count);
			}else{
				FFMS_GetAudio(audiosource, buf, Start, Count, &errinfo);}
		}
	catch(...)
		{
		memset(buf,0,Count);
		}
	
	

	//if (volume == 1.0) return;
/*
	if (BytesPerSample == 2) {
		// Read raw samples
		short *buffer = (short*) buf;
		int value;

		// Modify
		for (int64_t i=0;i<Count;i++) {
			value = (int)(buffer[i]*volume+0.5);
			if (value < -0x8000) value = -0x8000;
			if (value > 0x7FFF) value = 0x7FFF;
			buffer[i] = value;
		}
	}*/
	return;
	}

int ffmpeg2::GetSampleRate()
	{
	return SampleRate;
	}

int ffmpeg2::GetBytesPerSample()
	{
	return BytesPerSample;
	}

int ffmpeg2::GetChannels()
	{
	return (Channels<3)? Channels : 2;
	}

int64_t ffmpeg2::GetNumSamples()
	{
	return NumSamples;
	}

void ffmpeg2::GetWaveForm(int *min,int *peak,int64_t start,int w,int h,int samples,float scale) {
	// Setup
	int n = w * samples;
	for (int i=0;i<w;i++) {
		peak[i] = 0;
		min[i] = h;
	}

	// Prepare waveform
	int cur;
	int curvalue;

	// Prepare buffers
	int needLen = n*Channels*BytesPerSample;
	/*if (raw) {
		if (raw_len < needLen) {
			delete[] raw;
			raw = NULL;
		}
	}
	if (!raw) {
		raw_len = needLen;*/
	void *raw;
		raw = new char[needLen];
	//}

	if (BytesPerSample == 1) {
		// Read raw samples
		unsigned char *raw_char = (unsigned char*) raw;
		GetBuffer(raw,start,n,false);
		int amplitude = int(h*scale);

		// Calculate waveform
		for (int i=0;i<n;i++) {
			cur = i/samples;
			curvalue = h - (int(raw_char[i*Channels])*amplitude)/0xFF;
			if (curvalue > h) curvalue = h;
			if (curvalue < 0) curvalue = 0;
			if (curvalue < min[cur]) min[cur] = curvalue;
			if (curvalue > peak[cur]) peak[cur] = curvalue;
		}
	}

	if (BytesPerSample == 2) {
		// Read raw samples
		short *raw_short = (short*) raw;
		GetBuffer(raw,start,n,false);
		int half_h = h/2;
		int half_amplitude = int(half_h * scale);

		// Calculate waveform
		for (int i=0;i<n;i++) {
			cur = i/samples;
			curvalue = half_h - (int(raw_short[i*Channels])*half_amplitude)/0x8000;
			if (curvalue > h) curvalue = h;
			if (curvalue < 0) curvalue = 0;
			if (curvalue < min[cur]) min[cur] = curvalue;
			if (curvalue > peak[cur]) peak[cur] = curvalue;
		}
	}
	delete[] raw;
}


void ffmpeg2::DownMix(void *buf, int64_t Start, int64_t Count)
	{
	if (Count == 0) return;

	// We can do this ourselves
	if (Start >= NumSamples) {
		if (BytesPerSample == 1)
			// 8 bit formats are usually unsigned with bias 127
			memset(buf, 127, Count);
		else
			// While everything else is signed
			memset(buf, 0, Count*BytesPerSample);

		return;
	}

	char *tmp = new char[Count*BytesPerSample*Channels];

	FFMS_GetAudio(audiosource, tmp, Start, Count, &errinfo);


	int halfchan=Channels/2;

	if (BytesPerSample == 1) {
		uint8_t *src = (uint8_t *)tmp;
		uint8_t *dst = (uint8_t *)buf;

		while (Count > 0) {
			int sum = 0;
			for (int c = 0; c < Channels; c+=2)
				sum += *(src++);
			*(dst++) = (uint8_t)(sum / halfchan);

			sum = 0;
			for (int d = 1; d < Channels; d+=2)
				sum += *(src++);
			*(dst++) = (uint8_t)(sum / halfchan);
			
			Count--;
		}
	}
	else if (BytesPerSample == 2) {
		int16_t *src = (int16_t *)tmp;
		int16_t *dst = (int16_t *)buf;

		while (Count > 0) {
			int sum = 0;
			for (int c = 0; c < Channels; c+=2)
				sum += *(src++);
			*(dst++) = (int16_t)(sum / halfchan);

			sum = 0;
			for (int d = 0; d < Channels; d+=2)
				sum += *(src++);
			*(dst++) = (int16_t)(sum / halfchan);

			Count--;
		}
	}
	delete[] tmp;

	}

void ffmpeg2::DownMixMono(void *buf, int64_t Start, int64_t Count)
	{
	if (Count == 0) return;

	// We can do this ourselves
	if (Start >= NumSamples) {
		if (BytesPerSample == 1)
			// 8 bit formats are usually unsigned with bias 127
			memset(buf, 127, Count);
		else
			// While everything else is signed
			memset(buf, 0, Count*BytesPerSample);

		return;
	}

	char *tmp = new char[Count*BytesPerSample*Channels];

	FFMS_GetAudio(audiosource, tmp, Start, Count, &errinfo);


		int16_t *src = (int16_t *)tmp;
		int16_t *dst = (int16_t *)buf;

		while (Count > 0) {
			int sum = 0;
			for (int c = 0; c < Channels; c++)
				sum += *(src++);
			*(dst++) = (int16_t)(sum / Channels);

			Count--;
		}
	
	delete[] tmp;

	}