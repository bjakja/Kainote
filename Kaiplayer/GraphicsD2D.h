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

#include <d2d1.h>
#include <dwrite.h>
#include <wincodec.h>

IWICImagingFactory* wxWICImagingFactory();
ID2D1Factory* wxD2D1Factory();
IDWriteFactory* wxDWriteFactory();

#include "wx/bitmap.h"
#include "wx/colour.h"
#include "wx/gdicmn.h"  // for wxDash

enum wxGradientType
{
	wxGRADIENT_NONE,
	wxGRADIENT_LINEAR,
	wxGRADIENT_RADIAL
};

enum wxPenStyle
{
	wxPENSTYLE_INVALID = -1,

	//wxPENSTYLE_SOLID = wxSOLID,
	wxPENSTYLE_DOT = wxDOT,
	wxPENSTYLE_LONG_DASH = wxLONG_DASH,
	wxPENSTYLE_SHORT_DASH = wxSHORT_DASH,
	wxPENSTYLE_DOT_DASH = wxDOT_DASH,
	wxPENSTYLE_USER_DASH = wxUSER_DASH,

	wxPENSTYLE_TRANSPARENT = wxTRANSPARENT,

	wxPENSTYLE_STIPPLE_MASK_OPAQUE = wxSTIPPLE_MASK_OPAQUE,
	wxPENSTYLE_STIPPLE_MASK = wxSTIPPLE_MASK,
	wxPENSTYLE_STIPPLE = wxSTIPPLE,

	wxPENSTYLE_BDIAGONAL_HATCH = wxHATCHSTYLE_BDIAGONAL,
	wxPENSTYLE_CROSSDIAG_HATCH = wxHATCHSTYLE_CROSSDIAG,
	wxPENSTYLE_FDIAGONAL_HATCH = wxHATCHSTYLE_FDIAGONAL,
	wxPENSTYLE_CROSS_HATCH = wxHATCHSTYLE_CROSS,
	wxPENSTYLE_HORIZONTAL_HATCH = wxHATCHSTYLE_HORIZONTAL,
	wxPENSTYLE_VERTICAL_HATCH = wxHATCHSTYLE_VERTICAL,
	wxPENSTYLE_FIRST_HATCH = wxHATCHSTYLE_FIRST,
	wxPENSTYLE_LAST_HATCH = wxHATCHSTYLE_LAST
};

enum wxPenJoin
{
	wxJOIN_INVALID = -1,

	wxJOIN_BEVEL = 120,
	wxJOIN_MITER,
	wxJOIN_ROUND
};

enum wxPenCap
{
	wxCAP_INVALID = -1,

	wxCAP_ROUND = 130,
	wxCAP_PROJECTING,
	wxCAP_BUTT
};

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
		m_dash = NULL;
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
// wxGraphicsPenInfo describes a wxGraphicsPen
// ----------------------------------------------------------------------------

class wxGraphicsPenInfo : public wxPenInfoBase<wxGraphicsPenInfo>
{
public:
	explicit wxGraphicsPenInfo(const wxColour& colour = wxColour(),
		wxDouble width = 1.0,
		wxPenStyle style = wxPENSTYLE_SOLID)
		: wxPenInfoBase<wxGraphicsPenInfo>(colour, style)
	{
		m_width = width;
		m_gradientType = wxGRADIENT_NONE;
	}

	// Setters

	wxGraphicsPenInfo& Width(wxDouble width)
	{
		m_width = width; return *this;
	}

	wxGraphicsPenInfo&
		LinearGradient(wxDouble x1, wxDouble y1, wxDouble x2, wxDouble y2,
		const wxColour& c1, const wxColour& c2,
		const wxGraphicsMatrix& matrix = wxNullGraphicsMatrix)
	{
		m_gradientType = wxGRADIENT_LINEAR;
		m_x1 = x1;
		m_y1 = y1;
		m_x2 = x2;
		m_y2 = y2;
		m_stops.SetStartColour(c1);
		m_stops.SetEndColour(c2);
		m_matrix = matrix;
		return *this;
	}

