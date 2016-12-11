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


#include "Visuals.h"
#include "Videobox.h"
#include "VideoRenderer.h"
#include "DShowPlayer.h"
#include "KainoteApp.h"

#include <Dvdmedia.h>
#include "Vsfilterapi.h"



#if byvertices
struct CUSTOMVERTEX
{
	D3DXVECTOR3 position; // The position
	FLOAT       tu, tv;   // The texture coordinates
};
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_TEX1)
#endif
#if DXVA
#pragma comment(lib, "Dxva2.lib")
const IID IID_IDirectXVideoProcessorService ={ 0xfc51a552,0xd5e7,0x11d9,{0xaf,0x55,0x00,0x05,0x4e,0x43,0xff,0x02}};
#endif

void CreateVERTEX (VERTEX *v, float X, float Y, D3DCOLOR Color, float Z)
{	
	v->fX = X;	
	v->fY = Y;	
	v->fZ = Z;		
	v->Color = Color;	
}

VideoRend::VideoRend(wxWindow *_parent, const wxSize &size)
	:wxWindow(_parent,-1,wxDefaultPosition, size)//wxFULL_REPAINT_ON_RESIZE
	,panelHeight(66)
	,AR(0.0)
	,fps(0.0)
{
	hwnd=GetHWND();

	//---------------------------- format
	d3dformat=D3DFORMAT('21VN');
	//-----------------------------------
	vformat=NV12;
	time=0;
	playend=0;
	vstate=None;
	d3dobject=NULL;
	d3device=NULL;
	bars=NULL;
	VFF=NULL;
	instance=NULL;
	vobsub=NULL;
	framee=NULL;
	format=NULL;
	lines=NULL;
	Vclips=NULL;
	resized=seek=block=cross=pbar=VisEdit=false;
	IsDshow=true;
	devicelost=false;
	MainStream=NULL;
	datas=NULL;
	player=NULL;
	vplayer=NULL;
	rt3.bottom=0;
	rt3.right=0;
	rt3.left=0;
	rt3.top=0;
	m_font=NULL;
	lastframe=0;
	diff=0;
	avframetime=42;
#if byvertices
	vertex=NULL;
	texture=NULL;
#endif
#if DXVA
	dxvaProcessor=NULL;
	dxvaService=NULL;
#endif
}

bool VideoRend::InitDX(bool reset)
{

	if(!reset){
		d3dobject = Direct3DCreate9( D3D_SDK_VERSION );
		PTR(d3dobject,_("Nie można utwożyć objektu Direct3D"));
	}else{
		SAFE_RELEASE(MainStream);
		SAFE_RELEASE(bars);
		SAFE_RELEASE(lines);
		SAFE_RELEASE(m_font);
#if byvertices
		SAFE_RELEASE(texture);
		SAFE_RELEASE(vertex);
#endif
#if DXVA
		SAFE_RELEASE(dxvaProcessor);
		SAFE_RELEASE(dxvaService);
#endif
	}

	HRESULT hr;

	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory( &d3dpp, sizeof(d3dpp) );
	d3dpp.Windowed               = TRUE;
	d3dpp.hDeviceWindow          = hwnd;
	d3dpp.BackBufferWidth        = rt3.right;
	d3dpp.BackBufferHeight       = rt3.bottom;
	d3dpp.BackBufferCount	     = 1;
	d3dpp.SwapEffect			 = D3DSWAPEFFECT_COPY;//D3DSWAPEFFECT_DISCARD;//D3DSWAPEFFECT_COPY;//
	d3dpp.BackBufferFormat       = D3DFMT_X8R8G8B8;
	d3dpp.Flags					 = D3DPRESENTFLAG_VIDEO;
	//d3dpp.PresentationInterval   = D3DPRESENT_INTERVAL_DEFAULT;

	if(reset){
		hr=d3device->Reset(&d3dpp);
		if(FAILED(hr)){wxLogStatus(_("Nie można zresetować Direct3D"));}
	}else{
		hr=d3dobject->CreateDevice(D3DADAPTER_DEFAULT,D3DDEVTYPE_HAL,hwnd,
			D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED , &d3dpp, &d3device);//| D3DCREATE_FPU_PRESERVE
		if(FAILED(hr)){
			HR (d3dobject->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd,
				D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED,&d3dpp, &d3device ), _("Nie można utworzyć urządzenia D3D9")); 
		} 
	}

	hr = d3device->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, TRUE);
	hr = d3device->SetRenderState(D3DRS_ANTIALIASEDLINEENABLE, TRUE);

	hr = d3device->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE);
	hr = d3device->SetRenderState( D3DRS_ZENABLE, D3DZB_FALSE);
	hr = d3device->SetRenderState( D3DRS_LIGHTING, FALSE);
	hr = d3device->SetRenderState( D3DRS_DITHERENABLE, TRUE);

	hr = d3device->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE);
	hr = d3device->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	hr = d3device->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	hr = d3device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	hr = d3device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	hr = d3device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_SPECULAR);

	hr = d3device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	hr = d3device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	hr = d3device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	HR(hr,_("Zawiodło któreś z ustawień DirectX"));

	D3DXMATRIX matOrtho; 
	D3DXMATRIX matIdentity;

	D3DXMatrixOrthoOffCenterLH(&matOrtho, 0, rt3.right, rt3.bottom, 0, 0.0f, 1.0f);
	D3DXMatrixIdentity(&matIdentity);

	HR(d3device->SetTransform(D3DTS_PROJECTION, &matOrtho), _("Nie można ustawić macierzy porojekcji"));
	HR(d3device->SetTransform(D3DTS_WORLD, &matIdentity), _("Nie można ustawić macierzy świata"));
	HR(d3device->SetTransform(D3DTS_VIEW, &matIdentity), _("Nie można ustawić macierzy widoku"));

