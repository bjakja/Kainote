/////////////////////////////////////////////////////////////////////////////
// Name:        wx/private/graphics.h
// Purpose:     private graphics context header
// Author:      Stefan Csomor
// Modified by:
// Created:
// Copyright:   (c) Stefan Csomor
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////


#pragma once

#include <wx/colour.h>
#include <wx/gdicmn.h>  // for wxDash
#include <wx/object.h>
#include <wx/font.h>

enum wxAntialiasMode;
//{
//	wxANTIALIAS_NONE, // should be 0
//	wxANTIALIAS_DEFAULT
//};
//
enum wxInterpolationQuality;
//{
//	// default interpolation
//	wxINTERPOLATION_DEFAULT,
//	// no interpolation
//	wxINTERPOLATION_NONE,
//	// fast interpolation, suited for interactivity
//	wxINTERPOLATION_FAST,
//	// better quality
//	wxINTERPOLATION_GOOD,
//	// best quality, not suited for interactivity
//	wxINTERPOLATION_BEST
//};

enum wxGradientType
{
	wxGRADIENT_NONE,
	wxGRADIENT_LINEAR,
	wxGRADIENT_RADIAL
};

//enum wxPenStyle
//{
//	wxPENSTYLE_INVALID = -1,
//
//	//wxPENSTYLE_SOLID = wxSOLID,
//	wxPENSTYLE_DOT = wxDOT,
//	wxPENSTYLE_LONG_DASH = wxLONG_DASH,
//	wxPENSTYLE_SHORT_DASH = wxSHORT_DASH,
//	wxPENSTYLE_DOT_DASH = wxDOT_DASH,
//	wxPENSTYLE_USER_DASH = wxUSER_DASH,
//
//	wxPENSTYLE_TRANSPARENT = wxTRANSPARENT,
//
//	wxPENSTYLE_STIPPLE_MASK_OPAQUE = wxSTIPPLE_MASK_OPAQUE,
//	wxPENSTYLE_STIPPLE_MASK = wxSTIPPLE_MASK,
//	wxPENSTYLE_STIPPLE = wxSTIPPLE,
//
//	wxPENSTYLE_BDIAGONAL_HATCH = wxHATCHSTYLE_BDIAGONAL,
//	wxPENSTYLE_CROSSDIAG_HATCH = wxHATCHSTYLE_CROSSDIAG,
//	wxPENSTYLE_FDIAGONAL_HATCH = wxHATCHSTYLE_FDIAGONAL,
//	wxPENSTYLE_CROSS_HATCH = wxHATCHSTYLE_CROSS,
//	wxPENSTYLE_HORIZONTAL_HATCH = wxHATCHSTYLE_HORIZONTAL,
//	wxPENSTYLE_VERTICAL_HATCH = wxHATCHSTYLE_VERTICAL,
//	wxPENSTYLE_FIRST_HATCH = wxHATCHSTYLE_FIRST,
//	wxPENSTYLE_LAST_HATCH = wxHATCHSTYLE_LAST
//};

enum wxPenJoin;
//{
//	wxJOIN_INVALID = -1,
//
//	wxJOIN_BEVEL = 120,
//	wxJOIN_MITER,
//	wxJOIN_ROUND
//};

enum wxPenCap;
//{
//	wxCAP_INVALID = -1,
//
//	wxCAP_ROUND = 130,
//	wxCAP_PROJECTING,
//	wxCAP_BUTT
//};

// ----------------------------------------------------------------------------
// wxPenInfoBase is a common base for wxPenInfo and wxGraphicsPenInfo
// ----------------------------------------------------------------------------



class wxD2DRenderer;

//class GraphicsObjectRefData : public wxObjectRefData
//{
//public:
//	GraphicsObjectRefData(wxD2DRenderer* renderer);
//	GraphicsObjectRefData(const GraphicsObjectRefData* data);
//	wxD2DRenderer* GetRenderer() const;
//	virtual GraphicsObjectRefData* Clone() const;
//
//protected:
//	wxD2DRenderer* m_renderer;
//};

class GraphicsBitmapData //: public GraphicsObjectRefData
{
public:
	GraphicsBitmapData(wxD2DRenderer* renderer) /*:
		GraphicsObjectRefData(renderer) */{}

	virtual ~GraphicsBitmapData() {}

	// returns the native representation
	virtual void * GetNativeBitmap() const = 0;
};

class GraphicsMatrixData// : public GraphicsObjectRefData
{
public:
	GraphicsMatrixData(wxD2DRenderer* renderer) /*:
		GraphicsObjectRefData(renderer) */{}

