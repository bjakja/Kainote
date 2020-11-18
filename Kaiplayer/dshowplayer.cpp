//  Copyright (c) 2016 - 2020, Marcin Drob

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



#include "DShowPlayer.h"
#include "Videobox.h"
#include "Menu.h"

#include <Dvdmedia.h>

template<class T>
struct Selfdest {
	T *obj;

	Selfdest()
	{
		obj = 0;
	}

	Selfdest(T *_obj)
	{
		obj = _obj;
	}

	~Selfdest()
	{
		if (obj) obj->Release();
	}

	T * operator -> ()
	{
		return obj;
	}
};
const CLSID CLSID_VobsubAutoload = { 0x9852A670, 0xF845, 0x491B, { 0x9B, 0xE6, 0xEB, 0xD8, 0x41, 0xB8, 0xA6, 0x13 } };
const IID IID_IAMExtendedSeeking = { 0xFA2AA8F9, 0x8B62, 0x11D0, { 0xA5, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } };
//const CLSID CLSID_Vobsub={0x93A22E7A,0x5091,0x45EF,{0xBA,0x61,0x6D,0xA2,0x61,0x56,0xA5,0xD0}};
//const CLSID CLSID_LAVVIDEO={0xEE30215D,0x164F,0x4A92,{0xA4,0xEB,0x9D,0x4C,0x13,0x39,0x0F,0x9F}};

DShowPlayer::DShowPlayer(wxWindow*_parent) :
m_state(None),
hwndVid(_parent->GetHWND()),
m_pGraph(NULL),
m_pControl(NULL),
m_pSeek(NULL),
m_pBA(NULL),
//frend(NULL),
stream(NULL),
chapters(NULL)
{
	parent = _parent;

	HRESULT hr = CoInitialize(NULL);
	if (FAILED(hr)){ KaiLog(_("Nie można zainicjalizować COM")); }
}


DShowPlayer::~DShowPlayer()
{
	TearDownGraph();
	m_state = None;
	CoUninitialize();
}



