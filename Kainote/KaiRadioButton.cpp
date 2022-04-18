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


#include "KaiRadioButton.h"
#include "KaiStaticBoxSizer.h"
#include "ListControls.h"
#include "config.h"
#undef GetClassInfo

KaiRadioButton::KaiRadioButton(wxWindow *parent, int id, const wxString& label,
	const wxPoint& pos, const wxSize& size, long style)
	: KaiCheckBox(parent, id, label, pos, size, style)
	//, hasGroup(style & wxRB_GROUP)
	//,isCheckBox(true)
{
	isCheckBox = false;
	Bind(wxEVT_LEFT_DOWN, &KaiRadioButton::OnMouseLeft, this);
	Bind(wxEVT_LEFT_DCLICK, &KaiRadioButton::OnMouseLeft, this);
	if (style & wxRB_GROUP){ SetValue(true); }
	wxAcceleratorEntry entries[4];
	entries[0].Set(wxACCEL_NORMAL, WXK_LEFT, ID_ACCEL_LEFT);
	entries[1].Set(wxACCEL_NORMAL, WXK_RIGHT, ID_ACCEL_RIGHT);
	entries[2].Set(wxACCEL_NORMAL, WXK_UP, ID_ACCEL_LEFT);
	entries[3].Set(wxACCEL_NORMAL, WXK_DOWN, ID_ACCEL_RIGHT);
	wxAcceleratorTable accel(4, entries);
	SetAcceleratorTable(accel);
	Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &evt){SelectPrev(false); }, ID_ACCEL_LEFT);
	Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &evt){SelectNext(false); }, ID_ACCEL_RIGHT);
}

void KaiRadioButton::OnMouseLeft(wxMouseEvent &evt)
{
	clicked = true;
	Refresh(false);
	if (!value){
		value = !value;
		DeselectRest();
		wxCommandEvent evt(wxEVT_COMMAND_RADIOBUTTON_SELECTED, GetId());
		this->ProcessEvent(evt);
	}
	SetFocus();
}

void KaiRadioButton::SetValue(bool _value)
{
	value = _value;
	Refresh(false);
	if (!value){ return; }
	DeselectRest();
}