#if byvertices
	hr = d3device->SetSamplerState( 0, D3DSAMP_ADDRESSU,  D3DTADDRESS_CLAMP );
	hr = d3device->SetSamplerState( 0, D3DSAMP_ADDRESSV,  D3DTADDRESS_CLAMP );

	// Add filtering
	hr = d3device->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
	hr = d3device->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
	HR(hr,_("Zawiodło któreś z ustawień DirectX vertices"));
	HR(d3device->CreateTexture(vwidth, vheight, 1, D3DUSAGE_RENDERTARGET,
		D3DFMT_X8R8G8B8,D3DPOOL_DEFAULT,&texture, NULL), "Nie można utworzyć tekstury" );

	HR(texture->GetSurfaceLevel(0, &bars), "nie można utworzyć powierzchni");

	HR(d3device->CreateOffscreenPlainSurface(vwidth,vheight,d3dformat, D3DPOOL_DEFAULT,&MainStream,0),"Nie można utworzyć powierzchni");

	HR(d3device->CreateVertexBuffer( 4*sizeof(CUSTOMVERTEX),D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, D3DFVF_CUSTOMVERTEX,D3DPOOL_DEFAULT, &vertex, NULL ),
		"Nie można utworzyć bufora wertex")

		CUSTOMVERTEX* pVertices;
	HR ( hr = vertex->Lock( 0, 0, (void**)&pVertices, 0 ), "nie można zablokować bufora vertex" ); 

	pVertices[0].position = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	pVertices[0].tu       = 0.0f;
	pVertices[0].tv       = 0.0f; 
	pVertices[1].position = D3DXVECTOR3(vwidth, 0.0f, 0.0f);
	pVertices[1].tu       = 1.0f;
	pVertices[1].tv       = 0.0f;
	pVertices[2].position = D3DXVECTOR3(vwidth, vheight, 0.0f);
	pVertices[2].tu       = 1.0f;
	pVertices[2].tv       = 1.0f; 
	pVertices[3].position = D3DXVECTOR3(0.0f, vheight, 0.0f);
	pVertices[3].tu       = 0.0f;
	pVertices[3].tv       = 1.0f;

	vertex->Unlock();


	//if (d3dformat != ddsd.Format) {
	//wxLogStatus("Textura ma niewłaściwy format"); return false;	
	//}
#elif DXVA
	HR (d3device->GetBackBuffer(0,0, D3DBACKBUFFER_TYPE_MONO, &bars),L"Nie można stworzyć powierzchni");
	HR (DXVA2CreateVideoService(d3device, IID_IDirectXVideoProcessorService, (VOID**)&dxvaService),L"Nie można stworzyć DXVA processor service");
	//wxLogStatus("wymiary");

	//wxLogStatus("pobrane");
	videoDesc.SampleWidth                         = vwidth;
	videoDesc.SampleHeight                        = vheight;
	videoDesc.SampleFormat.VideoChromaSubsampling = DXVA2_VideoChromaSubsampling_MPEG2;
	videoDesc.SampleFormat.NominalRange           = DXVA2_NominalRange_0_255;
	videoDesc.SampleFormat.VideoTransferMatrix    = DXVA2_VideoTransferMatrix_BT709;//EX_COLOR_INFO[g_ExColorInfo][0];
	videoDesc.SampleFormat.VideoLighting          = DXVA2_VideoLighting_dim;
	videoDesc.SampleFormat.VideoPrimaries         = DXVA2_VideoPrimaries_BT709;
	videoDesc.SampleFormat.VideoTransferFunction  = DXVA2_VideoTransFunc_709;
	videoDesc.SampleFormat.SampleFormat           = DXVA2_SampleProgressiveFrame;
	videoDesc.Format                              = D3DFMT_X8R8G8B8;
	videoDesc.InputSampleFreq.Numerator           = 60;
	videoDesc.InputSampleFreq.Denominator         = 1;
	videoDesc.OutputFrameFreq.Numerator           = 60;
	videoDesc.OutputFrameFreq.Denominator         = 1;

	UINT count, count1;//, count2;
	GUID* guids = NULL;
	//wxLogStatus("desc");

	HR(dxvaService->GetVideoProcessorDeviceGuids(&videoDesc, &count, &guids),L"Nie moźna pobrać GUIDów DXVA");
	D3DFORMAT* formats = NULL;
	//D3DFORMAT* formats2 = NULL;
	bool isgood=false;
	DXVA2_VideoProcessorCaps DXVAcaps;
	for(UINT i=0; i<count;i++){
		//wxLogMessage("guid: %i",(int)i);
		hr=dxvaService->GetVideoProcessorRenderTargets(guids[i], &videoDesc, &count1, &formats);
		if(FAILED(hr)){wxLogStatus(L"Nie można uzyskać formatów DXVA");continue;}
		for (UINT j = 0; j < count1; j++)
		{
			if (formats[j] == D3DFMT_X8R8G8B8)
			{
				isgood=true; //break;
			}

		}

		CoTaskMemFree(formats);
		if(!isgood){ wxLogStatus(L"Format ten nie jest obsługiwany przez DXVA");continue;}
		isgood=false;
		
		hr=dxvaService->GetVideoProcessorCaps(guids[i], &videoDesc, D3DFMT_X8R8G8B8, &DXVAcaps);
		if(FAILED(hr)){wxLogStatus(L"GetVideoProcessorCaps zawiodło");continue;}
		if (DXVAcaps.NumForwardRefSamples > 0 || DXVAcaps.NumBackwardRefSamples > 0)
		{
			wxLogStatus(L"NumForwardRefSamples albo NumBackwardRefSample jest większe od zera");continue;
		}

		//if(DXVAcaps.DeviceCaps!=4){continue;}//DXVAcaps.InputPool
		hr = dxvaService->CreateSurface(vwidth,vheight, 0, d3dformat, D3DPOOL_DEFAULT, 0, DXVA2_VideoSoftwareRenderTarget, &MainStream, NULL);
		if(FAILED(hr)){wxLogStatus(L"Nie można stworzyć powierzchni dxva %i", (int)i);continue;}

		hr = dxvaService->CreateVideoProcessor(guids[i], &videoDesc,D3DFMT_X8R8G8B8,0,&dxvaProcessor);
		if(FAILED(hr)){wxLogStatus(L"Nie można stworzyć dxva processora");continue;}
		dxvaGuid=guids[i];isgood=true;
		break;
	}
	CoTaskMemFree(guids);
	PTR(isgood,"Nie ma żadnych guidów");



