#ifndef GRID_H
#define GRID_H

#include "SubsGrid.h"


class kainoteFrame;


class Grid: public SubsGrid
{
public:

	Grid(wxWindow* parent, kainoteFrame* kfparent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize, long style=0, const wxString& name=wxPanelNameStr);
	virtual ~Grid();
	void MoveTextTL(char mode);
	void ResizeSubs(float xnsize, float ynsize);
	void OnMkvSubs(wxCommandEvent &event);
	void ConnectAcc(int id);
	void OnAccelerator(wxCommandEvent &event);
	void OnJoin(wxCommandEvent &event);
	void ContextMenu(const wxPoint &pos, bool dummy=false);
protected:

    wxArrayInt selarr;

private:
	void CopyRows(int id);
    void OnInsertBefore();
	void OnInsertAfter();
	void OnDuplicate();
	void OnJoinF(int id);
	void OnPaste(int id);
	void OnPasteTextTl();
	void OnJoinToFirst(int id);
	void OnInsertBeforeVideo();
	void OnInsertAfterVideo();
	void OnSetFPSFromVideo();
	void OnSetNewFPS();
	void OnMakeContinous(int id);

	wxMenu *hidemenu;

	DECLARE_EVENT_TABLE()
};

//enum {
//	MENU_INSERT_BEFORE=5555,
//	MENU_INSERT_AFTER,
//	MENU_INSERT_BEFORE_VIDEO,
//	MENU_INSERT_AFTER_VIDEO,
//	MENU_SWAP,
//	MENU_DUPLICATE,
//	MENU_JOIN,
//	MENU_JOINF,
//	MENU_JOINL,
//	MENU_COPY,
//	MENU_PASTE,
//	MENU_CUT,
//	MENU_DELETE,
//	MENU_DELETE_TEXT,
//	MENU_PASTE_TEXTTL,
//	MENU_TLDIAL,
//	MENU_MKV_SUBS,
//	MENU_CONT_PREV,
//	MENU_CONT_NEXT,
//	MENU_PASTECOLS,
//	MENU_COPYCOLS,
//	MENU_FPSFROMVIDEO,
//	MENU_NEWFPS
//};

//#define IDS(XX) \
//	XX( InsertBefore,=5555 ) \
//	XX( InsertAfter, ) \
//	XX( InsertBeforeVideo, ) \
//	XX( InsertAfterVideo, ) \
//	XX( Swap, ) \
//	XX( Duplicate, ) \
//	XX( Join, ) \
//	XX( JoinToFirst, ) \
//	XX( JoinToLast, ) \
//	XX( Copy, ) \
//	XX( Paste, ) \
//	XX( Cut, ) \
//	XX( Remove, ) \
//	XX( RemoveText, ) \
//	XX( PasteTranslation, ) \
//	XX( TranslationDialog, ) \
//	XX( SubsFromMKV, ) \
//	XX( ContinousPrevious, ) \
//	XX( ContinousNext, ) \
//	XX( PasteCollumns, ) \
//	XX( CopyCollumns, ) \
//	XX( FPSFromVideo, ) \
//	XX( NewFPS, ) \
//

#endif
