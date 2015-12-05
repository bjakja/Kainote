
#include "Visuals.h"
#include "Videobox.h"
#include "VideoRenderer.h"
#include "dshowplayer.h"
#include "kainoteApp.h"

#include <Dvdmedia.h>
#include "vsfilterapi.h"




#if vertices
struct CUSTOMVERTEX
{
	D3DXVECTOR3 position; // The position
	FLOAT       tu, tv;   // The texture coordinates
};
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_TEX1)
#endif


VideoRend::VideoRend(wxWindow *_parent, const wxSize &size)
	:wxWindow(_parent,-1,wxDefaultPosition, size)//wxFULL_REPAINT_ON_RESIZE
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
	thread=NULL;
#if vertices
	vertex=NULL;
	texture=NULL;
#endif
}

bool VideoRend::InitDX(bool reset)
{

	if(!reset){
		d3dobject = Direct3DCreate9( D3D_SDK_VERSION );
		PTR(d3dobject,L"Nie mo¿na utwo¿yæ objektu Direct 3d");
	}else{
		SAFE_RELEASE(MainStream);
		SAFE_RELEASE(bars);
		SAFE_RELEASE(lines);
		SAFE_RELEASE(m_font);
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
	d3dpp.Flags					 = 0;//|D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
	//d3dpp.PresentationInterval   = D3DPRESENT_INTERVAL_DEFAULT;

	if(reset){
		hr=d3device->Reset(&d3dpp);
		if(FAILED(hr)){wxLogStatus("Nie mo¿na zresetowaæ d3d");}
	}else{
		hr=d3dobject->CreateDevice(D3DADAPTER_DEFAULT,D3DDEVTYPE_HAL,hwnd,
			D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED , &d3dpp, &d3device);//| D3DCREATE_FPU_PRESERVE
		if(FAILED(hr)){
			HR (d3dobject->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd,
				D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED,&d3dpp, &d3device ), L"Nie mo¿na utworzyæ urz¹dzenia D3D9"); 
		} 
	}
	//hr = d3device->SetSamplerState( 0, D3DSAMP_ADDRESSU,  D3DTADDRESS_CLAMP );
	//hr = d3device->SetSamplerState( 0, D3DSAMP_ADDRESSV,  D3DTADDRESS_CLAMP );
	hr = d3device->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
	hr = d3device->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
	hr = d3device->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, TRUE);
	hr = d3device->SetRenderState(D3DRS_ANTIALIASEDLINEENABLE, TRUE);
	hr = d3device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	hr = d3device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	hr = d3device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	hr = d3device->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE);
	hr = d3device->SetRenderState( D3DRS_LIGHTING, FALSE );
	hr = d3device->SetRenderState( D3DRS_ZENABLE, D3DZB_TRUE);
	hr = d3device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	hr = d3device->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
	hr = d3device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	hr = d3device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
	HR(hr,"Zawiod³o któreœ z ustawieñ dx");

	D3DXMATRIX matOrtho; 
	D3DXMATRIX matIdentity;

	D3DXMatrixOrthoOffCenterLH(&matOrtho, 0, rt3.right, rt3.bottom, 0, 0.0f, 1.0f);
	D3DXMatrixIdentity(&matIdentity);

	HR(d3device->SetTransform(D3DTS_PROJECTION, &matOrtho), "Nie mo¿na ustawiæ matrixa projection");
	HR(d3device->SetTransform(D3DTS_WORLD, &matIdentity), "Nie mo¿na ustawiæ matrixa world");
	HR(d3device->SetTransform(D3DTS_VIEW, &matIdentity), "Nie mo¿na ustawiæ matrixa view");