#else
	HR (d3device->GetBackBuffer(0,0, D3DBACKBUFFER_TYPE_MONO, &bars),_("Nie można stworzyć powierzchni"));

	HR (d3device->CreateOffscreenPlainSurface(vwidth,vheight,d3dformat, D3DPOOL_DEFAULT,&MainStream,0), _("Nie można stworzyć plain surface"));//D3DPOOL_DEFAULT
#endif
	HR(D3DXCreateLine(d3device, &lines), _("Nie można stworzyć linii D3DX"));
	HR(D3DXCreateFont(d3device, 20, 0, FW_BOLD, 0, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, TEXT("Tahoma"), &m_font ), _("Nie można stworzyć czcionki D3DX"));

	return true;
}
//w stosuj false tylko w przypadku gdy odświeżasz coś namalowanego na wideo, 
//w reszcie przypadków ma być pełne odświeżanie klatki

void VideoRend::Render(bool Frame)
{
	//wxLogStatus("render");
	if(Frame && !IsDshow){VFF->Refresh();resized=false; return;}
	wxMutexLocker lock(mutexRender);
	HRESULT hr = S_OK;

	if( devicelost )
	{
		if( FAILED( hr = d3device->TestCooperativeLevel() ) )
		{
			if( D3DERR_DEVICELOST == hr ||
				D3DERR_DRIVERINTERNALERROR == hr )
				return;

			if( D3DERR_DEVICENOTRESET == hr )
			{
				Clear();
				InitDX();
			}
			return;
		}

		devicelost = false;
	}

	hr = d3device->Clear( 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0,0,0), 1.0f, 0 );

#if DXVA
	DXVA2_VideoProcessBltParams blt={0};
	DXVA2_VideoSample samples={0};
	LONGLONG start_100ns = time*10000;
	LONGLONG end_100ns   = start_100ns + 170000;
	blt.TargetFrame = start_100ns;
	blt.TargetRect  = rt3;

	// DXVA2_VideoProcess_Constriction
	blt.ConstrictionSize.cx = rt3.right - rt3.left;
	blt.ConstrictionSize.cy = rt3.bottom - rt3.top;
	DXVA2_AYUVSample16 color;

	color.Cr    = 0x8000;
	color.Cb    = 0x8000;
	color.Y     = 0x0F00;
	color.Alpha = 0xFFFF;
	blt.BackgroundColor = color;

	// DXVA2_VideoProcess_YUV2RGBExtended
	blt.DestFormat.VideoChromaSubsampling = DXVA2_VideoChromaSubsampling_Unknown;
	blt.DestFormat.NominalRange           = DXVA2_NominalRange_0_255;//EX_COLOR_INFO[g_ExColorInfo][1];
	blt.DestFormat.VideoTransferMatrix    = DXVA2_VideoTransferMatrix_BT709;
	blt.DestFormat.VideoLighting          = DXVA2_VideoLighting_dim;
	blt.DestFormat.VideoPrimaries         = DXVA2_VideoPrimaries_BT709;
	blt.DestFormat.VideoTransferFunction  = DXVA2_VideoTransFunc_709;

	blt.DestFormat.SampleFormat = DXVA2_SampleProgressiveFrame;
	// Initialize main stream video sample.
	//
	samples.Start = start_100ns;
	samples.End   = end_100ns;

	// DXVA2_VideoProcess_YUV2RGBExtended
	samples.SampleFormat.VideoChromaSubsampling = DXVA2_VideoChromaSubsampling_MPEG2;
	samples.SampleFormat.NominalRange           = DXVA2_NominalRange_0_255;
	samples.SampleFormat.VideoTransferMatrix    = DXVA2_VideoTransferMatrix_BT709;//EX_COLOR_INFO[g_ExColorInfo][0];
	samples.SampleFormat.VideoLighting          = DXVA2_VideoLighting_dim;
	samples.SampleFormat.VideoPrimaries         = DXVA2_VideoPrimaries_BT709;
	samples.SampleFormat.VideoTransferFunction  = DXVA2_VideoTransFunc_709;

	samples.SampleFormat.SampleFormat = DXVA2_SampleProgressiveFrame;

	samples.SrcSurface = MainStream;

	// DXVA2_VideoProcess_SubRects
	//RECT srcrect;
	//srcrect.bottom=vheight;
	//srcrect.right=vwidth+diff;
	//srcrect.left=diff;
	//srcrect.top=0;
	samples.SrcRect = rt5;

	samples.DstRect = rt4;

	// DXVA2_VideoProcess_PlanarAlpha
	samples.PlanarAlpha = DXVA2_Fixed32OpaqueAlpha();

	hr = dxvaProcessor->VideoProcessBlt(bars, &blt, &samples, 1, NULL);





#endif

#ifndef byvertices
#ifndef DXVA
	hr = d3device->StretchRect(MainStream,&rt5,bars,&rt4,D3DTEXF_LINEAR);
	if(FAILED(hr)){wxLogStatus(_("Nie można nałożyć powierzchni na siebie"));}
#endif
#endif
#if byvertices
	hr = d3device->StretchRect(MainStream,&rt5,bars,&rt4,D3DTEXF_LINEAR);
	if(FAILED(hr)){wxLogStatus("cannot stretch main stream");}
#endif

	hr = d3device->BeginScene();

