//  Copyright (c) 2016-2018, Marcin Drob

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



#include "DirectShowPlayer.h"
#include "Videobox.h"
#include "CsriMod.h"
#include "kainoteMain.h"
#include "OpennWrite.h"

#include <Dvdmedia.h>
#pragma comment(lib, "Dxva2.lib")


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

const IID IID_IDirectXVideoProcessorService = { 0xfc51a552, 0xd5e7, 0x11d9, { 0xaf, 0x55, 0x00, 0x05, 0x4e, 0x43, 0xff, 0x02 } };

DirectShowPlayer::DirectShowPlayer(VideoCtrl* _parent) :
VideoPlayer(_parent),
m_pGraph(NULL),
m_pControl(NULL),
m_pSeek(NULL),
m_pBA(NULL),
stream(NULL),
chaptersControl(NULL)
{
	HRESULT hr = CoInitialize(NULL);
	if (FAILED(hr)){ KaiLog(_("Nie można zainicjalizować COM")); }
}


DirectShowPlayer::~DirectShowPlayer()
{
	TearDownGraph();
	vstate = None;
	CoUninitialize();
}

bool DirectShowPlayer::OpenFile(const wxString &fname, wxString *textsubs, bool vobsub, bool changeAudio)
{
	wxMutexLocker lock(mutexOpenFile);
	TabPanel *tab = ((TabPanel*)videoWindow->GetParent());
	VideoFfmpeg *tmpvff = NULL;
	if (vstate == Playing){ videoWindow->Stop(); }


	if (vstate != None){
		resized = seek = cross = pbar = false;
		vstate = None;
		Clear();
	}
	time = 0;
	numframe = 0;


	if (OpenFile(fname, vobsub)){
		return false;
	}
	vwidth = videoInfo.width, vheight = videoInfo.height;
	if (vwidth % 2 != 0){ vwidth++; }

	pitch = vwidth * videoInfo.bytes;
	fps = videoInfo.fps;
	vformat = videoInfo.CT;
	ax = videoInfo.ARatioX;
	ay = videoInfo.ARatioY;
	d3dformat = (vformat == 5) ? D3DFORMAT('21VN') : (vformat == 3) ? D3DFORMAT('21VY') : (vformat == 2) ? D3DFMT_YUY2 : D3DFMT_X8R8G8B8;


	frameDuration = (1000.0f / fps);
	if (ay == 0 || ax == 0){ AR = 0.0f; }
	else{ AR = (float)ay / (float)ax; }

	mainStreamRect.bottom = vheight;
	mainStreamRect.right = vwidth;
	mainStreamRect.left = 0;
	mainStreamRect.top = 0;
	if (datas){ delete[] datas; datas = NULL; }
	datas = new byte[vheight*pitch];

	if (!InitDX()){ return false; }
	UpdateRects();

	if (!framee){ framee = new csri_frame; }
	if (!format){ format = new csri_fmt; }
	for (int i = 1; i < 4; i++){
		framee->planes[i] = NULL;
		framee->strides[i] = NULL;
	}

	framee->pixfmt = (vformat == 5) ? CSRI_F_YV12A : (vformat == 3) ? CSRI_F_YV12 : (vformat == 2) ? CSRI_F_YUY2 : CSRI_F_BGR_;

	format->width = vwidth;
	format->height = vheight;
	format->pixfmt = framee->pixfmt;
	format->fps = 25.0f;

	if (!vobsub){
		OpenSubs(textsubs, false);
	}
	else{
		SAFE_DELETE(textsubs);
		OpenSubs(0, false);
	}
	vstate = Stopped;

	GetChapters(&videoWindow->chapters);

	if (Visual){
		Visual->SizeChanged(wxRect(backBufferRect.left, backBufferRect.top, backBufferRect.right, backBufferRect.bottom), lines, overlayFont, d3device);
	}
	return true;
}

