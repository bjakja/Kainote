//  Copyright (c) 2012 - 2020, Marcin Drob

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

#include "config.h"
#include "Registry.h"
#include "LogHandler.h"
#include <wx/stdpaths.h>
#include <ShlObj.h>


Registry::Registry(HKEY hKey, const wxString &strKey, bool &success, bool canWrite /*= false*/)
{
	success = OpenNewRegistry(hKey, strKey, canWrite);
}

bool Registry::OpenNewRegistry(HKEY hKey, const wxString &strKey, bool canWrite /*= false*/)
{
	long nError = RegOpenKeyEx(hKey, strKey.wc_str(), 0, KEY_ALL_ACCESS, &regHKey);

	if (nError == ERROR_FILE_NOT_FOUND && canWrite)
	{
		nError = RegCreateKeyEx(hKey, strKey.wc_str(), 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 0, &regHKey, nullptr);
		if (nError){
			return false;
		}
	}
	else if (nError){
		return false;
	}
	//else success!!!
	return true;
}

void Registry::CloseRegistry()
{
	if (regHKey){
		RegCloseKey(regHKey);
		regHKey = nullptr;
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
	long nError = RegSetValueExW(regHKey, (strKey != emptyString)? strKey.wc_str() : 0, 0, REG_SZ, (LPBYTE)data, (wcslen(data) + 1) * 2);
	if (nError){
		KaiLog(wxString::Format(L"cannot create key %s", strKey));
	}
}

bool Registry::GetStringValue(const wxString &strKey, wxString &outValue)
{
	if (!regHKey) return false;
	unsigned long type = REG_SZ;
	wchar_t data[20048];
	unsigned long size = 20048;
	long nError = RegQueryValueExW(regHKey, (strKey != emptyString) ? strKey.wc_str() : nullptr, nullptr, &type, (LPBYTE)&data, &size);
	if (nError){
		return false;
	}
	outValue = wxString(data);
	return true;
}

bool Registry::AddFileAssociation(const wxString &extension, const wxString &extName, int icon)
{
	wxStandardPathsBase &paths = wxStandardPaths::Get();
	wxString pathfull = paths.GetExecutablePath();
	wxString progName = pathfull.AfterLast(L'\\').BeforeFirst(L'.');
	wxString mainPath = L"Software\\Classes\\";
	bool success = false;//HKEY_CURRENT_USER//HKEY_LOCAL_MACHINE
	Registry reg(HKEY_CURRENT_USER, mainPath + extension, success, true);
	if (success){
		reg.SetStringValue(emptyString, progName + extension);
		reg.CloseRegistry();
	}
	else{ KaiLog(wxString::Format(L"Can not create extension %s", extension)); return false; }
	if (reg.OpenNewRegistry(HKEY_CURRENT_USER, mainPath + progName + extension, true)){
		reg.SetStringValue(emptyString, extName);
		reg.CloseRegistry();
	}
	else{
		KaiLog(L"Can not open extension class"); return false;
	}
	if (reg.OpenNewRegistry(HKEY_CURRENT_USER, mainPath + progName + extension + L"\\DefaultIcon", true)){
		reg.SetStringValue(emptyString, pathfull.BeforeLast(L'\\') + L"\\Icons.dll," + std::to_wstring(icon));
		reg.CloseRegistry();
	}
	else{
		KaiLog(L"Can not add icon"); return false;
	}
	if (reg.OpenNewRegistry(HKEY_CURRENT_USER, mainPath + progName + extension + L"\\Shell\\Open\\Command", true)){
		reg.SetStringValue(emptyString, L"\""+pathfull + L"\" \"%1\"");
		reg.CloseRegistry();
	}
	else{
		KaiLog(wxString::Format(L"Can not add open with %s", progName)); return false;
	}
	SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);
	return true;
}

bool Registry::RemoveFileAssociation(const wxString &extension)
{
	wxString mainPath = L"Software\\Classes\\";
	bool success = false;
	Registry reg(HKEY_CURRENT_USER, mainPath + extension, success, true);
	if (success){
		reg.SetStringValue(emptyString, emptyString);
		reg.CloseRegistry();
	}
	else{ KaiLog(wxString::Format(L"Can not remove extension %s", extension)); return false; }
	SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);
	return true;
}

void Registry::CheckFileAssociation(const wxString *extensions, int numExt, std::vector<bool> &output)
{
	wxStandardPathsBase &paths = wxStandardPaths::Get();
	wxString pathfull = paths.GetExecutablePath();
	wxString progName = pathfull.AfterLast(L'\\').BeforeFirst(L'.');
	wxString mainPath = L"Software\\Classes\\";
	for (int i = 0; i < numExt; i++){
		bool success = false;
		Registry reg(HKEY_CURRENT_USER, mainPath + extensions[i], success, false);
		if (success){
			wxString out;
			reg.GetStringValue(emptyString, out);
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
	SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);
}
