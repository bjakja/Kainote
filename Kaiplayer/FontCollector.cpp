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
#include "FontEnumerator.h"
#include <wx/wfstream.h>
#include <wx/dir.h>
#include <wx/regex.h>
#include "KaiMessageBox.h"


SubsFont::SubsFont(const wxString &_name, const LOGFONTW &_logFont, int _bold, bool _italic){
	name           = _name; 
	logFont        = _logFont; 
	fakeBold       = false; 
	fakeItalic     = false; 
	fakeNormal     = false; 
	fakeBoldItalic = false;
	italic         = _italic ? -1 : 0;
	bold           = _bold == 1 ? 700 : _bold == 0 ? 400 : _bold;
}

LOGFONTW &SubsFont::GetLogFont(HDC dc)
{
	if(logFont.lfItalic != italic || logFont.lfWeight != bold){
		std::vector<LOGFONTW> logFonts;
		EnumFontFamiliesEx(dc, &logFont, (FONTENUMPROCW)[](const LOGFONT *lf, const TEXTMETRIC *mt, DWORD style, LPARAM lParam) -> int {
			std::vector<LOGFONTW>* fonts = reinterpret_cast<std::vector<LOGFONTW>*>(lParam);
			fonts->push_back(*lf);
			return 1;
		}, (LPARAM)&logFonts, 0);
		size_t lfssize = logFonts.size();
		bool BoldItalic = false;// int BII=-1;
		bool Bold = false;// int BI=-1;
		bool Italic = false;// int II=-1;
		bool Normal = false;// int NI=-1;
		for(size_t i=0; i < lfssize; i++){
			if((logFonts[i].lfWeight >= 700) && (0 != logFonts[i].lfItalic)){
				BoldItalic = true;// BII=i;
			}else if(logFonts[i].lfWeight >= 700){
				Bold = true;// BI=i;
			}else if(0 != logFonts[i].lfItalic){
				Italic = true;// II=i;
			}else{
				Normal = true;// NI=i;
			}//Normal || ((logFonts[i].lfWeight < 700) && (0 == logFonts[i].lfItalic));
		}
		fakeBoldItalic = (bold>=700 && italic!=0)? !BoldItalic : false;
		fakeBold = (bold>=700 && italic!=0)? !BoldItalic : (bold>=700)? !Bold : false;
		fakeItalic = (italic!=0)? !Italic : false;
		fakeNormal = (italic==0 && bold < 700)? !Normal : false;
		//int whichFont = (bold>=700 && italic!=0 && BII>=0)? BII : 
		//(bold>=700 && BI>=0)? BI : (italic!=0 && II>=0)? II : (NI>=0)? NI : 0;
		//logFont = logFonts[0];
		logFont.lfItalic=italic;
		logFont.lfWeight=bold;
	}

	return logFont;
}

