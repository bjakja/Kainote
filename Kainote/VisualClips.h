//  Copyright (c) 2022, Marcin Drob

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
#include "VisualClipPoint.h"
#include "Visuals.h"
#include <wx/string.h>
#include <wx/window.h>
#include <d3d9.h>
#include <d3dx9.h>


class DrawingAndClip : public Visuals
{
public:
	DrawingAndClip();
	~DrawingAndClip();
	void DrawVisual(int time);
	virtual void OnMouseEvent(wxMouseEvent& event);
	virtual void GetVisual(wxString* visual);
	void ChangeVectorVisual(wxString* txt, wxString* visualText, wxPoint* changePos = nullptr, wxString* clipMaskTag = nullptr);
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
	ClipPoint FindSnapPoint(const ClipPoint& pos, size_t pointToSkip/*, bool coeff = false*/);
	void OnKeyPress(wxKeyEvent& evt);
	void OnMoveSelected(float x, float y);
	int CheckCurve(int pos, bool checkSpline = true);
	void AppendClipMask(wxString* mask);
	void CreateClipMask(const wxString& clip, wxString* clipTag = nullptr);
	void InvertClip();
	void SetZoom(D3DXVECTOR2 move, D3DXVECTOR2 scale) /*override*/;
	virtual void SetShape(int shape) {};
	virtual void SetScale(wxString* txt, size_t position, int* diff) {};
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

