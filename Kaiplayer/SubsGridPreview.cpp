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

#include "SubsGridPreview.h"
#include "SubsGrid.h"
#include "SubsGridFiltering.h"
#include "KainoteMain.h"
#include "Utils.h"
#include <wx/regex.h>

SubsGridPreview::SubsGridPreview(SubsGrid *_previewGrid, SubsGrid *windowToDraw, int posY, const wxSize &size)
	:wxWindow((wxWindow*)windowToDraw, -1, wxPoint(0, posY), size)
	, previewGrid(_previewGrid)
	, parent(windowToDraw)
{
	if (!previewGrid){
		NewSeeking(false);
	}
	else{
		previewGrid->thisPreview = this;
	}
	scrollbar = new KaiScrollbar(this, 5432, wxDefaultPosition, wxDefaultSize, wxVERTICAL);
	Bind(wxEVT_PAINT, &SubsGridPreview::OnPaint, this);
	Bind(wxEVT_SIZE, &SubsGridPreview::OnSize, this);
	Bind(wxEVT_ERASE_BACKGROUND, [=](wxEraseEvent &evt){});
	Bind(wxEVT_MOUSE_CAPTURE_LOST, &SubsGridPreview::OnLostCapture, this);
	Bind(wxEVT_SCROLL_THUMBTRACK, &SubsGridPreview::OnScroll, this);
	Bind(wxEVT_MOUSEWHEEL, &SubsGridPreview::OnMouseEvent, this);
	Bind(wxEVT_MOTION, &SubsGridPreview::OnMouseEvent, this);
	Bind(wxEVT_LEFT_DOWN, &SubsGridPreview::OnMouseEvent, this);
	Bind(wxEVT_LEFT_UP, &SubsGridPreview::OnMouseEvent, this);
	Bind(wxEVT_LEFT_DCLICK, &SubsGridPreview::OnMouseEvent, this);
	Bind(wxEVT_MIDDLE_DOWN, &SubsGridPreview::OnMouseEvent, this);
	Bind(wxEVT_RIGHT_DOWN, &SubsGridPreview::OnMouseEvent, this);
	Bind(wxEVT_LEAVE_WINDOW, &SubsGridPreview::OnMouseEvent, this);
	Bind(wxEVT_ENTER_WINDOW, &SubsGridPreview::OnMouseEvent, this);
	Bind(wxEVT_SET_FOCUS, &SubsGridPreview::OnFocus, this);
	Bind(wxEVT_KILL_FOCUS, &SubsGridPreview::OnFocus, this);
	wxAcceleratorEntry gentries[2];
	gentries[0].Set(wxACCEL_CTRL, (int)L'C', PREVIEW_COPY);
	gentries[1].Set(wxACCEL_CTRL, (int)L'V', PREVIEW_PASTE);
	wxAcceleratorTable accelg(2, gentries);
	SetAcceleratorTable(accelg);
	Bind(wxEVT_COMMAND_MENU_SELECTED, &SubsGridPreview::OnAccelerator, this);
	Bind(wxEVT_COMMAND_MENU_SELECTED, &SubsGridPreview::OnAccelerator, this);
	MakeVisible();
}

SubsGridPreview::~SubsGridPreview()
{
	if (bmp){ delete bmp; bmp = NULL; }
}

void SubsGridPreview::MakeVisible()
{
	int w, h;
	GetClientSize(&w, &h);
	int erow = previewGrid->currentLine;
	if ((previewGrid->scrollPosition > erow || previewGrid->GetKeyFromPosition(previewGrid->scrollPosition, (h / (previewGrid->GridHeight + 1))) < erow + 2)){
		previewGrid->scrollPosition = previewGrid->GetKeyFromPosition(erow, -((h / (previewGrid->GridHeight + 1)) / 2) + 1);
		previewGrid->scrollPositionId = previewGrid->file->GetElementByKey(previewGrid->scrollPosition);
	}
	if (!lastData.grid){
		lastData.lineRangeStart = erow;
	}
	Refresh(false);
}

void SubsGridPreview::DestroyPreview(bool refresh, bool destroyingPreviewTab)
{
	parent->preview = NULL;
	previewGrid->thisPreview = NULL;
	if (!Options.GetClosing()){
		TabPanel *tab = (destroyingPreviewTab) ? (TabPanel*)previewGrid->GetParent() : (TabPanel*)parent->GetParent();
		tab->Edit->SetGrid(tab->Grid);
		if (tab->Edit->TextEditOrig->IsShown() != tab->Grid->hasTLMode){
			tab->Edit->SetTlMode(tab->Grid->hasTLMode, true);
		}
		tab->Edit->SetLine(tab->Grid->currentLine);

		if (refresh)
			parent->Refresh(false);
	}
	Destroy();
}

void SubsGridPreview::NewSeeking(bool makeVisible/*=true*/)
{
	SeekForOccurences();
	previewGrid->ChangeActiveLine(lastData.lineRangeStart);
	if (makeVisible)
		MakeVisible();
}