bool DirectShowPlayer::OpenFile(wxString sFileName, bool vobsub)
{
	//MessageBeep(MB_ICONERROR);
	PTR(InitializeGraph(), _("Błąd inicjalizacji Direct Show"));


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
	HRESULT hr;
	CD2DVideoRender *renderer = new CD2DVideoRender(this, &hr);
	renderer->QueryInterface(IID_IBaseFilter, (void**)&frend.obj);
	HR(m_pGraph->AddFilter(frend.obj, L"Kainote Video Renderer"), _("Nie można dodać renderera wideo"));
	HR(CoCreateInstance(CLSID_DSoundRender, NULL, CLSCTX_INPROC, IID_IBaseFilter, (LPVOID *)&pAudioRenderer.obj), _("Nie można utworzyć instancji renderera dźwięku"));

	HR(m_pGraph->AddFilter(pAudioRenderer.obj, L"Direct Sound Renderer"), _("Nie można dodać renderera Direct Sound"));

	bool hasstream = false;

	if (vobsub){
		Selfdest<IBaseFilter> pVobsub;
		CoCreateInstance(CLSID_VobsubAutoload, NULL, CLSCTX_INPROC, IID_IBaseFilter, (LPVOID *)&pVobsub.obj);
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
		bool connected = false;
		while (S_OK == pEnum->Next(1, &pPin.obj, NULL))
		{
			if (SUCCEEDED(pPin->ConnectedTo(&tmpPin.obj)))
			{
				connected = true;
				break;
			}
		}
		if (!connected){ m_pGraph->RemoveFilter(pVobsub.obj); }

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
				pinfo.pFilter->QueryInterface(IID_IAMExtendedSeeking, (void**)&chaptersControl);
				hasstream = true;
				break;
			}
			if (info){ DeleteMediaType(info); }
			SAFE_RELEASE(spin.obj);
			SAFE_RELEASE(mtypes.obj);
		}
	}

	renderer->GetVidInfo(videoInfo);

	vstate = Stopped;
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
			KaiLog(_("Błąd interfejsu wyboru ścieżek"));
		}
	}
	hr = pSource->QueryInterface(IID_IAMExtendedSeeking, (void**)&chaptersControl);


	return true;
}


bool DirectShowPlayer::OpenSubs(wxString *textsubs, bool redraw, bool fromFile)
{
	wxCriticalSectionLocker lock(mutexRender);
	if (instance) csri_close(instance);
	instance = NULL;

	if (!textsubs) {
		if (redraw && datas){
			RecreateSurface();
		}
		hasDummySubs = true;
		return true;
	}

	if (hasVisualEdition && Visual->Visual == VECTORCLIP && Visual->dummytext){
		wxString toAppend = Visual->dummytext->Trim().AfterLast(L'\n');
		if (fromFile){
			OpenWrite ow(*textsubs, false);
			ow.PartFileWrite(toAppend);
			ow.CloseFile();
		}
		else{
			(*textsubs) << toAppend;
		}
	}

	hasDummySubs = !fromFile;

	wxScopedCharBuffer buffer = textsubs->mb_str(wxConvUTF8);
	int size = strlen(buffer);


	// Select renderer
	vobsub = csri_renderer_default();
	if (!vobsub){
		KaiLog(_("CSRI odmówiło posłuszeństwa."));
		delete textsubs;
		return false;
	}

	instance = (fromFile) ? csri_open_file(vobsub, buffer, NULL) : csri_open_mem(vobsub, buffer, size, NULL);
	if (!instance){
		KaiLog(_("Instancja VobSuba nie utworzyła się."));
		delete textsubs;
		return false;
	}

	if (!format || csri_request_fmt(instance, format)){
		KaiLog(_("CSRI nie obsługuje tego formatu."));
		csri_close(instance);
		instance = NULL;
		delete textsubs;
		return false;
	}

	if (redraw && datas){
		RecreateSurface();
	}

	delete textsubs;
	return true;
}

