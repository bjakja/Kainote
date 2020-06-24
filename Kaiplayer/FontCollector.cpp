//  Copyright (c) 2016 - 2020, Marcin Drob

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
#include <ShlObj.h>
#include "Utils.h"

wxDEFINE_EVENT(EVT_APPEND_MESSAGE, wxThreadEvent);
wxDEFINE_EVENT(EVT_ENABLE_BUTTONS, wxThreadEvent);
wxDEFINE_EVENT(EVT_ENABLE_OPEN_FOLDER, wxThreadEvent);

//void ThreadFunction(std::tuple<FontCollector*, SubsFile*, int*> *data)
//{
//	FontCollector *fc = std::get<0>(*data);
//	SubsFile *file = std::get<1>(*data);
//	int *num = std::get<2>(*data);
//	//fc->SendMessageD(wxString::Format(_("Wystartował wątek %i.\n\n"), *num), wxColour("#FFFFFF"));
//	fc->GetAssFonts(file, *num);
//	delete data;
//	delete num;
//}

SubsFont::SubsFont(const wxString &_name, const LOGFONTW &_logFont, int _bold, bool _italic){
	name = _name;
	logFont = _logFont;
	fakeBold = false;
	fakeItalic = false;
	fakeNormal = false;
	fakeBoldItalic = false;
	italic = _italic ? -1 : 0;
	bold = _bold == 1 ? 700 : _bold == 0 ? 400 : _bold;
}

LOGFONTW &SubsFont::GetLogFont(HDC dc)
{
	if (logFont.lfItalic != italic || logFont.lfWeight != bold){
		std::vector<LOGFONTW> logFonts;
		EnumFontFamiliesEx(dc, &logFont, (FONTENUMPROCW)[](const LOGFONT *lf, const TEXTMETRIC *mt, DWORD style, LPARAM lParam) -> int {
			std::vector<LOGFONTW>* fonts = reinterpret_cast<std::vector<LOGFONTW>*>(lParam);
			fonts->push_back(*lf);
			return 1;
		}, (LPARAM)&logFonts, 0);
		size_t lfssize = logFonts.size();
		bool BoldItalic = false;
		bool Bold = false;
		bool Italic = false;
		bool Normal = false;
		for (size_t i = 0; i < lfssize; i++){
			if ((logFonts[i].lfWeight >= 700) && (0 != logFonts[i].lfItalic)){
				BoldItalic = true;
			}
			else if (logFonts[i].lfWeight >= 700){
				Bold = true;
			}
			else if (0 != logFonts[i].lfItalic){
				Italic = true;
			}
			else{
				Normal = true;
			}
		}
		fakeBoldItalic = (bold >= 700 && italic != 0) ? !BoldItalic : false;
		fakeBold = (bold >= 700 && italic != 0) ? !BoldItalic : (bold >= 700) ? !Bold : false;
		fakeItalic = (italic != 0) ? !Italic : false;
		fakeNormal = (italic == 0 && bold < 700) ? !Normal : false;
		logFont.lfItalic = italic;
		logFont.lfWeight = bold;
	}

	return logFont;
}

void FontLogContent::DoLog(FontCollector *fc){
	fc->SendMessageD(info, (notFound) ? fc->fcd->warning : fc->fcd->normal);
	
	wxString messageText;
	if (styles.size())
		messageText << _("W stylach:\n");
	stylesArea.x = fc->currentTextPosition + messageText.length();
	for (auto &cur = styles.begin(); cur != styles.end(); cur++){
		messageText << L" - " << cur->first;
		//if (cur->second.GetCount() > 1){
			messageText << _(" zakładki: ");
			for (auto & tab : cur->second)
				messageText << (tab + 1) << L", ";
			messageText.RemoveLast(2);
		//}
		messageText << L"\n";
	}
	stylesArea.y = fc->currentTextPosition + messageText.length();
	if (lines.size())
		messageText << _("W linijkach: ");
	linesArea.x = fc->currentTextPosition + messageText.length();
	for (auto &cur = lines.begin(); cur != lines.end(); cur++){
		messageText << (cur->first + 1) << L", ";
	}
	if (lines.size()){
		messageText.RemoveLast(2);
		messageText << L"\n";
	}
	linesArea.y = fc->currentTextPosition + messageText.length();
	if (warnings.empty())
		messageText << L"\n";
	else
		warnings << L"\n";

	fc->SendMessageD(messageText, (notFound) ? fc->fcd->warning : fc->fcd->normal);
	fc->SendMessageD(warnings, fc->fcd->warning);

}

