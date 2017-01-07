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


#include "FontCollector.h"
#include "KainoteApp.h"
#include "Config.h"
#include "MKVWrap.h"
#include <wx/fontenum.h>
#include <wx/wfstream.h>
#include <wx/zipstrm.h>
#include <wx/dir.h>
#include <wx/regex.h>
#include "KaiMessageBox.h"

//#include <algorithm>



FontCollectorDialog::FontCollectorDialog(wxWindow *parent)
	: KaiDialog(parent,-1,_("Kolekcjoner czcionek"),wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER)
{
	SetForegroundColour(Options.GetColour("Window Text"));
	SetBackgroundColour(Options.GetColour("Window Background"));
	DialogSizer *Main = new DialogSizer(wxVERTICAL);
	wxBoxSizer *Pathc = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *Buttons = new wxBoxSizer(wxHORIZONTAL);

	path=new KaiTextCtrl(this,-1,Options.GetString("Font Collect Directory"),wxDefaultPosition, wxSize(150,-1));
	path->Enable(Options.GetInt("Font Collect Action")!=0);
	//path->SetToolTip("Można też wybrać folder napisów wpisując <subs dir>.");
	choosepath=new MappedButton(this,8799,_("Wybierz folder"));
	choosepath->Enable(Options.GetInt("Font Collect Action")!=0);
	Connect(8799,wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&FontCollectorDialog::OnButtonPath);

	Pathc->Add(path,1,wxEXPAND|wxALL,3);
	Pathc->Add(choosepath,0,wxBOTTOM|wxTOP|wxRIGHT,3);

	wxArrayString choices;
	choices.Add(_("Sprawdź dostępność czcionek"));
	choices.Add(_("Skopiuj do wybranego folderu"));
	choices.Add(_("Spakuj zipem"));
	choices.Add(_("Wmuxuj napisy w wideo (wymagany MKVToolnix)"));
	opts=new KaiRadioBox(this,9987,_("Opcje"),wxDefaultPosition,wxDefaultSize,choices,0,wxRA_SPECIFY_ROWS);
	opts->SetSelection(Options.GetInt("Font Collect Action"));
	Connect(9987,wxEVT_COMMAND_RADIOBOX_SELECTED, (wxObjectEventFunction)&FontCollectorDialog::OnChangeOpt);

	subsdir=new KaiCheckBox(this,7998,_("Zapisuj do folderu z napisami"));
	subsdir->Enable(Options.GetInt("Font Collect Action")!=0);
	subsdir->SetValue(Options.GetBool("Collector Subs Directory"));


	fromMKV=new KaiCheckBox(this,7991,_("Wyciągnij czcionki z wczytanego pliku MKV"));
	fromMKV->Enable(Notebook::GetTab()->VideoPath.Lower().EndsWith(".mkv"));
	fromMKV->SetValue(Options.GetBool("Collect From MKV"));

	Connect(7998,wxEVT_COMMAND_CHECKBOX_CLICKED, (wxObjectEventFunction)&FontCollectorDialog::OnChangeOpt);
	console=new wxTextCtrl(this,-1,"",wxDefaultPosition,wxSize(300,200),wxTE_RICH2|wxTE_MULTILINE|wxTE_READONLY);
	console->SetForegroundColour(Options.GetColour("Window Text"));
	console->SetBackgroundColour(Options.GetColour("Window Background"));
	bok=new MappedButton(this,9879,_("Rozpocznij"));
	bcancel=new MappedButton(this,wxID_CANCEL,_("Zamknij"));
	Connect(9879,wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&FontCollectorDialog::OnButtonStart);
	Buttons->Add(bok,0,wxLEFT|wxTOP|wxRIGHT,3);
	Buttons->Add(bcancel,0,wxBOTTOM|wxTOP|wxRIGHT,3);

	Main->Add(Pathc,0,wxEXPAND,0);
	Main->Add(opts,0,wxLEFT|wxRIGHT|wxEXPAND,3);
	Main->Add(subsdir,0,wxALL|wxEXPAND,3);
	Main->Add(fromMKV,0,wxALL|wxEXPAND,3);
	Main->Add(console,1,wxLEFT|wxRIGHT|wxEXPAND,3);
	Main->Add(Buttons,0,wxALIGN_CENTER,0);

	SetSizerAndFit(Main);
	CenterOnParent();
	ShowModal();
}



