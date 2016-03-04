

#include "Visuals.h"
#include "TabPanel.h"
#include <wx/regex.h>


Visuals::Visuals(wxWindow *_parent)
{
	from = lastmove = to = D3DXVECTOR2(0,0);
	type=0;
	line=0;
	font=0;
	grabbed=-1;
	start=end=oldtime=0;
	newline=newmove=blockevents=false;
	drawtxt=true;
	parent=_parent;
	subssize.x=-1;
	invClip=false;
	TabPanel* pan=(TabPanel*)parent->GetParent();
	Visual=pan->Edit->Visual;
	int nx=0, ny=0;
	pan->Grid1->GetASSRes(&nx, &ny);
	subssize=wxSize(nx,ny);
}
	
Visuals::~Visuals()
{
	Points.clear();
}
	
void Visuals::SetVisual(int _start,int _end)
{
	TabPanel* pan=(TabPanel*)parent->GetParent();
	Visual=pan->Edit->Visual;
	int nx=0, ny=0;
	pan->Grid1->GetASSRes(&nx, &ny);
	subssize=wxSize(nx,ny);
	start=_start;
	end=_end;
	D3DXVECTOR2 linepos = pan->Edit->GetPosnScale(&scale, &AN, tbl, &times);

	
	wxString vis;
	if(Visual<CLIPRECT){
		if(tbl[6]>3){linepos=CalcMovePos();}
		from = to = D3DXVECTOR2(linepos.x/wspw,linepos.y/wsph);
		lastmove = D3DXVECTOR2(0, 0);
	}else{

		if(Visual!=VECTORDRAW){
			bool found =pan->Edit->FindVal("(i?clip[^\\)]+)", &vis);
			if(found){int rres = vis.Replace(",",",");
				if( rres == 3 && Visual==VECTORCLIP) {vis = "";}
				else if( rres !=3 && Visual==CLIPRECT) {vis = "";} 
			}
		}else{
			wxString txt=pan->Edit->TextEdit->GetValue();
			wxRegEx re("}(m[^{]*){\\\\p0}", wxRE_ADVANCED);
			if(re.Matches(txt)){
				vis =re.GetMatch(txt,1);
			}
		}

	}
	if(Visual>=VECTORCLIP){

		if(Visual==VECTORDRAW){

			_x=linepos.x/scale.x;
			_y=(linepos.y/scale.y);
			wspw=((float)subssize.x/(float)widsize.x)/scale.x;
			wsph=((float)subssize.y/(float)widsize.y)/scale.y;
		}else{
			_x=0;
			_y=0;
			vis=vis.AfterFirst('(');
		}

		SetClip(vis);
	}else if(Visual==CLIPRECT){
		int x1=0,x2=subssize.x,y1=0,y2=subssize.y;
		wxString rest;
		Points.clear();
		if(vis!=""){
			if(vis.StartsWith("i")){invClip=true;}else{invClip=false;}
			vis.BeforeFirst('(',&vis);
			x1=wxAtoi(vis.BeforeFirst(',',&rest));
			wxString y11=rest;//niestety używając rest do tej samej operacji coś zwraca późniejsze liczby:/
			y1=wxAtoi(y11.BeforeFirst(',',&rest));
			wxString x22=rest;
			x2=wxAtoi(x22.BeforeFirst(',',&rest));
			y2=wxAtoi(rest);
		}
		Points.push_back(ClipPoint(x1, y1,"r",true));
		Points.push_back(ClipPoint(x2, y2,"r",true));
	}else if(Visual==MOVE){

		if(tbl[6]>3){to.x=tbl[2]/wspw, to.y=tbl[3]/wsph;}
		if(tbl[6]>5){
			moveStart=(int)tbl[4];
			if(tbl[4]>tbl[5]){tbl[5]=end;}
			moveEnd=(int)tbl[5];
		}else{moveStart=start; moveEnd=end;}

	}else if(Visual==SCALE){

		int addy=(AN>3)?60 : -60, addx= (AN % 3 == 0)?-60 : 60;
		to.x=from.x+(scale.x*addx);
		to.y=from.y+(scale.y*addy);

	}else if(Visual==ROTATEZ){
		
		wxString res;
		if(pan->Edit->FindVal("frz?([^\\\\}]+)", &res)){
			double result=0; res.ToDouble(&result);
			lastmove.y=result;
			lastmove.x+=lastmove.y;
		}
		if(pan->Edit->FindVal("org\\(([^\\)]+)", &res)){
			wxString rest;
			double orx,ory;
			if(res.BeforeFirst(',',&rest).ToDouble(&orx)){org.x=orx/wspw;}
			if(rest.ToDouble(&ory)){org.y=ory/wspw;}
		//wxLogStatus("%f %f", orx,ory);
		}else{org=from;}
		to=org;

	}
	else if(Visual==ROTATEXY){

		wxString res;
		scale=D3DXVECTOR2(0,0);//skala robi tu za przechowywanie wcześniejszych wartości by nie dawać dodatkowych zmiennych.
		if(pan->Edit->FindVal("frx([^\\\\}]+)", &res)){
			double result=0; res.ToDouble(&result);
			scale.y= result;
		//wxLogStatus("%f ", scale.y);
		}
		if(pan->Edit->FindVal("fry([^\\\\}]+)", &res)){
			//wxLogStatus("fry "+res);
			double result=0; res.ToDouble(&result);
			scale.x= result;
		//wxLogStatus("%f ", scale.x);
		}
		if(pan->Edit->FindVal("org\\(([^\\)]+)", &res)){
			wxString rest;
			double orx,ory;
			if(res.BeforeFirst(',',&rest).ToDouble(&orx)){org.x=orx/wspw;}
			if(rest.ToDouble(&ory)){org.y=ory/wsph;}
		}else{org=from;}
		//wxLogStatus("org %f %f %f %f", org.x, org.y, from.x, from.y);
		firstmove=to;
		angle=scale;
		lastmove=org;

	}
	/*else if(Visual==MOVEONCURVE){
		TabPanel* pan=(TabPanel*)parent->GetParent();
		curvelines=(end-start)/pan->Video->avtpf;
	}*/
	pan->Video->VisEdit=true;
	if(pan->Edit->Visual>=VECTORCLIP){
		pan->Edit->SetClip(GetClip(),true);
	}else{
		pan->Edit->SetVisual(GetVisual(),true,0);
	}
}

