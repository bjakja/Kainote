#include "TabPanel.h"
#include "Visuals.h"
#include <wx/tokenzr.h>
#include <math.h> 




ClipPoint::ClipPoint()
{
	x=0;
	y=0;
	type="m";
	start=true;
}

ClipPoint::ClipPoint(int _x, int _y, wxString _type, bool isstart)
{
	x=_x;
	y=_y;
	type=_type;
	start=isstart;
}
	
bool ClipPoint::IsInPos(wxPoint pos, int diff)
{
	return (abs(pos.x-x) <= diff && abs(pos.y-y) <= diff);
}

D3DXVECTOR2 ClipPoint::GetVector()
{
	D3DXVECTOR2 v((x+_x)/wspw,(y+_y)/wsph);
	return v;
}

int ClipPoint::wx()
{
	return (x+_x)/wspw;
}
int ClipPoint::wy()
{
	return (y+_y)/wsph;
}

//Rysuje clip jeœli dana linia jest widoczna na wideo
//cur - aktualny czas wideo
void Visuals::DrawClip()
{
	
	
	//wxLogStatus("Create line %i %i", Points[0].x, Points[0].y);
	HRESULT hr;
	hr=line->Begin();
	size_t g=1;
	while(g<Points.size()){
		if(Points[g].type=="b"||Points[g].type=="s"){
			g+=DrawCurve(g,(Points[g].type=="s"));
		}else if(Points[g].type=="l"){
			DrawLine(g);
			g++;
		}else{
			DrawRect(g-1);
			g++;}

	}
	
	DrawRect(Points.size()-1);
	
	hr=line->End();

	if(drawtxt){
		coords=""; 
		coords<<(acpoint.x)<<", "<<(acpoint.y);
		wxSize wsize=parent->GetClientSize();
		wsize.x/=2; wsize.y/=2;
		int x=acpoint.wx(), y=acpoint.wy();
		align=0;
		align |= (x>wsize.x)? DT_RIGHT : 0;
		align |= (y>wsize.y)? DT_BOTTOM : 0;
		cpos.left=(x>wsize.x)? x-150 : x+5;
		cpos.top=(y>wsize.y)? y-50 : y+5;
		cpos.right=(x>wsize.x)? x-5 : x+150;
		cpos.bottom=(y>wsize.y)? y-5 : y+50;
		DRAWOUTTEXT(font,coords,cpos,align,0xFFFF0000)
	}
}

	
void Visuals::SetClip(wxString clip, float x, float y)
{
	_x=x;
	_y=y;
	Points.clear();
	wxStringTokenizer tokens(clip," ");
	int tmpx=0;
	bool gotx=false;
	bool start=false;
	wxString type;
	while(tokens.HasMoreTokens()){
		wxString token=tokens.GetNextToken();
		if(token=="p"){token="s";}
		if(token=="m"||token=="l"||token=="b"||token=="s"){type=token;start=true;}
		else if(token=="c"){continue;}
		else if(gotx){
			int tmpy=wxAtoi(token);
			Points.push_back(ClipPoint(tmpx, tmpy,type,start));
			gotx=false;
			start=false;
		}
		else{
			tmpx=wxAtoi(token);
			gotx=true;
		}
	
	}
	//wxLogStatus(clip);
	if(Points.empty()){
		AddMove(wxPoint(((subssize.x)/(wspw*scale.x))/2, ((subssize.y)/(wsph*scale.y))/2),0);
	}
	acpoint=Points[0];
	
}
	
wxString Visuals::GetClip()
{
	wxString clip;
	int cntb=0;
	bool spline=false;
	for(size_t i=0; i<Points.size(); i++)
	{
		ClipPoint pos=Points[i];
		int x= pos.x;
		int y= pos.y;
		if(cntb && !pos.start){
			clip<<x<<" "<<y<<" ";
			cntb++;
			//if(cntb>2 && pos.type=="b"){cntb=0;}
		}else{
			if(spline){clip<<"c ";spline=false;}
			clip<<pos.type<<" "<<x<<" "<<y<<" ";
			if(pos.type=="b"||pos.type=="s"){cntb=1;if(pos.type=="s"){spline=true;}}
		}
	}
	if(spline){clip<<"c ";}
	return clip.Trim();
}

