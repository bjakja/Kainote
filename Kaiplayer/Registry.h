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

#include <wx/wx.h>
#include <vector>

class Registry{
public:
	Registry(HKEY hKey, const wxString &strKey, bool &success, bool canWrite = false);
	bool OpenNewRegistry(HKEY hKey, const wxString &strKey, bool canWrite = false);
	void CloseRegistry();
	~Registry();
	void SetStringValue(const wxString &strKey, const wxString &value);
	bool GetStringValue(const wxString &strKey, wxString &outValue);
	static bool AddFileAssociation(const wxString &extension, const wxString &extName, int icon);
	static bool RemoveFileAssociation(const wxString &extension);
	static void CheckFileAssociation(const wxString *extensions, int numExt, std::vector<bool> &output);
	static void RefreshRegistry();
private:
	HKEY regHKey = NULL;
};