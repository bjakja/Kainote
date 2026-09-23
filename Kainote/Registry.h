//  Copyright (c) 2012 - 2026, Marcin Drob

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

// winsock2.h first: windows.h would pull in the old winsock.h (sockaddr clash).
#ifdef __WXMSW__
#include <winsock2.h>
#endif
#include <windows.h>
#include <wx/string.h>
#include <vector>

class Registry{
public:
	Registry(HKEY hKey, const wxString &strKey, bool &success, bool canWrite = false);
	bool OpenNewRegistry(HKEY hKey, const wxString &strKey, bool canWrite = false);
	void CloseRegistry();
	~Registry();
	void SetStringValue(const wxString &strKey, const wxString &value);
	bool GetStringValue(const wxString &strKey, wxString &outValue);
	//  iconResourceId is an icon id from FileTypeIcons.h, not a position.
	static bool AddFileAssociation(const wxString &extension, const wxString &extName, int iconResourceId);
	static bool RemoveFileAssociation(const wxString &extension);
	static void CheckFileAssociation(const wxString *extensions, int numExt, std::vector<bool> &output);
	//  Repoints any association still naming the retired Icons.dll at the
	//  executable's own icons. Safe to call on every start.
	static void MigrateFileAssociationIcons();
	static void RefreshRegistry();
private:
	HKEY regHKey = NULL;
};