void Visuals::SizeChanged(wxSize wsize, LPD3DXLINE _line, LPD3DXFONT _font, LPDIRECT3DDEVICE9 _device)
{
	line=_line;
	font=_font;
	device=_device;
	widsize=wsize;
	wspw=((float)subssize.x/(float)wsize.x);
	wsph=((float)subssize.y/(float)wsize.y);

	HRN(device->SetFVF( D3DFVF_XYZ|D3DFVF_DIFFUSE), "fvf failed");
}
//drawarrow od razu przesuwa punkt tak by linia kończyła się przed strzałką
void Visuals::DrawArrow(D3DXVECTOR2 from, D3DXVECTOR2 *to, int diff)
{
	D3DXVECTOR2 pdiff=from- (*to);
	float len= sqrt((pdiff.x * pdiff.x) + (pdiff.y*pdiff.y));
	D3DXVECTOR2 diffUnits = (len==0)? D3DXVECTOR2(0,0) : pdiff / len;
	// długość może przyjmnować wartości ujemne, dlatego dajemy + strzałka nie była odwrotnie
	D3DXVECTOR2 pend=(*to) + (diffUnits * (12+diff));
	D3DXVECTOR2 halfbase = D3DXVECTOR2(-diffUnits.y, diffUnits.x) * 5.f;
	
	MYVERTEX v4[7];
	D3DXVECTOR2 v3[3];
	v3[0]= pend - diffUnits * 12;
	v3[1]= pend + halfbase;
	v3[2]= pend - halfbase;

	CreateMYVERTEX(&v4[0],v3[0].x,v3[0].y,0xAA121150);
	CreateMYVERTEX(&v4[1],v3[1].x,v3[1].y,0xAA121150);
	CreateMYVERTEX(&v4[2],v3[2].x,v3[2].y,0xAA121150);
	CreateMYVERTEX(&v4[3],v3[0].x,v3[0].y,0xFFFF0000);
	CreateMYVERTEX(&v4[4],v3[1].x,v3[1].y,0xFFFF0000);
	CreateMYVERTEX(&v4[5],v3[2].x,v3[2].y,0xFFFF0000);
	CreateMYVERTEX(&v4[6],v3[0].x,v3[0].y,0xFFFF0000);
	
	HRN(device->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 1, v4, sizeof(MYVERTEX) ),"primitive failed");
	HRN(device->DrawPrimitiveUP( D3DPT_LINESTRIP, 3, &v4[3], sizeof(MYVERTEX) ),"primitive failed");
	*to = pend;
}

void Visuals::Position()
{
	D3DXVECTOR2 v4[4];
	v4[0].x=from.x-15.0f;
	v4[0].y=from.y;
	v4[1].x=from.x+15.0f;
	v4[1].y=from.y;
	v4[2].x=from.x;
	v4[2].y=from.y-15.0f;
	v4[3].x=from.x;
	v4[3].y=from.y+15.0f;
	line->Begin();
	line->Draw(v4,2,0xFFFF0000);
	line->Draw(&v4[2],2,0xFFFF0000);
	line->End();
	DrawRect(0, &from);
}

void Visuals::Move(int time)
{
	D3DXVECTOR2 v4[6];
	v4[0].x=from.x;
	v4[0].y=from.y;
	v4[1].x=to.x;
	v4[1].y=to.y;
	//drawarrow od razu przesuwa punkt tak by linia kończyła się przed strzałką
	DrawArrow(from, &v4[1], 6);

	
	float tmpt=time-moveStart;
	float tmpt1=moveEnd-moveStart;
	float actime= tmpt/tmpt1;
	float distx, disty;
	if(time < moveStart){distx=from.x, disty= from.y;}
	else if(time > moveEnd){distx=to.x, disty= to.y;}
	else {
		distx= from.x -((from.x-to.x)*actime); 
		disty = from.y -((from.y-to.y)*actime);
	}
	//wxLogStatus(" times %i %i %i %i %i, %f %f", moveStart, moveEnd, start, end, time, distx, disty);
	//dokończ to i dorób linię do obracania.
	v4[2].x=distx-15.0f;
	v4[2].y=disty;
	v4[3].x=distx+15.0f;
	v4[3].y=disty;
	v4[4].x=distx;
	v4[4].y=disty-15.0f;
	v4[5].x=distx;
	v4[5].y=disty+15.0f;
	
	line->Begin();
	line->Draw(v4,2,0xFFFF0000);
	line->Draw(&v4[2],2,0xFFFF0000);
	line->Draw(&v4[4],2,0xFFFF0000);
	line->End();

	DrawRect(0, &from);
	DrawCircle(0, &to);
	
	
}

//int factk(int n)
//{
//	int k=1;
//	if(n>1){
//		for(int i=2; i<=n; i++){
//			k*=i;
//		}
//	}
//	return k;
//}
	
