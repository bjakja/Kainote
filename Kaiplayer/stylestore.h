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

#ifndef STYLESTORE_H
#define STYLESTORE_H



#include <wx/stattext.h>
#include <wx/filedlg.h>
#include <wx/dialog.h>
#include "StyleList.h"
#include "Styles.h"
#include "MappedButton.h"
#include "ListControls.h"
#include "KaiDialog.h"

class ColorChange;


class stylestore: public KaiDialog
{
	public:

		stylestore(wxWindow* parent ,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~stylestore();
		MappedButton* addToAss;
		MappedButton* close;
		MappedButton* assNew;
		MappedButton* assCopy;
		MappedButton* assLoad;
		MappedButton* storeSort;
		MappedButton* assDelete;
		MappedButton* assSort;
		MappedButton* addToStore;
		MappedButton* newCatalog;
		MappedButton* storeNew;
		MappedButton* storeCopy;
		MappedButton* storeLoad;
		MappedButton* storeDelete;
		MappedButton* SClean;
		StyleList* Store;
		KaiChoice* catalogList;
		StyleList* ASS;
		DialogSizer *Mainall;
		ColorChange* cc;
        void changestyle(Styles *cstyl);
		void StylesWindow(wxString newname="");
        void LoadStylesS(bool ass);
		void StyleonVideo(Styles *styl, bool fullskreen=false);
		void DoTooltips();
		void LoadAssStyles();
		void ReloadFonts();

	private:

		void OnAssStyleChange(wxCommandEvent& event);
		void OnAddToStore(wxCommandEvent& event);
		void OnAddToAss(wxCommandEvent& event);
		void OnStoreStyleChange(wxCommandEvent& event);
		void OnChangeCatalog(wxCommandEvent& event);
		void OnNewCatalog(wxCommandEvent& event);
		void OnDelCatalog(wxCommandEvent& event);
		void OnStoreLoad(wxCommandEvent& event);
		void OnStoreSort(wxCommandEvent& event);
		void OnAssLoad(wxCommandEvent& event);
		void OnAssSort(wxCommandEvent& event);
		void OnStoreCopy(wxCommandEvent& event);
		void OnStoreDelete(wxCommandEvent& event);
		void OnAssCopy(wxCommandEvent& event);
		void OnAssDelete(wxCommandEvent& event);
		void OnStoreNew(wxCommandEvent& event);
		void OnAssNew(wxCommandEvent& event);
		void OnConfirm(wxCommandEvent& event);
		void OnClose(wxCommandEvent& event);
		void OnSwitchLines(wxCommandEvent& event);
		void OnCleanStyles(wxCommandEvent& event);
		bool stass;
		bool dummy;
		int selnum;
		int prompt;
		wxString oldname;
		void modif();
		volatile bool stopcheck;
		static DWORD CheckFontProc(void* cls);
		HANDLE thread;
};

enum{
	ID_CATALOG=13245,
	ID_NEWCAT,
	ID_DELCAT,
	ID_ASSSTYLES,
	ID_STORESTYLES,
	ID_STORENEW,
	ID_STORECOPY,
	ID_STORELOAD,
	ID_STOREDEL,
	ID_STORESORT,
	ID_ADDTOSTORE,
	ID_ADDTOASS,
	ID_ASSNEW,
	ID_ASSCOPY,
	ID_ASSLOAD,
	ID_ASSDEL,
	ID_ASSSORT,
	ID_ASSCLEAN,
	ID_CONF,
	ID_CLOSEE
	};

#endif
