
#pragma comment(lib, "libhunspell.lib")
#include "SpellChecker.h"

#include <wx/filename.h>
#include <wx/dir.h>
#include <wx/msgdlg.h>
#include <wx/log.h>
#include "OpennWrite.h"

SpellChecker *SpellChecker::SC = NULL;

SpellChecker::SpellChecker()
{
	hunspell = NULL;
	conv = NULL;
	SC=NULL;
}

SpellChecker *SpellChecker::Get()
{
	if(!SC){
		SC = new SpellChecker();
		bool isgood = SC->Initialize();
		if(!isgood){Options.SetBool("Editbox Spellchecker", false);}
	}
	return SC;
}
void SpellChecker::Destroy()
{
	if(SC){delete SC; SC=NULL;}
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

void SpellChecker::AvailableDics(wxArrayString &dics)
{
	wxArrayString dic;
	wxArrayString aff;
	wxString pathhh=Options.pathfull+"\\Dictionary";
	wxDir kat(pathhh);
	if(kat.IsOpened()){

		kat.GetAllFiles(pathhh,&dic,"*.dic",wxDIR_FILES);
		kat.GetAllFiles(pathhh,&aff,"*.aff",wxDIR_FILES);
	}

	for(size_t i=0; i<dic.size(); i++){
		if(dic[i].BeforeLast('.') == aff[i].BeforeLast('.')){
			dics.Add(dic[i].AfterLast('\\').BeforeFirst('.'));
		}
	}
}

bool SpellChecker::Initialize()
{
	Cleaning();

	wxString pathhh=Options.pathfull+"\\Dictionary\\";
	wxString name=Options.GetString("Dictionary Name");
	if(name==""){name="pl";}
	wxString dic=pathhh+name+".dic";
	wxString aff=pathhh+name+".aff";
	// Check if language is available
	if(!wxFileExists(dic) || !wxFileExists(aff)) 
	{
		Options.SetBool("Editbox Spellchecker",false);
		wxMessageBox(wxString::Format(_("Brak plików słownika w folderze \"%s\\Dictionary\".\r\nSprawdzanie pisowni zostanie wyłączone"), Options.pathfull)); 
		return false;
	}
	// Load
	hunspell = new Hunspell(aff.mb_str(wxConvLocal), dic.mb_str(wxConvLocal));
	
	if (hunspell) {
		conv = new wxCSConv(wxString(hunspell->get_dic_encoding(),wxConvUTF8));
		if(!conv){wxMessageBox(_("Nie można poprać formatu konwersji słownika."));}
		// Load user dictionary
		wxString userpath=pathhh+"UserDic.udic";
		if (wxFileExists(userpath)) {
			OpenWrite op;
			wxString txt=op.FileOpen(userpath,false);
			if (txt=="") {return true;}
			wxStringTokenizer textIn(txt,"\n");
			while (textIn.HasMoreTokens()) {
				// Read line
				wxString curLine = textIn.NextToken();
				curLine.Trim();
				if (curLine.IsEmpty() || curLine.IsNumber()) continue;

				hunspell->add(curLine.mb_str(*conv));

			}

		}

		return true;
	}else{wxMessageBox(_("Nie można zainicjalizować sprawdzania pisowni."));}
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

	int n=hunspell->suggest(&result,buf);

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
	wxString pathhh=Options.pathfull+"\\Dictionary\\UserDic.udic";
	OpenWrite ow;
	wxString txt=ow.FileOpen(pathhh,false);
	if(txt==""){txt=word;}
	else{txt+="\n"+word;}
	ow.FileWrite(pathhh,txt);

	return true;
}