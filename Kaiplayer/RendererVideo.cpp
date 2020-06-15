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
	: m_HasZoom(false)
	, videoControl(control)
{
	tab = (TabPanel*)videoControl->GetParent();
	m_HWND = videoControl->GetHWND();

	//---------------------------- format
	m_D3DFormat = D3DFORMAT('2YUY');//D3DFORMAT('21VN');
	//-----------------------------------
	m_Format = NV12;
	m_Time = 0;
	m_PlayEndTime = 0;
	m_State = None;
	m_D3DObject = NULL;
	m_D3DDevice = NULL;
	m_BlackBarsSurface = NULL;
	instance = NULL;
	//vobsub = NULL;
	framee = NULL;
	format = NULL;
	m_D3DLine = NULL;
	m_Visual = NULL;
	m_VideoResized = m_DirectShowSeeking = m_BlockResize = cross = m_HasVisualEdition = false;
	//IsDshow = true;
	m_DeviceLost = false;
	m_MainSurface = NULL;
	m_FrameBuffer = NULL;
	m_AudioPlayer = NULL;
	m_WindowRect.bottom = 0;
	m_WindowRect.right = 0;
	m_WindowRect.left = 0;
	m_WindowRect.top = 0;
	m_D3DFont = NULL;
	m_Frame = 0;
	diff = 0;
	m_AverangeFrameTime = 42;
	m_ZoomParcent = 1.f;
#if byvertices
	vertex = NULL;
	texture = NULL;
#endif
	m_DXVAProcessor = NULL;
	m_DXVAService = NULL;
}

RendererVideo::~RendererVideo()
{
	Stop();

	m_State = None;
	Clear();
	SAFE_DELETE(m_Visual);
	SAFE_DELETE(framee);
	SAFE_DELETE(format);
	if (instance) { csri_close(instance); }

	if (m_FrameBuffer){ delete[] m_FrameBuffer; m_FrameBuffer = NULL; }

}

//sets new rects after change video resolution
bool RendererVideo::UpdateRects(bool changeZoom)
{

	wxRect rt;
	if (videoControl->m_IsFullscreen){
		m_HWND = videoControl->m_FullScreenWindow->GetHWND();
		rt = videoControl->m_FullScreenWindow->GetClientRect();
		if (videoControl->m_PanelOnFullscreen){ rt.height -= videoControl->m_FullScreenWindow->panelsize; }
		videoControl->m_FullScreenProgressBar = Options.GetBool(VIDEO_PROGRESS_BAR);
		cross = false;
	}
	else{
		m_HWND = videoControl->GetHWND();
		rt = videoControl->GetClientRect();
		rt.height -= videoControl->m_PanelHeight;
		videoControl->m_FullScreenProgressBar = false;
	}
	if (!rt.height || !rt.width){ return false; }

	m_WindowRect.bottom = rt.height;
	m_WindowRect.right = rt.width;
	m_WindowRect.left = rt.x;
	m_WindowRect.top = rt.y;

	/*if(tab->editor && !isFullscreen){
	backBufferRect=windowRect;
	}
	else
	{*/
	int arwidth = rt.height / videoControl->m_AspectRatio;
	int arheight = rt.width * videoControl->m_AspectRatio;

	if (arwidth > rt.width)
	{
		int onebar = (rt.height - arheight) / 2;
		//KaiLog(wxString::Format("onebar w %i, h %i, %i", onebar, rt.height, arheight));
		/*if(zoomParcent>1){
		int zoomARHeight = ((zoomRect.width - zoomRect.x)) * AR;
		onebar = (zoomRect.width - zoomRect.x > rt.width)? (rt.height - zoomARHeight)/2 : 0;
		wLogStatus("height %i %i %i, %i", zoomARHeight,arheight,rt.height,onebar);
		}*/
		m_BackBufferRect.bottom = arheight + onebar;
		//if(backBufferRect.bottom % 2 != 0){backBufferRect.bottom++;}
		m_BackBufferRect.right = rt.width;//zostaje bez zmian
		m_BackBufferRect.left = 0;
		m_BackBufferRect.top = onebar;
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
		m_BackBufferRect.bottom = rt.height;//zostaje bez zmian
		m_BackBufferRect.right = arwidth + onebar;
		//if(backBufferRect.right % 2 != 0){backBufferRect.right++;}
		m_BackBufferRect.left = onebar;
		m_BackBufferRect.top = 0;
	}
	else
	{
		//KaiLog(wxString::Format("equal %i %i", windowRect.right, windowRect.bottom));
		m_BackBufferRect = m_WindowRect;
	}
	//}
	if (/*zoomRect.width>0 && */changeZoom){
		wxSize s(m_BackBufferRect.right, m_BackBufferRect.bottom);
		float videoToScreenX = (float)s.x / (float)m_Width;
		float videoToScreenY = (float)s.y / (float)m_Height;
		m_ZoomRect.x = (m_MainStreamRect.left * videoToScreenX) + m_BackBufferRect.left;
		m_ZoomRect.y = (m_MainStreamRect.top * videoToScreenY) + m_BackBufferRect.top;
		m_ZoomRect.height = (m_MainStreamRect.bottom * videoToScreenY);
		m_ZoomRect.width = (m_MainStreamRect.right * videoToScreenX);
		if (m_Visual){
			SetVisualZoom();
		}
	}
	return true;
}

