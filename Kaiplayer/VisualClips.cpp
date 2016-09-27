#include "TabPanel.h"
#include "Visuals.h"
#include <wx/tokenzr.h>
#include <wx/regex.h>
#include <math.h> 

float DrawingAndClip::_x=0;
float DrawingAndClip::_y=0;

ClipPoint::ClipPoint()
{
	x=0;
	y=0;
	type="m";
	start=true;
}

ClipPoint::ClipPoint(float _x, float _y, wxString _type, bool isstart)
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
	D3DXVECTOR2 v = D3DXVECTOR2((x+DrawingAndClip::_x)/Visuals::wspw,(y+DrawingAndClip::_y)/Visuals::wsph);
	return v;
}

int ClipPoint::wx()
{
	return (x+DrawingAndClip::_x)/Visuals::wspw;
}
int ClipPoint::wy()
{
	return (y+DrawingAndClip::_y)/Visuals::wsph;
}

DrawingAndClip::DrawingAndClip()
	:Visuals()
	,newline(false)
	,newmove(false)
	,drawtxt(false)
	,invClip(false)
	,grabbed(-1)
{
}

DrawingAndClip::~DrawingAndClip()
{
	Points.clear();
}

void DrawingAndClip::DrawVisual(int time)
{
	if(Visual==VECTORDRAW && tbl[6]>2){D3DXVECTOR2 movePos = CalcMovePos(); _x = movePos.x; _y = movePos.y;}
	//wxLogStatus("Create line %i %i", Points[0].x, Points[0].y);
	size_t g=1;
	size_t size = Points.size();
	while(g < size){
		if(Points[g].type=="b"||Points[g].type=="s"){
			g+=DrawCurve(g,(Points[g].type=="s"));
		}else if(Points[g].type=="l"){
			DrawLine(g);
			g++;
		}else{
			DrawRect(g-1);
			g++;
		}

	}
	if(size>2){
		line->Begin();
		D3DXVECTOR2 v2[2]={Points[size-1].GetVector(), Points[0].GetVector()};
		line->Draw(v2, 2, 0xFFFF0000);
		line->End();
	}
	DrawRect(size-1);
	DrawRect(0);

	if(drawtxt){
		wxString coords=""; 
		coords<<wxString::Format(_T("%6.2f"),acpoint.x + offsetxy.x).Trim(false)<<", "<<
			wxString::Format(_T("%6.2f"),acpoint.y + offsetxy.y).Trim(false);
		wxSize wsize=tab->Video->GetClientSize();
		wsize.x/=2; wsize.y/=2;
		int x=acpoint.wx(), y=acpoint.wy();
		DWORD align=0;
		align |= (x>wsize.x)? DT_RIGHT : 0;
		align |= (y>wsize.y)? DT_BOTTOM : 0;
		RECT cpos;
		cpos.left=(x>wsize.x)? x-150 : x+5;
		cpos.top=(y>wsize.y)? y-50 : y+5;
		cpos.right=(x>wsize.x)? x-5 : x+150;
		cpos.bottom=(y>wsize.y)? y-5 : y+50;
		DRAWOUTTEXT(font,coords,cpos,align,0xFFFFFFFF)
	}
}

	
void DrawingAndClip::SetCurVisual()
{
	wxString clip;
	D3DXVECTOR2 linepos = GetPosnScale(&scale, &alignment, (Visual==VECTORDRAW)? tbl : NULL);
	if(Visual!=VECTORDRAW){
		bool found =tab->Edit->FindVal("(i?clip[^\\)]+)", &clip);
		if(found){int rres = clip.Replace(",",",");
			if( rres >= 3) {clip = "";} 
			else{clip = clip.AfterFirst('(');}
		}
		_x=0;
		_y=0;
	}else{
		wxString txt=tab->Edit->TextEdit->GetValue();
		wxRegEx re("(.*){[^}]*}(m[^{]+){[^}]*\\\\p0[^}]*}(.*)", wxRE_ADVANCED);
		if(re.Matches(txt)){
			clip = re.GetMatch(txt,2);
			textwithclip = re.GetMatch(txt,1).AfterLast('}') + re.GetMatch(txt,3).BeforeFirst('{');
			wxLogStatus("clip "+ clip + " text "+textwithclip);
		}else{
			wxRegEx re("(.*){[^}]*}(m[^{]+)", wxRE_ADVANCED);
			if(re.Matches(txt)){
				clip = re.GetMatch(txt,2);
				textwithclip = re.GetMatch(txt,1).AfterLast('}');
				wxLogStatus("clip "+ clip + " text "+textwithclip);
			}else{
				wxRegEx re("{[^}]*}", wxRE_ADVANCED);
				re.ReplaceAll(&txt,"");
				textwithclip = txt;
				wxLogStatus("text "+textwithclip);
			}
		}
		

		_x=linepos.x/scale.x;
		_y=(linepos.y/scale.y);
		wspw/=scale.x;
		wsph/=scale.y;
	}

	Points.clear();
	wxStringTokenizer tokens(clip," ");
	double tmpx=0;
	bool gotx=false;
	bool start=false;
	wxString type;
	while(tokens.HasMoreTokens()){
		wxString token=tokens.GetNextToken();
		if(token=="p"){token="s";}
		if(token=="m"||token=="l"||token=="b"||token=="s"){type=token;start=true;}
		else if(token=="c"){continue;}
		else if(gotx){
			double tmpy=0;
			token.ToCDouble(&tmpy);
			Points.push_back(ClipPoint(tmpx, tmpy,type,start));
			gotx=false;
			start=false;
		}
		else{
			token.ToCDouble(&tmpx);
			gotx=true;
		}
	
	}
	//wxLogStatus(clip);
	if(Points.empty()){
		AddMove(wxPoint(((SubsSize.x)/(wspw*scale.x))/2, ((SubsSize.y)/(wsph*scale.y))/2),0);
	}else if(Visual == VECTORDRAW){
		D3DXVECTOR2 xyoffset = CalcWH();
		for(size_t i = 0; i<Points.size(); i++){
			Points[i].x -= xyoffset.x;
			Points[i].y -= xyoffset.y;
		}
	}
	acpoint=Points[0];
}
	
