#include "SubsDialogue.h"
#include "config.h"
#include <wx/tokenzr.h>
#include <wx/regex.h>
#include <wx/log.h>

Dialogue::Dialogue()
{
	Form=ASS;
	Layer=0;
	End.mstime=5000;
	Style=_T("Default");
	MarginL=0;
	MarginR=0;
	MarginV=0;
	State=0;
	NonDial=false;
	IsComment=false;
	//iterator++;
}

Dialogue::~Dialogue()
{
	//spells.clear();
	//iterator--;
}


Dialogue::Dialogue(wxString ldial,wxString txttl)
{
	TextTl=txttl;
	SetRaw(ldial);
	//iterator++;
}
void Dialogue::SetRaw(wxString ldial)
{
	State=0;
	ldial.Trim(false);

	if(ldial.StartsWith(_T("Dialogue"))||ldial.StartsWith(_T("Comment"))){
		wxStringTokenizer assdal(ldial,_T(","),wxTOKEN_RET_EMPTY_ALL);
		if(assdal.CountTokens()>=9){
			NonDial=false;
			wxString token=assdal.GetNextToken();
			if(token.StartsWith(_T("Dialogue"))){IsComment=false;}else{IsComment=true;}
			if(token.Find("arked=")==-1){Layer=wxAtoi(token.AfterFirst(' '));}
			else{Layer=wxAtoi(token.AfterLast('='));}
			Form=ASS;
			Start.SetRaw(assdal.GetNextToken(),Form);
			End.SetRaw(assdal.GetNextToken(),Form);
			Style=assdal.GetNextToken();
			Actor=assdal.GetNextToken();
			MarginL=wxAtoi(assdal.GetNextToken());
			MarginR=wxAtoi(assdal.GetNextToken());
			MarginV=wxAtoi(assdal.GetNextToken());
			Effect=assdal.GetNextToken();
			Text=ldial.Mid(assdal.GetPosition());
			Text.Trim(false);
			Text.Trim(true);
			return;
		}
	}

	wxRegEx expresion1( _T("^\\{([0-9-]+)\\}{([0-9-]*)\\}([^\r\n]*)") , wxRE_ADVANCED);
	wxRegEx expresion2( _T("^\\[([0-9-]+)\\]\\[([0-9-]*)\\]([^\r\n]*)") , wxRE_ADVANCED);
	wxRegEx expresion( _T("^([0-9]+)[:;]([0-9]+)[:;]([0-9]+)[:;, ]([^\r\n]*)") , wxRE_ADVANCED);

	Layer=0;
	MarginL=0;
	MarginR=0;
	MarginV=0;
	if(ldial.Find(" --> ")!=-1){
		NonDial=false;
		IsComment=false;
		Form=SRT;

		wxString eend;
		wxString ttext;
		Start.SetRaw(ldial.BeforeFirst(' ',&eend),Form);
		eend=eend.AfterFirst(' ');
		End.SetRaw(eend.BeforeFirst('\r',&ttext),Form);
		Text = ttext.AfterFirst('\n');
		Text.Replace("\r\n","\\N");
	}
	else if( expresion1.Matches( ldial ) )
	{
		NonDial=false;
		IsComment=false;
		Form=MDVD;
		Start.SetRaw(expresion1.GetMatch( ldial, 1 ),Form);
		End.SetRaw(expresion1.GetMatch( ldial, 2 ),Form);
		Text = expresion1.GetMatch( ldial, 3 );
		Text.Trim(false);
		return;
	}else if( expresion2.Matches( ldial ) )
	{
		NonDial=false;
		IsComment=false;
		Form=MPL2;
		Start.SetRaw(expresion2.GetMatch( ldial, 1 ),Form);
		End.SetRaw(expresion2.GetMatch( ldial, 2 ),Form);
		Text = expresion2.GetMatch( ldial, 3 );
		Text.Trim(false);
		return;
	}else if( expresion.Matches( ldial) )
	{
		NonDial=false;
		IsComment=false;
		Form=TMP;
		wxString timeparts;
		Start.SetRaw(timeparts<<expresion.GetMatch( ldial, 1 )<<_T(":")<<expresion.GetMatch( ldial, 2 )<<_T(":")<<expresion.GetMatch( ldial, 3 ),Form);
		Text = expresion.GetMatch( ldial, 4 );
		Text.Trim(false);
		return;
	}else if(ldial.StartsWith(_T(";"))||ldial.StartsWith(_T("{"))){NonDial=true;IsComment=true;Text=ldial;Text.Trim(true);Form=ASS;return;
	}
	else{
		Form=0;
		ldial.Trim(true);
		NonDial=false;IsComment=false;
		Style=_T("Default");
		Text=ldial;
		Text.Replace("\r\n","\\N");
	}

}