void RendererVideo::UpdateVideoWindow()
{

	wxCriticalSectionLocker lock(m_MutexRendering);
	if (!UpdateRects()){ return; }

	if (!InitDX(true)){
		//need tests, if lost device return any error when reseting or not
		Clear();
		if (!InitDX()){
			return;
		}
	}

	if (m_FrameBuffer){
		RecreateSurface();
	}


	m_VideoResized = true;
	if (m_Visual){
		m_Visual->SizeChanged(wxRect(m_BackBufferRect.left, m_BackBufferRect.top, m_BackBufferRect.right, m_BackBufferRect.bottom), m_D3DLine, m_D3DFont, m_D3DDevice);
		SAFE_DELETE(m_Visual->dummytext);
		m_Visual->SetCurVisual();
		m_HasVisualEdition = true;
	}
	videoControl->SetScaleAndZoom();
}

bool RendererVideo::InitDX(bool reset)
{

	if (!reset){
		m_D3DObject = Direct3DCreate9(D3D_SDK_VERSION);
		PTR(m_D3DObject, _("Nie mo¿na utworzyæ obiektu Direct3D"));
	}
	else{
		Clear(false);
	}

	HRESULT hr;

	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.Windowed = TRUE;
	d3dpp.hDeviceWindow = m_HWND;
	d3dpp.BackBufferWidth = m_WindowRect.right;
	d3dpp.BackBufferHeight = m_WindowRect.bottom;
	d3dpp.BackBufferCount = 1;
	d3dpp.SwapEffect = D3DSWAPEFFECT_COPY;//D3DSWAPEFFECT_DISCARD;//D3DSWAPEFFECT_COPY;//
	d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
	d3dpp.Flags = D3DPRESENTFLAG_VIDEO;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;//D3DPRESENT_INTERVAL_DEFAULT;

	if (reset){
		hr = m_D3DDevice->Reset(&d3dpp);
		if (FAILED(hr)){
			KaiLog(_("Nie mo¿na zresetowaæ Direct3D"));
			return false;
		}
	}
	else{
		hr = m_D3DObject->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, m_HWND,
			D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED, &d3dpp, &m_D3DDevice);//| D3DCREATE_FPU_PRESERVE
		if (FAILED(hr)){
			HR(m_D3DObject->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, m_HWND,
				D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED, &d3dpp, &m_D3DDevice),
				_("Nie mo¿na utworzyæ urz¹dzenia D3D9"));
		}
	}

	hr = m_D3DDevice->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, TRUE);
	hr = m_D3DDevice->SetRenderState(D3DRS_ANTIALIASEDLINEENABLE, TRUE);

	hr = m_D3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	hr = m_D3DDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
	hr = m_D3DDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	hr = m_D3DDevice->SetRenderState(D3DRS_DITHERENABLE, TRUE);

	hr = m_D3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	hr = m_D3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	hr = m_D3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	hr = m_D3DDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	hr = m_D3DDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	hr = m_D3DDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_SPECULAR);

	hr = m_D3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	hr = m_D3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	hr = m_D3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	HR(hr, _("Zawiod³o któreœ z ustawieñ DirectX"));

	D3DXMATRIX matOrtho;
	D3DXMATRIX matIdentity;

	D3DXMatrixOrthoOffCenterLH(&matOrtho, 0, m_WindowRect.right, m_WindowRect.bottom, 0, 0.0f, 1.0f);
	D3DXMatrixIdentity(&matIdentity);

	HR(m_D3DDevice->SetTransform(D3DTS_PROJECTION, &matOrtho), _("Nie mo¿na ustawiæ macierzy projekcji"));
	HR(m_D3DDevice->SetTransform(D3DTS_WORLD, &matIdentity), _("Nie mo¿na ustawiæ macierzy œwiata"));
	HR(m_D3DDevice->SetTransform(D3DTS_VIEW, &matIdentity), _("Nie mo¿na ustawiæ macierzy widoku"));