wxString DrawingAndClip::GetVisual()
{
	wxString format = "6.2f"; //: "6.2f";
	wxString clip;
	int cntb=0;
	bool spline=false;
	offsetxy = (Visual==VECTORDRAW)? CalcWH() : D3DXVECTOR2(0,0);
	for(size_t i=0; i<Points.size(); i++)
	{
		ClipPoint pos=Points[i];
		float x= pos.x + offsetxy.x;
		float y= pos.y + offsetxy.y;
		if(cntb && !pos.start){
			clip<<getfloat(x,format)<<" "<<getfloat(y,format)<<" ";
			cntb++;
			//if(cntb>2 && pos.type=="b"){cntb=0;}
		}else{
			if(spline){clip<<"c ";spline=false;}
			clip<<pos.type<<" "<<getfloat(x,format)<<" "<<getfloat(y,format)<<" ";
			if(pos.type=="b"||pos.type=="s"){cntb=1;if(pos.type=="s"){spline=true;}}
		}
	}
	if(spline){clip<<"c ";}
	return clip.Trim();
}

void DrawingAndClip::SetPos(int x, int y)
{
	_x= x;
	_y= y;
}

// pos in skreen position
int DrawingAndClip::CheckPos(wxPoint pos, bool retlast, bool wsp)
{
	if(wsp){pos.x =(pos.x*wspw)-_x; pos.y =(pos.y*wsph)-_y;}
	for(size_t i=0; i<Points.size(); i++)
	{
		if(Points[i].IsInPos(pos,5)){return i;}
	}
	return (retlast)? Points.size() : -1;
}

// pos in clip position
void DrawingAndClip::MovePoint(wxPoint pos, int point)
{
	Points[point].x=pos.x;
	Points[point].y=pos.y;
}
// pos in skreen position	
void DrawingAndClip::AddCurve(wxPoint pos, int whereis, wxString type)
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
void DrawingAndClip::AddCurvePoint(wxPoint pos, int whereis)
{
	if(Points[whereis-1].type=="s"||((int)Points.size()>whereis && Points[whereis].type=="s"))
	{
		Points.insert(Points.begin()+whereis, ClipPoint((pos.x*wspw)-_x, (pos.y*wsph)-_y,"s",false));
	}
	else{wxBell();}
}
// pos in skreen position	
void DrawingAndClip::AddLine(wxPoint pos, int whereis)
{
	Points.insert(Points.begin()+whereis, ClipPoint((pos.x*wspw)-_x, (pos.y*wsph)-_y,"l",true));
	//wxLogStatus("line %i, %i, %f, %f, %f, %f", pos.x, pos.y, wspw, wsph, _x, _y);
	acpoint=Points[whereis];
}
// pos in skreen position	
void DrawingAndClip::AddMove(wxPoint pos, int whereis)
{
	//wxLogStatus(" wsps %i, %i, %f, %f, %f, %f", pos.x, pos.y, wspw, wsph, _x, _y);
	Points.insert(Points.begin()+whereis, ClipPoint((pos.x*wspw)-_x, (pos.y*wsph)-_y,"m",true));
	acpoint=Points[whereis];
}
	
