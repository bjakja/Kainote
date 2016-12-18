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

KaiStaticBox::KaiStaticBox(wxWindow *parent, const wxString& _label)
	: wxStaticBox(parent,-1,_label)
	, label(_label)
{
	//Bind(wxEVT_SIZE, &KaiStaticBox::OnSize, this);
	//Bind(wxEVT_PAINT, &KaiStaticBox::OnPaint, this);
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
void KaiStaticBox::PaintForeground(wxDC& tdc, const struct tagRECT& rc)
{
	int w=0;
	int h=0;
	w=rc.right - rc.left;
	h=rc.bottom - rc.top;
	if(w==0||h==0|| !IsShown() || !IsShownOnScreen()){return;}
	
	wxColour background = GetParent()->GetBackgroundColour();
	/*tdc.SetBrush(wxBrush(background));
	tdc.SetPen(wxPen(background));
	tdc.DrawRectangle(0,0,w,h);*/
	tdc.SetFont(GetFont());
	tdc.SetBrush(*wxTRANSPARENT_BRUSH);
	wxSize fsize = tdc.GetTextExtent(label);
	int halfY = fsize.y/2;
	tdc.SetPen(wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT)));
	tdc.DrawRectangle(4, halfY, w-8, h - halfY - 2);
	tdc.SetBackgroundMode(wxPENSTYLE_SOLID);
	tdc.SetTextBackground(background);
	tdc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
	tdc.DrawText(" "+label+" ", 8, 0);
}

wxSize KaiStaticBox::CalcBorders()
{
	return wxSize(8, heightText+5);
}

KaiStaticBoxSizer::KaiStaticBoxSizer(int orient, wxWindow *parent, const wxString& _label)
		:wxBoxSizer(orient)
		,box(new KaiStaticBox(parent, _label))
{
	box->SetContainingSizer(this);
}

KaiStaticBoxSizer::~KaiStaticBoxSizer(){
	if(box){delete box;box=NULL;}
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
    ret.x += 2*borders.x;

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