#if byvertices
	hr = m_D3DDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	hr = m_D3DDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

	// Add filtering
	hr = m_D3DDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	hr = m_D3DDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	HR(hr, _("Zawiod³o któreœ z ustawieñ DirectX vertices"));
	HR(m_D3DDevice->CreateTexture(m_Width, m_Height, 1, D3DUSAGE_RENDERTARGET,
		D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &texture, NULL), "Nie mo¿na utworzyæ tekstury");

	HR(texture->GetSurfaceLevel(0, &m_BlackBarsSurface), "nie mo¿na utworzyæ powierzchni");

	HR(m_D3DDevice->CreateOffscreenPlainSurface(m_Width, m_Height, m_D3DFormat, D3DPOOL_DEFAULT, &m_MainSurface, 0), "Nie mo¿na utworzyæ powierzchni");

	HR(m_D3DDevice->CreateVertexBuffer(4 * sizeof(CUSTOMVERTEX), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &vertex, NULL),
		"Nie mo¿na utworzyæ bufora wertex")

		CUSTOMVERTEX* pVertices;
	HR(hr = vertex->Lock(0, 0, (void**)&pVertices, 0), "nie mo¿na zablokowaæ bufora vertex");

	pVertices[0].position = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	pVertices[0].tu = 0.0f;
	pVertices[0].tv = 0.0f;
	pVertices[1].position = D3DXVECTOR3(m_Width, 0.0f, 0.0f);
	pVertices[1].tu = 1.0f;
	pVertices[1].tv = 0.0f;
	pVertices[2].position = D3DXVECTOR3(m_Width, m_Height, 0.0f);
	pVertices[2].tu = 1.0f;
	pVertices[2].tv = 1.0f;
	pVertices[3].position = D3DXVECTOR3(0.0f, m_Height, 0.0f);
	pVertices[3].tu = 0.0f;
	pVertices[3].tv = 1.0f;

	vertex->Unlock();
#endif

	if (!InitRendererDX())
		return false;

#ifndef byvertices
		HR(m_D3DDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &m_BlackBarsSurface), _("Nie mo¿na stworzyæ powierzchni"));

		HR(m_D3DDevice->CreateOffscreenPlainSurface(m_Width, m_Height, m_D3DFormat, D3DPOOL_DEFAULT, &m_MainSurface, 0),
			_("Nie mo¿na stworzyæ plain surface"));//D3DPOOL_DEFAULT
#endif

	HR(D3DXCreateLine(m_D3DDevice, &m_D3DLine), _("Nie mo¿na stworzyæ linii D3DX"));
	HR(D3DXCreateFont(m_D3DDevice, 20, 0, FW_BOLD, 0, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE, L"Tahoma", &m_D3DFont), _("Nie mo¿na stworzyæ czcionki D3DX"));

	return true;
}

void RendererVideo::Clear(bool clearObject)
{
	SAFE_RELEASE(m_MainSurface);
	SAFE_RELEASE(m_BlackBarsSurface);
	SAFE_RELEASE(m_D3DLine);
	SAFE_RELEASE(m_D3DFont);
#if byvertices
	SAFE_RELEASE(vertex);
	SAFE_RELEASE(texture);
#endif
	SAFE_RELEASE(m_DXVAProcessor);
	SAFE_RELEASE(m_DXVAService);
	if (clearObject){
		SAFE_RELEASE(m_D3DDevice);
		SAFE_RELEASE(m_D3DObject);
		m_HasZoom = false;
	}
}

