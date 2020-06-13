//  Copyright (c) 2020, Marcin Drob

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

#include "RendererVideo.h"
#include "kainoteMain.h"
#include "CsriMod.h"

#if byvertices
struct CUSTOMVERTEX
{
	D3DXVECTOR3 position; // The position
	FLOAT       tu, tv;   // The texture coordinates
};
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_TEX1)
#endif

void CreateVERTEX(VERTEX *v, float X, float Y, D3DCOLOR Color, float Z)
{
	v->fX = X;
	v->fY = Y;
	v->fZ = Z;
	v->Color = Color;
}

RendererVideo::RendererVideo(VideoCtrl *control)
	: panelHeight(44)
	, AR(0.0)
	, fps(0.0)
	, isFullscreen(false)
	, hasZoom(false)
	, videoControl(control)
{
	hwnd = videoControl->GetHWND();

	//---------------------------- format
	d3dformat = D3DFORMAT('2YUY');//D3DFORMAT('21VN');
	//-----------------------------------
	vformat = NV12;
	time = 0;
	playend = 0;
	vstate = None;
	d3dobject = NULL;
	d3device = NULL;
	bars = NULL;
	VFF = NULL;
	instance = NULL;
	//vobsub = NULL;
	framee = NULL;
	format = NULL;
	lines = NULL;
	Visual = NULL;
	resized = seek = block = cross = fullScreenProgressBar = hasVisualEdition = false;
	//IsDshow = true;
	devicelost = false;
	panelOnFullscreen = false;
	MainStream = NULL;
	frameBuffer = NULL;
	player = NULL;
	vplayer = NULL;
	windowRect.bottom = 0;
	windowRect.right = 0;
	windowRect.left = 0;
	windowRect.top = 0;
	m_font = NULL;
	numframe = 0;
	diff = 0;
	avframetime = 42;
	zoomParcent = 1.f;
#if byvertices
	vertex = NULL;
	texture = NULL;
#endif
	dxvaProcessor = NULL;
	dxvaService = NULL;
}

RendererVideo::~RendererVideo()
{
	Stop();

	vstate = None;

	SAFE_DELETE(VFF);
	Clear();
	SAFE_DELETE(Visual);
	SAFE_DELETE(vplayer);
	SAFE_DELETE(framee);
	SAFE_DELETE(format);
	if (instance) { csri_close(instance); }

	if (frameBuffer){ delete[] frameBuffer; frameBuffer = NULL; }

}

