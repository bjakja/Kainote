//  Copyright (c) 2016, Marcin Drob

//  Kainote is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.

//  Kainote is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.

//  You should have received a copy of the GNU General Public License
//  along with Kainote.  If not, see <http://www.gnu.org/licenses/>.

#include "SubsDialogue.h"
#include "config.h"
#include <wx/tokenzr.h>
#include <wx/regex.h>
#include <wx/log.h>
#include <map>
#include <iostream>


TagData::TagData(const wxString &name, unsigned int _startTextPos)
{
	tagName = name;
	startTextPos = _startTextPos;
}
	
void TagData::PutValue(const wxString &_value, bool _multiValue)
{
	value = _value;
	multiValue = _multiValue;
}


ParseData::ParseData()
{
}

void ParseData::AddData(TagData *data)
{
	tags.push_back(data);
}
	
ParseData::~ParseData()
{
	for(auto it = tags.begin(); it != tags.end(); it++){
		delete (*it);
	}
	tags.clear();
}

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
	pdata = NULL;
}

Dialogue::~Dialogue()
{
	ClearParse();
}

void Dialogue::ClearParse()
{
	if(pdata){delete pdata; pdata=NULL;}
}

Dialogue::Dialogue(const wxString &ldial,const wxString &txttl)
{
	pdata = NULL;
	TextTl=txttl;
	SetRaw(ldial);
}
void Dialogue::SetRaw(const wxString &ldial)
{
	State=0;
	//ldial.Trim(false);

	if(ldial.StartsWith("Dialogue")||ldial.StartsWith("Comment")){
		wxStringTokenizer assdal(ldial,_T(","),wxTOKEN_RET_EMPTY_ALL);
		if(assdal.CountTokens()>=9){
			NonDial=false;
			wxString token=assdal.GetNextToken();
			if(token.StartsWith("Dialogue")){IsComment=false;}else{IsComment=true;}
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
		wxString eend;
		wxString ttext;
		Form=SRT;
		Start.SetRaw(ldial.BeforeFirst(' ',&eend),Form);
		eend=eend.AfterFirst(' ');
		End.SetRaw(eend.BeforeFirst('\r',&ttext),Form);
		Text = ttext.AfterFirst('\n');
		Text.Replace("\r","");
		Text.Replace("\n","\\N");
		NonDial=false;
		IsComment=false;
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
	}else if(ldial.StartsWith(";")||(ldial.StartsWith("{") && ldial.EndsWith("}"))){
		NonDial=true;
		IsComment=true;
		Style="Default";
		Text=ldial;
		Text.Trim(true);
		Form=ASS;
		return;
	}
	else{
		Form=0;
		NonDial=false;
		IsComment=false;
		Style="Default";
		Text=ldial;
		Text.Replace("\r","");
		Text.Replace("\n","\\N");
		Text.Trim(true);
	}

}

void Dialogue::GetRaw(wxString *txt, bool tl, const wxString &style)
{
	wxString line;
	if (Form<SRT){
		if(IsComment){line=_T("Comment: ");}else{line=_T("Dialogue: ");}
		bool styleTl = style!="";
		const wxString &Styletl=(styleTl)?style : Style;
		const wxString &EffectTl = (State & 4 && styleTl)? "\fD" : Effect;
		line<<Layer<<_T(",")<<Start.raw(Form)<<_T(",")
			<<End.raw(Form)<<_T(",")<<Styletl<<_T(",")<<Actor<<_T(",")
			<<MarginL<<_T(",")
			<<MarginR<<_T(",")
			<<MarginV<<_T(",")
			<<EffectTl<<_T(",");
			line += (tl)?TextTl : Text;
		//line+=wxString::Format("%i,%s,%s,%s,%s,%i,%i,%i,%s,%s",(int)Layer,Start.raw().data(),End.raw().data(),Styletl.data(),Actor.data(),(int)MarginL,(int)MarginR,(int)MarginV,Effect.data(),txttl.data());

	}else if(Form==MDVD){
		line<<_T("{")<<Start.raw(Form)<<_T("}{")<<End.raw(Form)<<_T("}")<<Text;
	}
	else if(Form==MPL2){
		line<<_T("[")<<Start.raw(Form)<<_T("][")<<End.raw(Form)<<_T("]")<<Text;
	}
	else if(Form==TMP){
		line<<Start.raw(Form)<<_T(":")<<Text;
	}
	else if(Form==SRT){
		wxString txt = Text;
		txt.Replace("\\N","\r\n");
		line<<Start.raw(Form)<<" --> "<<End.raw(Form)<<"\r\n"<<txt<<"\r\n";
	}
	line<<_T("\r\n");
	(*txt)<<line;
}

wxString Dialogue::GetCols(int cols, bool tl, const wxString &style)
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

void Dialogue::Conv(char type, const wxString &pref)
{
	if(!Form){Form=0;if(type==ASS){return;}}
	if(Form == TMP && End.mstime==0){End=Start;End.mstime+=2000;}
	Start.ChangeFormat(type);
	End.ChangeFormat(type);
	if (type<SRT){
		Layer=0;
		Style=Options.GetString(ConvertStyle);
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
		wxRegEx regp(_T("\\\\p[0-9]+"),wxRE_ADVANCED);
		if(regp.Matches(tmp)){Text="";Form=type; return;}
		if(type==SRT){
			wxRegEx regibu(_T("\\\\([ibu])1"),wxRE_ADVANCED);
			wxRegEx regibu0(_T("\\\\([ibu])0"),wxRE_ADVANCED);
			regibu.ReplaceAll(&tmp,_T("}<\\1>{"));
			regibu0.ReplaceAll(&tmp,_T("}</\\1>{"));
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
	dial->State= (keepstate) ? State : 1 + (State & 4);
	dial->Style=Style;
	dial->Text=Text;
	dial->TextTl=TextTl;
	//dial->Scomment=Scomment;
	dial->pdata=NULL;
	return dial;
}

//Remember parse patterns need "tag1|tag2|..." whithout slashes.
//Remember string position is start of the value, position of tag -=tagname.len+1
void Dialogue::ParseTags(wxString *tags, size_t ntags, bool plainText)
{
	if(pdata){return;}
	wxString txt = (TextTl != "")? TextTl : Text;
	size_t pos=0;
	size_t plainStart=0;
	bool hasDrawing=false;
	size_t len = txt.Len();
	bool tagsBlock = false;
	pdata = new ParseData();
	if(len<1){return;}
	while(pos < len){
		wxUniChar ch=txt[pos];
		if(ch=='}'){tagsBlock=false;plainStart=pos+1;}
		else if(ch=='{' || pos >= len-1){
			tagsBlock=true;
			if(pos >= len-1){pos++;}
			//aby nie skraszowaæ programu odejmuj¹c od 0 przy unsigned dodam 1 do plain start
			if((plainText || hasDrawing ) && plainStart+1 <= pos){
				TagData *newTag = new TagData((hasDrawing)? "p" : "plain", plainStart);
				newTag->PutValue(txt.SubString(plainStart,pos-1));
				pdata->AddData(newTag);
			}
		}else if(tagsBlock && ch=='\\'){
			pos ++;
			//wxString tag;
			int slashPos = txt.find('\\', pos);
			int bracketPos = txt.find('}', pos);
			int tagEnd = (slashPos == -1 && bracketPos == -1)? len :
				(slashPos == -1)? bracketPos : (bracketPos == -1)? slashPos : 
				(bracketPos < slashPos)? bracketPos : slashPos;
			wxString tag = txt.SubString(pos, tagEnd - 1);
			if(tag.EndsWith(')')){tag.RemoveLast();}
			for(size_t i = 0; i < ntags; i++){
				wxString tagName = tags[i];
				int tagLen = tagName.Len();
				if(tag.StartsWith(tagName) && (tag[tagLen] == '(' || 
					wxString(tag[tagLen]).IsNumber() || tagName == "fn")){

					TagData *newTag = new TagData(tagName, pos+tagLen);
					wxString tagValue = tag.Mid(tagLen);
					if(tagName == "p"){
						hasDrawing = (tagValue.Trim().Trim(false) == "0")? false : true;
						newTag->PutValue(tagValue);
					}else if(tag[tagLen] == '('){
						newTag->startTextPos++;
						newTag->PutValue(tagValue.After('(').BeforeFirst(')'), true);
					}else{
						newTag->PutValue(tagValue);
					}
					pdata->AddData(newTag);
					pos = tagEnd - 1;
					break;
				}
			}
		}
		pos ++;
	}
}
//adding this time
void Dialogue::ChangeTimes(int start, int end)
{
	wxString tags[] = {"move","t","fad"};
	ParseTags(tags,3);/*|fade*/
	size_t replaceMismatch = 0;
	for(size_t i = 0; i < pdata->tags.size(); i++){
		TagData *tdata = pdata->tags[i];
		wxStringTokenizer splitValues(tdata->value,",",wxTOKEN_STRTOK);
		int tokenCount = splitValues.CountTokens();
		if(tdata->tagName == "move" && tokenCount < 5 || 
			tdata->tagName == "t" && tokenCount < 2 || 
			tdata->tagName == "fad" && tokenCount < 1){
				continue;
		}
		int numToken = 0;
		while(splitValues.HasMoreTokens()){
			wxString token = splitValues.GetNextToken();
			if(tdata->tagName == "move" && numToken < 4){
				numToken++;
				continue;
			}
			size_t t1Len = token.Len();
			
			int t1 = wxAtoi(token);
			size_t t1Pos = splitValues.GetPosition() + tdata->startTextPos - t1Len - replaceMismatch - 1;
			token = splitValues.GetNextToken();
			size_t t2Len = token.Len();
			
			size_t totalLen = t1Len + t2Len + 1;
			int t2 = wxAtoi(token);
			t1 = MAX(0, t1 + start);
			t2 = MAX(0, t2 + end);

			wxString timesString;
			timesString << t1 << "," << t2;
			replaceMismatch += totalLen - timesString.Len();
			if(TextTl != ""){
				TextTl.replace(t1Pos, totalLen, timesString);
			}else{
				Text.replace(t1Pos, totalLen, timesString);
			}
			break;
		}
	}
}