
#include <wx/wx.h>

enum{
	DIAL,
	STYLE,
	SINFO
};

class Styles;
class Dialogue;
class SInfo;

class Entry
{
public:
	Entry();
	Entry *Copy();

	byte type;
	wxString Scomment;
};