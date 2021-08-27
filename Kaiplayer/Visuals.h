//  Copyright (c) 2016 - 2020, Marcin Drob

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
#include <d3d9.h>
#include <d3dx9.h>
#undef DrawText
#include <wx/string.h>
#include <wx/bitmap.h>
#include <wx/event.h>
#include <wx/thread.h>
#include <vector>
#include <map>
#include "VisualAllTagsEdition.h"
#include "TagFindReplace.h"
#include "VisualAllTagsControls.h"


enum{
	CROSS = 0,
	CHANGEPOS,//in case of changing change in SubsGrid selectrow
	MOVE,
	SCALE,
	ROTATEZ,
	ROTATEXY,
	CLIPRECT,
	VECTORCLIP,
	VECTORDRAW,
	MOVEALL,
	ALL_TAGS
};

class Dialogue;
class TabPanel;
class DrawingAndClip;
class TextEditor;

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

class Visuals : public TagFindReplace
{
public:
	Visuals();
	virtual ~Visuals();
	static Visuals *Get(int Visual, wxWindow *_parent);
	virtual void SizeChanged(wxRect wsize, LPD3DXLINE _line, LPD3DXFONT _font, LPDIRECT3DDEVICE9 _device);
	void DrawRect(D3DXVECTOR2 vector, bool sel = false, float size = 5.0f);
	void DrawCircle(D3DXVECTOR2 vector, bool sel = false, float size = 6.0f);
	void DrawCross(D3DXVECTOR2 position, D3DCOLOR color = 0xFFFF0000, bool useBegin = true);
	void DrawArrow(D3DXVECTOR2 vector, D3DXVECTOR2 *vector1, int diff = 0);
	void DrawDashedLine(D3DXVECTOR2 *vector, size_t vectorSize, int dashLen = 4, unsigned int color = 0xFFBB0000);
	//can overwrite but need to use Visuals::SetZoom
	virtual void SetZoom(D3DXVECTOR2 move, D3DXVECTOR2 scale){
		zoomMove = move;
		zoomScale = scale;
	};
	void GetDialoguesWithoutPosition(Dialogue* dialogue);
	//get dialogue additional position when there is no pos or move tag
	D3DXVECTOR2 GetDialogueAdditionalPosition(Dialogue *dialogue);
	void GetRectFromSize(D3DXVECTOR2 size, int an, Dialogue* dial, Styles* style, D3DXVECTOR2* pos, D3DXVECTOR2* pos1);
	bool IsInRect(D3DXVECTOR2 pos, D3DXVECTOR2 pos1, D3DXVECTOR2 secondpos, D3DXVECTOR2 secondpos1);
	void SetPositionByAn(D3DXVECTOR2* pos, int an, Dialogue *dial, Styles *style);
	void RenderSubs(wxString *subs, bool redraw = true);