bool DShowPlayer::OpenFile(wxString sFileName, bool vobsub)
{
	PTR(InitializeGraph(), _("Błąd inicjalizacji Direct Show"));
	VideoCtrl *Video = (VideoCtrl*)parent;


	Selfdest<IBaseFilter> pSource;
	Selfdest<IBaseFilter> frend;
	Selfdest<IBaseFilter> pAudioRenderer;

	//bool anypin=false;

	HR(m_pGraph->AddSourceFilter(sFileName.wc_str(), L"Source Filter", &pSource.obj), _("Filtr źródła nie został dodany"));

	/*if(SUCCEEDED(CoCreateInstance(CLSID_LAVVIDEO, NULL, CLSCTX_INPROC, IID_IBaseFilter, (LPVOID *)&LAVVideo.obj)))
	{
	HR(m_pGraph->AddFilter(LAVVideo.obj, L"LAV Video Decoder"), L"Nie można dodać LAV Video Decodera");
	}else{wLogStatus("Jeśli masz zieloną plamę zamiast wideo zainstaluj Lav filter");}
	*/
	HRESULT hr;//renderer here have to be created
	CD2DVideoRender *renderer = new CD2DVideoRender(Video->GetRenderer(), &hr);
	renderer->QueryInterface(IID_IBaseFilter, (void**)&frend.obj);
	HR(m_pGraph->AddFilter(frend.obj, L"Kainote Video Renderer"), _("Nie można dodać renderera wideo"));
	HR(CoCreateInstance(CLSID_DSoundRender, NULL, CLSCTX_INPROC, IID_IBaseFilter, (LPVOID *)&pAudioRenderer.obj), _("Nie można utworzyć instancji renderera dźwięku"));

	HR(m_pGraph->AddFilter(pAudioRenderer.obj, L"Direct Sound Renderer"), _("Nie można dodać renderera Direct Sound"));

	bool hasstream = false;
	hasVobsub = false;

	if (vobsub){
		Selfdest<IBaseFilter> pVobsub;
		CoCreateInstance(CLSID_VobsubAutoload, NULL, CLSCTX_INPROC, IID_IBaseFilter, (LPVOID *)&pVobsub.obj);
		if (pVobsub.obj){
			m_pGraph->AddFilter(pVobsub.obj, L"VSFilter Autoload");//}
			Selfdest<IEnumPins> pEnum;
			Selfdest<IPin> pPin;
			//m_pGraph->RenderFile(sFileName.wc_str(),NULL);
			HR(hr = pSource->EnumPins(&pEnum.obj), _("Nie można wyliczyć pinów źródła"));
			//MessageBox(NULL, L"enumpins Initialized!", L"Open file", MB_OK);
			// Loop through all the pins
			bool anypin = false;

			while (S_OK == pEnum->Next(1, &pPin.obj, NULL))
			{
				// Try to render this pin.
				// It's OK if we fail some pins, if at least one pin renders.
				HRESULT hr2 = m_pGraph->Render(pPin.obj);

				SAFE_RELEASE(pPin.obj);

				if (SUCCEEDED(hr2))
				{
					anypin = true;
				}
			}
			if (!anypin){ return false; }
			SAFE_RELEASE(pEnum.obj);
			Selfdest<IPin> tmpPin;
			HR(hr = pVobsub->EnumPins(&pEnum.obj), _("Nie można wyliczyć pinów źródła"));
			while (S_OK == pEnum->Next(1, &pPin.obj, NULL))
			{
				if (SUCCEEDED(pPin->ConnectedTo(&tmpPin.obj)))
				{
					hasVobsub = true;
					break;
				}
			}
			if (!hasVobsub){ m_pGraph->RemoveFilter(pVobsub.obj); }
		}

	}
	else{
		// Enumerate the pins on the source filter.
		Selfdest<IEnumPins> Enumsrc;
		Selfdest<IEnumPins> Enumrend;
		Selfdest<IEnumPins> Enumaud;
		//Selfdest<IEnumPins> Enumlav;
		//HR(LAVVideo->EnumPins(&Enumlav.obj),L"Nie można wyliczyć pinów lav");
		HR(pSource->EnumPins(&Enumsrc.obj), _("Nie można wyliczyć pinów źródła"));
		HR(frend->EnumPins(&Enumrend.obj), _("Nie można wyliczyć pinów renderera"));
		HR(pAudioRenderer->EnumPins(&Enumaud.obj), _("Nie można wyliczyć pinów dsound"));


		Selfdest<IPin> spin;
		Selfdest<IPin> apin;
		Selfdest<IPin> rpin;
		//Selfdest<IPin> lpin;
		Selfdest<IEnumMediaTypes> mtypes;

		HR(Enumrend->Next(1, &rpin.obj, 0), _("Nie można pobrać pinu renderera"));
		HR(Enumaud->Next(1, &apin.obj, 0), _("Nie można pobrać pinu dsound"));
		/*PIN_INFO pinfo;

		while(Enumaud->Next(1,&lpin.obj,0)==S_OK){
		lpin->QueryPinInfo(&pinfo);
		if(pinfo.dir==PINDIR_OUTPUT){break;}

		}*/

		AM_MEDIA_TYPE *info = NULL;

		while (Enumsrc->Next(1, &spin.obj, 0) == S_OK)
		{
			HR(spin->EnumMediaTypes(&mtypes.obj), _("Brak IMediaTypes"));
			HR(mtypes->Next(1, &info, 0), _("Brak informacji o rodzaju ścieżki"));

			if (info->majortype == MEDIATYPE_Video){

				HR(m_pGraph->Connect(spin.obj, rpin.obj), _("Nie można połączyć pinu źródła z rendererem wideo"));
			}
			else if (info->majortype == MEDIATYPE_Audio){

				HR(m_pGraph->Connect(spin.obj, apin.obj), _("Nie można połączyć pinu źródła z rendererem audio"));
			}
			else if (info->majortype == MEDIATYPE_Stream){

				HR(m_pGraph->Connect(spin.obj, rpin.obj), _("Nie można połączyć pinu źródła z rendererem audio1"));
				SAFE_RELEASE(rpin.obj);
				HR(spin.obj->ConnectedTo(&rpin.obj), _("Nie można znaleźć połączonego pinu źródła"));
				PIN_INFO pinfo;
				HR(rpin.obj->QueryPinInfo(&pinfo), _("Nie można pobrać informacji o pinie splittera"));
				SAFE_RELEASE(spin.obj);
				SAFE_RELEASE(Enumrend.obj);
				HR(pinfo.pFilter->EnumPins(&Enumrend.obj), _("Nie można wyliczyć pinów splittera"));
				SAFE_RELEASE(mtypes.obj);
				//DeleteMediaType(info);info=0;
				while (Enumrend->Next(1, &spin.obj, 0) == S_OK)
				{
					HR(spin->EnumMediaTypes(&mtypes.obj), _("Brak IMediaTypes"));
					HR(mtypes->Next(1, &info, 0), _("Brak informacji o rodzaju ścieżki"));
					if (info->majortype == MEDIATYPE_Audio){
						HR(m_pGraph->Connect(spin.obj, apin.obj), _("Nie można połączyć pinu źródła z rendererem wideo2"));
						break;
					}
					//DeleteMediaType(info);info=0;
					SAFE_RELEASE(mtypes.obj);
					SAFE_RELEASE(spin.obj);
				}
				pinfo.pFilter->QueryInterface(IID_IAMStreamSelect, (void**)&stream);
				pinfo.pFilter->QueryInterface(IID_IAMExtendedSeeking, (void**)&chapters);
				hasstream = true;
				break;
			}
			if (info){ DeleteMediaType(info); }
			SAFE_RELEASE(spin.obj);
			SAFE_RELEASE(mtypes.obj);
		}
	}

	renderer->GetVidInfo(inf);

	m_state = Stopped;
	m_pSeek->SetTimeFormat(&TIME_FORMAT_MEDIA_TIME);

	if (!pSource.obj || hasstream){ return true; }
	//hr=pSource->QueryInterface(IID_IAMStreamSelect, (void**)&stream);
	if (FAILED(pSource->QueryInterface(IID_IAMStreamSelect, (void**)&stream))){
		Selfdest<IPin> spin;
		Selfdest<IPin> strpin;
		Selfdest<IEnumPins> pEnum;
		HR(hr = pSource->EnumPins(&pEnum.obj), _("Nie można wyliczyć pinów źródła"));
		HR(pEnum->Next(1, &spin.obj, NULL), _("Nie można pobrać pinu źródła"));
		spin->ConnectedTo(&strpin.obj);
		PIN_INFO pinfo;
		HR(strpin.obj->QueryPinInfo(&pinfo), _("Nie można pobrać informacji o pinie splittera"));
		if (FAILED(pinfo.pFilter->QueryInterface(IID_IAMStreamSelect, (void**)&stream)))
		{
			//No need to inform cause it will spam on avi/wmv
			//KaiLog(_("Błąd interfejsu wyboru ścieżek"));
		}
	}
	hr = pSource->QueryInterface(IID_IAMExtendedSeeking, (void**)&chapters);


	return true;
}