wxArrayString FontCollectorDialog::GetAssFonts(std::vector<bool> &founded, bool check)
{
	if(facenames.size()<1){facenames = wxFontEnumerator::GetFacenames();}
	wxArrayString fonts;
	std::map<wxString, wxString> notfindfonts;
	std::map<wxString, wxString> stylesfonts;
	File *subs= Notebook::GetTab()->Grid1->file->GetSubs();
	wxRegEx reg("\\{[^\\{]*\\}",wxRE_ADVANCED);

	console->SetDefaultStyle(wxTextAttr(*wxBLACK));
	for(size_t i=0; i<subs->styles.size(); i++)
	{
		wxString fn=subs->styles[i]->Fontname.Lower();
		if(facenames.Index(fn,false)==-1){
			notfindfonts[fn]<<subs->styles[i]->Name<<" ";
			continue;
		}
		else if(fonts.Index(fn,false)==-1)
		{
			fonts.Add(fn);
			console->AppendText( wxString::Format(_("Znaleziono czcionkę o nazwie \"%s\" w stylu \"%s\".\n \n"), 
				subs->styles[i]->Fontname, subs->styles[i]->Name) );
			console->AppendText(wxString::Format(_("Czcionka \"%s\" jest zainstalowana.\n \n"), subs->styles[i]->Fontname));
			founded.push_back(check);
		}
		stylesfonts[subs->styles[i]->Name]=fn;
		
	}

	for(size_t i=0; i<subs->dials.size(); i++)
	{
		wxString text=subs->dials[i]->Text + subs->dials[i]->TextTl;
		wxString ifont=stylesfonts[subs->dials[i]->Style];
		//wxLogStatus(ifont);
		int wlet=0;
		while(true)
		{
			wxString part = text.Mid(wlet);
			//wxLogStatus("line %i",i);
			int flet=part.find("\\fn");

			if(flet!=-1)
			{
				wxString fn= part.Mid(flet+3).BeforeFirst('\\').BeforeFirst('}').Lower();
				if(facenames.Index(fn,false)==-1){
					notfindfonts[fn]<<(i+1)<<" ";
				}
				else if(fonts.Index(fn,false)==-1)
				{
					wxString txt=text.SubString(wlet,flet)+"}";
					if(wlet!=0){txt+="{";};
					reg.ReplaceAll(&txt,"");
					PutChars(txt,ifont,i+1);
					fonts.Add(fn);
					console->AppendText(wxString::Format(_("Znaleziono czcionkę o nazwie \"%s\" w linii nr %i.\n \n"), fn, (i+1)));
					console->AppendText(wxString::Format(_("Czcionka \"%s\" jest zainstalowana.\n \n"), fn));
					founded.push_back(check);
				}
				wlet= wlet+flet+3+fn.Len();
				//wxLogMessage("wlet",wlet);
				ifont=fn;
			}else{
				if(ifont!=""){
					wxString txt=text;
					if(wlet!=0){txt= "{" + text.Mid(wlet); };
					reg.ReplaceAll(&txt,"");
					PutChars(txt,ifont,i+1);
				}
				wlet=0;break;
			}
		}
	}
	//wxLogStatus("size %i",(int)notfindfonts.size());
	console->SetDefaultStyle(wxTextAttr(*wxRED));
	for(auto cur=notfindfonts.begin(); cur!=notfindfonts.end(); cur++){
		wxString list=cur->second;
		wxLogStatus(list);
		wxStringTokenizer token(list," ");
		wxString result= _("Nie można znaleźć czcionki \"") + cur->first + "\".\r\n";
		founded.push_back(false);
		bool first=true;
		//wxLogStatus(cur->first+" "+cur->second);
		while(token.HasMoreTokens()){
			wxString tkn=token.GetNextToken();
			if(tkn.IsNumber()){
				wxLogStatus("Token %i %i",token.GetPosition(),tkn.Len());
				result+= _("Użytej w linijkach: ") + list.Mid(token.GetPosition() - tkn.Len() - 1) + "\r\n";
				break;
			}else{
				if(first){result += _("Użytej w stylach:\r\n"); first=false;}
				result+=" - "+tkn+"\r\n";
			}
		}

		console->AppendText(result+"\r\n");
	}

	return fonts;
}

