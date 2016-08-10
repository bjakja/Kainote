
#include "Utils.h"

MyTokenizer::MyTokenizer(const wxString &TextToTokenize, const wxString &delimiters, int _flag)
	/*:text(TextToTokenize)
	,delims(delimiters)
	,flag(_flag)
	,pos(-1)
	,hasMoreTokens(true)*/
{
	text=TextToTokenize;
	//text+=" ";
	delims=delimiters;
	flag=_flag;
	pos=-1;
	lastDelim=0;
	hasMoreTokens=true;
}

wxString MyTokenizer::GetNextToken()
{
	if(!hasMoreTokens){
		return "";
	}
	pos++;
	int oldpos=pos;
	int len=text.Len();
	for(size_t i=pos; i< len ;i++){
		if(delims.Find(text[i]) != -1 || i>=len-1 ){
			if(flag&FLAG_NOEMPTY && i==pos){continue;}
			if(text[i]=='\\' && i<text.Len()-1 && (text[i+1]=='N' || text[i+1]=='h')){pos=i+1; return text.SubString(oldpos, (flag&FLAG_RET_DELIMS)? i+1 : i-1);}
			else{pos=i;}
			return text.SubString(oldpos, (flag&FLAG_RET_DELIMS)? i : i-1);
		}
	}
	hasMoreTokens=false;
	if((flag&FLAG_NOEMPTY || flag&FLAG_EMPTYEND) && delims.Find(text[text.Len()-1])){return "";}
	return text.Mid(pos);
}

wxString MyTokenizer::GetPrevToken()
{
	if(pos==0){
		hasMoreTokens=false;
		return "";
	}
	hasMoreTokens=true;
	wxString trimed = text.Mid(0,pos-1);
	for(int i=pos; i>=0; i--){
		if(delims.Find(trimed[i]) != -1){
			if(flag&FLAG_NOEMPTY && i==0){continue;}
			pos=i;
			return trimed.Left((flag&FLAG_RET_DELIMS)? i : i+1);
		}
	}
	hasMoreTokens=false;
	if((flag&FLAG_NOEMPTY || flag&FLAG_EMPTYEND) && delims.Find(trimed[0])){return "";}
	return trimed;
}

void MyTokenizer::SetPosition(int position)
{
	hasMoreTokens=true;
	pos=position;
}

int MyTokenizer::GetPosition()
{
	return pos;
}

bool MyTokenizer::HasMoreTokens()
{
	return hasMoreTokens;
}

size_t MyTokenizer::CountTokens()
{
	size_t tokensCount=0;
	for(size_t i=0; i<text.Len();i++){
		if(delims.Find(text[i]) != -1){
			tokensCount++;
		}
	}
	return tokensCount;
}