//float bernstein(int i, int n, float t)
//{
//	return (factk(n) / (factk(i)*factk(n-i))) * pow(t, i) * pow((1-t), (n-i));
//}
//	
//D3DXVECTOR2 Bezier(int n, ClipPoint *points, float t)
//{
//
//	float p_x=0, p_y=0;
//	//wxLogStatus("points %f %f %f %f", points[2].x, points[2].y, points[3].x, points[3].y);
//	for(int i=0; i<n; i++)
//	{
//		float bern=bernstein(i,n-1,t);
//		p_x += ((points[i].x)*bern);
//		p_y += ((points[i].y)*bern);
//	}
//	if(t>=1){p_x= points[n-1].x; p_y=points[n-1].y;}
//	return D3DXVECTOR2(p_x,p_y);
//}
//
//void Visuals::MoveOnCurve(int time)
//{
//	int numBezier=Points.size();
//	if(Points.size()>3){
//		float curvestep= 1.f/curvelines;
//		float tmpt=time-start;
//		float tmpt1=end-start;
//		float actime= tmpt/tmpt1;
//		float acpos=actime;
//		//wxLogStatus("acpos %f", acpos);
//		
//		D3DXVECTOR2 *v5 = new D3DXVECTOR2[curvelines+1];
//		D3DXVECTOR2 v6[4];
//		bool cross=false;
//		float f=0.f;
//		for(int g=0; g<=curvelines; g++){
//			v5[g]=Bezier(numBezier,&Points[0],f);
//			if(f >= acpos && !cross){
//				//wxLogStatus("acpos %f %f", acpos, f);
//				v6[0].x=v5[g].x-15.0f;
//				v6[0].y=v5[g].y;
//				v6[1].x=v5[g].x+15.0f;
//				v6[1].y=v5[g].y;
//				v6[2].x=v5[g].x;
//				v6[2].y=v5[g].y-15.0f;
//				v6[3].x=v5[g].x;
//				v6[3].y=v5[g].y+15.0f;
//				cross=true;
//			}
//			f += curvestep;
//		}
//		
//		D3DXVECTOR2 *v1 = new D3DXVECTOR2[numBezier];
//		for(int p=0; p<numBezier; p++){
//			v1[p]=D3DXVECTOR2(Points[p].x, Points[p].y);
//		}
//
//		line->Begin();
//		line->Draw(v5, curvelines+1, 0xFFFF0000);
//		line->Draw(v1, numBezier, 0xFF0000FF);
//
//		DrawRect(0,false);
//		DrawRect(numBezier-1,false);
//		for(int j=1; j<numBezier-1; j++){
//			DrawCircle(j,false);
//		}
//		if(cross){
//			line->Draw(v6, 2, 0xFFFF0000);
//			line->Draw(&v6[2], 2, 0xFFFF0000);
//		}
//		line->End();
//		delete [] v5;
//		delete [] v1;
//	}else{
//		D3DXVECTOR2 v4[2];
//		v4[0].x=from.x-5.0f;
//		v4[0].y=from.y;
//		v4[1].x=from.x+5.0f;
//		v4[1].y=from.y;
//		line->SetWidth(10.f);
//		line->Begin();
//		line->Draw(v4, 2, 0xFFFF0000);
//		line->End();
//	}
//	
//}
	
void Visuals::Scale()
{
	D3DXVECTOR2 v4[15];
	int addy=(AN>3)?60 : -60, addx= (AN % 3 == 0)?-60 : 60;
	
	float movex=from.x+addx, movey=from.y+addy;
	
	if(type!=1){movex=to.x;}//strzałka w poziomie i część strzałki po skosie
	else{movex=from.x+(scale.x*addx);}
	if(type>0){movey=to.y;}//strzałka w pionie i część strzałki po skosie
	else{movey=from.y+(scale.y*addy);}
	if(movex==from.x){movex=from.x+addx;}
	else if(movey==from.y){movey=from.y+addy;}
	
	lastmove.x = movex;
	lastmove.y = movey;
	v4[0]=from;//strzałka pozioma
	v4[1].x=movex;
	v4[1].y=from.y;//strzałka pozioma
	v4[2]=from;//strzałka skośna
	v4[3].x=movex;
	v4[3].y=movey;//strzałka skośna
	v4[4]=from;//strzałka pionowa
	v4[5].x=from.x;
	v4[5].y=movey;//strzałka pionowa
	
	for(int i=1; i<6; i+=2){
		DrawArrow(v4[0], &v4[i]);
	}

	line->Begin();
	line->Draw(v4,2,0xFFFF0000);
	line->End();
	line->Begin();
	line->Draw(&v4[2],2,0xFFFF0000);
	line->End();
	line->Begin();
	line->Draw(&v4[4],2,0xFFFF0000);
	line->End();
	
}
	
void Visuals::RotateZ()
{
	float rad =0.01745329251994329576923690768489f;
	float radius= sqrt(pow(abs(org.x - from.x),2) + pow(abs(org.y - from.y),2)) +40;
	D3DXVECTOR2 v2[6];
	MYVERTEX v5[726];
	CreateMYVERTEX(&v5[0], org.x, org.y + (radius + 10.f), 0xAA121150);
	CreateMYVERTEX(&v5[1], org.x, org.y + radius, 0xAA121150);
	for(int j=0; j<181; j++){
		float xx= org.x + ((radius + 10.f) * sin ( (j*2) * rad ));
		float yy= org.y + ((radius + 10.f) * cos ( (j*2) * rad ));
		float xx1= org.x + (radius * sin ( (j*2) * rad ));
		float yy1= org.y + (radius * cos ( (j*2) * rad ));
		CreateMYVERTEX(&v5[j + 364], xx, yy, 0xAAFF0000);
		CreateMYVERTEX(&v5[j + 545], xx1, yy1, 0xAAFF0000);
		if(j<1){continue;}
		CreateMYVERTEX(&v5[(j*2)], xx, yy, 0xAA121150);
		CreateMYVERTEX(&v5[(j*2)+1], xx1, yy1, 0xAA121150);
		
	}
	if(radius){
		float xx1= org.x + ((radius-40) * sin ( lastmove.y * rad ));
		float yy1= org.y + ((radius-40) * cos ( lastmove.y * rad ));
		v2[0].x=xx1-5.0f;
		v2[0].y=yy1;
		v2[1].x=xx1+5.0f;
		v2[1].y=yy1;
		v2[2]=org;
		v2[3].x=xx1;
		v2[3].y=yy1;
		float xx2= xx1 + (radius * sin ( (lastmove.y+90) * rad ));
		float yy2= yy1 + (radius * cos ( (lastmove.y+90) * rad ));
		float xx3= xx1 + (radius * sin ( (lastmove.y-90) * rad ));
		float yy3= yy1 + (radius * cos ( (lastmove.y-90) * rad ));
		v2[4].x=xx2;
		v2[4].y=yy2;
		v2[5].x=xx3;
		v2[5].y=yy3;
		line->SetWidth(10.f);
		line->Begin();
		line->Draw(v2,2,0xAAFF0000);
		line->End();
		line->SetWidth(2.f);
		line->Begin();
		line->Draw(&v2[2],2,0xFFFF0000);
		line->Draw(&v2[4],2,0xFFFF0000);
		line->End();
	}
	v2[0]=org;
	v2[1]=to;
	v2[2].x=org.x-10.0f;
	v2[2].y=org.y;
	v2[3].x=org.x+10.0f;
	v2[3].y=org.y;
	v2[4].x=org.x;
	v2[4].y=org.y-10.0f;
	v2[5].x=org.x;
	v2[5].y=org.y+10.0f;
	line->SetWidth(5.f);

	HRN(device->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 360, v5, sizeof(MYVERTEX) ),"primitive failed");
	HRN(device->DrawPrimitiveUP( D3DPT_LINESTRIP, 180, &v5[364], sizeof(MYVERTEX) ),"primitive failed");
	HRN(device->DrawPrimitiveUP( D3DPT_LINESTRIP, 180, &v5[545], sizeof(MYVERTEX) ),"primitive failed");
	line->SetWidth(2.f);
	line->Begin();
	line->Draw(&v2[2],2,0xFFFF0000);
	line->End();
	line->Begin();
	line->Draw(&v2[4],2,0xFFFF0000);
	line->End();
	line->Begin();
	line->Draw(&v2[0],2,0xFFFF0000);
	line->End();

	
}

