//  Copyright (c) 2018, Marcin Drob

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

#include "AutomationHotkeysDialog.h"
#include "MappedButton.h"
#include "KaiMessageBox.h"
#include "KainoteMain.h"
#include <map>

class AutomationHotkeyItem : public Item{
public:
	AutomationHotkeyItem(const wxString &_accelerator, const wxString &_name, int _id) 
		:Item(ITEM_NORMAL){ accelerator = _accelerator; id = _id; name = _name; };
	void Save(){
		if (modified){
			modified = false;
		}
	}
	Item* Copy(){ return new AutomationHotkeyItem(*this); }
	void OnChangeHistory(){
		AutomationHotkeysDialog::allHotkeys[idAndType(id, GLOBAL_HOTKEY)] = hdata(name, accelerator);
		//modified = true;
	}
	void OnMouseEvent(wxMouseEvent &event, bool enter, bool leave, KaiListCtrl *theList, Item **changed /*= NULL*/)
	{
		if (enter){
			if (needTooltip)
				theList->SetToolTip(accelerator);
			else if (theList->HasToolTips())
				theList->UnsetToolTip();
		}
	}
	void OnPaint(wxMemoryDC *dc, int x, int y, int width, int height, KaiListCtrl *theList)
	{
		wxSize ex = dc->GetTextExtent(accelerator);

		if (modified){ dc->SetTextForeground(Options.GetColour(WindowWarningElements)); }
		needTooltip = ex.x > width - 8;
		wxRect cur(x, y, width - 8, height);
		dc->SetClippingRegion(cur);
		dc->DrawLabel(accelerator, cur, wxALIGN_CENTER_VERTICAL);
		dc->DestroyClippingRegion();
		if (modified){ dc->SetTextForeground(Options.GetColour(theList->IsThisEnabled() ? WindowText : WindowTextInactive)); }
	}
	wxString accelerator;
	int id;
};

std::map<idAndType, hdata> AutomationHotkeysDialog::allHotkeys;