//sets new rects after change video resolution
bool RendererVideo::UpdateRects(bool changeZoom)
{

	wxRect rt;
	TabPanel* tab = (TabPanel*)videoControl->GetParent();
	if (isFullscreen){
		hwnd = videoControl->TD->GetHWND();
		rt = videoControl->TD->GetClientRect();
		if (panelOnFullscreen){ rt.height -= videoControl->TD->panelsize; }
		fullScreenProgressBar = Options.GetBool(VIDEO_PROGRESS_BAR);
		cross = false;
	}
	else{
		hwnd = videoControl->GetHWND();
		rt = videoControl->GetClientRect();
		rt.height -= panelHeight;
		fullScreenProgressBar = false;
	}
	if (!rt.height || !rt.width){ return false; }

	windowRect.bottom = rt.height;
	windowRect.right = rt.width;
	windowRect.left = rt.x;
	windowRect.top = rt.y;

	/*if(tab->editor && !isFullscreen){
	backBufferRect=windowRect;
	}
	else
	{*/
	int arwidth = rt.height / AR;
	int arheight = rt.width * AR;

	if (arwidth > rt.width)
	{
		int onebar = (rt.height - arheight) / 2;
		//KaiLog(wxString::Format("onebar w %i, h %i, %i", onebar, rt.height, arheight));
		/*if(zoomParcent>1){
		int zoomARHeight = ((zoomRect.width - zoomRect.x)) * AR;
		onebar = (zoomRect.width - zoomRect.x > rt.width)? (rt.height - zoomARHeight)/2 : 0;
		wLogStatus("height %i %i %i, %i", zoomARHeight,arheight,rt.height,onebar);
		}*/
		backBufferRect.bottom = arheight + onebar;
		//if(backBufferRect.bottom % 2 != 0){backBufferRect.bottom++;}
		backBufferRect.right = rt.width;//zostaje bez zmian
		backBufferRect.left = 0;
		backBufferRect.top = onebar;
	}
	else if (arheight > rt.height)
	{
		int onebar = (rt.width - arwidth) / 2;
		//KaiLog(wxString::Format("onebar w %i, h %i, %i", onebar, rt.width, arwidth));
		/*if(zoomParcent>1){
		int zoomARWidth = ((zoomRect.height - zoomRect.y)) / AR;
		onebar = (zoomRect.height - zoomRect.y > rt.height)? (rt.width - zoomARWidth)/2 : 0;
		wLogStatus("width %i %i %i, %i", zoomARWidth,arwidth,rt.width,onebar);
		}*/
		backBufferRect.bottom = rt.height;//zostaje bez zmian
		backBufferRect.right = arwidth + onebar;
		//if(backBufferRect.right % 2 != 0){backBufferRect.right++;}
		backBufferRect.left = onebar;
		backBufferRect.top = 0;
	}
	else
	{
		//KaiLog(wxString::Format("equal %i %i", windowRect.right, windowRect.bottom));
		backBufferRect = windowRect;
	}
	//}
	if (/*zoomRect.width>0 && */changeZoom){
		wxSize s(backBufferRect.right, backBufferRect.bottom);
		float videoToScreenX = (float)s.x / (float)vwidth;
		float videoToScreenY = (float)s.y / (float)vheight;
		zoomRect.x = (mainStreamRect.left * videoToScreenX) + backBufferRect.left;
		zoomRect.y = (mainStreamRect.top * videoToScreenY) + backBufferRect.top;
		zoomRect.height = (mainStreamRect.bottom * videoToScreenY);
		zoomRect.width = (mainStreamRect.right * videoToScreenX);
		if (Visual){
			SetVisualZoom();
		}
	}
	return true;
}

void RendererVideo::UpdateVideoWindow()
{

	wxCriticalSectionLocker lock(mutexRender);
	if (!UpdateRects()){ return; }

	if (!InitDX(true)){
		//need tests, if lost device return any error when reseting or not
		Clear();
		if (!InitDX()){
			return;
		}
	}

	if (frameBuffer){
		RecreateSurface();
	}


	resized = true;
	if (Visual){
		Visual->SizeChanged(wxRect(backBufferRect.left, backBufferRect.top, backBufferRect.right, backBufferRect.bottom), lines, m_font, d3device);
		SAFE_DELETE(Visual->dummytext);
		Visual->SetCurVisual();
		hasVisualEdition = true;
	}
	SetScaleAndZoom();
}

bool RendererVideo::InitDX(bool reset)
{

	if (!reset){
		d3dobject = Direct3DCreate9(D3D_SDK_VERSION);
		PTR(d3dobject, _("Nie mo¿na utworzyæ obiektu Direct3D"));
	}
	else{
		Clear(false);
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
			KaiLog(_("Nie mo¿na zresetowaæ Direct3D"));
			return false;
		}
	}
	else{
		hr = d3dobject->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd,
			D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED, &d3dpp, &d3device);//| D3DCREATE_FPU_PRESERVE
		if (FAILED(hr)){
			HR(d3dobject->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd,
				D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED, &d3dpp, &d3device),
				_("Nie mo¿na utworzyæ urz¹dzenia D3D9"));
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
	HR(hr, _("Zawiod³o któreœ z ustawieñ DirectX"));

	D3DXMATRIX matOrtho;
	D3DXMATRIX matIdentity;

	D3DXMatrixOrthoOffCenterLH(&matOrtho, 0, windowRect.right, windowRect.bottom, 0, 0.0f, 1.0f);
	D3DXMatrixIdentity(&matIdentity);

	HR(d3device->SetTransform(D3DTS_PROJECTION, &matOrtho), _("Nie mo¿na ustawiæ macierzy projekcji"));
	HR(d3device->SetTransform(D3DTS_WORLD, &matIdentity), _("Nie mo¿na ustawiæ macierzy œwiata"));
	HR(d3device->SetTransform(D3DTS_VIEW, &matIdentity), _("Nie mo¿na ustawiæ macierzy widoku"));