	virtual void SetVisual(Dialogue* dial, int tool, bool noRefresh = false);
	virtual void Draw(int time);
	virtual void DrawVisual(int time){};
	virtual void SetCurVisual(){};
	virtual void ChangeTool(int _tool, bool blockSetCurVisual = false){};
	virtual void OnMouseEvent(wxMouseEvent &evt){};
	//function should skip events when not use it;
	virtual void OnKeyPress(wxKeyEvent &evt){};
	virtual void OnMouseCaptureLost(wxMouseCaptureLostEvent &evt){}
	//virtual void GetVisual(wxString *visual){};
	virtual void ChangeVisual(wxString *txt, Dialogue *_dial, size_t numOfSelections){};
	virtual wxPoint ChangeVisual(wxString* txt) { return wxPoint(0, 0); };
	virtual void AppendClipMask(wxString *mask) {};
	void DrawWarning(bool comment);
	//virtual void SetClip(bool dummy, bool redraw = true, bool changeEditorText = true) {};
	void SetVisual(bool dummy);
	void ChangeOrg(wxString *text, Dialogue *_dial, float coordx, float coordy);
	bool IsInPos(wxPoint pos, wxPoint secondPos, int diff){
		return (abs(pos.x - secondPos.x) < diff && abs(pos.y - secondPos.y) < diff) ? true : false;
	};
	void GetVectorPoints(const wxString &vector, std::vector<ClipPoint> *points);
	void RotateZ(D3DXVECTOR2* point, float sinOfAngle, float cosOfAngle, D3DXVECTOR2 orgpivot);
	void RotateDrawing(ClipPoint* point, float sinOfAngle, float cosOfAngle, D3DXVECTOR2 orgpivot);
	void GetMoveTimes(int *start, int *end);
	void SetModified(int action, bool dummy = false);
	bool GetTextExtents(const wxString &text, Styles *style, float* width, float* height, float* descent = NULL, float* extlead = NULL);
	void Curve(int pos, std::vector<ClipPoint>* vectorPoints, std::vector<D3DXVECTOR2>* table, bool bspline = false, int nBsplinePoints = 4, int currentPoint = 0);
	//bordshad must be 2 elements table
	D3DXVECTOR2 GetTextSize(Dialogue* dial, D3DXVECTOR2 *bord, Styles* style = NULL, 
		bool keepExtraLead = false, D3DXVECTOR2* extralead = NULL, 
		D3DXVECTOR2* drawingPosition = NULL, D3DXVECTOR2* bordshad = NULL);
	D3DXVECTOR2 CalcDrawingSize(int alignment, std::vector<ClipPoint>* points, bool withoutAlignment = false);
	D3DXVECTOR2 GetDrawingSize(const wxString& drawing, D3DXVECTOR2 *position = NULL);
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
	bool blockevents;
	bool notDialogue;
	bool replaceTagsInCursorPosition = true;
	wxString *dummytext;
	wxPoint dumplaced;
	wxPoint textplaced;
	D3DXVECTOR2 zoomMove;
	D3DXVECTOR2 zoomScale;
	wxArrayInt selPositions;
	wxString currentLineText;
private:
	TextEditor* editor = NULL;
	//int activeLineInTable = -1;
	//Dialogue adresses are valid only for one modification
	//need recreate on every checking
	std::vector<Dialogue*> dialoguesWithoutPosition;
};

class PosData{
public:
	PosData(int _numpos, D3DXVECTOR2 _pos, wxPoint _TextPos, bool _putinBracket){
		numpos = _numpos; 
		pos = _pos; 
		lastpos = pos; 
		TextPos = _TextPos; 
		putinBracket = _putinBracket;
	}
	D3DXVECTOR2 pos;
	D3DXVECTOR2 lastpos;
	wxPoint TextPos;
	bool putinBracket;
	int numpos;
};

class Cross : public Visuals{
public:
	Cross();
	void OnMouseEvent(wxMouseEvent &event);
	void Draw(int time);
	void DrawLines(wxPoint point);
	void SetCurVisual();
	void SizeChanged(wxRect wsize, LPD3DXLINE _line, LPD3DXFONT _font, LPDIRECT3DDEVICE9 _device);
private:
	D3DXVECTOR2 vectors[4];
	RECT crossRect;
	wxString coords;
	bool cross;
	wxMutex m_MutexCrossLines;
	float coeffX, coeffY;
	int diffX = 0, diffY = 0;
	bool isOnVideo = true;
	LPD3DXFONT calcfont;
};

class Position : public Visuals
{
public:
	Position();
	void OnMouseEvent(wxMouseEvent &event);
	wxString GetVisual(int datapos);
	void ChangeMultiline(bool all, bool dummy = false);
	void SetCurVisual();
	void Draw(int time);
	void ChangeTool(int _tool, bool blockSetCurVisual);
	void OnKeyPress(wxKeyEvent &evt);
private:
	int HitTest(const D3DXVECTOR2& pos, bool diff = false);
	void SortPoints();
	void SetPosition();
	D3DXVECTOR2 PositionToVideo(D3DXVECTOR2 point, bool changeX = true, bool changeY = true);
	void GetPositioningData();
	std::vector<PosData> data;
	wxPoint helperLinePos;
	bool hasHelperLine = false;
	bool movingHelperLine = false;
	D3DXVECTOR2 PositionRectangle[2] = { D3DXVECTOR2(0.f, 0.f), D3DXVECTOR2(0.f, 0.f)};
	D3DXVECTOR2 textSize;
	D3DXVECTOR2 border[2];
	D3DXVECTOR2 extlead;
	D3DXVECTOR2 drawingPosition;
	D3DXVECTOR2 diffs;
	D3DXVECTOR2 curLinePosition;
	bool hasPositionToRenctangle = false;
	bool hasPositionX = false;
	bool hasPositionY = false;
	bool rectangleVisible = false;
	int grabbed = -1;
	byte alignment = 1;
	byte curLineAlingment = -1;
};