#if vertices
	HR(d3device->CreateTexture(vwidth, vwidth, 1, D3DUSAGE_RENDERTARGET,
		D3DFMT_X8R8G8B8,D3DPOOL_DEFAULT,&texture, NULL), "Nie mo¿na utworzyæ tekstury" );

	HR(texture->GetSurfaceLevel(0, &bars), "nie mo¿na utworzyæ powierzchni");

	d3device->CreateOffscreenPlainSurface(vwidth,vheight,d3dformat, D3DPOOL_DEFAULT,&MainStream,0);

	HR(d3device->CreateVertexBuffer( 4*sizeof(CUSTOMVERTEX),D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, D3DFVF_CUSTOMVERTEX,D3DPOOL_DEFAULT, &vertex, NULL ),
		"Nie mo¿na utworzyæ bufora wertex")

		CUSTOMVERTEX* pVertices;
	HR ( hr = vertex->Lock( 0, 0, (void**)&pVertices, 0 ), "nie mo¿na zablokowaæ bufora vertex" ); 

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
	//wxLogStatus("Textura ma niew³aœciwy format"); return false;	
	//}

#else
	HR (d3device->GetBackBuffer(0,0, D3DBACKBUFFER_TYPE_MONO, &bars),L"Nie mo¿na stworzyæ powierzchni");

	d3device->CreateOffscreenPlainSurface(vwidth,vheight,d3dformat, D3DPOOL_DEFAULT,&MainStream,0);//D3DPOOL_DEFAULT
#endif

	D3DXCreateLine(d3device, &lines);
	D3DXCreateFont(d3device, 20, 0, FW_BOLD, 0, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, TEXT("Tahoma"), &m_font );
	return true;
}

void VideoRend::Render(bool Frame)
{
	//wxLogStatus("render");
	if(Frame&&!IsDshow){if(!DrawTexture()){return;}resized=false;}
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



#ifndef vertices
	hr = d3device->StretchRect(MainStream,&rt5,bars,&rt4,D3DTEXF_LINEAR);
	if(FAILED(hr)){wxLogStatus("cannot stretch main stream");}
#endif


	hr = d3device->BeginScene();

#if vertices


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

#if vertices
	hr = d3device->StretchRect(MainStream,&rt5,bars,&rt4,D3DTEXF_LINEAR);
	if(FAILED(hr)){wxLogStatus("cannot stretch main stream");}
#endif

	hr = d3device->Present(NULL, &rt3, NULL, NULL );
	if( D3DERR_DEVICELOST == hr ||
		D3DERR_DRIVERINTERNALERROR == hr )
		devicelost = true;
}


bool VideoRend::DrawTexture(byte *nframe, bool copy)
{

	wxMutexLocker lock(mutexDrawing);
	byte *fdata=NULL;
	byte *texbuf;
	byte bytes=(vformat==RGB32)? 4 : (vformat==YUY2)? 2 : 1;


	D3DLOCKED_RECT d3dlr;
	//wxLogStatus("kopiowanie");
	if(nframe){	
		fdata=(byte*)nframe;
		if(copy){byte *cpy = (byte*) datas; memcpy(cpy,fdata,vheight*pitch);}
	}
	else if(!IsDshow){
		fdata=(byte*)datas;
		VFF->GetFrame(lastframe,fdata);
	}
	else{
		wxLogStatus("nie ma wskaŸnika bufora");return false;
	}


	if(instance){
		framee->strides[0]= vwidth * bytes;
		framee->planes[0]= fdata;
		csri_render(instance,framee,(time/1000.0));
	}


	HR(MainStream->LockRect( &d3dlr,0, D3DLOCK_NOSYSLOCK), L"Nie mo¿na zablokowaæ bufora tekstury");//D3DLOCK_NOSYSLOCK

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
	//wxLogStatus(wxString(__FILE__));
	//wxLogStatus("Stop");
	//if(IsDshow){Stop();}
	Stop();
	//wxLogStatus("Clear");
	vstate=None;
	if(thread){WaitForSingleObject(thread,2000);}
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
	//SAFE_DELETE(VA);
	//wxLogStatus("VFF");
	SAFE_DELETE(VFF);

	//wxLogStatus("datas");
	if(datas){delete[] datas;datas=NULL;}
	//wxLogStatus("all");
}