bool RendererVideo::PlayLine(int start, int eend)
{
	int duration = GetDuration();
	if (m_State == None || start >= eend || start >= duration){ return false; }
	if (duration < eend){ eend = duration; }
	SetPosition(start, true, true, false);
	Play(eend);
	return true;
}

void RendererVideo::SetZoom()
{
	if (m_State == None){ return; }
	m_HasZoom = !m_HasZoom;
	if (m_ZoomRect.width < 1){
		m_ZoomRect = FloatRect(m_BackBufferRect.left, m_BackBufferRect.top, m_BackBufferRect.right, m_BackBufferRect.bottom);
	}
	Render();
}

void RendererVideo::ResetZoom()
{
	if (m_State == None){ return; }
	m_ZoomRect = FloatRect(m_BackBufferRect.left, m_BackBufferRect.top, m_BackBufferRect.right, m_BackBufferRect.bottom);
	wxSize size(m_BackBufferRect.right - m_BackBufferRect.left, m_BackBufferRect.bottom - m_BackBufferRect.top);
	float videoToScreenXX = size.x / (float)m_Width;
	float videoToScreenYY = size.y / (float)m_Height;
	m_MainStreamRect.left = (m_ZoomRect.x - m_BackBufferRect.left) / videoToScreenXX;
	m_MainStreamRect.top = (m_ZoomRect.y - m_BackBufferRect.top) / videoToScreenYY;
	m_MainStreamRect.right = (m_ZoomRect.width - m_BackBufferRect.left) / videoToScreenXX;
	m_MainStreamRect.bottom = (m_ZoomRect.height - m_BackBufferRect.top) / videoToScreenYY;
	m_ZoomParcent = size.x / (m_ZoomRect.width - m_ZoomRect.x/* + backBufferRect.left*/);
	Render();
	videoControl->SetScaleAndZoom();
}

void RendererVideo::Zoom(const wxSize &size)
{
	//wxSize s1(backBufferRect.right - backBufferRect.left, backBufferRect.bottom - backBufferRect.top);
	m_HasZoom = true;
	float videoToScreenXX = size.x / (float)m_Width;
	float videoToScreenYY = size.y / (float)m_Height;
	m_MainStreamRect.left = (m_ZoomRect.x - m_BackBufferRect.left) / videoToScreenXX;
	m_MainStreamRect.top = (m_ZoomRect.y - m_BackBufferRect.top) / videoToScreenYY;
	m_MainStreamRect.right = (m_ZoomRect.width - m_BackBufferRect.left) / videoToScreenXX;
	m_MainStreamRect.bottom = (m_ZoomRect.height - m_BackBufferRect.top) / videoToScreenYY;
	m_ZoomParcent = size.x / (m_ZoomRect.width - m_ZoomRect.x/* + backBufferRect.left*/);
	if (videoControl->m_IsFullscreen){ UpdateRects(false); }
	if (m_Visual){
		SetVisualZoom();
		if (m_Visual && (m_Visual->Visual < CLIPRECT || m_Visual->Visual > VECTORDRAW)){
			SAFE_DELETE(m_Visual->dummytext);
			m_Visual->SetCurVisual();
			m_HasVisualEdition = true;
		}
	}
	Render(false);
	videoControl->SetScaleAndZoom();
}

void RendererVideo::SetVisualZoom()
{
	float videoToScreenX = (float)(m_BackBufferRect.right - m_BackBufferRect.left) / (float)(m_Width);
	float videoToScreenY = (float)(m_BackBufferRect.bottom - m_BackBufferRect.top) / (float)(m_Height);
	float zoomX = m_MainStreamRect.left * videoToScreenX;
	float zoomY = m_MainStreamRect.top * videoToScreenY;
	D3DXVECTOR2 zoomScale((float)m_Width / (float)(m_MainStreamRect.right - m_MainStreamRect.left),
		(float)m_Height / (float)(m_MainStreamRect.bottom - m_MainStreamRect.top));
	m_Visual->SetZoom(D3DXVECTOR2(zoomX - (m_BackBufferRect.left / zoomScale.x),
		zoomY - (m_BackBufferRect.top / zoomScale.y)), zoomScale);
}

