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

#include "KaiStaticBoxSizer.h"
#include "config.h"

namespace
{

	// Offset of the first pixel of the label from the box left border.
	//
	// FIXME: value is hardcoded as this is what it is on my system, no idea if
	//        it's true everywhere
	const int LABEL_HORZ_OFFSET = 9;

	// Extra borders around the label on left/right and bottom sides.
	const int LABEL_HORZ_BORDER = 2;
	const int LABEL_VERT_BORDER = 2;

	// Offset of the box contents from left/right/bottom edge (top one is
	// different, see GetBordersForSizer()). This one is completely arbitrary.
	const int CHILDREN_OFFSET = 5;

} // anonymous namespace


KaiStaticBox::KaiStaticBox(wxWindow *parent, const wxString& _label)
	: wxStaticBox(parent,-1,_label)
	, label(_label)
{
	//Bind(wxEVT_SIZE, &KaiStaticBox::OnSize, this);
	//Bind(wxEVT_PAINT, &KaiStaticBox::OnPaint, this);
	Bind(wxEVT_ERASE_BACKGROUND,[=](wxEraseEvent &evt){});
	SetFont(parent->GetFont());
	wxSize fsize = GetTextExtent(label);
	SetInitialSize(wxSize(fsize.x+16, fsize.y+10));
	heightText = fsize.y;
}


//void KaiStaticBox::OnSize(wxSizeEvent& event)
//{
//	Refresh(false);
//}
//	
void KaiStaticBox::PaintForeground(wxDC& tdc, const RECT& rc)
{
	int w=0;
	int h=0;
	int width, height;
	tdc.GetTextExtent(label, &width, &height);

	// first we need to correctly paint the background of the label
	// as Windows ignores the brush offset when doing it
	const int x = FromDIP(LABEL_HORZ_OFFSET);
	RECT dimensions = { x, 0, 0, height };
	dimensions.left = x;
	dimensions.right = x + width;

	// need to adjust the rectangle to cover all the label background
	dimensions.left -= FromDIP(LABEL_HORZ_BORDER);
	dimensions.right += FromDIP(LABEL_HORZ_BORDER);
	dimensions.bottom += FromDIP(LABEL_VERT_BORDER);
	w = dimensions.right - dimensions.left;
	h = dimensions.bottom - dimensions.top;
	KaiLog(wxString::Format(L"right: %i; left: %i; bottom: %i; top: %i", dimensions.right, dimensions.left, dimensions.bottom, dimensions.top));
	if(w==0||h==0|| !IsShown() || !IsShownOnScreen()){return;}
	
	wxColour background = GetParent()->GetBackgroundColour();
	/*tdc.SetBrush(wxBrush(background));
	tdc.SetPen(wxPen(background));
	tdc.DrawRectangle(0,0,w,h);*/
	tdc.SetFont(GetFont());
	tdc.SetBrush(*wxTRANSPARENT_BRUSH);
	wxSize fsize = tdc.GetTextExtent(label);
	int halfY = fsize.y/2;
	tdc.SetPen(wxPen(Options.GetColour(StaticboxBorder)));
	tdc.DrawRectangle(4, halfY, w-8, h - halfY - 2);
	tdc.SetBackgroundMode(wxPENSTYLE_SOLID);
	tdc.SetTextBackground(background);
	tdc.SetTextForeground(GetParent()->GetForegroundColour());
	tdc.DrawText(" "+label+" ", 8, 0);
}

wxSize KaiStaticBox::CalcBorders()
{
	return wxSize(8, heightText+5);
}

bool KaiStaticBox::Enable(bool enable)
{
	bool succ = wxWindow::Enable(enable);
	Refresh(false);
	return succ;
}

KaiStaticBoxSizer::KaiStaticBoxSizer(int orient, wxWindow *parent, const wxString& _label)
		:wxBoxSizer(orient)
		,box(new KaiStaticBox(parent, _label))
{
	box->SetContainingSizer(this);
}

KaiStaticBoxSizer::~KaiStaticBoxSizer(){
	if(box){delete box; box=NULL;}
};

void KaiStaticBoxSizer::RecalcSizes()
{
	wxSize borders = box->CalcBorders();

	box->SetSize( m_position.x, m_position.y, m_size.x, m_size.y );
	wxSize old_size( m_size );
    m_size.x -= 2*borders.x;
    m_size.y -= borders.y + borders.x;
	wxPoint old_pos( m_position );
	m_position.x += borders.x;
    m_position.y += borders.y;

	wxBoxSizer::RecalcSizes();

    m_position = old_pos;
    m_size = old_size;
}

wxSize KaiStaticBoxSizer::CalcMin()
{
	wxSize borders = box->CalcBorders();

    wxSize ret( wxBoxSizer::CalcMin() );
    ret.x += 2 * borders.x;

    // ensure that we're wide enough to show the static box label (there is no
    // need to check for the static box best size in vertical direction though)
    const int boxWidth = box->GetBestSize().x;
    if ( ret.x < boxWidth )
        ret.x = boxWidth;

    ret.y += borders.x + borders.y;

    return ret;
}

void KaiStaticBoxSizer::ShowItems( bool show )
{
	box->Show( show );
    wxBoxSizer::ShowItems( show );
}

bool KaiStaticBoxSizer::Detach( wxWindow *window )
{
	if ( window == box )
    {
        box = NULL;
        return true;
    }

    return wxSizer::Detach( window );
}

bool KaiStaticBoxSizer::Enable(bool enable)
{
	return box->Enable(enable);
}