void DShowPlayer::Play()
{
	HRESULT hr = S_OK;
	if (m_state == None || !m_pGraph){ return; }
	if (m_state == Paused || m_state == Stopped)
	{

		hr = m_pControl->Run();
		if (SUCCEEDED(hr))
		{
			m_state = Playing;

		}
	}
}

void DShowPlayer::Pause()
{
	HRESULT hr;
	if (!m_pGraph){ return; }
	if (m_state != Playing && m_state != None)
	{

		hr = m_pControl->Run();
		if (SUCCEEDED(hr))
		{

			m_state = Playing;
		}
	}
	else if (m_state == Playing)
	{

		hr = m_pControl->Pause();
		if (SUCCEEDED(hr))
		{

			m_state = Paused;
		}
	}

}

void DShowPlayer::Stop()
{
	if (m_state != Playing && m_state != Paused)
	{
		return;
	}
	HRESULT hr;
	if (m_pControl){
		hr = m_pControl->Stop();

		if (SUCCEEDED(hr))
		{

			m_state = Stopped;
		}
	}
}



//-----------------------------------------------------------------------------
// DShowPlayer::SetPosition
// Description: Seeks to a new position.
//-----------------------------------------------------------------------------

void DShowPlayer::SetPosition(int pos)
{
	if (m_pControl == NULL || m_pSeek == NULL)
	{
		return;
	}

	HRESULT hr = S_OK;
	LONGLONG ppos = pos;
	ppos *= 10000;


	hr = m_pSeek->SetPositions(&ppos, AM_SEEKING_AbsolutePositioning,
		NULL, AM_SEEKING_NoPositioning);

	if (SUCCEEDED(hr))
	{
		// If playback is stopped, we need to put the graph into the paused
		// state to update the video renderer with the new frame, and then stop
		// the graph again. The IMediaControl::StopWhenReady does this.
		if (m_state == Stopped)
		{
			hr = m_pControl->StopWhenReady();
		}
	}

}