FontCollectorDialog::FontCollectorDialog(wxWindow *parent)
	: KaiDialog(parent,-1,_("Kolekcjoner czcionek"),wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER)
	, zip(NULL)
{
	warning = Options.GetColour("Window Warning Elements");
	normal = Options.GetColour("Window Text");
	DialogSizer *Main = new DialogSizer(wxVERTICAL);
	wxBoxSizer *Pathc = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *Buttons = new wxBoxSizer(wxHORIZONTAL);
	wxIcon icn;
	icn.CopyFromBitmap(CreateBitmapFromPngResource("fontcollector"));
	SetIcon(icn);

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
	console=new wxTextCtrl(this,-1,"",wxDefaultPosition,wxSize(500,400),wxTE_RICH2|wxTE_MULTILINE|wxTE_READONLY);
	console->SetForegroundColour(normal);
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



void FontCollectorDialog::GetAssFonts(std::vector<bool> &found, bool check)
{
	if(facenames.size()<1){EnumerateFonts();}
	std::map<wxString, Styles*> stylesfonts;
	File *subs= Notebook::GetTab()->Grid1->file->GetSubs();
	wxRegEx reg("\\{[^\\{]*\\}",wxRE_ADVANCED);
	bool first = true;
	console->SetDefaultStyle(wxTextAttr(normal));
	for(size_t i=0; i<subs->styles.size(); i++)
	{
		wxString fn=subs->styles[i]->Fontname;
		bool bold = subs->styles[i]->Bold;
		bool italic = subs->styles[i]->Italic;
		wxString fnl = fn.Lower()<<(int)bold<<(int)italic;
		int iresult = facenames.Index(fn,false);
		if(iresult==-1){
			notFindFontsLog[fn]<<subs->styles[i]->Name<<",";
			continue;
		}
		else
		{
			//pamiętaj, dodaj jeszcze boldy i itaici
			if(!(foundFonts.find(fnl) != foundFonts.end())){
				foundFonts[fnl] = SubsFont(fn,logFonts[iresult],(int)bold,italic);
				found.push_back(check);
			}
			if(!(findFontsLog.find(fn)!=findFontsLog.end())){
				findFontsLog[fn]<<wxString::Format(_("Znaleziono czcionkę \"%s\" w stylach:\n- %s\n"), 
					fn, subs->styles[i]->Name);
			}else{
				findFontsLog[fn]<<"- "<<subs->styles[i]->Name<<"\n";
			}
		}
		stylesfonts[subs->styles[i]->Name]=subs->styles[i];

	}
	first=true;
	for(size_t i=0; i<subs->dials.size(); i++)
	{
		wxString text=subs->dials[i]->Text + subs->dials[i]->TextTl;
		Styles *lstyle = stylesfonts[subs->dials[i]->Style];
		wxString ifont = (lstyle)? lstyle->Fontname : "";
		int wlet=0;
		while(true)
		{
			wxString part = text.Mid(wlet);
			int flet=part.find("\\fn");
			//int hasBoldTag=part.find("\\b");
			//int hasItalicTag=part.find("\\i");

			if(flet!=-1)
			{
				wxString fn= part.Mid(flet+3).BeforeFirst('\\').BeforeFirst('}');
				if(fn.IsEmpty()){
					if(!ifont.IsEmpty()){
						wxString txt=text.SubString(wlet,flet)+"}";
						if(wlet!=0){txt+="{";};
						reg.ReplaceAll(&txt,"");
						PutChars(txt,ifont,i+1);
					}
					wlet= wlet+flet+3; continue;
				}
				bool bold = lstyle->Bold;
				bool italic = lstyle->Italic;
				wxString fnl = fn.Lower()<<(int)bold<<(int)italic;
				int iresult = facenames.Index(fn,false);
				if(iresult==-1){
					notFindFontsLog[fn]<<(i+1)<<",";
				}
				else
				{
					wxString txt=text.SubString(wlet,flet)+"}";
					if(wlet!=0){txt+="{";};
					reg.ReplaceAll(&txt,"");
					PutChars(txt,ifont,i+1);
					if(!(foundFonts.find(fnl) != foundFonts.end())){
						foundFonts[fnl] = SubsFont(fn,logFonts[iresult],(int)bold,italic);
						found.push_back(check);
					}
					auto log = findFontsLog.find(fn);
					if(!(log!=findFontsLog.end())){
						findFontsLog[fn]<<wxString::Format(_("Znaleziono czcionkę \"%s\" w linijkach:\n%i,"), 
							fn, (i+1));
					}else{
						if(log->second.EndsWith("\n")){
							log->second<<wxString::Format(_("W linijkach:\n%i,"),(i+1));
						}
						log->second<<(i+1)<<",";
					}
					
				}
				wlet= wlet+flet+3+fn.Len();
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



}

void FontCollectorDialog::CopyFonts(bool check)
{
	std::vector<bool> ffound;
	console->SetDefaultStyle(wxTextAttr(normal));
	if(!check && fontSizes.size()<1){
		fontfolder = wxGetOSDirectory() + "\\fonts\\";
		wxString seekpath = fontfolder+"*";
		//wxArrayString fontfiles;
		//wxDir::GetAllFiles(fontrealpath, &fontfiles, wxEmptyString, wxDIR_FILES);
		STime processTime(sw.Time());
		//console->AppendText(wxString::Format(_("Pobrano ścieżki czcionek z folderu fonts upłynęło %sms.\n"),processTime.GetFormatted(SRT)));
		//console->Update();
		/*bool notfound=false;
		long lSize=0;

		for(size_t i=0; i<fontfiles.size(); i++)
		{
			wxString ext=fontfiles[i].AfterLast('.').Lower();
			if(ext=="ini" || ext=="compositefont"){continue;}
			FILE *fp = _wfopen(fontfiles[i].wc_str(), L"rb");
			if(!fp){continue;}
			fseek (fp , 0 , SEEK_END);
			lSize = ftell (fp);
			fclose(fp);
			fontSizes.insert(std::pair<long, wxString>(lSize, fontfiles[i]));
		}
		fontfiles.clear();*/
		WIN32_FIND_DATAW data;
		HANDLE h = FindFirstFileW(seekpath.wc_str(), &data);
		if (h == INVALID_HANDLE_VALUE)
		{
			console->SetDefaultStyle(wxTextAttr(warning));
			console->AppendText(_("Windows nie chce zwrócić właściwości o czcionkach z kopiowania czcionek nici.\n"));
			return;
		}
		fontSizes.insert(std::pair<long, wxString>(data.nFileSizeLow, wxString(data.cFileName)));
		while(1){
			int result = FindNextFile(h, &data);
			if(result == ERROR_NO_MORE_FILES || result == 0){break;}
			else if(data.nFileSizeLow == 0){continue;} 
			fontSizes.insert(std::pair<long, wxString>(data.nFileSizeLow, wxString(data.cFileName)));
		}
		FindClose(h);
		processTime.NewTime(sw.Time());
		console->AppendText(wxString::Format(_("Pobrano rozmiary %i czcionek upłynęło %sms.\n"),fontSizes.size(), processTime.GetFormatted(SRT)));
		console->Update();
	}
	if(!check){
		if(subsdir->GetValue()){
			wxString rest;
			copypath=Notebook::GetTab()->SubsPath.BeforeLast('\\',&rest);
			copypath<<_("\\Czcionki");
			if(opts->GetSelection()==2){copypath<<"\\"<<rest.BeforeLast('.')<<".zip";}
		}else{
			copypath=path->GetValue();
			Options.SetString("Font Collect Directory",copypath);
		}
		wxString extt=copypath.Right(4).Lower();
		if(opts->GetSelection()!=2){
			if(extt==".zip"){copypath=copypath.BeforeLast('\\');}
			if(!wxDir::Exists(copypath)){
				wxDir::Make(copypath);
			}
		}else{
			if(!wxDir::Exists(copypath.BeforeLast('\\'))){
				wxDir::Make(copypath.BeforeLast('\\'));
			}else if(wxFileExists(copypath)){
				if(KaiMessageBox(_("Plik zip już istnieje, usunąć go?"),_("Potwierdzenie"),wxYES_NO,this)==wxYES){
					wxRemoveFile(copypath);
				}
			}
			wxFFileOutputStream *out = new wxFFileOutputStream(copypath);
			zip = new wxZipOutputStream(out,9);
		}
	}

	GetAssFonts(ffound,true);
	bool allglyphs = CheckPathAndGlyphs(check,ffound);

	console->SetDefaultStyle(wxTextAttr(warning));
	for(auto cur=notFindFontsLog.begin(); cur!=notFindFontsLog.end(); cur++){
		wxString list=cur->second;
		//wxLogStatus(list);
		wxStringTokenizer token(list,",", wxTOKEN_STRTOK);
		wxString result= _("Nie znaleziono czcionki \"") + cur->first + "\".\r\n";
		ffound.push_back(false);
		bool first=true;
		//wxLogStatus(cur->first+" "+cur->second);
		while(token.HasMoreTokens()){
			wxString tkn=token.GetNextToken();
			if(tkn.IsNumber()){
				//wxLogStatus("Token %i %i",token.GetPosition(),tkn.Len());
				result+= _("Użytej w linijkach: ") + list.Mid(token.GetPosition() - tkn.Len() - 1) + "\r\n";
				break;
			}else{
				if(first){result += _("Użytej w stylach:\r\n"); first=false;}
				result+=" - "+tkn+"\r\n";
			}
		}

		console->AppendText("\r\n"+result+"\r\n");
	}
	int found=0;
	int nfound=0;
	for(size_t i=0; i<ffound.size(); i++){
		if(!ffound[i]){
			nfound++;
		}else{found++;}
	}
	FontMap.clear();
	notFindFontsLog.clear();
	findFontsLog.clear();
	foundFonts.clear();
	if(zip){
		zip->Close();
		delete zip;
		zip = NULL;
	}
	wxString noglyphs= (allglyphs)? L"" : _("\nNiektóre czcionki nie mają wszystkich znaków użytych w tekście.");

	if(nfound||!allglyphs){
		console->SetDefaultStyle(wxTextAttr(warning));
		if(check){
			console->AppendText(wxString::Format(_("Zakończono, znaleziono %i czcionek.\nNie znaleziono %i czcionek. "), found, nfound ) + noglyphs );
		}else{
			console->AppendText(wxString::Format(_("Zakończono, skopiowano %i czcionek.\nNie udało się skopiować %i czcionek."), (int)fontnames.size(), nfound) + noglyphs);
		}
	}else{
		console->SetDefaultStyle(wxTextAttr("#008000"));
		if(check){
			console->AppendText(wxString::Format(_("Zakończono powodzeniem, znaleziono %i czcionek."), found));
		}else{
			console->AppendText(wxString::Format(_("Zakończono powodzeniem, skopiowano %i czcionek."), (int)fontnames.size()));
		}
	}

}
bool FontCollectorDialog::SaveFont(const wxString &fontname)
{
	wxString fn = fontname.AfterLast('\\');
	if(zip){
		wxFFileInputStream in(fontname);
		bool isgood=in.IsOk();
		if(isgood){
			try {
				zip->PutNextEntry(fn);
				zip->Write(in);
				console->SetDefaultStyle(wxTextAttr(normal));
				console->AppendText(wxString::Format(_("Dodano do archiwum czcionkę \"%s\".\n"),fn));
			}
			catch (...) {
				isgood=false;
			}

		}

		if(!isgood){
			console->SetDefaultStyle(wxTextAttr(warning));
			console->AppendText(wxString::Format(_("Nie można spakować czcionki \"%s\".\n"), fn));
		}
		return isgood;
	}else{
		if(wxCopyFile(fontname,copypath+"\\"+fn)){
			console->SetDefaultStyle(wxTextAttr(normal));
			console->AppendText(wxString::Format(_("Skopiowano czcionkę \"%s\".\n"),fn));
			return true;
		}
		else
		{
			console->SetDefaultStyle(wxTextAttr(warning));
			console->AppendText(wxString::Format(_("Nie można skopiować czcionki \"%i\".\n"),fn));
			return false;
		}
	}
}

void FontCollectorDialog::CopyMKVFonts()
{
	wxString mkvpath=Notebook::GetTab()->VideoPath;
	MatroskaWrapper mw;
	mw.Open(mkvpath);
	std::map<int, wxString> names=mw.GetFontList();
	if(names.size()<1){
		console->SetDefaultStyle(wxTextAttr(warning));
		console->AppendText(_("Wczytany plik MKV nie ma żadnych czcionek."));
		return;
	}

	wxString copypath;
	if(subsdir->GetValue()){
		wxString rest;
		copypath=Notebook::GetTab()->SubsPath.BeforeLast('\\',&rest);
		copypath<<_("\\Czcionki");
		if(opts->GetSelection()==2){copypath<<"\\"<<rest.BeforeLast('.')<<".zip";}
	}else{
		copypath=path->GetValue();
		//Options.SetString("Font Collect Directory",copypath);
	}

	Options.SetString("Font Collect Directory",copypath);

	size_t cpfonts=names.size();
	wxString onlypath;
	zip=NULL;
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
		out = new wxFFileOutputStream(copypath);
		zip = new wxZipOutputStream(out,9);
	}
	for(auto fontI : names){
		if(mw.SaveFont(fontI.first, onlypath + fontI.second, zip))
		{
			console->SetDefaultStyle(wxTextAttr(normal));
			console->AppendText(_("Zapisano czcionkę o nazwie \"")+ fontI.second +"\".\n \n");
		}
		else
		{
			console->SetDefaultStyle(wxTextAttr(warning));
			console->AppendText(_("Nie można zapisać czcionki o nazwie \"")+ fontI.second +"\".\n \n");
			cpfonts--;
		}
	}
	if(zip)
	{
		zip->Close();
		delete zip;
		zip=NULL;
		//delete out;
	}
	if(cpfonts<names.size()){
		console->SetDefaultStyle(wxTextAttr(warning));
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
	sw.Start();
	console->SetValue("");
	path->Enable(false);
	choosepath->Enable(false);
	subsdir->Enable(false);
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
	opts->Enable(true);
	path->Enable(opts->GetSelection()!=0);
	choosepath->Enable(opts->GetSelection()!=0);
	subsdir->Enable(opts->GetSelection()!=0);
	bok->Enable(true);
	bcancel->Enable(true);
	console->SetDefaultStyle(normal);
	STime processTime(sw.Time());
	console->AppendText(wxString::Format(_("\n\nZakończono w %sms"), processTime.GetFormatted(SRT)));
}

void FontCollectorDialog::OnChangeOpt(wxCommandEvent &event)
{
	path->Enable(opts->GetSelection()!=0);
	path->Refresh(false);
	choosepath->Enable(opts->GetSelection()!=0);
	choosepath->Refresh(false);
	subsdir->Enable(opts->GetSelection()!=0);
	subsdir->Refresh(false);

	fromMKV->Enable(opts->GetSelection()!=0 && Notebook::GetTab()->VideoPath.Lower().EndsWith(".mkv"));
	fromMKV->Refresh(false);
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
	//txt.erase(std::unique(txt.begin(), txt.end()), txt.end());
	CharMap ch=FontMap[fn];

	for(size_t i=0; i<txt.Len(); i++)
	{
		if(!(ch.find(txt[i])!=ch.end())){
			ch[txt[i]] ="";//<< move << " ";
		}
	}
	FontMap[fn]=ch;
}

void FontCollectorDialog::EnumerateFonts()
{
	LOGFONTW lf;
	lf.lfCharSet = DEFAULT_CHARSET;
	lf.lfPitchAndFamily = 0;
	HDC dc = ::CreateCompatibleDC(NULL);
	memcpy(lf.lfFaceName, L"\0", LF_FACESIZE);
	EnumFontFamiliesEx(dc, &lf, (FONTENUMPROCW)[](const LOGFONT *lf, const TEXTMETRIC *mt, DWORD style, LPARAM lParam) -> int {
		FontCollectorDialog * dial = reinterpret_cast<FontCollectorDialog*>(lParam);
		dial->facenames.push_back(lf->lfFaceName);
		dial->logFonts.push_back(*lf);
		return 1;
	}, (LPARAM)this, 0);
}

bool FontCollectorDialog::CheckPathAndGlyphs(bool onlyGlyphs, std::vector<bool> &found)
{
	bool allfound=true;
	HDC dc = ::CreateCompatibleDC(NULL);
	auto it = foundFonts.begin();
	wxString lastfn;
	for(size_t k = 0; k < foundFonts.size(); k++){
		LOGFONTW mlf = it->second.GetLogFont(dc);
		wxString fn = it->second.name;
		if(lastfn != fn){
			console->Update();
			console->SetDefaultStyle(wxTextAttr(normal));
			console->AppendText("\n \n"+findFontsLog[fn].RemoveLast()+"\n");
			console->SetDefaultStyle(wxTextAttr(warning));
		}
		lastfn=fn;
		SubsFont &font = it->second;
		it++;
		auto hfont = CreateFontIndirectW(&mlf);
		SelectObject(dc, hfont);
		if(font.fakeNormal){
			console->AppendText(wxString::Format(_("Czcionka \"%s\" nie ma stylu normalnego.\n"), fn));
		}else if(font.fakeBold){
			console->AppendText(wxString::Format(_("Czcionka \"%s\" nie ma pogrubienia.\n"), fn));
		}else if(font.fakeItalic){
			console->AppendText(wxString::Format(_("Czcionka \"%s\" nie ma kursywy.\n"), fn));
		}else if(font.fakeBoldItalic){
			console->AppendText(wxString::Format(_("Czcionka \"%s\" nie ma pogrubienia z kursywą.\n"), fn));
		}
		wxString text;
		wxString missing;
		CharMap ch=FontMap[fn];
		for(auto it = ch.begin(); it!=ch.end(); it++)
		{
			text<<it->first;
		}
		if(!FontEnum.CheckGlyphsExists(dc, text, missing))
		{
			console->AppendText(wxString::Format(_("Nie można sprawdzić znaków czcionki \"%s\".\n"), fn));
		}
		if(missing.Len()>0){
			allfound=false;
			console->AppendText(wxString::Format(_("Czcionka \"%s\" nie zawiera znaków: \"%s\".\n"), fn, missing));
		}
		if(!onlyGlyphs){
			DWORD ttcf = 0x66637474;
			auto size = GetFontData(dc, ttcf, 0, nullptr, 0);
			if (size == GDI_ERROR) {
				ttcf = 0;
				size = GetFontData(dc, 0, 0, nullptr, 0);
			}
			if (size == GDI_ERROR || size == 0){
				console->AppendText(wxString::Format(_("Nie można pobrać zawartości czcionki \"%s\".\n"), fn));
				found[k] = false;
			}else{
				std::string buffer;
				buffer.resize(size);
				GetFontData(dc, ttcf, 0, &buffer[0], (int)size);
				std::string file_buffer;
				file_buffer.resize(size);
				bool succeeded = false;

				for (auto it=fontSizes.equal_range(size).first; it!=fontSizes.equal_range(size).second; ++it){
					wxString fullpath = fontfolder + it->second;
					FILE *fp = _wfopen(fullpath.wc_str(), L"rb");
					if(!fp){continue;}
					fseek (fp , 0 , SEEK_END);
					long lSize = ftell (fp);
					rewind (fp);
					if(lSize != size){
						console->AppendText(wxString::Format(_("Rozmiar czcionki \"%s\" się różni.\n"), fn));
						found[k] = false;
						fclose(fp); 
						continue;
					}
					int result = fread (&file_buffer[0],1,size,fp);
					if(result != size){
						console->AppendText(wxString::Format(_("Nie można odczytać czcionki \"%s\" z folderu fonts.\n"), fn));
						found[k] = false;
					}
					fclose(fp);
					if (memcmp(&file_buffer[0], &buffer[0], size) == 0) {
						if(fontnames.Index(fullpath,true)==-1){
							fontnames.Add(fullpath);
							console->SetDefaultStyle(wxTextAttr(normal));
							console->AppendText(wxString::Format(_("Znaleziono plik czcionki \"%s\".\n"), fullpath));
							SaveFont(fullpath);
							wxString ext = it->second.AfterLast('.').Lower();
							if(ext == "pfm" || ext == "pfb"){
								wxString repl = (ext == "pfm")? "pfb" : "pfm";
								//char ch = 'a';
								if(fullpath[fullpath.Len()-1] < 'Z'){repl = repl.Upper();}
								wxString secondPath = fullpath.RemoveLast(3) + repl;
								fontnames.Add(secondPath);
								console->AppendText(wxString::Format(_("Znaleziono plik czcionki \"%s\".\n"), secondPath));
								SaveFont(secondPath);
							}
							console->SetDefaultStyle(wxTextAttr(warning));
						}//else{
						//fake italic/bold font when another normal is added
						//must succeed but not add a path
						//}

						succeeded = true;
						found[k] = true;
						break;
					}
				}
				if(!succeeded){
					console->AppendText(wxString::Format(_("Nie można znaleźć czcionki \"%s\" w folderze fonts.\n"), fn));
					found[k] = false;
				}
			}

		}
		SelectObject(dc, NULL);
		DeleteObject(hfont);
	}
	::DeleteDC(dc);
	return allfound;
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