void SubsGridPreview::OnPaint(wxPaintEvent &evt)
{
	int w = 0;
	int h = 0;
	GetClientSize(&w, &h);
	bool bg = false;
	int size = previewGrid->file->GetIdCount();
	int panelrows = (h / (previewGrid->GridHeight + 1));
	if (previewGrid->scrollPosition < 0){ previewGrid->scrollPosition = 0; previewGrid->scrollPositionId = 0; }
	int scrows = previewGrid->scrollPositionId + panelrows;
	//On the end of subtitles
	if (scrows >= size + 2){
		bg = true;
		scrows = size;
		previewGrid->scrollPositionId = (scrows - panelrows) + 2;//end of subtitles
		previewGrid->scrollPosition = previewGrid->file->GetElementById(previewGrid->scrollPositionId);
		//when all subtitles are visible do not scrolling position = 0
		if (panelrows > size + 3){ previewGrid->scrollPosition = 0; previewGrid->scrollPositionId = 0; }
	}
	else if (scrows >= size + 1){
		bg = true;
		scrows--;
		//reduced to avoid crash or maybe now not needed cause is key + i < getcount()
	}

	scrollbar->SetSize(w - 21, previewGrid->GridHeight, 17, h - previewGrid->GridHeight - 4);
	scrollbar->SetScrollbar(previewGrid->scrollPositionId, panelrows, size + 3, panelrows - 3);

	// Prepare bitmap
	if (bmp) {
		if (bmp->GetWidth() < w + scHor || bmp->GetHeight() < h) {
			delete bmp;
			bmp = NULL;
		}
	}
	if (!bmp) bmp = new wxBitmap(w + scHor, h);

	// Draw bitmap
	wxMemoryDC tdc;
	tdc.SelectObject(*bmp);
	int firstCol = previewGrid->GridWidth[0] + 1;



	tdc.SetFont(previewGrid->font);

	const wxColour &header = Options.GetColour(GRID_HEADER);
	const wxColour &headerText = Options.GetColour(GRID_HEADER_TEXT);
	const wxColour &labelBkCol = Options.GetColour(GRID_LABEL_SAVED);
	const wxColour &labelBkColN = Options.GetColour(GRID_LABEL_NORMAL);
	const wxColour &labelBkColM = Options.GetColour(GRID_LABEL_MODIFIED);
	const wxColour &labelBkColD = Options.GetColour(GRID_LABEL_DOUBTFUL);
	const wxColour &linesCol = Options.GetColour(GRID_LINES);
	const wxColour &subsBkCol = Options.GetColour(GRID_DIALOGUE);
	const wxColour &comm = Options.GetColour(GRID_COMMENT);
	const wxColour &seldial = Options.GetColour(GRID_SELECTION);
	const wxColour &textcol = Options.GetColour(GRID_TEXT);
	const wxColour &collcol = Options.GetColour(GRID_COLLISIONS);
	const wxColour &SpelcheckerCol = Options.GetColour(GRID_SPELLCHECKER);
	const wxColour &ComparisonCol = Options.GetColour(GRID_COMPARISON_OUTLINE);
	const wxColour &ComparisonBG = Options.GetColour(GRID_COMPARISON_BACKGROUND_NOT_MATCH);
	const wxColour &ComparisonBGMatch = Options.GetColour(GRID_COMPARISON_BACKGROUND_MATCH);
	const wxColour &ComparisonBGCmnt = Options.GetColour(GRID_COMPARISON_COMMENT_BACKGROUND_NOT_MATCH);
	const wxColour &ComparisonBGCmntMatch = Options.GetColour(GRID_COMPARISON_COMMENT_BACKGROUND_MATCH);
	const wxString &chtag = Options.GetString(GRID_TAGS_SWAP_CHARACTER);
	const wxColour &visibleOnVideo = Options.GetColour(GRID_LINE_VISIBLE_ON_VIDEO);
	bool SpellCheckerOn = Options.GetBool(SPELLCHECKER_ON);

	tdc.SetPen(*wxTRANSPARENT_PEN);
	tdc.SetBrush(wxBrush(linesCol));
	tdc.DrawRectangle(0, 0, w + scHor, h);

	int ilcol;
	int posY = 0;
	int posX = 4;

	bool isComment = false;
	bool unkstyle = false;
	bool shorttime = false;
	bool startBlock = false;
	bool hasFocus = HasFocus();
	int states = 0;
	int startDrawPosYFromPlus = 0;

	size_t keySize = previewGrid->GetCount();

	if (previewGrid->SpellErrors.size() < keySize){
		previewGrid->SpellErrors.resize(keySize);
	}

	TabPanel *tab = (TabPanel*)previewGrid->GetParent();
	TabPanel *tabp = (TabPanel*)parent->GetParent();
	Dialogue *acdial = /*(size > 0) ? */previewGrid->GetDialogue(previewGrid->currentLine)/* : NULL*/;
	Dialogue *Dial = NULL;

	int VideoPos = tab->Video->GetState() != None ? tab->Video->Tell() : -1;

	int fw, fh, bfw, bfh;
	wxColour kol;

	std::vector<wxString> strings;
	int key = previewGrid->scrollPosition - 1;
	int id = previewGrid->scrollPositionId - 1;
	int idmarkerPos = -1;
	int idcurrentLine = -1;

	while (key + 1 <= keySize && id < scrows){
		bool isHeadline = (key < previewGrid->scrollPosition);
		if (!isHeadline){
			Dial = previewGrid->file->GetDialogue(key);
			if (!Dial->isVisible){ key++; continue; }
		}

		if (previewGrid->isFiltered){
			posX = 15;
			unsigned char hasHiddenBlock = previewGrid->file->CheckIfHasHiddenBlock(key, isHeadline);
			if (hasHiddenBlock){
				tdc.SetBrush(*wxTRANSPARENT_BRUSH);
				tdc.SetPen(textcol);
				int halfGridHeight = (previewGrid->GridHeight / 2);
				int newPosY = posY + previewGrid->GridHeight + 1;
				int startDrawPosY = newPosY + ((previewGrid->GridHeight - 10) / 2) - halfGridHeight;
				tdc.DrawRectangle(5, startDrawPosY, 9, 9);
				tdc.DrawLine(7, newPosY - 1, 12, newPosY - 1);
				if (hasHiddenBlock == 1){
					tdc.DrawLine(9, startDrawPosY + 2, 9, startDrawPosY + 7);
				}
				//tdc.SetPen(SpelcheckerCol);
				tdc.DrawLine(14, newPosY - 1, w + scHor, newPosY - 1);
			}
			if (Dial){
				if (!startBlock && Dial->isVisible == VISIBLE_BLOCK){
					startDrawPosYFromPlus = posY + 4; startBlock = true;
				}
				bool isLastLine = (id >= scrows - 2);
				bool notVisibleBlock = Dial->isVisible != VISIBLE_BLOCK;
				if (startBlock && (notVisibleBlock || isLastLine)){
					tdc.SetBrush(*wxTRANSPARENT_BRUSH);
					tdc.SetPen(textcol);
					int halfLine = posY - 1;
					if (isLastLine && !notVisibleBlock){ halfLine = posY + previewGrid->GridHeight; }
					tdc.DrawLine(9, startDrawPosYFromPlus, 9, halfLine);
					if (notVisibleBlock || key + 1 >= keySize || previewGrid->file->GetDialogue(key + 1)->isVisible != VISIBLE_BLOCK)
						tdc.DrawLine(9, halfLine, w + scHor, halfLine);
					startBlock = false;
				}
			}


		}
		else{
			posX = 4;
		}

		bool comparison = false;
		bool isSelected = false;
		strings.clear();

		if (isHeadline){
			tdc.SetBrush(wxBrush(Options.GetColour(hasFocus ? WINDOW_BORDER_BACKGROUND : WINDOW_BORDER_BACKGROUND_INACTIVE)));
			tdc.SetPen(*wxTRANSPARENT_PEN);
			tdc.DrawRectangle(0, posY, w + scHor, previewGrid->GridHeight);
			GetTextExtent(tab->SubsName, &fw, &fh, NULL, NULL, &previewGrid->font);
			int center = ((w - fw) / 2) + scHor;
			tdc.SetTextForeground(Options.GetColour(WINDOW_HEADER_TEXT));
			tdc.DrawText(tab->SubsName, center, 1);
			int xHeight = previewGrid->GridHeight - 6;
			int wPos = w + scHor - 21;
			if (onX || pushedX){
				tdc.SetBrush(Options.GetColour(pushedX ? WINDOW_PUSHED_CLOSE_BUTTON : WINDOW_HOVER_CLOSE_BUTTON));
				tdc.DrawRectangle(wPos, 3, xHeight + 2, xHeight + 2);
			}
			tdc.SetPen(wxPen(Options.GetColour(WINDOW_HEADER_TEXT), 2));
			tdc.DrawLine(wPos + 2, 5, wPos + xHeight - 2, xHeight + 1);
			tdc.DrawLine(wPos + xHeight - 2, 5, wPos + 2, xHeight + 1);

			posY += previewGrid->GridHeight + 1;
			id++;
			key++;
			continue;
		}
		else{


			strings.push_back(wxString::Format(L"%i", id + 1));

			isComment = Dial->IsComment;
			if (key == previewGrid->markedLine)
				idmarkerPos = id;
			if (key == previewGrid->currentLine)
				idcurrentLine = id;

			states = Dial->GetState();
			if (previewGrid->subsFormat < SRT){
				strings.push_back(wxString::Format(L"%i", Dial->Layer));
			}

			if (previewGrid->showFrames && tab->Video->HasFFMS2()){
				VideoFfmpeg *FFMS2 = tab->Video->GetFFMS2();
				wxString frame;
				frame << FFMS2->GetFramefromMS(Dial->Start.mstime);
				strings.push_back(frame);
				if (previewGrid->subsFormat != TMP){
					frame = L"";
					frame << FFMS2->GetFramefromMS(Dial->End.mstime) - 1;
					strings.push_back(frame);
				}
			}
			else{
				strings.push_back(Dial->Start.raw(previewGrid->subsFormat));
				if (previewGrid->subsFormat != TMP){ strings.push_back(Dial->End.raw(previewGrid->subsFormat)); }
			}

			if (previewGrid->subsFormat < SRT){
				if (previewGrid->FindStyle(Dial->Style) == -1){ unkstyle = true; }
				else{ unkstyle = false; }
				strings.push_back(Dial->Style);
				strings.push_back(Dial->Actor);
				strings.push_back(wxString::Format(L"%i", Dial->MarginL));
				strings.push_back(wxString::Format(L"%i", Dial->MarginR));
				strings.push_back(wxString::Format(L"%i", Dial->MarginV));
				strings.push_back(Dial->Effect);
			}
			wxString txt = Dial->Text;
			wxString txttl = Dial->TextTl;

			if (previewGrid->subsFormat != TMP && !(CPS & previewGrid->visibleColumns)){
				int chtime;
				if (previewGrid->SpellErrors[key].size() < 1){
					chtime = previewGrid->CalcChars((previewGrid->hasTLMode && txttl != L"") ? txttl : txt) /
						((Dial->End.mstime - Dial->Start.mstime) / 1000.0f);
					if (chtime < 0 || chtime > 999){ chtime = 999; }
					previewGrid->SpellErrors[key].push_back(chtime);

				}
				else{ chtime = previewGrid->SpellErrors[key][0]; }
				strings.push_back(wxString::Format(L"%i", chtime));
				shorttime = chtime > 15;
			}
			else{
				if (previewGrid->subsFormat != TMP){ strings.push_back(L""); }
				if (previewGrid->SpellErrors[key].size() == 0){ previewGrid->SpellErrors[key].push_back(0); }
			}

			if (previewGrid->hideOverrideTags){
				wxRegEx reg(L"\\{[^\\{]*\\}", wxRE_ADVANCED);
				reg.ReplaceAll(&txt, chtag);
				if (previewGrid->showOriginal){ reg.ReplaceAll(&txttl, chtag); }
			}
			if (txt.Len() > 1000){ txt = txt.SubString(0, 1000) + L"..."; }
			strings.push_back((!previewGrid->showOriginal && previewGrid->hasTLMode && txttl != L"") ? txttl : txt);
			if (previewGrid->showOriginal){ strings.push_back(txttl); }

			if (SpellCheckerOn && (!previewGrid->hasTLMode && txt != L"" || previewGrid->hasTLMode && txttl != L"")){
				if (previewGrid->SpellErrors[key].size() < 2){
					previewGrid->CheckText(strings[strings.size() - 1], previewGrid->SpellErrors[key], chtag);
				}
			}
			isSelected = previewGrid->file->IsSelected(key);
			comparison = (previewGrid->Comparison && previewGrid->Comparison->at(key).size() > 0);
			bool comparisonMatch = (previewGrid->Comparison && !previewGrid->Comparison->at(key).differences);
			bool visibleLine = (Dial->Start.mstime <= VideoPos && Dial->End.mstime > VideoPos);
			kol = (comparison) ? ComparisonBG :
				(comparisonMatch) ? ComparisonBGMatch :
				(visibleLine) ? visibleOnVideo :
				subsBkCol;
			if (isComment){ kol = (comparison) ? ComparisonBGCmnt : (comparisonMatch) ? ComparisonBGCmntMatch : comm; }
			if (isSelected){
				kol = GetColorWithAlpha(seldial, kol);
			}
		}


		ilcol = strings.size();


		wxRect cur;
		bool isCenter;
		wxColour label = (states == 0) ? labelBkColN : (states == 2) ? labelBkCol :
			(states == 1) ? labelBkColM : labelBkColD;
		if (key >= lastData.lineRangeStart && key < lastData.lineRangeStart + lastData.lineRangeLen){
			label = GetColorWithAlpha(wxColour(0, 0, 255, 60), kol);
		}
		for (int j = 0; j < ilcol; j++){
			if (previewGrid->showOriginal&&j == ilcol - 2){
				int podz = (w + scHor - posX) / 2;
				previewGrid->GridWidth[j] = podz;
				previewGrid->GridWidth[j + 1] = podz;
			}

			if (!previewGrid->showOriginal&&j == ilcol - 1){ previewGrid->GridWidth[j] = w + scHor - posX; }


			if (previewGrid->GridWidth[j] < 1){
				continue;
			}
			tdc.SetPen(*wxTRANSPARENT_PEN);

			tdc.SetBrush(wxBrush((j == 0 && !isHeadline) ? label : kol));
			if (unkstyle && j == 4 || shorttime && (j == 10 || (j == 3 && previewGrid->subsFormat>ASS))){
				tdc.SetBrush(wxBrush(SpelcheckerCol));
			}

			tdc.DrawRectangle(posX, posY, previewGrid->GridWidth[j], previewGrid->GridHeight);

			if (!isHeadline && j == ilcol - 1){

				if (previewGrid->SpellErrors[key].size() > 2){
					tdc.SetBrush(wxBrush(SpelcheckerCol));
					for (size_t s = 1; s < previewGrid->SpellErrors[key].size(); s += 2){

						wxString err = strings[j].SubString(previewGrid->SpellErrors[key][s], previewGrid->SpellErrors[key][s + 1]);
						err.Trim();
						if (previewGrid->SpellErrors[key][s]>0){
							wxString berr = strings[j].Mid(0, previewGrid->SpellErrors[key][s]);
							GetTextExtent(berr, &bfw, &bfh, NULL, NULL, &previewGrid->font);
						}
						else{ bfw = 0; }

						GetTextExtent(err, &fw, &fh, NULL, NULL, &previewGrid->font);
						tdc.DrawRectangle(posX + bfw + 3, posY, fw, previewGrid->GridHeight);
					}
				}


				if (comparison){
					tdc.SetTextForeground(ComparisonCol);

					for (size_t c = 1; c < previewGrid->Comparison->at(key).size(); c += 2){
						//if(Comparison->at(i-1)[k]==Comparison->at(i-1)[k+1]){continue;}
						wxString cmp = strings[j].SubString(previewGrid->Comparison->at(key)[c],
							previewGrid->Comparison->at(key)[c + 1]);

						if (cmp == L""){ continue; }
						if (cmp == L" "){ cmp = L"_"; }
						wxString bcmp;
						if (previewGrid->Comparison->at(key)[c]>0){
							bcmp = strings[j].Mid(0, previewGrid->Comparison->at(key)[c]);
							GetTextExtent(bcmp, &bfw, &bfh, NULL, NULL, &previewGrid->font);
						}
						else{ bfw = 0; }

						GetTextExtent(cmp, &fw, &fh, NULL, NULL, &previewGrid->font);
						tdc.DrawText(cmp, posX + bfw + 2, posY);
						tdc.DrawText(cmp, posX + bfw + 4, posY);
						tdc.DrawText(cmp, posX + bfw + 2, posY + 2);
						tdc.DrawText(cmp, posX + bfw + 4, posY + 2);
					}

				}

			}


			bool collis = (!isHeadline && acdial && key != previewGrid->currentLine &&
				(Dial->Start < acdial->End && Dial->End > acdial->Start));

			if (previewGrid->subsFormat < SRT){ isCenter = !(j == 4 || j == 5 || j == 9 || j == 11 || j == 12); }
			else if (previewGrid->subsFormat == TMP){ isCenter = !(j == 2); }
			else{ isCenter = !(j == 4); }

			tdc.SetTextForeground((isHeadline) ? headerText : (collis) ? collcol : textcol);
			cur = wxRect(posX + 3, posY, previewGrid->GridWidth[j] - 6, previewGrid->GridHeight);
			tdc.SetClippingRegion(cur);
			tdc.DrawLabel(strings[j], cur, isCenter ? wxALIGN_CENTER : (wxALIGN_CENTER_VERTICAL | wxALIGN_LEFT));
			tdc.DestroyClippingRegion();

			posX += previewGrid->GridWidth[j] + 1;


		}

		posY += previewGrid->GridHeight + 1;
		id++;
		key++;
	}

	posX = (previewGrid->isFiltered) ? 15 : 4;
	if (bg){
		tdc.SetPen(*wxTRANSPARENT_PEN);
		tdc.SetBrush(wxBrush(Options.GetColour(GRID_BACKGROUND)));
		tdc.DrawRectangle(posX, posY, w + scHor - 8, h);
	}
	if (size > 0){
		if (idmarkerPos != -1){
			tdc.SetBrush(*wxTRANSPARENT_BRUSH);
			tdc.SetPen(wxPen(Options.GetColour(GRID_ACTIVE_LINE), 3));
			tdc.DrawRectangle(posX + 1, ((idmarkerPos - previewGrid->scrollPositionId + 1) *
				(previewGrid->GridHeight + 1)) - 1, (previewGrid->GridWidth[0] - 1), previewGrid->GridHeight + 2);
		}

		if (idcurrentLine != -1){
			tdc.SetBrush(*wxTRANSPARENT_BRUSH);
			tdc.SetPen(wxPen(Options.GetColour(GRID_ACTIVE_LINE)));
			tdc.DrawRectangle(posX, ((idcurrentLine - previewGrid->scrollPositionId + 1) *
				(previewGrid->GridHeight + 1)) - 1, w + scHor - posX - 21, previewGrid->GridHeight + 2);
		}
	}
	tdc.SetBrush(wxBrush(Options.GetColour(hasFocus ? WINDOW_BORDER_BACKGROUND : WINDOW_BORDER_BACKGROUND_INACTIVE)));
	tdc.SetPen(*wxTRANSPARENT_PEN);
	tdc.DrawRectangle(0, h - 4, w + scHor, 4);
	tdc.DrawRectangle(0, 0, 4, h);
	tdc.DrawRectangle(w + scHor - 4, 0, 4, h);
	tdc.SetBrush(*wxTRANSPARENT_BRUSH);
	tdc.SetPen(wxPen(Options.GetColour(hasFocus ? WINDOW_BORDER : WINDOW_BORDER_INACTIVE)));
	tdc.DrawRectangle(0, 0, w + scHor, h);

	wxPaintDC dc(this);
	dc.Blit(0, 0, firstCol + posX, h, &tdc, 0, 0);
	dc.Blit(firstCol + posX, 0, w + scHor, h, &tdc, scHor + firstCol + posX, 0);
}