void Visuals::RotateXY()
{
	wxSize s = widsize;
	float ratio= (float)s.x/(float)s.y;
	float xxx=((org.x/s.x)*2)-1;
	float yyy=((org.y/s.y)*2)-1;
	D3DXMATRIX mat;
	D3DXMATRIX matRotate;    // a matrix to store the rotation information
	D3DXMATRIX matTramsate;
	
	D3DXMatrixRotationYawPitchRoll(&matRotate, D3DXToRadian(-angle.x),D3DXToRadian(angle.y),0);
	if(from!=org){
		float txx=((from.x/s.x)*60)-30;
		float tyy=((from.y/s.y)*60)-30;
		//wxLogStatus(" %f %f %f %f", from.x-org.x, from.y-org.y, txx, tyy);
		D3DXMatrixTranslation(&matTramsate,txx-(xxx),-(tyy-(yyy*30)),0.0f);
		matRotate=matTramsate*matRotate;
	}

	D3DXMATRIX matView;    // the view transform matrix

	D3DXMatrixLookAtLH(&matView,
					   &D3DXVECTOR3 (0.0f, 0.0f, -17.2f),    // the camera position
					   &D3DXVECTOR3 (0.0f, 0.0f, 0.0f),    // the look-at position
					   &D3DXVECTOR3 (0.0f, 1.0f, 0.0f));    // the up direction
	
	device->SetTransform(D3DTS_VIEW, &matView);    // set the view transform to matView

	D3DXMATRIX matProjection;     // the projection transform matrix
	
	D3DXMatrixPerspectiveFovLH(&matProjection,
								D3DXToRadian(120),    // the horizontal field of view
								ratio, // aspect ratio
								1.0f,    // the near view-plane
								100000.0f);    // the far view-plane

	D3DXMatrixTranslation(&matTramsate,xxx,-yyy,0.0f);
	device->SetTransform(D3DTS_PROJECTION, &(matProjection*matTramsate));    // set the projection
	
	device->SetTransform(D3DTS_WORLD, &matRotate);

	device->SetFVF(D3DFVF_XYZ|D3DFVF_DIFFUSE);
	MYVERTEX vertices[199];
	bool ster=true;
	
	float mm=60.0f/12.0f;
	float j=30-mm;
	float gg = 1.0f/12.f;
	float g=gg;
	int re=122,gr=57,bl=36;
	
	for(int i=0; i<44; i+=4)
	{
		if(i==20){re=255;gr=155;}else{re=122;gr=57;}
		CreateMYVERTEX(&vertices[i],j,-30.f,D3DCOLOR_ARGB((int)(g*155),re,gr,bl));
		CreateMYVERTEX(&vertices[i+1],j,0.f,D3DCOLOR_ARGB((int)(255),re,gr,bl));
		CreateMYVERTEX(&vertices[i+2],j,0.f,D3DCOLOR_ARGB((int)(255),re,gr,bl));
		CreateMYVERTEX(&vertices[i+3],j,30.f,D3DCOLOR_ARGB((int)(g*155),re,gr,bl));
		CreateMYVERTEX(&vertices[i+44],-30.f,j,D3DCOLOR_ARGB((int)(g*155),re,gr,bl));
		CreateMYVERTEX(&vertices[i+45],0.f,j,D3DCOLOR_ARGB((int)(255),re,gr,bl));
		CreateMYVERTEX(&vertices[i+46],0.f,j,D3DCOLOR_ARGB((int)(255),re,gr,bl));
		CreateMYVERTEX(&vertices[i+47],30.f,j,D3DCOLOR_ARGB((int)(g*155),re,gr,bl));
		j-=mm;
		if(g==1.f){ster=false;}
		if(ster){
			g+=gg;
		}else{
			g-=gg;
		}
	}
	device->DrawPrimitiveUP(D3DPT_LINELIST, 44, vertices, sizeof(MYVERTEX));
	float addy=(AN<4)?9.f : -9.f, addx= (AN % 3 == 0)?-9.f : 9.f;
	float add1y=(AN<4)?10.f : -10.f, add1x= (AN % 3 == 0)?-10.f : 10.f;
	CreateMYVERTEX(&vertices[176],0.f,addy,0xFFFF0000);//linia y
	CreateMYVERTEX(&vertices[177],0.f,0.f,0xFFFF0000);
	CreateMYVERTEX(&vertices[178],addx,0.f,0xFFFF0000); //linia x
	CreateMYVERTEX(&vertices[179],0.f,0.f,0xFFFF0000); //linia z
	CreateMYVERTEX(&vertices[180],0.f,0.f,0xFFFF0000,9.f);
	CreateMYVERTEX(&vertices[181],0.f,add1y,0xFFFF0000); //strzałka y
	CreateMYVERTEX(&vertices[182],0.f,addy,0xFFFF0000,-0.6f);
	CreateMYVERTEX(&vertices[183],-0.6f,addy,0xFFFF0000);
	CreateMYVERTEX(&vertices[184],0.f,addy,0xFFFF0000, 0.6f);
	CreateMYVERTEX(&vertices[185],0.6f,addy,0xFFFF0000);
	CreateMYVERTEX(&vertices[186],0.f,addy,0xFFFF0000, -0.6f);
	CreateMYVERTEX(&vertices[187],add1x,0.f,0xFFFF0000);//strzałka x
	CreateMYVERTEX(&vertices[188],addx,0.f,0xFFFF0000, -0.6f);
	CreateMYVERTEX(&vertices[189],addx,-0.6f,0xFFFF0000);
	CreateMYVERTEX(&vertices[190],addx,0.f,0xFFFF0000, 0.6f);
	CreateMYVERTEX(&vertices[191],addx,0.6f,0xFFFF0000, 0.f);
	CreateMYVERTEX(&vertices[192],addx,0.f,0xFFFF0000, -0.6f);
	CreateMYVERTEX(&vertices[193],0.f,0.f,0xFFFF0000,10.f); //strzałka z
	CreateMYVERTEX(&vertices[194],-0.6f,0.f,0xFFFF0000,9.f);
	CreateMYVERTEX(&vertices[195],0.f,0.6f,0xFFFF0000,9.f);
	CreateMYVERTEX(&vertices[196],0.6f,0.f,0xFFFF0000,9.f);
	CreateMYVERTEX(&vertices[197],0.f,-0.6f,0xFFFF0000,9.f);
	CreateMYVERTEX(&vertices[198],-0.6f,0.f,0xFFFF0000,9.f);
	device->DrawPrimitiveUP(D3DPT_LINESTRIP, 2, &vertices[176], sizeof(MYVERTEX));
	device->DrawPrimitiveUP(D3DPT_LINELIST, 1, &vertices[179], sizeof(MYVERTEX));
	device->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 4, &vertices[181], sizeof(MYVERTEX));
	device->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 4, &vertices[187], sizeof(MYVERTEX));
	device->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 4, &vertices[193], sizeof(MYVERTEX));
	D3DXMATRIX matOrtho; 
	D3DXMATRIX matIdentity;

	D3DXMatrixOrthoOffCenterLH(&matOrtho, 0, s.x, s.y, 0, 0.0f, 1.0f);
	D3DXMatrixIdentity(&matIdentity);

	HRN(device->SetTransform(D3DTS_PROJECTION, &matOrtho), _("Nie można ustawić matrixa projection"));
	HRN(device->SetTransform(D3DTS_WORLD, &matIdentity), _("Nie można ustawić matrixa world"));
	HRN(device->SetTransform(D3DTS_VIEW, &matIdentity), _("Nie można ustawić matrixa view"));
	D3DXVECTOR2 v2[4];
	v2[0].x=org.x-10.0f;
	v2[0].y=org.y;
	v2[1].x=org.x+10.0f;
	v2[1].y=org.y;
	v2[2].x=org.x;
	v2[2].y=org.y-10.0f;
	v2[3].x=org.x;
	v2[3].y=org.y+10.0f;
	line->Begin();
	line->Draw(&v2[0],2,0xFFFF0000);
	line->End();
	line->Begin();
	line->Draw(&v2[2],2,0xFFFF0000);
	line->End();
}
	
