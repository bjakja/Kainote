#ifndef LOCALIZATION
#define LOCALIZATION
#include <wx/wx.h>
#include <wx/intl.h>

class Locale{
public:
	Locale(int language);
	~Locale();


private:
	wxLocale *locale;

};

#endif