void SubsGridPreview::OnMouseEvent(wxMouseEvent &event)
{
	int w, h;
	GetClientSize(&w, &h);

	bool shift = event.ShiftDown();
	bool alt = event.AltDown();
	bool ctrl = event.CmdDown();

	bool click = event.LeftDown();
	bool left_up = event.LeftUp();
	bool dclick = event.LeftDClick();
	bool middle = event.MiddleDown();
	bool right = event.RightDown();
	//border on left 4px
	int curY = (event.GetY());
	int curX = (event.GetX()) - 4;

	TabPanel *tab = (TabPanel*)previewGrid->GetParent();
	TabPanel *tabp = (TabPanel*)parent->GetParent();

	if (event.ButtonDown())
		tabp->Edit->SetGrid(previewGrid, true);

	if (onX){
		onX = false;
		wxRect rect(w - 21, 2, 20, previewGrid->GridHeight - 4);
		Refresh(false, &rect);
		if (event.Leaving()){ return; }
	}
	if (curY < previewGrid->GridHeight){
		if (curX + 4 >= w - 21){
			int Width = (previewGrid->GridHeight - 4);
			if (curX + 4 >= (w - 21) + Width || curY < 2 || curY > Width + 2){ return; }
			if (left_up){
				DestroyPreview(true);
			}
			else if (click){
				pushedX = true;
				wxRect rect(w - 21, 2, 20, Width);
				Refresh(false, &rect);
			}
			else if (!onX){
				onX = true;
				wxRect rect(w - 21, 2, 20, Width);
				Refresh(false, &rect);
			}
			return;
		}
	}

	if (left_up && holding) {
		holding = false;
		//Save swap lines after alt release 
		ReleaseMouse();
		if (oldX != -1){ return; }
	}
	if (curX < 0 || curX > w - 4){ return; }

	int row = previewGrid->GetKeyFromScrollPos(curY / (previewGrid->GridHeight + 1)) - 1;
	int hideColumnWidth = (previewGrid->isFiltered) ? 12 : 0;
	bool isNumerizeColumn = (curX >= hideColumnWidth && curX < previewGrid->GridWidth[0] + hideColumnWidth);

	if (left_up && !holding) {
		return;
	}

	// Get focus
	if (event.ButtonDown()) {
		SetFocus();
	}


	//Check if it is tree description line
	if (previewGrid->file->CheckIfIsTree(row)){
		if (event.GetModifiers() == 0){
			if (click){
				int diff = previewGrid->file->OpenCloseTree(row);
				previewGrid->RefreshColumns();
				if (previewGrid->SpellErrors.size() > row + 1)
					previewGrid->SpellErrors.erase(previewGrid->SpellErrors.begin() + (row + 1), previewGrid->SpellErrors.end());

				if (previewGrid->currentLine > row){
					int firstSel = previewGrid->FirstSelection();
					if (firstSel < 0){
						if (previewGrid->currentLine < previewGrid->GetCount())
							previewGrid->file->InsertSelection(previewGrid->currentLine);
						else
							tab->Edit->SetLine(previewGrid->GetCount() - 1);
					}
					else
						tab->Edit->SetLine(firstSel);
				}
			}
			else if (right){
				previewGrid->ContextMenuTree(event.GetPosition(), row);
			}
		}
		return;
	}
	// Seeking video by click on numeration column
	if (click && isNumerizeColumn){
		if (tabp->Video->GetState() != None && !(row < previewGrid->scrollPosition || row >= previewGrid->GetCount())){
			if (tabp->Video->GetState() != Paused){
				if (tabp->Video->GetState() == Stopped){ tabp->Video->Play(); }
				tabp->Video->Pause();
			}
			int vtime = 0;
			bool isstart = true;
			if (shift && previewGrid->subsFormat != TMP){
				vtime = previewGrid->GetDialogue(row)->End.mstime;
				isstart = false;
			}
			else{
				vtime = previewGrid->GetDialogue(row)->Start.mstime;
				isstart = true;
			}
			if (ctrl){ vtime -= 1000; }
			tabp->Video->Seek(MAX(0, vtime), isstart, true, false);
		}
		return;
	}
	// Popup
	if (right && !ctrl) {
		if (isNumerizeColumn){
			previewGrid->markedLine = row;
			Refresh(false);

		}
		//else if(lastData.grid){
		ContextMenu(event.GetPosition());
		//}
		return;
	}

	// Mouse wheel
	if (event.GetWheelRotation() != 0 && row >= previewGrid->scrollPosition) {
		int step = 3 * event.GetWheelRotation() / event.GetWheelDelta();
		previewGrid->scrollPosition = previewGrid->GetKeyFromPosition(previewGrid->scrollPosition, -step);
		previewGrid->scrollPositionId = previewGrid->file->GetElementByKey(previewGrid->scrollPosition);
		Refresh(false);
		return;
	}

	if (curX < hideColumnWidth){
		int filterRow = previewGrid->GetKeyFromScrollPos(((curY + (previewGrid->GridHeight / 2)) / (previewGrid->GridHeight + 1)) - 1) - 1;
		if (filterRow < previewGrid->GetCount() && curY >(previewGrid->GridHeight / 2)) {
			if (click || dclick){
				unsigned char state = previewGrid->file->CheckIfHasHiddenBlock(filterRow, filterRow < previewGrid->scrollPosition);
				if (state){
					SubsGridFiltering filter(previewGrid, previewGrid->currentLine);
					if (filterRow < previewGrid->scrollPosition){
						if (state == 1){
							filterRow = previewGrid->GetKeyFromPosition(filterRow, -1, false);
							previewGrid->scrollPosition = filterRow + 1;
							previewGrid->scrollPositionId = previewGrid->file->GetElementByKey(previewGrid->scrollPosition);
						}
						else{
							previewGrid->scrollPositionId += 1;
							previewGrid->scrollPosition = previewGrid->GetKeyFromPosition(previewGrid->scrollPosition, 1);
						}
					}
					filter.FilterPartial(filterRow);
					Refresh(false);
				}
			}
		}
		return;
	}
	// Click type
	else if (click && curX >= hideColumnWidth) {
		holding = true;
		if (!shift) previewGrid->lastRow = row;
		previewGrid->lastsel = row;
		oldX = (curY < previewGrid->GridHeight) ? curX : -1;
		CaptureMouse();
	}

	if (holding && oldX != -1){
		int diff = (oldX - curX);
		if ((scHor == 0 && diff < 0) || diff == 0 || (scHor > 1500 && diff > 0)){ return; }
		scHor = scHor + diff;
		oldX = curX;
		if (scHor < 0){ scHor = 0; }
		Refresh(false);
		return;
	}
	VideoCtrl *video = tabp->Video;
	bool changeActive = Options.GetBool(GRID_CHANGE_ACTIVE_ON_SELECTION);
	if (!(row < previewGrid->scrollPosition || row >= previewGrid->GetCount())) {


		// Toggle selected
		if (left_up && ctrl && !shift && !alt) {
			if (!(previewGrid->currentLine == row && previewGrid->file->SelectionsSize() == 1 && previewGrid->file->IsSelected(row))){
				previewGrid->SelectRow(row, true, !previewGrid->file->IsSelected(row));
				if (previewGrid->file->SelectionsSize() < 1){ previewGrid->SelectRow(previewGrid->currentLine); }
				Refresh(false);
				return;
			}

		}


		// Normal click
		if (!shift && !alt) {


			//jakbym chcia³ znów daæ zmianê edytowanej linii z ctrl to muszê dorobiæ mu refresh
			if (click && (changeActive || !ctrl) || (dclick && ctrl)) {
				previewGrid->lastActiveLine = previewGrid->currentLine;
				tabp->Edit->SetLine(row, true, true, true, !ctrl);
				if (!previewGrid->Comparison)
					tab->Edit->SetLine(row, true, true, true, !ctrl);

				if (previewGrid->hasTLMode){ tab->Edit->SetActiveLineToDoubtful(); }
				if (changeActive)
					Refresh(false);
				if (!ctrl || dclick){
					previewGrid->SelectRow(row);
					previewGrid->extendRow = -1;
					Refresh(false);
				}
				if (previewGrid->Comparison && !ctrl){
					lastData.lineRangeStart = row;
					lastData.lineRangeLen = 1;
					previewGrid->ShowSecondComparedLine(row, false, true);
				}
			}

			//1-klikniêcie lewym
			//2-klikniêcie lewym i edycja na pauzie
			//3-klikniêcie lewym i edycja na pauzie i odtwarzaniu

			//if (dclick || (click && previewGrid->lastActiveLine != row && mvtal < 4 && mvtal > 0) && pas < 2){
			//tabp->Grid->SetVideoLineTime(event, mvtal);
			//}

			if (click || dclick || left_up)
				return;
		}

		if (middle){
			if (video->GetState() != None){
				video->PlayLine(previewGrid->GetDialogue(row)->Start.mstime,
					video->GetPlayEndTime(previewGrid->GetDialogue(row)->End.mstime));
			}

		}
	}

	// Scroll to keep visibleColumns
	if (holding) {
		previewGrid->MakeVisible(row);
		// End the hold if this was a mousedown to avoid accidental
		// selection of extra lines
		if (click) {
			holding = false;
			left_up = true;
			ReleaseMouse();
		}
	}

	// Block select
	if ((left_up && shift && !alt) || (holding && !ctrl && !alt && !shift && previewGrid->lastsel != row)) {
		if (previewGrid->lastRow != -1) {
			// Keyboard selection continues from where the mouse was last used
			previewGrid->extendRow = previewGrid->lastRow;

			// Set boundaries
			row = MID(0, row, previewGrid->GetCount() - 1);
			int i1 = row;
			int i2 = previewGrid->lastRow;
			if (i1 > i2) {
				int aux = i1;
				i1 = i2;
				i2 = aux;
			}

			// Toggle each
			previewGrid->file->InsertSelections(i1, i2, !ctrl);
			if (changeActive){
				previewGrid->lastActiveLine = previewGrid->currentLine;
				tab->Edit->SetLine(row, true, true, false);
				if (previewGrid->hasTLMode){ tab->Edit->SetActiveLineToDoubtful(); }

			}
			previewGrid->lastsel = row;
			Refresh(false);
		}
	}
}

