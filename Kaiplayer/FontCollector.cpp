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

wxDEFINE_EVENT(EVT_APPEND_MESSAGE, wxThreadEvent);
wxDEFINE_EVENT(EVT_ENABLE_BUTTONS, wxThreadEvent);

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

FontCollectorDialog::FontCollectorDialog(wxWindow *parent, FontCollector *_fc)
	: KaiDialog(parent,-1,_("Kolekcjoner czcionek"),wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER)
	,fc(_fc)
{
	warning = Options.GetColour(WindowWarningElements);
	normal = Options.GetColour(WindowText);
	DialogSizer *Main = new DialogSizer(wxVERTICAL);
	wxBoxSizer *Pathc = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *Buttons = new wxBoxSizer(wxHORIZONTAL);
	wxIcon icn;
	icn.CopyFromBitmap(CreateBitmapFromPngResource("fontcollector"));
	SetIcon(icn);

	path=new KaiTextCtrl(this,-1,Options.GetString(FontCollectorDirectory),wxDefaultPosition, wxSize(150,-1));
	path->Enable(Options.GetInt(FontCollectorAction)!=0);
	//path->SetToolTip("Można też wybrać folder napisów wpisując <subs dir>.");
	choosepath=new MappedButton(this,8799,_("Wybierz folder"));
	choosepath->Enable(Options.GetInt(FontCollectorAction)!=0);
	Connect(8799,wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&FontCollectorDialog::OnButtonPath);

	Pathc->Add(path,1,wxEXPAND|wxALL,3);
	Pathc->Add(choosepath,0,wxBOTTOM|wxTOP|wxRIGHT,3);

	wxArrayString choices;
	choices.Add(_("Sprawdź dostępność czcionek"));
	choices.Add(_("Skopiuj do wybranego folderu"));
	choices.Add(_("Spakuj zipem"));
	choices.Add(_("Wmuxuj napisy w wideo (wymagany MKVToolnix)"));
	opts=new KaiRadioBox(this,9987,_("Opcje"),wxDefaultPosition,wxDefaultSize,choices,0,wxRA_SPECIFY_ROWS);
	opts->SetSelection(Options.GetInt(FontCollectorAction));
	Connect(9987,wxEVT_COMMAND_RADIOBOX_SELECTED, (wxObjectEventFunction)&FontCollectorDialog::OnChangeOpt);

	subsdir=new KaiCheckBox(this,7998,_("Zapisuj do folderu z napisami"));
	subsdir->Enable(Options.GetInt(FontCollectorAction)!=0);
	subsdir->SetValue(Options.GetBool(FontCollectorUseSubsDirectory));


	fromMKV=new KaiCheckBox(this,7991,_("Wyciągnij czcionki z wczytanego pliku MKV"));
	fromMKV->Enable(Notebook::GetTab()->VideoPath.Lower().EndsWith(".mkv"));
	fromMKV->SetValue(Options.GetBool(FontCollectorFromMKV));

	Connect(7998,wxEVT_COMMAND_CHECKBOX_CLICKED, (wxObjectEventFunction)&FontCollectorDialog::OnChangeOpt);
	console=new wxTextCtrl(this,-1,"",wxDefaultPosition,wxSize(500,400),wxTE_RICH2|wxTE_MULTILINE|wxTE_READONLY);
	console->SetForegroundColour(normal);
	console->SetBackgroundColour(Options.GetColour(WindowBackground));
	bok=new MappedButton(this,9879,_("Rozpocznij"));
	bok->SetFocus();
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

	Bind(EVT_APPEND_MESSAGE,[=](wxThreadEvent evt){
		std::pair<wxString, wxColour> *data = evt.GetPayload<std::pair<wxString, wxColour>*>();
		console->SetDefaultStyle(wxTextAttr(data->second));
		console->AppendText(data->first);
		delete data;
	});
	Bind(EVT_ENABLE_BUTTONS,[=](wxThreadEvent evt){
		EnableControls();
	});
	SetSizerAndFit(Main);
	CenterOnParent();
}

FontCollectorDialog::~FontCollectorDialog(){};

void FontCollectorDialog::EnableControls()
{
	opts->Enable(true);
	path->Enable(opts->GetSelection()!=0);
	choosepath->Enable(opts->GetSelection()!=0);
	subsdir->Enable(opts->GetSelection()!=0);
	bok->Enable(true);
	bcancel->Enable(true);
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
	Options.SetString(FontCollectorDirectory,destdir);
	Options.SaveOptions(true,false);
	path->SetValue(destdir);
}

