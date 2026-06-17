//  Copyright (c) 2026, Marcin Drob

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

#include "Automation.h"
#include "OpennWrite.h"
#include "config.h"
#include <wx/filename.h>
#include <wx/dir.h>


namespace Auto {
	
	void AutomationDummyLoader::FastReloadScripts()
	{
		wxDir dir;

		if (!dir.Open(AutoloadPath)) {
			KaiLog(wxString::Format(L"Failed to open a directory in the Automation autoload path: %s", 
				AutoloadPath.wc_str()));
			return;
		}

		wxString fn;
		wxFileName script_path(AutoloadPath, L"");
		bool more = dir.GetFirst(&fn, wxEmptyString, wxDIR_FILES);
		std::vector<size_t> tableOfScripts;

		while (more) {
			script_path.SetName(fn);
			try {
				wxString fullpath = script_path.GetFullPath();
				wxString ext = fullpath.AfterLast(L'.').Lower();

				if (ext != L"lua" && ext != L"moon") {
					more = dir.GetNext(&fn);
					continue;
				}
				size_t result = FindFilename(fullpath);
				if(result == -1){
					KaiLogSilent(L"Not added script " + fullpath);
					if (!((Automation*)this)->Add(fullpath, false, true)) {
						more = dir.GetNext(&fn);
						continue;
					}
					
					result = Scripts.size() - 1;
				}
				tableOfScripts.push_back(result);
			}
			catch (...) {
				
			}

			more = dir.GetNext(&fn);
		}

		if (tableOfScripts.size() != Scripts.size()) {
			for (size_t i = Scripts.size(); i > 0; i--) {
				bool found = false;
				for (size_t k = 0; k < tableOfScripts.size(); k++) {
					if (tableOfScripts[k] == i - 1) {
						found = true;
						break;
					}
				}
				if (!found) {
					delete Scripts[i - 1];
					Scripts.erase(Scripts.begin() + (i - 1));
				}
			}
		}
	}

	void AutomationDummyLoader::SaveDummy()
	{
		wxString automationDummy = Options.pathfull + L"/Config/Automation.txt";
		OpenWrite ow(automationDummy);

		for (size_t i = 0; i < Scripts.size(); i++) {
			LuaScript* script = Scripts[i];
			wxString result = script->GetFilename();
			result << L" = {\n\tname = ";
			result << script->GetName();
			result << L"\n\tdescription = ";
			result << script->GetDescription();
			result << L"\n\tlowTime = " << std::to_wstring(script->GetLowTime());
			result << L"\n\thighTime = " << std::to_wstring(script->GetHighTime());
			result << L"\n\tmacros = {\n";
			const std::vector<LuaCommand*>& macros = script->GetMacros();
			for (size_t k = 0; k < macros.size(); k++) {
				LuaCommand* macro = macros[k];
				result << L"\t\tname = ";
				result << macro->StrDisplay();
				result << L"\n\t\thelp = ";
				result << macro->StrHelp() << L"\n";
			}
			result << L"\t}\n}\n";
			ow.PartFileWrite(result);
		}
		ow.CloseFile();

	}

	bool AutomationDummyLoader::LoadDummy()
	{
		wxString automationDummy = Options.pathfull + L"/Config/Automation.txt";
		wxString automationList;
		OpenWrite ow;
		ow.FileOpen(automationDummy, &automationList, false);
		if (automationList.empty()) {
			return false;
		}
		ParseDummy(automationList);
		FastReloadScripts();
		return Scripts.size() > 0;
	}

	void AutomationDummyLoader::AddDummy(const wxString& filename, const wxString& name, const wxString& description, const std::vector<wxString>& macros,
		unsigned long lowTime, unsigned long highTime)
	{
		LuaScript* script = new LuaScript(filename, name, description, 
			macros, lowTime, highTime);
		Scripts.push_back(script);
	}

	void AutomationDummyLoader::GetNameValue(const wxString& input)
	{
		wxString value;
		wxString label = input.BeforeFirst(L'=', &value);
		label.Trim(true);
		value.Trim(false);
		if (label == L"macros") {
			parseMacro = true;
			KaiLogSilent(L"parseMacro = true");
		}
		else if (parseMacro) {
			if (label == L"name") {
				macros.push_back(value);
				KaiLogSilent(L"name = " + value);
			}
			else if (label == L"help") {
				macros.push_back(value);
				KaiLogSilent(L"help = " + value);
			}
			else if (input == L"}") {
				parseMacro = false;
				KaiLogSilent(L"parseMacro = false");
			}
			else {
				if ((lastLabel == L"name" || lastLabel == L"help") && macros.size()) {
					macros[macros.size() - 1] << L"\n" << input;
					KaiLogSilent(lastLabel + L" = " + macros[macros.size() - 1]);
					return;
				}
			}
		}
		else if (label == L"name") {
			name = value;
			KaiLogSilent(L"name = " + value);
		}
		else if (label == L"description") {
			description = value;
			KaiLogSilent(L"description = " + value);
		}
		else if (label == L"lowTime") {
			value.ToULong(&lowTime);
			KaiLogSilent(L"lowTime = " + value);
		}
		else if (label == L"highTime") {
			value.ToULong(&highTime);
			KaiLogSilent(L"highTime = " + value);
		}
		else if(value == L"{") {
			filename = label;
			KaiLogSilent(L"filename = " + label);
		}
		else if (input == L"}") {
			KaiLogSilent(L"add dummy");
			AddDummy(filename, name, description, macros, lowTime, highTime);
			macros.clear();
		}
		else {
			if (lastLabel == L"name") {
				name << L"\n" << input;
				KaiLogSilent(L"name = " + name);
			}
			else if (lastLabel == L"description") {
				description << L"\n" << input;
				KaiLogSilent(L"description = " + description);
			}

			return;
		}
		lastLabel = label;
	}

	void AutomationDummyLoader::ParseDummy(const wxString& input)
	{
		wxStringTokenizer tokenizer(input, L"\n", wxTOKEN_STRTOK);
		wxString token;
		bool block = false;

		while (tokenizer.HasMoreTokens()) {
			token = tokenizer.NextToken();
			token.Trim(false);
			token.Trim(true);
			if (!token.empty()) { GetNameValue(token); }
		}
	}

	size_t AutomationDummyLoader::FindFilename(const wxString& filename)
	{
		for (size_t i = 0; i < Scripts.size(); i++) {
			wxString scriptFilename = Scripts[i]->GetFilename();
			if (filename == scriptFilename) {
				return i;
			}
		}
		return -1;
	}

}