
#pragma comment(lib, "libhunspell.lib")
#include "SpellChecker.h"

#include <wx/filename.h>
#include <wx/dir.h>
#include <wx/msgdlg.h>
#include "OpennWrite.h"


SpellChecker::SpellChecker()
	{
hunspell = NULL;
	conv = NULL;

	}

SpellChecker::~SpellChecker()
	{
	Cleaning();
	}
void SpellChecker::Cleaning()
	{
	if(hunspell){delete hunspell;hunspell=NULL;}
	if(conv){delete conv;conv=NULL;}

	}

bool SpellChecker::Initialize()
{//if(hunspell) return true;
	Cleaning();
	
	wxArrayString dic;
	wxArrayString aff;
	wxString pathhh=Options.pathfull+_T("\\Slownik");
	wxDir kat(pathhh);
          if(kat.IsOpened()){
          
	kat.GetAllFiles(pathhh,&dic,_T("*.dic"),wxDIR_FILES);
	kat.GetAllFiles(pathhh,&aff,_T("*.aff"),wxDIR_FILES);
    }
	
	// Check if language is available
		  if(dic.size()<1&&aff.size()<1) 
			  {Options.SetBool("Editbox Spellchecker",false);
		  wxMessageBox(_T("plików s³ownika nie ma w folderze \"")+Options.pathfull+_T("\\Slownik\",\r\nsprawdzanie pisowni zostanie wy³¹czone")); return false;}
		  //wxMessageBox(_T("spellchecker if exist ")+aff[0]+_T(" ")+dic[0]);
	//if (!wxFileExists(aff[0]) || !wxFileExists(dic[0])) wxMessageBox("spellchecker nie znalaz³ s³ownika"); return false;

	// Load
	hunspell = new Hunspell(aff[0].mb_str(wxConvLocal),dic[0].mb_str(wxConvLocal));
	//wxMessageBox("spellchecker new");
	//conv = NULL;
	if (hunspell) {
		//wxMessageBox("spellchecker konw");
		conv = new wxCSConv(wxString(hunspell->get_dic_encoding(),wxConvUTF8));
		// Load user dictionary
		wxString userpath=pathhh+"\\UserDic.udic";
		if (wxFileExists(userpath)) {
			OpenWrite op;
			wxString txt=op.FileOpen(userpath,false);
			if (txt=="") {return true;}
			wxStringTokenizer textIn(txt,_T("\n"));
			while (textIn.HasMoreTokens()) {
				// Read line
				wxString curLine = textIn.NextToken();
				curLine.Trim();
				if (curLine.IsEmpty() || curLine.IsNumber()) continue;

				hunspell->add(curLine.mb_str(*conv));

			}
			
		}

		return true;
		}else{wxMessageBox(_T("spellchecker siê nie zinicjalizowa³1"));}
	return false;
}

bool SpellChecker::CheckWord(wxString word) {
	if (!hunspell) return true;

	wxCharBuffer buf = word.mb_str(*conv);
	if (buf) return (hunspell->spell(buf) == 1);
	
	return false;
}

wxArrayString SpellChecker::Suggestions(wxString word)
	{
	wxArrayString Results;
	if(!hunspell) return Results;

	char **result;

	wxCharBuffer buf = word.mb_str(*conv);
	if (!buf) return Results;

	//wxMessageBox("funkcja suggest");
	int n=hunspell->suggest(&result,buf);

	//wxMessageBox(wxString::Format("%i sugestii",n));

	for(int i=0; i<n; ++i)
		{
		
		wxString cur(result[i],*conv);
		Results.Add(cur);
		}
	
	hunspell->free_list(&result, n);
	
	return Results;
	}

bool SpellChecker::AddWord(wxString word)
{
	if (word.IsEmpty() || word.IsNumber()) return false;

	hunspell->add(word.mb_str(*conv));
	wxString pathhh=Options.pathfull+_T("\\Slownik\\UserDic.udic");
	OpenWrite ow;
	wxString txt=ow.FileOpen(pathhh,false);
	if(txt==""){txt=word;}
	else{txt+="\n"+word;}
	ow.FileWrite(pathhh,txt);

	return true;
}