void SubsGridPreview::OnSize(wxSizeEvent &evt)
{
	Refresh(false);
	GetParent()->Refresh(false);
}

void SubsGridPreview::OnScroll(wxScrollEvent& event)
{
	int newPos = event.GetPosition();
	if (previewGrid->scrollPosition != newPos) {
		previewGrid->scrollPositionId = newPos;
		previewGrid->scrollPosition = previewGrid->file->GetElementById(newPos);
		Refresh(false);
	}
}


void SubsGridPreview::OnAccelerator(wxCommandEvent &evt)
{
	int id = evt.GetId();

	previewGrid->file->GetSelections(previewGrid->selections);
	int sels = previewGrid->selections.size();
	if (id == PREVIEW_COPY && sels > 0) previewGrid->CopyRows(GRID_COPY);
	if (id == PREVIEW_PASTE && sels > 0){ previewGrid->OnPaste(GRID_PASTE); MakeVisible(); }
}

void SubsGridPreview::SeekForOccurences()
{
	if (lastData.grid){ lastData.grid->thisPreview = NULL; }
	TabPanel *tabp = (TabPanel*)parent->GetParent();
	Dialogue * actualDial = parent->GetDialogue(tabp->Grid->currentLine);
	int startTime = actualDial->Start.mstime;
	int endTime = actualDial->End.mstime;
	Notebook * nb = Notebook::GetTabs();
	SubsFile *thisSubs = parent->file;
	previewData.clear();
	int tabI = 0;
	for (int i = 0; i < nb->Size(); i++){
		TabPanel *tab = nb->Page(i);
		SubsFile *subs = tab->Grid->file;
		if (thisSubs == subs){ continue; }
		int lastLine = -2;
		int startMin = INT_MAX;
		int startMax = -1;
		int endMin = INT_MAX;
		int endMax = -1;
		int keyStartMin = 0, keyStartMax = 0, keyEndMin = 0, keyEndMax = 0;
		for (size_t j = 0; j < subs->GetCount(); j++){
			Dialogue *dial = subs->GetDialogue(j);
			if (!dial->isVisible){ continue; }
			if (dial->Start.mstime < endTime && dial->End.mstime > startTime){
				if (lastLine + 1 == j){
					lastLine = j;
					int lastData = previewData.size() - 1;
					previewData[lastData].lineRangeLen = (j - previewData[lastData].lineRangeStart) + 1;
				}
				else{
					lastLine = j;
					previewData.push_back(MultiPreviewData(tab, tab->Grid, j));
				}
			}
			else if (dial->Start.mstime > startMax && dial->Start.mstime < startTime){
				startMax = dial->Start.mstime; keyStartMax = j;
			}
			else if (dial->Start.mstime < startMin && dial->Start.mstime > startTime){
				startMin = dial->Start.mstime; keyStartMin = j;
			}
			else if (dial->End.mstime > endMax && dial->End.mstime < endTime){
				endMax = dial->End.mstime; keyEndMax = j;
			}
			else if (dial->End.mstime < endMin && dial->End.mstime > endTime){
				endMin = dial->End.mstime; keyEndMin = j;
			}

		}
		if (lastLine == -2){
			int bestStart, bestJ;
			int bestEnd, bestJE;

			if (abs(startTime - startMax) > abs(startTime - startMin)){
				bestStart = startMin; bestJ = keyStartMin;
			}
			else{ bestStart = startMax; bestJ = keyStartMax; }
			if (abs(endTime - endMax) > abs(endTime - endMin)){
				bestEnd = endMin; bestJE = keyEndMin;
			}
			else{ bestEnd = endMax; bestJE = keyEndMax; }
			if (abs(startTime - bestStart) > abs(endTime - bestEnd)){
				bestJ = bestJE;
			}
			//bestJ jest naszym wynikiem w tym przypadku, nie potrzebujemy samego czasu który jest najlepszy
			previewData.push_back(MultiPreviewData(tab, tab->Grid, bestJ, 0));
		}
		tabI++;
	}
	size_t previewDataSize = previewData.size();
	for (size_t i = 0; i < previewDataSize; i++){
		if (lastData.grid == previewData[i].grid || i >= previewDataSize - 1){
			size_t position = (lastData.grid == previewData[i].grid) ? i : 0;
			previewGrid = previewData[position].grid;
			previewGrid->thisPreview = this;
			lastData = previewData[position];
			break;
		}
	}

}