#if byvertices
	hr = d3device->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	hr = d3device->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

	// Add filtering
	hr = d3device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	hr = d3device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	HR(hr, _("Zawiod³o któreœ z ustawieñ DirectX vertices"));
	HR(d3device->CreateTexture(vwidth, vheight, 1, D3DUSAGE_RENDERTARGET,
		D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &texture, NULL), "Nie mo¿na utworzyæ tekstury");

	HR(texture->GetSurfaceLevel(0, &bars), "nie mo¿na utworzyæ powierzchni");

	HR(d3device->CreateOffscreenPlainSurface(vwidth, vheight, d3dformat, D3DPOOL_DEFAULT, &MainStream, 0), "Nie mo¿na utworzyæ powierzchni");

	HR(d3device->CreateVertexBuffer(4 * sizeof(CUSTOMVERTEX), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &vertex, NULL),
		"Nie mo¿na utworzyæ bufora wertex")

		CUSTOMVERTEX* pVertices;
	HR(hr = vertex->Lock(0, 0, (void**)&pVertices, 0), "nie mo¿na zablokowaæ bufora vertex");

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

	InitRendererDX();

#ifndef byvertices
		HR(d3device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &bars), _("Nie mo¿na stworzyæ powierzchni"));

		HR(d3device->CreateOffscreenPlainSurface(vwidth, vheight, d3dformat, D3DPOOL_DEFAULT, &MainStream, 0),
			_("Nie mo¿na stworzyæ plain surface"));//D3DPOOL_DEFAULT
#endif

	HR(D3DXCreateLine(d3device, &lines), _("Nie mo¿na stworzyæ linii D3DX"));
	HR(D3DXCreateFont(d3device, 20, 0, FW_BOLD, 0, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE, L"Tahoma", &m_font), _("Nie mo¿na stworzyæ czcionki D3DX"));

	return true;
}

void RendererVideo::Clear(bool clearObject)
{
	SAFE_RELEASE(MainStream);
	SAFE_RELEASE(bars);
	SAFE_RELEASE(lines);
	SAFE_RELEASE(m_font);
#if byvertices
	SAFE_RELEASE(vertex);
	SAFE_RELEASE(texture);
#endif
	SAFE_RELEASE(dxvaProcessor);
	SAFE_RELEASE(dxvaService);
	if (clearObject){
		SAFE_RELEASE(d3device);
		SAFE_RELEASE(d3dobject);
		hasZoom = false;
	}
}

bool RendererVideo::PlayLine(int start, int eend)
{
	int duration = GetDuration();
	if (vstate == None || start >= eend || start >= duration){ return false; }
	if (duration < eend){ eend = duration; }
	SetPosition(start, true, true, false);
	Play(eend);
	return true;
}

void RendererVideo::SetZoom()
{
	if (vstate == None){ return; }
	hasZoom = !hasZoom;
	if (zoomRect.width < 1){
		zoomRect = FloatRect(backBufferRect.left, backBufferRect.top, backBufferRect.right, backBufferRect.bottom);
	}
	Render();
}

