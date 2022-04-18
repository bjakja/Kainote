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

#include <wx/pen.h>
#include <wx/font.h>
#include <wx/dcclient.h>


enum wxAntialiasMode
{
	wxANTIALIAS_NONE, // should be 0
	wxANTIALIAS_DEFAULT
};

enum wxInterpolationQuality
{
	// default interpolation
	wxINTERPOLATION_DEFAULT,
	// no interpolation
	wxINTERPOLATION_NONE,
	// fast interpolation, suited for interactivity
	wxINTERPOLATION_FAST,
	// better quality
	wxINTERPOLATION_GOOD,
	// best quality, not suited for interactivity
	wxINTERPOLATION_BEST
};

enum wxGradientType
{
	wxGRADIENT_NONE,
	wxGRADIENT_LINEAR,
	wxGRADIENT_RADIAL
};



//enum wxPenJoin
//{
//	wxJOIN_INVALID = -1,
//
//	wxJOIN_BEVEL = 120,
//	wxJOIN_MITER,
//	wxJOIN_ROUND
//};



// ----------------------------------------------------------------------------
// wxPenInfoBase is a common base for wxPenInfo and wxGraphicsPenInfo
// ----------------------------------------------------------------------------

// This class uses CRTP, the template parameter is the derived class itself.
template <class T>
class wxPenInfoBase
{
public:
	// Setters for the various attributes. All of them return the object itself
	// so that the calls to them could be chained.

	T& Colour(const wxColour& colour)
	{
		m_colour = colour; return This();
	}

	T& Style(wxPenStyle style)
	{
		m_style = style; return This();
	}
	T& Stipple(const wxBitmap& stipple)
	{
		m_stipple = stipple; m_style = wxPENSTYLE_STIPPLE; return This();
	}
	T& Dashes(int nb_dashes, const wxDash *dash)
	{
		m_nb_dashes = nb_dashes; m_dash = const_cast<wxDash*>(dash); return This();
	}
	T& Join(wxPenJoin join)
	{
		m_join = join; return This();
	}
	T& Cap(wxPenCap cap)
	{
		m_cap = cap; return This();
	}

	// Accessors are mostly meant to be used by wxWidgets itself.

	wxColour GetColour() const { return m_colour; }
	wxBitmap GetStipple() const { return m_stipple; }
	wxPenStyle GetStyle() const { return m_style; }
	wxPenJoin GetJoin() const { return m_join; }
	wxPenCap GetCap() const { return m_cap; }
	int GetDashes(wxDash **ptr) const { *ptr = m_dash; return m_nb_dashes; }

	int GetDashCount() const { return m_nb_dashes; }
	wxDash* GetDash() const { return m_dash; }

	// Convenience

	bool IsTransparent() const { return m_style == wxPENSTYLE_TRANSPARENT; }

protected:
	wxPenInfoBase(const wxColour& colour, wxPenStyle style)
		: m_colour(colour)
	{
		m_nb_dashes = 0;
		m_dash = nullptr;
		m_join = wxJOIN_ROUND;
		m_cap = wxCAP_ROUND;
		m_style = style;
	}

private:
	// Helper to return this object itself cast to its real type T.
	T& This() { return static_cast<T&>(*this); }

	wxColour m_colour;
	wxBitmap m_stipple;
	wxPenStyle m_style;
	wxPenJoin m_join;
	wxPenCap m_cap;

	int m_nb_dashes;
	wxDash* m_dash;
};



// ----------------------------------------------------------------------------
// wxFontInfo describes a wxFont
// ----------------------------------------------------------------------------

class wxFontInfo
{
public:
	// Default ctor uses the default font size appropriate for the current
	// platform.
	wxFontInfo()
	{
		InitPointSize(-1.0f);
	}

	// These ctors specify the font size, either in points or in pixels.
	// Point size is a floating point number, however we also accept all
	// integer sizes using the simple template ctor below.
	explicit wxFontInfo(float pointSize)
	{
		InitPointSize(pointSize);
	}
	explicit wxFontInfo(const wxSize& pixelSize) : m_pixelSize(pixelSize)
	{
		Init();
	}

	// Need to define this one to avoid casting double to int too.
	explicit wxFontInfo(double pointSize)
	{
		InitPointSize(static_cast<float>(pointSize));
	}
	template <typename T>
	explicit wxFontInfo(T pointSize)
	{
		InitPointSize(ToFloatPointSize(pointSize));
	}

	// Setters for the various attributes. All of them return the object itself
	// so that the calls to them could be chained.
	wxFontInfo& Family(wxFontFamily family)
	{
		m_family = family; return *this;
	}
	wxFontInfo& FaceName(const wxString& faceName)
	{
		m_faceName = faceName; return *this;
	}

