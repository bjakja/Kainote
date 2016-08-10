

#include "Visuals.h"
#include "TabPanel.h"
#include <wx/regex.h>


Visuals *Visuals::Get(int Visual, wxWindow *_parent)
{
	Visuals *visual;
	switch(Visual)
	{
	case CHANGEPOS:
		visual=new Position();
		break;
	case MOVE:
		visual=new Move();
		break;
	case MOVEALL:
		visual=new MoveAll();
		break;
	case SCALE:
		visual=new Scale();
		break;
	case ROTATEZ:
		visual=new RotationZ();
		break;
	case ROTATEXY:
		visual=new RotationXY();
		break;
	case CLIPRECT:
		visual=new ClipRect();
		break;
	case VECTORCLIP:
	case VECTORDRAW:
		visual=new DrawingAndClip();
		break;
	default:
		visual=new Position();
		break;
	}
	
	visual->tab = (TabPanel*)_parent->GetParent();
	visual->Visual= Visual;
	return visual;
}


float Visuals::wspw=0;
float Visuals::wsph=0;

Visuals::Visuals()
{
	from = lastmove = to = D3DXVECTOR2(0,0);
	line=0;
	font=0;
	device=0;
	start=end=oldtime=0;
	blockevents=false;
}
	
Visuals::~Visuals()
{
}
	
void Visuals::SetVisual(int _start,int _end)
{
	int nx=0, ny=0;
	tab->Grid1->GetASSRes(&nx, &ny);
	SubsSize=wxSize(nx,ny);
	start=_start;
	end=_end;
	
	wspw=((float)SubsSize.x/(float)VideoSize.x);
	wsph=((float)SubsSize.y/(float)VideoSize.y);
	tab->Video->VisEdit=true;
	
	SetCurVisual();
	if(Visual==MOVEALL){tab->Video->Render(); return;}
	if(Visual==VECTORCLIP||Visual==VECTORDRAW){
		tab->Edit->SetClip(GetVisual(),true); return;
	}
	tab->Edit->SetVisual(GetVisual(),true,0);
}

void Visuals::SizeChanged(wxSize wsize, LPD3DXLINE _line, LPD3DXFONT _font, LPDIRECT3DDEVICE9 _device)
{
	line=_line;
	font=_font;
	device=_device;
	VideoSize=wsize;
	wspw=((float)SubsSize.x/(float)wsize.x);
	wsph=((float)SubsSize.y/(float)wsize.y);

	HRN(device->SetFVF( D3DFVF_XYZ|D3DFVF_DIFFUSE), "fvf failed");
}

//drawarrow od razu przesuwa punkt tak by linia kończyła się przed strzałką
//from i to są koordynatami linii
//diff można przesunąć strzałkę w tył od punktu "to"
void Visuals::DrawArrow(D3DXVECTOR2 from, D3DXVECTOR2 *to, int diff)
{
	D3DXVECTOR2 pdiff=from- (*to);
	float len= sqrt((pdiff.x * pdiff.x) + (pdiff.y*pdiff.y));
	D3DXVECTOR2 diffUnits = (len==0)? D3DXVECTOR2(0,0) : pdiff / len;
	// długość może przyjmnować wartości ujemne, dlatego dajemy + strzałka nie była odwrotnie
	D3DXVECTOR2 pend=(*to) + (diffUnits * (12+diff));
	D3DXVECTOR2 halfbase = D3DXVECTOR2(-diffUnits.y, diffUnits.x) * 5.f;
	
	VERTEX v4[7];
	D3DXVECTOR2 v3[3];
	v3[0]= pend - diffUnits * 12;
	v3[1]= pend + halfbase;
	v3[2]= pend - halfbase;

	CreateVERTEX(&v4[0],v3[0].x,v3[0].y,0xAA121150);
	CreateVERTEX(&v4[1],v3[1].x,v3[1].y,0xAA121150);
	CreateVERTEX(&v4[2],v3[2].x,v3[2].y,0xAA121150);
	CreateVERTEX(&v4[3],v3[0].x,v3[0].y,0xFFFF0000);
	CreateVERTEX(&v4[4],v3[1].x,v3[1].y,0xFFFF0000);
	CreateVERTEX(&v4[5],v3[2].x,v3[2].y,0xFFFF0000);
	CreateVERTEX(&v4[6],v3[0].x,v3[0].y,0xFFFF0000);
	
	HRN(device->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 1, v4, sizeof(VERTEX) ),"primitive failed");
	HRN(device->DrawPrimitiveUP( D3DPT_LINESTRIP, 3, &v4[3], sizeof(VERTEX) ),"primitive failed");
	*to = pend;
}