void FontCollectorDialog::CopyFonts(bool check)
{


	FT_Library ft2lib;
	if(FT_Init_FreeType(&ft2lib))
	{wxLogMessage(_("Dupa blada freetype nie chce się zinitializować"));return;}
	bool allglyphs=true;

	wxArrayString foundedfaces;
	wxArrayString fontfiles;

	wxString fontrealpath=wxGetOSDirectory() + "\\fonts\\";
	wxDir::GetAllFiles(fontrealpath, &fontfiles, wxEmptyString, wxDIR_FILES);

	std::vector<bool> founded;
	wxArrayString fontfaces = GetAssFonts(founded);

	console->SetDefaultStyle(wxTextAttr(*wxBLACK));
	console->AppendText(_("Rozpoczęto szukanie czcionek.\n \n"));
	bool notfound=false;
	for(size_t i=0; i<fontfaces.size(); i++)
	{
		if(fontfaces[i].StartsWith("@")){
			fontfaces[i]=fontfaces[i].Mid(1);
		}
	}
	long lSize=0;
	byte *buff=0;

	for(size_t i=0; i<fontfiles.size(); i++)
	{
		FT_Face face;
		wxString ext=fontfiles[i].AfterLast('.').Lower();
		for(int findex=0; true ; findex++){//wxConvFileName
			int error =FT_New_Face(ft2lib, fontfiles[i].mb_str(*wxConvFileName), findex, &face);
			if(error == 1){
				if(findex==0){
					FILE *fp = _wfopen(fontfiles[i].wc_str(),L"rb");

					fseek (fp , 0 , SEEK_END);
					lSize = ftell (fp);
					rewind (fp);

					buff = (byte*)malloc(lSize);
					if(!buff){fclose(fp);break;}
					int result = fread (buff,1,lSize,fp);
					fclose(fp);
					if(result!=(int)lSize){free(buff);buff=0;break;}
				}
				error = FT_New_Memory_Face(ft2lib, &buff[0], lSize, findex, &face);
			}
			if(error && findex==0 && ext!="ini" && ext!="pfm" && ext!="compositefont"){
				wxString namefrompath=fontfiles[i].AfterLast('\\').BeforeLast('.');
				int inx=fontfaces.Index(namefrompath,false);
				if(inx!=-1){
					founded[inx]=true;
					//console->AppendText("Znaleziono czcionkę \""+all+"\".\n \n");
					fontnames.Add(fontfiles[i]);
					foundedfaces.Add(namefrompath);
				}

				//console->AppendText("Czcionki: "+fontfiles[i]+"\nnie trawi FreeType, masz pecha nie skopiujesz jej.\n\n");
			}
			if(error){if(buff){free(buff); buff=0;}break;}


			int inx;
			// wxMBConvUTF16BE conv;
			wxString fnf=wxString(face->family_name, wxConvLocal).Lower();
			wxString style=wxString(face->style_name, wxConvLocal);

			if(ext=="fon" || ext=="pfb"){
				inx=fontfaces.Index(fnf,false);
				if(inx!=-1){
					wxString all=fnf+" "+style;
					if( foundedfaces.Index(all,false)==-1 && fontnames.Index(fontfiles[i],false)==-1){
						founded[inx]=true;
						//console->AppendText("Znaleziono czcionkę \""+all+"\".\n \n");
						wxString res=CheckChars(face,&FontMap[fnf]);
						//wxLogStatus(all+" "+res);
						if(res!=""){
							allglyphs=false;
							console->SetDefaultStyle(wxTextAttr(*wxRED));
							console->AppendText(wxString::Format(_("Czcionka \"%s\" nie ma znaków: "), all) + res);
						}
						fontnames.Add(fontfiles[i]);
						foundedfaces.Add(all);
						if(ext=="pfb"){fontnames.Add(fontfiles[i].RemoveLast(1)+"M");}
					}
				}

				goto done;
			}
			int count = FT_Get_Sfnt_Name_Count(face);


			for (int ii=0;ii<count;ii++) {

				FT_SfntName name;
				FT_Get_Sfnt_Name(face,ii,&name);
				if (name.name_id == 1) {
					char *str = new char[name.string_len+2];
					memcpy(str,name.string,name.string_len);
					str[name.string_len] = 0;
					str[name.string_len+1] = 0;
					wxString result;
					if (name.encoding_id == 0) result = wxString(str, wxConvLocal);
					else if (name.encoding_id == 1) { wxMBConvUTF16BE conv; result = wxString(str,conv);}
					if(result==""){delete [] str; continue;}
					result=result.Lower();

					if((result!="arial" || fnf=="arial")){

						inx=fontfaces.Index(result,false);
						if(inx!=-1)
						{

							wxString all=result+" "+style;
							if( foundedfaces.Index(all,false)==-1 && fontnames.Index(fontfiles[i],false)==-1){
								wxString res=CheckChars(face,&FontMap[result]);
								//wxLogStatus(result+" "+res);
								if(res!=""){
									allglyphs=false;
									console->SetDefaultStyle(wxTextAttr(*wxRED));
									console->AppendText(wxString::Format(_("Czcionka \"%s\" nie ma znaków: "), all) + res);
								}
								founded[inx]=true;
								fontnames.Add(fontfiles[i]);
								foundedfaces.Add(all);
							}
						}
					}

					delete [] str;
				}
			}
done:
			FT_Done_Face(face);
		}
	}

	FT_Done_FreeType( ft2lib);
	if(check){
		FontMap.clear();
		int found=0;
		int nfound=0;
		for(size_t i=0; i<founded.size(); i++){
			if(!founded[i]){
				nfound++;
			}else{found++;}
		}
		if(nfound||!allglyphs){
			console->SetDefaultStyle(wxTextAttr(*wxRED));
			wxString noglyphs= (allglyphs)? L"" : _("\nCzcionka nie ma wszystkich znaków użytych w tekście");
			console->AppendText(wxString::Format(_("Zakończono, znaleziono %i czcionek.\nNie znaleziono %i czcionek. "), found, nfound ) + noglyphs );}
		else{
			console->SetDefaultStyle(wxTextAttr("#008000"));
			console->AppendText(wxString::Format(_("Zakończono powodzeniem, znaleziono %i czcionek."), found));
		}
		fontfaces.clear();
		foundedfaces.clear();
		fontfiles.clear();
		return;
	}

	
	if(subsdir->GetValue()){
		wxString rest;
		copypath=Notebook::GetTab()->SubsPath.BeforeLast('\\',&rest);
		copypath<<_("\\Czcionki");
		if(opts->GetSelection()==2){copypath<<"\\"<<rest.BeforeLast('.')<<".zip";}
	}else{copypath=path->GetValue();
	Options.SetString("Font Collect Directory",copypath);
	}
	//wxLogStatus("copy");
	wxString extt=copypath.Right(4).Lower();
	if(opts->GetSelection()!=2){
		if(extt==".zip"){copypath=copypath.BeforeLast('\\');}
		if(!wxDir::Exists(copypath))
		{
			wxDir::Make(copypath);
		}
		for(size_t i=0; i< fontnames.size(); i++)
		{
			//wxLogMessage(fontrealpath+fontnames[i]+" "+copypath+"\\"+fontnames[i]);
			if(wxCopyFile(fontnames[i],copypath+"\\"+fontnames[i].AfterLast('\\'))){
				console->SetDefaultStyle(wxTextAttr(*wxBLACK));
				console->AppendText(_("Skopiowano czcionkę o nazwie \"")+fontnames[i].AfterLast('\\')+"\".\n \n");
			}
			else
			{
				console->SetDefaultStyle(wxTextAttr(*wxRED));
				console->AppendText(_("Nie można skopiować czcionki o nazwie \"")+fontnames[i].AfterLast('\\')+"\".\n \n");
			}
		}
	}
	else
	{
		if(!wxDir::Exists(copypath.BeforeLast('\\')))
		{
			wxDir::Make(copypath.BeforeLast('\\'));
		}
		else if(wxFileExists(copypath)){
			if(KaiMessageBox(_("Plik zip już istnieje, usunąć go?"),_("Potwierdzenie"),wxYES_NO,this)==wxYES){
				wxRemoveFile(copypath);
			}
		}
		wxFFileOutputStream *out =new wxFFileOutputStream(copypath);
		wxZipOutputStream *zip = new wxZipOutputStream(*out);
		zip->SetLevel(9);
		for(size_t i=0; i< fontnames.size(); i++)
		{

			wxString filename = fontnames[i];
			wxFFileInputStream in(filename);
			bool isgood=in.IsOk();
			if(isgood){
				try {
					//wxFileName fn(filename);
					wxString fn=fontnames[i].AfterLast('\\');
					//fn.Replace("_0","");
					zip->PutNextEntry(fn);
					zip->Write(in);
					console->SetDefaultStyle(wxTextAttr(*wxBLACK));
					console->AppendText(_("Skopiowano czcionkę o nazwie \"")+fontnames[i].AfterLast('\\')+"\".\n \n");
				}
				catch (...) {
					isgood=false;
				}

			}

			if(!isgood)
			{
				console->SetDefaultStyle(wxTextAttr(*wxRED));
				console->AppendText(_("Nie można spakować czcionki o nazwie \"")+fontnames[i].AfterLast('\\')+"\".\n \n");
				founded[i]=false;
			}

		}
		zip->Close();
		delete zip;
		delete out;

	}
	int found=0;
	int nfound=0;
	for(size_t i=0; i<founded.size(); i++){
		if(!founded[i]){
			nfound++;
		}else{found++;}
	}

	fontfaces.clear();
	foundedfaces.clear();
	fontfiles.clear();
	FontMap.clear();

	if(nfound){
		console->SetDefaultStyle(wxTextAttr(*wxRED));
		console->AppendText(wxString::Format(_("Zakończono, skopiowano %i czcionek.\nNie udało się skopiować %i czcionek."), (int)fontnames.size(), nfound));}
	else{
		console->SetDefaultStyle(wxTextAttr("#008000"));
		console->AppendText(wxString::Format(_("Zakończono powodzeniem, skopiowano %i czcionek."), (int)fontnames.size()));
	}
}



