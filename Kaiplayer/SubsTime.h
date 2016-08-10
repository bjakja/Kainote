#ifndef TIMECONV_H_INCLUDED
#define TIMECONV_H_INCLUDED
#include <wx/string.h>
#include <wx/thread.h>


class STime
{
private:

    char form;
    
public:
	int orgframe;
	//wxString raw;
	int mstime;

	STime();
	STime(int ms);
	~STime();
	void SetRaw(wxString rawtime, char format);
	void NewTime(int ms);
	void ParseMS(wxString time);
	wxString raw(char format=0);//, float fps=0
	char GetFormat();
	void ChangeFormat(char format,float fps=0);
	wxString GetFormatted(char format);
	void Change(int ms);
	bool operator> (STime por);
	bool operator< (STime por);
	bool operator>= (STime por);
	bool operator<= (STime por);
	bool operator== (STime por);
	STime operator- (STime por);
};







#endif // TIMECONV_H_INCLUDED