bool DirectShowPlayer::InitDX(bool reset)
{

	if (!reset){
		d3dobject = Direct3DCreate9(D3D_SDK_VERSION);
		PTR(d3dobject, _("Nie można utwożyć objektu Direct3D"));
	}
	else{
		SAFE_RELEASE(MainStream);
		SAFE_RELEASE(bars);
		SAFE_RELEASE(lines);
		SAFE_RELEASE(overlayFont);

#if byvertices
		SAFE_RELEASE(texture);
		SAFE_RELEASE(vertex);
#endif
		SAFE_RELEASE(dxvaProcessor);
		SAFE_RELEASE(dxvaService);
	}

	HRESULT hr;

	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.Windowed = TRUE;
	d3dpp.hDeviceWindow = hwnd;
	d3dpp.BackBufferWidth = windowRect.right;
	d3dpp.BackBufferHeight = windowRect.bottom;
	d3dpp.BackBufferCount = 1;
	d3dpp.SwapEffect = D3DSWAPEFFECT_COPY;//D3DSWAPEFFECT_DISCARD;//D3DSWAPEFFECT_COPY;//
	d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
	d3dpp.Flags = D3DPRESENTFLAG_VIDEO;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;//D3DPRESENT_INTERVAL_DEFAULT;

	if (reset){
		hr = d3device->Reset(&d3dpp);
		if (FAILED(hr)){
			KaiLog(_("Nie można zresetować Direct3D"));
			return false;
		}
	}
	else{
		hr = d3dobject->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd,
			D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED, &d3dpp, &d3device);//| D3DCREATE_FPU_PRESERVE
		if (FAILED(hr)){
			HR(d3dobject->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd,
				D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED, &d3dpp, &d3device), _("Nie można utworzyć urządzenia D3D9"));
		}
	}

	hr = d3device->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, TRUE);
	hr = d3device->SetRenderState(D3DRS_ANTIALIASEDLINEENABLE, TRUE);

	hr = d3device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	hr = d3device->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
	hr = d3device->SetRenderState(D3DRS_LIGHTING, FALSE);
	hr = d3device->SetRenderState(D3DRS_DITHERENABLE, TRUE);

	hr = d3device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	hr = d3device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	hr = d3device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	hr = d3device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	hr = d3device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	hr = d3device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_SPECULAR);

	hr = d3device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	hr = d3device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	hr = d3device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	HR(hr, _("Zawiodło któreś z ustawień DirectX"));

	D3DXMATRIX matOrtho;
	D3DXMATRIX matIdentity;

	D3DXMatrixOrthoOffCenterLH(&matOrtho, 0, windowRect.right, windowRect.bottom, 0, 0.0f, 1.0f);
	D3DXMatrixIdentity(&matIdentity);

	HR(d3device->SetTransform(D3DTS_PROJECTION, &matOrtho), _("Nie można ustawić macierzy projekcji"));
	HR(d3device->SetTransform(D3DTS_WORLD, &matIdentity), _("Nie można ustawić macierzy świata"));
	HR(d3device->SetTransform(D3DTS_VIEW, &matIdentity), _("Nie można ustawić macierzy widoku"));

#if byvertices
	hr = d3device->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	hr = d3device->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

	// Add filtering
	hr = d3device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	hr = d3device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	HR(hr, _("Zawiodło któreś z ustawień DirectX vertices"));
	HR(d3device->CreateTexture(vwidth, vheight, 1, D3DUSAGE_RENDERTARGET,
		D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &texture, NULL), "Nie można utworzyć tekstury");

	HR(texture->GetSurfaceLevel(0, &bars), "nie można utworzyć powierzchni");

	HR(d3device->CreateOffscreenPlainSurface(vwidth, vheight, d3dformat, D3DPOOL_DEFAULT, &MainStream, 0), "Nie można utworzyć powierzchni");

	HR(d3device->CreateVertexBuffer(4 * sizeof(CUSTOMVERTEX), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &vertex, NULL),
		"Nie można utworzyć bufora wertex")

		CUSTOMVERTEX* pVertices;
	HR(hr = vertex->Lock(0, 0, (void**)&pVertices, 0), "nie można zablokować bufora vertex");

	pVertices[0].position = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	pVertices[0].tu = 0.0f;
	pVertices[0].tv = 0.0f;
	pVertices[1].position = D3DXVECTOR3(vwidth, 0.0f, 0.0f);
	pVertices[1].tu = 1.0f;
	pVertices[1].tv = 0.0f;
	pVertices[2].position = D3DXVECTOR3(vwidth, vheight, 0.0f);
	pVertices[2].tu = 1.0f;
	pVertices[2].tv = 1.0f;
	pVertices[3].position = D3DXVECTOR3(0.0f, vheight, 0.0f);
	pVertices[3].tu = 0.0f;
	pVertices[3].tv = 1.0f;

	vertex->Unlock();