class Move : public Visuals
{
public:
	Move();
	void DrawVisual(int time);
	void OnMouseEvent(wxMouseEvent &event);
	//void GetVisual(wxString *visual);
	void ChangeVisual(wxString *txt, Dialogue *_dial, size_t numOfSelections);
	wxPoint ChangeVisual(wxString* txt);
	void SetCurVisual();
	void ChangeTool(int _tool, bool blockSetCurVisual);
	void OnKeyPress(wxKeyEvent &evt);
private:
	//returns true if visual can be set or false if only need to refresh video
	bool SetMove();
	int moveStart;
	int moveEnd;
	byte type;
	int grabbed;
	wxPoint diffs;
	D3DXVECTOR2 lastFrom;
	D3DXVECTOR2 lastTo;
	D3DXVECTOR2 moveDistance;
	D3DXVECTOR2 lineToMoveStart = D3DXVECTOR2(0, 0);
	D3DXVECTOR2 lineToMoveEnd = D3DXVECTOR2(0, 0);
	wxPoint helperLinePos;
	bool hasHelperLine = false;
	bool movingHelperLine = false;
	bool hasLineToMove = false;
	bool lineToMoveVisibility[2] = { false, false };
	int lastVideoTime = -1;
	int lineStartTime = -1;
};

class moveElems
{
public:
	moveElems(D3DXVECTOR2 element, byte tagtype, std::vector<ClipPoint>* vPoints = NULL) {
		elem = element;
		type = tagtype;
		vectorPoints = vPoints;
	};
	moveElems() {}
	~moveElems() {
		if (vectorPoints)
			delete vectorPoints;
	}
	D3DXVECTOR2 elem;
	byte type;
	std::vector<ClipPoint>* vectorPoints = NULL;
};

class MoveAll : public Visuals
{
public:
	MoveAll();
	virtual ~MoveAll() {
		Clear();
	};
	void DrawVisual(int time);
	void OnMouseEvent(wxMouseEvent &event);
	void SetCurVisual();
	void ChangeInLines(bool all);
	void ChangeTool(int _tool, bool blockSetCurVisual);
	void OnKeyPress(wxKeyEvent &evt);
	int DrawCurve(int i, std::vector<ClipPoint>* vectorPoints, bool bspline);
	void Curve(int pos, std::vector<ClipPoint>* vectorPoints, std::vector<D3DXVECTOR2>* table, bool bspline, int spoints = 4, int acpt = 0);
	D3DXVECTOR2 GetVector(const ClipPoint& point);
	void DrawLine(int i, std::vector<ClipPoint>* vectorPoints);
	void Clear();
	std::vector<moveElems *> elems;
	int numElem;
	int elemsToMove;
	byte selectedTags;
	wxPoint diffs;
	wxPoint dumplaced;
	D3DXVECTOR2 beforeMove;
	D3DXVECTOR2 drawingPos = D3DXVECTOR2(0, 0);
	D3DXVECTOR2 drawingOriginalPos = D3DXVECTOR2(0, 0);
	D3DXVECTOR2 drawingScale;
	D3DXVECTOR2 scale;
	float vectorClipScale = 1.f;
	float vectorDrawScale = 1.f;
};

