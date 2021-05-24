//  Copyright (c) 2021, Marcin Drob

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

#include "VisualAllTagsEdition.h"
#include "OpennWrite.h"
#include "ListControls.h"
#include "KaiTextCtrl.h"
#include "config.h"

AllTagsEdition::AllTagsEdition(wxWindow* parent, const wxPoint& pos, std::vector<AllTagsSetting>* _tags)
	:KaiDialog(parent, -1, _("Edycja tagów"))
	,tags(_tags)
{
	wxArrayString list;
	GetNames(tags, &list);
	DialogSizer *main = new DialogSizer(wxVERTICAL);
	KaiChoice* tagList = new KaiChoice(this, -1, wxDefaultPosition, wxDefaultSize, list);

}

void LoadSettings(std::vector<AllTagsSetting>* tags)
{
	wxString path = Options.pathfull + L"\\Config\\AllTagsSettings.txt";
	OpenWrite ow;
	wxString txtSettings;
	if (!ow.FileOpen(path, &txtSettings, false)) {
		//write entire setings in plain text
		//Tag: name, tag, min, max, value, step, mode, [valuey]
		txtSettings = L"Tag: blur, \\blur, 0, 200, 0, 0.5, 0\n"\
			L"Tag: border, \\bord, 0, 50, 0, 1, 0\n"\
			L"Tag: blur edge, \\be, 0, 100, 0, 1, 0\n"\
			L"Tag: fading, \\fad, 0, 2000, 0, 5, 1, 0\n"\
			L"Tag: fax, \\fax, -10, 10, 0, 0.05, 0\n"\
			L"Tag: fay, \\fay, -10, 10, 0, 0.05, 0\n"\
			L"Tag: font size, \\fs, 0, 300, 0, 1, 0\n"\
			L"Tag: spacing, \\fsp, -100, 100, 0, 1, 0\n"\
			L"Tag: shadow, \\shad, 0, 80, 0, 1, 0\n"\
			L"Tag: xborder, \\xbord, 0, 80, 0, 1, 0\n"\
			L"Tag: yborder, \\ybord, 0, 80, 0, 1, 0\n"\
			L"Tag: xshadow, \\xshad, -80, 80, 0, 1, 0\n"\
			L"Tag: yshadow, \\yshad, -80, 80, 0, 1, 0\n";
	}
	wxStringTokenizer tokenzer(txtSettings, "\n", wxTOKEN_STRTOK);
	while (tokenzer.HasMoreTokens()) {
		wxString token = tokenzer.GetNextToken();
		wxString tagtxt;
		if (token.StartsWith(L"Tag: ", &tagtxt)) {
			wxStringTokenizer tkzer(tagtxt, ",", wxTOKEN_STRTOK);
			AllTagsSetting tmp;
			tmp.name = tkzer.GetNextToken();
			if (!tkzer.HasMoreTokens())
				continue;
			tmp.tag = tkzer.GetNextToken();
			double tmpval = 0;
			if (!tkzer.HasMoreTokens())
				continue;
			if (!tkzer.GetNextToken().ToCDouble(&tmpval))
				continue;
			tmp.rangeMin = tmpval;
			if (!tkzer.HasMoreTokens())
				continue;
			if (!tkzer.GetNextToken().ToCDouble(&tmpval))
				continue;
			tmp.rangeMax = tmpval;
			if (!tkzer.HasMoreTokens())
				continue;
			if (!tkzer.GetNextToken().ToCDouble(&tmpval))
				continue;
			tmp.value = tmpval;
			if (!tkzer.HasMoreTokens())
				continue;
			if (!tkzer.GetNextToken().ToCDouble(&tmpval))
				continue;
			tmp.step = tmpval;
			if (!tkzer.HasMoreTokens())
				continue;
			tmp.mode = wxAtoi(tkzer.GetNextToken());
			if (tkzer.HasMoreTokens()) {
				if (tkzer.GetNextToken().ToCDouble(&tmpval)) {
					tmp.value2 = tmpval;
					tmp.has2value = true;
				}
			}
		}
	}
}

void GetNames(std::vector<AllTagsSetting>* tags, wxArrayString* nameList)
{
}