#endif


	HR(d3device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &bars), _("Nie można stworzyć powierzchni"));
	HR(DXVA2CreateVideoService(d3device, IID_IDirectXVideoProcessorService, (VOID**)&dxvaService), _("Nie można stworzyć DXVA processor service"));
	DXVA2_VideoDesc videoDesc;
	videoDesc.SampleWidth = vwidth;
	videoDesc.SampleHeight = vheight;
	videoDesc.SampleFormat.VideoChromaSubsampling = DXVA2_VideoChromaSubsampling_MPEG2;
	videoDesc.SampleFormat.NominalRange = DXVA2_NominalRange_0_255;
	videoDesc.SampleFormat.VideoTransferMatrix = DXVA2_VideoTransferMatrix_BT709;//EX_COLOR_INFO[g_ExColorInfo][0];
	videoDesc.SampleFormat.VideoLighting = DXVA2_VideoLighting_dim;
	videoDesc.SampleFormat.VideoPrimaries = DXVA2_VideoPrimaries_BT709;
	videoDesc.SampleFormat.VideoTransferFunction = DXVA2_VideoTransFunc_709;
	videoDesc.SampleFormat.SampleFormat = DXVA2_SampleProgressiveFrame;
	videoDesc.Format = D3DFMT_X8R8G8B8;
	videoDesc.InputSampleFreq.Numerator = 60;
	videoDesc.InputSampleFreq.Denominator = 1;
	videoDesc.OutputFrameFreq.Numerator = 60;
	videoDesc.OutputFrameFreq.Denominator = 1;

	UINT count, count1;//, count2;
	GUID* guids = NULL;

	HR(dxvaService->GetVideoProcessorDeviceGuids(&videoDesc, &count, &guids), _("Nie moźna pobrać GUIDów DXVA"));
	D3DFORMAT* formats = NULL;
	//D3DFORMAT* formats2 = NULL;
	bool isgood = false;
	GUID dxvaGuid;
	DXVA2_VideoProcessorCaps DXVAcaps;
	for (UINT i = 0; i < count; i++){
		hr = dxvaService->GetVideoProcessorRenderTargets(guids[i], &videoDesc, &count1, &formats);
		if (FAILED(hr)){ KaiLog(_("Nie można uzyskać formatów DXVA")); continue; }
		for (UINT j = 0; j < count1; j++)
		{
			if (formats[j] == D3DFMT_X8R8G8B8)
			{
				isgood = true; //break;
			}

		}

		CoTaskMemFree(formats);
		if (!isgood){ KaiLog(_("Ten format nie jest obsługiwany przez DXVA")); continue; }
		isgood = false;

		hr = dxvaService->GetVideoProcessorCaps(guids[i], &videoDesc, D3DFMT_X8R8G8B8, &DXVAcaps);
		if (FAILED(hr)){ KaiLog(_("GetVideoProcessorCaps zawiodło")); continue; }
		if (DXVAcaps.NumForwardRefSamples > 0 || DXVAcaps.NumBackwardRefSamples > 0)
		{
			/*wLogStatus(L"NumForwardRefSamples albo NumBackwardRefSample jest większe od zera");*/continue;
		}

		//if(DXVAcaps.DeviceCaps!=4){continue;}//DXVAcaps.InputPool
		hr = dxvaService->CreateSurface(vwidth, vheight, 0, d3dformat, D3DPOOL_DEFAULT, 0, DXVA2_VideoSoftwareRenderTarget, &MainStream, NULL);
		if (FAILED(hr)){ KaiLog(wxString::Format(_("Nie można stworzyć powierzchni DXVA %i"), (int)i)); continue; }

		hr = dxvaService->CreateVideoProcessor(guids[i], &videoDesc, D3DFMT_X8R8G8B8, 0, &dxvaProcessor);
		if (FAILED(hr)){ KaiLog(_("Nie można stworzyć processora DXVA")); continue; }
		dxvaGuid = guids[i]; isgood = true;
		break;
	}
	CoTaskMemFree(guids);
	PTR(isgood, "Nie ma żadnych guidów");




#ifndef byvertices
	HR(d3device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &bars), _("Nie można stworzyć powierzchni"));

	HR(d3device->CreateOffscreenPlainSurface(vwidth, vheight, d3dformat, D3DPOOL_DEFAULT, &MainStream, 0), _("Nie można stworzyć plain surface"));//D3DPOOL_DEFAULT
#endif


	HR(D3DXCreateLine(d3device, &lines), _("Nie można stworzyć linii D3DX"));
	HR(D3DXCreateFont(d3device, 20, 0, FW_BOLD, 0, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, TEXT("Tahoma"), &overlayFont), _("Nie można stworzyć czcionki D3DX"));

	return true;
}

//w stosuj false tylko w przypadku gdy odświeżasz coś namalowanego na wideo, 
//w reszcie przypadków ma być pełne odświeżanie klatki