void RendererVideo::ResetZoom()
{
	if (vstate == None){ return; }
	zoomRect = FloatRect(backBufferRect.left, backBufferRect.top, backBufferRect.right, backBufferRect.bottom);
	wxSize size(backBufferRect.right - backBufferRect.left, backBufferRect.bottom - backBufferRect.top);
	float videoToScreenXX = size.x / (float)vwidth;
	float videoToScreenYY = size.y / (float)vheight;
	mainStreamRect.left = (zoomRect.x - backBufferRect.left) / videoToScreenXX;
	mainStreamRect.top = (zoomRect.y - backBufferRect.top) / videoToScreenYY;
	mainStreamRect.right = (zoomRect.width - backBufferRect.left) / videoToScreenXX;
	mainStreamRect.bottom = (zoomRect.height - backBufferRect.top) / videoToScreenYY;
	zoomParcent = size.x / (zoomRect.width - zoomRect.x/* + backBufferRect.left*/);
	Render();
	SetScaleAndZoom();
}

void RendererVideo::Zoom(const wxSize &size)
{
	//wxSize s1(backBufferRect.right - backBufferRect.left, backBufferRect.bottom - backBufferRect.top);
	hasZoom = true;
	float videoToScreenXX = size.x / (float)vwidth;
	float videoToScreenYY = size.y / (float)vheight;
	mainStreamRect.left = (zoomRect.x - backBufferRect.left) / videoToScreenXX;
	mainStreamRect.top = (zoomRect.y - backBufferRect.top) / videoToScreenYY;
	mainStreamRect.right = (zoomRect.width - backBufferRect.left) / videoToScreenXX;
	mainStreamRect.bottom = (zoomRect.height - backBufferRect.top) / videoToScreenYY;
	zoomParcent = size.x / (zoomRect.width - zoomRect.x/* + backBufferRect.left*/);
	if (isFullscreen){ UpdateRects(false); }
	if (Visual){
		SetVisualZoom();
		if (Visual && (Visual->Visual < CLIPRECT || Visual->Visual > VECTORDRAW)){
			SAFE_DELETE(Visual->dummytext);
			Visual->SetCurVisual();
			hasVisualEdition = true;
		}
	}
	Render(false);
	SetScaleAndZoom();
}

void RendererVideo::SetVisualZoom()
{
	float videoToScreenX = (float)(backBufferRect.right - backBufferRect.left) / (float)(vwidth);
	float videoToScreenY = (float)(backBufferRect.bottom - backBufferRect.top) / (float)(vheight);
	float zoomX = mainStreamRect.left * videoToScreenX;
	float zoomY = mainStreamRect.top * videoToScreenY;
	D3DXVECTOR2 zoomScale((float)vwidth / (float)(mainStreamRect.right - mainStreamRect.left),
		(float)vheight / (float)(mainStreamRect.bottom - mainStreamRect.top));
	Visual->SetZoom(D3DXVECTOR2(zoomX - (backBufferRect.left / zoomScale.x),
		zoomY - (backBufferRect.top / zoomScale.y)), zoomScale);
}

void RendererVideo::DrawZoom()
{
	D3DXVECTOR2 v2[5];
	wxSize s(backBufferRect.right, backBufferRect.bottom);
	v2[0].x = zoomRect.x;
	v2[0].y = zoomRect.y;
	v2[1].x = v2[0].x;
	v2[1].y = zoomRect.height - 1;
	v2[2].x = zoomRect.width - 1;
	v2[2].y = v2[1].y;
	v2[3].x = v2[2].x;
	v2[3].y = v2[0].y;
	v2[4].x = v2[0].x;
	v2[4].y = v2[0].y;


	VERTEX v24[12];
	CreateVERTEX(&v24[0], 0, 0, 0x88000000);
	CreateVERTEX(&v24[1], s.x, 0, 0x88000000);
	CreateVERTEX(&v24[2], v2[2].x, v2[0].y, 0x88000000);
	CreateVERTEX(&v24[3], v2[0].x, v2[0].y, 0x88000000);
	CreateVERTEX(&v24[4], v2[0].x, v2[2].y, 0x88000000);
	CreateVERTEX(&v24[5], 0, s.y, 0x88000000);
	CreateVERTEX(&v24[6], s.x, s.y, 0x88000000);
	CreateVERTEX(&v24[7], 0, s.y, 0x88000000);
	CreateVERTEX(&v24[8], v2[0].x, v2[2].y, 0x88000000);
	CreateVERTEX(&v24[9], v2[2].x, v2[2].y, 0x88000000);
	CreateVERTEX(&v24[10], v2[2].x, v2[0].y, 0x88000000);
	CreateVERTEX(&v24[11], s.x, 0, 0x88000000);

	HRN(d3device->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE), L"FVF failed");
	HRN(d3device->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 4, v24, sizeof(VERTEX)), L"Primitive failed");
	HRN(d3device->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 4, &v24[6], sizeof(VERTEX)), L"Primitive failed");
	lines->SetWidth(1);
	lines->Begin();
	lines->Draw(v2, 5, 0xFFBB0000);
	lines->End();

}