	wxFontInfo& Weight(int weight)
	{
		m_weight = weight; return *this;
	}
	wxFontInfo& Bold(bool bold = true)
	{
		return Weight(bold ? wxFONTWEIGHT_BOLD : wxFONTWEIGHT_NORMAL);
	}
	wxFontInfo& Light(bool light = true)
	{
		return Weight(light ? wxFONTWEIGHT_LIGHT : wxFONTWEIGHT_NORMAL);
	}

	wxFontInfo& Italic(bool italic = true)
	{
		SetFlag(wxFONTFLAG_ITALIC, italic); return *this;
	}
	wxFontInfo& Slant(bool slant = true)
	{
		SetFlag(wxFONTFLAG_SLANT, slant); return *this;
	}
	wxFontInfo& Style(wxFontStyle style)
	{
		if (style == wxFONTSTYLE_ITALIC)
			return Italic();

		if (style == wxFONTSTYLE_SLANT)
			return Slant();

		return *this;
	}

	wxFontInfo& AntiAliased(bool antiAliased = true)
	{
		SetFlag(wxFONTFLAG_ANTIALIASED, antiAliased); return *this;
	}
	wxFontInfo& Underlined(bool underlined = true)
	{
		SetFlag(wxFONTFLAG_UNDERLINED, underlined); return *this;
	}
	wxFontInfo& Strikethrough(bool strikethrough = true)
	{
		SetFlag(wxFONTFLAG_STRIKETHROUGH, strikethrough); return *this;
	}

	wxFontInfo& Encoding(wxFontEncoding encoding)
	{
		m_encoding = encoding; return *this;
	}


	// Set all flags at once.
	wxFontInfo& AllFlags(int flags)
	{
		m_flags = flags;

		m_weight = m_flags & wxFONTFLAG_BOLD
			? wxFONTWEIGHT_BOLD
			: m_flags & wxFONTFLAG_LIGHT
			? wxFONTWEIGHT_LIGHT
			: wxFONTWEIGHT_NORMAL;

		return *this;
	}


	// Accessors are mostly meant to be used by wxFont itself to extract the
	// various pieces of the font description.

	bool IsUsingSizeInPixels() const { return m_pixelSize != wxDefaultSize; }
	float GetFractionalPointSize() const { return m_pointSize; }
	int GetPointSize() const { return ToIntPointSize(m_pointSize); }
	wxSize GetPixelSize() const { return m_pixelSize; }

	// If face name is not empty, it has priority, otherwise use family.
	bool HasFaceName() const { return !m_faceName.empty(); }
	wxFontFamily GetFamily() const { return m_family; }
	const wxString& GetFaceName() const { return m_faceName; }

	wxFontStyle GetStyle() const
	{
		return m_flags & wxFONTFLAG_ITALIC
			? wxFONTSTYLE_ITALIC
			: m_flags & wxFONTFLAG_SLANT
			? wxFONTSTYLE_SLANT
			: wxFONTSTYLE_NORMAL;
	}

	int GetNumericWeight() const
	{
		return m_weight;
	}

	wxFontWeight GetWeight() const
	{
		return GetWeightClosestToNumericValue(m_weight);
	}

	bool IsAntiAliased() const
	{
		return (m_flags & wxFONTFLAG_ANTIALIASED) != 0;
	}

	bool IsUnderlined() const
	{
		return (m_flags & wxFONTFLAG_UNDERLINED) != 0;
	}

	bool IsStrikethrough() const
	{
		return (m_flags & wxFONTFLAG_STRIKETHROUGH) != 0;
	}

	wxFontEncoding GetEncoding() const { return m_encoding; }


	// Default copy ctor, assignment operator and dtor are OK.


	// Helper functions for converting between integer and fractional sizes.
	static int ToIntPointSize(float pointSize) { return wxRound(pointSize); }
	static float ToFloatPointSize(int pointSize)
	{
		//wxCHECK_MSG(pointSize == -1 || pointSize >= 0,
			//-1, "Invalid font point size");

		// Huge values are not exactly representable as floats, so don't accept
		// those neither as they can only be due to a mistake anyhow: nobody
		// could possibly need a font of size 16777217pt (which is the first
		// value for which this fails).
		const float f = static_cast<float>(pointSize);
		//wxCHECK_MSG(static_cast<int>(f) == pointSize,
			//-1, "Font point size out of range");

		return f;
	}

