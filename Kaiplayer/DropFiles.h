#ifndef DROPFILES_H_INCLUDED
#define DROPFILES_H_INCLUDED

#include <wx/dnd.h>
#include <wx/wxprec.h>
class kainoteFrame;

class DragnDrop : public wxFileDropTarget
{
    private:
    kainoteFrame* Kai;
    public:
    DragnDrop(kainoteFrame* kfparent);
	virtual ~DragnDrop(){ };
    bool OnDropFiles(wxCoord x, wxCoord y, const wxArrayString& filenames);

};

class DragScripts : public wxFileDropTarget
	{
	private:
		kainoteFrame* Kai;

	public:
		DragScripts(kainoteFrame* kfparent);
		virtual ~DragScripts(){ };
		bool OnDropFiles(wxCoord x, wxCoord y, const wxArrayString& filenames);
	};

#endif // DROPFILES_H_INCLUDED