void Visuals::DrawCross(D3DXVECTOR2 position, D3DCOLOR color, bool useBegin)
{
	D3DXVECTOR2 cross[4];
	cross[0].x = position.x-15.0f;
	cross[0].y = position.y;
	cross[1].x = position.x+15.0f;
	cross[1].y = position.y;
	cross[2].x = position.x;
	cross[2].y = position.y-15.0f;
	cross[3].x = position.x;
	cross[3].y = position.y+15.0f;
	if(useBegin){line->Begin();}
	line->Draw(cross,2,color);
	line->Draw(&cross[2],2,color);
	if(useBegin){line->End();}

}

void Visuals::DrawRect(D3DXVECTOR2 pos)
{
	//line->End();
    
	VERTEX v9[9];
	CreateVERTEX(&v9[0], pos.x-5.0f, pos.y-5.0f, 0xAA121150);
	CreateVERTEX(&v9[1], pos.x+5.0f, pos.y-5.0f, 0xAA121150);
	CreateVERTEX(&v9[2], pos.x-5.0f, pos.y+5.0f, 0xAA121150);
	CreateVERTEX(&v9[3], pos.x+5.0f, pos.y+5.0f, 0xAA121150);
	CreateVERTEX(&v9[4], pos.x-5.0f, pos.y-5.0f, 0xFFFF0000);
	CreateVERTEX(&v9[5], pos.x+5.0f, pos.y-5.0f, 0xFFFF0000);
	CreateVERTEX(&v9[6], pos.x+5.0f, pos.y+5.0f, 0xFFFF0000);
	CreateVERTEX(&v9[7], pos.x-5.0f, pos.y+5.0f, 0xFFFF0000);
	CreateVERTEX(&v9[8], pos.x-5.0f, pos.y-5.0f, 0xFFFF0000);

	HRN(device->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, v9, sizeof(VERTEX) ),"primitive failed");
	HRN(device->DrawPrimitiveUP( D3DPT_LINESTRIP, 4, &v9[4], sizeof(VERTEX) ),"primitive failed");
	//line->Begin();
}
	
void Visuals::DrawCircle(D3DXVECTOR2 pos)
{
	//line->End();
	
	VERTEX v5[41];
	float rad =0.01745329251994329576923690768489f;
	
	float xx = pos.x;
	float yy = pos.y;
	CreateVERTEX(&v5[0], xx, yy, 0xAA121150);
	for(int j=0; j<20; j++)
	{
		float xx1= pos.x + (6.f * sin ( (j*20) * rad ));
		float yy1= pos.y + (6.f * cos ( (j*20) * rad ));
		CreateVERTEX(&v5[j+1], xx1, yy1, 0xAA121150);
		CreateVERTEX(&v5[j+21], xx1, yy1, 0xFFFF0000);
		xx=xx1;
		yy=yy1;
		
	}

	HRN(device->DrawPrimitiveUP( D3DPT_TRIANGLEFAN, 18, v5, sizeof(VERTEX) ),"primitive failed");
	HRN(device->DrawPrimitiveUP( D3DPT_LINESTRIP, 18, &v5[21], sizeof(VERTEX) ),"primitive failed");
	//line->Begin();
}

	
void Visuals::Draw(int time)
{
	if(!(time>=start && time<=end)){blockevents = true; return;}else if(blockevents){blockevents=false;}
	wxMutexLocker lock(clipmutex);
	line->SetAntialias(TRUE);
	line->SetWidth(2.0);
	
	DrawVisual(time);
	
	line->SetAntialias(FALSE);
	oldtime=time;
}



D3DXVECTOR2 Visuals::CalcMovePos()
{
	D3DXVECTOR2 ppos;
	int time= tab->Video->Tell();
	if(tbl[6]<6){tbl[4]=start; tbl[5]=end;}
	float tmpt= time - tbl[4];
	float tmpt1=tbl[5] - tbl[4];
	float actime= tmpt/tmpt1;
	float distx, disty;
	if(time < tbl[4]){distx=tbl[0], disty= tbl[1];}
	else if(time > tbl[5]){distx=tbl[2], disty= tbl[3];}
	else {
		distx= tbl[0] -((tbl[0]-tbl[2])*actime); 
		disty = tbl[1] -((tbl[1]-tbl[3])*actime);
	}
	ppos.x=distx, ppos.y=disty;
	return ppos;
}

//BEGIN_EVENT_TABLE(Visuals, wxEvtHandler)
//	EVT_MOUSE_EVENTS(Visuals::OnMouseEvent)
//END_EVENT_TABLE()