
#include "Visuals.h"
#include "TabPanel.h"
#include <wx/regex.h>

Position::Position()
	: Visuals()
{
}


void Position::Draw(int time)
{
	//if(!(time>=start && time<=end)){blockevents = true; return;}else if(blockevents){blockevents=false;}
	wxMutexLocker lock(clipmutex);
	line->SetAntialias(TRUE);
	line->SetWidth(2.0);
	//wxLogStatus("Drawing");
	bool nothintoshow=true;
	for(auto pos : data ){
		if((time>=pos.dial->Start.mstime && time<=pos.dial->End.mstime)){
			//wxLogStatus("Drawing pos %f %f", pos.pos.x, pos.pos.y);
			DrawCross(pos.pos);
			DrawRect(pos.pos);
			nothintoshow=false;
		}
	}
	
	line->SetAntialias(FALSE);
	oldtime=time;
	if(nothintoshow){blockevents = true;}else if(blockevents){blockevents=false;}
}

void Position::OnMouseEvent(wxMouseEvent &evt)
{
	
	if(blockevents){return;}
	bool click = evt.LeftDown()||evt.RightDown()||evt.MiddleDown();
	bool holding = (evt.LeftIsDown()||evt.RightIsDown()||evt.MiddleIsDown());
	
	int x, y;
	if(tab->Video->isfullskreen){wxGetMousePosition(&x,&y);}
	else{evt.GetPosition(&x,&y);}
	
	if(evt.ButtonUp()){
		if(tab->Video->HasCapture()){tab->Video->ReleaseMouse();}
		//tab->Edit->SetVisual(GetVisual(),false,0);
		ChangeMultiline(true);
		if(!hasArrow){tab->Video->SetCursor(wxCURSOR_ARROW);hasArrow=true;}
	}

	if(click){
		tab->Video->SetCursor(wxCURSOR_SIZING);
		hasArrow=false;
		firstmove.x=x;
		firstmove.y=y;
		lastmove=from;
	}else if(holding){
		for(size_t i=0; i < data.size(); i++ ){
			data[i].pos.x = data[i].lastpos.x - (firstmove.x-x);
			data[i].pos.y = data[i].lastpos.y - (firstmove.y-y);
		}
		//tab->Edit->SetVisual(GetVisual(),true,0);
		ChangeMultiline(false);
		//lastmove=from;
	}
}
	
wxString Position::GetVisual()
{
	return "\\pos("+getfloat(from.x*wspw)+","+getfloat(from.y*wsph)+")";
}

wxString Position::GetVisual(int datapos)
{
	return "\\pos("+getfloat(data[datapos].pos.x * wspw)+","+getfloat(data[datapos].pos.y * wsph)+")";
}

	
void Position::SetCurVisual()
{

	//D3DXVECTOR2 linepos = tab->Edit->GetPosnScale(NULL, NULL, tbl);
	//from = D3DXVECTOR2(linepos.x/wspw,linepos.y/wsph);
	data.clear();
	wxArrayInt sels= tab->Grid1->GetSels();
	//wxLogStatus("Getpos size %i", sels.size());
	bool pib; wxPoint tp;
	for(size_t i = 0; i < sels.size(); i++){
		Dialogue *dial = tab->Grid1->GetDial(sels[i]);
		D3DXVECTOR2 pos = GetPos(dial,&pib,&tp);
		if(sels[i]==tab->Edit->ebrow){from = D3DXVECTOR2(pos.x/wspw, pos.y/wsph);}
		//wxLogStatus("Getpos %f %f %i %i %i", pos.x, pos.y, tp.x, tp.y, (int)pib);
		data.push_back(PosData(dial, D3DXVECTOR2(pos.x/wspw, pos.y/wsph), tp, pib));
	}
}

void Position::ChangeMultiline(bool all)
{
	//D3DXVECTOR2 moving = from - lastmove;
	
	wxString *dummyText;
	if(!all){
		bool visible=false; wxPoint point;
		dummyText = tab->Grid1->GetVisible(&point,&visible,false);
	}
	wxArrayInt sels= tab->Grid1->GetSels();
	for(size_t i = 0; i< sels.size(); i++){
		wxString visual = GetVisual(i);
		Dialogue *Dial = data[i].dial;
		bool istxttl = (tab->Grid1->transl && Dial->TextTl!="");
		wxString txt = (istxttl)? Dial->TextTl : Dial->Text;
		
		if(data[i].putinBracket){visual = "{" + visual + "}";}
		txt.replace(data[i].TextPos.x, data[i].TextPos.y, visual);

		if(all){
			if(istxttl){
				tab->Grid1->CopyDial(sels[i])->TextTl=txt;
			}else{
				tab->Grid1->CopyDial(sels[i])->Text=txt;
			}
		}else{
			Dialogue *dial = Dial->Copy();
			if(istxttl){dial->TextTl = txt;}else{dial->Text = txt;}
			(*dummyText)<<dial->GetRaw();
			delete dial;
		}

		//wxRegEx re(tagpattern, wxRE_ADVANCED);
		//size_t startMatch=0, lenMatch=0;
		//if(re.Matches(txt)){
		//	//wxString tag=re.GetMatch(txt, 1); tag te¿ nigdzie nie jest potrzebny, bo wycinamy tylko jego wartoœæ.
		//	tmp= re.GetMatch(txt, 1);
		//	//re.GetMatch(&startMatch, &lenMatch, 2); niepotrzebny drugi raz u¿ycie tego samego
		//	wxStringTokenizer tkn(tmp, delimiter,wxTOKEN_STRTOK);
		//	int count=0;
		//	while(tkn.HasMoreTokens()){
		//		wxString token=tkn.GetNextToken().Trim().Trim(false);
		//		double val;
		//		if(token.ToDouble(&val)){
		//			if(count % 2 == 0){val += (moving.x * wspw);}else{val += (moving.y * wsph);}
		//			visual += getfloat(val) + delimiter;
		//			count++;
		//		}else{
		//			visual+=token+delimiter;
		//		}
		//	}
		//	if(re.GetMatch(&startMatch, &lenMatch, 1)){
		//		visual.RemoveLast();
		//		if(lenMatch){txt.erase(txt.begin()+startMatch, txt.begin()+startMatch+lenMatch);}
		//		txt.insert(startMatch,visual);
		//		if(all){
		//			if(istxttl){
		//				tab->Grid1->CopyDial(sels[i])->TextTl=txt;
		//			}else{
		//				tab->Grid1->CopyDial(sels[i])->Text=txt;
		//			}
		//		}else{
		//			Dialogue *dial = Dial->Copy();
		//			if(istxttl){dial->TextTl = txt;}else{dial->Text = txt;}
		//			(*dummyText)<<dial->GetRaw();
		//			delete dial;
		//		}
		//	}
		//
		//}/*else{
		//	if(txt[0]=='{'){txt.replace(1,0,GetVisual());}
		//	else{txt.Prepend("{"+GetVisual()+"}");}
		//}*/

	}
	//if(!all){wxLogStatus("Text "+ *dummyText);}
	if(all){
		tab->Video->VisEdit=true;
		if(tab->Edit->splittedTags){tab->Edit->TextEditTl->modified=true;}
		tab->Grid1->SetModified(true);
		tab->Grid1->Refresh();
	}else{
		if(!tab->Video->OpenSubs(dummyText)){wxLogStatus(_("Nie mo¿na otworzyæ napisów"));}
		tab->Video->VisEdit=true;
		tab->Video->Render();
	}
	
}