//-----------------------------------------------------------------------------
// DShowPlayer::GetCurrentPosition
// Description: Gets the current playback position.
//-----------------------------------------------------------------------------

int DShowPlayer::GetPosition()
{
	if (m_pSeek == NULL)
	{
		return 0;
	}
	LONGLONG pTimeNow;
	m_pSeek->GetCurrentPosition(&pTimeNow);
	return (int)(pTimeNow / 10000);
}



// Graph building

//-----------------------------------------------------------------------------
// DShowPlayer::InitializeGraph
// Description: Create a new filter graph. (Tears down the old graph.)
//-----------------------------------------------------------------------------

bool DShowPlayer::InitializeGraph()
{
	HRESULT hr = S_OK;
	TearDownGraph();

	// Create the Filter Graph Manager.
	HR(hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void**)&m_pGraph), _("Nie można stworzyć interfejsu filtrów"));

	// Query for graph interfaces. (These interfaces are exposed by the graph
	// manager regardless of which filters are in the graph.)
	HR(hr = m_pGraph->QueryInterface(IID_IMediaControl, (void**)&m_pControl), _("Nie można stworzyć kontrolera"));
	HR(hr = m_pGraph->QueryInterface(IID_IMediaSeeking, (void**)&m_pSeek), _("Nie można stworzyć interfejsu szukania"));
	HR(hr = m_pGraph->QueryInterface(IID_IBasicAudio, (void**)&m_pBA), _("Nie można stworzyć interfejsu audio"));



	return SUCCEEDED(hr);
}

//-----------------------------------------------------------------------------
// DShowPlayer::TearDownGraph
// Description: Tear down the filter graph and release resources.
//-----------------------------------------------------------------------------

void DShowPlayer::TearDownGraph()
{
	if (m_pControl && m_state != Stopped)
	{
		m_pControl->Stop();
	}

	SAFE_RELEASE(stream);
	SAFE_RELEASE(chapters);
	SAFE_RELEASE(m_pControl);
	SAFE_RELEASE(m_pBA);

	SAFE_RELEASE(m_pSeek);

	SAFE_RELEASE(m_pGraph);

	m_state = None;
}



wxSize DShowPlayer::GetVideoSize()
{
	return wxSize(inf.width, inf.height);
}

int DShowPlayer::GetDuration()
{
	LONGLONG dur = 0;
	if (m_pSeek)
	{
		m_pSeek->GetDuration(&dur);
		return dur / 10000;
	}
	return 0;
}

void DShowPlayer::SetVolume(long volume)
{
	HRESULT hr;
	if (m_pBA)
	{
		hr = m_pBA->put_Volume(volume);
		if (FAILED(hr)){ KaiLogSilent(L"Set volume dont working!"); }
	}

}

long DShowPlayer::GetVolume()
{
	HRESULT hr;
	long volume;
	if (m_pBA)
	{
		hr = m_pBA->get_Volume(&volume);
		if (FAILED(hr)){ KaiLogSilent(L"Get volume dont working!"); }
	}
	return volume;
}

