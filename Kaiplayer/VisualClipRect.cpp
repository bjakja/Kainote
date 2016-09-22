#include "Visuals.h"
#include "TabPanel.h"
#include <wx/regex.h>

ClipRect::ClipRect()
	: Visuals()
	, invClip(false)
	, grabbed(-1)
{
}

void ClipRect::DrawVisual(int time)
{
	
	D3DXVECTOR2 v2[5];
	wxSize s = VideoSize;
	v2[0].x=Corner[0].x/wspw;
	v2[0].y=Corner[0].y/wsph;
	v2[1].x=v2[0].x;
	v2[1].y=(Corner[1].y/wsph)-1;
	v2[2].x=(Corner[1].x/wspw)-1;
	v2[2].y=v2[1].y;
	v2[3].x=v2[2].x;
	v2[3].y=v2[0].y;
	v2[4].x=v2[0].x;
	v2[4].y=v2[0].y;

	if(!invClip){
		
		
		VERTEX v24[12];
		CreateVERTEX(&v24[0],0, 0, 0x88000000);
		CreateVERTEX(&v24[1],s.x, 0, 0x88000000);
		CreateVERTEX(&v24[2],v2[2].x, v2[0].y, 0x88000000);
		CreateVERTEX(&v24[3],v2[0].x, v2[0].y, 0x88000000);
		CreateVERTEX(&v24[4],v2[0].x, v2[2].y, 0x88000000);
		CreateVERTEX(&v24[5],0, s.y, 0x88000000);
		CreateVERTEX(&v24[6],s.x, s.y, 0x88000000);
		CreateVERTEX(&v24[7],0, s.y, 0x88000000);
		CreateVERTEX(&v24[8],v2[0].x, v2[2].y, 0x88000000);
		CreateVERTEX(&v24[9],v2[2].x, v2[2].y, 0x88000000);
		CreateVERTEX(&v24[10],v2[2].x, v2[0].y, 0x88000000);
		CreateVERTEX(&v24[11],s.x, 0, 0x88000000);

		HRN(device->SetFVF(D3DFVF_XYZ|D3DFVF_DIFFUSE), "fvf failed");
		HRN(device->DrawPrimitiveUP( D3DPT_TRIANGLEFAN, 4, v24, sizeof(VERTEX) ),"primitive failed");
		HRN(device->DrawPrimitiveUP( D3DPT_TRIANGLEFAN, 4, &v24[6], sizeof(VERTEX) ),"primitive failed");
	}else{
		VERTEX v24[4];
		CreateVERTEX(&v24[0],v2[0].x, v2[0].y, 0x88000000);
		CreateVERTEX(&v24[1],v2[2].x, v2[0].y, 0x88000000);
		CreateVERTEX(&v24[2],v2[0].x, v2[2].y, 0x88000000);
		CreateVERTEX(&v24[3],v2[2].x, v2[2].y, 0x88000000);
		HRN(device->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, v24, sizeof(VERTEX) ),"primitive failed");
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

wxString ClipRect::GetVisual()
{
	return wxString::Format("\\%sclip(%i,%i,%i,%i)",(invClip)? "i" : "", Corner[0].x, Corner[0].y, Corner[1].x, Corner[1].y);
}

void ClipRect::OnMouseEvent(wxMouseEvent &evt)
{
	if(blockevents){return;}
	bool click = evt.LeftDown();
	bool holding = (evt.LeftIsDown());

	int x, y;
	if(tab->Video->isfullskreen){wxGetMousePosition(&x,&y);}
	else{evt.GetPosition(&x,&y);}

	if(evt.ButtonUp()){
		if(tab->Video->HasCapture()){tab->Video->ReleaseMouse();}
		SetVisual(GetVisual(),false,0);
		if(!hasArrow){tab->Video->SetCursor(wxCURSOR_ARROW);hasArrow=true;}
	}

	if(!holding){

		bool setarrow=false;
		for(int i = 0; i<2; i++){
			int pointx = Corner[i].x/wspw, pointy = Corner[i].y/wsph;
			if(abs(x-pointx)<5){
				setarrow=true;
				tab->Video->SetCursor(wxCURSOR_SIZEWE);
				hasArrow=false;break;}
			if(abs(y-pointy)<5){
				setarrow=true;
				tab->Video->SetCursor(wxCURSOR_SIZENS);
				hasArrow=false;break;
			}
		}
		if(!setarrow && !hasArrow){tab->Video->SetCursor(wxCURSOR_ARROW);hasArrow=true;}
	}
	if(click){
		grabbed=-1;
		for(int i = 0; i<2; i++){
			int pointx = Corner[i].x/wspw, pointy = Corner[i].y/wsph;
			if(abs(x-pointx)<5){
				diffs.x=(pointx)-x; grabbed=i;
				break;}
			if(abs(y-pointy)<5){
				diffs.y=(pointy)-y; grabbed=i+2;
				break;
			}
		}
	}else if(holding && grabbed!=-1){

		if(grabbed<2){
			x=MID(0,x,VideoSize.x);
			Corner[grabbed].x=((x+diffs.x)*wspw);
			if(grabbed==0 && Corner[0].x > Corner[1].x){Corner[0].x = Corner[1].x;}
			if(grabbed==1 && Corner[1].x < Corner[0].x){Corner[1].x = Corner[0].x;}	
		}else{
			y=MID(0,y,VideoSize.y);
			Corner[grabbed-2].y=((y+diffs.y)*wsph);
			if(grabbed==2 && Corner[0].y > Corner[1].y){Corner[0].y = Corner[1].y;}
			if(grabbed==3 && Corner[1].y < Corner[0].y){Corner[1].y = Corner[0].y;}	
		}
		SetVisual(GetVisual(),true,0);
	}


}

void ClipRect::SetCurVisual()
{
	int x1=0,x2=SubsSize.x,y1=0,y2=SubsSize.y;
	wxString clip;
	bool found =tab->Edit->FindVal("(i?clip[^\\)]+)", &clip);
	if(found){int rres = clip.Replace(",",",");
		if( rres >= 3){
			int match=1;
			wxRegEx re;
			if(rres>3){
				re.Compile("\\(([0-9-]+)[, ]*([0-9-]+)[, ]*([0-9-]+)[, ]*([0-9-]+)[, ]*([0-9-]+)", wxRE_ADVANCED);
				match=2;
			}else{
				re.Compile("\\(([0-9-]+)[, ]*([0-9-]+)[, ]*([0-9-]+)[, ]*([0-9-]+)", wxRE_ADVANCED);
			}
			if(re.Matches(clip)){
				x1=wxAtoi(re.GetMatch(clip,match));
				y1=wxAtoi(re.GetMatch(clip,match+1));
				x2=wxAtoi(re.GetMatch(clip,match+2));
				y2=wxAtoi(re.GetMatch(clip,match+3));
			}
		}
	}
	
	Corner[0] = wxPoint(x1, y1);
	Corner[1] = wxPoint(x2, y2);

}