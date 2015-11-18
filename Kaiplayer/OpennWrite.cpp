
#include "OpennWrite.h"
#include <wx/filename.h>
#include <wx/ffile.h>
#include <wx/filefn.h>
#include <wx/log.h>

OpenWrite::OpenWrite()
{

}

OpenWrite::OpenWrite(wxString fileName, bool clear)
{
    wxFileName fname;
      fname.Assign(fileName);
      if(!fname.DirExists()){wxMkdir(fileName.BeforeLast('\\'));}
      if(fname.FileExists()&&!fname.IsFileReadable()){return;}
        if(!file.Exists(fileName)){
			if(!file.Create(fileName,false,wxS_DEFAULT)){wxLogStatus("cant create");}
       }
   else{
	   if(!file.Open(fileName,(clear)?wxFile::write : wxFile::write_append,wxS_DEFAULT)){wxLogStatus("cant open");};
   }
        isfirst=clear;
}

OpenWrite::~OpenWrite()
{
    CloseFile();
}

wxString OpenWrite::FileOpen(wxString filename, bool test)
{

    wxString s;
    bool utf8=true;
	wxFile filetest;
    wxFileName fname;
    fname.Assign(filename);
    if(!fname.IsFileReadable()){/*wxMessageBox("nie mo¿na odczytaæ pliku o œcie¿ce \""+filename+"\".");*/return _("");}
	if (test){
	  	wchar_t *b= new wchar_t[4];
	filetest.Open(filename,wxFile::read,wxS_DEFAULT);
    filetest.Read (b, 4);
	filetest.Close();
		utf8 =((static_cast < int >(b[0]) > 48000))? true : false;
		//wxLogStatus("utf %i %i", static_cast < int >(b[0]), utf8);
		delete[] b;
		//wxString kkk;
		 
			// wxMessageBox(kkk<<"pierwszy znak "<< static_cast < int >(b[0])<<", drugi znak "<<static_cast < int >(b[1])<<"trzeci znak "<<static_cast < int >(b[2]));
		}
			wxFFile fileo;
			fileo.Open(filename, wxT("r"));
    if (fileo.IsOpened()){
		if(utf8){
		fileo.ReadAll(&s);
			}else{fileo.ReadAll(&s, wxConvLocal);}
		fileo.Close();}
			 
	return s;
}

void OpenWrite::FileWrite(wxString fileName, wxString textfile, bool utf)
{

      wxFileName fname;
      fname.Assign(fileName);
      if(!fname.DirExists()){wxMkdir(fileName.BeforeLast('\\'));}
      if(fname.FileExists()&&!fname.IsFileReadable()){wxLogStatus("file is not readable");return;}
        if(utf){wchar_t bom = 0xFEFF;
		textfile = wxString(bom) + textfile;}
        wxFile file;
   if(!file.Exists(fileName)){
        file.Create(fileName,false,wxS_DEFAULT);
       }
   else{
        file.Open(fileName,wxFile::write,wxS_DEFAULT);}
        if (file.IsOpened()){
        if(utf){file.Write(textfile,wxConvUTF8);}else{file.Write(textfile,wxConvLocal);}
        file.Close();}

}
void OpenWrite::PartFileWrite(wxString parttext)
{
    if(!file.IsOpened()){wxLogStatus("file not opened");return;}
    if(isfirst){
        wchar_t bom = 0xFEFF;
		parttext = wxString(bom) + parttext;
		isfirst=false;
    }
	if(!file.Write(parttext,wxConvUTF8)){wxLogStatus("cant write");};
}

void OpenWrite::CloseFile()
{
    if(file.IsOpened()){file.Close();}
}