AutomationHotkeysDialog::AutomationHotkeysDialog(wxWindow *parent, Auto::Automation *Auto)
	: KaiDialog(parent, -1, _("Lista skrótów klawiszowych skryptów automatyzacji"))
	, automation(Auto)
{
	DialogSizer *mainSizer = new DialogSizer(wxVERTICAL);
	hotkeysList = new KaiListCtrl(this, ID_HOTKEYS_LIST, wxDefaultPosition, wxSize(800,300));
	hotkeysList->InsertColumn(1, _("Ścieżka i nazwa skryptu"), TYPE_TEXT, 400);
	hotkeysList->InsertColumn(2, _("Makro"), TYPE_TEXT, 300);
	hotkeysList->InsertColumn(3, _("Skrót"), TYPE_TEXT, 80);

	allHotkeys = std::map<idAndType, hdata>(Hkeys.GetHotkeysMap());
	std::map<idAndType, hdata> mappedhkeys;
	std::map<idAndType, hdata>::iterator end;
	auto cur = end = allHotkeys.find(idAndType(30100));
	while(cur != allHotkeys.end()){
		if (end->first.id < 30100){
			mappedhkeys.insert(cur, end);
			break;
		}
		end++;
	}
	lastScriptId = 30100 + mappedhkeys.size();

	auto findHotkey = [=](const wxString &hotkeyName, wxString *accel, int *id){
		for (auto it = mappedhkeys.begin(); it != mappedhkeys.end(); it++){
			if (hotkeyName == it->second.Name){
				if (accel)
					*accel = it->second.Accel;
				if (id)
					*id = it->first.id;
				break;
			}
		}
	};

	hotkeysList->AppendItem(new ItemText(_("Folder automatycznego wczytywania")));
	for (int i = 0; i < automation->Scripts.size(); i++){
		Auto::LuaScript * script = automation->Scripts[i];
		auto macros = script->GetMacros();

		for (int k = 0; k < macros.size(); k++){
			auto macro = macros[k];
			long pos = hotkeysList->AppendItem(new ItemText(script->GetFilename()));
			hotkeysList->SetItem(pos, 1, new ItemText(macro->StrDisplay()));
			int id = -1;
			wxString accel;
			wxString scriptHotkeyName;
			scriptHotkeyName << "Script " << script->GetFilename() << "-" << k;
			findHotkey(scriptHotkeyName, &accel, &id);
			hotkeysList->SetItem(pos, 2, new AutomationHotkeyItem(accel, scriptHotkeyName, id));
		}
	}
	hotkeysList->AppendItem(new ItemText(_("Z napisów")));
	for (int i = 0; i < automation->ASSScripts.size(); i++){
		Auto::LuaScript * script = automation->ASSScripts[i];
		auto macros = script->GetMacros();

		for (int k = 0; k < macros.size(); k++){
			auto macro = macros[k];
			long pos = hotkeysList->AppendItem(new ItemText(script->GetFilename()));
			hotkeysList->SetItem(pos, 1, new ItemText(macro->StrDisplay()));
			int id = -1;
			wxString accel;
			wxString scriptHotkeyName;
			scriptHotkeyName << "Script " << script->GetFilename() << "-" << k;
			findHotkey(scriptHotkeyName, &accel, &id);
			hotkeysList->SetItem(pos, 2, new AutomationHotkeyItem(accel, scriptHotkeyName, id));
		}
	}
	hotkeysList->StartEdition();
	hotkeysList->SetSelection(0);
	Connect(ID_HOTKEYS_LIST, LIST_ITEM_DOUBLECLICKED, (wxObjectEventFunction)&AutomationHotkeysDialog::OnMapHkey);

	wxBoxSizer *buttonsSizer = new wxBoxSizer(wxHORIZONTAL);
	MappedButton *OK = new MappedButton(this, ID_HOTKEYS_OK, "OK");
	Connect(ID_HOTKEYS_OK, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&AutomationHotkeysDialog::OnOK);
	MappedButton *setHotkey = new MappedButton(this, ID_HOTKEYS_MAP, _("Mapuj skrót"));
	Connect(ID_HOTKEYS_MAP, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&AutomationHotkeysDialog::OnMapHkey);
	MappedButton *deleteHotkey = new MappedButton(this, ID_HOTKEYS_DELETE, _("Usuń skrót"));
	Connect(ID_HOTKEYS_DELETE, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&AutomationHotkeysDialog::OnDeleteHkey);
	MappedButton *cancel= new MappedButton(this, wxID_CANCEL, _("Anuluj"));

	buttonsSizer->Add(OK, 0, wxALL, 2);
	buttonsSizer->Add(setHotkey, 0, wxALL, 2);
	buttonsSizer->Add(deleteHotkey, 0, wxALL, 2);
	buttonsSizer->Add(cancel, 0, wxALL, 2);
	
	mainSizer->Add(hotkeysList, 1, wxEXPAND | wxALL, 4);
	mainSizer->Add(buttonsSizer, 0, wxALL | wxALIGN_CENTER, 2);
	SetSizerAndFit(mainSizer);
	CenterOnParent();
	SetEnterId(ID_HOTKEYS_OK);
}

AutomationHotkeysDialog::~AutomationHotkeysDialog()
{

}

void AutomationHotkeysDialog::OnOK(wxCommandEvent &evt)
{
	if (hotkeysList->GetModified()){
		hotkeysList->SaveAll(2);
		Hkeys.SetHotkeysMap(allHotkeys);
		Hkeys.SetAccels(true);
		Hkeys.SaveHkeys();
		EndModal(wxID_OK);
	}

}