void DShowPlayer::GetFpsnRatio(float *fps, long *arx, long *ary)
{
	if (arx){ *arx = inf.ARatioX; }
	if (ary){ *ary = inf.ARatioY; }
	if (fps){ *fps = inf.fps; }
}


wxArrayString DShowPlayer::GetStreams()
{
	wxArrayString streamnames;
	if (!stream){ return streamnames; }
	DWORD streams = 0;
	stream->Count(&streams);
	if (streams > 0){
		wxString names;
		DWORD st;
		LPWSTR text = NULL;
		for (DWORD i = 0; i < streams; i++){
			stream->Info(i, NULL, &st, NULL, NULL, &text, NULL, NULL);
			wxString enable = (st != 0) ? L"1" : L"0";
			streamnames.Add(wxString(text) + L" " + enable);
			CoTaskMemFree(text);
		}
	}
	return streamnames;
}

void DShowPlayer::GetChapters(std::vector<chapter> *chaps)
{
	if (!chapters || !chaps){ return; }
	long mcount = 0;
	if (FAILED(chapters->get_MarkerCount(&mcount))){ return; }
	chaps->clear();

	for (long i = 1; i <= mcount; i++)
	{
		LPWSTR string;
		double time = 0;
		if (FAILED(chapters->GetMarkerName(i, &string))){ continue; }
		chapters->GetMarkerTime(i, &time);
		chapter ch;
		ch.name = wxString(string);
		ch.time = time * 1000;
		chaps->push_back(ch);
		SysFreeString(string);
	}
}

bool DShowPlayer::EnumFilters(Menu *menu)
{
	Selfdest<IEnumFilters> efilters;
	IBaseFilter *bfilter = 0;
	ISpecifyPropertyPages *ppages = 0;
	FILTER_INFO fi;
	int numfilter = 0;
	HR(m_pGraph->EnumFilters(&efilters.obj), _("Nie można wyliczyć filtrów"));
	while (S_OK == efilters->Next(1, &bfilter, 0))
	{
		bfilter->QueryInterface(__uuidof(ISpecifyPropertyPages), (void**)&ppages);
		HR(bfilter->QueryFilterInfo(&fi), _("Nie można pobrać nazwy filtra"));
		menu->Append(13000 + numfilter, wxString(fi.achName))->Enable(ppages != 0);

		numfilter++;
		fi.pGraph->Release();
		SAFE_RELEASE(bfilter);
		SAFE_RELEASE(ppages);
	}
	return true;
}



bool DShowPlayer::FilterConfig(wxString name, int idx, wxPoint pos)
{
	Selfdest<IBaseFilter> bfilter;
	int numfilter = 0;
	Selfdest<ISpecifyPropertyPages> ppages;
	CAUUID caGUID;
	caGUID.pElems = NULL;
	HR(m_pGraph->FindFilterByName(name.wc_str(), &bfilter.obj), _("Nie można wyliczyć filtrów"));
	bfilter->QueryInterface(__uuidof(ISpecifyPropertyPages), (void**)&ppages.obj);
	if (ppages.obj == 0){ return false; }
	HR(ppages->GetPages(&caGUID), _("Nie można pobrać konfiguracji filtra"));
	IUnknown* lpUnk = NULL;
	ppages->QueryInterface(&lpUnk);
	try
	{
		OleCreatePropertyFrame(parent->GetHWND(), pos.x, pos.y, name.wc_str(), 1, (IUnknown**)&lpUnk, caGUID.cElems, caGUID.pElems, 0, 0, 0);
	}
	catch (...)
	{
	}
	if (caGUID.pElems) CoTaskMemFree(caGUID.pElems);
	return true;
}