FontCollectorDialog::FontCollectorDialog(wxWindow *parent, FontCollector *_fc)
	: KaiDialog(parent, -1, _("Kolekcjoner czcionek"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
	, fc(_fc)
{
	warning = Options.GetColour(WINDOW_WARNING_ELEMENTS);
	normal = Options.GetColour(WINDOW_TEXT);
	DialogSizer *Main = new DialogSizer(wxVERTICAL);
	wxBoxSizer *Pathc = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *Buttons = new wxBoxSizer(wxHORIZONTAL);
	wxIcon icn;
	icn.CopyFromBitmap(CreateBitmapFromPngResource(L"fontcollector"));
	SetIcon(icn);

	KaiTextValidator valid(wxFILTER_EXCLUDE_CHAR_LIST);
	wxArrayString excludes;
	excludes.Add(L"/");
	excludes.Add(L"*");
	excludes.Add(L"?");
	excludes.Add(L"\"");
	excludes.Add(L"<");
	excludes.Add(L">");
	excludes.Add(L"|");
	valid.SetExcludes(excludes);
	path = new KaiTextCtrl(this, -1, Options.GetString(FONT_COLLECTOR_DIRECTORY), wxDefaultPosition, wxSize(150, -1), 0, valid);
	path->Enable(Options.GetInt(FONT_COLLECTOR_ACTION) != 0);
	choosepath = new MappedButton(this, 8799, _("Wybierz folder"));
	choosepath->Enable(Options.GetInt(FONT_COLLECTOR_ACTION) != 0);
	Connect(8799, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&FontCollectorDialog::OnButtonPath);

	Pathc->Add(path, 1, wxEXPAND | wxALL, 3);
	Pathc->Add(choosepath, 0, wxBOTTOM | wxTOP | wxRIGHT, 3);

	wxArrayString choices;
	choices.Add(_("Sprawdź dostępność czcionek"));
	choices.Add(_("Skopiuj do wybranego folderu"));
	choices.Add(_("Spakuj zipem"));
	//choices.Add(_("Wmuxuj napisy w wideo (wymagany MKVToolnix)"));
	opts = new KaiRadioBox(this, 9987, _("Opcje"), wxDefaultPosition, wxDefaultSize, choices, 0, wxRA_SPECIFY_ROWS);
	opts->SetSelection(Options.GetInt(FONT_COLLECTOR_ACTION));
	Connect(9987, wxEVT_COMMAND_RADIOBOX_SELECTED, (wxObjectEventFunction)&FontCollectorDialog::OnChangeOpt);

	subsdir = new KaiCheckBox(this, 7998, _("Zapisuj do folderu z napisami / wideo."));
	subsdir->SetToolTip(_("Zapisuje do folderu z wideo\nprzy wyciąganiu czcionek z pliku MKV."));
	subsdir->Enable(Options.GetInt(FONT_COLLECTOR_ACTION) != 0);
	subsdir->SetValue(Options.GetBool(FONT_COLLECTOR_USE_SUBS_DIRECTORY));


	fromMKV = new KaiCheckBox(this, 7991, _("Wyciągnij czcionki z wczytanego pliku MKV"));
	fromMKV->Enable(Notebook::GetTab()->VideoPath.Lower().EndsWith(L".mkv"));
	fromMKV->SetValue(Options.GetBool(FONT_COLLECTOR_FROM_MKV));

	Connect(7998, wxEVT_COMMAND_CHECKBOX_CLICKED, (wxObjectEventFunction)&FontCollectorDialog::OnChangeOpt);
	console = new KaiTextCtrl(this, -1, L"", wxDefaultPosition, wxSize(500, 400), wxTE_MULTILINE | wxTE_READONLY);
	console->Bind(wxEVT_LEFT_DCLICK, &FontCollectorDialog::OnConsoleDoubleClick, this);
	//console->SetBackgroundColour(Options.GetColour(WINDOW_BACKGROUND));
	bok = new MappedButton(this, 9879, _("Rozpocznij"));
	bok->SetFocus();
	bStartOnAllTabs = new MappedButton(this, 9880, _("Rozpocznij na zakładkach"));
	bOpenFontFolder = new MappedButton(this, 9877, _("Folder zapisu"));
	bOpenFontFolder->Enable(false);
	bClose = new MappedButton(this, 9881, _("Zamknij"));
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){
		fc->fcd = NULL;
		Destroy();
	}, 9881);
	Connect(9879, 9880, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&FontCollectorDialog::OnButtonStart);
	Buttons->Add(bok, 0, wxALL, 5);
	Buttons->Add(bStartOnAllTabs, 0, wxTOP | wxBOTTOM, 5);
	Buttons->Add(bOpenFontFolder, 0, wxTOP | wxBOTTOM | wxLEFT, 5);
	Buttons->Add(bClose, 0, wxBOTTOM | wxTOP | wxLEFT, 5);

	Main->Add(Pathc, 0, wxEXPAND | wxALL, 1);
	Main->Add(opts, 0, wxEXPAND);
	Main->Add(subsdir, 0, wxALL | wxEXPAND, 4);
	Main->Add(fromMKV, 0, wxALL | wxEXPAND, 4);
	Main->Add(console, 1, wxLEFT | wxRIGHT | wxEXPAND, 4);
	Main->Add(Buttons, 0, wxALIGN_CENTER);

	Bind(EVT_APPEND_MESSAGE, [=](wxThreadEvent evt){
		std::pair<wxString, wxColour> *data = evt.GetPayload<std::pair<wxString, wxColour>*>();
		//console->SetDefaultStyle(wxTextAttr(data->second));
		console->AppendTextWithStyle(data->first, data->second);
		delete data;
	});
	Bind(EVT_ENABLE_BUTTONS, [=](wxThreadEvent evt){
		EnableControls();
	});
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent evt){
		
		/*CoInitialize(0);
		ITEMIDLIST *pidl = ILCreateFromPathW(copypath.wc_str());
		if (pidl) {
			SHOpenFolderAndSelectItems(pidl, 0, 0, 0);
			ILFree(pidl);
		}
		CoUninitialize();*/
		SelectInFolder(copypath);
	}, 9877);
	Bind(EVT_ENABLE_OPEN_FOLDER, [=](wxThreadEvent evt){
		bOpenFontFolder->Enable();
	});
	SetSizerAndFit(Main);
	CenterOnParent();
	SetEscapeId(9881);
	SetEnterId(9879);
}

FontCollectorDialog::~FontCollectorDialog()
{
	fc->ClearTables();
};

void FontCollectorDialog::EnableControls(bool enable)
{
	if (enable){
		if (disabler){
			delete disabler;
			disabler = NULL;
		}
	}
	else{
		disabler = new wxWindowDisabler(this);
	}
	opts->Enable(enable);
	bool enablePathChoose = enable && (opts->GetSelection() != 0);
	path->Enable(enablePathChoose);
	choosepath->Enable(enablePathChoose);
	subsdir->Enable(enablePathChoose);
	bok->Enable(enable);
	bStartOnAllTabs->Enable(enable);
	bClose->Enable(enable);
}

void FontCollectorDialog::OnConsoleDoubleClick(wxMouseEvent &evt)
{
	auto flc = fc->findFontsLog;
	auto nflc = fc->notFindFontsLog;
	bool isStyle = false;
	wxPoint ht;
	console->HitTest(evt.GetPosition(), &ht);
	for (auto cur = flc.begin(); cur != flc.end(); cur++){
		if (!cur->second)
			continue;

		if (cur->second->CheckPosition(ht.x, &isStyle)){
			ParseDoubleClickResults(cur->second, ht.x, isStyle);
			return;
		}
	}
	for (auto cur = nflc.begin(); cur != nflc.end(); cur++){
		if (!cur->second)
			continue;

		if (cur->second->CheckPosition(ht.x, &isStyle)){
			ParseDoubleClickResults(cur->second, ht.x, isStyle);
			return;
		}
	}
	
	evt.Skip();
}