void DirectShowPlayer::Render(bool Frame)
{
	wxCriticalSectionLocker lock(mutexRender);
	resized = false;
	HRESULT hr = S_OK;

	if (devicelost)
	{
		if (FAILED(hr = d3device->TestCooperativeLevel()))
		{
			if (D3DERR_DEVICELOST == hr ||
				D3DERR_DRIVERINTERNALERROR == hr){
				//wxLogMessage("cooperative level device lost");
				return;
			}

			if (D3DERR_DEVICENOTRESET == hr)
			{
				Clear();
				InitDX();
				RecreateSurface(); 
				if (Visual){
					Visual->SizeChanged(wxRect(backBufferRect.left, backBufferRect.top, backBufferRect.right, backBufferRect.bottom), lines, overlayFont, d3device);
				}
				devicelost = false;
				Render(true);
				return;
			}
			return;
		}
		devicelost = false;
	}

	hr = d3device->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);

	DXVA2_VideoProcessBltParams blt = { 0 };
	DXVA2_VideoSample samples = { 0 };
	LONGLONG start_100ns = time * 10000;
	LONGLONG end_100ns = start_100ns + 170000;
	blt.TargetFrame = start_100ns;
	blt.TargetRect = windowRect;

	// DXVA2_VideoProcess_Constriction
	blt.ConstrictionSize.cx = windowRect.right - windowRect.left;
	blt.ConstrictionSize.cy = windowRect.bottom - windowRect.top;
	DXVA2_AYUVSample16 color;

	color.Cr = 0x8000;
	color.Cb = 0x8000;
	color.Y = 0x0F00;
	color.Alpha = 0xFFFF;
	blt.BackgroundColor = color;

	// DXVA2_VideoProcess_YUV2RGBExtended
	blt.DestFormat.VideoChromaSubsampling = DXVA2_VideoChromaSubsampling_Unknown;
	blt.DestFormat.NominalRange = DXVA2_NominalRange_0_255;//EX_COLOR_INFO[g_ExColorInfo][1];
	blt.DestFormat.VideoTransferMatrix = DXVA2_VideoTransferMatrix_BT709;
	blt.DestFormat.VideoLighting = DXVA2_VideoLighting_dim;
	blt.DestFormat.VideoPrimaries = DXVA2_VideoPrimaries_BT709;
	blt.DestFormat.VideoTransferFunction = DXVA2_VideoTransFunc_709;

	blt.DestFormat.SampleFormat = DXVA2_SampleProgressiveFrame;
	// Initialize main stream video sample.
	//
	samples.Start = start_100ns;
	samples.End = end_100ns;

	// DXVA2_VideoProcess_YUV2RGBExtended
	samples.SampleFormat.VideoChromaSubsampling = DXVA2_VideoChromaSubsampling_MPEG2;
	samples.SampleFormat.NominalRange = DXVA2_NominalRange_0_255;
	samples.SampleFormat.VideoTransferMatrix = DXVA2_VideoTransferMatrix_BT709;//EX_COLOR_INFO[g_ExColorInfo][0];
	samples.SampleFormat.VideoLighting = DXVA2_VideoLighting_dim;
	samples.SampleFormat.VideoPrimaries = DXVA2_VideoPrimaries_BT709;
	samples.SampleFormat.VideoTransferFunction = DXVA2_VideoTransFunc_709;

	samples.SampleFormat.SampleFormat = DXVA2_SampleProgressiveFrame;

	samples.SrcSurface = MainStream;

	samples.SrcRect = mainStreamRect;

	samples.DstRect = backBufferRect;

	// DXVA2_VideoProcess_PlanarAlpha
	samples.PlanarAlpha = DXVA2_Fixed32OpaqueAlpha();

	hr = dxvaProcessor->VideoProcessBlt(bars, &blt, &samples, 1, NULL);


	hr = d3device->BeginScene();

#if byvertices


	// Render the vertex buffer contents
	hr = d3device->SetStreamSource(0, vertex, 0, sizeof(CUSTOMVERTEX));
	hr = d3device->SetVertexShader(NULL);
	hr = d3device->SetFVF(D3DFVF_CUSTOMVERTEX);
	hr = d3device->SetTexture(0, texture);
	hr = d3device->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);