class Scale : public Visuals
{
public:
	Scale();
	void DrawVisual(int time);
	void OnMouseEvent(wxMouseEvent &event);
	//void GetVisual(wxString *visual);
	void ChangeVisual(wxString *txt, Dialogue *_dial, size_t numOfSelections);
	wxPoint ChangeVisual(wxString* txt);
	void SetCurVisual();
	void ChangeTool(int _tool, bool blockSetCurVisual);
	void OnKeyPress(wxKeyEvent &evt);
	int HitTest(const D3DXVECTOR2& pos, bool originalRect = false, bool diff = false);
private:
	void SortPoints();
	void SetScale();
	D3DXVECTOR2 ScaleToVideo(D3DXVECTOR2 point);
	void SetSecondRectScale();
	void ChangeClipScale(wxString* txt, const D3DXVECTOR2& activeLinePos, float Scalex, float scaley);
	byte type;
	int grabbed;
	byte AN;
	D3DXVECTOR2 scale = D3DXVECTOR2(1.f, 1.f);
	D3DXVECTOR2 lastScale;
	D3DXVECTOR2 originalScale;
	D3DXVECTOR2 diffs/* = D3DXVECTOR2(0.f, 0.f)*/;
	bool wasUsedShift = false;
	D3DXVECTOR2 arrowLengths = D3DXVECTOR2(100.f, 100.f);
	D3DXVECTOR2 sizingRectangle[4] = { D3DXVECTOR2(0.f, 0.f), D3DXVECTOR2(0.f, 0.f), D3DXVECTOR2(0.f, 0.f), D3DXVECTOR2(0.f, 0.f) };
	D3DXVECTOR2 originalSize;
	D3DXVECTOR2 border;
	bool hasScaleToRenctangle = false;
	bool hasOriginalRectangle = false;
	bool hasScaleX = false;
	bool hasScaleY = false;
	bool preserveAspectRatio = false;
	bool rectangleVisible = false;
	bool originalRectangleVisible = false;
	bool rightHolding = false;
	bool preserveProportions = false;
	bool changeAllTags = false;
};


class RotationZ : public Visuals
{
public:
	RotationZ();
	void DrawVisual(int time);
	void OnMouseEvent(wxMouseEvent &event);
	//void GetVisual(wxString *visual);
	void ChangeVisual(wxString *txt, Dialogue *_dial, size_t numOfSelections);
	wxPoint ChangeVisual(wxString* txt);
	void SetCurVisual();
	void ChangeTool(int _tool, bool blockSetCurVisual);
	void OnKeyPress(wxKeyEvent &evt);
private:
	void ChangeClipRotationZ(wxString* txt, const D3DXVECTOR2& orgPivot, float sinus, float cosinus);
	bool isOrg;
	D3DXVECTOR2 org;
	D3DXVECTOR2 lastOrg;
	D3DXVECTOR2 twoPoints[2];
	D3DXVECTOR2 diffs;
	float lastAngle = 0.f;
	bool hasTwoPoints = false;
	bool hover[2] = { false, false };
	bool visibility[2] = { false, false };
	bool preserveProportions = false;
	bool changeAllTags = false;
	bool isfirst = true;
	int grabbed = -1;
};

class RotationXY : public Visuals
{
public:
	RotationXY();
	void DrawVisual(int time);
	void OnMouseEvent(wxMouseEvent &event);
	//void GetVisual(wxString *visual);
	void ChangeVisual(wxString *txt, Dialogue *_dial, size_t numOfSelections);
	wxPoint ChangeVisual(wxString* txt);
	void SetCurVisual();
	void ChangeTool(int _tool, bool blockSetCurVisual);
	void OnKeyPress(wxKeyEvent &evt);
	bool isOrg;
	D3DXVECTOR2 angle;
	D3DXVECTOR2 oldAngle;
	D3DXVECTOR2 org;
	D3DXVECTOR2 lastOrg;
	byte type;
	byte AN;
	wxPoint diffs;
	bool changeAllTags = false;
};