void RendererVideo::DrawZoom()
{
	D3DXVECTOR2 v2[5];
	wxSize s(m_BackBufferRect.right, m_BackBufferRect.bottom);
	v2[0].x = m_ZoomRect.x;
	v2[0].y = m_ZoomRect.y;
	v2[1].x = v2[0].x;
	v2[1].y = m_ZoomRect.height - 1;
	v2[2].x = m_ZoomRect.width - 1;
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

	HRN(m_D3DDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE), L"FVF failed");
	HRN(m_D3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 4, v24, sizeof(VERTEX)), L"Primitive failed");
	HRN(m_D3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 4, &v24[6], sizeof(VERTEX)), L"Primitive failed");
	m_D3DLine->SetWidth(1);
	m_D3DLine->Begin();
	m_D3DLine->Draw(v2, 5, 0xFFBB0000);
	m_D3DLine->End();

}

void RendererVideo::ZoomMouseHandle(wxMouseEvent &evt)
{
	int x = evt.GetX();
	int y = evt.GetY();

	wxSize s(m_BackBufferRect.right, m_BackBufferRect.bottom);
	wxSize s1(m_BackBufferRect.right - m_BackBufferRect.left, m_BackBufferRect.bottom - m_BackBufferRect.top);
	float ar = (float)s1.x / (float)s1.y;

	FloatRect tmp = m_ZoomRect;
	
	bool rotation = evt.GetWheelRotation() != 0;

	if (evt.ButtonUp()){
		if (videoControl->HasCapture()){ videoControl->ReleaseMouse(); }
		if (!videoControl->m_HasArrow){ 
			videoControl->SetCursor(wxCURSOR_ARROW); 
			videoControl->m_HasArrow = true; 
		}
	}


	if (!(evt.LeftDown() || evt.LeftIsDown())){
		bool setarrow = false;

		if (abs(x - m_ZoomRect.x) < 5){
			setarrow = true;
			videoControl->SetCursor(wxCURSOR_SIZEWE);
			videoControl->m_HasArrow = false;
		}
		if (abs(y - m_ZoomRect.y) < 5){
			setarrow = true;
			videoControl->SetCursor(wxCURSOR_SIZENS);
			videoControl->m_HasArrow = false;
		}
		if (abs(x - m_ZoomRect.width) < 5){
			setarrow = true;
			videoControl->SetCursor(wxCURSOR_SIZEWE);
			videoControl->m_HasArrow = false;
		}
		if (abs(y - m_ZoomRect.height) < 5){
			setarrow = true;
			videoControl->SetCursor(wxCURSOR_SIZENS);
			videoControl->m_HasArrow = false;
		}

		if (!setarrow && !videoControl->m_HasArrow){ 
			videoControl->SetCursor(wxCURSOR_ARROW); 
			videoControl->m_HasArrow = true;
		}
	}
	if (evt.LeftDown()){
		if (!videoControl->HasCapture()){ 
			videoControl->CaptureMouse(); 
		}
		m_Grabbed = -1;
		if (abs(x - m_ZoomRect.x) < 5){
			m_ZoomDiff.x = m_ZoomRect.x - x;
			m_Grabbed = 0;
		}
		else if (abs(y - m_ZoomRect.y) < 5){
			m_ZoomDiff.y = m_ZoomRect.y - y;
			m_Grabbed = 1;
		}
		else if (abs(x - m_ZoomRect.width) < 5){
			m_ZoomDiff.x = m_ZoomRect.width - x;
			m_Grabbed = 2;
		}
		else if (abs(y - m_ZoomRect.height) < 5){
			m_ZoomDiff.y = m_ZoomRect.height - y;
			m_Grabbed = 3;
		}
		else{
			m_ZoomDiff.x = x - m_ZoomRect.x;
			m_ZoomDiff.y = y - m_ZoomRect.y;
		}

	}
	else if (evt.LeftIsDown() || rotation){
		int minx = m_BackBufferRect.left;
		int miny = m_BackBufferRect.top;
		if (rotation){
			int step = 5 * evt.GetWheelRotation() / evt.GetWheelDelta();
			m_ZoomRect.x -= step;
			m_ZoomRect.y -= step / ar;
			m_ZoomRect.width += step;
			m_ZoomRect.height += step / ar;
		}
		else if (m_Grabbed < 0){
			float oldzx = m_ZoomRect.x;
			float oldzy = m_ZoomRect.y;
			if (m_ZoomRect.x >= minx && m_ZoomRect.width < s.x || (m_ZoomRect.width == s.x && m_ZoomRect.x > x - m_ZoomDiff.x)){
				m_ZoomRect.x = x - m_ZoomDiff.x;
			}
			if (m_ZoomRect.y >= miny && m_ZoomRect.height < s.y || (m_ZoomRect.height == s.y && m_ZoomRect.y > y - m_ZoomDiff.y)){
				m_ZoomRect.y = y - m_ZoomDiff.y;
			}
			if (m_ZoomRect.x >= minx && m_ZoomRect.width <= s.x){
				m_ZoomRect.width += (m_ZoomRect.x - oldzx);
			}
			if (m_ZoomRect.y >= miny && m_ZoomRect.height <= s.y){
				m_ZoomRect.height += (m_ZoomRect.y - oldzy);
			}
			m_ZoomRect.x = MID(minx, m_ZoomRect.x, s.x);
			m_ZoomRect.y = MID(miny, m_ZoomRect.y, s.y);
			m_ZoomRect.width = MIN(m_ZoomRect.width, s.x);
			m_ZoomRect.height = MIN(m_ZoomRect.height, s.y);
			Zoom(s1);
			return;
		}
		else if (m_Grabbed < 2){
			if (m_Grabbed == 0){
				float oldzx = m_ZoomRect.x;
				m_ZoomRect.x = x - m_ZoomDiff.x;
				//if(zoomRect.x<minx){zoomRect = FloatRect(minx, miny, s.x, s.y);Zoom(s);return;}
				m_ZoomRect.y += (m_ZoomRect.x - oldzx) / ar;
				m_ZoomRect.width -= (m_ZoomRect.x - oldzx);
				m_ZoomRect.height -= (m_ZoomRect.x - oldzx) / ar;
			}
			else{
				float oldzy = m_ZoomRect.y;
				m_ZoomRect.y = y - m_ZoomDiff.y;
				//if(zoomRect.y<miny){zoomRect = FloatRect(minx, miny, s.x, s.y);Zoom(s);return;}
				m_ZoomRect.x += (m_ZoomRect.y - oldzy) * ar;
				m_ZoomRect.height -= (m_ZoomRect.y - oldzy);
				m_ZoomRect.width -= (m_ZoomRect.y - oldzy) * ar;
			}
		}
		else{
			if (m_Grabbed == 2){
				//if(zoomRect.width - zoomRect.x < 100 || (zoomRect.width - zoomRect.x == 100 && zoomRect.x < x - zoomDiff.x) ){return;}
				float oldzw = m_ZoomRect.width;
				m_ZoomRect.width = (x - m_ZoomDiff.x);
				m_ZoomRect.height += (m_ZoomRect.width - oldzw) / ar;
			}
			else{
				//if(zoomRect.width - zoomRect.x < 100 || (zoomRect.width - zoomRect.x == 100 && zoomRect.y < y - zoomDiff.y) ){return;}
				float oldzh = m_ZoomRect.height;
				m_ZoomRect.height = (y - m_ZoomDiff.y);
				m_ZoomRect.width += (m_ZoomRect.height - oldzh) * ar;
			}

		}
		if (m_ZoomRect.width > s.x){
			m_ZoomRect.x -= m_ZoomRect.width - s.x;
			m_ZoomRect.width = s.x;
		}
		if (m_ZoomRect.height > s.y){
			m_ZoomRect.y -= m_ZoomRect.height - s.y;
			m_ZoomRect.height = s.y;
		}
		if (m_ZoomRect.x < minx){
			m_ZoomRect.width -= (m_ZoomRect.x - minx);
			m_ZoomRect.x = minx;
		}
		if (m_ZoomRect.y < miny){
			m_ZoomRect.height -= (m_ZoomRect.y - miny);
			m_ZoomRect.y = miny;
		}

		m_ZoomRect.width = MIN(m_ZoomRect.width, s.x);
		m_ZoomRect.height = MIN(m_ZoomRect.height, s.y);
		m_ZoomRect.x = MID(minx, m_ZoomRect.x, s.x);
		m_ZoomRect.y = MID(miny, m_ZoomRect.y, s.y);
		if (m_ZoomRect.width - m_ZoomRect.x < 100){
			m_ZoomRect = tmp;
		}
		Zoom(s1);
	}

}