wxString Dialogue::GetRaw(bool tl, wxString style)
{
	wxString line;
	wxString txttl=(tl)?TextTl:Text;
	if (Form<SRT){
		if(IsComment){line=_T("Comment: ");}else{line=_T("Dialogue: ");};
		wxString Styletl=(style!="")?style:Style;
		line<<Layer<<_T(",")<<Start.raw()<<_T(",")
			<<End.raw()<<_T(",")<<Styletl<<_T(",")<<Actor<<_T(",")
			<<MarginL<<_T(",")
			<<MarginR<<_T(",")
			<<MarginV<<_T(",")
			<<Effect<<_T(",")
			<<txttl;
		//line+=wxString::Format("%i,%s,%s,%s,%s,%i,%i,%i,%s,%s",(int)Layer,Start.raw().data(),End.raw().data(),Styletl.data(),Actor.data(),(int)MarginL,(int)MarginR,(int)MarginV,Effect.data(),txttl.data());

	}else if(Form==MDVD){
		line<<_T("{")<<Start.raw()<<_T("}{")<<End.raw()<<_T("}")<<txttl;
	}
	else if(Form==MPL2){
		line<<_T("[")<<Start.raw()<<_T("][")<<End.raw()<<_T("]")<<txttl;
	}
	else if(Form==TMP){
		line<<Start.raw()<<_T(":")<<txttl;
	}
	else if(Form==SRT){
		txttl.Replace("\\N","\r\n");
		line<<Start.raw()<<" --> "<<End.raw()<<"\r\n"<<txttl<<"\r\n";
	}
	line<<_T("\r\n");
	return line;
}

wxString Dialogue::GetCols(int cols, bool tl, wxString style)
{

	wxString line;
	wxString txttl=(tl)?TextTl:Text;
	if(cols & 2048){
		wxRegEx reg(_T("\\{[^\\{]*\\}"),wxRE_ADVANCED);
		reg.ReplaceAll(&txttl,_T(""));
		cols |= 1024;
	}
	if (Form<SRT){
		wxString Styletl=(style!="")?style:Style;
		if(cols & 1){line<<Layer<<_T(",");}
		if(cols & 2){line<<Start.raw()<<_T(",");}
		if(cols & 4){line<<End.raw()<<_T(",");}
		if(cols & 8){line<<Styletl<<_T(",");}
		if(cols & 16){line<<Actor<<_T(",");}
		if(cols & 32){line<<MarginL<<_T(",");}
		if(cols & 64){line<<MarginR<<_T(",");}
		if(cols & 128){line<<MarginV<<_T(",");}
		if(cols & 256){line<<Effect<<_T(",");}
		if(cols & 1024){line<<txttl;}
		//line+=wxString::Format("%i,%s,%s,%s,%s,%i,%i,%i,%s,%s",(int)Layer,Start.raw().data(),End.raw().data(),Styletl.data(),Actor.data(),(int)MarginL,(int)MarginR,(int)MarginV,Effect.data(),txttl.data());

	}else if(Form==MDVD){
		if(cols & 2){line<<_T("{")<<Start.raw()<<"}";}
		if(cols & 4){line<<_T("{")<<End.raw()<<"}";}
		if(cols & 1024){line<<txttl;}
	}
	else if(Form==MPL2){
		if(cols & 2){line<<_T("[")<<Start.raw()<<"]";}
		if(cols & 4){line<<_T("[")<<End.raw()<<"]";}
		if(cols & 1024){line<<txttl;}
	}
	else if(Form==TMP){
		if(cols & 2){line<<Start.raw()<<":";}
		if(cols & 1024){line<<txttl;}
	}
	else if(Form==SRT){
		txttl.Replace("\\N","\r\n");
		line<<Start.raw()<<" --> "<<End.raw()<<"\r\n"<<txttl<<"\r\n";
		if(cols & 2){line<<Start.raw();}
		if(cols & 4){line<<" --> "<<End.raw()<<"\r\n";}
		if(cols & 1024){line<<txttl<<"\r\n";}
	}

	line<<_T("\r\n");
	return line;
}