	virtual ~GraphicsMatrixData() {}

	// concatenates the matrix
	virtual void Concat(const GraphicsMatrixData *t) = 0;

	// sets the matrix to the respective values
	virtual void Set(wxDouble a = 1.0, wxDouble b = 0.0, wxDouble c = 0.0, wxDouble d = 1.0,
		wxDouble tx = 0.0, wxDouble ty = 0.0) = 0;

	// gets the component valuess of the matrix
	virtual void Get(wxDouble* a = NULL, wxDouble* b = NULL, wxDouble* c = NULL,
		wxDouble* d = NULL, wxDouble* tx = NULL, wxDouble* ty = NULL) const = 0;

	// makes this the inverse matrix
	virtual void Invert() = 0;

	// returns true if the elements of the transformation matrix are equal ?
	virtual bool IsEqual(const GraphicsMatrixData* t) const = 0;

	// return true if this is the identity matrix
	virtual bool IsIdentity() const = 0;

	//
	// transformation
	//

	// add the translation to this matrix
	virtual void Translate(wxDouble dx, wxDouble dy) = 0;

	// add the scale to this matrix
	virtual void Scale(wxDouble xScale, wxDouble yScale) = 0;

	// add the rotation to this matrix (radians)
	virtual void Rotate(wxDouble angle) = 0;

	//
	// apply the transforms
	//

	// applies that matrix to the point
	virtual void TransformPoint(wxDouble *x, wxDouble *y) const = 0;

	// applies the matrix except for translations
	virtual void TransformDistance(wxDouble *dx, wxDouble *dy) const = 0;

	// returns the native representation
	virtual void * GetNativeMatrix() const = 0;
};

class GraphicsPathData/* : public GraphicsObjectRefData*/
{
public:
	GraphicsPathData(wxD2DRenderer* renderer)/* : GraphicsObjectRefData(renderer) */{}
	virtual ~GraphicsPathData() {}

	//
	// These are the path primitives from which everything else can be constructed
	//

	// begins a new subpath at (x,y)
	virtual void MoveToPoint(wxDouble x, wxDouble y) = 0;

	// adds a straight line from the current point to (x,y)
	virtual void AddLineToPoint(wxDouble x, wxDouble y) = 0;

	// adds a cubic Bezier curve from the current point, using two control points and an end point
	virtual void AddCurveToPoint(wxDouble cx1, wxDouble cy1, wxDouble cx2, wxDouble cy2, wxDouble x, wxDouble y) = 0;

	// adds another path
	virtual void AddPath(const GraphicsPathData* path) = 0;

	// closes the current sub-path
	virtual void CloseSubpath() = 0;

	// gets the last point of the current path, (0,0) if not yet set
	virtual void GetCurrentPoint(wxDouble* x, wxDouble* y) const = 0;

	// adds an arc of a circle centering at (x,y) with radius (r) from startAngle to endAngle
	virtual void AddArc(wxDouble x, wxDouble y, wxDouble r, wxDouble startAngle, wxDouble endAngle, bool clockwise) = 0;

	//
	// These are convenience functions which - if not available natively will be assembled
	// using the primitives from above
	//

	// adds a quadratic Bezier curve from the current point, using a control point and an end point
	virtual void AddQuadCurveToPoint(wxDouble cx, wxDouble cy, wxDouble x, wxDouble y){};

	// appends a rectangle as a new closed subpath
	virtual void AddRectangle(wxDouble x, wxDouble y, wxDouble w, wxDouble h){};

	// appends an ellipsis as a new closed subpath fitting the passed rectangle
	virtual void AddCircle(wxDouble x, wxDouble y, wxDouble r){};

	// appends a an arc to two tangents connecting (current) to (x1,y1) and (x1,y1) to (x2,y2), also a straight line from (current) to (x1,y1)
	virtual void AddArcToPoint(wxDouble x1, wxDouble y1, wxDouble x2, wxDouble y2, wxDouble r){};

	// appends an ellipse
	virtual void AddEllipse(wxDouble x, wxDouble y, wxDouble w, wxDouble h){};

	// appends a rounded rectangle
	virtual void AddRoundedRectangle(wxDouble x, wxDouble y, wxDouble w, wxDouble h, wxDouble radius){};

	// returns the native path
	virtual void * GetNativePath() const = 0;

	// give the native path returned by GetNativePath() back (there might be some deallocations necessary)
	virtual void UnGetNativePath(void *p) const = 0;

	// transforms each point of this path by the matrix
	virtual void Transform(const GraphicsMatrixData* matrix) = 0;