#if byvertices


	// Render the vertex buffer contents
	hr = d3device->SetStreamSource( 0, vertex, 0, sizeof(CUSTOMVERTEX) );
	hr = d3device->SetVertexShader( NULL );
	hr = d3device->SetFVF( D3DFVF_CUSTOMVERTEX);
	hr = d3device->SetTexture( 0, texture );
	hr = d3device->DrawPrimitive( D3DPT_TRIANGLEFAN, 0, 2 );
#endif

	if(Vclips){Vclips->Draw(time);}

	if(cross){
		DRAWOUTTEXT(m_font,coords,rt1, (rt1.left < vectors[0].x)? 10 : 8 ,0xFFFFFFFF)
		lines->SetWidth(3);
		hr=lines->Begin();
		hr=lines->Draw(&vectors[0], 2, 0xFF000000);
		hr=lines->Draw(&vectors[2], 2, 0xFF000000);
		hr=lines->End();
		lines->SetWidth(1);
		D3DXVECTOR2 v1[4];
		v1[0]=vectors[0];
		v1[0].x += 0.5f;
		v1[1]=vectors[1];
		v1[1].x += 0.5f;
		v1[2]=vectors[2];
		v1[2].y += 0.5f;
		v1[3]=vectors[3];
		v1[3].y += 0.5f;
		hr=lines->Begin();
		hr=lines->Draw(&v1[0], 2, 0xFFFFFFFF);
		hr=lines->Draw(&v1[2], 2, 0xFFFFFFFF);
		hr=lines->End();
	}

	if(pbar){
		DRAWOUTTEXT(m_font,pbtime,rt2,DT_LEFT|DT_TOP,0xFFFFFFFF)
		hr=lines->SetWidth(1);
		hr=lines->Begin();
		hr=lines->Draw(&vectors[4], 5, 0xFF000000);
		hr=lines->Draw(&vectors[9], 5, 0xFFFFFFFF);
		hr=lines->End();
		hr=lines->SetWidth(7);
		hr=lines->Begin();
		hr=lines->Draw(&vectors[14], 2, 0xFFFFFFFF);
		hr=lines->End();
	}


	// End the scene
	hr = d3device->EndScene();



	hr = d3device->Present(NULL, &rt3, NULL, NULL );
	if( D3DERR_DEVICELOST == hr ||
		D3DERR_DRIVERINTERNALERROR == hr )
		devicelost = true;
}


bool VideoRend::DrawTexture(byte *nframe, bool copy)
{

	wxMutexLocker lock(mutexRender);
	byte *fdata=NULL;
	byte *texbuf;
	byte bytes=(vformat==RGB32)? 4 : (vformat==YUY2)? 2 : 1;


	D3DLOCKED_RECT d3dlr;
	//wxLogStatus("kopiowanie");
	if(nframe){	
		fdata= nframe;
		if(copy){byte *cpy = (byte*) datas; memcpy(cpy,fdata,vheight*pitch);}
	}
	else{
		wxLogStatus(_("Nie ma wskaźnika bufora klatki"));return false;
	}


	if(instance){
		framee->strides[0]= vwidth * bytes;
		framee->planes[0]= fdata;
		csri_render(instance,framee,(time/1000.0));
	}

#ifdef byvertices
	HR(MainStream->LockRect( &d3dlr,0, 0), _("Nie można zablokować bufora tekstury"));//D3DLOCK_NOSYSLOCK
#else
	HR(MainStream->LockRect( &d3dlr,0, D3DLOCK_NOSYSLOCK), _("Nie można zablokować bufora tekstury"));//D3DLOCK_NOSYSLOCK
#endif
	texbuf = static_cast<byte *>(d3dlr.pBits);

	diff=d3dlr.Pitch- (vwidth*bytes);
	//wxLogStatus("diff %i", diff);	
	//int check=0;	
	if (!diff){memcpy(texbuf,fdata,(vheight*pitch));}
	else{

		if(vformat>=YV12){	
			for(int i=0; i <vheight; i++){
				memcpy(texbuf,fdata,vwidth);
				texbuf+=vwidth;
				fdata+=vwidth;
				memset(texbuf,0,diff);
				texbuf+=diff;
				//check+=(vwidth+diff);
			}
			int hheight=vheight/2;
			int fwidth=(vformat==NV12)? vwidth : vwidth/2;
			int fdiff=(vformat==NV12)? diff : diff/2;

			for(int i=0; i <hheight; i++){
				memcpy(texbuf,fdata,fwidth);
				texbuf+=fwidth;
				fdata+=fwidth;
				memset(texbuf,0,fdiff);
				texbuf+=fdiff;
				//check+=(fwidth+diff);
			}
			if(vformat<NV12){
				for(int i=0; i <hheight; i++){
					memcpy(texbuf,fdata,fwidth);
					texbuf+=fwidth;
					fdata+=fwidth;
					memset(texbuf,0,fdiff);
					texbuf+=fdiff;
					//check+=(fwidth+diff);
				}
			}
			//wxLogStatus("%i, %i",check,(int)(d3dlr.Pitch*vheight*1.5));
		}
		else
		{
			int fheight=(vformat==YUY2)? vheight : vheight * bytes;
			int fwidth=vwidth * bytes;

			for(int i=0; i <fheight; i++){
				memcpy(texbuf,fdata,fwidth);
				texbuf+=(fwidth+diff);
				fdata+=fwidth;	
			}
		}

	}

	MainStream->UnlockRect();

	return true;
}


VideoRend::~VideoRend()
{
	Stop();

	vstate=None;

	SAFE_DELETE(VFF);
	Clear();
	//wxLogStatus("Vclips");
	SAFE_DELETE(Vclips);
	//wxLogStatus("vplayer");
	SAFE_DELETE(vplayer);
	//wxLogStatus("vobsub");
	SAFE_DELETE(framee);
	SAFE_DELETE(format);
	if (instance) {csri_close(instance);}
	if (vobsub) {csri_close_renderer(vobsub);}
	
	if(datas){delete[] datas;datas=NULL;}
	
}