void FontCollectorDialog::ParseDoubleClickResults(FontLogContent *flc, int cursorPos, bool isStyle)
{
	wxString text = console->GetValue();
	wxString delim = L" \n,";
	int start = 0, end = 0;
	for (size_t i = cursorPos; i > 0; i--){
		if (delim.find(text[i]) != -1){
			start = i + 1;
			break;
		}
	}
	for (size_t i = cursorPos; i < text.length(); i++){
		if (delim.find(text[i]) != -1){
			end = i - 1;
			if (end < start)
				end = start - 1;
			break;
		}
	}

	wxString word = text.Mid(start, end - start + 1);
	//need to write own find word between space or \n
	
	if (word.IsNumber()){
		if (isStyle){
			//find style name
			//it's on start of line
			wxString line = text.Mid(0, end).AfterLast(L'\n');
			wxString styleName;
			if (line.StartsWith(L" - ", &styleName)){
				int result = styleName.find(_(" zakładki: "));
				if (result == -1)
					return;

				styleName = styleName.Mid(0, result);

				auto cur = flc->styles.find(styleName);
				if (cur != flc->styles.end() && cur->second.size()){
					//wonder if it's possible that this table was empty
					//better to check
					int numtab = atoi(word) - 1;
					//we have style and tab number, now only open it
					OpenStyle(numtab, styleName);
				}
			}
			// else wtf?
			else{
				bool thisIsBad = true;
			}
		}
		else{
			//line number
			//line in text field is increased
			int line = atoi(word) - 1;
			auto cur = flc->lines.find(line);
			if (cur != flc->lines.end() && cur->second.size()){
				int tab = cur->second[0];
				SetLine(tab, line);
			}
		}
	}
	else if(!word.empty()){
		//style name or shit like tabs: or - maybe even ,
		//auto cur = flc->styles.find(word);
		//if (cur != flc->styles.end() && cur->second.size()){
		//	//wonder if it's possible that this table was empty
		//	//better to check
		//	int tab = cur->second[0];
		//	//we have style and tab number, now only open it
		//	OpenStyle(tab, word);
		//}
		//else{ 
			wxString lineFromStart = text.Mid(0, end).AfterLast(L'\n');
			wxString lineFromEnd = text.Mid(end).BeforeFirst(L'\n');
			wxString line = lineFromStart + lineFromEnd;
			wxString styleName;
			if (line.StartsWith(L" - ", &styleName)){
				size_t result = styleName.find(_(" zakładki: "));
				if (result != -1){
					styleName = styleName.Mid(0, result);
				}

				auto cur = flc->styles.find(styleName);
				if (cur != flc->styles.end() && cur->second.size()){
					int tab = cur->second[0];
					//we have style and tab number, now only open it
					OpenStyle(tab, styleName);
				}
			}
		//}
	}
}

void FontCollectorDialog::OpenStyle(int numtab, const wxString &style)
{
	Notebook * tabs = Notebook::GetTabs();
	tabs->ChangePage(numtab, true);
	TabPanel *tab = tabs->GetTab();
	SubsGrid *grid = tab->Grid;
	bool lineSet = false;
	for (size_t i = 0; i < grid->GetCount(); i++){
		Dialogue *dial = grid->GetDialogue(i);
		if (dial->Style == style){
			grid->ChangeActiveLine(i, true, true);
			lineSet = true;
			break;
		}
	}

	StyleStore::ShowStyleEdit(style);
}

void FontCollectorDialog::SetLine(int numtab, int line)
{
	Notebook * tabs = Notebook::GetTabs();
	tabs->ChangePage(numtab, true);
	TabPanel *tab = tabs->GetTab();
	tab->Grid->ChangeActiveLine(line, true, true);
}