void Visuals::ClipRect()
{
	//wxLogStatus("siz %i,%i", Points[1].x, Points[1].y);
	D3DXVECTOR2 v2[5];
	wxSize s = widsize;
	v2[0].x=Points[0].x/wspw;
	v2[0].y=Points[0].y/wsph;
	v2[1].x=v2[0].x;
	v2[1].y=(Points[1].y/wsph)-1;
	v2[2].x=(Points[1].x/wspw)-1;
	v2[2].y=v2[1].y;
	v2[3].x=v2[2].x;
	v2[3].y=v2[0].y;
	v2[4].x=v2[0].x;
	v2[4].y=v2[0].y;

	if(!invClip){
		
		
		MYVERTEX v24[12];
		CreateMYVERTEX(&v24[0],0, 0, 0x88000000);
		CreateMYVERTEX(&v24[1],s.x, 0, 0x88000000);
		CreateMYVERTEX(&v24[2],v2[2].x, v2[0].y, 0x88000000);
		CreateMYVERTEX(&v24[3],v2[0].x, v2[0].y, 0x88000000);
		CreateMYVERTEX(&v24[4],v2[0].x, v2[2].y, 0x88000000);
		CreateMYVERTEX(&v24[5],0, s.y, 0x88000000);
		CreateMYVERTEX(&v24[6],s.x, s.y, 0x88000000);
		CreateMYVERTEX(&v24[7],0, s.y, 0x88000000);
		CreateMYVERTEX(&v24[8],v2[0].x, v2[2].y, 0x88000000);
		CreateMYVERTEX(&v24[9],v2[2].x, v2[2].y, 0x88000000);
		CreateMYVERTEX(&v24[10],v2[2].x, v2[0].y, 0x88000000);
		CreateMYVERTEX(&v24[11],s.x, 0, 0x88000000);

		HRN(device->SetFVF(D3DFVF_XYZ|D3DFVF_DIFFUSE), "fvf failed");
		HRN(device->DrawPrimitiveUP( D3DPT_TRIANGLEFAN, 4, v24, sizeof(MYVERTEX) ),"primitive failed");
		HRN(device->DrawPrimitiveUP( D3DPT_TRIANGLEFAN, 4, &v24[6], sizeof(MYVERTEX) ),"primitive failed");
	}else{
		MYVERTEX v24[4];
		CreateMYVERTEX(&v24[0],v2[0].x, v2[0].y, 0x88000000);
		CreateMYVERTEX(&v24[1],v2[2].x, v2[0].y, 0x88000000);
		CreateMYVERTEX(&v24[2],v2[0].x, v2[2].y, 0x88000000);
		CreateMYVERTEX(&v24[3],v2[2].x, v2[2].y, 0x88000000);
		HRN(device->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, v24, sizeof(MYVERTEX) ),"primitive failed");
	}

	line->Begin();
	line->Draw(v2,5,0xFFFF0000);
	line->End();
	/*line->Begin();
	line->Draw(&v2[2],2,0xFFFF0000);
	line->End();
	line->Begin();
	line->Draw(&v2[4],2,0xFFFF0000);
	line->End();
	line->Begin();
	line->Draw(&v2[6],2,0xFFFF0000);
	line->End();*/
}
	
