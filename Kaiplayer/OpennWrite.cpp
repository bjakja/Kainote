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
		if(!file.Create(fileName,false,wxS_DEFAULT)){wxLogStatus(_("Nie można utworzyć pliku."));}
	}
	else{
		if(!file.Open(fileName,(clear)?wxFile::write : wxFile::write_append,wxS_DEFAULT)){wxLogStatus(_("Nie można otworzyć pliku."));};
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
	if(!fname.IsFileReadable()){/*wxMessageBox("nie można odczytać pliku o ścieżce \""+filename+"\".");*/return "";}
	if (test){
		wchar_t b[4];
		filetest.Open(filename,wxFile::read,wxS_DEFAULT);
		filetest.Read (b, 4);
		
		utf8 =((static_cast < int >(b[0]) > 48000))? true : false;
		if(!utf8){
			size_t size = filetest.Length ();
			char *buff=new char[size];
			filetest.Read(buff, size);
			utf8=IsUTF8withoutBOM(buff, size);
			delete[] buff;
		}
		//wxString kkk;
		filetest.Close();
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
	if(fname.FileExists()&&!fname.IsFileReadable()){wxLogStatus(_("Pliku nie można odczytać."));return;}
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
	if(!file.IsOpened()){wxLogStatus(_("Plik nie został otwarty."));return;}
	if(isfirst){
		wchar_t bom = 0xFEFF;
		parttext = wxString(bom) + parttext;
		isfirst=false;
	}
	if(!file.Write(parttext,wxConvUTF8)){wxLogStatus(_("Nie można zapisać do pliku."));};
}

void OpenWrite::CloseFile()
{
	if(file.IsOpened()){file.Close();}
}

bool OpenWrite::IsUTF8withoutBOM(const char* buf, size_t size)
{
	bool	only_saw_ascii_range = true;
	size_t	pos = 0;
	int	more_chars;
	while (pos < size)
	{
		unsigned char ch = buf[pos++];
		//wxLogStatus("nch %i", (int)ch);
		if (ch <= 127)
		{
			// 1 byte
			more_chars = 0;
			//wxLogStatus("ch < 127");
		}
		else if (ch >= 194 && ch <= 223)
		{
			// 2 Byte
			more_chars = 1;
			//wxLogStatus("ch >= 194 && ch <= 223");
		}
		else if (ch >= 224 && ch <= 239)
		{
			// 3 Byte
			more_chars = 2;
			//wxLogStatus("ch >= 224 && ch <= 239");
		}
		else if (ch >= 240 && ch <= 244)
		{
			// 4 Byte
			more_chars = 3;
			//wxLogStatus("ch >= 240 && ch <= 244");
		}
		else
		{
			//return false; // Not utf8
		}
		// Check secondary chars are in range if we are expecting any
		while (more_chars && pos < size)
		{
			only_saw_ascii_range = false; // Seen non-ascii chars now
			ch = buf[pos++];
			//wxLogStatus("more chars %i", (int)ch);
			if (ch < 128 || ch > 205) {return false;} // Not utf8
			--more_chars;
		}
	}

	if(only_saw_ascii_range){ return false;}

	return true;

}