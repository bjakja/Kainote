//  Copyright (c) 2016, Marcin Drob

//  Kainote is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.

//  Kainote is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.

//  You should have received a copy of the GNU General Public License
//  along with Kainote.  If not, see <http://www.gnu.org/licenses/>.


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
	else if(filenames.size()>0){
		int w,h;
		Kai->GetClientSize(&w,&h);
		if(y>h-26){Kai->InsertTab();}
		Kai->OpenFile(filenames[0]);
	}
	return true;
}

//DragScripts::DragScripts(kainoteFrame* kfparent)
//{
//	Kai=kfparent;
//}

//bool DragScripts::OnDropFiles(wxCoord x, wxCoord y, const wxArrayString& filenames)
//{
//	Automation *An = Kai->Auto;
//	int error_count=0;
//	for(size_t i=0; i<filenames.size(); i++)
//	{
//		try {
//			wxString fullpath =filenames[i];
//
//			if(!fullpath.Lower().EndsWith("lua") || !An->Add(fullpath)){continue;}
//
//			int cnt=An->Scripts.size()-1;
//			if (!An->Scripts[cnt]->GetLoadedState()) error_count++;
//			long pos = Kai->LSD->ScriptsList->InsertItem(cnt,wxString::Format("%i",(int)(cnt+1)));
//			Kai->LSD->ScriptsList->SetItem(pos,1,An->Scripts[cnt]->name);
//			Kai->LSD->ScriptsList->SetItem(pos,2,An->Scripts[cnt]->filename.AfterLast('\\'));
//			Kai->LSD->ScriptsList->SetItem(pos,3,An->Scripts[cnt]->description);
//			Kai->LSD->ScriptsList->ScrollList(0,11111111);
//			Grid *grid=Kai->GetTab()->Grid1;
//			wxString scripts = grid->GetSInfo("Automation Scripts")+=(An->Scripts[i]->filename+"|");
//			grid->AddSInfo("Automation Scripts", scripts);
//
//		}
//		catch (const wchar_t *e) {
//			error_count++;
//			wxLogError(_("Błąd wczytywania skryptu Lua: %s\n%s"), filenames[i].c_str(), e);
//		}
//		catch (...) {
//			error_count++;
//			wxLogError(_("Nieznany błąd wczytywania skryptu Lua: %s."), filenames[i].c_str());
//		}
//
//
//	}
//	if (error_count > 0) {
//		wxLogWarning(_("Co najmniej jeden skrypt z pliku napisów zawiera błędy.\nZobacz opisy skryptów, by uzyskać więcej informacji."));
//	}
//	return true;
//
//}