void KaiRadioButton::DeselectRest()
{
	const wxWindowList& siblings = GetParent()->GetChildren();
	wxWindowList::compatibility_iterator nodeThis = siblings.Find(this);
	wxCHECK_RET(nodeThis, wxT("radio button not a child of its parent?"));

	if (!HasFlag(wxRB_GROUP))
	{
		// ... turn off all radio buttons before it
		for (wxWindowList::compatibility_iterator nodeBefore = nodeThis->GetPrevious();
			nodeBefore;
			nodeBefore = nodeBefore->GetPrevious())
		{
			KaiRadioButton *btn = wxDynamicCast(nodeBefore->GetData(),
				KaiRadioButton);
			if (!btn)
			{
				// don't stop on non radio buttons, we could have intermixed
				// buttons and e.g. static labels
				continue;
			}

			if (btn->HasFlag(wxRB_SINGLE))
			{
				// A wxRB_SINGLE button isn't part of this group
				break;
			}

			/*if ( btn == focus )
				shouldSetFocus = true;
				else if ( btn == focusInTLW )
				shouldSetTLWFocus = true;*/

			btn->SetValue(false);

			if (btn->HasFlag(wxRB_GROUP))
			{
				// even if there are other radio buttons before this one,
				// they're not in the same group with us
				break;
			}
		}
	}
	// ... and also turn off all buttons after this one
	for (wxWindowList::compatibility_iterator nodeAfter = nodeThis->GetNext();
		nodeAfter;
		nodeAfter = nodeAfter->GetNext())
	{
		KaiRadioButton *btn = wxDynamicCast(nodeAfter->GetData(),
			KaiRadioButton);

		if (!btn)
			continue;

		if (btn->HasFlag(wxRB_GROUP | wxRB_SINGLE))
		{
			// no more buttons or the first button of the next group
			break;
		}

		/*if ( btn == focus )
			shouldSetFocus = true;
			else if ( btn == focusInTLW )
			shouldSetTLWFocus = true;*/

		btn->SetValue(false);
	}

	//if(!HasFocus()){SetFocus();}
}
void KaiRadioButton::SelectPrev(bool first)
{
	const wxWindowList& siblings = GetParent()->GetChildren();
	wxWindowList::compatibility_iterator nodeThis = siblings.Find(this);
	wxCHECK_RET(nodeThis, wxT("radio button not a child of its parent?"));

	if (!HasFlag(wxRB_GROUP))
	{
		// ... turn off all radio buttons before it
		for (wxWindowList::compatibility_iterator nodeBefore = nodeThis->GetPrevious();
			nodeBefore;
			nodeBefore = nodeBefore->GetPrevious())
		{
			KaiRadioButton *btn = wxDynamicCast(nodeBefore->GetData(),
				KaiRadioButton);
			if (!btn)
			{
				// don't stop on non radio buttons, we could have intermixed
				// buttons and e.g. static labels
				continue;
			}

			if (btn->HasFlag(wxRB_SINGLE))
			{
				// A wxRB_SINGLE button isn't part of this group
				SelectNext(true);
				break;
			}

			if (first){
				if (btn->HasFlag(wxRB_GROUP))
				{
					btn->SetValue(true);
					btn->SetFocus();
					wxCommandEvent evt(wxEVT_COMMAND_RADIOBUTTON_SELECTED, GetId());
					this->ProcessEvent(evt);
					break;
				}
			}
			else if (btn->CanBeFocused()){
				btn->SetValue(true);
				btn->SetFocus();
				wxCommandEvent evt(wxEVT_COMMAND_RADIOBUTTON_SELECTED, GetId());
				this->ProcessEvent(evt);
				break;
			}
		}
	}
	else{
		SelectNext(true);
	}


}

void KaiRadioButton::SelectNext(bool last)
{
	const wxWindowList& siblings = GetParent()->GetChildren();
	wxWindowList::compatibility_iterator nodeThis = siblings.Find(this);
	wxCHECK_RET(nodeThis, wxT("radio button not a child of its parent?"));
	bool done = false;
	KaiRadioButton *btntmp = nullptr;

	for (wxWindowList::compatibility_iterator nodeAfter = nodeThis->GetNext();
		nodeAfter;
		nodeAfter = nodeAfter->GetNext())
	{
		KaiRadioButton *btn = wxDynamicCast(nodeAfter->GetData(),
			KaiRadioButton);

		if (!btn){
			if (btntmp && btntmp->CanBeFocused()){
				btntmp->SetValue(true); done = true;
				btntmp->SetFocus();
				wxCommandEvent evt(wxEVT_COMMAND_RADIOBUTTON_SELECTED, GetId());
				this->ProcessEvent(evt);
				break;
			}
			continue;
		}

		if (btn->HasFlag(wxRB_GROUP | wxRB_SINGLE))
		{
			// no more buttons or the first button of the next group
			if (last && btn->CanBeFocused()){
				btn->SetValue(true);
				btn->SetFocus();
				wxCommandEvent evt(wxEVT_COMMAND_RADIOBUTTON_SELECTED, GetId());
				this->ProcessEvent(evt);
			}
			else{
				SelectPrev(true);
			}
			done = true;
			break;
		}

		if (!last && btn->CanBeFocused()){
			btn->SetValue(true);
			btn->SetFocus();
			wxCommandEvent evt(wxEVT_COMMAND_RADIOBUTTON_SELECTED, GetId());
			this->ProcessEvent(evt);
			done = true;
			break;
		}
		btntmp = btn;
	}
	if (!done){
		SelectPrev(true);
	}
}

wxIMPLEMENT_ABSTRACT_CLASS(KaiRadioButton, wxWindow);