void VideoRend::Clear()
{

	//wxLogStatus("MainStream");
	SAFE_RELEASE(MainStream);
	//wxLogStatus("bars");
	SAFE_RELEASE(bars);
	//wxLogStatus("d3device");
#if byvertices
	SAFE_RELEASE(vertex);
	SAFE_RELEASE(texture);
#endif
#if DXVA
	SAFE_RELEASE(dxvaProcessor);
	SAFE_RELEASE(dxvaService);
#endif
	SAFE_RELEASE(d3device);
	SAFE_RELEASE(d3dobject);
	SAFE_RELEASE(lines);
	SAFE_RELEASE(m_font);
}



bool VideoRend::OpenFile(const wxString &fname, wxString *textsubs, bool Dshow, bool __vobsub, bool fullscreen)
{
	wxMutexLocker lock(mutexOpenFile);
	//block=true;
	kainoteApp *Kaia=(kainoteApp*)wxTheApp;
	VideoFfmpeg *tmpvff=NULL;
	//wxLogStatus("stop");
	if(vstate==Playing){((VideoCtrl*)this)->Stop();}
	//wxLogStatus("stoped");
	IsDshow=Dshow;
	if(!Dshow){
		bool success;
		tmpvff=new VideoFfmpeg(fname, this, &success);
		if(!success || !tmpvff){
			SAFE_DELETE(tmpvff);/*block=false;*/return false;
		}
	}

	if(vstate!=None){
		resized=seek=cross=pbar=false;
		vstate=None;Clear();SAFE_DELETE(VFF);
	}

	time=0;
	lastframe=0;


	if(!Dshow){
		SAFE_DELETE(vplayer);

		//VFF=new VideoFfmpeg(fname, Kaia->Frame->Tabs->GetSelection(),&success);
		VFF=tmpvff;
		d3dformat=D3DFORMAT('21VN');
		vformat=NV12;
		vwidth=VFF->width;vheight=VFF->height;fps=VFF->fps;ax=VFF->arwidth;ay=VFF->arheight;
		if(vwidth % 2 != 0 ){vwidth++;}
		pitch=vwidth*1.5f;

		TabPanel *pan = ((TabPanel*)GetParent());
		if(VFF->GetSampleRate()>0){
			Kaia->Frame->OpenAudioInTab(pan,40000,fname);
			player = pan->Edit->ABox->audioDisplay;
		}else if(player){Kaia->Frame->OpenAudioInTab(pan,CloseAudio,"");}
	}else{

		if(!vplayer){vplayer= new DShowPlayer(this);}

		if(!vplayer->OpenFile(fname, __vobsub)){/*block=false;*/return false;}

		wxSize siz=vplayer->GetVideoSize();
		vwidth=siz.x;vheight=siz.y;
		if(vwidth % 2 != 0 ){vwidth++;}

		pitch=vwidth*vplayer->inf.bytes;
		fps=vplayer->inf.fps;
		vformat=vplayer->inf.CT;
		ax=vplayer->inf.ARatioX;
		ay=vplayer->inf.ARatioY;
		d3dformat=(vformat==5)? D3DFORMAT('21VN') : (vformat==3)? D3DFORMAT('21VY') : (vformat==2)? D3DFMT_YUY2 : D3DFMT_X8R8G8B8;
		if(player){
			Kaia->Frame->OpenAudioInTab(((TabPanel*)GetParent()),CloseAudio,"");
		}
	}
	diff=0;
	avtpf=(1000.0f/fps);
	if(ay==0||ax==0){AR=0.0f;}else{AR=(float)ay/(float)ax;}

	rt5.bottom=vheight;
	rt5.right=vwidth;
	rt5.left=0;
	rt5.top=0;
	if(datas){delete[] datas;datas=NULL;}
	datas=new char[vheight*pitch];

	if(!InitDX()){/*block=false;*/return false;}
	UpdateRects(!fullscreen);
	
	if(!framee){framee=new csri_frame;}
	if(!format){format=new csri_fmt;}
	for(int i=1;i<4;i++){
		framee->planes[i]=NULL;
		framee->strides[i]=NULL;
	}

	framee->pixfmt=(vformat==5)? CSRI_F_YV12A : (vformat==3)? CSRI_F_YV12 : (vformat==2)? CSRI_F_YUY2 : CSRI_F_BGR_;

	format->width = vwidth;
	format->height = vheight;
	format->pixfmt = framee->pixfmt;
	format->fps=(!IsDshow)? 25.0f : fps;

	if(!__vobsub){
		OpenSubs(textsubs,false);
	}else{
		SAFE_DELETE(textsubs);
		OpenSubs(0,false);
	}
	/*block=false;*/
	vstate=Stopped;
	if(IsDshow && vplayer){chaps = vplayer->GetChapters();}
	if(Vclips){
		Vclips->SizeChanged(wxSize(rt3.right, rt3.bottom),lines, m_font, d3device);
	}
	return true;
}

bool VideoRend::Play(int end)
{
	SetThreadExecutionState(ES_DISPLAY_REQUIRED|ES_CONTINUOUS);
	VideoCtrl *vb=((VideoCtrl*)this);
	if( !(IsShown() || (vb->TD && vb->TD->IsShown())) ){return false;}
	TabPanel* pan=(TabPanel*)GetParent();
	if(VisEdit){
		wxString *txt=pan->Grid1->SaveText();
		OpenSubs(txt);
		SAFE_DELETE(Vclips->dummytext);
		VisEdit=false;
	}else if(pan->Edit->OnVideo){OpenSubs(pan->Grid1->SaveText());pan->Edit->OnVideo=false;}

	if(end>0){playend=end;}else if(IsDshow){playend=0;}else{playend=GetDuration();}
	if(IsDshow){vplayer->Play();}

	vstate=Playing;

	if(!IsDshow){
		/*if(thread){
			WaitForSingleObject(thread,2000);CloseHandle(thread);thread=NULL;
		}*/
		lasttime=timeGetTime()-time;
		if(player){player->Play(time,-1,false);}
		//lastframe++;
		time=VFF->Timecodes[lastframe];
		VFF->Play();
		//DWORD kkk;
		//thread = CreateThread( NULL, 0,  (LPTHREAD_START_ROUTINE)playingProc, this, 0, &kkk);
		//if(thread){
		//SetThreadPriority(thread, THREAD_PRIORITY_TIME_CRITICAL);
		//}

	}
	return true;
}