void RendererVideo::DrawProgBar()
{
	//Full screen progress bar position
	wxMutexLocker lock(m_MutexProgressBar);
	int w, h;
	VideoCtrl *vb = (VideoCtrl*)this;
	vb->m_FullScreenWindow->GetClientSize(&w, &h);
	m_ProgressBarRect.top = 16;
	m_ProgressBarRect.bottom = 60;
	m_ProgressBarRect.left = w - 167;
	m_ProgressBarRect.right = w - 3;
	//coordinates of black frame
	vectors[0].x = w - 170;
	vectors[0].y = 5;
	vectors[1].x = w - 5;
	vectors[1].y = 5;
	vectors[2].x = w - 5;
	vectors[2].y = 15;
	vectors[3].x = w - 170;
	vectors[3].y = 15;
	vectors[4].x = w - 170;
	vectors[4].y = 5;
	//coordinates of white frame
	vectors[5].x = w - 169;
	vectors[5].y = 6;
	vectors[6].x = w - 6;
	vectors[6].y = 6;
	vectors[7].x = w - 6;
	vectors[7].y = 14;
	vectors[8].x = w - 169;
	vectors[8].y = 14;
	vectors[9].x = w - 169;
	vectors[9].y = 6;
	//coordinates of progress bar
	int rw = w - 168;
	vectors[10].x = rw;
	vectors[10].y = 10.5;
	int Duration = GetDuration();
	vectors[11].x = (Duration > 0) ? (((float)m_Time / (float)Duration) * 161) + rw : 161 + rw;
	vectors[11].y = 10.5;
}