KaiRadioBox::KaiRadioBox(wxWindow *parent, int id, const wxString& label,
	const wxPoint& pos, const wxSize& size, const wxArrayString &names, int spacing, long style)
	: wxWindow(parent, id, pos, size)
{
	SetBackgroundColour(parent->GetBackgroundColour());
	SetForegroundColour(parent->GetForegroundColour());
	box = new KaiStaticBoxSizer(style, this, label);
	selected = 0;
	for (size_t i = 0; i < names.size(); i++){
		buttons.push_back(new KaiRadioButton(this, 9876 + i, names[i], wxDefaultPosition, wxDefaultSize, (i == 0) ? wxRB_GROUP : 0));
		box->Add(buttons[i], 1, wxALL, spacing);
	}
	Bind(wxEVT_COMMAND_RADIOBUTTON_SELECTED, [=](wxCommandEvent &evt){
		selected = evt.GetId() - 9876;
		wxCommandEvent *evtrb = new wxCommandEvent(wxEVT_COMMAND_RADIOBOX_SELECTED, GetId());
		wxQueueEvent(GetParent(), evtrb);
	}, 9876, 9876 + names.size() - 1);
	SetSizerAndFit(box);
	Bind(wxEVT_ERASE_BACKGROUND, [=](wxEraseEvent &evt){});
	//Bind(wxEVT_NAVIGATION_KEY, &KaiRadioBox::OnNavigation, this);
}


int KaiRadioBox::GetSelection()
{
	for (size_t i = 0; i < buttons.size(); i++) {
		if (buttons[i]->GetValue()) {
			selected = i;
			return i;
		}
	}
	return -1;
}

void KaiRadioBox::SetSelection(int sel)
{
	selected = MID(0, (size_t)sel, buttons.size() - 1);
	buttons[selected]->SetValue(true);
}

bool KaiRadioBox::Enable(bool enable)
{
	for (size_t i = 0; i < buttons.size(); i++){
		buttons[i]->Enable(enable);
	}
	box->Enable(enable);
	return wxWindow::Enable(enable);
}

void KaiRadioBox::SetFocus()
{
	for (size_t i = 0; i < buttons.size(); i++) {
		if (buttons[i]->GetValue()) {
			buttons[i]->SetFocus();
			selected = i;
		}
	}
}

//void KaiRadioBox::OnNavigation(wxNavigationKeyEvent& evt)
//{
//	bool next = evt.GetDirection();
//	wxWindow* focused = FindFocus();
//	wxWindowList& list = this->GetChildren();
//	auto node = list.Find(focused);
//	if (node) {
//		auto nextWindow = next ? node->GetNext() : node->GetPrevious();
//		while (1) {
//			if (!nextWindow) {
//				//nextWindow = next ? list.GetFirst() : list.GetLast();
//				wxWindowList& list1 = this->GetParent()->GetChildren();
//				nextWindow = list1.Find(this);
//				if (!nextWindow) {
//					KaiLog("Cannot find radiobox control");
//					return;
//				}
//				nextWindow = next ? nextWindow->GetNext() : nextWindow->GetPrevious();
//				if (!nextWindow) {
//					nextWindow = next ? list1.GetFirst() : list1.GetLast();
//				}
//				if (!nextWindow) {
//					KaiLog("Cannot find any controls on window with radiobox");
//					return;
//				}
//			}
//			if (nextWindow) {
//				wxObject* data = nextWindow->GetData();
//				if (data) {
//					wxWindow* win = wxDynamicCast(data, wxWindow);
//					if (win && win->IsFocusable()) {
//						win->SetFocus(); return;
//					}
//				}
//			}
//			nextWindow = next ? nextWindow->GetNext() : nextWindow->GetPrevious();
//		}
//	}
//}

wxIMPLEMENT_ABSTRACT_CLASS(KaiRadioBox, wxWindow);
