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


#pragma once

#include <wx/wx.h>
#include <vector>
#include <d3d9.h>
#include <d3dx9.h>
#include <map>


enum{
	CHANGEPOS = 1,//w przypadku zmiany, zmieniæ w subsgridzie w selectrow;
	MOVE,
	SCALE,
	ROTATEZ,
	ROTATEXY,
	CLIPRECT,
	VECTORCLIP,
	VECTORDRAW,
	MOVEALL,
	SCALE_ROTATION
};

class Dialogue;
class TabPanel;
class DrawingAndClip;

class ClipPoint
{
public:
	ClipPoint(float x, float y, wxString type, bool isstart);
	ClipPoint();
	bool IsInPos(D3DXVECTOR2 pos, float diff);
	D3DXVECTOR2 GetVector(DrawingAndClip *parent);
	float wx(DrawingAndClip *parent, bool zoomConversion = false);
	float wy(DrawingAndClip *parent, bool zoomConversion = false);
	float x;
	float y;
	wxString type;
	bool start;
	bool isSelected;
};

class Visuals
{
public:
	Visuals();
	virtual ~Visuals();
	static Visuals *Get(int Visual, wxWindow *_parent);
	void SizeChanged(wxRect wsize, LPD3DXLINE _line, LPD3DXFONT _font, LPDIRECT3DDEVICE9 _device);
	void DrawRect(D3DXVECTOR2 vector, bool sel = false, float size = 5.0f);
	void DrawCircle(D3DXVECTOR2 vector, bool sel = false, float size = 6.0f);
	void DrawCross(D3DXVECTOR2 position, D3DCOLOR color = 0xFFFF0000, bool useBegin = true);
	void DrawArrow(D3DXVECTOR2 vector, D3DXVECTOR2 *vector1, int diff = 0);
	void DrawDashedLine(D3DXVECTOR2 *vector, size_t vectorSize, int dashLen = 4, unsigned int color = 0xFFBB0000);
	void SetZoom(D3DXVECTOR2 move, D3DXVECTOR2 scale){
		zoomMove = move;
		zoomScale = scale;
	};
	void GetDialoguesWithoutPosition();
	int GetDialoguePosition();
	void RenderSubs(wxString *subs, bool redraw = true);

	virtual void SetVisual(int _start, int _end, bool notDial, bool noRefresh = false);
	virtual void Draw(int time);
	virtual void DrawVisual(int time){};
	virtual void SetCurVisual(){};
	virtual void ChangeTool(int _tool){};
	virtual void OnMouseEvent(wxMouseEvent &evt){};
	//function should skip events when not use it;
	virtual void OnKeyPress(wxKeyEvent &evt){};
	virtual wxString GetVisual(){ return ""; };
	virtual void ChangeVisual(wxString *txt, Dialogue *_dial){};
	void DrawWarning(bool comment);
	void SetClip(wxString clip, bool dummy, bool redraw = true, bool changeEditorText = true);
	void SetVisual(bool dummy, int type);
	void ChangeOrg(wxString *text, Dialogue *_dial, float coordx, float coordy);
	bool IsInPos(wxPoint pos, wxPoint secondPos, int diff){
		return (abs(pos.x - secondPos.x) < diff && abs(pos.y - secondPos.y) < diff) ? true : false;
	};
	D3DXVECTOR2 GetPosnScale(D3DXVECTOR2 *scale, byte *AN, double *tbl);
	D3DXVECTOR2 CalcMovePos();
	D3DXVECTOR2 GetPosition(Dialogue *Dial, bool *putinBracket, wxPoint *TextPos);
	D3DXVECTOR2 to;
	D3DXVECTOR2 lastmove;
	D3DXVECTOR2 firstmove;
	D3DXVECTOR2 from;

	double moveValues[7];
	// coeffw and h - needed to convert from video to subs or subs to video resolution
	float coeffW, coeffH;

	LPD3DXLINE line;
	LPD3DXFONT font;
	LPDIRECT3DDEVICE9 device;
	wxMutex clipmutex;

	int start;
	int end;
	int oldtime;

	unsigned char Visual;
	unsigned char axis;

	wxSize SubsSize;
	wxRect VideoSize;
	TabPanel *tab;
	bool hasArrow;
	bool blockevents;
	bool notDialogue;
	wxString *dummytext;
	wxPoint dumplaced;
	wxPoint textplaced;
	D3DXVECTOR2 zoomMove;
	D3DXVECTOR2 zoomScale;
	wxArrayInt selPositions;
	//Dialogue adresses are valid only for one modification
	//need recreate on every checking
	std::vector<Dialogue*> dialoguesWithoutPosition;
};

