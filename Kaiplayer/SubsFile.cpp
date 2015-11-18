
#include "SubsFile.h"
#include <wx/log.h>


File::File()
{
}

File::~File()
{
	dials.clear();
	styles.clear();
	sinfo.clear();
	ddials.clear();
	dstyles.clear();
	dsinfo.clear();
	//wxLogStatus("Clearing");
}
void File::Clear()
{
	
	for(std::vector<Dialogue*>::iterator it = ddials.begin(); it != ddials.end(); it++)
	{
		
		delete (*it);
		
	}
	
	for(std::vector<Styles*>::iterator it = dstyles.begin(); it != dstyles.end(); it++)
	{
		delete (*it);
		
	}
	
	for(std::vector<SInfo*>::iterator it = dsinfo.begin(); it != dsinfo.end(); it++)
	{
		delete (*it);
		
	}
}



File *File::Copy()
{
	File *file=new File();
	file->dials = dials;
	file->styles= styles;
	file->sinfo = sinfo;
	return file;
}

SubsFile::SubsFile()
{
    iter=0;
	edited=false;
	subs = new File();
}

SubsFile::~SubsFile()
{
	subs->Clear();
	delete subs;
	for(std::vector<File*>::iterator it = undo.begin(); it != undo.end(); it++)
	{
		(*it)->Clear();
		delete (*it);
	}
    undo.clear();
}


void SubsFile::SaveUndo()
{
	int size=maxx();
	if(iter!=size){
		for(std::vector<File*>::iterator it = undo.begin()+iter+1; it != undo.end(); it++)
		{
			(*it)->Clear();
			delete (*it);
		}
		undo.erase(undo.begin()+iter+1, undo.end());
	}
	//wxLogStatus("sizes di %i, si %i, sii %i dd %i ds %i dsi %i", Dialogue::iterator, Styles::iterator, SInfo::iterator, (int)subs->ddials.size(), (int)(subs->dstyles.size()+Options.StoreSize()), (int)subs->dsinfo.size());   
	undo.push_back(subs);
	subs=subs->Copy();
	iter++;
	//wxLogStatus("size %i", undo.size());
	edited=false;
}


void SubsFile::Redo()
{
    if(iter<maxx()){
		iter++;
		delete subs;
		subs=undo[iter]->Copy();
	}
}

void SubsFile::Undo()
{
    if(iter>0){
		iter--;
		delete subs;
		subs=undo[iter]->Copy();
	}
}
bool SubsFile::IsNotSaved()
{
    if(subs->ddials.size()==0 && subs->dstyles.size()==0 && subs->dsinfo.size()==0 && !edited){return false;}
    return true;
}

int SubsFile::maxx()
{
    return undo.size()-1;
}

int SubsFile::Iter()
{
    return iter;
}

Dialogue *SubsFile::CopyDial(int i, bool push, bool keepstate)
{
	Dialogue *dial=subs->dials[i]->Copy(keepstate);
	subs->ddials.push_back(dial);
	if(push){subs->dials[i]=dial;}
	return dial;
}
	
Styles *SubsFile::CopyStyle(int i, bool push)
{
	Styles *styl=subs->styles[i]->Copy();
	subs->dstyles.push_back(styl);
	if(push){subs->styles[i]=styl;}
	return styl;
}
	
SInfo *SubsFile::CopySinfo(int i, bool push)
{
	SInfo *sinf=subs->sinfo[i]->Copy();
	subs->dsinfo.push_back(sinf);
	if(push){subs->sinfo[i]=sinf;}
	return sinf;
}

void SubsFile::EndLoad()
{
	undo.push_back(subs);
	subs=subs->Copy();
}

void SubsFile::RemoveFirst(int num)
{
	//uwaga pierwszym elementem tablicy s¹ napisy zaraz po wczytaniu dlatego te¿ nie nale¿y go usuwaæ
	for(std::vector<File*>::iterator it = undo.begin()+1; it != undo.begin()+num; it++)
	{
		(*it)->Clear();
		delete (*it);
	}
	undo.erase(undo.begin()+1, undo.begin()+num);
	iter-=(num-1);
}