void RendererVideo::ZoomMouseHandle(wxMouseEvent &evt)
{
	int x = evt.GetX();
	int y = evt.GetY();

	wxSize s(backBufferRect.right, backBufferRect.bottom);
	wxSize s1(backBufferRect.right - backBufferRect.left, backBufferRect.bottom - backBufferRect.top);
	float ar = (float)s1.x / (float)s1.y;

	FloatRect tmp = zoomRect;
	//wxWindow *window = (isFullscreen)? (wxWindow*)((VideoCtrl*)this)->TD : this; 

	bool rotation = evt.GetWheelRotation() != 0;

	if (evt.ButtonUp()){
		if (videoControl->HasCapture()){ videoControl->ReleaseMouse(); }
		if (!videoControl->hasArrow){ 
			videoControl->SetCursor(wxCURSOR_ARROW); 
			videoControl->hasArrow = true; 
		}
	}


	if (!(evt.LeftDown() || evt.LeftIsDown())){
		bool setarrow = false;

		if (abs(x - zoomRect.x) < 5){
			setarrow = true;
			videoControl->SetCursor(wxCURSOR_SIZEWE);
			videoControl->hasArrow = false;
		}
		if (abs(y - zoomRect.y) < 5){
			setarrow = true;
			videoControl->SetCursor(wxCURSOR_SIZENS);
			videoControl->hasArrow = false;
		}
		if (abs(x - zoomRect.width) < 5){
			setarrow = true;
			videoControl->SetCursor(wxCURSOR_SIZEWE);
			videoControl->hasArrow = false;
		}
		if (abs(y - zoomRect.height) < 5){
			setarrow = true;
			videoControl->SetCursor(wxCURSOR_SIZENS);
			videoControl->hasArrow = false;
		}

		if (!setarrow && !videoControl->hasArrow){ 
			videoControl->SetCursor(wxCURSOR_ARROW); 
			videoControl->hasArrow = true;
		}
	}
	if (evt.LeftDown()){
		if (!videoControl->HasCapture()){ 
			videoControl->CaptureMouse(); 
		}
		grabbed = -1;
		if (abs(x - zoomRect.x) < 5){
			zoomDiff.x = zoomRect.x - x;
			grabbed = 0;
		}
		else if (abs(y - zoomRect.y) < 5){
			zoomDiff.y = zoomRect.y - y;
			grabbed = 1;
		}
		else if (abs(x - zoomRect.width) < 5){
			zoomDiff.x = zoomRect.width - x;
			grabbed = 2;
		}
		else if (abs(y - zoomRect.height) < 5){
			zoomDiff.y = zoomRect.height - y;
			grabbed = 3;
		}
		else{
			zoomDiff.x = x - zoomRect.x;
			zoomDiff.y = y - zoomRect.y;
		}

	}
	else if (evt.LeftIsDown() || rotation){
		int minx = backBufferRect.left;
		int miny = backBufferRect.top;
		if (rotation){
			int step = 5 * evt.GetWheelRotation() / evt.GetWheelDelta();
			zoomRect.x -= step;
			zoomRect.y -= step / ar;
			zoomRect.width += step;
			zoomRect.height += step / ar;
		}
		else if (grabbed < 0){
			float oldzx = zoomRect.x;
			float oldzy = zoomRect.y;
			if (zoomRect.x >= minx && zoomRect.width < s.x || (zoomRect.width == s.x && zoomRect.x > x - zoomDiff.x)){
				zoomRect.x = x - zoomDiff.x;
			}
			if (zoomRect.y >= miny && zoomRect.height < s.y || (zoomRect.height == s.y && zoomRect.y > y - zoomDiff.y)){
				zoomRect.y = y - zoomDiff.y;
			}
			if (zoomRect.x >= minx && zoomRect.width <= s.x){
				zoomRect.width += (zoomRect.x - oldzx);
			}
			if (zoomRect.y >= miny && zoomRect.height <= s.y){
				zoomRect.height += (zoomRect.y - oldzy);
			}
			zoomRect.x = MID(minx, zoomRect.x, s.x);
			zoomRect.y = MID(miny, zoomRect.y, s.y);
			zoomRect.width = MIN(zoomRect.width, s.x);
			zoomRect.height = MIN(zoomRect.height, s.y);
			Zoom(s1);
			return;
		}
		else if (grabbed < 2){
			if (grabbed == 0){
				float oldzx = zoomRect.x;
				zoomRect.x = x - zoomDiff.x;
				//if(zoomRect.x<minx){zoomRect = FloatRect(minx, miny, s.x, s.y);Zoom(s);return;}
				zoomRect.y += (zoomRect.x - oldzx) / ar;
				zoomRect.width -= (zoomRect.x - oldzx);
				zoomRect.height -= (zoomRect.x - oldzx) / ar;
			}
			else{
				float oldzy = zoomRect.y;
				zoomRect.y = y - zoomDiff.y;
				//if(zoomRect.y<miny){zoomRect = FloatRect(minx, miny, s.x, s.y);Zoom(s);return;}
				zoomRect.x += (zoomRect.y - oldzy) * ar;
				zoomRect.height -= (zoomRect.y - oldzy);
				zoomRect.width -= (zoomRect.y - oldzy) * ar;
			}
		}
		else{
			if (grabbed == 2){
				//if(zoomRect.width - zoomRect.x < 100 || (zoomRect.width - zoomRect.x == 100 && zoomRect.x < x - zoomDiff.x) ){return;}
				float oldzw = zoomRect.width;
				zoomRect.width = (x - zoomDiff.x);
				zoomRect.height += (zoomRect.width - oldzw) / ar;
			}
			else{
				//if(zoomRect.width - zoomRect.x < 100 || (zoomRect.width - zoomRect.x == 100 && zoomRect.y < y - zoomDiff.y) ){return;}
				float oldzh = zoomRect.height;
				zoomRect.height = (y - zoomDiff.y);
				zoomRect.width += (zoomRect.height - oldzh) * ar;
			}

		}
		if (zoomRect.width > s.x){
			zoomRect.x -= zoomRect.width - s.x;
			zoomRect.width = s.x;
		}
		if (zoomRect.height > s.y){
			zoomRect.y -= zoomRect.height - s.y;
			zoomRect.height = s.y;
		}
		if (zoomRect.x < minx){
			zoomRect.width -= (zoomRect.x - minx);
			zoomRect.x = minx;
		}
		if (zoomRect.y < miny){
			zoomRect.height -= (zoomRect.y - miny);
			zoomRect.y = miny;
		}

		zoomRect.width = MIN(zoomRect.width, s.x);
		zoomRect.height = MIN(zoomRect.height, s.y);
		zoomRect.x = MID(minx, zoomRect.x, s.x);
		zoomRect.y = MID(miny, zoomRect.y, s.y);
		if (zoomRect.width - zoomRect.x < 100){
			zoomRect = tmp;
		}
		Zoom(s1);
	}

}

