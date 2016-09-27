#include "Visuals.h"
#include "TabPanel.h"
#include <wx/regex.h>


enum {
	TAGPOS=1,
	TAGMOVES,
	TAGMOVEE=4,
	TAGORG=8,
	TAGCLIP=16,
	TAGP=32
};

MoveAll::MoveAll()
	: Visuals()
	, numElem(-1)
	, selectedTags(0)
{
}

void MoveAll::DrawVisual(int time)
{
	for(size_t i = 0; i <elems.size(); i++){
		//if(!(selectedTags & elems[i].type)){continue;}
		if( elems[i].type == TAGPOS || elems[i].type == TAGMOVES){
			DrawRect(elems[i].elem);
		}else if( elems[i].type == TAGMOVEE ){
			DrawCircle(elems[i].elem);
		}else{
			D3DXCOLOR col= (elems[i].type == TAGCLIP)? 0xFF0000FF : (elems[i].type == TAGP)? 0xFFFF00FF : 0xFF8800FF;
			DrawCross(elems[i].elem, col);
		}
	}
	
}

void MoveAll::OnMouseEvent(wxMouseEvent &evt)
{
	if(blockevents){return;}
	bool click = evt.LeftDown()||evt.RightDown()||evt.MiddleDown();
	bool holding = (evt.LeftIsDown()||evt.RightIsDown()||evt.MiddleIsDown());

	int x, y;
	if(tab->Video->isfullskreen){wxGetMousePosition(&x,&y);}
	else{evt.GetPosition(&x,&y);}

	if(evt.ButtonUp()){
		if(tab->Video->HasCapture()){tab->Video->ReleaseMouse();}
		if(numElem>=0){ChangeInLines(true);}
		if(!hasArrow){tab->Video->SetCursor(wxCURSOR_ARROW);hasArrow=true;}
		numElem=-1;
	}
	
	if(click){

		for(size_t i = 0; i <elems.size(); i++){
			//if(!(selectedTags & elems[i].type)){continue;}
			if(abs(elems[i].elem.x - x) < 8 && abs(elems[i].elem.y - y) < 8){
				numElem=i;
				beforeMove = lastmove = elems[i].elem;
				diffs.x=elems[i].elem.x-x;
				diffs.y=elems[i].elem.y-y;
				tab->Video->CaptureMouse();
			}
		}

	}else if(holding && numElem >= 0 ){
		elems[numElem].elem.x = x + diffs.x;
		elems[numElem].elem.y = y + diffs.y;
		ChangeInLines(false);
	}

}

void MoveAll::SetCurVisual()
{
	D3DXVECTOR2 linepos = GetPosnScale(NULL, NULL, tbl);
	if(tbl[6]>3){linepos=CalcMovePos();}
	from = to = D3DXVECTOR2(linepos.x/wspw,linepos.y/wsph);
	elems.clear();

	wxString res;
	if(tab->Edit->FindVal("org\\(([^\\)]+)", &res)){
		wxString rest;
		double orx,ory;
		
		moveElems elem;
		if(res.BeforeFirst(',',&rest).ToDouble(&orx)){elem.elem.x=orx/wspw;}
		if(rest.ToDouble(&ory)){elem.elem.y=ory/wsph;}
		elem.type=TAGORG;
		elems.push_back(elem);
	}
	if(tab->Edit->FindVal("(i?clip[^\\)]+)", &res)){
		wxRegEx re("m ([0-9.-]+) ([0-9.-]+)", wxRE_ADVANCED);
		moveElems elem;
		if(re.Matches(res)){
			elem.elem = D3DXVECTOR2(wxAtoi(re.GetMatch(res,1))/wspw, wxAtoi(re.GetMatch(res,2))/wsph);
		}else{
			wxString txt = tab->Edit->TextEdit->GetValue();
			int repl = txt.Replace(",", ",");
			wxRegEx re("\\(([0-9.-]+)[, ]*([0-9.-]+)[, ]*([0-9.-]+)", wxRE_ADVANCED);
			if(repl==3){
				elem.elem = D3DXVECTOR2(wxAtoi(re.GetMatch(res,1))/wspw, wxAtoi(re.GetMatch(res,2))/wsph);
			}else if(repl>3){
				elem.elem = D3DXVECTOR2(wxAtoi(re.GetMatch(res,2))/wspw, wxAtoi(re.GetMatch(res,3))/wsph);
			}
		}
		elem.type=TAGCLIP;
		elems.push_back(elem);
	}
	if(tab->Edit->FindVal("p([0-9]+)", &res)){
		res=tab->Edit->TextEdit->GetValue();
		wxRegEx re("} ?m ([.0-9-]+) ([.0-9-]+)", wxRE_ADVANCED);
		if(re.Matches(res)){
			moveElems elem;

			elem.elem= D3DXVECTOR2(wxAtoi(re.GetMatch(res,1))/wspw, wxAtoi(re.GetMatch(res,2))/wsph);
			elem.type=TAGP;
			elems.push_back(elem);
		}

	}
	if(tbl[6]==2){
		moveElems elem;
		elem.elem= D3DXVECTOR2(tbl[0] / wspw, tbl[1] / wsph);
		elem.type=TAGPOS;
		elems.push_back(elem);
	}
	if(tbl[6]>=4){
		moveElems elem;
		elem.elem= D3DXVECTOR2(tbl[0] / wspw, tbl[1] / wsph);
		elem.type=TAGMOVES;
		elems.push_back(elem);
		elem.type=TAGMOVEE;
		elem.elem= D3DXVECTOR2(tbl[2] / wspw, tbl[3] / wsph);
		elems.push_back(elem);
	}

}