void SubsGridPreview::ContextMenu(const wxPoint &pos)
{
	Menu *menu = new Menu();
	SeekForOccurences();
	for (int i = 0; i < previewData.size(); i++){
		wxString name = previewData[i].tab->SubsName + L" (" + std::to_wstring(previewData[i].lineRangeStart) +
			L" " + std::to_wstring(previewData[i].lineRangeLen) + L")";
		MenuItem * Item = menu->Append(4880 + i, name, L"", true, NULL, NULL, (lastData == previewData[i]) ? ITEM_RADIO : ITEM_NORMAL);
	}
	int result = menu->GetPopupMenuSelection(pos, this);
	delete menu;
	if (result < 0){ return; }
	int line = result - 4880;
	if (lastData.grid){ lastData.grid->thisPreview = NULL; }
	lastData = previewData[line];
	TabPanel *tabp = (TabPanel*)parent->GetParent();
	tabp->Edit->SetGrid(parent);
	previewGrid = previewData[line].grid;
	previewGrid->thisPreview = this;
	previewGrid->ChangeActiveLine(previewData[line].lineRangeStart);
	MakeVisible();
}

void SubsGridPreview::OnFocus(wxFocusEvent &evt)
{
	/*if (this == evt.GetWindow()){

	}*/
	Refresh(false);
}

//size_t SubsGridPreview::GetKeyFromScrollPos(size_t numOfLines)
//{
//	size_t visibleLines = 0;
//	for (size_t i = previewGrid->scrollPosition; i < previewGrid->GetCount(); i++){
//		if (numOfLines == visibleLines)
//			return i;
//
//		if (*previewGrid->GetDialogue(i)->isVisible)
//			visibleLines++;
//	}
//
//	return -1;
//}