void Visuals::SetPos(int x, int y)
{
	_x= x;
	_y= y;
	//wxLogStatus("x y %i %i", _x, _y);
}

// pos in skreen position
int Visuals::CheckPos(wxPoint pos, bool retlast)
{
	pos.x =(pos.x*wspw)-_x; pos.y =(pos.y*wsph)-_y;
	for(size_t i=0; i<Points.size(); i++)
	{
		if(Points[i].IsInPos(pos,5)){return i;}
	}
	return (retlast)? Points.size() : -1;
}

// pos in clip position
void Visuals::MovePoint(wxPoint pos, int point)
{
	Points[point].x=pos.x;
	Points[point].y=pos.y;
}
// pos in skreen position	
void Visuals::AddCurve(wxPoint pos, int whereis, wxString type)
{
	pos.x =(pos.x*wspw)-_x; pos.y =(pos.y*wsph)-_y;
	wxPoint oldpos;
	if(whereis!=Points.size()){whereis++;}//gdy wstawiamy beziera w œrodku to trzeba mu przesun¹æ punkt o 1,
	//bo stworzy nam krzyw¹ zamiast poprzedniej linii
	oldpos.x=Points[whereis-1].x;
	oldpos.y=Points[whereis-1].y;
	int diffx=(pos.x-oldpos.x)/3.0f;
	int diffy=(pos.y-oldpos.y)/3.0f;
	Points.insert(Points.begin()+whereis, ClipPoint(pos.x-(diffx*2),pos.y-(diffy*2),type, true));
	Points.insert(Points.begin()+whereis+1, ClipPoint(pos.x-diffx,pos.y-diffy,type,false));
	Points.insert(Points.begin()+whereis+2, ClipPoint(pos.x,pos.y,type,false));
	acpoint=Points[whereis+2];
}
// pos in skreen position
void Visuals::AddCurvePoint(wxPoint pos, int whereis)
{
	if(Points[whereis-1].type=="s"||((int)Points.size()>whereis && Points[whereis].type=="s"))
	{
		Points.insert(Points.begin()+whereis, ClipPoint((pos.x*wspw)-_x, (pos.y*wsph)-_y,"s",false));
	}
	else{wxBell();}
}
// pos in skreen position	
void Visuals::AddLine(wxPoint pos, int whereis)
{
	Points.insert(Points.begin()+whereis, ClipPoint((pos.x*wspw)-_x, (pos.y*wsph)-_y,"l",true));
	acpoint=Points[whereis];
}
// pos in skreen position	
void Visuals::AddMove(wxPoint pos, int whereis)
{
	Points.insert(Points.begin()+whereis, ClipPoint((pos.x*wspw)-_x, (pos.y*wsph)-_y,"m",true));
	acpoint=Points[whereis];
}
	
void Visuals::DrawLine(int i)
{
	int diff = (Points[i-1].type=="s")? 2 : 1;
	D3DXVECTOR2 v2[2]={Points[i-diff].GetVector(),Points[i].GetVector()};
	line->Draw(v2, 2, 0xFFFF0000);
	DrawRect(i-1);
	/*MYVERTEX v5[3];
	CreateMYVERTEX(&v5[0], 0, 0, 0xAAFF0000);
	CreateMYVERTEX(&v5[1], v2[0].x, v2[0].y, 0xAAFF0000);
	CreateMYVERTEX(&v5[2], v2[1].x, v2[1].y, 0xAAFF0000);
	HRN(device->SetFVF( D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1), "fvf failed");
	HRN(device->SetTexture( 0, texture ),"texture failed");
    HRN(device->DrawPrimitiveUP( D3DPT_TRIANGLELIST, 1, v5, sizeof(MYVERTEX) ),"primitive failed");*/
	
}
	