void RendererVideo::SetVisual(bool settext/*=false*/, bool noRefresh /*= false*/)
{
	wxMutexLocker lock(m_MutexVisualChange);
	
	m_HasVisualEdition = false;
	int vis = tab->Edit->Visual;
	if (!m_Visual){
		m_Visual = Visuals::Get(vis, videoControl);
	}
	else if (m_Visual->Visual != vis){
		bool vectorclip = m_Visual->Visual == VECTORCLIP;
		delete m_Visual;
		m_Visual = Visuals::Get(vis, videoControl);
		if (vectorclip && !settext){ OpenSubs(tab->Grid->GetVisible()); }
	}
	else{ SAFE_DELETE(m_Visual->dummytext); }
	if (settext){ OpenSubs(tab->Grid->GetVisible()); }
	m_Visual->SizeChanged(wxRect(m_BackBufferRect.left, m_BackBufferRect.top,
		m_BackBufferRect.right, m_BackBufferRect.bottom), m_D3DLine, m_D3DFont, m_D3DDevice);
	SetVisualZoom();
	m_Visual->SetVisual(tab->Edit->line->Start.mstime, tab->Edit->line->End.mstime,
		tab->Edit->line->IsComment, noRefresh);
	m_HasVisualEdition = true;
}

void RendererVideo::ResetVisual()
{
	wxMutexLocker lock(m_MutexVisualChange);
	m_HasVisualEdition = false;
	SAFE_DELETE(m_Visual->dummytext);
	m_Visual->SetCurVisual();
	m_HasVisualEdition = true;
	Render();
}

bool RendererVideo::RemoveVisual(bool noRefresh)
{
	wxMutexLocker lock(m_MutexVisualChange);
	m_HasVisualEdition = false;
	SAFE_DELETE(m_Visual);
	tab->Edit->Visual = 0;
	if (!noRefresh){
		OpenSubs(tab->Grid->GetVisible());
		Render();
	}
	return true;
}

