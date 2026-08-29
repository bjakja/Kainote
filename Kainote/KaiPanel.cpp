//  Copyright (c) 2021 - 2026, Marcin Drob

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

#include "KaiPanel.h"
#include "ListControls.h"
#include "KaiRadioButton.h"

#include <vector>

void KaiContainer::OnNavigation(wxNavigationKeyEvent& evt)
{
	if (!container) {
		evt.Skip();
		return;
	}
	bool next = evt.GetDirection();
	wxWindow* focused = container->FindFocus();
	if (!focused) {
		evt.Skip();
		return;
	}
	wxWindow* focusedParent = focused->GetParent();
	if (!focusedParent) {
		evt.Skip();
		return;
	}
	bool nextWindowWasNULL = false;
	if (focusedParent->IsKindOf(CLASSINFO(KaiChoice))) {
		focused = focusedParent;
		focusedParent = focusedParent->GetParent();
		if (!focusedParent) {
			evt.Skip();
			return;
		}
	}

	wxWindowList& list = focusedParent->GetChildren();
	auto node = list.Find(focused);
	if (node) {
		auto nextWindow = next ? node->GetNext() : node->GetPrevious();
		while (1) {
			if (!nextWindow) {
				if (nextWindowWasNULL)
					break;

				nextWindowWasNULL = true;
				wxWindow* fparent = focusedParent;
				while (fparent) {
					wxWindow* parent = fparent->GetParent();
					if (!parent)
						break;
					wxWindowList& list1 = parent->GetChildren();
					//if panel is empty then just continue
					//don't give it focus
					if (!list1.GetCount()) {
						break;
					}
					auto node1 = list1.Find(fparent);
					if (node1) {
						fparent = fparent->GetParent();
						nextWindow = next ? node1->GetNext() : node1->GetPrevious();
						FindFocusable(next, &nextWindow);
						if (!nextWindow) {
							nextWindow = next ? list1.GetFirst() : list1.GetLast();
							FindFocusable(next, &nextWindow);
						}
						if (nextWindow)
							break;
					}
					else
						fparent = nullptr;
				}
				if (!nextWindow)
					nextWindow = next ? list.GetFirst() : list.GetLast();
			}
			if (nextWindow) {
				wxObject* data = nextWindow->GetData();
				if (data) {
					wxWindow* win = wxDynamicCast(data, wxWindow);
					if (win) {
						if (win->IsKindOf(CLASSINFO(KaiRadioButton)) && !win->HasFlag(wxRB_SINGLE)) {
							win = FindCheckedRadiobutton(next, &nextWindow, focused);
							if (!win) {
								//get next window before continue
								if (nextWindow)
									nextWindow = next ? nextWindow->GetNext() : nextWindow->GetPrevious();
								continue;
							}
						}
						//while (win->IsKindOf(CLASSINFO(KaiPanel)) || win->HasMultiplePages()) {
						//	wxWindowList& list1 = win->GetChildren();
						//	//if panel is empty then just continue
						//	//don't give it focus
						//	if (!list1.GetCount()) {
						//		win = nullptr;
						//		break;
						//	}
						//	nextWindow = next ? list1.GetFirst() : list1.GetLast();
						//	wxObject* data1 = nextWindow->GetData();
						//	if (data1) {
						//		win = wxDynamicCast(data1, wxWindow);
						//	}
						//}
						if (win->IsFocusable()) {
							win->SetFocus(); return;
						}
					}
				}
			}
			if (!nextWindow)
				break;
			nextWindow = next ? nextWindow->GetNext() : nextWindow->GetPrevious();
		}
	}
}

void KaiContainer::FindFocusable(bool next, wxWindowListNode** node, wxWindow** window)
{
	wxWindowListNode* nextWindow = *node;
	while (nextWindow) {
		//check the window and return focusable window
		//to avoid infinite loop
		wxObject* data = nextWindow->GetData();
		if (data) {
			wxWindow* win = wxDynamicCast(data, wxWindow);
			if (win && win->IsFocusable()) {
				if (window)
					*window = win;
				break;
			}
		}
		nextWindow = next ? nextWindow->GetNext() : nextWindow->GetPrevious();
	}
	*node = nextWindow;
}

wxWindow* KaiContainer::FindCheckedRadiobutton(bool next, wxWindowListNode** listWithRadioButton, wxWindow* focused)
{
	wxWindow* result = nullptr;
	bool beforeGroup = false;
	while (1) {
		if ((*listWithRadioButton)) {
			wxObject* data = (*listWithRadioButton)->GetData();
			if (data) {
				wxWindow* win = wxDynamicCast(data, wxWindow);
				if (win && win->IsFocusable()) {
					if (win->IsKindOf(CLASSINFO(KaiRadioButton))) {
						KaiRadioButton* krb = wxDynamicCast(win, KaiRadioButton);
						if (krb) {
							if (beforeGroup) {
								break;
							}
							if (krb->HasFlag(wxRB_GROUP) || krb->HasFlag(wxRB_SINGLE)) {
								if (next && focused->IsKindOf(CLASSINFO(KaiRadioButton)))
									break;
								else if (!next)
									beforeGroup = true;
							}
							if (krb->GetValue()) {
								result = win;
								break;
							}
						}
					}
					else {
						result = win;
						break;
					}
				}

			}
		}
		else
			break;

		(*listWithRadioButton) = next ? (*listWithRadioButton)->GetNext() :
			(*listWithRadioButton)->GetPrevious();
	}
	return result;
}


void KaiContainer::OnSetFocus(wxFocusEvent& evt)
{
	if (!container) {
		evt.Skip();
		return;
	}

	std::vector<wxWindow*> pending{ container };
	while (!pending.empty()) {
		wxWindow* current = pending.back();
		pending.pop_back();
		wxWindowList& list = current->GetChildren();
		wxWindowListNode* node = list.GetFirst();
		wxWindow* win = nullptr;
		FindFocusable(true, &node, &win);
		if (win) {
			win->SetFocus();
			return;
		}

		for (wxWindowListNode* childNode = list.GetLast(); childNode;
			childNode = childNode->GetPrevious()) {
			wxObject* data = childNode->GetData();
			if (data) {
				wxWindow* child = wxDynamicCast(data, wxWindow);
				if (child && child->GetChildren().GetCount())
					pending.push_back(child);
			}
		}
	}

	evt.Skip();
}