int Visuals::DrawCurve(int i, bool bspline)
{
	std::vector<D3DXVECTOR2> v4;
	
	int pts=3;
	ClipPoint tmp(0,0,"r",true);
	if(Points[i-1].type=="s"){tmp=Points[i-1];Points[i-1]=Points[i-2];}
	if(bspline){
		
		int acpos=i-1;
		int bssize=1;
		int spos=i+1;
		while(spos<(int)Points.size()){
			if(Points[spos].start){break;}
			bssize++;spos++;
		}
		pts=bssize;
		bssize++;
		for(int k=0; k<bssize; k++){
			Curve(acpos, &v4, true, bssize, k);
		}
		D3DXVECTOR2 *v2=new D3DXVECTOR2[pts+2];
		if(tmp.type=="s"){Points.insert(Points.begin()+i-1,tmp);}
		for(int j=0, g=i-1; j<bssize; j++, g++)
		{
			v2[j]=Points[g].GetVector();
		}
		v2[bssize]=Points[i-1].GetVector();
		line->Draw(v2, pts+2, 0xFFAA33AA);
		delete[] v2;
		
	}else{
		Curve(i-1, &v4,false);
		if(tmp.type=="s"){Points[i-1]=tmp;}
		D3DXVECTOR2 v2[4]={Points[i-1].GetVector(),Points[i].GetVector(),Points[i+1].GetVector(),Points[i+2].GetVector()};
		line->Draw(v2, 2, 0xFF0000FF);
		line->Draw(&v2[2], 2, 0xFF0000FF);
	}
	line->Draw(&v4[0], v4.size(), 0xFFFF0000);

	DrawRect(i-1);
	for(int j=1; j<pts; j++){DrawCircle(i+j-1);}
	return pts;
}

void Visuals::DrawRect(int i)
{
    D3DXVECTOR2 tmp = Points[i].GetVector();
	D3DXVECTOR2 v2[2];
	line->End();
	line->SetWidth(10.0f);
	line->Begin();
	v2[0].x=tmp.x-5;
	v2[0].y=tmp.y;
	v2[1].x=tmp.x+5;
	v2[1].y=tmp.y;
	
	line->Draw(&v2[0], 2, 0xAAFF0000);
	line->End();
	line->SetWidth(2.0f);
	line->Begin();
}
	