void FontCollectorDialog::OnButtonPath(wxCommandEvent &event)
{

	if (opts->GetSelection() == 1){
		destdir = wxDirSelector(_("Wybierz folder zapisu"), path->GetValue(), 0, wxDefaultPosition, this);
	}
	else{
		destdir = wxFileSelector(_("Wybierz nazwę archiwum"), (path->GetValue().EndsWith(L"zip")) ? 
			path->GetValue().BeforeLast(L'\\') : path->GetValue(),
			(path->GetValue().EndsWith(L"zip")) ? path->GetValue().AfterLast(L'\\') : L"",
			L"zip", _("Pliki archiwum (*.zip)|*.zip"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT, this);
	}
	Options.SetString(FONT_COLLECTOR_DIRECTORY, destdir);
	Options.SaveOptions(true, false);
	path->SetValue(destdir);
}

void FontCollectorDialog::OnButtonStart(wxCommandEvent &event)
{
	console->SetValue(L"");
	EnableControls(false);
	
	int operation = (fromMKV->GetValue() && fromMKV->IsEnabled()) ? FontCollector::COPY_MKV_FONTS :
		/*(opts->GetSelection() == 3) ? FontCollector::MUX_VIDEO_WITH_SUBS : */
		(opts->GetSelection() == 0) ? FontCollector::CHECK_FONTS: FontCollector::COPY_FONTS;
	if (event.GetId() == 9880){
		operation |= FontCollector::ON_ALL_TABS;
	}
	if (opts->GetSelection() == 0)
		fc->StartCollect(operation);
	else
	{
		if (bOpenFontFolder->IsEnabled())
			bOpenFontFolder->Enable(false);

		bool subsfromMkv = fromMKV->GetValue();
		bool subsDirectory = subsdir->GetValue();
		Options.SetString(FONT_COLLECTOR_DIRECTORY, path->GetValue());
		if (opts->GetSelection() == 3 && (Notebook::GetTab()->VideoPath == L"" || Notebook::GetTab()->SubsPath == L"")){
			KaiMessageBox(_("Brak wczytanego wideo lub napisów"), L"", 4L, this);
			EnableControls();
			return;
		}
		if (path->GetValue() == L"" && !subsDirectory){
			KaiMessageBox(_("Wybierz folder, gdzie mają zostać skopiowane czcionki"), L"", 4L, this);
			EnableControls();
			path->SetFocus();
			return;
		}
		if (!subsfromMkv && subsDirectory && Notebook::GetTab()->SubsPath == L""){
			KaiMessageBox(_("Brak wczytanych napisów, wczytaj napisy albo odznacz tę opcję."), L"", 4L, this);
			EnableControls();
			return;
		}
		if (opts->GetSelection() == 2 && path->GetValue().EndsWith(L"\\") && !subsdir->GetValue()){
			KaiMessageBox(_("Wybierz nazwę dla archiwum"), L"", 4L, this);
			EnableControls();
			path->SetFocus();
			return;
		}
		if (subsDirectory){
			wxString rest;
			copypath = (subsfromMkv) ? Notebook::GetTab()->VideoPath.BeforeLast(L'\\', &rest) : Notebook::GetTab()->SubsPath.BeforeLast(L'\\', &rest);
			copypath << L"\\Czcionki\\";
			if (opts->GetSelection() == 2){ copypath << rest.BeforeLast(L'.') << L".zip"; }
		}
		else{
			copypath = path->GetValue();
			wxFileName fname(copypath);
			if (!fname.IsOk() || fname.GetVolume().length() != 1){
				KaiMessageBox(_("Wybrana ścieżka zapisu jest niepoprawna."), L"", 4L, this);
				EnableControls();
				return;
			}
			if (opts->GetSelection() != 2 && !copypath.EndsWith("\\L")){ copypath << L"\\"; }
			else if (opts->GetSelection() == 2 && !copypath.EndsWith(L".zip")){ copypath << L".zip"; }
		}
		if (opts->GetSelection() != 2){
			wxString extt = copypath.Right(4).Lower();
			if (extt == L".zip"){ copypath = copypath.BeforeLast(L'\\') + L"\\"; }
			if (!wxDir::Exists(copypath)){
				if (!wxDir::Make(copypath, 511, wxPATH_MKDIR_FULL)){
					KaiMessageBox(_("Nie można utworzyć folderu."), L"", 4L, this);
					EnableControls();
					return;
				}
			}
		}
		else{
			if (!wxDir::Exists(copypath.BeforeLast(L'\\'))){
				if (!wxDir::Make(copypath.BeforeLast(L'\\'), 511, wxPATH_MKDIR_FULL)){
					KaiMessageBox(_("Nie można utworzyć folderu."), L"", 4L, this);
					EnableControls();
					return;
				}
			}
			else if (wxFileExists(copypath)){
				if (KaiMessageBox(_("Plik zip już istnieje, usunąć go?"), _("Potwierdzenie"), wxYES_NO, this) == wxYES){
					if (!wxRemoveFile(copypath)){
						EnableControls();
						return;
					}
				}
			}
		}


		if (opts->GetSelection() == 2){ operation |= FontCollector::AS_ZIP; }
		
		/*if (opts->GetSelection() == 3){
			fc->muxerpath = L"C:\\Program Files\\MKVtoolnix\\mkvmerge.exe";
			if (!wxFileExists(fc->muxerpath)){
				wxFileDialog *fd = new wxFileDialog(this, _("Wybierz plik mkvmerge.exe"), L"C:\\Program Files", L"mkvmerge.exe", _("Programy (.exe)|*.exe"), wxFD_OPEN | wxFD_FILE_MUST_EXIST);
				if (fd->ShowModal() != wxID_OK){
					EnableControls();
					KaiMessageBox(_("Muxowanie zostało anulowane, bo nie wybrano mkvmerge.exe")); fd->Destroy();
					return;
				}
				fc->muxerpath = fd->GetPath();
				fd->Destroy();
			}
		}*/
		fc->StartCollect(operation);
	}
}

void FontCollectorDialog::OnChangeOpt(wxCommandEvent &event)
{
	path->Enable(opts->GetSelection() != 0);
	choosepath->Enable(opts->GetSelection() != 0);
	subsdir->Enable(opts->GetSelection() != 0);

	fromMKV->Enable(opts->GetSelection() != 0 && Notebook::GetTab()->VideoPath.Lower().EndsWith(L".mkv"));
	Options.SetInt(FONT_COLLECTOR_ACTION, opts->GetSelection());
	Options.SetBool(FONT_COLLECTOR_USE_SUBS_DIRECTORY, subsdir->GetValue());
	Options.SaveOptions(true, false);
}

FontCollector::FontCollector(wxWindow *parent)
	:zip(NULL)
	, fcd(NULL)
	, reloadFonts(false)
{
	FontEnum.AddClient(parent, [=](){reloadFonts = true; });
}

FontCollector::~FontCollector()
{
	//test if it not crash nothing
	if (fcd)
		fcd->Destroy();
};

void FontCollector::GetAssFonts(SubsFile *subs, int tab)
{
	std::map<wxString, Styles*> stylesfonts;

	std::vector<Styles*> * styles = subs->GetStyleTable();

	for (size_t i = 0; i < styles->size(); i++)
	{
		Styles *style = (*styles)[i];
		wxString fn = style->Fontname;
		bool bold = style->Bold;
		bool italic = style->Italic;
		wxString fnl = fn.Lower() << (int)bold << (int)italic;
		int iresult = facenames.Index(fn, false);
		if (iresult == -1){
			FontLogContent *nflc = notFindFontsLog[fn];
			if (!nflc){
				nflc = new FontLogContent(_("Nie znaleziono czcionki \"") + fn + L"\".\n", true);
				notFindFontsLog[fn] = nflc;
			}
			nflc->SetStyle(tab, style->Name);
			//continue;
		}
		else
		{
			//pamiętaj, dodaj jeszcze boldy i itaici
			if (!(foundFonts.find(fnl) != foundFonts.end())){
				foundFonts[fnl] = new SubsFont(fn, logFonts[iresult], (int)bold, italic);
			}
			FontLogContent *flc = findFontsLog[fn];
			if (!flc){
				flc = new FontLogContent(wxString::Format(_("Znaleziono czcionkę \"%s\"\n"), fn));
				findFontsLog[fn] = flc;
			}
			flc->SetStyle(tab, style->Name);
		}
		stylesfonts[style->Name] = style;

	}

	wxString tags[] = { L"fn", L"b", L"i", L"p" };

	for (size_t i = 0; i < subs->GetCount(); i++)
	{
		Dialogue *dial = subs->GetDialogue(i);
		if (dial->IsComment){ continue; }
		dial->ParseTags(tags, 4, true);
		ParseData *pdata = dial->parseData;
		if (!pdata){ continue; }

		const wxString &text = dial->GetTextNoCopy();

		Styles *lstyle = stylesfonts[dial->Style];

		wxString ifont = (lstyle) ? lstyle->Fontname : L"";
		int bold = (lstyle) ? (int)lstyle->Bold : 0;
		int italic = (lstyle) ? (int)lstyle->Italic : 0;
		bool newFont = false;
		bool lastPlain = false;
		wxString textHavingFont;
		size_t tagsSize = pdata->tags.size();

		for (size_t j = 0; j < tagsSize; j++)
		{
			TagData *tag = pdata->tags[j];
			if (tag->tagName == L"p"){ continue; }

			if (tag->tagName == L"plain"){
				textHavingFont += tag->value;
				if ((lastPlain && j < tagsSize - 1) || ifont.IsEmpty()){ continue; }
				lastPlain = true;
				wxString fnl = ifont.Lower() << bold << italic;
				int iresult = facenames.Index(ifont, false);
				if (iresult == -1){
					FontLogContent *nflc = notFindFontsLog[ifont];
					if (!nflc){
						nflc = new FontLogContent(_("Nie znaleziono czcionki \"") + ifont + L"\".\n", true);
						notFindFontsLog[ifont] = nflc;
					}
					if (newFont){
						nflc->SetLine(tab, i);
					}
				}
				else
				{
					if (!(foundFonts.find(fnl) != foundFonts.end())){
						foundFonts[fnl] = new SubsFont(ifont, logFonts[iresult], bold, (italic != 0));
					}
					if (newFont){
						FontLogContent *flc = findFontsLog[ifont];
						if (!flc){
							flc = new FontLogContent(wxString::Format(_("Znaleziono czcionkę \"%s\"\n"), ifont));
							findFontsLog[ifont] = flc;
						}
						flc->SetLine(tab, i);
						newFont = false;
					}
				}
				//we add all texts to check if even not found font is even needed
				//when is not needed leave only info cause not inform on the end.
				textHavingFont.Replace(L"\\N", L"");
				textHavingFont.Replace(L"\\n", L"");
				textHavingFont.Replace(L"\\h", L" ");
				PutChars(textHavingFont, ifont);
				textHavingFont.clear();
			}
			else{
				if (tag->tagName == L"fn"){
					ifont = tag->value;
					newFont = true;
				}
				else if (tag->tagName == L"b"){
					bold = wxAtoi(tag->value);
				}
				else if (tag->tagName == L"i"){
					italic = wxAtoi(tag->value);
				}
				lastPlain = false;
			}


		}
		dial->ClearParse();
	}

}

bool FontCollector::AddFont(const wxString &string)
{
	return true;
}

void FontCollector::CheckOrCopyFonts()
{
	if (!(operation & CHECK_FONTS) && (fontSizes.size() < 1 || reloadFonts)){
		fontfolder = wxGetOSDirectory() + L"\\fonts\\";
		wxString seekpath = fontfolder + L"*";

		fontSizes.clear();
		WIN32_FIND_DATAW data;
		HANDLE h = FindFirstFileW(seekpath.wc_str(), &data);
		if (h == INVALID_HANDLE_VALUE)
		{
			SendMessageD(_("Nie można pobrać rozmiarów i nazw plików czcionek\nkopiowanie zostaje przerwane.\n"), fcd->warning);
			return;
		}
		//first file is "." second ".." after there are a font files
		//fontSizes.insert(std::pair<long, wxString>(data.nFileSizeLow, wxString(data.cFileName)));
		while (1){
			int result = FindNextFile(h, &data);
			if (result == ERROR_NO_MORE_FILES || result == 0){ break; }
			else if (data.nFileSizeLow == 0){ continue; }
			fontSizes.insert(std::pair<long, wxString>(data.nFileSizeLow, wxString(data.cFileName)));
		}
		FindClose(h);

		WCHAR appDataPath[MAX_PATH];
		
		if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA | CSIDL_FLAG_CREATE, NULL, 0, appDataPath))){
			fontFolderLocal = wxString(appDataPath) + L"\\Microsoft\\Windows\\Fonts\\";
			wxString localPath = fontFolderLocal + L"*";
			WIN32_FIND_DATAW data1;
			HANDLE h1 = FindFirstFileW(localPath.wc_str(), &data1);
			if (h1 != INVALID_HANDLE_VALUE){
				//fontSizes.insert(std::pair<long, wxString>(data1.nFileSizeLow, wxString(data1.cFileName)));
				while (1){
					int result = FindNextFile(h1, &data1);
					if (result == ERROR_NO_MORE_FILES || result == 0){ break; }
					else if (data1.nFileSizeLow == 0){ continue; }
					fontSizes.insert(std::pair<long, wxString>(data1.nFileSizeLow, wxString(data1.cFileName)));
				}
				FindClose(h1);
			}
		}
		STime processTime(sw.Time());
		SendMessageD(wxString::Format(_("Pobrano rozmiary i nazwy %i czcionek, upłynęło %sms.\n\n"), (int)fontSizes.size() - 2, processTime.GetFormatted(SRT)), fcd->normal);
	}
	if (operation & AS_ZIP){
		wxFFileOutputStream *out = new wxFFileOutputStream(fcd->copypath);
		zip = new wxZipOutputStream(out, 9);

	}

	int found = 0;
	int notFound = 0;
	int notCopied = 0;

	if (facenames.size() < 1 || reloadFonts){ EnumerateFonts(); }
	

	if (operation & ON_ALL_TABS){
		Notebook * tabs = Notebook::GetTabs();
		size_t tabsSize = tabs->Size();
		//HANDLE *threads = new HANDLE[tabsSize];

		for (size_t i = 0; i < tabsSize; i++){
			GetAssFonts(tabs->Page(i)->Grid->file, i);
			//std::tuple<FontCollector*, SubsFile*, int*> *data = 
				//new std::tuple<FontCollector *, SubsFile *, int*>(this, tabs->Page(i)->Grid->file, new int(i));
			
			//threads[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadFunction, data, 0, 0);
			
		}
		//WaitForMultipleObjects(tabsSize, threads, TRUE, INFINITE);
		//delete[] threads;
	}
	else{
		GetAssFonts(Notebook::GetTab()->Grid->file, Notebook::GetTabs()->iter);
	}
	bool allglyphs = CheckPathAndGlyphs(&found, &notFound, &notCopied);

	//checking glyphs not work on not existed fonts, 
	notFound += notFindFontsLog.size();
	for (auto cur = findFontsLog.begin(); cur != findFontsLog.end(); cur++){
		cur->second->DoLog(this);
	}
	for (auto cur = notFindFontsLog.begin(); cur != notFindFontsLog.end(); cur++){
		CharMap &ch = FontMap[cur->first];
		if (!ch.size()){
			cur->second->AppendWarnings(
				wxString::Format(_("Czcionka \"%s\" należy do stylu,\nktóry nie jest wykorzystywany."), 
				cur->first));
			notFound--;
		}
		cur->second->DoLog(this);
	}
	
	if (zip){
		zip->Close();
		delete zip;
		zip = NULL;
	}
	wxString noglyphs = (allglyphs) ? L"" : _("Niektóre czcionki nie mają wszystkich znaków użytych w tekście.\n");

	bool checkFonts = (operation & CHECK_FONTS);

	if (notFound || !allglyphs){
		wxString message;
		
		message += L"\n" + wxString::Format(_("Zakończono, %s %s.\n"), (checkFonts) ? _("znaleziono") : _("skopiowano"),
			MakePolishPlural(found, _("czcionkę"), _("czcionki"), _("czcionek")));
		if (notFound){
			message += wxString::Format(_("Nie znaleziono %s.\n"),
				MakePolishPlural(notFound, _("czcionki"), _("czcionek"), _("czcionek")));
		}
		if (notCopied){
			message += wxString::Format(_("Nie udało się skopiować %s.\n"),
				MakePolishPlural(notCopied, _("czcionki"), _("czcionek"), _("czcionek")));
		}
		
		message += noglyphs;
		SendMessageD(message, fcd->warning);
	}
	else{
		SendMessageD(L"\n" + wxString::Format(_("Zakończono powodzeniem, %s %s.\n"),
			(checkFonts) ? _("znaleziono") : _("skopiowano"),
			MakePolishPlural(found, _("czcionkę"), _("czcionki"), _("czcionek"))), wxColour("#008000"));
	}
	fontnames.clear();
}

