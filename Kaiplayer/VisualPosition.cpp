
#include "Visuals.h"
#include "TabPanel.h"
#include <wx/regex.h>

Position::Position()
	: Visuals()
{
}


void Position::Draw(int time)
{
	wxMutexLocker lock(clipmutex);
	line->SetAntialias(TRUE);
	line->SetWidth(2.0);
	//wxLogStatus("Drawing");
	bool nothintoshow=true;
	for(auto pos : data ){
		if(time >= pos.dial->Start.mstime && time <= pos.dial->End.mstime){
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
	bool click = evt.LeftDown();
	bool holding = evt.LeftIsDown();
	
	int x, y;
	if(tab->Video->isfullskreen){wxGetMousePosition(&x,&y);}
	else{evt.GetPosition(&x,&y);}
	
	if(evt.RightDown() || evt.LeftDClick()){
		for(size_t i = 0; i < data.size(); i++){
			if(data[i].numpos == tab->Edit->ebrow){
				data[i].pos.x = x;
				data[i].pos.y = y;
				data[i].lastpos = data[i].pos;
				D3DXVECTOR2 diff(data[i].pos.x - data[i].lastpos.x, data[i].pos.y - data[i].lastpos.y);

				for(size_t j = 0; j < data.size(); j++){
					if(j==i){continue;}
					data[j].pos += diff;
					data[j].lastpos = data[j].pos;
				}
				ChangeMultiline(evt.RightDown());
				break;
			}

		}
		return;
	}

	if(evt.LeftUp()){
		if(tab->Video->HasCapture()){tab->Video->ReleaseMouse();}
		//tab->Edit->SetVisual(GetVisual(),false,0);
		ChangeMultiline(true);
		for(size_t i=0; i < data.size(); i++ ){
			data[i].lastpos = data[i].pos;
		}
		if(!hasArrow){tab->Video->SetCursor(wxCURSOR_ARROW);hasArrow=true;}
	}

	if(click){
		tab->Video->SetCursor(wxCURSOR_SIZING);
		hasArrow=false;
		wxArrayInt sels= tab->Grid1->GetSels();
		if(sels.size()!=data.size()){SetCurVisual();tab->Video->Render();}
		firstmove.x=x;
		firstmove.y=y;
		//lastmove=from;
	}else if(holding){
		for(size_t i=0; i < data.size(); i++ ){
			data[i].pos.x = data[i].lastpos.x - (firstmove.x-x);
			data[i].pos.y = data[i].lastpos.y - (firstmove.y-y);
			if(evt.ShiftDown()){
				if(abs(data[i].pos.x - data[i].lastpos.x)<15){
					data[i].pos.x = data[i].lastpos.x;
				}
				if(abs(data[i].pos.y - data[i].lastpos.y)<15){
					data[i].pos.y = data[i].lastpos.y;
				}
			}
		}
		ChangeMultiline(false);
	}
}
	

wxString Position::GetVisual(int datapos)
{
	return "\\pos("+getfloat(data[datapos].pos.x * wspw)+","+getfloat(data[datapos].pos.y * wsph)+")";
}

	
void Position::SetCurVisual()
{
	data.clear();
	wxArrayInt sels= tab->Grid1->GetSels();
	//wxLogStatus("Getpos size %i", sels.size());
	bool pib; wxPoint tp;
	for(size_t i = 0; i < sels.size(); i++){
		//fix by uzyskaæ reakcjê na edycjê w editboxie
		Dialogue *dial = (sels[i]==tab->Edit->ebrow)? tab->Edit->line : tab->Grid1->GetDial(sels[i]);
		D3DXVECTOR2 pos = GetPos(dial,&pib,&tp);
		data.push_back(PosData(dial, sels[i], D3DXVECTOR2(pos.x/wspw, pos.y/wsph), tp, pib));
	}
}

void Position::ChangeMultiline(bool all)
{
	//wxArrayInt sels= tab->Grid1->GetSels();
	wxString *dtxt;
	if(!all && !dummytext){
		bool visible=false; 
		dummytext = tab->Grid1->GetVisible(&visible,0,true);
		//wxLogStatus("dummytext "+ *dummytext);
	}
	if(!all){ dtxt=new wxString(*dummytext);}
	int _time = tab->Video->Tell();
	for(size_t i = 0; i< data.size(); i++){
		
		Dialogue *Dial = data[i].dial;
		if(!all && !(_time >= Dial->Start.mstime && _time <= Dial->End.mstime)){continue;}
		wxString visual = GetVisual(i);
		
		bool istxttl = (tab->Grid1->transl && Dial->TextTl!="");
		wxString txt = (istxttl)? Dial->TextTl : Dial->Text;
		
		if(data[i].putinBracket){visual = "{" + visual + "}";}
		txt.replace(data[i].TextPos.x, data[i].TextPos.y, visual);
		//wxLogStatus("text "+txt);
		if(all){
			if(istxttl){
				tab->Grid1->CopyDial(data[i].numpos)->TextTl=txt;
			}else{
				tab->Grid1->CopyDial(data[i].numpos)->Text=txt;
			}
		}else{
			Dialogue Cpy=Dialogue(*Dial);
			if(istxttl) {
				Cpy.TextTl = txt;
				(*dtxt)<<Cpy.GetRaw(true);
				(*dtxt)<<Cpy.GetRaw(false,tab->Grid1->GetSInfo("TLMode Style"));
			}else{
				Cpy.Text = txt;
				(*dtxt)<<Cpy.GetRaw();
			}
		}


	}
	//if(!all){wxLogStatus("Text "+ *dtxt);}
	if(all){
		tab->Video->VisEdit=true;
		if(tab->Edit->splittedTags){tab->Edit->TextEditTl->modified=true;}
		tab->Grid1->SetModified(true);
		tab->Grid1->Refresh();
	}else{
		
		if(!tab->Video->OpenSubs(dtxt)){wxLogStatus(_("Nie mo¿na otworzyæ napisów"));}
		tab->Video->VisEdit=true;
		tab->Video->Render();
	}
	
}