void Visuals::DrawCircle(int i)
{
	line->End();
	D3DXVECTOR2 tmp = Points[i].GetVector();
	MYVERTEX v5[41];
	float rad =0.01745329251994329576923690768489f;
	
	float xx = tmp.x;
	float yy = tmp.y;
	CreateMYVERTEX(&v5[0], xx, yy, 0x4CFFA928);
	for(int j=0; j<20; j++)
	{
		float xx1= tmp.x + (6.f * sin ( (j*20) * rad ));
		float yy1= tmp.y + (6.f * cos ( (j*20) * rad ));
		CreateMYVERTEX(&v5[j+1], xx1, yy1, 0x4CFFA928);
		CreateMYVERTEX(&v5[j+21], xx1, yy1, 0xAAFF0000);
		xx=xx1;
		yy=yy1;
		
	}
	
	
	
	HRN(device->SetFVF( D3DFVF_XYZ|D3DFVF_DIFFUSE), "fvf failed");
	HRN(device->DrawPrimitiveUP( D3DPT_TRIANGLEFAN, 18, v5, sizeof(MYVERTEX) ),"primitive failed");
	HRN(device->DrawPrimitiveUP( D3DPT_LINESTRIP, 18, &v5[21], sizeof(MYVERTEX) ),"primitive failed");
	line->Begin();
}

	
void Visuals::Curve(int pos, std::vector<D3DXVECTOR2> *table, bool bspline, int spoints, int acpt)
{
	float a[4], b[4];
	float x[4], y[4];
	for(int g=0; g<4; g++)
	{
		if(acpt>(spoints-1)){acpt=0;}
		//if(g==0 && Points[pos].type=="s" ){acpt--;}
		x[g]=(Points[pos+acpt].x+_x)/wspw;
		y[g]=(Points[pos+acpt].y+_y)/wsph;
		//if(g==0 && Points[pos].type=="s" ){acpt++;}
		acpt++;
	}
	
	if(bspline){
		a[3] = (-x[0] + 3 * x[1] - 3 * x[2] + x[3]) / 6.0;
		a[2] = (3 * x[0] - 6 * x[1] + 3 * x[2]) / 6.0;
		a[1] = (-3 * x[0] + 3 * x[2]) / 6.0;
		a[0] = (x[0] + 4 * x[1] + x[2]) / 6.0;
		b[3] = (-y[0] + 3 * y[1] - 3 * y[2] + y[3]) / 6.0;
		b[2] = (3 * y[0] - 6 * y[1] + 3 * y[2]) / 6.0;
		b[1] = (-3 * y[0] + 3 * y[2]) / 6.0;
		b[0] = (y[0] + 4 * y[1] + y[2]) / 6.0;
	}else{
		a[3] = -  x[0]+3*x[1]-3*x[2]+x[3];
		a[2] =  3*x[0]-6*x[1]+3*x[2];
		a[1] = -3*x[0]+3*x[1];
		a[0] =    x[0];
		b[3] = -  y[0]+3*y[1]-3*y[2]+y[3];
		b[2] =  3*y[0]-6*y[1]+3*y[2];
		b[1] = -3*y[0]+3*y[1];
		b[0] =    y[0];
	}

	float maxaccel1 = fabs(2*b[2]) + fabs(6*b[3]);
    float maxaccel2 = fabs(2*a[2]) + fabs(6*a[3]);
    float maxaccel = maxaccel1 > maxaccel2 ? maxaccel1 : maxaccel2;
    float h = 1.0f;
    if(maxaccel > 4.0f) h = sqrt(4.0f / maxaccel);
	float p_x, p_y;
	for(float t = 0; t < 1.0; t += h)
    {
		p_x = a[0] + t*(a[1] + t*(a[2] + t*a[3]));
        p_y = b[0] + t*(b[1] + t*(b[2] + t*b[3]));
		table->push_back(D3DXVECTOR2(p_x,p_y));
	}
	p_x = a[0] + a[1] + a[2] + a[3];
    p_y = b[0] + b[1] + b[2] + b[3];
	table->push_back(D3DXVECTOR2(p_x,p_y));
}