bool FontCollector::SaveFont(const wxString &fontPath, FontLogContent *flc)
{
	wxString fn = fontPath.AfterLast(L'\\');
	if (zip){
		wxFFileInputStream in(fontPath);
		bool isgood = in.IsOk();
		if (isgood){
			try {
				zip->PutNextEntry(fn);
				zip->Write(in);
				flc->AppendInfo(wxString::Format(_("Dodano do archiwum czcionkę \"%s\"."), fn));
			}
			catch (...) {
				isgood = false;
			}

		}

		if (!isgood){
			flc->AppendWarnings(wxString::Format(_("Nie można spakować czcionki \"%s\"."), fn));
		}
		return isgood;
	}
	else{
		if (wxCopyFile(fontPath, fcd->copypath + L"\\" + fn)){
			flc->AppendInfo(wxString::Format(_("Skopiowano czcionkę \"%s\"."), fn));
			return true;
		}
		else
		{
			flc->AppendWarnings(wxString::Format(_("Nie można skopiować czcionki \"%s\"."), fn));
			return false;
		}
	}
}

void FontCollector::CopyMKVFonts()
{
	zip = NULL;
	if (operation & AS_ZIP){
		wxFFileOutputStream *out = new wxFFileOutputStream(fcd->copypath);
		zip = new wxZipOutputStream(out, 9);
	}

	if (operation & ON_ALL_TABS){
		Notebook *tabs = Notebook::GetTabs();
		for (size_t i = 0; i < tabs->Size(); i++){
			wxString mkvpath = tabs->Page(i)->VideoPath;
			SendMessageD(wxString::Format(_("Wideo: %s\n\n"), mkvpath), fcd->normal);
			CopyMKVFontsFromTab(mkvpath);
		}
	}
	else{
		wxString mkvpath = Notebook::GetTab()->VideoPath;
		CopyMKVFontsFromTab(mkvpath);
	}
	
	if (zip)
	{
		zip->Close();
		delete zip;
		zip = NULL;
	}
}