	// Another helper for converting arbitrary numeric weight to the closest
	// value of wxFontWeight enum. It should be avoided in the new code (also
	// note that the function for the conversion in the other direction is
	// trivial and so is not provided, we only have GetNumericWeightOf() which
	// contains backwards compatibility hacks, but we don't need it here).
	static wxFontWeight GetWeightClosestToNumericValue(int numWeight)
	{
		if (numWeight > 0) {
			return;
		};
		if(numWeight <= 1000) {
			return;
		};

		// round to nearest hundredth = wxFONTWEIGHT_ constant
		int weight = ((numWeight + 50) / 100) * 100;

		if (weight < wxFONTWEIGHT_NORMAL)
			weight = wxFONTWEIGHT_NORMAL;
		if (weight > wxFONTWEIGHT_MAX)
			weight = wxFONTWEIGHT_MAX;

		return static_cast<wxFontWeight>(weight);
	}

private:
	void Init()
	{
		m_pointSize = -1;
		m_family = wxFONTFAMILY_DEFAULT;
		m_flags = wxFONTFLAG_DEFAULT;
		m_weight = wxFONTWEIGHT_NORMAL;
		m_encoding = wxFONTENCODING_DEFAULT;
	}

	void InitPointSize(float pointSize)
	{
		Init();

		m_pointSize = pointSize;
		m_pixelSize = wxDefaultSize;
	}

	// Turn on or off the given bit in m_flags depending on the value of the
	// boolean argument.
	void SetFlag(int flag, bool on)
	{
		if (on)
			m_flags |= flag;
		else
			m_flags &= ~flag;
	}

	// The size information: if m_pixelSize is valid (!= wxDefaultSize), then
	// it is used. Otherwise m_pointSize is used, except if it is < 0, which
	// means that the platform dependent font size should be used instead.
	float m_pointSize;
	wxSize m_pixelSize;

	wxFontFamily m_family;
	wxString m_faceName;
	int m_flags;
	int m_weight;
	wxFontEncoding m_encoding;
};

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
	virtual void Get(wxDouble* a = nullptr, wxDouble* b = nullptr, wxDouble* c = nullptr,
		wxDouble* d = nullptr, wxDouble* tx = nullptr, wxDouble* ty = nullptr) const = 0;

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
	virtual void* GetNativeContext(){ return nullptr; };

	virtual bool SetAntialiasMode(wxAntialiasMode antialias){ return false; };

	virtual bool SetInterpolationQuality(wxInterpolationQuality interpolation){ return false; };

	virtual void BeginLayer(wxDouble opacity){};

	virtual void EndLayer(){};

	virtual void Translate(wxDouble dx, wxDouble dy){};

	virtual void Scale(wxDouble xScale, wxDouble yScale){};

	virtual void Rotate(wxDouble angle){};

	virtual void ConcatTransform(GraphicsMatrixData* matrix){};

	virtual void SetTransform(GraphicsMatrixData* matrix){};

	virtual GraphicsMatrixData *GetTransform() const{ return nullptr; };

	virtual void StrokePath(GraphicsPathData * p){};

	virtual void FillPath(GraphicsPathData * p, wxPolygonFillMode fillStyle = wxODDEVEN_RULE){};

	virtual void DrawRectangle(wxDouble x, wxDouble y, wxDouble w, wxDouble h){};

	virtual void DrawRoundedRectangle(wxDouble x, wxDouble y, wxDouble w, wxDouble h, wxDouble radius){};

	virtual void DrawEllipse(wxDouble x, wxDouble y, wxDouble w, wxDouble h){};

	virtual void DrawBitmap(const wxBitmap& bmp, wxDouble x, wxDouble y, wxDouble w, wxDouble h){};

	virtual void DrawIcon(const wxIcon& icon, wxDouble x, wxDouble y, wxDouble w, wxDouble h){};

	virtual GraphicsPathData * CreatePath(){ return nullptr; };

	virtual void SetPen(const wxPen& pen, double width = 1.0){};

	virtual void SetBrush(const wxBrush& brush){};

	virtual void SetFont(const wxFont& font, const wxColour& col){};

	virtual void PushState(){};

	virtual void PopState(){};

	virtual void GetTextExtent(
		const wxString& str,
		wxDouble* width,
		wxDouble* height = nullptr,
		wxDouble* descent = nullptr,
		wxDouble* externalLeading = nullptr) const {};

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

	virtual GraphicsContext * CreateContext(const wxWindowDC& dc){ return nullptr; };

	virtual GraphicsContext * CreateContext(const wxMemoryDC& dc){ return nullptr; };

	virtual GraphicsContext * CreateContextFromNativeContext(void* context){ return nullptr; };

	virtual GraphicsContext * CreateContextFromNativeWindow(void* window){ return nullptr; };

	virtual GraphicsContext * CreateContextFromNativeHDC(WXHDC dc){ return nullptr; };

	virtual GraphicsContext * CreateContext(wxWindow* window){ return nullptr; };

#if wxUSE_IMAGE
	virtual GraphicsContext * CreateContextFromImage(wxImage& image){ return nullptr; };
#endif // wxUSE_IMAGE

	virtual GraphicsContext * CreateMeasuringContext(){ return nullptr; };

};