void RendererVideo::DrawLines(wxPoint point)
{
	wxMutexLocker lock(mutexLines);
	int w, h;
	videoControl->GetClientSize(&w, &h);
	w /= 2; h /= 2;
	crossRect.top = (h > point.y) ? point.y - 12 : point.y - 40;
	crossRect.bottom = (h > point.y) ? point.y + 23 : point.y - 5;
	crossRect.left = (w < point.x) ? point.x - 100 : point.x + 5;
	crossRect.right = (w < point.x) ? point.x - 5 : point.x + 100;

	vectors[0].x = point.x;
	vectors[0].y = 0;
	vectors[1].x = point.x;
	vectors[1].y = backBufferRect.bottom;
	vectors[2].x = 0;
	vectors[2].y = point.y;
	vectors[3].x = backBufferRect.right;
	vectors[3].y = point.y;
	cross = true;
	if (vstate == Paused && !block){
		Render(resized);
	}
}

void RendererVideo::DrawProgBar()
{
	//Full screen progress bar position
	wxMutexLocker lock(mutexProgBar);
	int w, h;
	VideoCtrl *vb = (VideoCtrl*)this;
	vb->TD->GetClientSize(&w, &h);
	progressBarRect.top = 16;
	progressBarRect.bottom = 60;
	progressBarRect.left = w - 167;
	progressBarRect.right = w - 3;
	//coordinates of black frame
	vectors[4].x = w - 170;
	vectors[4].y = 5;
	vectors[5].x = w - 5;
	vectors[5].y = 5;
	vectors[6].x = w - 5;
	vectors[6].y = 15;
	vectors[7].x = w - 170;
	vectors[7].y = 15;
	vectors[8].x = w - 170;
	vectors[8].y = 5;
	//coordinates of white frame
	vectors[9].x = w - 169;
	vectors[9].y = 6;
	vectors[10].x = w - 6;
	vectors[10].y = 6;
	vectors[11].x = w - 6;
	vectors[11].y = 14;
	vectors[12].x = w - 169;
	vectors[12].y = 14;
	vectors[13].x = w - 169;
	vectors[13].y = 6;
	//coordinates of progress bar
	int rw = w - 168;
	vectors[14].x = rw;
	vectors[14].y = 10.5;
	int Duration = GetDuration();
	vectors[15].x = (Duration > 0) ? (((float)time / (float)Duration) * 161) + rw : 161 + rw;
	vectors[15].y = 10.5;
}