bool VideoRend::PlayLine(int start, int eend)
{
	int duration = GetDuration();
	if(vstate==None || start>=eend || start >= duration ){return false;}
	if( duration < eend){eend = duration;}
	SetPosition(start);
	Play(eend);
	return true;
}

bool VideoRend::Pause()
{
	if(vstate==Playing){
		SetThreadExecutionState(ES_CONTINUOUS);
		vstate=Paused;
		if(!IsDshow){
			if(player){player->player->Stop();}
		}else{
			vplayer->Pause();
		}
	}
	else if(vstate==Paused || vstate==Stopped){
		Play();
	}else{return false;}
	return true;
}

bool VideoRend::Stop()
{
	if(vstate==Playing){
		SetThreadExecutionState(ES_CONTINUOUS);
		vstate=Stopped;
		if(IsDshow){vplayer->Stop();}

		if(!IsDshow){if(player){player->Stop();}//if(thread){CloseHandle(thread);thread=NULL;}
		}

		time=0;
		playend=(IsDshow)? 0 : GetDuration();

		return true;
	}
	return false;
}

void VideoRend::SetPosition(int _time, bool starttime, bool corect, bool reloadSubs)
{
	TabPanel* pan=(TabPanel*)GetParent();
	if(IsDshow){
		time=MID(0,_time,GetDuration());
		if(corect&&IsDshow){
			time/=avtpf;
			if(starttime){time++;}
			time*=avtpf;
		}
		if(VisEdit){
			SAFE_DELETE(Vclips->dummytext);
			if(Vclips->Visual==VECTORCLIP){
				Vclips->SetClip(Vclips->GetVisual(),true, false);
				//SetVisual(pan->Edit->line->Start.mstime, pan->Edit->line->End.mstime,false, false);
				//OpenSubs((vstate==Playing)? pan->Grid1->SaveText() : pan->Grid1->GetVisible());
			}else{
				OpenSubs((vstate==Playing)? pan->Grid1->SaveText() : pan->Grid1->GetVisible());
			}
			VisEdit=false;
		}else if(pan->Edit->OnVideo){
			if(time >= pan->Edit->line->Start.mstime && time <= pan->Edit->line->End.mstime){
				wxCommandEvent evt;pan->Edit->OnEdit(evt);
				//pan->Edit->OnVideo=false;
			}
		}	
		playend=(IsDshow)? 0 : GetDuration();
		seek=true; vplayer->SetPosition(time);
	}else{
		auto oldvstate = vstate;
		vstate=Paused;
		int decr= (vstate==Playing)? 1 : 0;
		lastframe = VFF->GetFramefromMS(_time,(time>_time)? 0 : lastframe); //- decr;
		if(!starttime){lastframe--;if(VFF->Timecodes[lastframe]>=_time){lastframe--;}}
		time = VFF->Timecodes[lastframe];
		if(VisEdit){
			SAFE_DELETE(Vclips->dummytext);
			if(Vclips->Visual==VECTORCLIP){
				Vclips->SetClip(Vclips->GetVisual(),true, false);
				//SetVisual(pan->Edit->line->Start.mstime, pan->Edit->line->End.mstime,false, false);
				//OpenSubs((vstate==Playing)? pan->Grid1->SaveText() : pan->Grid1->GetVisible());
			}else{
				OpenSubs((vstate==Playing)? pan->Grid1->SaveText() : pan->Grid1->GetVisible());
			}
			VisEdit=false;
		}else if(pan->Edit->OnVideo){
			if(time >= pan->Edit->line->Start.mstime && time <= pan->Edit->line->End.mstime){
				wxCommandEvent evt;pan->Edit->OnEdit(evt);
				//pan->Edit->OnVideo=false;
			}
		}	
		if(oldvstate==Playing){
			vstate=Playing;
			lasttime=timeGetTime()-time;
			if(player){
				player->player->SetCurrentPosition(player->GetSampleAtMS(time));
			}
			VFF->Play();
		}
		else{
			if(player){player->UpdateImage(true);}
			Render();
		}
	}
}

bool VideoRend::OpenSubs(wxString *textsubs, bool redraw)
{
	wxMutexLocker lock(mutexRender);
	if (instance) csri_close(instance);
	instance = NULL;

	if(!textsubs) {if (vobsub) {csri_close_renderer(vobsub);}return false;}
	//const char *buffer= textsubs.mb_str(wxConvUTF8).data();
	if(VisEdit && Vclips->Visual==VECTORCLIP && Vclips->dummytext){
		//wxLogStatus("clip background");
		(*textsubs)<<Vclips->dummytext->Trim().AfterLast('\n');
	}
	wxScopedCharBuffer buffer= textsubs->mb_str(wxConvUTF8);
	int size = strlen(buffer);


	// Select renderer
	//if(!vobsub){
		vobsub = csri_renderer_default();
		if(!vobsub){wxLogStatus(_("CSRI odmówiło posłuszeństwa.")); delete textsubs; return false;}
	//}

	instance = csri_open_mem(vobsub,buffer,size,NULL);
	if(!instance){wxLogStatus(_("Instancja VobSuba nie utworzyła się.")); delete textsubs; return false;}

	if(!format || csri_request_fmt(instance,format)){
		wxLogStatus(_("CSRI nie obsługuje tego formatu."));
		csri_close(instance);
		instance = NULL;
		delete textsubs; return false;
	}

	if(redraw && vstate!=None && IsDshow && datas){
		int all=vheight*pitch;
		char *cpy=new char[all];
		byte *cpy1=(byte*)cpy;
		byte *data1=(byte*)datas;
		memcpy(cpy1,data1,all);
		DrawTexture(cpy1);
		delete[] cpy;
	}

	delete textsubs;
	return true;
}