void FontCollector::CopyMKVFontsFromTab(const wxString &mkvpath)
{
	wxString ext = mkvpath.AfterLast(L'.').Lower();
	
	if (ext != L"mkv"){
		SendMessageD(_("To wideo nie jest plikiem MKV."), fcd->warning);
		return;
	}
	MatroskaWrapper mw;
	mw.Open(mkvpath);
	std::map<int, wxString> names = mw.GetFontList();
	if (names.size() < 1){
		SendMessageD(_("Wczytany plik MKV nie ma żadnych czcionek."), fcd->warning);
		return;
	}

	size_t cpfonts = names.size();

	for (auto fontI : names){
		if (mw.SaveFont(fontI.first, fcd->copypath.BeforeLast(L'\\') + L"\\" + fontI.second, zip))
		{
			SendMessageD(_("Zapisano czcionkę o nazwie \"") + fontI.second + L"\".\n \n", fcd->normal);
		}
		else
		{
			SendMessageD(_("Nie można zapisać czcionki o nazwie \"") + fontI.second + L"\".\n \n", fcd->warning);
			cpfonts--;
		}
	}

	if (cpfonts < names.size()){
		SendMessageD(wxString::Format(_("Zakończono, skopiowano %i czcionek.\nNie udało się skopiować %i czcionek."), 
			(int)cpfonts, (int)(names.size() - cpfonts)), fcd->warning);
	}
	else{
		SendMessageD(wxString::Format(_("Zakończono powodzeniem, skopiowano %i czcionek."), (int)cpfonts), wxColour(L"#008000"));
	}

	mw.Close();
}