void Dialogue::Conv(char type,wxString pref)
{
	if(!Form){Form=0;if(type==ASS){return;}}
	if(Form == TMP && End.mstime==0){End=Start;End.mstime+=2000;}
	Start.ChangeFormat(type);
	End.ChangeFormat(type);
	if (type<=SRT){
		Layer=0;
		Style=Options.GetString(_T("Default Style"));
		Actor=_T("");
		MarginL=0;
		MarginR=0;
		MarginV=0;
		Effect=_T("");
		wxString tmp=Text;
		if(Form!=SRT){
			wxRegEx regib(_T("\\{y[:+]([ib])\\}"),wxRE_ADVANCED);
			wxRegEx reg(_T("\\{[^\\\\]([^}]*)\\}"),wxRE_ADVANCED);
			reg.ReplaceAll(&tmp,_T(""));
			wxString ital;
			if(type!=SRT){
				regib.ReplaceAll(&tmp,_T("{\\\\\\1\t1}"));
				tmp.Replace("\t","");
				ital=_T("{\\i1}");
			}
			else{
				regib.ReplaceAll(&tmp,_T("<\\1>"));
				ital="<i>";
			}
			tmp.Replace(_T("|"),_T("\\N"));
			size_t il=tmp.Replace(_T("/"),_T(""));
			if(type<SRT){Text= pref;}else{Text="";}
			if(il>0){Text<<ital;}
			Text<<tmp;
			Text.Replace("}{","");
		}else{
			wxRegEx regibu(_T("\\<([ibu])\\>"),wxRE_ADVANCED);
			wxRegEx regibu0(_T("\\</([ibu])\\>"),wxRE_ADVANCED);
			regibu.ReplaceAll(&tmp,"{\\\\\\1\t1}");
			regibu0.ReplaceAll(&tmp,_T("{\\\\\\1\t0}"));
			tmp.Replace("\t","");
			Text=pref+tmp;
		}

	}else if(Form<SRT){
		wxString tmp=Text;
		tmp.Replace(_T("\\h"),_T(" "));
		if(type==SRT){
			wxRegEx regibu(_T("\\{\\\\([ibu])\\1}"),wxRE_ADVANCED);
			wxRegEx regibu0(_T("\\{\\\\([ibu])\\0}"),wxRE_ADVANCED);
			regibu.ReplaceAll(&tmp,_T("<\\1>"));
			regibu0.ReplaceAll(&tmp,_T("</\\1>"));
		}
		wxRegEx reg(_T("\\{[^}]*\\}"),wxRE_ADVANCED);
		reg.ReplaceAll(&tmp,_T(""));
		if(type!=SRT){
			tmp.Replace(_T("\\N"),_T("|"));}
		Text=tmp;
	}

	Form=type;
}

Dialogue *Dialogue::Copy(bool keepstate)
{
	Dialogue *dial =new Dialogue();
	//if(!dial){return NULL;}
	dial->Actor=Actor;
	dial->Effect=Effect;
	dial->End=End;
	dial->Form=Form;
	dial->IsComment=IsComment;
	dial->Layer=Layer;
	dial->MarginL=MarginL;
	dial->MarginR=MarginR;
	dial->MarginV=MarginV;
	dial->NonDial=NonDial;
	dial->Start=Start;
	dial->State= (keepstate) ? State : 1;
	dial->Style=Style;
	dial->Text=Text;
	dial->TextTl=TextTl;
	//dial->Scomment=Scomment;
	return dial;
}

