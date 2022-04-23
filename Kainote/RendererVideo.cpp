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
#include "VideoToolbar.h"
#include "kainoteFrame.h"
#include "CsriMod.h"
#include "DshowRenderer.h"
#include "RendererFFMS2.h"
#include "Provider.h"

#include <wx/dir.h>
#include <wx/clipbrd.h>

#include <vector>
#include <dxva2api.h>


void CreateVERTEX(vertex *v, float X, float Y, D3DXCOLOR* Color, float Z)
{
	v->floatX = X;
	v->floatX = Y;
	v->floatX = Z;
	v->Color = Color;
}

RendererVideo::RendererVideo(VideoBox *control, bool visualDisabled)
	: m_HasZoom(false)
	, videoControl(control)
{
	tab = (TabPanel*)videoControl->GetParent();
	m_SubsProvider = SubtitlesProviderManager::Get();
	m_HWND = videoControl->GetHWND();

	//---------------------------- format
	m_D3DFormat = D3DFORMAT('2YUY');//D3DFORMAT('21VN');
	//-----------------------------------
	m_Format = NV12;
	m_Time = 0;
	m_PlayEndTime = 0;
	m_State = None;
	m_D3DObject = nullptr;
	m_D3DDevice = nullptr;
	m_BlackBarsSurface = nullptr;
	m_D3DLine = nullptr;
	m_Visual = (tab->editor && !visualDisabled)? Visuals::Get(CROSS, videoControl) : nullptr;
	m_VideoResized = m_DirectShowSeeking = m_BlockResize = m_HasVisualEdition = false;
	m_DeviceLost = false;
	m_MainSurface = nullptr;
	m_FrameBuffer = nullptr;
	m_AudioPlayer = nullptr;
	m_WindowRect.bottom = 0;
	m_WindowRect.right = 0;
	m_WindowRect.left = 0;
	m_WindowRect.top = 0;
	m_D3DFont = nullptr;
	m_Frame = 0;
	diff = 0;
	m_AverangeFrameTime = 42;
	m_ZoomParcent = 1.f;

	m_DXVAProcessor = nullptr;
	m_DXVAService = nullptr;
}