void Visuals::OnMouseEvent(wxMouseEvent &event)
{
	
	int x=event.GetX();
	int y=event.GetY();
	wxPoint xy=event.GetPosition();
	bool click=event.LeftDown();
	bool right=event.RightDown();
	bool ctrl=event.ControlDown();
	//bool click=event.LeftDown();
	drawtxt=(event.MiddleUp())?false : true;

	if(!event.ButtonDown() && !event.LeftIsDown()){
		//wxLogStatus(" bdown %i", (int)event.ButtonDown());
		int pos = CheckPos(xy);
		if(pos!= -1 && hasArrow){
			parent->SetCursor(wxCURSOR_SIZING);hasArrow=false;
		}else if(pos== -1 && !hasArrow){parent->SetCursor(wxCURSOR_ARROW);hasArrow=true;}
	}
	
	if(event.ButtonUp()){
		TabPanel* pan=(TabPanel*)parent->GetParent();
		pan->Edit->SetClip(GetClip(),false);
	}

	if(event.LeftUp()){
		
		if(parent->HasCapture()){
			parent->ReleaseMouse();}
		return;
	}
	if(right&&ctrl&&event.AltDown())
	{
		//wxLogStatus("point");
		if(Points.empty()){wxBell(); return;}
		AddCurvePoint(xy,CheckPos(xy, true));
		TabPanel* pan=(TabPanel*)parent->GetParent();
		pan->Edit->SetClip(GetClip(),true);
		return;
	}
	if(right&&ctrl)
	{
		//wxLogStatus("curve");
		if(Points.empty()){wxBell(); return;}
		AddCurve(xy,CheckPos(xy, true),"s");
		TabPanel* pan=(TabPanel*)parent->GetParent();
		pan->Edit->SetClip(GetClip(),true);
		return;
	}
	if(right)
	{
		if(Points.empty()){wxBell(); return;}
		AddLine(xy,CheckPos(xy, true));
		TabPanel* pan=(TabPanel*)parent->GetParent();
		pan->Edit->SetClip(GetClip(),true);
		return;
	}
	if(ctrl&&event.AltDown()&&click)
	{
		if(Points.empty()){wxBell(); return;}
		AddCurve(xy,CheckPos(xy, true));
		TabPanel* pan=(TabPanel*)parent->GetParent();
		pan->Edit->SetClip(GetClip(),true);
		return;
	}
	else if(ctrl&&click)
	{
		newmove=true;
	}
	if(event.MiddleDown()){
		for(size_t i=1; i<Points.size(); i++)
		{
			float pointx=(Points[i].x+_x)/wspw, pointy=(Points[i].y+_y)/wsph;
			if(abs(pointx-x)<5 && abs(pointy-y)<5)
			{
				if(Points[i].start || Points[i].type=="s"){
					int er=(Points[i].type=="b")? 3 : 1;
					Points.erase(Points.begin()+i,Points.begin()+i+er);
				}
				else{
					for(int j=i-1; j>=0; j--)
					{
						if(Points[j].start){
							int er=(Points[j].type=="b")? 3 : 1;
							Points.erase(Points.begin()+j,Points.begin()+j+er);
							break;
						}
					}
				}
				drawtxt=false;
				TabPanel* pan=(TabPanel*)parent->GetParent();
				pan->Edit->SetClip(GetClip(),true);
				break;
			}
		}
		return;
	}

	

	if(click)
	{

		grabbed=-1;
		for(size_t i=0; i<Points.size(); i++)
		{
			float pointx=(Points[i].x+_x)/wspw, pointy=(Points[i].y+_y)/wsph;
			if(abs(pointx-x)<5 && abs(pointy-y)<5)
			{
				acpoint=Points[i];
				grabbed=i;
				diffs.x=pointx-x;
				diffs.y=pointy-y;
				parent->CaptureMouse();
				break;
			}
		}
		if(newmove && grabbed==-1)
		{
			
			AddMove(xy,CheckPos(xy, true));newmove=false;
			TabPanel* pan=(TabPanel*)parent->GetParent();
			pan->Edit->SetClip(GetClip(),true);
			grabbed=Points.size()-1;
		}

	}

	if(event.LeftIsDown() && grabbed!=-1)
	{
		
		wxSize wsize=parent->GetClientSize();
		
		x=MID(0,x,wsize.x);
		y=MID(0,y,wsize.y-44);
		Points[grabbed].x=((x+diffs.x)*wspw)-_x;
		Points[grabbed].y=((y+diffs.y)*wsph)-_y;
		acpoint=Points[grabbed];
		TabPanel* pan=(TabPanel*)parent->GetParent();
		pan->Edit->SetClip(GetClip(),true);

	}
	

}

void Visuals::SetNewSize(wxSize wsize, LPD3DXLINE _line, LPD3DXFONT _font, LPDIRECT3DDEVICE9 _device)
{
	line=_line;
	font=_font;
	device=_device;
	wspw=((float)subssize.x/(float)wsize.x);
	wsph=(float)subssize.y/(float)(wsize.y-44);
	//wxLogStatus(" wsp %f %f %i %i", wspw, wsph ,subssize.x, wsize.x );
	if(Visual==VECTORDRAW)
	{
		wspw/=scale.x;
		wsph/=scale.y;
	}

}

D3DXVECTOR2 Visuals::CalcWH()
{
	if (Points.size()<1){return D3DXVECTOR2(0,0);}
	int minx=INT_MAX;
	int miny=INT_MAX;
	int maxx=-INT_MAX;
	int maxy=-INT_MAX;
	for(size_t i = 0; i<Points.size(); i++)
	{
		ClipPoint p=Points[i];
		if(p.x<minx){minx=p.x;}
		if(p.y<miny){miny=p.y;}
		if(p.x>maxx){maxx=p.x;}
		if(p.y>maxy){maxy=p.y;}
	}
	D3DXVECTOR2 sizes((maxx-minx)*wspw,(maxy-miny)*wsph);
	return sizes;
}