void FontCollectorDialog::CopyMKVFonts()
{
	wxString mkvpath=Notebook::GetTab()->VideoPath;
	MatroskaWrapper mw;
	mw.Open(mkvpath);
	std::map<int, wxString> names=mw.GetFontList();
	if(names.size()<1){
		console->SetDefaultStyle(wxTextAttr(*wxRED));
		console->AppendText(_("Wczytany plik MKV nie ma żadnych czcionek."));
		return;}

	wxString copypath;
	if(subsdir->GetValue()){
		wxString rest;
		copypath=Notebook::GetTab()->SubsPath.BeforeLast('\\',&rest);
		copypath<<_("\\Czcionki");
		if(opts->GetSelection()==2){copypath<<"\\"<<rest.BeforeLast('.')<<".zip";}
	}else{copypath=path->GetValue();
	Options.SetString("Font Collect Directory",copypath);
	}

	Options.SetString("Font Collect Directory",copypath);
	
	size_t cpfonts=names.size();
	wxString onlypath;
	wxZipOutputStream *zip=NULL;
	wxFFileOutputStream *out=NULL;
	if(opts->GetSelection()!=2){
		wxString extt=copypath.Right(4).Lower();
		if(extt==".zip"){copypath=copypath.BeforeLast('\\');}
		if(!wxDir::Exists(copypath)){wxDir::Make(copypath);}
		onlypath=copypath+"\\";
	}
	else
	{
		if(!wxDir::Exists(copypath.BeforeLast('\\'))){wxDir::Make(copypath.BeforeLast('\\'));}
		else if(wxFileExists(copypath)){
			if(KaiMessageBox(_("Plik zip już istnieje, usunąć go?"),_("Potwierdzenie"),wxYES_NO,this)==wxYES){wxRemoveFile(copypath);}}
		out =new wxFFileOutputStream(copypath);
		zip = new wxZipOutputStream(*out);
	}
	for(auto fontI : names){
		if(mw.SaveFont(fontI.first, onlypath + fontI.second, zip))
		{
			console->SetDefaultStyle(wxTextAttr(*wxBLACK));
			console->AppendText(_("Zapisano czcionkę o nazwie \"")+ fontI.second +"\".\n \n");
		}
		else
		{
			console->SetDefaultStyle(wxTextAttr(*wxRED));
			console->AppendText(_("Nie można zapisać czcionki o nazwie \"")+ fontI.second +"\".\n \n");
			cpfonts--;
		}
	}
	if(zip)
	{
		zip->Close();
		delete zip;
		delete out;
	}
	if(cpfonts<names.size()){
		console->SetDefaultStyle(wxTextAttr(*wxRED));
		console->AppendText(wxString::Format(_("Zakończono, skopiowano %i czcionek.\nNie udało się skopiować %i czcionek."), cpfonts, names.size() - cpfonts));}
	else{
		console->SetDefaultStyle(wxTextAttr("#008000"));
		console->AppendText(wxString::Format(_("Zakończono powodzeniem, skopiowano %i czcionek."), cpfonts));
	}

	names.clear();
	mw.Close();
}

