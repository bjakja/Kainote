
#include "AVSAV.h"
#include "config.h"
#include "kainoteApp.h"
#include <wx/progdlg.h>


AVSProvider::AVSProvider(wxString _filename, int tab, bool *success)
{
	Cache=0;
	blnum=0;
	wxFileName avspath(Options.pathfull + _T("\\avisynth.dll"));
	hLib=LoadLibrary(avspath.GetShortPath().wc_str());
	if (hLib == NULL) {
		*success=false;
		wxMessageBox(_T("Nie mo¿na wczytaæ avisynth.dll z œcie¿ki ")+avspath.GetShortPath());
	}
	else{
		FUNC *CreateScriptEnv = (FUNC*)GetProcAddress(hLib, "CreateScriptEnvironment");

		env=CreateScriptEnv(3);

		if (env == NULL) {
			*success=false;
			throw wxString(_T("B³¹d przy tworzeniu œrodowiska avisynt. Czy¿by wersja by³a za stara?"));
		}else{
			filename=_filename;
			*success=OpenAVSAudio();
		}
	}
}

AVSProvider::~AVSProvider()
{
	clip=NULL;
	//wxMessageBox("Env");
	if (env){delete env; env=0;}
	//wxMessageBox("free lib");
	FreeLibrary(hLib);
	//wxMessageBox("freed");
	Clearcache();
}

bool AVSProvider::LoadFromClip(AVSValue _clip)
{
	// Prepare avisynth
	AVSValue script;

	// Check if it has audio
	VideoInfo vi = _clip.AsClip()->GetVideoInfo();
	if (!vi.HasAudio()) throw wxString(_T("Nie znaleziono audio."));
	if(!vi.HasVideo()) throw wxString(_T("Nie znaleziono wideo."));
	// Convert to one channel
	script = env->Invoke("ConvertToMono", _clip);

	// Convert to 16 bits per sample
	script = env->Invoke("ConvertAudioTo16bit", script);
	if(!vi.IsVPlaneFirst()){
		AVSValue args[2] = { script, "Rec709" };
		const char *argnames[2] = { 0, "matrix" };
		script = env->Invoke("ConvertToYV12", AVSValue(args, 2), argnames);
	}
	vi = script.AsClip()->GetVideoInfo();


	// Convert sample rate
	//if (vi.SamplesPerSecond() < 32000){
	//	AVSValue args[2] = { script, 44100 };
	//	script = env->Invoke("ResampleAudio", AVSValue(args,2));
	//}

	// Set clip
	PClip tempclip = script.AsClip();
	vi = tempclip->GetVideoInfo();
	fps = (float)vi.fps_numerator / vi.fps_denominator;
	width=vi.width;
	height=vi.height;
	NumFrames=vi.num_frames;
	Duration=NumFrames*fps;
	Delay=0;
	arwidth=width;
	arheight=height;

	// Read properties
	Channels = vi.AudioChannels();
	NumSamples = vi.num_audio_samples;
	SampleRate = vi.SamplesPerSecond();
	BytesPerSample = vi.BytesPerAudioSample();

	// Set
	clip = tempclip;
	return true;
}

bool AVSProvider::OpenAVSAudio()
{
	AVSValue script;

	// Prepare avisynth
	//wxMutexLocker lock(AviSynthMutex);

	try {
		// Include
		if (filename.EndsWith(_T(".avs"))) {
			wxFileName fn(filename);
			char *fname = env->SaveString(fn.GetShortPath().mb_str(wxConvLocal));
			script = env->Invoke("Import", fname);
		}

		// Use DirectShowSource
		else {
			wxFileName fn(filename);
			const char * argnames[3] = { 0, "video", "audio" };
			AVSValue args[3] = { env->SaveString(fn.GetShortPath().mb_str(wxConvLocal)), false, true };

			// Load DirectShowSource.dll from app dir if it exists
			wxFileName dsspath(Options.pathfull + _T("\\DirectShowSource.dll"));
			if (dsspath.FileExists()) {
				env->Invoke("LoadPlugin",env->SaveString(dsspath.GetShortPath().mb_str(wxConvLocal)));
			}

			// Load audio with DSS if it exists
			if (env->FunctionExists("DirectShowSource")) {
				script = env->Invoke("DirectShowSource", AVSValue(args,3),argnames);
			}
			// Otherwise fail
			else {
				throw AvisynthError("Brak filtra DirectShowSource.dll w folderze Kainote, umieœæ go tam i ponownie otwórz audio.");
			}
		}

		LoadFromClip(script);
		if(!CacheIt()){return false;}
	}

	catch (AvisynthError &err) {
		throw wxString::Format(_T("AviSynth error: %s"), wxString(err.msg,wxConvLocal));
	}
	return true;
}