#endif

	if (Visual){ Visual->Draw(time); }

	if (cross){
		DRAWOUTTEXT(overlayFont, coords, crossRect, (crossRect.left < vectors[0].x) ? 10 : 8, 0xFFFFFFFF)
			hr = lines->SetWidth(3);
		hr = lines->Begin();
		hr = lines->Draw(&vectors[0], 2, 0xFF000000);
		hr = lines->Draw(&vectors[2], 2, 0xFF000000);
		hr = lines->End();
		hr = lines->SetWidth(1);
		D3DXVECTOR2 v1[4];
		v1[0] = vectors[0];
		v1[0].x += 0.5f;
		v1[1] = vectors[1];
		v1[1].x += 0.5f;
		v1[2] = vectors[2];
		v1[2].y += 0.5f;
		v1[3] = vectors[3];
		v1[3].y += 0.5f;
		hr = lines->Begin();
		hr = lines->Draw(&v1[0], 2, 0xFFFFFFFF);
		hr = lines->Draw(&v1[2], 2, 0xFFFFFFFF);
		hr = lines->End();
	}

	if (pbar){
		DRAWOUTTEXT(overlayFont, pbtime, progressBarRect, DT_LEFT | DT_TOP, 0xFFFFFFFF)
			hr = lines->SetWidth(1);
		hr = lines->Begin();
		hr = lines->Draw(&vectors[4], 5, 0xFF000000);
		hr = lines->Draw(&vectors[9], 5, 0xFFFFFFFF);
		hr = lines->End();
		hr = lines->SetWidth(7);
		hr = lines->Begin();
		hr = lines->Draw(&vectors[14], 2, 0xFFFFFFFF);
		hr = lines->End();
	}
	if (hasZoom){ DrawZoom(); }
	// End the scene
	hr = d3device->EndScene();
	hr = d3device->Present(NULL, &windowRect, NULL, NULL);
	if (D3DERR_DEVICELOST == hr ||
		D3DERR_DRIVERINTERNALERROR == hr){
		if (!devicelost){
			devicelost = true;
		}
		Render(true);
	}

}

bool DirectShowPlayer::Play(int end)
{
	SetThreadExecutionState(ES_DISPLAY_REQUIRED | ES_CONTINUOUS);
	VideoCtrl *vb = ((VideoCtrl*)this);
	if (!(vb->IsShown() || (vb->TD && vb->TD->IsShown()))){ return false; }
	TabPanel* tab = (TabPanel*)vb->GetParent();
	if (hasVisualEdition){
		wxString *txt = tab->Grid->SaveText();
		OpenSubs(txt, false, true);
		SAFE_DELETE(Visual->dummytext);
		hasVisualEdition = false;
	}
	else if (hasDummySubs && tab->editor){
		OpenSubs(tab->Grid->SaveText(), false, true);
	}

	if (end > 0)
		playend = end;
	else
		playend = 0;

	if (time < GetDuration() - frameDuration){

		HRESULT hr = S_OK;
		if (!m_pGraph){ return; }
		if (vstate == Paused || vstate == Stopped)
		{

			hr = m_pControl->Run();
			if (SUCCEEDED(hr))
			{
				vstate = Playing;

			}
		}
		return true;
	}
	else
		return false;
}

bool DirectShowPlayer::Pause(bool skipWhenOnEnd)
{
	HRESULT hr;
	if (vstate == Playing){
		hr = m_pControl->Pause();
		if (SUCCEEDED(hr))
		{
			SetThreadExecutionState(ES_CONTINUOUS);
			vstate = Paused;
		}
	}
	else {
		if (time >= GetDuration() && skipWhenOnEnd){ return false; }
		Play();
	}
	return true;

}

bool DirectShowPlayer::Stop()
{
	HRESULT hr;
	if (vstate == Playing){
		SetThreadExecutionState(ES_CONTINUOUS);
		vstate = Stopped;
		if (m_pControl){
			hr = m_pControl->Stop();

			if (SUCCEEDED(hr))
			{

				vstate = Stopped;
			}
		}
		playend = 0;
		time = 0;
		return true;
	}
	return false;
}



//-----------------------------------------------------------------------------
// DShowPlayer::SetPosition
// Description: Seeks to a new position.
//-----------------------------------------------------------------------------

