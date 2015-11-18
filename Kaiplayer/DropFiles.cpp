
#include "DropFiles.h"
#include "kainoteMain.h"

DragnDrop::DragnDrop(kainoteFrame* kfparent)
{
    Kai=kfparent;
}

bool DragnDrop::OnDropFiles(wxCoord x, wxCoord y, const wxArrayString& filenames)
    {
	if(filenames.size()>1){
		Kai->OpenFiles(filenames);}
	else{
		int w,h;
		Kai->GetClientSize(&w,&h);
		if(y>h-26){Kai->InsertTab();}
		Kai->OpenFile(filenames[0]);
	}
        return true;
    }

DragScripts::DragScripts(kainoteFrame* kfparent)
	{
	Kai=kfparent;
	}

bool DragScripts::OnDropFiles(wxCoord x, wxCoord y, const wxArrayString& filenames)
	{
	Auto::Automation *An = Kai->Auto;
	int error_count=0;
	for(size_t i=0; i<filenames.size(); i++)
		{
		try {
					wxString fullpath =filenames[i];
					
					if(!fullpath.Lower().EndsWith("lua") || !An->Add(fullpath)){continue;}
						
					int cnt=An->Scripts.size()-1;
						if (!An->Scripts[cnt]->loaded) error_count++;
						long pos = Kai->LSD->ScriptsList->InsertItem(cnt,wxString::Format("%i",(int)(cnt+1)));
						Kai->LSD->ScriptsList->SetItem(pos,1,An->Scripts[cnt]->name);
						Kai->LSD->ScriptsList->SetItem(pos,2,An->Scripts[cnt]->filename.AfterLast('\\'));
						Kai->LSD->ScriptsList->SetItem(pos,3,An->Scripts[cnt]->description);
						Kai->LSD->ScriptsList->ScrollList(0,11111111);
						Grid *grid=Kai->GetTab()->Grid1;
						wxString scripts = grid->GetSInfo("Automation Scripts")+=(An->Scripts[i]->filename+"|");
						grid->AddSInfo("Automation Scripts", scripts);

				}
				catch (const wchar_t *e) {
					error_count++;
					wxLogError(_T("B³¹d wczytywania skryptu Lua: %s\n%s"), filenames[i].c_str(), e);
				}
				catch (...) {
					error_count++;
					wxLogError(_T("Nieznany b³¹d wczytywania skryptu Lua: %s."), filenames[i].c_str());
				}
		

		}
	if (error_count > 0) {
			wxLogWarning(_T("Jeden b¹dŸ wiêcej skryptów autoload zawiera b³êdy,\n obejrzyj opisy skryptów by uzyskaæ wiêcej informacji."));
		}
	return true;
	
	}