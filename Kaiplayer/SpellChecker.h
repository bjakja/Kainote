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
	void AvailableDics(wxArrayString &dics);
	void Cleaning();
	bool CheckWord(wxString word);
	bool AddWord(wxString word);
	wxArrayString Suggestions(wxString word);

	private:
	Hunspell *hunspell;
	wxCSConv *conv;


	};



#endif