
#include "wx/wx.h"
#include "Menu.h"

class KaiChoice :public wxWindow
{
public:
	KaiChoice(wxWindow *parent, int id, const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize, int n = 0, const wxString choices[] = NULL,
        long style = 0, const wxValidator& validator = wxDefaultValidator);

	KaiChoice(wxWindow *parent, int id, const wxPoint& pos,
        const wxSize& size, const wxArrayString &choices,
        long style = 0, const wxValidator& validator = wxDefaultValidator);

	virtual ~KaiChoice();
	void SetSelection(int sel);
	void Clear();
	void Append(wxString what);
	/*void Prepend(wxString what);
	void Insert(wxString what, int position);*/
	wxString GetString(int pos){return (*list)[pos];};
	int GetSelection(){return choice;};
	void Select(int sel){choice=sel;}
private:
	void OnSize(wxSizeEvent& event);
	void OnPaint(wxPaintEvent& event);
	void OnMouseEvent(wxMouseEvent &event);
	void OnKeyPress(wxKeyEvent &event);
	void ShowList();
	void SetBitmap(wxBitmap newbmp){ actualBmp=newbmp; Refresh(false);}

	bool enter;
	wxArrayString *list;
	wxBitmap tmpbmp;
	wxBitmap actualBmp;
	static wxBitmap normal;
	static wxBitmap pushed;
	wxBitmap *bmp;
	Menu *listMenu;
	int choice;
	bool listIsShown;
	bool choiceChanged;
};
