
#include "FontCollector.h"
#include "kainoteApp.h"
#include "config.h"
#include "mkv_wrap.h"
#include <wx/fontenum.h>
#include <wx/wfstream.h>
#include <wx/zipstrm.h>
#include <wx/dir.h>
#include <wx/regex.h>
//#include <wx/msw/registry.h>

//#include <algorithm>



FontCollectorDialog::FontCollectorDialog(wxWindow *parent)
	: wxDialog(parent,-1,_("Kolekcjoner czcionek"),wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER)
{
	wxBoxSizer *Main = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *Pathc = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *Buttons = new wxBoxSizer(wxHORIZONTAL);

	path=new wxTextCtrl(this,-1,Options.GetString("Font Collect Directory"),wxDefaultPosition, wxSize(150,-1));
	path->Enable(Options.GetInt("Font Collect Action")!=0);
	//path->SetToolTip("Mo¿na te¿ wybraæ folder napisów wpisuj¹c <subs dir>.");
	choosepath=new wxButton(this,8799,_("Wybierz folder"));
	choosepath->Enable(Options.GetInt("Font Collect Action")!=0);
	Connect(8799,wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&FontCollectorDialog::OnButtonPath);

	Pathc->Add(path,1,wxEXPAND|wxALL,3);
	Pathc->Add(choosepath,0,wxBOTTOM|wxTOP|wxRIGHT,3);

	wxArrayString choices;
	choices.Add(_("SprawdŸ dostêpnoœæ czcionek"));
	choices.Add(_("Skopiuj do wybranego folderu"));
	choices.Add(_("Spakuj zipem"));
	opts=new wxRadioBox(this,9987,_("Opcje"),wxDefaultPosition,wxDefaultSize,choices,0,wxRA_SPECIFY_ROWS);
	opts->SetSelection(Options.GetInt("Font Collect Action"));
	Connect(9987,wxEVT_COMMAND_RADIOBOX_SELECTED, (wxObjectEventFunction)&FontCollectorDialog::OnChangeOpt);

	subsdir=new wxCheckBox(this,7998,_("Zapisuj do folderu z napisami"));
	subsdir->Enable(Options.GetInt("Font Collect Action")!=0);
	subsdir->SetValue(Options.GetBool("Collector Subs Directory"));


	fromMKV=new wxCheckBox(this,7991,_("Wyci¹gnij czcionki z wczytanego pliku MKV"));
	fromMKV->Enable(Notebook::GetTab()->VideoPath.Lower().EndsWith(".mkv"));
	fromMKV->SetValue(Options.GetBool("Collect From MKV"));

	Connect(7998,wxEVT_COMMAND_CHECKBOX_CLICKED, (wxObjectEventFunction)&FontCollectorDialog::OnChangeOpt);
	console=new wxTextCtrl(this,-1,"",wxDefaultPosition,wxSize(300,200),wxTE_RICH2|wxTE_MULTILINE|wxTE_READONLY);
	bok=new wxButton(this,9879,_("Rozpocznij"));
	bcancel=new wxButton(this,wxID_CANCEL,_("Zamknij"));
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
	wxRegEx reg(_T("\\{[^\\{]*\\}"),wxRE_ADVANCED);

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
			console->AppendText( wxString::Format(_("Znaleziono czcionkê o nazwie \"%s\" w stylu \"%s\".\n \n"), 
				subs->styles[i]->Fontname, subs->styles[i]->Name) );
			console->AppendText(wxString::Format(_("Czcionka \"%s\" jest zainstalowana.\n \n"), subs->styles[i]->Fontname));
			founded.push_back(check);
		}
		stylesfonts[subs->styles[i]->Name]=fn;
		//wxLogStatus(subs->styles[i]->Name + " " + fn);
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
					notfindfonts[fn]<<i<<" ";
				}
				else if(fonts.Index(fn,false)==-1)
				{
					wxString txt=text.SubString(wlet,flet)+"}";
					if(wlet!=0){txt+="{";};
					reg.ReplaceAll(&txt,"");
					PutChars(txt,ifont,i+1);
					fonts.Add(fn);
					wxString kkk;
					kkk<<(i+1);
					console->AppendText(wxString::Format(_("Znaleziono czcionkê o nazwie \"%s\" w linii nr %i.\n \n"), fn, kkk));
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
		wxStringTokenizer token(list," ");
		wxString result= _("Nie mo¿na znaleŸæ czcionki \"") + cur->first + "\".\r\n";
		founded.push_back(false);
		bool first=true;
		//wxLogStatus(cur->first+" "+cur->second);
		while(token.HasMoreTokens()){
			wxString tkn=token.GetNextToken();
			if(tkn.IsNumber()){
				result+= _("U¿ytej w linijkach: ") + list.Mid(token.GetPosition() - tkn.Len());
				break;
			}else{
				if(first){result += _("U¿ytej w stylach:\r\n"); first=false;}
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
	{wxLogMessage(_("Dupa blada freetype nie chce siê zinitializowaæ"));return;}
	bool allglyphs=true;

	wxArrayString foundedfaces;
	wxArrayString fontnames;
	wxArrayString fontfiles;

	wxString fontrealpath=wxGetOSDirectory() + _T("\\fonts\\");
	wxDir::GetAllFiles(fontrealpath, &fontfiles, wxEmptyString, wxDIR_FILES);

	std::vector<bool> founded;
	wxArrayString fontfaces = GetAssFonts(founded);

	console->SetDefaultStyle(wxTextAttr(*wxBLACK));
	console->AppendText(_("Rozpoczêto szukanie czcionek.\n \n"));
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
					//console->AppendText("Znaleziono czcionkê \""+all+"\".\n \n");
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
						//console->AppendText("Znaleziono czcionkê \""+all+"\".\n \n");
						wxString res=CheckChars(face,&FontMap[fnf]);
						wxLogStatus(all+" "+res);
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
								wxLogStatus(result+" "+res);
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
			wxString noglyphs= (allglyphs)? "" : _("\nCzcionka nie ma wszystkich znaków u¿ytych w tekœcie");
			console->AppendText(wxString::Format(_("Zakoñczono, znaleziono %i czcionek.\nNie znaleziono %i czcionek. "), found, nfound ) + noglyphs );}
		else{
			console->SetDefaultStyle(wxTextAttr("#008000"));
			console->AppendText(wxString::Format(_("Zakoñczono powodzeniem, znaleziono %i czcionek."), found));
		}
		fontfaces.clear();
		foundedfaces.clear();
		fontfiles.clear();
		return;
	}

	wxString copypath;
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
	if(opts->GetSelection()==1){
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
				console->AppendText(_("Skopiowano czcionkê o nazwie \"")+fontnames[i].AfterLast('\\')+"\".\n \n");
			}
			else
			{
				console->SetDefaultStyle(wxTextAttr(*wxRED));
				console->AppendText(_("Nie mo¿na skopiowaæ czcionki o nazwie \"")+fontnames[i].AfterLast('\\')+"\".\n \n");
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
			if(wxMessageBox(_("Plik zip ju¿ istnieje, usun¹æ go?"),_("Potwierdzenie"),wxYES_NO,this)==wxYES){
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
					console->AppendText(_("Skopiowano czcionkê o nazwie \"")+fontnames[i].AfterLast('\\')+"\".\n \n");
				}
				catch (...) {
					isgood=false;
				}

			}

			if(!isgood)
			{
				console->SetDefaultStyle(wxTextAttr(*wxRED));
				console->AppendText(_("Nie mo¿na spakowaæ czcionki o nazwie \"")+fontnames[i].AfterLast('\\')+"\".\n \n");
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
		console->AppendText(wxString::Format(_("Zakoñczono, skopiowano %i czcionek.\nNie uda³o siê skopiowaæ %i czcionek."), (int)fontnames.size(), nfound));}
	else{
		console->SetDefaultStyle(wxTextAttr("#008000"));
		console->AppendText(wxString::Format(_("Zakoñczono powodzeniem, skopiowano %i czcionek."), (int)fontnames.size()));
	}
}