class ClipRect : public Visuals
{
public:
	ClipRect();
	void DrawVisual(int time);
	void OnMouseEvent(wxMouseEvent &event);
	//void GetVisual(wxString *visual);
	void ChangeVisual(wxString *txt, Dialogue *_dial, size_t numOfSelections);
	wxPoint ChangeVisual(wxString* txt);
	void SetCurVisual();
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
	virtual void OnMouseEvent(wxMouseEvent &event);
	virtual void GetVisual(wxString *visual);
	void ChangeVectorVisual(wxString *txt, wxString *visualText, wxPoint *changePos = NULL, wxString *clipMaskTag = NULL);
	void SetClip(bool dummy, bool redraw = true, bool changeEditorText = true);
	void SetCurVisual();
	void SetPos(int x, int y);
	int CheckPos(D3DXVECTOR2 pos, bool retlast = false, bool wsp = true);
	void MovePoint(D3DXVECTOR2 pos, int point);
	void AddCurve(D3DXVECTOR2 pos, int whereis, wxString type = L"b");
	void AddCurvePoint(D3DXVECTOR2 pos, int whereis);
	void AddLine(D3DXVECTOR2 pos, int whereis);
	void AddMove(D3DXVECTOR2 pos, int whereis);
	void DrawLine(int coord);
	void DrawRect(int coord);
	void DrawCircle(int coord);
	int DrawCurve(int i, bool bspline = false);
	void Curve(int pos, std::vector<D3DXVECTOR2>* table, bool bspline, int spoints = 4, int acpt = 0);
	void SelectPoints();
	void ChangeSelection(bool select = false);
	void ChangeTool(int _tool, bool blockSetCurVisual);
	int FindPoint(int pos, wxString type, bool nextStart = false, bool fromEnd = false);
	ClipPoint FindSnapPoint(const ClipPoint &pos, size_t pointToSkip/*, bool coeff = false*/);
	void OnKeyPress(wxKeyEvent &evt);
	void OnMoveSelected(float x, float y);
	int CheckCurve(int pos, bool checkSpline = true);
	void AppendClipMask(wxString *mask);
	void CreateClipMask(const wxString &clip, wxString *clipTag = NULL);
	void InvertClip();
	void SetZoom(D3DXVECTOR2 move, D3DXVECTOR2 scale) override;
	virtual void SetShape(int shape) {};
	virtual void SetScale(wxString* txt, size_t position, int *diff) {};
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
	int shapeSelection = 0;
	byte alignment;
	wxPoint diffs;
	wxRect selection;
	D3DXVECTOR2 scale = D3DXVECTOR2(1.f, 1.f);
	// _x and _y points of move of drawings
	float _x, _y;
	D3DXVECTOR2 offsetxy;
	D3DXVECTOR2 org;
	float frz = 0.f;
	wxString clipMask;
};

class AllTags : public Visuals
{
	friend class AllTagsSlider;
public:
	AllTags();
	~AllTags() {
	};
	void DrawVisual(int time);
	void OnMouseEvent(wxMouseEvent& event);
	void OnKeyPress(wxKeyEvent& evt);
	void SetCurVisual();
	void FindTagValues();
	void ChangeTool(int _tool, bool blockSetCurVisual);
	void GetVisualValue(wxString* visual, const wxString &curValue);
	wxPoint ChangeVisual(wxString* txt) override;
	void ChangeVisual(wxString* txt, Dialogue* _dial, size_t numOfSelections) override;
private:
	/*enum {
		THUMB_RELEASED = 0,
		THUMB_HOVER,
		THUMB_PUSHED
	};*/
	void CheckRange(float val);
	void CheckTag();
	void OnMouseCaptureLost(wxMouseCaptureLostEvent& evt);
	void SetupSlidersPosition(int _sliderPositionY = 40);
	std::vector<AllTagsSetting> *tags;
	AllTagsSetting actualTag;
	wxString floatFormat = L"5.3f";
	AllTagsSlider slider[4];
	bool rholding = false;
	//bool changeMoveDiff = false;
	int currentTag = 0;
	int sliderPositionY = 40;
	int sliderPositionDiff = 0;
	int increase = 80;
	int mode = 0;
	float multiplyCounter = 0;
	int lastTool = -1;
	int tagMode = 0;
};