void DirectShowPlayer::SetPosition(int _time, bool starttime/*=true*/, bool corect/*=true*/)
{
	bool playing = vstate == Playing;
	TabPanel* tab = (TabPanel*)videoWindow->GetParent();
	time = MID(0, _time, GetDuration());
	if (corect){
		time /= frameDuration;
		if (starttime){ time++; }
		time *= frameDuration;
	}
	//albo to przypadek albo ustawianie pozycji przed ustawianiem clipów jest rozwiązaniem dość częstego krasza
	//przy wielu plikach jednocześnie, był zawsze po seekingu
	playend = 0;
	seek = true;

	//dshow section
	if (m_pControl == NULL || m_pSeek == NULL)
	{
		return;
	}

	HRESULT hr = S_OK;
	LONGLONG ppos = _time;
	ppos *= 10000;


	hr = m_pSeek->SetPositions(&ppos, AM_SEEKING_AbsolutePositioning,
		NULL, AM_SEEKING_NoPositioning);

	if (SUCCEEDED(hr))
	{
		// If playback is stopped, we need to put the graph into the paused
		// state to update the video renderer with the new frame, and then stop
		// the graph again. The IMediaControl::StopWhenReady does this.
		if (vstate == Stopped)
		{
			hr = m_pControl->StopWhenReady();
		}
	}
	//end of dshow section
	if (hasVisualEdition){
		SAFE_DELETE(Visual->dummytext);
		if (Visual->Visual == VECTORCLIP){
			Visual->SetClip(Visual->GetVisual(), true, false, false);
		}
		else{
			OpenSubs((playing) ? tab->Grid->SaveText() : tab->Grid->GetVisible(), true, playing);
			if (vstate == Playing){ hasVisualEdition = false; }
		}
	}
	else if (hasDummySubs && tab->editor){
		OpenSubs((playing) ? tab->Grid->SaveText() : tab->Grid->GetVisible(), true, playing);
	}
}


//-----------------------------------------------------------------------------
// DShowPlayer::GetCurrentPosition
// Description: Gets the current playback position.
//-----------------------------------------------------------------------------

int DirectShowPlayer::GetPosition()
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

bool DirectShowPlayer::InitializeGraph()
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

void DirectShowPlayer::TearDownGraph()
{
	if (m_pControl && vstate != Stopped)
	{
		m_pControl->Stop();
	}

	SAFE_RELEASE(stream);
	SAFE_RELEASE(chaptersControl);
	SAFE_RELEASE(m_pControl);
	SAFE_RELEASE(m_pBA);

	SAFE_RELEASE(m_pSeek);

	SAFE_RELEASE(m_pGraph);

	vstate = None;
}



wxSize DirectShowPlayer::GetVideoSize()
{
	return wxSize(videoInfo.width, videoInfo.height);
}

int DirectShowPlayer::GetDuration()
{
	LONGLONG dur = 0;
	if (m_pSeek)
	{
		m_pSeek->GetDuration(&dur);
		return dur / 10000;
	}
	return 0;
}

void DirectShowPlayer::SetVolume(int volume)
{
	HRESULT hr;
	if (m_pBA)
	{
		hr = m_pBA->put_Volume(volume);
		if (FAILED(hr)){ KaiLog(L"Cannot set volume."); }
	}

}

int DirectShowPlayer::GetVolume()
{
	HRESULT hr;
	long volume;
	if (m_pBA)
	{
		hr = m_pBA->get_Volume(&volume);
		if (FAILED(hr)){ KaiLog(L"Cannot get volume."); }
	}
	return volume;
}

void DirectShowPlayer::GetFpsnRatio(float *fps, long *arx, long *ary)
{
	if (arx){ *arx = videoInfo.ARatioX; }
	if (ary){ *ary = videoInfo.ARatioY; }
	if (fps){ *fps = videoInfo.fps; }
}


wxArrayString DirectShowPlayer::GetStreams()
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
			streamnames.Add(wxString(text) + " " + enable);
			CoTaskMemFree(text);
		}
	}
	return streamnames;
}

void DirectShowPlayer::GetChapters(std::vector<chapter> *chaps)
{
	if (!chaptersControl || !chaps){ return; }
	long mcount = 0;
	if (FAILED(chaptersControl->get_MarkerCount(&mcount))){ return; }
	chaps->clear();

	for (long i = 1; i <= mcount; i++)
	{
		LPWSTR string;
		double time = 0;
		if (FAILED(chaptersControl->GetMarkerName(i, &string))){ continue; }
		chaptersControl->GetMarkerTime(i, &time);
		chapter ch;
		ch.name = wxString(string);
		ch.time = time * 1000;
		chaps->push_back(ch);
		SysFreeString(string);
	}
}

bool DirectShowPlayer::EnumFilters(Menu *menu)
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



bool DirectShowPlayer::FilterConfig(wxString name, int idx, wxPoint pos)
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
		OleCreatePropertyFrame(videoWindow->GetHWND(), pos.x, pos.y, name.wc_str(), 1,
			(IUnknown**)&lpUnk, caGUID.cElems, caGUID.pElems, 0, 0, 0);
	}
	catch (...)
	{
	}
	if (caGUID.pElems) CoTaskMemFree(caGUID.pElems);
	return true;
}