void FontCollectorDialog::OnButtonStart(wxCommandEvent &event)
{
	console->SetValue("");
	path->Enable(false);
	choosepath->Enable(false);
	opts->Enable(false);
	bok->Enable(false);
	bcancel->Enable(false);

	if(opts->GetSelection()==0)
	{
		CopyFonts(true);
	}
	else
	{
		if(opts->GetSelection()==3 && ( Notebook::GetTab()->VideoPath=="" || Notebook::GetTab()->SubsPath=="" )){KaiMessageBox(_("Brak zaczytanego wideo bądź napisów"));return;}
		if(path->GetValue()==""&& !subsdir->GetValue()){
			KaiMessageBox(_("Wybierz folder, gdzie mają zostać skopiowane czcionki"));
			goto done;
		}
		if(opts->GetSelection()==2 && !path->GetValue().EndsWith(".zip") && !subsdir->GetValue()){
			KaiMessageBox(_("Wybierz nazwę dla archiwum"));
			goto done;
		}
		if(fromMKV->GetValue() && fromMKV->IsEnabled()){CopyMKVFonts();}
		else{CopyFonts();}
		if(opts->GetSelection()==3){MuxVideoWithSubs();}
	}
done:
	path->Enable(opts->GetSelection()!=0);
	choosepath->Enable(opts->GetSelection()!=0);
	opts->Enable(true);
	bok->Enable(true);
	bcancel->Enable(true);
}