void FontCollectorDialog::OnButtonStart(wxCommandEvent &event)
{
	console->SetValue("");
	path->Enable(false);
	choosepath->Enable(false);
	subsdir->Enable(false);
	opts->Enable(false);
	bok->Enable(false);
	bcancel->Enable(false);
	if(opts->GetSelection()==0)
	{
		fc->StartCollect(1);
	}
	else
	{
		if(opts->GetSelection()==3 && ( Notebook::GetTab()->VideoPath=="" || Notebook::GetTab()->SubsPath=="" )){
			KaiMessageBox(_("Brak wczytanego wideo bądź napisów"));return;
		}
		if(path->GetValue()=="" && !subsdir->GetValue() ){
			KaiMessageBox(_("Wybierz folder, gdzie mają zostać skopiowane czcionki"));
			EnableControls();
			path->SetFocus();
			return;
		}
		if(subsdir->GetValue() && Notebook::GetTab()->SubsPath==""){
			KaiMessageBox(_("Brak wczytanych napisów, wczytaj napisy albo odznacz tę opcję."));
			EnableControls();
			return;
		}
		if(opts->GetSelection()==2 && path->GetValue().EndsWith("\\") && !subsdir->GetValue()){
			KaiMessageBox(_("Wybierz nazwę dla archiwum"));
			EnableControls();
			path->SetFocus();
			return;
		}
		if(subsdir->GetValue()){
			wxString rest;
			copypath=Notebook::GetTab()->SubsPath.BeforeLast('\\',&rest);
			copypath<<"\\Czcionki\\";
			if(opts->GetSelection()==2){copypath<<rest.BeforeLast('.')<<".zip";}
		}else{
			copypath=path->GetValue();
			if(opts->GetSelection()!=2 && !copypath.EndsWith("\\")){copypath<<"\\";}
			else if(opts->GetSelection()==2 && !copypath.EndsWith(".zip")){copypath<<".zip";}
		}
		if(opts->GetSelection()!=2){
			wxString extt=copypath.Right(4).Lower();
			if(extt==".zip"){copypath=copypath.BeforeLast('\\')+"\\";}
			if(!wxDir::Exists(copypath)){wxDir::Make(copypath);}
		}else{
			if(!wxDir::Exists(copypath.BeforeLast('\\'))){wxDir::Make(copypath.BeforeLast('\\'));}
			else if(wxFileExists(copypath)){
				if(KaiMessageBox(_("Plik zip już istnieje, usunąć go?"),_("Potwierdzenie"),wxYES_NO,this)==wxYES){
					wxRemoveFile(copypath);
				}
			}
		}
		Options.SetString(FontCollectorDirectory,copypath);

		int operation = (fromMKV->GetValue() && fromMKV->IsEnabled())? FontCollector::COPY_MKV_FONTS : 
			(opts->GetSelection()==3)? FontCollector::MUX_VIDEO_WITH_SUBS : FontCollector::COPY_FONTS;
		if(opts->GetSelection()==2){operation |= FontCollector::AS_ZIP;}
		if(opts->GetSelection()==3){
			fc->muxerpath=L"C:\\Program Files\\MKVtoolnix\\mkvmerge.exe";
			if(!wxFileExists(fc->muxerpath)){
				wxFileDialog *fd = new wxFileDialog(this,_("Wybierz plik mkvmerge.exe"), L"C:\\Program Files",L"mkvmerge.exe",_("Programy (.exe)|*.exe"),wxFD_OPEN|wxFD_FILE_MUST_EXIST);
				if(fd->ShowModal()!=wxID_OK){
					KaiMessageBox(_("Muxowanie zostało anulowane, bo nie wybrano mkvmerge.exe"));fd->Destroy();return;
				}
				fc->muxerpath=fd->GetPath();
				fd->Destroy();
			}
		}
		fc->StartCollect(operation);
	}
}

void FontCollectorDialog::OnChangeOpt(wxCommandEvent &event)
{
	path->Enable(opts->GetSelection()!=0);
	choosepath->Enable(opts->GetSelection()!=0);
	subsdir->Enable(opts->GetSelection()!=0);

	fromMKV->Enable(opts->GetSelection()!=0 && Notebook::GetTab()->VideoPath.Lower().EndsWith(".mkv"));
	Options.SetInt(FontCollectorAction,opts->GetSelection());
	Options.SetBool(FontCollectorUseSubsDirectory,subsdir->GetValue());
	Options.SaveOptions(true,false);
}