int VideoRend::GetCurrentPosition()
{
	return time;
}

int VideoRend::GetDuration()
{
	if(IsDshow){return vplayer->GetDuration();}
	return VFF->Duration*1000.0;
}


//ustawia nowe recty po zmianie rozdzielczości wideo
bool VideoRend::UpdateRects(bool VideoPanel)
{
	VideoCtrl* Video=(VideoCtrl*) this;
	wxRect rt;
	TabPanel* tab=(TabPanel*)Video->GetParent();
	if(VideoPanel){hwnd=GetHWND();rt=GetClientRect();rt.height-=panelHeight;pbar=false;}
	else{hwnd=Video->TD->GetHWND();rt=Video->TD->GetClientRect();pbar=true;cross=false;}
	if(!rt.height || !rt.width){return false;}
	rt3.bottom=rt.height;
	rt3.right=rt.width;
	rt3.left=rt.x;
	rt3.top=rt.y;

	if(tab->edytor&&!Video->isfullskreen){
		rt4=rt3;
	}
	else
	{
		int arwidth=rt.height / AR;
		int arheight=rt.width * AR;
		if(arwidth > rt.width)
		{
			int onebar=(rt.height-arheight)/2;
			rt4.bottom=arheight+onebar;
			rt4.right=rt.width;//zostaje bez zmian
			rt4.left=0;
			rt4.top=onebar;
		}
		else if(arheight > rt.height)
		{
			int onebar=(rt.width-arwidth)/2;
			rt4.bottom=rt.height;//zostaje bez zmian
			rt4.right=arwidth+onebar;
			rt4.left=onebar;
			rt4.top=0;
		}
		else
		{
			rt4=rt3;
		}
	}
	return true;
}

//funkcja zmiany rozdziałki okna wideo
void VideoRend::UpdateVideoWindow(bool bar)
{


	wxMutexLocker lock(mutexRender);
	block=true;
	if(!UpdateRects(bar)){block=false;return;}

	if(!InitDX(true)){block=false;return;}

	if(IsDshow && datas){

		int all=vheight*pitch;
		char *cpy=new char[all];
		byte *cpy1=(byte*)cpy;
		byte *data1=(byte*)datas;
		memcpy(cpy1,data1,all);
		DrawTexture(cpy1);
		delete[] cpy;
	}


	resized=true;
	if(Vclips){
		Vclips->SizeChanged(wxSize(rt3.right, rt3.bottom),lines, m_font, d3device);
		TabPanel* tab=(TabPanel*)GetParent();
		SetVisual(tab->Edit->line->Start.mstime, tab->Edit->line->End.mstime);
	}
	block=false;
}

void VideoRend::GetFpsnRatio(float *fps, long *arx, long *ary)
{
	if(IsDshow){vplayer->GetFpsnRatio(fps,arx,ary);return;}
	*fps=VFF->fps;
	*arx=VFF->arwidth;
	*ary=VFF->arheight;
}

void VideoRend::GetVideoSize(int *width, int *height)
{
	if(IsDshow){wxSize sz = vplayer->GetVideoSize();
	*width=sz.x;
	*height=sz.y;
	return;}
	*width=VFF->width;
	*height=VFF->height;
}

wxSize VideoRend::GetVideoSize()
{
	wxSize sz;
	if(IsDshow){sz = vplayer->GetVideoSize();return sz;}
	sz.x=VFF->width;
	sz.y=VFF->height;
	return sz;
}

void VideoRend::DrawLines(wxPoint point)
{
	wxMutexLocker lock(mutexLines);
	int w, h;
	GetClientSize(&w, &h);
	w/=2; h/=2;
	rt1.top= (h>point.y)? point.y-12 : point.y-40;
	rt1.bottom= (h>point.y)? point.y+23 : point.y-5;
	rt1.left= (w<point.x)? point.x-100 : point.x+5;
	rt1.right= (w<point.x)? point.x-5 : point.x+100;

	vectors[0].x = point.x;
	vectors[0].y = 0;
	vectors[1].x = point.x;
	vectors[1].y = rt4.bottom;
	vectors[2].x = 0;
	vectors[2].y = point.y;
	vectors[3].x = rt4.right;
	vectors[3].y = point.y;
	cross=true;	
	if(vstate==Paused && !block){Render(/*false*/resized);}      
}

void VideoRend::DrawProgBar()
{
	//pozycja zegara
	wxMutexLocker lock(mutexProgBar);
	int w,h;
	VideoCtrl *vc=(VideoCtrl*)this;
	vc->TD->GetClientSize(&w,&h);
	rt2.top=16;
	rt2.bottom=60;
	rt2.left=w-167;
	rt2.right=w-3;
	//koordynaty czarnej ramki
	vectors[4].x = w-170;
	vectors[4].y = 5;
	vectors[5].x = w-5;
	vectors[5].y = 5;
	vectors[6].x = w-5;
	vectors[6].y = 15;
	vectors[7].x = w-170;
	vectors[7].y = 15;
	vectors[8].x = w-170;
	vectors[8].y = 5;
	//koordynaty białej ramki
	vectors[9].x = w-169;
	vectors[9].y = 6;
	vectors[10].x = w-6;
	vectors[10].y = 6;
	vectors[11].x = w-6;
	vectors[11].y = 14;
	vectors[12].x = w-169;
	vectors[12].y = 14;
	vectors[13].x = w-169;
	vectors[13].y = 6;
	//koordynaty paska postępu
	int rw=w-168;
	vectors[14].x = rw;
	vectors[14].y = 10.5;
	vectors[15].x = (GetDuration()>0)? (((float)time/(float)GetDuration())*161)+rw : 161+rw;
	vectors[15].y = 10.5;
}