void AutomationHotkeysDialog::OnMapHkey(wxCommandEvent &evt)
{
	int inum = hotkeysList->GetSelection();
	if (inum < 0)
		return;

	AutomationHotkeyItem *hitem = (AutomationHotkeyItem *)hotkeysList->GetItem(inum, 2);
	if (!hitem)
		return;

	int id = hitem->id;
	wxString name = hitem->name;

	HkeysDialog hkd(this, name, GLOBAL_HOTKEY, false);

	if (hkd.ShowModal() == wxID_OK){

		wxString hotkey = hitem->accelerator;
		std::vector< std::map<idAndType, hdata>::iterator> idtypes;
		lastScriptId = 30100;
		for (auto cur = allHotkeys.begin(); cur != allHotkeys.end(); cur++){
			if (id < 0 && cur->first.id >= 30100){
				lastScriptId++;
			}
			if (cur->second.Accel == hkd.hotkey && cur->second.Name != name){
				idtypes.push_back(cur);
			}
		}

		if (id < 0){
			id = lastScriptId;
			lastScriptId++;
		}

		if (idtypes.size()){
			wxString windowNames[] = { _("Globalny"), _("Napisy"), _("Edytor"), _("Wideo"), _("Audio") };
			bool doubledHotkey = false;
			wxString doubledHkName;
			for (auto &idtype : idtypes){
				if (idtype->first.Type == hkd.type){
					doubledHotkey = true;
					doubledHkName = Hkeys.GetName(idtype->first.id);
					if (doubledHkName.empty())
						doubledHkName = idtype->second.Name;
					break;
				}
				else{
					if (!doubledHkName.empty())
						doubledHkName += L", ";

					wxString hotkeyName = Hkeys.GetName(idtype->first.id);
					if (hotkeyName.empty())
						hotkeyName = idtype->second.Name;
					doubledHkName += windowNames[idtype->first.Type] + L" " + hotkeyName;
				}
			}

			int result = wxCANCEL;
			if (doubledHotkey){
				KaiMessageDialog msg(this,
					wxString::Format(_("Ten skrót już istnieje jako skrót do \"%s\".\nCo zrobić?"),
					doubledHkName), _("Uwaga"), wxYES | wxOK | wxCANCEL);
				msg.SetOkLabel(_("Zamień skróty"));
				msg.SetYesLabel(_("Usuń skrót"));
				result = msg.ShowModal();
			}
			else{
				int buttonFlag = (idtypes.size() < 2) ? wxOK : 0;
				KaiMessageDialog msg(this,
					wxString::Format(_("Ten skrót już istnieje w %s jako skrót do \"%s\".\nCo zrobić?"),
					(idtypes.size() > 1) ? _("innych oknach") : _("innym oknie"), doubledHkName), _("Uwaga"), wxYES_NO | buttonFlag | wxCANCEL);
				if (idtypes.size() < 2)
					msg.SetOkLabel(_("Zamień skróty"));
				msg.SetYesLabel(_("Usuń skrót"));
				msg.SetNoLabel(_("Ustaw mimo to"));
				result = msg.ShowModal();
			}
			if (result == wxYES || result == wxOK){
				if (result == wxYES){ hotkey = ""; }
				for (auto &idtype : idtypes){
					if (doubledHotkey && idtype->first.Type != hkd.type)
						continue;
					int nitem = hotkeysList->FindItem(2, idtype->second.Name);
					if (nitem >= 0){
						ChangeHotkey(nitem, id, hotkey);
					}
					idtype->second.Accel = hotkey;
				}
			}
			else if (result == wxCANCEL){ return; }
		}
		ChangeHotkey(inum, id, hkd.hotkey);
		hotkeysList->Refresh(false);
		allHotkeys[idAndType(id,GLOBAL_HOTKEY)] = hdata(name, hkd.hotkey);
		hotkeysList->SetModified(true);
		hotkeysList->PushHistory();
	}
}

void AutomationHotkeysDialog::OnDeleteHkey(wxCommandEvent &evt)
{
	int inum = hotkeysList->GetSelection();
	if (inum < 0)
		return;

	AutomationHotkeyItem *hitem = (AutomationHotkeyItem *)hotkeysList->GetItem(inum, 2);
	if (!hitem)
		return;
	int id = hitem->id;
	wxString name = hitem->name;
	if (id < 0)
		return;

	ChangeHotkey(inum, id, "");
	hotkeysList->SetModified(true);
	hotkeysList->PushHistory();
	allHotkeys[idAndType(id, GLOBAL_HOTKEY)] = hdata(name, "");
}

void AutomationHotkeysDialog::ChangeHotkey(int row, int id, const wxString &hotkey)
{
	AutomationHotkeyItem* item = (AutomationHotkeyItem*)hotkeysList->CopyRow(row, 2);
	if (!item)
		return;
	item->accelerator = hotkey;
	item->id = id;
	item->modified = true;
	ItemText* textitem = (ItemText*)hotkeysList->GetItem(row, 0);
	textitem->modified = true;
	textitem = (ItemText*)hotkeysList->GetItem(row, 1);
	textitem->modified = true;
	hotkeysList->Refresh(false);
}