RendererVideo::~RendererVideo()
{

	Clear();
	SAFE_DELETE(m_Visual);
	SAFE_RELEASE(m_SubsProvider);

	if (m_FrameBuffer){ delete[] m_FrameBuffer; m_FrameBuffer = nullptr; }

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
		
		m_BackBufferRect.bottom = arheight + onebar;
		m_BackBufferRect.right = rt.width;//zostaje bez zmian
		m_BackBufferRect.left = 0;
		m_BackBufferRect.top = onebar;
	}
	else if (arheight > rt.height)
	{
		int onebar = (rt.width - arwidth) / 2;
		
		m_BackBufferRect.bottom = rt.height;//zostaje bez zmian
		m_BackBufferRect.right = arwidth + onebar;
		m_BackBufferRect.left = onebar;
		m_BackBufferRect.top = 0;
	}
	else
	{
		m_BackBufferRect = m_WindowRect;
	}
	
	if (changeZoom){
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

	if (!InitDX()){
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

bool RendererVideo::InitDX()
{

	if (!m_D3DObject){
		m_D3DObject = Direct3DCreate9(D3D_SDK_VERSION);
		PTR(m_D3DObject, _("Nie można utworzyć obiektu Direct3D"));
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
	d3dpp.SwapEffect = D3DSWAPEFFECT_COPY;//D3DSWAPEFFECT_COPY;//D3DSWAPEFFECT_DISCARD;//
	d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
	d3dpp.Flags = D3DPRESENTFLAG_VIDEO;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;//D3DPRESENT_INTERVAL_DEFAULT;
	d3dpp.EnableAutoDepthStencil = FALSE;
	d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;

	if (m_D3DDevice){
		hr = m_D3DDevice->Reset(&d3dpp);
		if (FAILED(hr)){
			KaiLogSilent(_("Nie można zresetować Direct3D"));
			return false;
		}
	}
	else{
		hr = m_D3DObject->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, m_HWND,
			D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED | D3DCREATE_FPU_PRESERVE/* | D3DCREATE_PUREDEVICE*/, &d3dpp, &m_D3DDevice);
		if (FAILED(hr)){
			HR(m_D3DObject->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, m_HWND,
				D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED | D3DCREATE_FPU_PRESERVE /*| D3DCREATE_PUREDEVICE*/, &d3dpp, &m_D3DDevice),
				_("Nie można utworzyć urządzenia D3D9"));
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
	HR(hr, _("Zawiodło któreś z ustawień DirectX"));

	D3DXMATRIX matOrtho;
	D3DXMATRIX matIdentity;
	//fix to shitty subs on radeons texture is stretched and need filtering linear and looks blured or on filter point are pixelized
	D3DXMatrixOrthoOffCenterLH(&matOrtho, 0.5f, m_WindowRect.right + 0.5f, m_WindowRect.bottom + 0.5f, 0.5f, 0.0f, 1.0f);
	D3DXMatrixIdentity(&matIdentity);

	HR(m_D3DDevice->SetTransform(D3DTS_PROJECTION, &matOrtho), _("Nie można ustawić macierzy projekcji"));
	HR(m_D3DDevice->SetTransform(D3DTS_WORLD, &matIdentity), _("Nie można ustawić macierzy świata"));
	HR(m_D3DDevice->SetTransform(D3DTS_VIEW, &matIdentity), _("Nie można ustawić macierzy widoku"));



	if (!InitRendererDX())
		return false;

	wxFont *font12 = Options.GetFont(4);
	wxSize pixelSize = font12->GetPixelSize();
	HR(D3DXCreateFontW(m_D3DDevice, pixelSize.y, pixelSize.x, FW_BOLD, 0, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE, L"Tahoma", &m_D3DFont), _("Nie można stworzyć czcionki D3DX"));
	HR(D3DXCreateFontW(m_D3DDevice, pixelSize.y, pixelSize.x, FW_BOLD, 0, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE, L"Tahoma", &m_D3DCalcFont), _("Nie można stworzyć czcionki D3DX"));
	HR(D3DXCreateLine(m_D3DDevice, &m_D3DLine), _("Nie można stworzyć linii D3DX"));

	return true;
}

void RendererVideo::Clear(bool clearObject)
{
	SAFE_RELEASE(m_MainSurface);
	SAFE_RELEASE(m_BlackBarsSurface);
	SAFE_RELEASE(m_D3DLine);
	SAFE_RELEASE(m_D3DFont);
	SAFE_RELEASE(m_D3DCalcFont);
	SAFE_RELEASE(m_DXVAProcessor);
	SAFE_RELEASE(m_DXVAService);

	//clear elements in dshow class
	ClearObject();
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
	ZoomChanged();
	Render();
	videoControl->SetScaleAndZoom();
}

void RendererVideo::Zoom(const wxSize &size)
{
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
	ZoomChanged();
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

	D3DXCOLOR color = 0x88000000;
	vertex v24[12];
	CreateVERTEX(&v24[0], 0, 0, &color);
	CreateVERTEX(&v24[1], s.x, 0, &color);
	CreateVERTEX(&v24[2], v2[2].x, v2[0].y, &color);
	CreateVERTEX(&v24[3], v2[0].x, v2[0].y, &color);
	CreateVERTEX(&v24[4], v2[0].x, v2[2].y, &color);
	CreateVERTEX(&v24[5], 0, s.y, &color);
	CreateVERTEX(&v24[6], s.x, s.y, &color);
	CreateVERTEX(&v24[7], 0, s.y, &color);
	CreateVERTEX(&v24[8], v2[0].x, v2[2].y, &color);
	CreateVERTEX(&v24[9], v2[2].x, v2[2].y, &color);
	CreateVERTEX(&v24[10], v2[2].x, v2[0].y, &color);
	CreateVERTEX(&v24[11], s.x, 0, &color);

	HRN(m_D3DDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE), L"FVF failed");
	HRN(m_D3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 4, v24, sizeof(vertex)), L"Primitive failed");
	HRN(m_D3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 4, &v24[6], sizeof(vertex)), L"Primitive failed");
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
		if (!videoControl->HasArrow()){ 
			videoControl->SetCursor(wxCURSOR_ARROW); 
		}
	}


	if (!(evt.LeftDown() || evt.LeftIsDown())){
		bool setarrow = false;

		if (abs(x - m_ZoomRect.x) < 5){
			setarrow = true;
			videoControl->SetCursor(wxCURSOR_SIZEWE);
		}
		if (abs(y - m_ZoomRect.y) < 5){
			setarrow = true;
			videoControl->SetCursor(wxCURSOR_SIZENS);
		}
		if (abs(x - m_ZoomRect.width) < 5){
			setarrow = true;
			videoControl->SetCursor(wxCURSOR_SIZEWE);
		}
		if (abs(y - m_ZoomRect.height) < 5){
			setarrow = true;
			videoControl->SetCursor(wxCURSOR_SIZENS);
		}

		if (!setarrow && !videoControl->HasArrow()){ 
			videoControl->SetCursor(wxCURSOR_ARROW); 
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
			if (m_ZoomRect.x >= minx && m_ZoomRect.width < s.x || 
				(m_ZoomRect.width == s.x && m_ZoomRect.x > x - m_ZoomDiff.x)){
				float zoomwidth = m_ZoomRect.width - m_ZoomRect.x;
				m_ZoomRect.x = x - m_ZoomDiff.x;
				m_ZoomRect.x = MID(minx, m_ZoomRect.x, (s.x - zoomwidth));
				m_ZoomRect.width += (m_ZoomRect.x - oldzx);
			}
			if (m_ZoomRect.y >= miny && m_ZoomRect.height < s.y || 
				(m_ZoomRect.height == s.y && m_ZoomRect.y > y - m_ZoomDiff.y)){
				float zoomheight = m_ZoomRect.height - m_ZoomRect.y;
				m_ZoomRect.y = y - m_ZoomDiff.y;
				m_ZoomRect.y = MID(miny, m_ZoomRect.y, (s.y - zoomheight));
				m_ZoomRect.height += (m_ZoomRect.y - oldzy);
			}
		
			
			
			if(m_ZoomRect.x != oldzx || m_ZoomRect.y != oldzy)
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
		if (m_ZoomRect.width - m_ZoomRect.x < 100 || m_ZoomRect.height - m_ZoomRect.y < 56){
			m_ZoomRect = tmp;
		}
		Zoom(s1);
	}

}

