#ifndef UTILS_H
#define UTILS_H

#include <wx/wx.h>

class MyTokenizer{
public:
	MyTokenizer(const wxString &TextToTokenize, const wxString &delimiters, int flag);
	wxString GetNextToken();
	wxString GetPrevToken();
	void SetPosition(int position);
	int GetPosition();
	bool HasMoreTokens();
	size_t CountTokens();

private:
	wxString text;
	wxString delims;
	int pos;
	int lastDelim;
	int flag;
	bool hasMoreTokens;
};

enum{
	FLAG_NOEMPTY=1,
	FLAG_EMPTYALL=2,
	FLAG_EMPTYEND=4,
	FLAG_RET_DELIMS=8

};

#endif // !UTILS_H

