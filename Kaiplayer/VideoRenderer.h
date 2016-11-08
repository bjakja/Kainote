#ifndef VIDEORENDERER
#define VIDEORENDERER

#pragma once
#include <wx/wx.h>
#include <d3d9.h>
#include <d3dx9.h>


#undef DrawText

#include "VideoFfmpeg.h"
#include "Menu.h"
#include "Visuals.h"

//#define byvertices 5
#define DXVA 77

#if DXVA
#include <dxva2api.h>
#endif


#ifndef SAFE_DELETE
#define SAFE_DELETE(x) if (x !=NULL) { delete x; x = NULL; }
#endif

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(x) if (x != NULL) { x->Release(); x = NULL; } 
#endif



#ifndef PTR
#define PTR(what,err) if(!what) {wxLogStatus(err); return false;}
#endif

#ifndef PTR1
#define PTR1(what,err) if(!what) {wxLogStatus(err); return;}
#endif

#ifndef HR
#define HR(what,err) if(FAILED(what)) {wxLogStatus(err); return false;}
#endif

#ifndef HRN
#define HRN(what,err) if(FAILED(what)) {wxLogStatus(err); return;}
#endif



typedef void csri_inst;
typedef void csri_rend;

enum PlaybackState
{
	Playing,
	Stopped,
	Paused,
	None
};
struct chapter
{
	wxString name;
	int time;
};

struct VERTEX
{	
	float fX;	
	float fY;	
	float fZ;
	D3DCOLOR Color;	
};

void CreateVERTEX (VERTEX *v, float X, float Y, D3DCOLOR Color, float Z=0.0f);


class AudioDisplay;
class DShowPlayer;
struct csri_fmt;
struct csri_frame;

class VideoRend : public wxWindow
{
	
	public:
		VideoRend(wxWindow *parent, const wxSize &size=wxDefaultSize);
		virtual ~VideoRend();

		bool OpenFile(const wxString &fname, wxString *textsubs, bool Dshow=true, bool vobsub=false, bool fullscreen=false);
		bool OpenSubs(wxString *textsubs, bool redraw=true);
		bool Play(int end=-1);
		bool PlayLine(int start, int end);
		bool Pause();
		bool Stop();
        void SetPosition(int time, bool starttime=true, bool corect=true, bool reloadSubs=true);
		void GoToNextKeyframe();
        void GoToPrevKeyframe();
		int GetCurrentPosition();
		int GetDuration();
		int GetVolume();
		void GetVideoSize(int *width, int *height);
		wxSize GetVideoSize();
		void GetFpsnRatio(float *fps, long *arx, long *ary);
		void UpdateVideoWindow(bool bars=true);
		void SetVolume(int vol);
		
		void Render(bool RecreateFrame=true);
		void DrawLines(wxPoint point);
		void DrawProgBar();
		bool DrawTexture(byte *nframe=NULL, bool copy=false);
		void EnableStream(long index);
		void MovePos(int cpos);
		void ChangeVobsub(bool vobsub=false);
		wxArrayString GetStreams();
		void SetVisual(int start, int end, bool remove=false, bool settext=false);
		void SetVisual();
		byte *GetFramewithSubs(bool subs, bool *del);
		bool UpdateRects(bool bar);
		LPDIRECT3DSURFACE9 MainStream;
		LPDIRECT3DDEVICE9 d3device;
		D3DFORMAT d3dformat;
		volatile bool block;
		bool IsDshow;
		bool seek;
		bool VisEdit;
		bool cross;
		bool pbar;
		bool resized;
		int vwidth;
		int vheight;
		int pitch;
		int time;
		int lastframe;
		/*const */int panelHeight;
		long ax,ay;
		float AR, fps;
		char *datas;
		byte vformat;
		float avtpf;
		wxString coords;
		wxString pbtime;
		ID3DXLine *lines;
		LPD3DXFONT m_font;
		VideoFfmpeg *VFF;
		AudioDisplay *player;
		//wxMutex mutexSizing;
		wxMutex mutexRender;
		//wxMutex mutexDrawing;
		wxMutex mutexLines;
		wxMutex mutexProgBar;
		wxMutex mutexOpenFile;
		//wxMutex mutexOpenSubs;
		PlaybackState vstate;
		Visuals *Vclips;
		int playend;
		size_t lasttime;
		std::vector<chapter> chaps;
		bool EnumFilters(Menu *menu);
		bool FilterConfig(wxString name, int idx, wxPoint pos);
	private:
		bool InitDX(bool reset=false);
		
		void Clear();
		
		
		LPDIRECT3D9 d3dobject;
		LPDIRECT3DSURFACE9 bars;


#if byvertices
		LPDIRECT3DVERTEXBUFFER9 vertex;
		LPDIRECT3DTEXTURE9 texture;
#endif
#if DXVA
		IDirectXVideoProcessorService *dxvaService;
		IDirectXVideoProcessor *dxvaProcessor;
		DXVA2_VideoDesc videoDesc;
		GUID dxvaGuid;
#endif
		HWND hwnd;
		bool devicelost;
		bool playblock;
		
		int diff;
		
		csri_inst *instance;
		csri_rend *vobsub;
		RECT rt1;
		RECT rt2;
		RECT rt3;
		RECT rt4;
		RECT rt5;
		
		
		int avframetime;
		
		D3DXVECTOR2 vectors[16];
		DShowPlayer *vplayer;
		
		csri_frame *framee;
		csri_fmt *format;
		
};

#ifndef DRAWOUTTEXT
#define DRAWOUTTEXT(font,text,rect,align,color)\
	RECT tmpr=rect;\
	tmpr.top--;tmpr.bottom--;\
	tmpr.left--;tmpr.right--;\
	for(int i=0; i<9; i++)\
	{\
		if(i%3==0 && i>0){tmpr.left=rect.left-1; tmpr.right=rect.right-1; tmpr.top++;tmpr.bottom++;}\
		if(i!=4){font->DrawTextW(NULL, text.wchar_str(), -1, &tmpr, align, 0xFF000000 );}\
		tmpr.left++;tmpr.right++;\
	}\
	font->DrawTextW(NULL, text.wchar_str(), -1, &rect, align, color );
#endif

#endif