class PosData{
public:
	PosData(Dialogue *_dial, int _numpos, D3DXVECTOR2 _pos, wxPoint _TextPos, bool _putinBracket){
		dial = _dial; numpos = _numpos; pos = _pos; lastpos = pos; TextPos = _TextPos; putinBracket = _putinBracket;
	}
	D3DXVECTOR2 pos;
	D3DXVECTOR2 lastpos;
	wxPoint TextPos;
	bool putinBracket;
	int numpos;
	Dialogue *dial;
};

class Position : public Visuals
{
public:
	Position();
	//~Position();
	void OnMouseEvent(wxMouseEvent &event);
	wxString GetVisual(){ return ""; };
	void ChangeVisual(wxString *txt, Dialogue *_dial){};
	wxString GetVisual(int datapos);
	void ChangeMultiline(bool all);
	void SetCurVisual();
	void Draw(int time);
	void ChangeTool(int _tool){};
	void OnKeyPress(wxKeyEvent &evt);
	//void SetVisual(int _start,int _end);
	std::vector<PosData> data;
	wxPoint helperLinePos;
	bool hasHelperLine = false;
	bool movingHelperLine = false;
};

class Move : public Visuals
{
public:
	Move();
	void DrawVisual(int time);
	void OnMouseEvent(wxMouseEvent &event);
	wxString GetVisual();
	void ChangeVisual(wxString *txt, Dialogue *_dial);
	void SetCurVisual();
	void ChangeTool(int _tool){};
	void OnKeyPress(wxKeyEvent &evt);
	int moveStart;
	int moveEnd;
	byte type;
	int grabbed;
	wxPoint diffs;
	D3DXVECTOR2 lastFrom;
	D3DXVECTOR2 lastTo;
	D3DXVECTOR2 moveDistance;
	wxPoint helperLinePos;
	bool hasHelperLine = false;
	bool movingHelperLine = false;
};

struct moveElems
{
	D3DXVECTOR2 elem;
	byte type;
};

class MoveAll : public Visuals
{
public:
	MoveAll();
	void DrawVisual(int time);
	void OnMouseEvent(wxMouseEvent &event);
	wxString GetVisual();
	void ChangeVisual(wxString *txt, Dialogue *_dial){};
	void SetCurVisual();
	void ChangeInLines(bool all);
	void ChangeTool(int _tool);
	void OnKeyPress(wxKeyEvent &evt);
	std::vector<moveElems> elems;
	int numElem;
	int elemsToMove;
	byte selectedTags;
	wxPoint diffs;
	wxPoint dumplaced;
	D3DXVECTOR2 beforeMove;
};

class Scale : public Visuals
{
public:
	Scale();
	void DrawVisual(int time);
	void OnMouseEvent(wxMouseEvent &event);
	wxString GetVisual();
	void ChangeVisual(wxString *txt, Dialogue *_dial);
	void SetCurVisual();
	void ChangeTool(int _tool){};
	void OnKeyPress(wxKeyEvent &evt);
	byte type;
	int grabbed;
	byte AN;
	D3DXVECTOR2 scale;
	wxPoint diffs;
	D3DXVECTOR2 arrowLengths = D3DXVECTOR2(100.f, 100.f);
};


class RotationZ : public Visuals
{
public:
	RotationZ();
	void DrawVisual(int time);
	void OnMouseEvent(wxMouseEvent &event);
	wxString GetVisual();
	void ChangeVisual(wxString *txt, Dialogue *_dial);
	void SetCurVisual();
	void ChangeTool(int _tool){};
	void OnKeyPress(wxKeyEvent &evt);
	bool isOrg;
	D3DXVECTOR2 org;
	D3DXVECTOR2 lastOrg;
	wxPoint diffs;
};

class RotationXY : public Visuals
{
public:
	RotationXY();
	void DrawVisual(int time);
	void OnMouseEvent(wxMouseEvent &event);
	wxString GetVisual();
	void ChangeVisual(wxString *txt, Dialogue *_dial);
	void SetCurVisual();
	void ChangeTool(int _tool){};
	void OnKeyPress(wxKeyEvent &evt);
	bool isOrg;
	D3DXVECTOR2 angle;
	D3DXVECTOR2 oldAngle;
	D3DXVECTOR2 org;
	D3DXVECTOR2 lastOrg;
	byte type;
	byte AN;
	wxPoint diffs;
};