void VideoRend::SetVolume(int vol)
{
	if(vstate==None){return;}
	if(!IsDshow){
		vol=8100+vol;
		double dvol=vol/8100.0;
		dvol=pow(dvol,2);
		if(player){player->player->SetVolume(dvol);}
	}
	else
	{
		vplayer->SetVolume(vol);
	}
}

int VideoRend::GetVolume()
{
	if(vstate==None){return 0;}
	if(!IsDshow && player){
		double dvol=player->player->GetVolume();
		dvol= sqrt(dvol);
		dvol*=8100.0;
		dvol-=8100.0;
		return dvol;}
	else if(IsDshow){
		return vplayer->GetVolume();
	}
	return 0;
}

void VideoRend::MovePos(int cpos)
{	
	if(!IsDshow){
		lastframe=MID(0,lastframe+cpos,VFF->NumFrames-1);
		time = VFF->Timecodes[lastframe];
		TabPanel* pan=(TabPanel*)GetParent();
		if(VisEdit){
			wxString *txt=pan->Grid1->SaveText();
			OpenSubs(txt);VisEdit=false;
		}else if(pan->Edit->OnVideo){OpenSubs(pan->Grid1->SaveText());pan->Edit->OnVideo=false;}
		if(player){player->UpdateImage(true);}
		Render(true);
	}
	else{
		//for(int i = 1; i<100; i++){
		time+=((avtpf)*cpos);
		//int tmpt=time;
		SetPosition(time,true,false);
		//wxLogStatus("times %i, %i", tmpt, time);
		//if(tmpt!=time){break;}
		//}
	}
	VideoCtrl *vb=(VideoCtrl*)this;
	vb->displaytime();
}


wxArrayString VideoRend::GetStreams()
{
	if(vplayer){return vplayer->GetStreams();}
	wxArrayString streams;
	return streams;
}

void VideoRend::EnableStream(long index)
{
	if(vplayer->stream){seek=true;vplayer->stream->Enable(index,AMSTREAMSELECTENABLE_ENABLE);}
}



void VideoRend::ChangeVobsub(bool vobsub)
{
	if(!vplayer){return;}
	kainoteApp *Kaia=(kainoteApp*)wxTheApp;

	int tmptime = time;
	TabPanel *pan=Kaia->Frame->GetTab();
	OpenSubs((vobsub)? NULL : pan->Grid1->SaveText());
	vplayer->OpenFile(pan->VideoPath,vobsub);
	SetPosition(tmptime);
	if(vstate==Paused){vplayer->Play();vplayer->Pause();}
	else if(vstate==Playing){vplayer->Play();}
	int pos=pan->Video->volslider->GetValue();
	SetVolume(-(pos*pos));
	pan->Video->ChangeStream();
}

void VideoRend::SetVisual(int start, int end, bool remove, bool settext)
{
	TabPanel* pan=(TabPanel*)GetParent();

	if(remove){
		SAFE_DELETE(Vclips); pan->Edit->Visual=0;
		VisEdit=false;
		OpenSubs(pan->Grid1->SaveText());
		Render();
	}
	else{

		int vis=pan->Edit->Visual;
		if(!Vclips){
			Vclips = Visuals::Get(vis,this);
		}else if(Vclips->Visual != vis){
			bool vectorclip = Vclips->Visual == VECTORCLIP;
			delete Vclips;
			Vclips = Visuals::Get(vis,this);
			if(vectorclip && !settext){OpenSubs(pan->Grid1->GetVisible());}
		}else{SAFE_DELETE(Vclips->dummytext);}
		if(settext){OpenSubs(pan->Grid1->GetVisible());}
		Vclips->SizeChanged(wxSize(rt3.right, rt3.bottom),lines, m_font, d3device);

		Vclips->SetVisual(start, end);
		//VisEdit=true;
	}
}

void VideoRend::SetVisual()
{

	//TabPanel* pan=(TabPanel*)GetParent();
	SAFE_DELETE(Vclips->dummytext);
	Vclips->SetCurVisual();
	VisEdit=true;
	Render();
}

bool VideoRend::EnumFilters(Menu *menu)
{
	if(vplayer){return vplayer->EnumFilters(menu);}
	return false;
}

bool VideoRend::FilterConfig(wxString name, int idx, wxPoint pos)
{
	if(vplayer){return vplayer->FilterConfig(name,idx,pos);}
	return false;
}

byte *VideoRend::GetFramewithSubs(bool subs, bool *del)
{
	bool dssubs=(IsDshow && subs && Notebook::GetTab()->edytor);
	bool ffnsubs=(!IsDshow && !subs);
	byte *cpy1;
	byte bytes=(vformat==RGB32)? 4 : (vformat==YUY2)? 2 : 1;
	int all=vheight*pitch;
	if(dssubs||ffnsubs){
		*del=true;
		char *cpy=new char[all];
		cpy1=(byte*)cpy;
	}else{*del=false;}
	if(ffnsubs){
		VFF->GetFrame(time,cpy1);
	}else if(instance && dssubs){
		byte *data1=(byte*)datas;
		memcpy(cpy1,data1,all);
		framee->strides[0]= vwidth * bytes;
		framee->planes[0]= cpy1;
		csri_render(instance,framee,(time/1000.0));
	}
	return (dssubs||ffnsubs)? cpy1 : (byte*)datas;
}

void VideoRend::GoToNextKeyframe()
{
	if(!VFF){return;}
	for(size_t i=0; i<VFF->KeyFrames.size(); i++){
		if(VFF->KeyFrames[i]>time){
			SetPosition(VFF->KeyFrames[i]);
			return;
		}
	}
	SetPosition(VFF->KeyFrames[0]);
}
void VideoRend::GoToPrevKeyframe()
{
	if(!VFF){return;}
	for(int i=VFF->KeyFrames.size()-1; i>=0 ; i--){
		if(VFF->KeyFrames[i]<time){
			SetPosition(VFF->KeyFrames[i]);
			return;
		}
	}
	SetPosition(VFF->KeyFrames[VFF->KeyFrames.size()-1]);
}