void DrawingAndClip::DrawLine(int i)
{
	line->Begin();
	int diff = (Points[i-1].type=="s")? 2 : 1;
	D3DXVECTOR2 v2[2]={Points[i-diff].GetVector(),Points[i].GetVector()};
	line->Draw(v2, 2, 0xFFFF0000);
	line->End();
	if(i>1){DrawRect(i-1);}
	
}

void DrawingAndClip::DrawRect(int coord)
{
	Visuals::DrawRect(Points[coord].GetVector());
}
	
void DrawingAndClip::DrawCircle(int coord)
{
	Visuals::DrawCircle(Points[coord].GetVector());
}
	
int DrawingAndClip::DrawCurve(int i, bool bspline)
{
	line->Begin();
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
		int iplus1= (i+bssize < (int)Points.size()-1)? i+bssize+1 : 0;
		if(i-1 != 0 || iplus1 != 0){
			D3DXVECTOR2 v3[3]={Points[i-1].GetVector(), v4[0], Points[iplus1].GetVector()};
			line->Draw(v3, 3, 0xFFFF0000);
		}
		delete[] v2;
	}else{
		Curve(i-1, &v4,false);
		if(tmp.type=="s"){Points[i-1]=tmp;}
		D3DXVECTOR2 v2[4]={Points[i-1].GetVector(),Points[i].GetVector(),Points[i+1].GetVector(),Points[i+2].GetVector()};
		line->Draw(v2, 2, 0xFF0000FF);
		line->Draw(&v2[2], 2, 0xFF0000FF);
	}
	line->Draw(&v4[0], v4.size(), 0xFFFF0000);
	line->End();
	if(i>1){DrawRect(i-1);}
	for(int j=1; j<pts; j++){DrawCircle(i+j-1);}
	return pts;
}


	
void DrawingAndClip::Curve(int pos, std::vector<D3DXVECTOR2> *table, bool bspline, int spoints, int acpt)
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
		//if(bspline && t==0){Visuals::DrawRect((*table)[table->size()-1]);}
	}
	p_x = a[0] + a[1] + a[2] + a[3];
    p_y = b[0] + b[1] + b[2] + b[3];
	table->push_back(D3DXVECTOR2(p_x,p_y));
}