class ClipRect : public Visuals
{
public:
	ClipRect();
	void DrawVisual(int time);
	void OnMouseEvent(wxMouseEvent &event);
	wxString GetVisual();
	void ChangeVisual(wxString *txt, Dialogue *_dial);
	void SetCurVisual();
	void ChangeTool(int _tool){};
	int HitTest(D3DXVECTOR2 pos, bool diff = true);
	void OnKeyPress(wxKeyEvent &evt);
	D3DXVECTOR2 Corner[2];
	bool invClip;
	bool showClip;
	int grabbed;
	D3DXVECTOR2 diffs;
};

class DrawingAndClip : public Visuals
{
public:
	DrawingAndClip();
	~DrawingAndClip();
	void DrawVisual(int time);
	void OnMouseEvent(wxMouseEvent &event);
	wxString GetVisual();
	void ChangeVisual(wxString *txt, int line){};
	void SetCurVisual();
	void SetPos(int x, int y);
	int CheckPos(D3DXVECTOR2 pos, bool retlast = false, bool wsp = true);
	void MovePoint(D3DXVECTOR2 pos, int point);
	void AddCurve(D3DXVECTOR2 pos, int whereis, wxString type = "b");
	void AddCurvePoint(D3DXVECTOR2 pos, int whereis);
	void AddLine(D3DXVECTOR2 pos, int whereis);
	void AddMove(D3DXVECTOR2 pos, int whereis);
	void DrawLine(int coord);
	void DrawRect(int coord);
	void DrawCircle(int coord);
	int DrawCurve(int i, bool bspline = false);
	void Curve(int pos, std::vector<D3DXVECTOR2> *table, bool bspline, int spoints = 4, int acpt = 0);
	D3DXVECTOR2 CalcWH();
	void SelectPoints();
	void ChangeSelection(bool select = false);
	void ChangeTool(int _tool){
		tool = _tool;
	};
	int FindPoint(int pos, wxString type, bool nextStart = false, bool fromEnd = false);
	ClipPoint FindSnapPoint(const ClipPoint &pos, size_t pointToSkip/*, bool coeff = false*/);
	void OnKeyPress(wxKeyEvent &evt);
	std::vector<ClipPoint> Points;
	ClipPoint acpoint;
	ClipPoint lastpoint;
	bool invClip;
	bool drawtxt;
	bool snapXminus;
	bool snapYminus;
	bool snapXplus;
	bool snapYplus;
	bool drawSelection;
	bool drawToolLines;
	bool drawCross;
	int grabbed;
	int tool;
	int x;
	int y;
	int lastpos;
	float pointArea;
	int vectorScale;
	byte alignment;
	wxPoint diffs;
	wxRect selection;
	D3DXVECTOR2 scale;
	// _x i _y to punkt przemieszczenia w przypadku rysunków.
	float _x, _y;
	D3DXVECTOR2 offsetxy;
	wxString textwithclip;
};

class ScaleRotation : public Visuals
{
public:
	ScaleRotation();
	~ScaleRotation(){};
	void DrawVisual(int time);
	void OnMouseEvent(wxMouseEvent &event);
	void SetCurVisual();
	void ChangeTool(int _tool);
	//wxString GetVisual();

private:
	void DrawScale(int time);
	void DrawRotationZ(int time);
	void DrawRotationXY(int time);
	void ChangeInLines(bool dummy = true);
	void OnHoldingRotation(int x, int y);
	void OnHoldingScaling(int x, int y, bool hasShift);
	void OnClickRotationZ(int x, int y);
	void OnClickRotationXY(int x, int y, bool leftClick, bool rightClick, bool middleClick);
	void OnClickScaling(int x, int y, bool leftClick, bool rightClick, bool middleClick, bool shiftDown);
	bool SeekTags(const wxString &text, const wxString &pattern, wxString *result);
	void OnKeyPress(wxKeyEvent &evt);
	bool isOrg = false;
	bool hasOrg = false;
	bool onlyFirst = false;
	//with rotation xy angle.x = fry and angle.y = frx
	D3DXVECTOR2 angle;
	D3DXVECTOR2 oldAngle;
	D3DXVECTOR2 org;
	D3DXVECTOR2 lastOrg;
	D3DXVECTOR2 scale;
	//tagvalues needed to calculate value change
	//add defferent from this two values
	D3DXVECTOR2 beforeMove;//tagvalue
	D3DXVECTOR2 afterMove;//tagvalue
	byte type = 255;
	byte AN;
	wxPoint diffs;
	byte selectedTool = 0;
	bool tagXFound = false;
	bool tagYFound = false;
};

int ChangeText(wxString *txt, const wxString &what, bool inbracket, const wxPoint &pos);


