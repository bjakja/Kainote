#ifndef SUBSDIALOGUE_H_INCLUDED
#define SUBSDIALOGUE_H_INCLUDED

#include "timeconv.h"
#include <wx/colour.h>
#include <vector>

class Dialogue
{

public:
	wxString Style, Actor, Effect, Text, TextTl;
	//wxString Scomment;
	STime Start, End;
	int Layer;
	short MarginL, MarginR, MarginV;
	char State, Form;
	bool NonDial, IsComment;

	void SetRaw(wxString ldial);
	wxString GetRaw(bool tl=false,wxString style="");
	wxString GetCols(int cols, bool tl=false,wxString style="");
	void Conv(char type,wxString pref="");
	Dialogue *Copy(bool keepstate=false);
	
	Dialogue();
	Dialogue(wxString ldial,wxString txttl="");
	~Dialogue();
};

#endif // SUBSDIALOGUE_H_INCLUDED
