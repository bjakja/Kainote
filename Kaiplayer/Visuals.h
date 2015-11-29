
#define UNICODE

#include <wx/wx.h>
#include <vector>
#include <d3d9.h>
#include <d3dx9.h>
// wspw i h - potrzebne s¹ do przejœcia z rozdzielczoœci napisów i do niej
// _x i _y to punkt przemieszczenia w przypadku rysunków.


enum{
	CHANGEPOS=1,
	MOVE,
	//MOVEONCURVE,
	SCALE,
	ROTATEZ,
	ROTATEXY,
	CLIPRECT,
	//FAXY,
	VECTORCLIP,
	VECTORDRAW
};

struct MYVERTEX
{	
	float fX;	
	float fY;	
	float fZ;
	D3DCOLOR Color;	
};


class ClipPoint
{
public:
	ClipPoint(int x, int y, wxString type, bool isstart);
	ClipPoint();
	bool IsInPos(wxPoint pos, int diff);
	D3DXVECTOR2 GetVector(bool wsp=true);
	int wx();
	int wy();
	int x;
	int y;
	wxString type;
	bool start;
};

class Visuals : public wxEvtHandler
{
public:
	Visuals(wxWindow *_parent);
	virtual ~Visuals();
	//clips
	void DrawClip();
	void SetClip(wxString clip, float x, float y);
	void SetNewSize(wxSize wsize);
	wxString GetClip();
	void SetPos(int x, int y);
	int CheckPos(wxPoint pos, bool retlast=false, bool wsp=true);
	void MovePoint(wxPoint pos, int point);
	void AddCurve(wxPoint pos, int whereis, wxString type="b");
	void AddCurvePoint(wxPoint pos, int whereis);
	void AddLine(wxPoint pos, int whereis);
	void AddMove(wxPoint pos, int whereis);
	void DrawLine(int i);
	int DrawCurve(int i,bool bspline=false);
	void DrawRect(int i, bool wsp=true);
	void DrawCircle(int i, bool wsp=true);
	void OnMouseEvent(wxMouseEvent &event);
	void Curve(int pos, std::vector<D3DXVECTOR2> *table, bool bspline, int spoints=4, int acpt=0);
	D3DXVECTOR2 CalcWH();
	void CreateMYVERTEX (MYVERTEX *v, float X, float Y, D3DCOLOR Color, float Z=0.0f)
	{	
		v->fX = X;	
		v->fY = Y;	
		v->fZ = Z;		
		v->Color = Color;	
	}
	//visuals
	void SetVisual(int visual, wxString vis,int _start,int _end,wxSize wsize, wxSize ssize, D3DXVECTOR2 linepos, D3DXVECTOR2 scale, byte AN, LPD3DXLINE line, LPD3DXFONT _font, LPDIRECT3DDEVICE9 device);
	void Position();
	void Move(int time);
	void MoveOnCurve(int time);
	void Scale();
	void RotateZ();
	void RotateXY();
	void ClipRect();
	void FaXY();
	void Draw(int time);
	D3DXVECTOR2 IsOnPoint(wxPoint pos);
	wxString GetVisual(bool org=false);

	void MouseEvent(wxMouseEvent &evt);
	D3DXVECTOR2 angle;
	D3DXVECTOR2 to;
	D3DXVECTOR2 lastmove;
	D3DXVECTOR2 firstmove;
	D3DXVECTOR2 from;
	D3DXVECTOR2 org;
	byte AN;
	unsigned char type;
	static float wspw, wsph, _x, _y;

private:
	LPD3DXLINE line;
	LPD3DXFONT font;
	LPDIRECT3DDEVICE9 device;
	std::vector<ClipPoint> Points;
	
	wxMutex clipmutex;

	int start;
	int end;
	int moveStart;
	int moveEnd;
	int x, y;
	int curvelines;
	wxString times;
	
	unsigned char Visual;
	
	wxSize subssize;
	wxSize widsize;
	D3DXVECTOR2 scale;
	int grabbed;
	bool drawtxt;
	bool newline;
	bool newmove;
	DWORD align;
	wxPoint diffs;
	ClipPoint acpoint;
	wxWindow *parent;
	wxString coords;
	RECT cpos;
	bool hasArrow;
	bool invClip;
	DECLARE_EVENT_TABLE()
};