void Visuals::FaXY()
{
	D3DXVECTOR2 v2[2];
	v2[0]=from;
	v2[1]=to;
	line->Begin();
	line->Draw(v2,2,0xFFFF0000);
	line->End();
}

void Visuals::Draw(int time)
{
	if(!(time>=start && time<end)){blockevents = true; return;}else if(blockevents){blockevents=false;}
	wxMutexLocker lock(clipmutex);
	line->SetAntialias(TRUE);
	line->SetWidth(2.0);

	if(time != oldtime && Visual<CLIPRECT && Visual != MOVE && Visual != CHANGEPOS && tbl[6]>3){
		bool corg = ((Visual==ROTATEZ || Visual== ROTATEXY) && org==from);
		from=CalcMovePos();
		from.x/=wspw; from.y/=wsph;
		to=from;
		if(corg){org=from;}
		else{
			to=org;
		}
		
		
	}
	switch(Visual)
	{
	case CHANGEPOS:
		Position();
		break;
	case MOVE:
		Move(time);
		break;
	/*case MOVEONCURVE:
		MoveOnCurve(time);
		break;*/
	case SCALE:
		Scale();
		break;
	case ROTATEZ:
		RotateZ();
		break;
	case ROTATEXY:
		RotateXY();
		break;
	case CLIPRECT:
		ClipRect();
		break;
	/*case FAXY:
		FaXY();
		break;*/
	case VECTORCLIP:
	case VECTORDRAW:
		DrawClip();
		break;
	default:
		wxLogStatus(_("Niewłaściwy wisual %i"), Visual);
		break;
	}
	
	/*if(drawtxt){
		wxSize wsize = widsize;
		wsize.x/=2; wsize.y/=2;
		align=0;
		align |= (x>wsize.x)? DT_RIGHT : 0;
		align |= (y>wsize.y)? DT_BOTTOM : 0;
		cpos.left=(x>wsize.x)? x-150 : x+5;
		cpos.top=(y>wsize.y)? y-50 : y+5;
		cpos.right=(x>wsize.x)? x-5 : x+150;
		cpos.bottom=(y>wsize.y)? y-5 : y+50;
		DRAWOUTTEXT(font,coords,cpos,align,0xFFFFFFFF)
	}*/
	line->SetAntialias(FALSE);
	oldtime=time;
}


wxString Visuals::GetVisual(bool _org)
{
	wxString result;
	if(_org){
		result = "\\org("+getfloat(org.x*wspw)+","+getfloat(org.y*wsph)+")";
	}else if(Visual==CLIPRECT && Points.size()>1){
		result = wxString::Format("\\%sclip(%i,%i,%i,%i)",(invClip)? "i" : "", Points[0].x, Points[0].y, Points[1].x, Points[1].y);
	}else if(Visual==CHANGEPOS){
		result = "\\pos("+getfloat(from.x*wspw)+","+getfloat(from.y*wsph)+")";
	}else if(Visual==MOVE){
		result = "\\move("+getfloat(from.x*wspw)+","+getfloat(from.y*wsph)+","+getfloat(to.x*wspw)+","+getfloat(to.y*wsph)+times+")";
	}else if(Visual==SCALE){
		if(to.x==from.x){to.x=from.x+60.f;}
		if(to.y==from.y){to.y=from.y+60.f;}
		//wxLogStatus("to %f %f from %f %f", to.x, to.y, from.x, from.y);
		if(type!=1){
			float res=(abs(to.x - from.x))/60.f;
			result += "\\fscx" + getfloat(res*100);
			scale.x=res;
			//wxLogStatus("fscx %f to-from %f", res, to.x - from.x);
		}if(type!=0){
			float res=(abs(to.y - from.y))/60.f;
			result += "\\fscy" + getfloat(res*100);
			scale.y=res;
			//wxLogStatus("fscy %f to - from %f ", res, to.y-from.y);
		}
	}else if(Visual==ROTATEZ){

		float angle = lastmove.x - atan2((org.y-to.y), (org.x-to.x)) * (180.f / 3.1415926536f);
		angle = fmodf(angle + 360.f, 360.f);
		//wxLogStatus("angle %f %f", angle, lastmove.x);
		result="\\frz"+ getfloat(angle);
		lastmove.y=angle;
		
	}else if(Visual==ROTATEXY){
		if(type!=1){
			angle.x = (to.x - firstmove.x) + scale.x;
			angle.x = fmodf(angle.x + 360.f, 360.f);
			result += "\\fry"+ getfloat(angle.x);
			//wxLogStatus("rot x %f", angle.x);
		}
		if(type!=0){
			float angy = (to.y - firstmove.y) - scale.y;// zmieniony plus na minus by nie trzebabyło 
			angle.y = (fmodf((-angy) + 360.f, 360.f));//przetrzymywać scale i angle w minusach.
			result += "\\frx" + getfloat(angle.y);
			//wxLogStatus("rot y %f", angle.y);
		}
		//wxLogStatus("rot xy %f, %f", angle.x, angle.y);
	}
	return result;
}