void DrawingAndClip::OnMouseEvent(wxMouseEvent &event)
{
	int x, y;
	if(tab->Video->isfullskreen){wxGetMousePosition(&x,&y);}
	else{event.GetPosition(&x,&y);}
	wxPoint xy=wxPoint(x, y);
	bool click=event.LeftDown();
	bool right=event.RightDown();
	bool ctrl=event.ControlDown();
	//bool click=event.LeftDown();
	drawtxt=(event.MiddleUp())?false : true;
	//if(Visual==VECTORDRAW){CalcWH();}
	if(!event.ButtonDown() && !event.LeftIsDown()){
		//wxLogStatus(" bdown %i", (int)event.ButtonDown());
		int pos = CheckPos(xy);
		if(pos!= -1 && hasArrow){
			tab->Video->SetCursor(wxCURSOR_SIZING);hasArrow=false;
		}else if(pos== -1 && !hasArrow){tab->Video->SetCursor(wxCURSOR_ARROW);hasArrow=true;}
	}
	
	if(event.ButtonUp()){
		SetClip(GetVisual(),false);
	}

	if(event.LeftUp()){
		
		if(tab->Video->HasCapture()){
			tab->Video->ReleaseMouse();}
		return;
	}
	if(right&&ctrl&&event.AltDown())
	{
		//wxLogStatus("point");
		if(Points.empty()){wxBell(); return;}
		AddCurvePoint(xy,CheckPos(xy, true));
		SetClip(GetVisual(),true);
		return;
	}
	if(right&&ctrl)
	{
		//wxLogStatus("curve");
		if(Points.empty()){wxBell(); return;}
		AddCurve(xy,CheckPos(xy, true),"s");
		SetClip(GetVisual(),true);
		return;
	}
	if(right)
	{
		if(Points.empty()){wxBell(); return;}
		AddLine(xy,CheckPos(xy, true));
		SetClip(GetVisual(),true);
		return;
	}
	if(ctrl&&event.AltDown()&&click)
	{
		if(Points.empty()){wxBell(); return;}
		AddCurve(xy,CheckPos(xy, true));
		SetClip(GetVisual(),true);
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
				SetClip(GetVisual(),true);
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
				lastpoint = acpoint = Points[i];
				grabbed=i;
				diffs.x=pointx-x;
				diffs.y=pointy-y;
				tab->Video->CaptureMouse();
				snapYminus=false;snapYplus=false;snapXminus=false;snapXplus=false;
				break;
			}
		}
		if(newmove && grabbed==-1)
		{
			
			AddMove(xy,CheckPos(xy, true));newmove=false;
			SetClip(GetVisual(),true);
			grabbed=Points.size()-1;
		}

	}

	if(event.LeftIsDown() && grabbed!=-1)
	{
		x=MID(0,x,VideoSize.x);
		y=MID(0,y,VideoSize.y);
		Points[grabbed].x=((x+diffs.x)*wspw)-_x;
		Points[grabbed].y=((y+diffs.y)*wsph)-_y;
		if(event.ShiftDown()){
			int grabbedPlus1 = (grabbed >= (int)Points.size()-1)? 0 : grabbed+1;
			int grabbedMinus1 = (grabbed < 1)? Points.size()-1 : grabbed-1;
			if(Points[grabbed].y == Points[grabbedMinus1].y || snapYminus){snapYminus=true;Points[grabbedMinus1].y = Points[grabbed].y;}
			if(Points[grabbed].y == Points[grabbedPlus1].y || snapYplus){snapYplus=true;Points[grabbedPlus1].y = Points[grabbed].y;}
			if(Points[grabbed].x == Points[grabbedMinus1].x || snapXminus){snapXminus=true;Points[grabbedMinus1].x = Points[grabbed].x;}
			if(Points[grabbed].x == Points[grabbedPlus1].x || snapXplus){snapXplus=true;Points[grabbedPlus1].x = Points[grabbed].x;}
			if(!(snapYminus||snapYplus||snapXminus||snapXminus)){
				if(abs(Points[grabbed].x - lastpoint.x)<10){
					Points[grabbed].x = lastpoint.x;
				}
				if(abs(Points[grabbed].y - lastpoint.y)<10){
					Points[grabbed].y = lastpoint.y;
				}
			}
		}
		acpoint=Points[grabbed];
		SetClip(GetVisual(),true);

	}
	

}



D3DXVECTOR2 DrawingAndClip::CalcWH()
{
	if (alignment==7 || Points.size()<1){return D3DXVECTOR2(0,0);}
	float offx=0, offy=0;
	if(textwithclip!=""){
		Styles *textstyle= tab->Grid1->GetStyle(0,tab->Edit->line->Style);
		wxFont stylefont(wxAtoi(textstyle->Fontsize),wxSWISS,(textstyle->Italic)? wxFONTSTYLE_ITALIC : wxFONTSTYLE_NORMAL,
			(textstyle->Bold)? wxBOLD : wxNORMAL, textstyle->Underline, textstyle->Fontname);//, textstyle->Encoding
		int ex=0, ey=0, eb=0, et=0;
		wxClientDC dc(tab);
		dc.GetTextExtent(textwithclip, &ex, &ey, &eb, &et, &stylefont);
		offx = ex / wspw;
		offy = eb / wsph;
		//wxLogStatus("textextent %i %i %i %i", ex, ey, eb, et);
	}
	//no i tutaj jeszcze zosta³o dopisaæ obliczanie rozmiaru
	float minx = FLT_MAX;
	float miny = FLT_MAX;
	float maxx = -FLT_MAX;
	float maxy = -FLT_MAX;
	for(size_t i = 0; i<Points.size(); i++)
	{
		ClipPoint p=Points[i];
		if(p.x<minx){minx=p.x;}
		if(p.y<miny){miny=p.y;}
		if(p.x>maxx){maxx=p.x;}
		if(p.y>maxy){maxy=p.y;}
	}
	D3DXVECTOR2 sizes((maxx-minx)+offx,(maxy-miny)+offy);
	//wxLogStatus("sizes %f, %f, %i", sizes.x, sizes.y, (int)alignment);
	D3DXVECTOR2 result = D3DXVECTOR2(0,0);
	if(alignment % 3==2){
		result.x = sizes.x/2.0;
	}else if(alignment % 3==0){
		result.x = sizes.x;
	}
	if(alignment < 4){
		result.y = sizes.y;
	}else if(alignment < 7){
		result.y = sizes.y/2.0;
	}
	//wxLogStatus("sizes2 %f, %f", result.x, result.y);
	return result;
}