void FontCollectorDialog::OnChangeOpt(wxCommandEvent &event)
{
	path->Enable(opts->GetSelection()!=0);
	choosepath->Enable(opts->GetSelection()!=0);
	subsdir->Enable(opts->GetSelection()!=0);

	fromMKV->Enable(opts->GetSelection()!=0 && Notebook::GetTab()->VideoPath.Lower().EndsWith(".mkv"));

	Options.SetInt("Font Collect Action",opts->GetSelection());
	Options.SetBool("Collector Subs Directory",subsdir->GetValue());
	Options.SaveOptions(true,false);
}

void FontCollectorDialog::OnButtonPath(wxCommandEvent &event)
{
	
	if(opts->GetSelection()==1){
		destdir = wxDirSelector(_("Wybierz folder zapisu"), path->GetValue(),0,wxDefaultPosition,this);
	}
	else{
		destdir= wxFileSelector(_("Wybierz nazwę archiwum"), (path->GetValue().EndsWith("zip"))? path->GetValue().BeforeLast('\\') : path->GetValue(), 
			(path->GetValue().EndsWith("zip"))? path->GetValue().AfterLast('\\') : "",
			"zip",_("Pliki archiwum (*.zip)|*.zip"),wxFD_SAVE|wxFD_OVERWRITE_PROMPT,this);
	}
	Options.SetString("Font Collect Directory",destdir);
	Options.SaveOptions(true,false);
	path->SetValue(destdir);
}