/*void DShowPlayer::Disconvobsub()
	{
	if(!m_pGraph||vobsub){return;}
	IBaseFilter *Avobsub=NULL;
	m_pGraph->FindFilterByName(L"Vobsub Autoload",&Avobsub);
	if(!Avobsub){return;}
	if(m_state==Playing){Pause();}
	IEnumPins *enumIn=NULL;
	IPin *vobpin1=NULL;
	IPin *vobpin2=NULL;
	IPin *vobpin3=NULL;
	IPin *Outpin1=NULL;
	IPin *Outpin2=NULL;
	IPin *Outpin3=NULL;

	HRESULT hr;
	CHECK_HR1(hr=Avobsub->EnumPins(&enumIn),L"Enum Pins");
	if(S_OK == enumIn->Next(1, &vobpin1, NULL)){
	vobpin1->ConnectedTo(&Outpin1);if(Outpin1){CHECK_HR1(hr=Outpin1->Disconnect(),L"Cannot Disconnect Outpin1");MessageBox(NULL, L"Outpin1!", L"vinfo", MB_OK);}
	if(S_OK == enumIn->Next(1, &vobpin2, NULL)){
	vobpin2->ConnectedTo(&Outpin2);if(Outpin2){CHECK_HR1(hr=Outpin2->Disconnect(),L"Cannot Disconnect Outpin2");MessageBox(NULL, L"Outpin2!", L"vinfo", MB_OK);}}
	if(S_OK == enumIn->Next(1, &vobpin3, NULL)){
	vobpin3->ConnectedTo(&Outpin3);if(Outpin3){

	PIN_DIRECTION pd2;CHECK_HR1(hr=Outpin3->QueryDirection(&pd2),L"pin dir outpin3");
	if(pd2==PINDIR_INPUT){MessageBox(NULL, L"Pindir input outpin3", L"vinfo", MB_OK);}else{MessageBox(NULL, L"Pindir output outpin3!", L"vinfo", MB_OK);}
	}}
	MessageBox(NULL, L"Connection!", L"vinfo", MB_OK);
	Outpin1->

	if(Outpin3){PIN_DIRECTION pd;CHECK_HR1(hr=Outpin3->QueryDirection(&pd),L"pin dir outpin1");
	if(pd==PINDIR_INPUT){MessageBox(NULL, L"Pindir input outpin1!", L"vinfo", MB_OK);}else{MessageBox(NULL, L"Pindir output outpin1!", L"vinfo", MB_OK);}

	if(Outpin2){PIN_DIRECTION pd1;CHECK_HR1(hr=Outpin2->QueryDirection(&pd1),L"pin dir outpin1");
	if(pd1==PINDIR_INPUT){MessageBox(NULL, L"Pindir input outpin2!", L"vinfo", MB_OK);}else{MessageBox(NULL, L"Pindir output outpin2!", L"vinfo", MB_OK);}

	CHECK_HR1(hr=m_pGraph->Connect((pd==PINDIR_INPUT)?Outpin2:Outpin3,(pd==PINDIR_INPUT)?Outpin3:Outpin2),L"Connect outpin1 and 2 failed");}
	//if(Outpin3){CHECK_HR1(hr=m_pGraph->Connect((pd==PINDIR_INPUT)?Outpin3:Outpin1,(pd==PINDIR_INPUT)?Outpin1:Outpin3),L"Connect outpin1 and 3 failed");}
	}
	else if(Outpin2){PIN_DIRECTION pd;CHECK_HR1(hr=Outpin2->QueryDirection(&pd),L"pin dir outpin2");
	if(Outpin1){CHECK_HR1(hr=m_pGraph->Connect((pd==PINDIR_INPUT)?Outpin1:Outpin2,(pd==PINDIR_INPUT)?Outpin2:Outpin1),L"Connect outpin2 and 1 failed");}
	if(Outpin3){CHECK_HR1(hr=m_pGraph->Connect((pd==PINDIR_INPUT)?Outpin3:Outpin2,(pd==PINDIR_INPUT)?Outpin2:Outpin3),L"Connect outpin2 and 3 failed");}
	}
	CHECK_HR1(hr=m_pGraph->RemoveFilter(Avobsub),L"vobsub remove failed");

	done:
	SAFE_RELEASE(Avobsub);
	SAFE_RELEASE(enumIn);
	SAFE_RELEASE(vobpin1);
	SAFE_RELEASE(vobpin2);
	SAFE_RELEASE(vobpin3);
	SAFE_RELEASE(Outpin1);
	SAFE_RELEASE(Outpin2);
	SAFE_RELEASE(Outpin3);
	}


	*/