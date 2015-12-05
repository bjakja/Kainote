#ifndef VIDEOVISUALS
#define VIDEOVISUALS
//#define UNICODE
#pragma once
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
	//FAXY,
	CLIPRECT,
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
	D3DXVECTOR2 GetVector();
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
	void SetClip(wxString clip);
	void SizeChanged(wxSize wsize, LPD3DXLINE _line, LPD3DXFONT _font, LPDIRECT3DDEVICE9 _device);
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
	void DrawRect(int i, D3DXVECTOR2 *vector=NULL);
	void DrawCircle(int i, D3DXVECTOR2 *vector=NULL);
	void DrawArrow(D3DXVECTOR2 vector, D3DXVECTOR2 *vector1, int diff=0);
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
	void SetVisual(int _start,int _end);
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
	D3DXVECTOR2 CalcMovePos();

	void MouseEvent(wxMouseEvent &evt);
	D3DXVECTOR2 angle;
	D3DXVECTOR2 to;
	D3DXVECTOR2 lastmove;
	D3DXVECTOR2 firstmove;
	D3DXVECTOR2 from;
	D3DXVECTOR2 org;
	byte AN;
	wxString times;
	D3DXVECTOR2 scale;
	double tbl[7];
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
	int oldtime;
	
	
	unsigned char Visual;
	
	wxSize subssize;
	wxSize widsize;
	
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

#endif