wxString MoveAll::GetVisual()
{
	return "";
}

void MoveAll::ChangeInLines(bool all)
{
	D3DXVECTOR2 moving;
	moving = elems[numElem].elem - beforeMove;
	

	char type = elems[numElem].type;
	bool vector= type==TAGCLIP||type==TAGP;
	wxString delimiter= (vector)? " " : ",";
	wxString tmp;
	wxString origText=tab->Edit->TextEdit->GetValue();
	wxString tagpattern = (type==TAGPOS)? "pos\\(([^\\)]+)" : (type==TAGORG)? "org\\(([^\\)]+)" : (type==TAGCLIP)? "i?clip\\(([^\\)]+)" : (type==TAGP)? "p[0-9-]+[^}]*} ?m ([^{]+)" : "move\\(([^\\)]+)"; 
	wxArrayInt sels= tab->Grid1->GetSels();
	for(size_t i = 0; i< sels.size(); i++){
		wxString visual;
		wxString txt;
		if(all){
			Dialogue *Dial = tab->Grid1->GetDial(sels[i]);
			txt = (tab->Grid1->transl && Dial->TextTl!="")? Dial->TextTl : Dial->Text;
		}else{
			txt = origText;
		}

		wxRegEx re(tagpattern, wxRE_ADVANCED);
		size_t startMatch=0, lenMatch=0;
		if(re.Matches(txt)){
			//wxString tag=re.GetMatch(txt, 1); tag te¿ nigdzie nie jest potrzebny, bo wycinamy tylko jego wartoœæ.
			tmp= re.GetMatch(txt, 1);
			//re.GetMatch(&startMatch, &lenMatch, 2); niepotrzebny drugi raz u¿ycie tego samego
			wxStringTokenizer tkn(tmp, delimiter,wxTOKEN_STRTOK);
			int count=0;
			while(tkn.HasMoreTokens()){
				wxString token=tkn.GetNextToken().Trim().Trim(false);
				double val;
				if(token.ToDouble(&val)){
					if(count % 2 == 0){val += (moving.x * wspw);}else{val += (moving.y * wsph);}
					if(type==TAGMOVES && count > 1){visual+=token+delimiter; continue;}
					else if(type==TAGMOVEE && count != 2 && count != 3){visual+=token+delimiter; count++; continue;}
					if(vector){visual<<getfloat(val,"6.2f")<<delimiter;}
					else{visual += getfloat(val) + delimiter;}
					count++;
				}else{
					visual+=token+delimiter;
					if(!vector){count++;}
				}
			}
			if(re.GetMatch(&startMatch, &lenMatch, 1)){
				visual.RemoveLast();
				if(lenMatch){txt.erase(txt.begin()+startMatch, txt.begin()+startMatch+lenMatch);}
				txt.insert(startMatch,visual);

				if(all){
					tab->Grid1->CopyDial(sels[i])->Text=txt;
				}else{
					if(!dummytext){
						bool vis=false;
						dummytext= tab->Grid1->GetVisible(&vis,&dumplaced);
						if(!vis){SAFE_DELETE(dummytext); return;}
					}//else{
						
					//}
					dummytext->replace(dumplaced.x,dumplaced.y,txt);
					dumplaced.y = txt.Len();

					wxString *dtxt=new wxString(*dummytext);
					if(!tab->Video->OpenSubs(dtxt)){wxLogStatus(_("Nie mo¿na otworzyæ napisów"));}
					tab->Video->VisEdit=true;
					tab->Video->Render();
					break;
				}
			}
		
		}

	}
	if(all){
		//tab->Edit->TextEdit->Refresh(false);
		//tab->Edit->TextEdit->modified=true;tab->Edit->TextEdit->SetTextS(txt, true);
		tab->Video->VisEdit=true;
		if(tab->Edit->splittedTags){tab->Edit->TextEditTl->modified=true;}
		tab->Grid1->SetModified(true);
		tab->Grid1->Refresh();
	}

}