void VideoRend::Clear()
{

	//wxLogStatus("MainStream");
	SAFE_RELEASE(MainStream);
	//wxLogStatus("bars");
	SAFE_RELEASE(bars);
	//wxLogStatus("d3device");
#if vertices
	SAFE_RELEASE(vertex);
	SAFE_RELEASE(texture);
#endif
	SAFE_RELEASE(d3device);
	//wxLogStatus("d3dobject");
	SAFE_RELEASE(d3dobject);
	//wxLogStatus("lines");
	SAFE_RELEASE(lines);
	//wxLogStatus("font");
	SAFE_RELEASE(m_font);
	if(thread){CloseHandle(thread);thread=NULL;}
}



bool VideoRend::OpenFile(const wxString &fname, wxString *textsubs, bool Dshow, bool __vobsub, bool fullscreen)
{
	wxMutexLocker lock(mutexOpenFile);
	block=true;
	kainoteApp *Kaia=(kainoteApp*)wxTheApp;
	VideoFfmpeg *tmpvff=NULL;
	//wxLogStatus("stop");
	if(vstate==Playing){((VideoCtrl*)this)->Stop();}
	//wxLogStatus("stoped");
	IsDshow=Dshow;
	if(!Dshow){
		bool success;
		tmpvff=new VideoFfmpeg(fname, Kaia->Frame->Tabs->GetSelection(),&success);
		if(!success || !tmpvff){SAFE_DELETE(tmpvff);return false;}
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

		if(VFF->GetSampleRate()>0){
			wxCommandEvent evt;
			evt.SetId(40000);
			evt.SetString(fname);
			//wxLogStatus("przed audio");
			Kaia->Frame->OnOpenAudio(evt);

			player=Kaia->Frame->GetTab()->Edit->ABox->audioDisplay;
		}else if(player){wxCommandEvent evt;evt.SetId(ID_CLOSEAUDIO);Kaia->Frame->OnOpenAudio(evt);}
	}else{

		if(!vplayer){vplayer= new DShowPlayer(this);}

		if(!vplayer->OpenFile(fname, __vobsub)){return false;}

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
			wxCommandEvent evt;
			evt.SetId(ID_CLOSEAUDIO);
			Kaia->Frame->OnOpenAudio(evt);
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

	if(!InitDX()){block=false;return false;}
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
	format->fps=fps;
	OpenSubs(textsubs,false);
	block=false;
	vstate=Stopped;
	if(IsDshow && vplayer){chaps = vplayer->GetChapters();}
	return true;
}

void VideoRend::Play(int end)
{
	VideoCtrl *vb=((VideoCtrl*)this);
	if( !(IsShown() || (vb->TD && vb->TD->IsShown())) ){return;}
	TabPanel* pan=(TabPanel*)GetParent();
	if(VisEdit){
		wxString *txt=pan->Grid1->SaveText();
		if(pan->Edit->Visual==VECTORCLIP){(*txt)<<pan->Edit->dummytext->Trim().AfterLast('\n');}
		OpenSubs(txt);VisEdit=false;
	}else if(pan->Edit->OnVideo){OpenSubs(pan->Grid1->SaveText());pan->Edit->OnVideo=false;}

	if(end>0){playend=end;}else if(IsDshow){playend=0;}else{playend=GetDuration();}
	if(IsDshow){vplayer->Play();}

	vstate=Playing;

	if(!IsDshow){
		if(thread){
			WaitForSingleObject(thread,2000);CloseHandle(thread);thread=NULL;
			//CloseHandle(thread1);thread1=NULL;CloseHandle(event1);event1=NULL;
		}
		lasttime=timeGetTime()-time;
		if(player){player->Play(time,-1,false);}
		lastframe++;
		time=VFF->Timecodes[lastframe];

		DWORD kkk;
		thread = CreateThread( NULL, 0,  (LPTHREAD_START_ROUTINE)playingProc, this, 0, &kkk);
		//if(thread){
		//SetThreadPriority(thread, THREAD_PRIORITY_TIME_CRITICAL);
		//}

	}

}

void VideoRend::PlayLine(int start, int eend)
{
	if(vstate==None || start>=eend){return;}

	SetPosition(start);
	Play(eend);

}

void VideoRend::Pause()
{
	if(vstate==Playing){
		vstate=Paused;
		if(!IsDshow){if(player){player->player->Stop();/*player->Refresh(false);*/}//if(thread){CloseHandle(thread);thread=NULL;}
		}
		else{vplayer->Pause();}
	}
	else if(vstate==Paused || vstate==Stopped){
		Play();
	}

}

void VideoRend::Stop()
{
	if(vstate==Playing){
		vstate=Stopped;
		if(IsDshow){vplayer->Stop();}

		if(!IsDshow){if(player){player->Stop();}//if(thread){CloseHandle(thread);thread=NULL;}
		}

		time=0;
		playend=(IsDshow)? 0 : GetDuration();


	}

}

void VideoRend::SetPosition(int _time, bool starttime, bool corect)
{

	if(IsDshow){
		time=MID(0,_time,GetDuration());
		if(corect&&IsDshow){
			time/=avtpf;
			if(starttime){time++;}
			time*=avtpf;
		}
		if(VisEdit){TabPanel* pan=(TabPanel*)GetParent();
		SAFE_DELETE(pan->Edit->dummytext);
		SetVisual(pan->Edit->line->Start.mstime, pan->Edit->line->End.mstime);
		VisEdit=false;
		}	
		playend=(IsDshow)? 0 : GetDuration();
		seek=true; vplayer->SetPosition(time);
	}else{
		lastframe=VFF->GetFramefromMS(_time,(time>_time)? 1 : lastframe);
		if(!starttime){lastframe--;if(VFF->Timecodes[lastframe]>=_time){lastframe--;}}
		time = VFF->Timecodes[lastframe];
		if(VisEdit){TabPanel* pan=(TabPanel*)GetParent();
		SAFE_DELETE(pan->Edit->dummytext);
		SetVisual(pan->Edit->line->Start.mstime, pan->Edit->line->End.mstime);
		VisEdit=false;
		}	
		if(vstate==Playing){
			lasttime=timeGetTime()-time;
			if(player){
				player->player->SetCurrentPosition(player->GetSampleAtMS(time));}
		}
		else{Render();}
	}
}

bool VideoRend::OpenSubs(wxString *textsubs, bool redraw)
{
	wxMutexLocker lock(mutexOpenSubs);
	if (instance) csri_close(instance);
	instance = NULL;

	if(!textsubs) {if (vobsub) {csri_close_renderer(vobsub);}return false;}
	//const char *buffer= textsubs.mb_str(wxConvUTF8).data();
	wxScopedCharBuffer buffer= textsubs->mb_str(wxConvUTF8);
	int size = strlen(buffer);


	// Select renderer
	//if(!vobsub){
	vobsub = csri_renderer_default();
	//}
	PTR(vobsub,"Csri failed.");


	instance = csri_open_mem(vobsub,buffer,size,NULL);
	PTR(instance,"Instancja vobsuba nie utworzy³a siê.");

	if(csri_request_fmt(instance,format)){wxLogStatus("request format failed");}

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


DWORD VideoRend::playingProc(void* cls)
{
	((VideoRend*)cls)->playing();
	return 0;
}

//funkcja odtwarzania przez ffms2 automatycznie wywo³ywana timerem
void VideoRend::playing()
{
	int tdiff=0;

	wxRect rt(0,0,1,1);
	while(1){
		Refresh(false,&rt);
		//DrawTexture((byte*)datas);
		//Render();

		if(time>=playend){
			wxCommandEvent *evt = new wxCommandEvent(wxEVT_COMMAND_BUTTON_CLICKED,23333);
			wxQueueEvent(this, evt);
			break;
		}
		else if(vstate!=Playing){
			break;}	




		time= timeGetTime() - lasttime;

		lastframe++;

		while(true)
		{
			if(VFF->Timecodes[lastframe]>=time)
			{
				break;
			}
			else{lastframe++;}
		}
		//wxThreadEvent *evt = new wxThreadEvent(wxEVT_THREAD,23334);
		tdiff=VFF->Timecodes[lastframe] - time;
		time = VFF->Timecodes[lastframe];

		//wxQueueEvent(parent, evt);
		Sleep(tdiff);
	}
}

//ustawia nowe recty po zmianie rozdzielczoœci wideo
void VideoRend::UpdateRects(bool bar)
{
	VideoCtrl* Video=(VideoCtrl*) this;
	wxRect rt;
	TabPanel* tab=(TabPanel*)Video->GetParent();
	if(bar){hwnd=GetHWND();rt=GetClientRect();rt.height-=44;}
	else{hwnd=Video->TD->GetHWND();rt=Video->TD->GetClientRect();pbar=true;cross=false;}

	rt3.bottom=rt.height;
	rt3.right=rt.width;
	rt3.left=rt.x;
	rt3.top=rt.y;
	if(tab->edytor&&!Video->isfullskreen){
		rt4=rt3;
	}
	else
	{
		int arwidth=rt.height/Video->AR;
		int arheight=rt.width*Video->AR;
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
}

//funkcja zmiany rozdzia³ki okna wideo
void VideoRend::UpdateVideoWindow(bool bar, bool firstload)
{

	
	wxMutexLocker lock(mutexSizing);
	//if(block){while(!block){Sleep(5);}}
	block=true;
	UpdateRects(bar);

	if(!InitDX(true)){return;}

	if(IsDshow&& datas){

		int all=vheight*pitch;
		char *cpy=new char[all];
		byte *cpy1=(byte*)cpy;
		byte *data1=(byte*)datas;
		memcpy(cpy1,data1,all);
		DrawTexture(cpy1);
		delete[] cpy;
	}


	resized=true;
	block=false;
	if(Vclips){
		Vclips->SizeChanged(wxSize(rt3.right, rt3.bottom),lines, m_font, d3device);
		TabPanel* tab=(TabPanel*)GetParent();
		SetVisual(tab->Edit->line->Start.mstime, tab->Edit->line->End.mstime);
	}
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
	if(vstate==Paused && !block){Render(resized);}
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
	//koordynaty bia³ej ramki
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
	//koordynaty paska postêpu
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
			(*txt)<<pan->Edit->dummytext->Trim().AfterLast('\n');
			OpenSubs(txt);VisEdit=false;
		}else if(pan->Edit->OnVideo){OpenSubs(pan->Grid1->SaveText());pan->Edit->OnVideo=false;}
		Render();
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

void VideoRend::SetVisual(int start, int end, bool remove)
{
	TabPanel* pan=(TabPanel*)GetParent();
	SAFE_DELETE(pan->Edit->dummytext);
	if(remove){
		SAFE_DELETE(Vclips); pan->Edit->Visual=0;
		VisEdit=false;
		OpenSubs(pan->Grid1->SaveText());
		Render();
	}
	else{
		if(!Vclips){
			Vclips=new Visuals(this);
			Vclips->SizeChanged(wxSize(rt3.right, rt3.bottom),lines, m_font, d3device);
		}
		Vclips->SetVisual(start, end);
	}
}

bool VideoRend::EnumFilters(wxMenu *menu)
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

void VideoRend::SetEvent(wxMouseEvent& event)
{
	Vclips->MouseEvent(event);
}