void FontCollectorDialog::PutChars(wxString txt, wxString fn, int move)
{
	CharMap ch=FontMap[fn];

	for(size_t i=0; i<txt.Len(); i++)
	{
		ch[txt[i]] << move << " ";
	}
	FontMap[fn]=ch;
}

wxString FontCollectorDialog::CheckChars(FT_Face face, std::map<wxUniChar, wxString> *chars)
{
	wxString rettxt;
	for(auto cur=chars->begin(); cur!= chars->end(); cur++)
	{
		size_t haschar=FT_Get_Char_Index(face,static_cast<ULONG>(cur->first));
		if(haschar==0){
			rettxt<< "\"" << cur->first << _("\", który występuje w linijkach:\n") << cur->second << ".\n";

		}
	}

	return rettxt;
}

void FontCollectorDialog::MuxVideoWithSubs()
{
	wxString muxerpath=L"C:\\Program Files\\MKVtoolnix\\mkvmerge.exe";
	if(!wxFileExists(muxerpath)){
		wxFileDialog *fd = new wxFileDialog(this,_("Wybierz plik mkvmerge.exe"), L"C:\\Program Files",L"mkvmerge.exe",_("Programy (.exe)|*.exe"),wxFD_OPEN|wxFD_FILE_MUST_EXIST);
		if(fd->ShowModal()!=wxID_OK){
			KaiMessageBox(_("Muxowanie zostało anulowane, bo nie wybrano mkvmerge.exe"));fd->Destroy();return;
		}
		muxerpath=fd->GetPath();
		fd->Destroy();
	}

	wxString command=L"\"--ui-language\" \"pl\" \"--output\" \"" + Notebook::GetTab()->VideoPath.BeforeLast('.') + L" (1).mkv\" " + 
		L"\"--language\" \"0:und\" \"--language\" \"1:und\" ( \""\
		+ Notebook::GetTab()->VideoPath +
		L"\" ) \"--language\" \"0:und\" ( \""+ Notebook::GetTab()->SubsPath +
		L"\" ) ";// odtąd trzeba dodać czcionki
	for(size_t i=0; i< fontnames.size(); i++){
		wxString ext= fontnames[i].AfterLast('.').Lower();
		/*if(ext!="ttf" && ext!="ttc" && ext!="otf"){
			KaiMessageBox(wxString::Format(_("Rozszerzenie czcionki \"%s\" nie jest obsługiwane"),ext));
			continue;
		}*/
		command << L"\"--attachment-name\" \"";
		wxString name=fontnames[i].AfterLast('\\');
		command << name << L"\" ";
		command << L"\"--attachment-mime-type\" ";
		//if(ext=="ttf" || ext=="ttc"){
			command << L"\"application/x-truetype-font\" ";//}
		//else if(ext=="otf"){command << L"\"application/font-woff\" ";}
		//else otf itp
		command << L"\"--attach-file\" \""; 
		command << copypath << L"\\" << name << L"\" ";
	}
	command.Replace("\\","/");
	command << L"\"--track-order\" \"0:0,0:1,1:0\"";
	wxString fullcommand = L"\"" + muxerpath + L"\"" + command;
	//wxLogStatus(command);
	
	STARTUPINFO si = { sizeof( si ) };
    PROCESS_INFORMATION pi;
	if(!CreateProcessW(NULL, (LPWSTR)fullcommand.wc_str(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi )){
		wxLogStatus(_("Nie można stworzyć procesu, muxowanie przerwane"));
	}


}