	wxGraphicsPenInfo&
		LinearGradient(wxDouble x1, wxDouble y1, wxDouble x2, wxDouble y2,
		const wxGraphicsGradientStops& stops,
		const wxGraphicsMatrix& matrix = wxNullGraphicsMatrix)
	{
		m_gradientType = wxGRADIENT_LINEAR;
		m_x1 = x1;
		m_y1 = y1;
		m_x2 = x2;
		m_y2 = y2;
		m_stops = stops;
		m_matrix = matrix;
		return *this;
	}

	wxGraphicsPenInfo&
		RadialGradient(wxDouble startX, wxDouble startY,
		wxDouble endX, wxDouble endY, wxDouble radius,
		const wxColour& oColor, const wxColour& cColor,
		const wxGraphicsMatrix& matrix = wxNullGraphicsMatrix)
	{
		m_gradientType = wxGRADIENT_RADIAL;
		m_x1 = startX;
		m_y1 = startY;
		m_x2 = endX;
		m_y2 = endY;
		m_radius = radius;
		m_stops.SetStartColour(oColor);
		m_stops.SetEndColour(cColor);
		m_matrix = matrix;
		return *this;
	}

	wxGraphicsPenInfo&
		RadialGradient(wxDouble startX, wxDouble startY,
		wxDouble endX, wxDouble endY,
		wxDouble radius, const wxGraphicsGradientStops& stops,
		const wxGraphicsMatrix& matrix = wxNullGraphicsMatrix)
	{
		m_gradientType = wxGRADIENT_RADIAL;
		m_x1 = startX;
		m_y1 = startY;
		m_x2 = endX;
		m_y2 = endY;
		m_radius = radius;
		m_stops = stops;
		m_matrix = matrix;
		return *this;
	}

	// Accessors

	wxDouble GetWidth() const { return m_width; }
	wxGradientType GetGradientType() const { return m_gradientType; }
	wxDouble GetX1() const { return m_x1; }
	wxDouble GetY1() const { return m_y1; }
	wxDouble GetX2() const { return m_x2; }
	wxDouble GetY2() const { return m_y2; }
	wxDouble GetStartX() const { return m_x1; }
	wxDouble GetStartY() const { return m_y1; }
	wxDouble GetEndX() const { return m_x2; }
	wxDouble GetEndY() const { return m_y2; }
	wxDouble GetRadius() const { return m_radius; }
	const wxGraphicsGradientStops& GetStops() const { return m_stops; }
	const wxGraphicsMatrix& GetMatrix() const { return m_matrix; }

private:
	wxDouble m_width;
	wxGradientType m_gradientType;
	wxDouble m_x1, m_y1, m_x2, m_y2; // also used for m_xo, m_yo, m_xc, m_yc
	wxDouble m_radius;
	wxGraphicsGradientStops m_stops;
	wxGraphicsMatrix m_matrix;
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
		wxCHECK_MSG(pointSize == -1 || pointSize >= 0,
			-1, "Invalid font point size");

		// Huge values are not exactly representable as floats, so don't accept
		// those neither as they can only be due to a mistake anyhow: nobody
		// could possibly need a font of size 16777217pt (which is the first
		// value for which this fails).
		const float f = static_cast<float>(pointSize);
		wxCHECK_MSG(static_cast<int>(f) == pointSize,
			-1, "Font point size out of range");

		return f;
	}

	// Another helper for converting arbitrary numeric weight to the closest
	// value of wxFontWeight enum. It should be avoided in the new code (also
	// note that the function for the conversion in the other direction is
	// trivial and so is not provided, we only have GetNumericWeightOf() which
	// contains backwards compatibility hacks, but we don't need it here).
	static wxFontWeight GetWeightClosestToNumericValue(int numWeight)
	{
		wxASSERT(numWeight > 0);
		wxASSERT(numWeight <= 1000);

		// round to nearest hundredth = wxFONTWEIGHT_ constant
		int weight = ((numWeight + 50) / 100) * 100;

		if (weight < wxFONTWEIGHT_THIN)
			weight = wxFONTWEIGHT_THIN;
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
