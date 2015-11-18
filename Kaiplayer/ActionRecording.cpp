
#if Arec
#include "ActionRecording.h"
#include "findreplace.h"
#include "stylestore.h"
#include "kainoteMain.h"
#include "ScriptInfo.h"

ActionsRec::ActionsRec(kainoteFrame *Kaif)
	{
	Kai=Kaif;
	actnum=0;
	Recording=false;
	}

ActionsRec::~ActionsRec()
	{
	actions.clear();
	}

void ActionsRec::StartRecording()
	{
	Recording=true;
	}

void ActionsRec::StopRecording()
	{
	Recording=false;
	actions.push_back(Action);
	}

void ActionsRec::PauseRecording()
	{
	Recording = !Recording;
	}

void ActionsRec::RecordFindR(findreplace FindR, int act)
	{
	if(act==RepAll){
		Action<<"[FindRep]\n";
		Action<<FindR.FindText->GetValue()<<"\n";
		Action<<FindR.RepText->GetValue()<<"\n";
		Action<<FindR.MatchCase->GetValue()<<"\n";
		Action<<FindR.RegEx->GetValue()<<"\n";
		Action<<FindR.StartLine->GetValue()<<"\n";
		Action<<FindR.EndLine->GetValue()<<"\n";
		wxString val=(FindR.RadioButton1->GetValue())? "0" : (FindR.RadioButton2->GetValue())? "1" :
			(FindR.RadioButton5->GetValue())? "2" : "3";
		Action<<val<<"\n";
		val=(FindR.RadioButton3->GetValue())? "0" : (FindR.RadioButton4->GetValue())? "1" : "2";
		Action<<val<<"\n";
		Action<<FindR.tcstyle->GetValue()<<"\n";
		}
	else if(act==Sellines){
		Action<<"[SelLines]\n";
		Action<<FindR.FindText->GetValue()<<"\n";
		Action<<FindR.MatchCase->GetValue()<<"\n";
		Action<<FindR.RegEx->GetValue()<<"\n";//Fdial
		Action<<FindR.Fdial->GetValue()<<"\n";
		Action<<FindR.Fcomm->GetValue()<<"\n";
		Action<<FindR.Selections->GetSelection()<<"\n";
		Action<<FindR.Actions->GetSelection()<<"\n";
		}

		actnum++;
	}

void ActionsRec::RecordStore(stylestore store, int act)
	{
	StyleList *slist = (act==StoreAss)?store.ListBox2 : store.ListBox1;
		wxArrayInt selnum;
		int kkk=slist->GetSelections(selnum);
		if(kkk<1){return;}
		Action<<"[Store"<<act<<"]\n";
		for(int i=0; i<kkk; i++)
			{
			Action<<slist->GetString(selnum[i])<<"\n";
			}
		
	}

void ActionsRec::RecordGrid()
	{


	}

void ActionsRec::RecordKai()
	{

	}

void ActionsRec::RecordSinfo(ScriptInfo Sinfo)
	{

	}

#endif