void RendererVideo::SetVisual(bool settext/*=false*/, bool noRefresh /*= false*/)
{
	wxMutexLocker lock(mutexVisualChange);
	TabPanel* tab = (TabPanel*)videoControl->GetParent();

	hasVisualEdition = false;
	int vis = tab->Edit->Visual;
	if (!Visual){
		Visual = Visuals::Get(vis, videoControl);
	}
	else if (Visual->Visual != vis){
		bool vectorclip = Visual->Visual == VECTORCLIP;
		delete Visual;
		Visual = Visuals::Get(vis, videoControl);
		if (vectorclip && !settext){ OpenSubs(tab->Grid->GetVisible()); }
	}
	else{ SAFE_DELETE(Visual->dummytext); }
	if (settext){ OpenSubs(tab->Grid->GetVisible()); }
	Visual->SizeChanged(wxRect(backBufferRect.left, backBufferRect.top,
		backBufferRect.right, backBufferRect.bottom), lines, m_font, d3device);
	SetVisualZoom();
	Visual->SetVisual(tab->Edit->line->Start.mstime, tab->Edit->line->End.mstime,
		tab->Edit->line->IsComment, noRefresh);
	hasVisualEdition = true;
}

void RendererVideo::ResetVisual()
{
	wxMutexLocker lock(mutexVisualChange);
	hasVisualEdition = false;
	SAFE_DELETE(Visual->dummytext);
	Visual->SetCurVisual();
	hasVisualEdition = true;
	Render();
}