void RendererVideo::DrawProgressBar(const wxString &timesString)
{
	//Full screen progress bar position
	wxMutexLocker lock(m_MutexProgressBar);
	m_ProgressBarTime = timesString;
	int fw, fh;
	RECT rcRect = { 0, 0, 0, 0 };
	if (m_D3DCalcFont->DrawTextW(nullptr, m_ProgressBarTime.wchar_str(), -1, &rcRect, DT_CALCRECT, 0xFF000000)) {
		fw = rcRect.right - rcRect.left;
		fh = rcRect.bottom - rcRect.top;
	}
	else {
		videoControl->GetTextExtent(m_ProgressBarTime, &fw, &fh, nullptr, nullptr, Options.GetFont(4));
	}
	int progresbarHeight = fh * 0.6f;
	int margin = fh * 0.25f;
	int w, h;
	videoControl->m_FullScreenWindow->GetClientSize(&w, &h);
	m_ProgressBarRect.top = progresbarHeight + (margin * 1.5f);
	m_ProgressBarRect.bottom = fh + m_ProgressBarRect.top;
	m_ProgressBarRect.left = w - (fw + margin);
	m_ProgressBarRect.right = w - (margin);
	//coordinates of black frame
	vectors[0].x = w - (fw + margin);
	vectors[0].y = margin;
	vectors[1].x = w - margin;
	vectors[1].y = margin;
	vectors[2].x = w - margin;
	vectors[2].y = progresbarHeight + margin;
	vectors[3].x = w - (fw + margin);
	vectors[3].y = progresbarHeight + margin;
	vectors[4].x = w - (fw + margin);
	vectors[4].y = margin;
	//coordinates of white frame inside black frame
	vectors[5].x = w - (fw - 1 + margin);
	vectors[5].y = margin + 1;
	vectors[6].x = w - (margin + 1);
	vectors[6].y = margin + 1;
	vectors[7].x = w - (margin + 1);
	vectors[7].y = progresbarHeight - 1 + margin;
	vectors[8].x = w - (fw - 1 + margin);
	vectors[8].y = progresbarHeight - 1 + margin;
	vectors[9].x = w - (fw - 1 + margin);
	vectors[9].y = margin + 1;
	//coordinates of progress bar inside white frame
	int rw = w - (fw - 2 + margin);
	//two pixels from both sides
	int progBarLen = fw - 4;
	m_ProgressBarLineWidth = progresbarHeight - 2;
	vectors[10].x = rw;
	vectors[10].y = (margin + 1) + (m_ProgressBarLineWidth / 2);
	int Duration = GetDuration();
	vectors[11].x = (Duration > 0) ? (((float)m_Time / (float)Duration) * progBarLen) + rw : progBarLen + rw;
	vectors[11].y = (margin + 1) + (m_ProgressBarLineWidth / 2);
	
}