void Visuals::MouseEvent(wxMouseEvent &evt)
{
	if(blockevents){return;}
	if(Visual>=VECTORCLIP){OnMouseEvent(evt);return;}//clipy wektorowe i rysunki
	bool click = evt.LeftDown()||evt.RightDown()||evt.MiddleDown();
	bool holding = (evt.LeftIsDown()||evt.RightIsDown()||evt.MiddleIsDown());
	
	TabPanel* pan=(TabPanel*)parent->GetParent();
	
	if(pan->Video->isfullskreen){wxGetMousePosition(&x,&y);}
	else{evt.GetPosition(&x,&y);}

	if(evt.ButtonUp()){
		if(parent->HasCapture()){parent->ReleaseMouse();}
		drawtxt=false;
		//usunąć po zrobieniu całości visuala
		/*if(Visual!=MOVEONCURVE){*/
			pan->Edit->SetVisual(GetVisual(grabbed==100),false,(grabbed==100) ? 100 : type);
		//}
		if(Visual==ROTATEZ){
			to=org;
			if(grabbed==100){
				lastmove.x = atan2((org.y-y), (org.x-x)) * (180.f / 3.1415926536f);
				lastmove.x+=lastmove.y;
			}
			pan->Video->Render();
		}
		if(Visual==ROTATEXY){
			scale=angle;// vobsub jeden frx ma zamieniony i stąd te minusy // nie ma już minusów i precz z nimi;
		}
		if(!hasArrow){parent->SetCursor(wxCURSOR_ARROW);hasArrow=true;}
		grabbed=-1;
		
	}

	if(Visual==CLIPRECT && !holding){

		bool setarrow=false;
		for(int i = 0; i<2; i++){
			int pointx = Points[i].x/wspw, pointy = Points[i].y/wsph;
			if(abs(x-pointx)<5){
				setarrow=true;
				parent->SetCursor(wxCURSOR_SIZEWE);
				hasArrow=false;break;}
			if(abs(y-pointy)<5){
				setarrow=true;
				parent->SetCursor(wxCURSOR_SIZENS);
				hasArrow=false;break;
			}
		}
		if(!setarrow && !hasArrow){parent->SetCursor(wxCURSOR_ARROW);hasArrow=true;}
		//return;
	}else if(Visual==SCALE && !holding){
		if(abs(lastmove.x-x)<8 && abs(lastmove.y-y)<8){if(hasArrow){parent->SetCursor(wxCURSOR_SIZING);hasArrow=false;}}
		else if(abs(lastmove.x-x)<8 && abs(from.y-y)<8){if(hasArrow){parent->SetCursor(wxCURSOR_SIZEWE);hasArrow=false;}}
		else if(abs(lastmove.y-y)<8 && abs(from.x-x)<8){if(hasArrow){parent->SetCursor(wxCURSOR_SIZENS);hasArrow=false;}}
		else if(!hasArrow){parent->SetCursor(wxCURSOR_ARROW);hasArrow=true;}
	}

	/*if(click || holding){
		drawtxt=true;
		coords=""; 
		coords<<(int)(x*wspw)<<", "<<(int)(y*wsph);
	}*/
	//kliknięcie
	if(click){
		if(Visual==CHANGEPOS){
			parent->SetCursor(wxCURSOR_SIZING);
			hasArrow=false;
			firstmove.x=x;
			firstmove.y=y;
			lastmove=from;
		}else if(Visual==CLIPRECT){
			grabbed=-1;
			for(int i = 0; i<2; i++){
				int pointx = Points[i].x/wspw, pointy = Points[i].y/wsph;
				if(abs(x-pointx)<5){
					diffs.x=(pointx)-x; grabbed=i;
					break;}
				if(abs(y-pointy)<5){
					diffs.y=(pointy)-y; grabbed=i+2;
					break;
				}
			}
			return;
		}
		else if(Visual==SCALE){
			if(evt.LeftDown()){type=0;}
			if(evt.RightDown()){type=1;}
			if(evt.MiddleDown()){type=2;}
			if(abs(lastmove.x-x)<8 && abs(from.y-y)<8){grabbed=0;type=0;}
			else if(abs(lastmove.y-y)<8 && abs(from.x-x)<8){grabbed=1;type=1;}
			else if(abs(lastmove.x-x)<8 && abs(lastmove.y-y)<8){grabbed=2;type=2;}
			diffs.x=lastmove.x-x;
			diffs.y=lastmove.y-y;
			if(type==0){parent->SetCursor(wxCURSOR_SIZEWE);}
			if(type==1){parent->SetCursor(wxCURSOR_SIZENS);}
			if(type==2){parent->SetCursor(wxCURSOR_SIZING);}
			hasArrow=false;
			if(grabbed==-1){
				int addy=(AN>3)?60 : -60, addx= (AN % 3 == 0)?-60 : 60;
				diffs.x=(from.x-x)+(addx*scale.x);
				diffs.y=(from.y-y)+(addy*scale.y);
			}
		}
		else if(Visual==ROTATEZ){
			parent->CaptureMouse();
			grabbed=-1;
			parent->SetCursor(wxCURSOR_SIZING);
			hasArrow=false;
			if(abs(org.x-x)<8 && abs(org.y-y)<8){
				grabbed=100; 
				diffs.x=org.x-x;
				diffs.y=org.y-y;
				return;
			}else{
				lastmove.x = atan2((org.y-y), (org.x-x)) * (180.f / 3.1415926536f);
				lastmove.x+=lastmove.y;
			}
			
		}
		else if(Visual==ROTATEXY){
			parent->CaptureMouse();
			if(evt.LeftDown()){type=0;}
			if(evt.RightDown()){type=1;}
			if(evt.MiddleDown()){type=2;}
			if(abs(org.x-x)<8 && abs(org.y-y)<8){grabbed=100;
				diffs.x=org.x-x;
				diffs.y=org.y-y;
			}
			firstmove= D3DXVECTOR2(x,y);
			if(type==0){parent->SetCursor(wxCURSOR_SIZEWE);}
			if(type==1){parent->SetCursor(wxCURSOR_SIZENS);}
			if(type==2){parent->SetCursor(wxCURSOR_SIZING);}
			hasArrow=false;
		}else if(Visual==MOVE){

			parent->SetCursor(wxCURSOR_SIZING );
			hasArrow=false;
			if(evt.LeftDown()){type=0;}
			if(evt.RightDown()){type=1;}
			if(abs(from.x-x)<8 && abs(from.y-y)<8){
				grabbed=0;type=0;
				diffs.x=from.x-x;
				diffs.y=from.y-y;
			}
			else if(abs(to.x-x)<8 && abs(to.y-y)<8){
				grabbed=1;type=1;
				diffs.x=to.x-x;
				diffs.y=to.y-y;
			}
			else{
				grabbed= -1;
				if(type==1){
					to.x=x;
					to.y=y;
				}else{
					from.x=x;
					from.y=y;
				}
				diffs=wxPoint(0,0);
			}
			
			pan->Edit->SetVisual(GetVisual(),true,type);
			return;
		}/*else if(Visual==MOVEONCURVE){
			if(Points.size()<4){
				Points.clear();
				int diffx=(from.x-x)/3.0f;
				int diffy=(from.y-y)/3.0f;
				Points.push_back(ClipPoint(from.x,from.y,"r",true));
				Points.push_back(ClipPoint(x+(diffx*2),y+(diffy*2),"r", false));
				Points.push_back(ClipPoint(x+diffx,y+diffy,"r",false));
				Points.push_back(ClipPoint(x,y,"r",false));
			}

			grabbed = CheckPos(wxPoint(x,y),false,false);
			if(grabbed!= -1){
				acpoint=Points[grabbed];
				diffs.x=acpoint.x-x;
				diffs.y=acpoint.y-y;
			}
			pan->Video->Render();
			return;
		}*/
		to.x=x;to.y=y;

		//trzymamie myszy
	}else if(holding){
		if(Visual==CHANGEPOS){
			from.x = lastmove.x-(firstmove.x-x);
			from.y = lastmove.y-(firstmove.y-y);
		}else if(Visual==CLIPRECT && grabbed!=-1){

			if(grabbed<2){
				x=MID(0,x,widsize.x);
				Points[grabbed].x=((x+diffs.x)*wspw);
				if(grabbed==0 && Points[0].x > Points[1].x){Points[0].x = Points[1].x;}
				if(grabbed==1 && Points[1].x < Points[0].x){Points[1].x = Points[0].x;}	
			}else{
				y=MID(0,y,widsize.y);
				Points[grabbed-2].y=((y+diffs.y)*wsph);
				if(grabbed==2 && Points[0].y > Points[1].y){Points[0].y = Points[1].y;}
				if(grabbed==3 && Points[1].y < Points[0].y){Points[1].y = Points[0].y;}	
			}
			
		}else if(grabbed==100){// przenoszenie org dostało liczbę 100 by przypadkowo nie wbiło się na inny punkt.
			org.x = x+diffs.x;
			org.y = y+diffs.y;
			//wxLogStatus("org %f, %f, %i %i %i %i", org.x,org.y, x, y, diffs.x, diffs.y);
			
			pan->Edit->SetVisual(GetVisual(true),true,grabbed);//type także ma liczbę 100 by było rozpoznawalne.
			
			return;
		}else if(Visual==SCALE){
			//wxLogStatus("type hold %i %i",type, grabbed);
			//if(grabbed!= -1){
				if(type!=1){
					to.x=x+diffs.x;
				}
				if(type!=0){
					to.y=y+diffs.y;
				}
				//wxLogStatus("hold %f %f",to.x, to.y);
			goto done;
		}else if(Visual==MOVE){
			if(type==0){
				from.x=x+diffs.x;
				from.y=y+diffs.y;
			}else{
				to.x=x+diffs.x;
				to.y=y+diffs.y;
			}
			goto done;
		}
		//else if(Visual==MOVEONCURVE && grabbed!=-1){
		//
		//	x=MID(0,x,widsize.x);
		//	y=MID(0,y,widsize.y-44);
		//	Points[grabbed].x=(x+diffs.x);
		//	Points[grabbed].y=(y+diffs.y);
		//	//acpoint=Points[grabbed];
		//	TabPanel* pan=(TabPanel*)parent->GetParent();
		//	pan->Video->Render();//Edit->SetClip(GetClip(),true);
		//	return;
		//}
		//}else if((Visual==SCALE || Visual==ROTATEXY)&& newmove){
		//	if(type==0 && (abs(to.y-y) > abs(to.x-x))){type=1;}
		//	else if(type==1 && (abs(to.y-y) < abs(to.x-x))){type=0;}
		//	//wxLogStatus("type hold %f %f %i",abs(to.y-y), abs(to.y-y), type);
		//	newmove=false;
		//}

		to.x=x;to.y=y;
done:
		pan->Edit->SetVisual(GetVisual(),true,type);
		
	}



}

D3DXVECTOR2 Visuals::CalcMovePos()
{
	D3DXVECTOR2 ppos;
	int time=((TabPanel*) parent->GetParent())->Video->Tell();
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
	//wxLogStatus("pos %f %f",ppos.x, ppos.y, from.x, from.y);
	return ppos;
}

BEGIN_EVENT_TABLE(Visuals, wxEvtHandler)
	EVT_MOUSE_EVENTS(Visuals::MouseEvent)
END_EVENT_TABLE()