bool RendererVideo::RemoveVisual(bool noRefresh)
{
	if (!Visual)
		return false;
	wxMutexLocker lock(mutexVisualChange);
	hasVisualEdition = false;
	SAFE_DELETE(Visual);
	TabPanel* tab = (TabPanel*)videoControl->GetParent();
	tab->Edit->Visual = 0;
	if (!noRefresh){
		OpenSubs(tab->Grid->GetVisible());
		Render();
	}
	return true;
}

bool RendererVideo::DrawTexture(byte *nframe, bool copy)
{

	wxCriticalSectionLocker lock(mutexRender);
	byte *fdata = NULL;
	byte *texbuf;
	byte bytes = (vformat == RGB32) ? 4 : (vformat == YUY2) ? 2 : 1;
	//DWORD black = (vformat == RGB32) ? 0 : (vformat == YUY2) ? 0x80108010 : 0x10101010;
	//DWORD blackuv = (vformat == RGB32) ? 0 : (vformat == YUY2) ? 0x80108010 : 0x8080;

	D3DLOCKED_RECT d3dlr;

	if (nframe){
		fdata = nframe;
		if (copy){
			byte *cpy = (byte*)frameBuffer;
			memcpy(cpy, fdata, vheight * pitch);
		}
	}
	else{
		KaiLog(_("Brak bufora klatki")); return false;
	}


	if (instance){
		//for swap -pitch and buffer set to last element - pitch
		framee->strides[0] = (swapFrame) ? -(vwidth * bytes) : vwidth * bytes;
		framee->planes[0] = (swapFrame) ? fdata + (vwidth * (vheight - 1) * bytes) : fdata;
		csri_render(instance, framee, (time / 1000.0));
	}


#ifdef byvertices
	HR(MainStream->LockRect(&d3dlr, 0, 0), _("Nie mo¿na zablokowaæ bufora tekstury"));//D3DLOCK_NOSYSLOCK
#else
	HR(MainStream->LockRect(&d3dlr, 0, D3DLOCK_NOSYSLOCK), _("Nie mo¿na zablokowaæ bufora tekstury"));
#endif
	texbuf = static_cast<byte *>(d3dlr.pBits);

	diff = d3dlr.Pitch - (vwidth*bytes);
	if (swapFrame){
		int framePitch = vwidth * bytes;
		byte * reversebyte = fdata + (framePitch * vheight) - framePitch;
		for (int j = 0; j < vheight; ++j){
			memcpy(texbuf, reversebyte, framePitch);
			texbuf += framePitch + diff;
			reversebyte -= framePitch;
		}
	}
	else if (!diff){
		memcpy(texbuf, fdata, (vheight * pitch));
	}
	else if (diff > 0){

		if (vformat >= YV12){
			for (int i = 0; i < vheight; ++i){
				memcpy(texbuf, fdata, vwidth);
				texbuf += (vwidth + diff);
				fdata += vwidth;
			}
			int hheight = vheight / 2;
			int fwidth = (vformat == NV12) ? vwidth : vwidth / 2;
			int fdiff = (vformat == NV12) ? diff : diff / 2;

			for (int i = 0; i < hheight; i++){
				memcpy(texbuf, fdata, fwidth);
				texbuf += (fwidth + fdiff);
				fdata += fwidth;
			}
			if (vformat < NV12){
				for (int i = 0; i < hheight; ++i){
					memcpy(texbuf, fdata, fwidth);
					texbuf += (fwidth + fdiff);
					fdata += fwidth;
				}
			}
		}
		else
		{
			int fwidth = vwidth * bytes;
			for (int i = 0; i < vheight; i++){
				memcpy(texbuf, fdata, fwidth);
				texbuf += (fwidth + diff);
				fdata += fwidth;
			}
		}

	}
	else{
		KaiLog(wxString::Format(L"bad pitch diff %i pitch %i dxpitch %i", diff, pitch, d3dlr.Pitch));
	}

	MainStream->UnlockRect();

	return true;
}