void RendererVideo::SetVisual(bool settext/*=false*/, bool noRefresh /*= false*/)
{
	wxMutexLocker lock(m_MutexVisualChange);

	m_HasVisualEdition = false;
	int vis = tab->edit->Visual;
	if (!m_Visual) {
		m_Visual = Visuals::Get(vis, videoControl);
	}
	else if (m_Visual->Visual != vis) {
		if (m_Visual->Visual == VECTORCLIP)
			settext = true;
		delete m_Visual;
		m_Visual = Visuals::Get(vis, videoControl);
	}
	else { SAFE_DELETE(m_Visual->dummytext); }
	m_Visual->SizeChanged(wxRect(m_BackBufferRect.left, m_BackBufferRect.top,
		m_BackBufferRect.right, m_BackBufferRect.bottom), m_D3DLine, m_D3DFont, m_D3DDevice);
	SetVisualZoom();
	int tool = tab->video->GetVideoToolbar()->GetItemToggled();
	m_Visual->SetVisual(tab->edit->line, tool, noRefresh);
	bool vectorclip = m_Visual->Visual == VECTORCLIP;
	if (vectorclip || settext) { OpenSubs(OPEN_DUMMY); }
	Render(!noRefresh);
	m_HasVisualEdition = vis > 0;
}

void RendererVideo::ResetVisual()
{
	wxMutexLocker lock(m_MutexVisualChange);
	m_HasVisualEdition = false;
	int tool = tab->video->GetVideoToolbar()->GetItemToggled();
	m_Visual->SetVisual(tab->edit->line, tool);
	m_HasVisualEdition = true;
	Render();
}

bool RendererVideo::RemoveVisual(bool noRefresh, bool disable)
{
	wxMutexLocker lock(m_MutexVisualChange);
	m_HasVisualEdition = false;
	if (disable) {
		tab->edit->Visual = -1;
		SAFE_DELETE(m_Visual);
	}
	else {
		tab->edit->Visual = 0;
		SetVisual();
	}
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

bool RendererVideo::HasVisual(bool hasDefault)
{
	if (hasDefault)
		return m_Visual && m_Visual->Visual != CROSS;
	else
		return m_Visual != nullptr;
}

Visuals *RendererVideo::GetVisual()
{
	return m_Visual;
}

void RendererVideo::SetAudioPlayer(AudioDisplay *player)
{
	m_AudioPlayer = player;
}

void RendererVideo::SaveFrame(int id)
{
	bool del = false;
	byte* framebuf = GetFrameWithSubs(id > VIDEO_COPY_FRAME_TO_CLIPBOARD, &del);
	if (!framebuf)
		return;

	size_t rgb32size = m_Height * m_Width * 4;
	size_t rgb24size = m_Height * m_Width * 3;
	byte* rgb24 = (byte*)malloc(rgb24size);
	memset(rgb24, 0, rgb24size);
	byte* buff = rgb24;
	for (size_t i = 0; i < rgb32size; i += 4) {
		*buff++ = framebuf[i + 2];
		*buff++ = framebuf[i + 1];
		*buff++ = framebuf[i + 0];
	}
	wxImage frame(m_Width, m_Height, false);
	frame.SetData(rgb24);

	if (id == VIDEO_SAVE_FRAME_TO_PNG || id == VIDEO_SAVE_SUBBED_FRAME_TO_PNG) {
		wxString path;
		int num = 1;
		wxArrayString paths;
		wxString filespec;
		wxString dirpath = tab->VideoPath.BeforeLast(L'\\', &filespec);
		wxDir kat(dirpath);
		path = tab->VideoPath;
		if (kat.IsOpened()) {
			kat.GetAllFiles(dirpath, &paths, filespec.BeforeLast(L'.') << L"_*_*.png", wxDIR_FILES);
		}
		for (wxString& file : paths) {
			if (file.find(L"_" + std::to_string(num) + L"_") == wxNOT_FOUND) {
				break;
			}
			num++;
		}
		path = tab->VideoPath.BeforeLast(L'.');
		SubsTime currentTime;
		currentTime.mstime = m_Time;
		wxString timestring = currentTime.raw(SRT);
		timestring.Replace(L":", L";");
		//path.Replace(L",", L".");
		path << L"_" << num << L"_" << timestring << L".png";
		frame.SaveFile(path, wxBITMAP_TYPE_PNG);
	}
	else {
		if (wxTheClipboard->Open())
		{
			wxTheClipboard->SetData(new wxBitmapDataObject(frame));
			wxTheClipboard->Close();
		}
	}
	if (del) { delete[] framebuf; }
}

PlaybackState RendererVideo::GetState()
{
	return m_State;
}