bool RendererVideo::DrawTexture(byte *nframe, bool copy)
{

	wxCriticalSectionLocker lock(m_MutexRendering);
	byte *fdata = NULL;
	byte *texbuf;
	byte bytes = (m_Format == RGB32) ? 4 : (m_Format == YUY2) ? 2 : 1;
	//DWORD black = (vformat == RGB32) ? 0 : (vformat == YUY2) ? 0x80108010 : 0x10101010;
	//DWORD blackuv = (vformat == RGB32) ? 0 : (vformat == YUY2) ? 0x80108010 : 0x8080;

	D3DLOCKED_RECT d3dlr;

	if (nframe){
		fdata = nframe;
		if (copy){
			byte *cpy = (byte*)m_FrameBuffer;
			memcpy(cpy, fdata, m_Height * m_Pitch);
		}
	}
	else{
		KaiLog(_("Brak bufora klatki")); return false;
	}


	if (instance){
		//for swap -pitch and buffer set to last element - pitch
		framee->strides[0] = (m_SwapFrame) ? -(m_Width * bytes) : m_Width * bytes;
		framee->planes[0] = (m_SwapFrame) ? fdata + (m_Width * (m_Height - 1) * bytes) : fdata;
		csri_render(instance, framee, (m_Time / 1000.0));
	}


#ifdef byvertices
	HR(m_MainSurface->LockRect(&d3dlr, 0, 0), _("Nie mo¿na zablokowaæ bufora tekstury"));//D3DLOCK_NOSYSLOCK
#else
	HR(m_MainSurface->LockRect(&d3dlr, 0, D3DLOCK_NOSYSLOCK), _("Nie mo¿na zablokowaæ bufora tekstury"));
#endif
	texbuf = static_cast<byte *>(d3dlr.pBits);

	diff = d3dlr.Pitch - (m_Width*bytes);
	if (m_SwapFrame){
		int framePitch = m_Width * bytes;
		byte * reversebyte = fdata + (framePitch * m_Height) - framePitch;
		for (int j = 0; j < m_Height; ++j){
			memcpy(texbuf, reversebyte, framePitch);
			texbuf += framePitch + diff;
			reversebyte -= framePitch;
		}
	}
	else if (!diff){
		memcpy(texbuf, fdata, (m_Height * m_Pitch));
	}
	else if (diff > 0){

		if (m_Format >= YV12){
			for (int i = 0; i < m_Height; ++i){
				memcpy(texbuf, fdata, m_Width);
				texbuf += (m_Width + diff);
				fdata += m_Width;
			}
			int hheight = m_Height / 2;
			int fwidth = (m_Format == NV12) ? m_Width : m_Width / 2;
			int fdiff = (m_Format == NV12) ? diff : diff / 2;

			for (int i = 0; i < hheight; i++){
				memcpy(texbuf, fdata, fwidth);
				texbuf += (fwidth + fdiff);
				fdata += fwidth;
			}
			if (m_Format < NV12){
				for (int i = 0; i < hheight; ++i){
					memcpy(texbuf, fdata, fwidth);
					texbuf += (fwidth + fdiff);
					fdata += fwidth;
				}
			}
		}
		else
		{
			int fwidth = m_Width * bytes;
			for (int i = 0; i < m_Height; i++){
				memcpy(texbuf, fdata, fwidth);
				texbuf += (fwidth + diff);
				fdata += fwidth;
			}
		}

	}
	else{
		KaiLog(wxString::Format(L"bad pitch diff %i pitch %i dxpitch %i", diff, m_Pitch, d3dlr.Pitch));
	}

	m_MainSurface->UnlockRect();

	return true;
}


int RendererVideo::GetCurrentPosition()
{
	return m_Time;
}

int RendererVideo::GetCurrentFrame()
{
	return m_Frame;
}

void RendererVideo::VisualChangeTool(int tool)
{
	if (m_Visual)
		m_Visual->ChangeTool(tool);
}

bool RendererVideo::HasVisual()
{
	return m_Visual != NULL;
}

Visuals *RendererVideo::GetVisual()
{
	return m_Visual;
}