void FontCollector::ClearTables()
{
	currentTextPosition = 0;
	FontMap.clear();
	for (auto cur = notFindFontsLog.begin(); cur != notFindFontsLog.end(); cur++)
		delete cur->second;
	notFindFontsLog.clear();
	for (auto cur = findFontsLog.begin(); cur != findFontsLog.end(); cur++)
		delete cur->second;
	findFontsLog.clear();
	for (auto cur = foundFonts.begin(); cur != foundFonts.end(); cur++)
		delete cur->second;
	foundFonts.clear();
}

void FontCollector::PutChars(const wxString &txt, const wxString &fn)
{
	CharMap &ch = FontMap[fn];

	for (size_t i = 0; i < txt.length(); i++)
	{
		if (!(ch.find(txt[i]) != ch.end())){
			ch.insert(txt[i]);
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
	::DeleteDC(dc);
}

bool FontCollector::CheckPathAndGlyphs(int *found, int *notFound, int *notCopied)
{
	bool allfound = true;
	bool copyFonts = !(operation & CHECK_FONTS);
	HDC dc = ::CreateCompatibleDC(NULL);
	auto it = foundFonts.begin();
	wxString lastfn;
	for (size_t k = 0; k < foundFonts.size(); k++){
		// reference can a bit make it faster, when create indirect font
		// not release it
		LOGFONTW mlf = it->second->GetLogFont(dc);
		wxString fn = it->second->name;
		bool isNewFont = lastfn != fn;
		lastfn = fn;
		SubsFont *font = it->second;
		FontLogContent *flc = findFontsLog[fn];
		if (!flc){
			flc = new FontLogContent(wxString::Format(_("Znaleziono czcionkę \"%s\"."), fn));
			findFontsLog[fn] = flc;
			// chyba potencjalnie niemożliwe, ale różne błędy się zdarzają
		}
		it++;
		//skip not used font before it make any other messages
		CharMap &ch = FontMap[fn];
		if (!ch.size()){
			if(isNewFont)
				flc->AppendWarnings(wxString::Format(_("Czcionka \"%s\" należy do stylu,\nktóry nie jest wykorzystywany.%s"), fn, (copyFonts) ? _("\nNie zostanie skopiowana.") : L""));
			
			continue;
			//no goto cause font is not created yet
		}
		auto hfont = CreateFontIndirectW(&mlf);
		SelectObject(dc, hfont);
		if (font->fakeNormal){
			flc->AppendWarnings(wxString::Format(_("Czcionka \"%s\" nie ma stylu normalnego."), fn));
		}
		else if (font->fakeBoldItalic){
			flc->AppendWarnings(wxString::Format(_("Czcionka \"%s\" nie ma pogrubienia z kursywą."), fn));
		}
		else if (font->fakeBold){
			flc->AppendWarnings(wxString::Format(_("Czcionka \"%s\" nie ma pogrubienia."), fn));
		}
		else if (font->fakeItalic){
			flc->AppendWarnings(wxString::Format(_("Czcionka \"%s\" nie ma kursywy."), fn));
		}
		if (isNewFont){
			wxString text;
			wxString missing;
			
			for (auto character = ch.begin(); character != ch.end(); character++)
			{
				const auto &achar = (*character);
				if (achar != 65279)
					text << achar;
			}
			if (!FontEnum.CheckGlyphsExists(dc, text, missing))
			{
				flc->AppendWarnings(wxString::Format(_("Nie można sprawdzić znaków czcionki \"%s\"."), fn));
			}
			if (missing.length() > 0){
				allfound = false;
				flc->AppendWarnings(wxString::Format(_("Czcionka \"%s\" nie zawiera znaków: \"%s\"."), fn, missing));
			}
		}
		
		if (copyFonts){
			DWORD ttcf = 0x66637474;
			auto size = GetFontData(dc, ttcf, 0, nullptr, 0);
			if (size == GDI_ERROR) {
				ttcf = 0;
				size = GetFontData(dc, 0, 0, nullptr, 0);
			}
			if (size == GDI_ERROR || size == 0){
				flc->AppendWarnings(wxString::Format(_("Nie można pobrać zawartości czcionki \"%s\"."), fn));
				if (isNewFont)
					(*notFound)++;

				goto done;
			}
			std::string buffer;
			buffer.resize(size);
			GetFontData(dc, ttcf, 0, &buffer[0], (int)size);
			std::string file_buffer;
			file_buffer.resize(size);
			bool succeeded = false;

			for (auto fontSize = fontSizes.equal_range(size).first; fontSize != fontSizes.equal_range(size).second; ++fontSize){
				wxString fullpath = fontfolder + fontSize->second;
				FILE *fp = _wfopen(fullpath.wc_str(), L"rb");
				if (!fp){
					fullpath = fontFolderLocal + fontSize->second;
					fp = _wfopen(fullpath.wc_str(), L"rb");
				}
				if (!fp){ 
					flc->AppendWarnings(wxString::Format(_("Nie można otworzyć pliku \"%s\"."), fontSize->second));
					//goto done; 
					continue;
				}
				fseek(fp, 0, SEEK_END);
				long lSize = ftell(fp);
				rewind(fp);
				if (lSize != size){
					flc->AppendWarnings(wxString::Format(_("Rozmiar czcionki \"%s\" się różni."), fn));
					fclose(fp);
					//goto done;
					continue;
				}
				int result = fread(&file_buffer[0], 1, size, fp);
				if (result != size){
					flc->AppendWarnings(wxString::Format(_("Nie można odczytać czcionki \"%s\" z folderu Fonts."), fn));
				}
				fclose(fp);
				if (memcmp(&file_buffer[0], &buffer[0], size) == 0) {
					if (fontnames.Index(fullpath, true) == -1){
						fontnames.Add(fullpath);
						flc->AppendInfo(wxString::Format(_("Znaleziono plik czcionki \"%s\"."), fullpath));
						if (operation & COPY_FONTS){ 
							if (!SaveFont(fullpath, flc))
								(*notCopied)++;
							else
								(*found)++;
						}
						wxString ext = fontSize->second.AfterLast(L'.').Lower();
						if (ext == L"pfm" || ext == L"pfb"){
							wxString repl = (ext == L"pfm") ? L"pfb" : L"pfm";
							if (fullpath[fullpath.length() - 1] < L'Z'){ repl = repl.Upper(); }
							wxString secondPath = fullpath.RemoveLast(3) + repl;
							fontnames.Add(secondPath);
							flc->AppendInfo(wxString::Format(_("Znaleziono plik czcionki \"%s\"."), secondPath));
							if (operation & COPY_FONTS){
								if (!SaveFont(fullpath, flc))
									(*notCopied)++;
								else
									(*found)++;
							}
						}
					}
					//fake italic/bold font when another normal is added
					//must succeed but not add a path

					succeeded = true;
					break;
				}
			}
			if (!succeeded){
				flc->AppendWarnings(wxString::Format(_("Nie można znaleźć czcionki \"%s\" w folderze Fonts."), fn));
				(*notFound)++;
				(*found)--;
			}
				
		} else if (isNewFont)
			(*found)++;
		//rest fonts it's just bold/italic version not count it
	done:

		SelectObject(dc, NULL);
		DeleteObject(hfont);
	}
	::DeleteDC(dc);
	return allfound;
}

void FontCollector::MuxVideoWithSubs()
{


	wxString command = L"\"--ui-language\" \"pl\" \"--output\" \"" + Notebook::GetTab()->VideoPath.BeforeLast(L'.') + L" (1).mkv\" " +
		L"\"--language\" \"0:und\" \"--language\" \"1:und\" ( \""\
		+ Notebook::GetTab()->VideoPath +
		L"\" ) \"--language\" \"0:und\" ( \"" + Notebook::GetTab()->SubsPath +
		L"\" ) ";// odtąd trzeba dodać czcionki
	for (size_t i = 0; i < fontnames.size(); i++){
		wxString ext = fontnames[i].AfterLast(L'.').Lower();
		
		command << L"\"--attachment-name\" \"";
		wxString name = fontnames[i].AfterLast(L'\\');
		command << name << L"\" ";
		command << L"\"--attachment-mime-type\" ";
		//if(ext=="ttf" || ext=="ttc"){
		command << L"\"application/x-truetype-font\" ";//}
		//else if(ext=="otf"){command << L"\"application/font-woff\" ";}
		//else otf itp
		command << L"\"--attach-file\" \"";
		command << fcd->copypath << L"\\" << name << L"\" ";
	}
	command.Replace("\\", "/");
	command << L"\"--track-order\" \"0:0,0:1,1:0\"";
	wxString fullcommand = L"\"" + muxerpath + L"\"" + command;

	STARTUPINFO si = { sizeof(si) };
	PROCESS_INFORMATION pi;
	if (!CreateProcessW(NULL, (LPWSTR)fullcommand.wc_str(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)){
		KaiLog(_("Nie można stworzyć procesu, muxowanie przerwane"));
	}


}

void FontCollector::ShowDialog(wxWindow *parent)
{
	if (fcd)
		return;

	fcd = new FontCollectorDialog(parent, this);
	fcd->Show();
}

void FontCollector::StartCollect(int _operation)
{
	operation = _operation;
	FontCollectorThread *ft = new FontCollectorThread(this);
}

void FontCollector::SendMessageD(const wxString &string, const wxColour &col)
{
	wxThreadEvent *evt = new wxThreadEvent(EVT_APPEND_MESSAGE, fcd->GetId());
	evt->SetPayload(new std::pair<wxString, wxColour>(string, col));
	currentTextPosition += string.length();
	wxQueueEvent(fcd, evt);
}

FontCollectorThread::FontCollectorThread(FontCollector *_fc)
	:wxThread(wxTHREAD_DETACHED)
{
	fc = _fc;
	Create();
	Run();
}

wxThread::ExitCode FontCollectorThread::Entry()
{
	fc->sw.Start();
	fc->ClearTables();
	if (fc->operation &FontCollector::CHECK_FONTS || fc->operation &FontCollector::COPY_FONTS){
		fc->CheckOrCopyFonts();
	}
	else if (fc->operation &FontCollector::COPY_MKV_FONTS){
		fc->CopyMKVFonts();
	}
	else if (fc->operation &FontCollector::MUX_VIDEO_WITH_SUBS){
		fc->CheckOrCopyFonts();
		fc->MuxVideoWithSubs();
	}
	else{
		fc->sw.Pause(); return 0;
	}

	wxThreadEvent *evt = new wxThreadEvent(EVT_ENABLE_BUTTONS, fc->fcd->GetId());
	STime processTime(fc->sw.Time());
	fc->SendMessageD(wxString::Format(_("\nZakończono w %sms"), processTime.GetFormatted(SRT)), fc->fcd->normal);
	fc->sw.Pause();
	wxQueueEvent(fc->fcd, evt);
	if (fc->operation & FontCollector::COPY_FONTS || fc->operation & FontCollector::COPY_MKV_FONTS){
		wxThreadEvent *evtof = new wxThreadEvent(EVT_ENABLE_OPEN_FOLDER, fc->fcd->GetId());
		wxQueueEvent(fc->fcd, evtof);
	}
	return 0;
}

//TabFontSeekThread::TabFontSeekThread(FontCollector *_fc, SubsFile *_file, int num)
//	:wxThread(wxTHREAD_JOINABLE)
//{
//	fc = _fc;
//	file = _file;
//	numTab = num;
//	Create();
//	Run();
//}
//
//wxThread::ExitCode TabFontSeekThread::Entry()
//{
//	fc->GetAssFonts(file, numTab);
//}