void DirectShowPlayer::RecreateSurface()
{
	int all = vheight * pitch;
	char *cpy = new char[all];
	byte *cpy1 = (byte*)cpy;
	byte *data1 = (byte*)datas;
	memcpy(cpy1, data1, all);
	DrawTexture(cpy1);
	delete[] cpy;
}

void DirectShowPlayer::ChangePositionByFrame(int step)
{
	if (vstate == Playing){ return; }

	time += (frameDuration * step);
	SetPosition(time, true, false);
	videoWindow->RefreshTime();
}

int DirectShowPlayer::GetFrameTime(bool start)
{

	int halfFrame = (start) ? -(frameDuration / 2.0f) : (frameDuration / 2.0f) + 1;
	return time + halfFrame;

}

void DirectShowPlayer::GetStartEndDelay(int startTime, int endTime, int *retStart, int *retEnd)
{
	if (!retStart || !retEnd){ return; }

	int frameStartTime = (((float)startTime / 1000.f) * fps);
	int frameEndTime = (((float)endTime / 1000.f) * fps);
	frameStartTime++;
	frameEndTime++;
	*retStart = (((frameStartTime * 1000) / fps) + 0.5f) - startTime;
	*retEnd = (((frameEndTime * 1000) / fps) + 0.5f) - endTime;

}

int DirectShowPlayer::GetFrameTimeFromTime(int _time, bool start)
{

	int halfFrame = (start) ? -(frameDuration / 2.0f) : (frameDuration / 2.0f) + 1;
	return _time + halfFrame;

}

int DirectShowPlayer::GetFrameTimeFromFrame(int frame, bool start)
{

	int halfFrame = (start) ? -(frameDuration / 2.0f) : (frameDuration / 2.0f) + 1;
	return (frame * (1000.f / fps)) + halfFrame;

}

int DirectShowPlayer::GetPlayEndTime(int _time)
{

	int newTime = _time;
	newTime /= frameDuration;
	newTime = (newTime * frameDuration) + 1.f;
	if (_time == newTime && newTime % 10 == 0){ newTime -= 5; }
	return newTime;

}

void DirectShowPlayer::EnableStream(long index)
{
	if (stream){
		seek = true;
		auto hr = stream->Enable(index, AMSTREAMSELECTENABLE_ENABLE);
		if (FAILED(hr)){
			KaiLog("Cannot change stream");
		}
	}
}

void DirectShowPlayer::ChangeVobsub(bool vobsub)
{
	int tmptime = time;
	TabPanel *pan = (TabPanel *)videoWindow->GetParent();
	OpenSubs((vobsub) ? NULL : pan->Grid->SaveText(), true, true);
	OpenFile(pan->VideoPath, vobsub);
	vformat = videoInfo.CT;
	D3DFORMAT tmpd3dformat = (vformat == 5) ? D3DFORMAT('21VN') : (vformat == 3) ? D3DFORMAT('21VY') : (vformat == 2) ? D3DFMT_YUY2 : D3DFMT_X8R8G8B8;
	if (tmpd3dformat != d3dformat){
		d3dformat = tmpd3dformat;
		int tmppitch = vwidth * videoInfo.bytes;
		if (tmppitch != pitch){
			pitch = tmppitch;
			if (datas){ delete[] datas; datas = NULL; }
			datas = new byte[vheight * pitch];
		}
		UpdateVideoWindow();
	}
	SetPosition(tmptime);
	if (vstate == Paused){ Play(); Pause(); }
	else if (vstate == Playing){ Play(); }
	int pos = videoWindow->volslider->GetValue();
	SetVolume(-(pos * pos));
	videoWindow->ChangeStream();
}

byte *DirectShowPlayer::GetFramewithSubs(bool subs, bool *del)
{
	bool dssubs = (subs && Notebook::GetTab()->editor);

	byte *cpy1;
	byte bytes = (vformat == RGB32) ? 4 : (vformat == YUY2) ? 2 : 1;
	int all = vheight*pitch;
	if (dssubs){
		*del = true;
		byte *cpy = new byte[all];
		cpy1 = cpy;
	}
	else{ *del = false; }
	if (instance && dssubs){
		byte *data1 = datas;
		memcpy(cpy1, data1, all);
		framee->strides[0] = vwidth * bytes;
		framee->planes[0] = cpy1;
		csri_render(instance, framee, (time / 1000.0));
	}
	return (dssubs) ? cpy1 : datas;
}