void AVSProvider::GetAudio(void *buf, int64_t start, int64_t count)
{

	if (start+count > NumSamples) {
		int64_t oldcount = count;
		count = NumSamples-start;
		if (count < 0) count = 0;

		// Fill beyond with zero
		if (BytesPerSample == 1) {
			char *temp = (char *) buf;
			for (int i=count;i<oldcount;i++) {
				temp[i] = 0;
			}
		}
		if (BytesPerSample == 2) {
			short *temp = (short *) buf;
			for (int i=count;i<oldcount;i++) {
				temp[i] = 0;
			}
		}
	}

	if (count) {
		clip->GetAudio(buf,start,count,env);
	}
}

void AVSProvider::GetBuffer(void *buf, int64_t start, int64_t count)
{
	wxMutexLocker lock(AudioMutex);

	if (start+count > NumSamples) {
		int64_t oldcount = count;
		count = NumSamples-start;
		if (count < 0) count = 0;

		// Fill beyond with zero
		if (BytesPerSample == 1) {
			char *temp = (char *) buf;
			for (int i=count;i<oldcount;i++) {
				temp[i] = 0;
			}
		}
		if (BytesPerSample == 2) {
			short *temp = (short *) buf;
			for (int i=count;i<oldcount;i++) {
				temp[i] = 0;
			}
		}
	}

	if (count) {
		char *tmpbuf = (char *)buf;
		int i = (start*BytesPerSample) >> 22;
		int blsize=(1<<22);
		int offset = (start*BytesPerSample) & (blsize-1);
		int64_t remaining = count*BytesPerSample;
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
}

void AVSProvider::GetWaveForm(int *min,int *peak,int64_t start,int w,int h,int samples,float scale)
{
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
		GetBuffer(raw,start,n);
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
		GetBuffer(raw,start,n);
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

bool AVSProvider::CacheIt()
{
	//wxLogStatus(_("bufor"));
	int64_t end=NumSamples*BytesPerSample;
	int blsize=(1<<22);
	blnum=((float)end/(float)blsize)+1;
	Cache=0;
	Cache=new char*[blnum];
	if(!Cache){wxMessageBox("Za ma³o pamiêci ram");return false;}
	//wxMessageBox("before progress");
	kainoteApp* Kaia=(kainoteApp*) wxTheApp;
	wxProgressDialog *progress = new wxProgressDialog(_("Odczyt audio"),_("Odczyt audio i zapis do pamiêci RAM"),100,Kaia->Frame,wxPD_ELAPSED_TIME|wxPD_AUTO_HIDE|wxPD_CAN_ABORT);
	progress->Show();
	//wxMessageBox("after progress");

	int64_t pos=0;
	int halfsize=(blsize/BytesPerSample);
	for(int i = 0; i< blnum; i++)
	{
		if(i >= blnum-1){blsize=end-pos;}
		Cache[i]= new char[blsize];
		GetAudio(Cache[i], pos, halfsize);
		pos+=halfsize;
		progress->Update(((float)i/(float)(blnum-1))*100);
		if(progress->WasCancelled()){blnum=i+1;progress->Destroy();return false;}
	}

	progress->Destroy();
	return true;
}

int AVSProvider::GetSampleRate()
{
	return SampleRate;
}

int AVSProvider::GetBytesPerSample()
{
	return BytesPerSample;
}

int AVSProvider::GetChannels()
{
	return Channels;
}

int64_t AVSProvider::GetNumSamples()
{
	return NumSamples;
}

void AVSProvider::Clearcache()
{
	for(int i=0; i<blnum; i++)
	{
		delete[ ] Cache[i];
	}
	delete[ ] Cache;
	Cache=0;
	blnum=0;
}

void AVSProvider::Cleardiskc(){


}

bool AVSProvider::DiskCache()
{

}

void AVSProvider::GetFrame(int frame, byte* buff)
{
	wxMutexLocker lock(VideoMutex);

	auto vframe = clip->GetFrame(frame, env);
	buff = (byte*)vframe->GetReadPtr();
}
	
int AVSProvider::TimefromFrame(int nframe)
{
	return (nframe*1000)/fps;
}
	
int AVSProvider::FramefromTime(int time)
{
	((float)time/1000.0)*fps;
}
	
int AVSProvider::GetMSfromFrame(int frame)
{
	return (frame*1000)/fps;
}
	
int AVSProvider::GetFramefromMS(int MS, int seekfrom=1)
{
	((float)MS/1000.0)*fps;
}

