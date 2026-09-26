//  Copyright (c) 2018 - 2026, Marcin Drob

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

// Checks GitHub for a newer release and tells the user about it. It never
// downloads or installs anything; the dialog opens the release page in a
// browser, as Aegisub's version check does.

#pragma once

#include <wx/string.h>

class wxWindow;

class UpdateChecker
{
public:
	// Honours the auto-check option and the next-check time, and stays silent
	// on error or when already current. Safe to call with no network.
	static void CheckOnStartup(wxWindow *parent);

	// Menu entry: always checks, and reports being up to date and any failure.
	static void CheckNow(wxWindow *parent);

	// Semantic version precedence, with or without a leading "v". False when
	// either side is not a semantic version, such as the old four-part tags.
	static bool IsNewerVersion(const wxString &tag, const wxString &current);

	// "v1.2.0-rc.1" and the like, so a prerelease tag published without
	// GitHub's prerelease flag still stays out of "stable only".
	static bool IsPrerelease(const wxString &tag);
};