FontCollector::FontCollector(wxWindow *parent)
	:zip(NULL)
	,fcd(NULL)
	,reloadFonts(false)
{
	FontEnum.AddClient(parent, [=](){reloadFonts=true;});
}

FontCollector::~FontCollector()
{
	//fcd->Destroy();
};

void FontCollector::GetAssFonts(std::vector<bool> &found, bool check)
{
	if(facenames.size()<1 || reloadFonts){EnumerateFonts();}
	std::map<wxString, Styles*> stylesfonts;
	File *subs= Notebook::GetTab()->Grid1->file->GetSubs();
	
	
	for(size_t i=0; i<subs->styles.size(); i++)
	{
		wxString fn=subs->styles[i]->Fontname;
		bool bold = subs->styles[i]->Bold;
		bool italic = subs->styles[i]->Italic;
		wxString fnl = fn.Lower()<<(int)bold<<(int)italic;
		int iresult = facenames.Index(fn,false);
		if(iresult==-1){
			notFindFontsLog[fn]<<subs->styles[i]->Name<<",";
			//continue;
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

	for(size_t i=0; i<subs->dials.size(); i++)
	{
		Dialogue *dial = subs->dials[i];
		if(dial->IsComment){continue;}
		dial->ParseTags("fn|b|i|p", true);
		ParseData *pdata = dial->pdata;
		if(!pdata){continue;}
		
		wxString text = (dial->TextTl!="")? dial->TextTl : dial->Text;

		Styles *lstyle = stylesfonts[dial->Style];
		//if(!lstyle){
			//SendMessageD(wxString::Format(_("Styl '%s' z linii %i nie istnieje.\n"), dial->Style, (int)i-1), fcd->warning);
		//}
		wxString ifont = (lstyle)? lstyle->Fontname : "";
		int bold = (lstyle)? (int)lstyle->Bold : 0;
		int italic = (lstyle)? (int)lstyle->Italic : 0;
		int wlet=0;
		bool newFont = false;
		bool lastPlain = false;
		wxString textHavingFont;
		size_t tagsSize = pdata->tags.size();

		for(size_t j = 0; j < tagsSize; j++)
		{
			TagData *tag = pdata->tags[j];
			if(tag->tagName == "p"){continue;}
				
			if(tag->tagName == "plain"){
				textHavingFont += tag->value;
				if((lastPlain && j<tagsSize-1) || ifont.IsEmpty()){continue;}
				lastPlain=true;
				wxString fnl = ifont.Lower() << bold << italic;
				int iresult = facenames.Index(ifont,false);
				if(iresult==-1){
					notFindFontsLog[ifont]<<(i+1)<<",";
				}
				else
				{
					//wxLogStatus(textHavingFont);
					textHavingFont.Replace("\\N","");
					textHavingFont.Replace("\\n","");
					textHavingFont.Replace("\\h"," ");
					PutChars(textHavingFont,ifont);
					if(!(foundFonts.find(fnl) != foundFonts.end())){
						foundFonts[fnl] = SubsFont(ifont,logFonts[iresult],bold,(italic!=0));
						found.push_back(check);
					}
					if(newFont){
						auto log = findFontsLog.find(ifont);
						if(!(log!=findFontsLog.end())){
							findFontsLog[ifont]<<wxString::Format(_("Znaleziono czcionkę \"%s\" w linijkach:\n%i,"), 
								ifont, (int)(i+1));
						}else{
							if(log->second.EndsWith("\n")){
								log->second<<wxString::Format(_("W linijkach:\n%i,"),(int)(i+1));
							}
							log->second<<(i+1)<<",";
						}
						newFont=false;
					}
				}
				textHavingFont.Empty();
			}else{
				if(tag->tagName == "fn"){
					ifont = tag->value;
					newFont = true;
				}else if(tag->tagName == "b"){
					bold = wxAtoi(tag->value);
				}else if(tag->tagName == "i"){
					italic = wxAtoi(tag->value);
				}
				lastPlain=false;
			}
			
			
		}
		dial->ClearParse();
	}

}

bool FontCollector::AddFont(const wxString &string)
{
	return true;
}

void FontCollector::CopyFonts()
{
	std::vector<bool> ffound;
	if(!(operation & CHECK_FONTS) && (fontSizes.size()<1 || reloadFonts)){
		fontfolder = wxGetOSDirectory() + "\\fonts\\";
		wxString seekpath = fontfolder+"*";
		
		fontSizes.clear();
		WIN32_FIND_DATAW data;
		HANDLE h = FindFirstFileW(seekpath.wc_str(), &data);
		if (h == INVALID_HANDLE_VALUE)
		{
			SendMessageD(_("Nie można pobrać rozmiarów i nazw plików czcionek\nkopiowanie zostaje przerwane.\n"),fcd->warning);
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
		STime processTime(sw.Time());
		SendMessageD(wxString::Format(_("Pobrano rozmiary i nazwy %i czcionek upłynęło %sms.\n"),(int)fontSizes.size()-2, processTime.GetFormatted(SRT)),fcd->normal);
	}
	if(operation &AS_ZIP){
		wxFFileOutputStream *out = new wxFFileOutputStream(fcd->copypath);
		zip = new wxZipOutputStream(out,9);
		
	}

	GetAssFonts(ffound,true);
	bool allglyphs = CheckPathAndGlyphs(ffound);

	for(auto cur=notFindFontsLog.begin(); cur!=notFindFontsLog.end(); cur++){
		wxString list=cur->second;
		wxStringTokenizer token(list,",", wxTOKEN_STRTOK);
		wxString result= _("Nie znaleziono czcionki \"") + cur->first + "\".\n";
		ffound.push_back(false);
		bool first=true;
		while(token.HasMoreTokens()){
			wxString tkn=token.GetNextToken();
			if(tkn.IsNumber()){
				result+= _("Użytej w linijkach: ") + list.Mid(token.GetPosition() - tkn.Len() - 1) + "\n";
				break;
			}else{
				if(first){result += _("Użytej w stylach:\n"); first=false;}
				result+=" - "+tkn+"\n";
			}
		}

		SendMessageD("\n"+result+"\n", fcd->warning);
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
		if((operation & CHECK_FONTS)){
			SendMessageD(wxString::Format(_("Zakończono, znaleziono %s.\nNie znaleziono %s. "), 
				MakePolishPlural(found,_("czcionkę"),_("czcionki"),_("czcionek")), 
				MakePolishPlural(nfound,_("czcionki"),_("czcionki"),_("czcionek"))) + noglyphs, fcd->warning );
		}else{
			SendMessageD(wxString::Format(_("Zakończono, skopiowano %s.\nNie znaleziono/nie udało się skopiować %s."),
				MakePolishPlural((int)fontnames.size(),_("czcionkę"),_("czcionki"),_("czcionek")), 
				MakePolishPlural(nfound,_("czcionki"),_("czcionki"),_("czcionek"))) + noglyphs, fcd->warning);
		}
	}else{
		if((operation & CHECK_FONTS)){
			SendMessageD(wxString::Format(_("Zakończono powodzeniem, znaleziono %s."), 
				MakePolishPlural(found,_("czcionkę"),_("czcionki"),_("czcionek"))), wxColour("#008000"));
		}else{
			SendMessageD(wxString::Format(_("Zakończono powodzeniem, skopiowano %s."), 
				MakePolishPlural((int)fontnames.size(),_("czcionkę"),_("czcionki"),_("czcionek"))), wxColour("#008000"));
		}
	}
	fontnames.clear();
}
bool FontCollector::SaveFont(const wxString &fontPath)
{
	wxString fn = fontPath.AfterLast('\\');
	if(zip){
		wxFFileInputStream in(fontPath);
		bool isgood=in.IsOk();
		if(isgood){
			try {
				zip->PutNextEntry(fn);
				zip->Write(in);
				SendMessageD(wxString::Format(_("Dodano do archiwum czcionkę \"%s\".\n"),fn),fcd->normal);
			}
			catch (...) {
				isgood=false;
			}

		}

		if(!isgood){
			SendMessageD(wxString::Format(_("Nie można spakować czcionki \"%s\".\n"), fn),fcd->warning);
		}
		return isgood;
	}else{
		if(wxCopyFile(fontPath,fcd->copypath+"\\"+fn)){
			SendMessageD(wxString::Format(_("Skopiowano czcionkę \"%s\".\n"),fn),fcd->normal);
			return true;
		}
		else
		{
			SendMessageD(wxString::Format(_("Nie można skopiować czcionki \"%s\".\n"),fn),fcd->warning);
			return false;
		}
	}
}

void FontCollector::CopyMKVFonts()
{
	wxString mkvpath=Notebook::GetTab()->VideoPath;
	MatroskaWrapper mw;
	mw.Open(mkvpath);
	std::map<int, wxString> names=mw.GetFontList();
	if(names.size()<1){
		SendMessageD(_("Wczytany plik MKV nie ma żadnych czcionek."),fcd->warning);
		return;
	}

	

	size_t cpfonts=names.size();
	zip=NULL;

	if(operation & AS_ZIP){
		wxFFileOutputStream *out = new wxFFileOutputStream(fcd->copypath);
		zip = new wxZipOutputStream(out,9);
	}
	for(auto fontI : names){
		if(mw.SaveFont(fontI.first, fcd->copypath.BeforeLast('\\') + "\\" + fontI.second, zip))
		{
			SendMessageD(_("Zapisano czcionkę o nazwie \"")+ fontI.second +"\".\n \n",fcd->normal);
		}
		else
		{
			SendMessageD(_("Nie można zapisać czcionki o nazwie \"")+ fontI.second +"\".\n \n",fcd->warning);
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
		SendMessageD(wxString::Format(_("Zakończono, skopiowano %i czcionek.\nNie udało się skopiować %i czcionek."), (int)cpfonts, (int)(names.size() - cpfonts)),fcd->warning);}
	else{
		SendMessageD(wxString::Format(_("Zakończono powodzeniem, skopiowano %i czcionek."), (int)cpfonts), wxColour("#008000"));
	}

	names.clear();
	mw.Close();
}




void FontCollector::PutChars(const wxString &txt, const wxString &fn)
{
	CharMap &ch=FontMap[fn];

	for(size_t i=0; i<txt.Len(); i++)
	{
		if(!(ch.find(txt[i])!=ch.end())){
			ch.insert(txt[i]);//<< move << " ";
		}
	}
}

void FontCollector::EnumerateFonts()
{
	facenames.clear();
	logFonts.clear();
	LOGFONTW lf;
	lf.lfCharSet = DEFAULT_CHARSET;
	lf.lfPitchAndFamily = 0;
	HDC dc = ::CreateCompatibleDC(NULL);
	memcpy(lf.lfFaceName, L"\0", LF_FACESIZE);
	EnumFontFamiliesEx(dc, &lf, (FONTENUMPROCW)[](const LOGFONT *lf, const TEXTMETRIC *mt, DWORD style, LPARAM lParam) -> int {
		FontCollector * fc = reinterpret_cast<FontCollector*>(lParam);
		fc->facenames.push_back(lf->lfFaceName);
		fc->logFonts.push_back(*lf);
		return 1;
	}, (LPARAM)this, 0);
}

bool FontCollector::CheckPathAndGlyphs(std::vector<bool> &found)
{
	bool allfound=true;
	HDC dc = ::CreateCompatibleDC(NULL);
	auto it = foundFonts.begin();
	wxString lastfn;
	for(size_t k = 0; k < foundFonts.size(); k++){
		LOGFONTW mlf = it->second.GetLogFont(dc);
		wxString fn = it->second.name;
		if(lastfn != fn){
			SendMessageD("\n \n"+findFontsLog[fn].RemoveLast()+"\n",fcd->normal);
		}
		lastfn=fn;
		SubsFont &font = it->second;
		it++;
		auto hfont = CreateFontIndirectW(&mlf);
		SelectObject(dc, hfont);
		if(font.fakeNormal){
			SendMessageD(wxString::Format(_("Czcionka \"%s\" nie ma stylu normalnego.\n"), fn),fcd->warning);
		}else if(font.fakeBoldItalic){
			SendMessageD(wxString::Format(_("Czcionka \"%s\" nie ma pogrubienia z kursywą.\n"), fn),fcd->warning);
		}else if(font.fakeBold){
			SendMessageD(wxString::Format(_("Czcionka \"%s\" nie ma pogrubienia.\n"), fn),fcd->warning);
		}else if(font.fakeItalic){
			SendMessageD(wxString::Format(_("Czcionka \"%s\" nie ma kursywy.\n"), fn),fcd->warning);
		}
		if(lastfn != fn){
			wxString text;
			wxString missing;
			CharMap ch=FontMap[fn];
			for(auto it = ch.begin(); it!=ch.end(); it++)
			{
				text<<(*it);
			}
			if(!FontEnum.CheckGlyphsExists(dc, text, missing))
			{
				SendMessageD(wxString::Format(_("Nie można sprawdzić znaków czcionki \"%s\".\n"), fn),fcd->warning);
			}
			if(missing.Len()>0){
				allfound=false;
				SendMessageD(wxString::Format(_("Czcionka \"%s\" nie zawiera znaków: \"%s\".\n"), fn, missing),fcd->warning);
			}
		}
		if(!(operation & CHECK_FONTS)){
			DWORD ttcf = 0x66637474;
			auto size = GetFontData(dc, ttcf, 0, nullptr, 0);
			if (size == GDI_ERROR) {
				ttcf = 0;
				size = GetFontData(dc, 0, 0, nullptr, 0);
			}
			if (size == GDI_ERROR || size == 0){
				SendMessageD(wxString::Format(_("Nie można pobrać zawartości czcionki \"%s\".\n"), fn),fcd->warning);
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
						SendMessageD(wxString::Format(_("Rozmiar czcionki \"%s\" się różni.\n"), fn),fcd->warning);
						found[k] = false;
						fclose(fp); 
						continue;
					}
					int result = fread (&file_buffer[0],1,size,fp);
					if(result != size){
						SendMessageD(wxString::Format(_("Nie można odczytać czcionki \"%s\" z folderu fonts.\n"), fn),fcd->warning);
						found[k] = false;
					}
					fclose(fp);
					if (memcmp(&file_buffer[0], &buffer[0], size) == 0) {
						if(fontnames.Index(fullpath,true)==-1){
							fontnames.Add(fullpath);
							SendMessageD(wxString::Format(_("Znaleziono plik czcionki \"%s\".\n"), fullpath),fcd->normal);
							if(operation & COPY_FONTS){SaveFont(fullpath);}
							wxString ext = it->second.AfterLast('.').Lower();
							if(ext == "pfm" || ext == "pfb"){
								wxString repl = (ext == "pfm")? "pfb" : "pfm";
								//char ch = 'a';
								if(fullpath[fullpath.Len()-1] < 'Z'){repl = repl.Upper();}
								wxString secondPath = fullpath.RemoveLast(3) + repl;
								fontnames.Add(secondPath);
								SendMessageD(wxString::Format(_("Znaleziono plik czcionki \"%s\".\n"), secondPath),fcd->normal);
								if(operation & COPY_FONTS){SaveFont(secondPath);}
							}
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
					SendMessageD(wxString::Format(_("Nie można znaleźć czcionki \"%s\" w folderze fonts.\n"), fn),fcd->warning);
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

void FontCollector::MuxVideoWithSubs()
{
	

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
		command << fcd->copypath << L"\\" << name << L"\" ";
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

void FontCollector::ShowDialog(wxWindow *parent)
{
	fcd = new FontCollectorDialog(parent,this);
	fcd->ShowModal();
	fcd->Destroy();
	fcd = NULL;
}

void FontCollector::StartCollect(int _operation)
{
	operation = _operation;
	FontCollectorThread *ft = new FontCollectorThread(this);
}

void FontCollector::SendMessageD(wxString string, wxColour col)
{
	wxThreadEvent *evt = new wxThreadEvent(EVT_APPEND_MESSAGE, fcd->GetId());
	evt->SetPayload(new std::pair<wxString, wxColour>(string, col));
	wxQueueEvent(fcd, evt);
}

FontCollectorThread::FontCollectorThread(FontCollector *_fc)
	:wxThread(wxTHREAD_DETACHED)
{
	fc=_fc;
	Create();
	Run();
}

wxThread::ExitCode FontCollectorThread::Entry()
{
	fc->sw.Start();
	if(fc->operation &FontCollector::CHECK_FONTS || fc->operation &FontCollector::COPY_FONTS){
		fc->CopyFonts();
	}else if(fc->operation &FontCollector::COPY_MKV_FONTS){
		fc->CopyMKVFonts();
	}else if(fc->operation &FontCollector::MUX_VIDEO_WITH_SUBS){
		fc->CopyFonts();
		fc->MuxVideoWithSubs();
	}else{
		fc->sw.Pause(); return 0;
	}

	wxThreadEvent *evt = new wxThreadEvent(EVT_ENABLE_BUTTONS, fc->fcd->GetId());
	wxQueueEvent(fc->fcd, evt);
	STime processTime(fc->sw.Time());
	fc->SendMessageD(wxString::Format(_("\n\nZakończono w %sms"), processTime.GetFormatted(SRT)), fc->fcd->normal);
	fc->sw.Pause();
	return 0;
}