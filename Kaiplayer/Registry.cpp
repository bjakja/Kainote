//  Copyright (c) 2012-2018, Marcin Drob

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

#include "Registry.h"
#include <wx/stdpaths.h>
#include <ShlObj.h>

Registry::Registry(HKEY hKey, const wxString &strKey, bool &success, bool canWrite /*= false*/)
{
	success = OpenNewRegistry(hKey, strKey, canWrite);
}

bool Registry::OpenNewRegistry(HKEY hKey, const wxString &strKey, bool canWrite /*= false*/)
{
	LONG nError = RegOpenKeyEx(hKey, strKey.wc_str(), NULL, KEY_ALL_ACCESS, &regHKey);

	if (nError == ERROR_FILE_NOT_FOUND && canWrite)
	{
		nError = RegCreateKeyEx(hKey, strKey.wc_str(), NULL, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &regHKey, NULL);
		if (nError){
			//wxLogStatus("cannot create key %s", strKey);
			return false;
		}
	}
	else if (nError){
		//wxLogStatus("cannot open key %s", strKey);
		return false;
	}
	//else success!!!
	return true;
}

void Registry::CloseRegistry()
{
	if (regHKey){
		RegCloseKey(regHKey);
		regHKey = NULL;
	}
}

Registry::~Registry()
{
	CloseRegistry();
}

void Registry::SetStringValue(const wxString &strKey, const wxString &value)
{
	if (!regHKey) return;

	const wchar_t *data = value.wc_str();
	LONG nError = RegSetValueExW(regHKey, (strKey!="")? strKey.wc_str() : NULL, NULL, REG_SZ, (LPBYTE)data, (wcslen(data) + 1) * 2);
	if (nError){
		wxLogStatus("cannot create key %s", strKey);
	}
}

bool Registry::GetStringValue(const wxString &strKey, wxString &outValue)
{
	if (!regHKey) return false;
	DWORD type = REG_SZ;
	wchar_t data[20048];
	DWORD size = 20048;
	LONG nError = RegQueryValueExW(regHKey, (strKey!="") ? strKey.wc_str() : NULL, NULL, &type, (LPBYTE)&data, &size);
	if (nError){
		//wxLogStatus("cannot create key %s", strKey);
		return false;
	}
	outValue = wxString(data);
	return true;
}

bool Registry::AddFileAssociation(const wxString &extension, const wxString &extName, int icon)
{
	wxStandardPathsBase &paths = wxStandardPaths::Get();
	wxString pathfull = paths.GetExecutablePath();
	wxString progName = pathfull.AfterLast('\\').BeforeFirst('.');
	wxString mainPath = "Software\\Classes\\";
	bool success = false;
	Registry reg(HKEY_CURRENT_USER, mainPath + extension, success, true);
	if (success){
		reg.SetStringValue("", progName + extension);
		reg.CloseRegistry();
	}
	else{ wxLogStatus("Nie mo¿na utowrzyæ rozszerzenia %s", extension); return false; }
	if (reg.OpenNewRegistry(HKEY_CURRENT_USER, mainPath + progName + extension, true)){
		reg.SetStringValue("", extName);
		reg.CloseRegistry();
	}
	else{
		wxLogStatus("Nie mo¿na otowrzyæ klasy rozszerzenia %s"); return false;
	}
	if (reg.OpenNewRegistry(HKEY_CURRENT_USER, mainPath + progName + extension + "\\DefaultIcon", true)){
		reg.SetStringValue("", pathfull.BeforeLast('\\') + "\\Icons.dll," + std::to_string(icon));
		reg.CloseRegistry();
	}
	else{
		wxLogStatus("Nie mo¿na dodaæ ikony"); return false;
	}
	if (reg.OpenNewRegistry(HKEY_CURRENT_USER, mainPath + progName + extension + "\\Shell\\Open\\Command", true)){
		reg.SetStringValue("", "\""+pathfull + "\" \"%1\"");
		reg.CloseRegistry();
	}
	else{
		wxLogStatus("Nie mo¿na dodaæ otwierania za pomoc¹ %s", progName); return false;
	}
	SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
	return true;
}

bool Registry::RemoveFileAssociation(const wxString &extension)
{
	wxString mainPath = "Software\\Classes\\";
	bool success = false;
	Registry reg(HKEY_CURRENT_USER, mainPath + extension, success, true);
	if (success){
		reg.SetStringValue("", "");
		reg.CloseRegistry();
	}
	else{ wxLogStatus("Nie mo¿na usun¹æ rozszerzenia %s", extension); return false; }
	SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
	return true;
}

void Registry::CheckFileAssociation(const wxString *extensions, int numExt, std::vector<bool> &output)
{
	wxStandardPathsBase &paths = wxStandardPaths::Get();
	wxString pathfull = paths.GetExecutablePath();
	wxString progName = pathfull.AfterLast('\\').BeforeFirst('.');
	wxString mainPath = "Software\\Classes\\";
	for (int i = 0; i < numExt; i++){
		bool success = false;
		Registry reg(HKEY_CURRENT_USER, mainPath + extensions[i], success, false);
		if (success){
			wxString out;
			reg.GetStringValue("", out);
			if (out == progName + extensions[i]){
				output.push_back(true);
			}
			else{
				output.push_back(false);
			}
			reg.CloseRegistry();
			
		}
		else{ output.push_back(false); }
	}
}

void Registry::RefreshRegistry()
{
	SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
}
