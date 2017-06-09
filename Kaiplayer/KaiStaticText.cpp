//  Copyright (c) 2017, Marcin Drob

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

#include "KaiStaticText.h"
#include <wx/dcmemory.h>
#include <wx/dcclient.h>
#include <wx/sizer.h>

KaiStaticText::KaiStaticText(wxWindow *parent, int id, const wxString& _text, const wxPoint &pos, const wxSize &size, int style)
	: wxWindow(parent, id, pos, size, style)
	, text(_text)
	, textColour(WindowText)
	, textHeight(0)
{
	int fullw=0;
	textHeight=0;
	wxSize newSize = size;
	wxArrayString lines = wxStringTokenize(text, "\n",wxTOKEN_RET_EMPTY_ALL);
	for(size_t i=0; i < lines.size(); i++){
		int fw, fh;
		GetTextExtent((lines[i]=="")? "|" : lines[i], &fw, &fh, 0, 0, &GetFont());
		textHeight += fh;
		if(fullw < fw){fullw = fw;}
	}
	if(size.x <1){
		newSize.x = fullw+20;
	}
	if(size.y <1){
		newSize.y = textHeight+2;
		if(textHeight<17){newSize.y=17;}
	}
	SetMinSize(newSize);
	Bind(wxEVT_ERASE_BACKGROUND,[=](wxEraseEvent &evt){});
	Bind(wxEVT_PAINT, &KaiStaticText::OnPaint, this);
}

void KaiStaticText::SetLabelText(const wxString &_text){
	text = _text; 
	int fullw=0;
	textHeight=0;
	wxSize size=GetClientSize();
	wxArrayString lines = wxStringTokenize(text, "\n",wxTOKEN_RET_EMPTY_ALL);
	for(size_t i=0; i < lines.size(); i++){
		int fw, fh;
		GetTextExtent((lines[i]=="")? "|" : lines[i], &fw, &fh, 0, 0, &GetFont());
		textHeight += fh;
		if(fullw < fw){fullw = fw;}
	}
	if(size.x != fullw || size.y != textHeight){
		size.x = fullw;
		size.y = textHeight;
		SetMinSize(size);
		wxSizer *sizer = GetSizer();
		if(sizer){
			sizer->Layout();
		}
		//GetParent()->Layout();
		Refresh(false);
		//Update();
		return;
	}
	
	Refresh(false);
}
	
void KaiStaticText::OnPaint(wxPaintEvent &evt)
{
	int w=0;
	int h=0;
	GetClientSize (&w, &h);
	if(w==0||h==0){return;}
	wxMemoryDC tdc;
	tdc.SelectObject(wxBitmap(w,h));
	tdc.SetFont(GetFont());
	tdc.SetBrush(Options.GetColour(WindowBackground));
	tdc.SetPen(*wxTRANSPARENT_PEN);
	tdc.DrawRectangle(0,0,w,h);
	//wxRect cur(0, 0, w, h);
	tdc.SetTextForeground(Options.GetColour(textColour));
	//tdc.SetClippingRegion(cur);
	//tdc.DrawLabel(text,cur,GetWindowStyle());
	//tdc.DestroyClippingRegion();
	//int fw=0, fh=0;
	//tdc.GetTextExtent(text, &fw, &fh, 0, 0, &GetFont());
	tdc.DrawText(text, 0, (h-textHeight)/2);
	wxPaintDC dc(this);
	dc.Blit(0,0,w,h,&tdc,0,0);
}

