#ifndef DSPL
#define DSPL


#include "VideoRenderer.h"

#include <wx/wx.h>
#include <dshow.h>
#include <qnetwork.h>

#include "DshowRenderer.h"


class DShowPlayer
	{
public:

	DShowPlayer(wxWindow*_parent);
	~DShowPlayer();
	bool OpenFile(wxString fname, bool vobsub=false);
	void Play();
	void Pause();
	void Stop();
	void SetPosition(int pos);

	int GetPosition();
	int GetDuration();

	void SetVolume(long volume);
	long GetVolume();

	void GetFpsnRatio(float *fps, long *arx, long *ary);
	bool EnumFilters(wxMenu *menu);
	bool FilterConfig(wxString name, int idx, wxPoint pos);
	std::vector<chapter> GetChapters();
	wxSize GetVideoSize();
	void TearDownGraph();
	PlaybackState m_state;
	VideoInf inf;
	IMediaControl	*m_pControl;
	wxArrayString GetStreams();
	IAMStreamSelect *stream;
	IAMExtendedSeeking *chapters;
private:
	bool InitializeGraph();
	HWND hwndVid;			
	
    IGraphBuilder	*m_pGraph;
	IMediaSeeking	*m_pSeek;
	IBasicAudio		*m_pBA;
	//IBaseFilter		*frend;
	
	

	wxWindow *parent;
	
};

#endif