	// gets the bounding box enclosing all points (possibly including control points)
	virtual void GetBox(wxDouble *x, wxDouble *y, wxDouble *w, wxDouble *h) const = 0;

	virtual bool Contains(wxDouble x, wxDouble y, wxPolygonFillMode fillStyle = wxODDEVEN_RULE) const = 0;
};

class GraphicsContext 
{
public:
	static wxGraphicsContext * Create(const wxWindowDC& dc);
	static wxGraphicsContext * Create(const wxMemoryDC& dc);
	static wxGraphicsContext * Create(wxWindow* window);

	virtual ~GraphicsContext(){};

	virtual void Clip(const wxRegion& region){};

	virtual void Clip(wxDouble x, wxDouble y, wxDouble w, wxDouble h){};

	virtual void ResetClip(){};
	// The native context used by GraphicsContext is a Direct2D render target.
	virtual void* GetNativeContext(){ return NULL; };

	virtual bool SetAntialiasMode(wxAntialiasMode antialias){ return false; };

	virtual bool SetInterpolationQuality(wxInterpolationQuality interpolation){ return false; };

	virtual void BeginLayer(wxDouble opacity){};

	virtual void EndLayer(){};

	virtual void Translate(wxDouble dx, wxDouble dy){};

	virtual void Scale(wxDouble xScale, wxDouble yScale){};

	virtual void Rotate(wxDouble angle){};

	virtual void ConcatTransform(GraphicsMatrixData* matrix){};

	virtual void SetTransform(GraphicsMatrixData* matrix){};

	virtual GraphicsMatrixData *GetTransform() const{ return NULL; };

	virtual void StrokePath(GraphicsPathData * p){};

	virtual void FillPath(GraphicsPathData * p, wxPolygonFillMode fillStyle = wxODDEVEN_RULE){};

	virtual void DrawRectangle(wxDouble x, wxDouble y, wxDouble w, wxDouble h){};

	virtual void DrawRoundedRectangle(wxDouble x, wxDouble y, wxDouble w, wxDouble h, wxDouble radius){};

	virtual void DrawEllipse(wxDouble x, wxDouble y, wxDouble w, wxDouble h){};

	virtual void DrawBitmap(const wxBitmap& bmp, wxDouble x, wxDouble y, wxDouble w, wxDouble h){};

	virtual void DrawIcon(const wxIcon& icon, wxDouble x, wxDouble y, wxDouble w, wxDouble h){};

	virtual GraphicsPathData * CreatePath(){ return NULL; };

	virtual void SetPen(const wxPen& pen, double width = 1.0){};

	virtual void SetBrush(const wxBrush& brush){};

	virtual void SetFont(const wxFont& font, const wxColour& col){};

	virtual void PushState(){};

	virtual void PopState(){};

	virtual void GetTextExtent(
		const wxString& str,
		wxDouble* width,
		wxDouble* height = NULL,
		wxDouble* descent = NULL,
		wxDouble* externalLeading = NULL) const {};

	virtual void GetPartialTextExtents(const wxString& text, wxArrayDouble& widths) const {};

	virtual void Flush(){};

	virtual void GetDPI(wxDouble* dpiX, wxDouble* dpiY) const {};
	// non virtual functions
	void StrokeLine(wxDouble x1, wxDouble y1, wxDouble x2, wxDouble y2);

	void DrawTextU(const wxString& str, wxDouble x, wxDouble y);
private:
	virtual void DoDrawText(const wxString& str, wxDouble x, wxDouble y){};
};

//-----------------------------------------------------------------------------
// wxD2DRenderer declaration
//-----------------------------------------------------------------------------

class GraphicsRenderer
{
public:
	GraphicsRenderer(){};

	virtual ~GraphicsRenderer(){};

	static GraphicsRenderer * GetDirect2DRenderer();

	virtual GraphicsContext * CreateContext(const wxWindowDC& dc){ return NULL; };

	virtual GraphicsContext * CreateContext(const wxMemoryDC& dc){ return NULL; };

	virtual GraphicsContext * CreateContextFromNativeContext(void* context){ return NULL; };

	virtual GraphicsContext * CreateContextFromNativeWindow(void* window){ return NULL; };

	virtual GraphicsContext * CreateContextFromNativeHDC(WXHDC dc){ return NULL; };

	virtual GraphicsContext * CreateContext(wxWindow* window){ return NULL; };

#if wxUSE_IMAGE
	virtual GraphicsContext * CreateContextFromImage(wxImage& image){ return NULL; };
#endif // wxUSE_IMAGE

	virtual GraphicsContext * CreateMeasuringContext(){ return NULL; };

};

//#endif