void FontCollectorDialog::CopyMKVFonts()
{
	wxString mkvpath=Notebook::GetTab()->VideoPath;
	MatroskaWrapper mw;
	mw.Open(mkvpath);
	wxArrayString names=mw.GetFontList();
	if(names.size()<1){
		console->SetDefaultStyle(wxTextAttr(*wxRED));
		console->AppendText(_("Wczytany plik MKV nie ma ¿adnych czcionek."));
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
	//copypath.Replace("<subs dir>",Notebook::GetTab()->SubsPath.BeforeLast('\\'));
	//wxLogStatus("copy");
	size_t cpfonts=names.size();
	wxString onlypath;
	wxZipOutputStream *zip=NULL;
	wxFFileOutputStream *out=NULL;
	if(opts->GetSelection()==1){
		wxString extt=copypath.Right(4).Lower();
		if(extt==".zip"){copypath=copypath.BeforeLast('\\');}
		if(!wxDir::Exists(copypath)){wxDir::Make(copypath);}
		onlypath=copypath+"\\";
	}
	else
	{
		if(!wxDir::Exists(copypath.BeforeLast('\\'))){wxDir::Make(copypath.BeforeLast('\\'));}
		else if(wxFileExists(copypath)){
			if(wxMessageBox(_("Plik zip ju¿ istnieje, usun¹æ go?"),_("Potwierdzenie"),wxYES_NO,this)==wxYES){wxRemoveFile(copypath);}}
		out =new wxFFileOutputStream(copypath);
		zip = new wxZipOutputStream(*out);
	}
	for(size_t i=0; i<names.size(); i++){
		if(mw.SaveFont(i, onlypath+names[i], zip))
		{
			console->SetDefaultStyle(wxTextAttr(*wxBLACK));
			console->AppendText(_("Zapisano czcionkê o nazwie \"")+names[i]+"\".\n \n");
		}
		else
		{
			console->SetDefaultStyle(wxTextAttr(*wxRED));
			console->AppendText(_("Nie mo¿na zapisaæ czcionki o nazwie \"")+names[i]+"\".\n \n");
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
		console->AppendText(wxString::Format(_("Zakoñczono, skopiowano %i czcionek.\nNie uda³o siê skopiowaæ %i czcionek."), cpfonts, names.size() - cpfonts));}
	else{
		console->SetDefaultStyle(wxTextAttr("#008000"));
		console->AppendText(wxString::Format(_("Zakoñczono powodzeniem, skopiowano %i czcionek."), cpfonts));
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
		if(path->GetValue()==""&& !subsdir->GetValue()){
			wxMessageBox(_("Wybierz folder, gdzie maj¹ zostaæ skopiowane czcionki"));
			goto done;
		}
		if(opts->GetSelection()==2 && !path->GetValue().EndsWith(".zip") && !subsdir->GetValue()){
			wxMessageBox(_("Wybierz nazwê dla archiwum"));
			goto done;
		}
		if(fromMKV->GetValue() && fromMKV->IsEnabled()){CopyMKVFonts();}
		else{CopyFonts();}
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
	else
	{
		destdir= wxFileSelector(_("Wybierz nazwê archiwum"), (path->GetValue().EndsWith("zip"))? path->GetValue().BeforeLast('\\') : path->GetValue(), 
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
			rettxt<< _T("\"") << cur->first << _("\", który wystêpuje w linijkach:\n") << cur->second << _T(".\n");

		}
	}

	return rettxt;
}