#ifndef SPELLCHECK
#define SPELLCHECK

#include "Config.h"
#include <hunspell.hxx>
#include <wx/string.h>



class SpellChecker
{
public:

	SpellChecker();
	~SpellChecker();
	bool Initialize();
	static void AvailableDics(wxArrayString &dics);
	void Cleaning();
	bool CheckWord(wxString word);
	bool AddWord(wxString word);
	wxArrayString Suggestions(wxString word);
	static SpellChecker *Get();
	static void Destroy();

private:
	Hunspell *hunspell;
	wxCSConv *conv;
	static SpellChecker *SC;

};



#endif