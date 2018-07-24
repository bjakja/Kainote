//  Copyright (c) 2012-2017, Marcin Drob

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
	,previewGrid(_previewGrid)
	, parent(windowToDraw)
{
	if (!previewGrid){ 
		NewSeeking(false);
	}
	else{
		previewGrid->thisPreview = this;
	}
	scPos = previewGrid->scPos;
	scrollbar = new KaiScrollbar(this, 5432, wxDefaultPosition, wxDefaultSize,wxVERTICAL);
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
	TabPanel *tab = (TabPanel*)previewGrid->GetParent();
	int erow = tab->Grid->currentLine;
	if ((scPos > erow || scPos + (h / (previewGrid->GridHeight + 1)) < erow + 2)){
		scPos = MAX(0, erow - ((h / (previewGrid->GridHeight + 1)) / 2) + 1);
	}
	if (!lastData.grid){
		lastData.lineRangeStart = erow;
	}
	Refresh(false);
}

void SubsGridPreview::DestroyPreview(bool refresh)
{
	parent->preview = NULL;
	previewGrid->thisPreview = NULL;
	if(refresh)
		parent->Refresh(false);
	Destroy();
}

void SubsGridPreview::NewSeeking(bool makeVisible/*=true*/)
{
	SeekForOccurences();
	/*if (!occurencesList){
		wxSize size = GetClientSize();
		occurencesList = new KaiListCtrl(this, 3232, occurences, wxPoint(size.x - 304, previewGrid->GridHeight + 1), wxSize(300, size.y - (previewGrid->GridHeight + 5)));
		occurencesList->SetFont(wxFont(7, wxSWISS, wxFONTSTYLE_NORMAL, wxNORMAL, false, "Tahoma", wxFONTENCODING_DEFAULT));
		Bind(LIST_ITEM_LEFT_CLICK, &SubsGridPreview::OnOccurenceChanged, this, 3232);
	}
	else{
		occurencesList->SetTextArray(occurences);
	}
	occurencesList->SetSelection(0);*/
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
	int size = previewGrid->GetCount();
	int panelrows = (h / (previewGrid->GridHeight + 1));
	if (scPos < 0){ scPos = 0; }
	int scrows = scPos + panelrows;
	//gdy widzimy koniec napisów
	if (scrows >= size + 2){
		bg = true;
		scrows = size;
		scPos = (scrows - panelrows) + 2;// dojechanie do koñca napisów
		if (panelrows > size + 3){ scPos = 0; }// w przypadku gdy ca³e napisy s¹ widoczne, wtedy nie skrollujemy i pozycja =0
	}
	else if (scrows >= size + 1){
		bg = true;
		scrows--;//w przypadku gdy mamy liniê przed koñcem napisów musimy zani¿yæ wynik bo przekroczy tablicê.
	}
	//umiejscowienie gridheight+1 od góry, wysokoœæ to co u góry - 4 pasek u do³u
	scrollbar->SetSize(w - 21, previewGrid->GridHeight, 17, h - previewGrid->GridHeight - 4);
	scrollbar->SetScrollbar(scPos, panelrows, size + 3, panelrows - 3);

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

	const wxColour &header = Options.GetColour(GridHeader);
	const wxColour &headerText = Options.GetColour(GridHeaderText);
	const wxColour &labelBkCol = Options.GetColour(GridLabelSaved);
	const wxColour &labelBkColN = Options.GetColour(GridLabelNormal);
	const wxColour &labelBkColM = Options.GetColour(GridLabelModified);
	const wxColour &labelBkColD = Options.GetColour(GridLabelDoubtful);
	const wxColour &linesCol = Options.GetColour(GridLines);
	const wxColour &subsBkCol = Options.GetColour(GridDialogue);
	const wxColour &comm = Options.GetColour(GridComment);
	const wxColour &seldial = Options.GetColour(GridSelection);
	const wxColour &textcol = Options.GetColour(GridText);
	const wxColour &collcol = Options.GetColour(GridCollisions);
	const wxColour &SpelcheckerCol = Options.GetColour(GridSpellchecker);
	const wxColour &ComparisonCol = Options.GetColour(GridComparisonOutline);
	const wxColour &ComparisonBG = Options.GetColour(GridComparisonBackgroundNotMatch);
	const wxColour &ComparisonBGMatch = Options.GetColour(GridComparisonBackgroundMatch);
	const wxColour &ComparisonBGCmnt = Options.GetColour(GridComparisonCommentBackgroundNotMatch);
	const wxColour &ComparisonBGCmntMatch = Options.GetColour(GridComparisonCommentBackgroundMatch);
	const wxString &chtag = Options.GetString(GridTagsSwapChar);
	const wxColour &visibleOnVideo = Options.GetColour(GridVisibleOnVideo);
	bool SpellCheckerOn = Options.GetBool(SpellcheckerOn);

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

	if (previewGrid->SpellErrors.size() < (size_t)size){
		previewGrid->SpellErrors.resize(size);
	}

	TabPanel *tab = (TabPanel*)previewGrid->GetParent();
	TabPanel *tabp = (TabPanel*)parent->GetParent();
	Dialogue *acdial = (size > 0) ? previewGrid->GetDialogue(MID(0, previewGrid->currentLine, size - 1)) : NULL;
	Dialogue *Dial = NULL;
	
	int VideoPos = tab->Video->vstate != None ? tab->Video->Tell() : -1;

	int fw, fh, bfw, bfh;
	wxColour kol;

	std::vector<wxString> strings;
	int i = previewGrid->file->GetElementById(scPos) - 1;
	int k = scPos - 1;

	while (i < previewGrid->file->GetAllCount() && k < scrows){
		bool isHeadline = (k < scPos);
		if (!isHeadline){
			Dial = previewGrid->file->GetDialogueByKey(i);
			if (!Dial->isVisible){ i++; continue; }
		}
		bool comparison = false;
		bool isSelected = false;
		strings.clear();

		if (isHeadline){
			tdc.SetBrush(wxBrush(Options.GetColour(hasFocus ? WindowBorderBackground : WindowBorderBackgroundInactive)));
			tdc.SetPen(*wxTRANSPARENT_PEN);
			tdc.DrawRectangle(0, posY, w + scHor, previewGrid->GridHeight);
			tdc.GetTextExtent(tab->SubsName, &fw, &fh, NULL, NULL, &previewGrid->font);
			int center = ((w - fw) / 2) + scHor;
			tdc.SetTextForeground(Options.GetColour(WindowHeaderText));
			tdc.DrawText(tab->SubsName, center, 1);
			int xHeight = previewGrid->GridHeight - 6;
			int wPos = w + scHor - 21;
			if (onX || pushedX){
				tdc.SetBrush(Options.GetColour(pushedX ? WindowPushedCloseButton : WindowHoverCloseButton));
				tdc.DrawRectangle(wPos, 3, xHeight+2, xHeight+2);
			}
			tdc.SetPen(wxPen(Options.GetColour(WindowHeaderText),2));
			tdc.DrawLine(wPos + 2, 5, wPos + xHeight-2, xHeight+1);
			tdc.DrawLine(wPos + xHeight-2, 5, wPos + 2, xHeight+1);

			posY += previewGrid->GridHeight + 1;
			k++;
			i++;
			continue;
		}
		else{


			strings.push_back(wxString::Format("%i", k + 1));

			isComment = Dial->IsComment;
			//gdy zrobisz inaczej niepewne to u¿yj ^ 4 by wywaliæ 4 ze state.
			states = Dial->GetState();
			if (previewGrid->subsFormat < SRT){
				strings.push_back(wxString::Format("%i", Dial->Layer));
			}

			if (previewGrid->showFrames && tab->Video->VFF){
				VideoFfmpeg *VFF = tab->Video->VFF;
				wxString frame;
				frame << VFF->GetFramefromMS(Dial->Start.mstime);
				strings.push_back(frame);
				if (previewGrid->subsFormat != TMP){
					frame = "";
					frame << VFF->GetFramefromMS(Dial->End.mstime) - 1;
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
				strings.push_back(wxString::Format("%i", Dial->MarginL));
				strings.push_back(wxString::Format("%i", Dial->MarginR));
				strings.push_back(wxString::Format("%i", Dial->MarginV));
				strings.push_back(Dial->Effect);
			}
			wxString txt = Dial->Text;
			wxString txttl = Dial->TextTl;

			if (previewGrid->subsFormat != TMP && !(CPS & previewGrid->visibleColumns)){
				int chtime;
				if (previewGrid->SpellErrors[k].size() < 1){
					chtime = previewGrid->CalcChars((previewGrid->hasTLMode && txttl != "") ? txttl : txt) /
						((Dial->End.mstime - Dial->Start.mstime) / 1000.0f);
					if (chtime < 0 || chtime>999){ chtime = 999; }
					previewGrid->SpellErrors[k].push_back(chtime);

				}
				else{ chtime = previewGrid->SpellErrors[k][0]; }
				strings.push_back(wxString::Format("%i", chtime));
				shorttime = chtime > 15;
			}
			else{
				if (previewGrid->subsFormat != TMP){ strings.push_back(""); }
				if (previewGrid->SpellErrors[k].size() == 0){ previewGrid->SpellErrors[k].push_back(0); }
			}

			if (previewGrid->hideOverrideTags){
				wxRegEx reg("\\{[^\\{]*\\}", wxRE_ADVANCED);
				reg.ReplaceAll(&txt, chtag);
				if (previewGrid->showOriginal){ reg.ReplaceAll(&txttl, chtag); }
			}
			if (txt.Len() > 1000){ txt = txt.SubString(0, 1000) + "..."; }
			strings.push_back((!previewGrid->showOriginal && previewGrid->hasTLMode && txttl != "") ? txttl : txt);
			if (previewGrid->showOriginal){ strings.push_back(txttl); }

			if (SpellCheckerOn && (!previewGrid->hasTLMode && txt != "" || previewGrid->hasTLMode && txttl != "")){
				if (previewGrid->SpellErrors[k].size() < 2){
					previewGrid->CheckText(strings[strings.size() - 1], previewGrid->SpellErrors[k], chtag);
				}
			}
			isSelected = previewGrid->file->IsSelectedByKey(i);
			comparison = (previewGrid->Comparison && previewGrid->Comparison->at(i).size() > 0);
			bool comparisonMatch = (previewGrid->Comparison && !previewGrid->Comparison->at(i).differences);
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

		if (previewGrid->isFiltered){
			posX = 15;
			unsigned char hasHiddenBlock = previewGrid->file->CheckIfHasHiddenBlock(k);
			if (hasHiddenBlock){
				tdc.SetBrush(*wxTRANSPARENT_BRUSH);
				tdc.SetPen(textcol);
				int halfGridHeight = (previewGrid->GridHeight / 2);
				int newPosY = posY + previewGrid->GridHeight + 1;
				int startDrawPosY = newPosY + ((previewGrid->GridHeight - 10) / 2) - halfGridHeight;
				tdc.DrawRectangle(1, startDrawPosY, 9, 9);
				tdc.DrawLine(3, newPosY - 1, 8, newPosY - 1);
				if (hasHiddenBlock == 1){
					tdc.DrawLine(5, startDrawPosY + 2, 5, startDrawPosY + 7);
				}
				//tdc.SetPen(SpelcheckerCol);
				tdc.DrawLine(10, newPosY - 1, w + scHor, newPosY - 1);
			}
			if (Dial){
				if (!startBlock && Dial->isVisible == VISIBLE_BLOCK){
					startDrawPosYFromPlus = posY + 4; startBlock = true;
				}
				bool isLastLine = (k >= scrows - 2);
				bool notVisibleBlock = Dial->isVisible != VISIBLE_BLOCK;
				if (startBlock && (notVisibleBlock || isLastLine)){
					tdc.SetBrush(*wxTRANSPARENT_BRUSH);
					tdc.SetPen(textcol);
					int halfLine = posY - 1;
					if (isLastLine && !notVisibleBlock){ halfLine = posY + previewGrid->GridHeight; }
					tdc.DrawLine(5, startDrawPosYFromPlus, 5, halfLine);
					tdc.DrawLine(5, halfLine, w + scHor, halfLine);
					startBlock = false;
				}
			}


		}
		else{
			posX = 4;
		}

		ilcol = strings.size();


		wxRect cur;
		bool isCenter;
		wxColour label = (states == 0) ? labelBkColN : (states == 2) ? labelBkCol :
			(states == 1) ? labelBkColM : labelBkColD;
		if (i >= lastData.lineRangeStart && i< lastData.lineRangeStart + lastData.lineRangeEnd){ label = GetColorWithAlpha(wxColour(0,0,255,60), kol); }
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

				if (previewGrid->SpellErrors[k].size() > 2){
					tdc.SetBrush(wxBrush(SpelcheckerCol));
					for (size_t s = 1; s < previewGrid->SpellErrors[k].size(); s += 2){

						wxString err = strings[j].SubString(previewGrid->SpellErrors[k][s], previewGrid->SpellErrors[k][s + 1]);
						err.Trim();
						if (previewGrid->SpellErrors[k][s]>0){
							wxString berr = strings[j].Mid(0, previewGrid->SpellErrors[k][s]);
							tdc.GetTextExtent(berr, &bfw, &bfh, NULL, NULL, &previewGrid->font);
						}
						else{ bfw = 0; }

						tdc.GetTextExtent(err, &fw, &fh, NULL, NULL, &previewGrid->font);
						tdc.DrawRectangle(posX + bfw + 3, posY, fw, previewGrid->GridHeight);
					}
				}


				if (comparison){
					tdc.SetTextForeground(ComparisonCol);

					for (size_t c = 1; c < previewGrid->Comparison->at(i).size(); c += 2){
						//if(Comparison->at(i-1)[k]==Comparison->at(i-1)[k+1]){continue;}
						wxString cmp = strings[j].SubString(previewGrid->Comparison->at(i)[c], previewGrid->Comparison->at(i)[c + 1]);

						if (cmp == ""){ continue; }
						if (cmp == " "){ cmp = "_"; }
						wxString bcmp;
						if (previewGrid->Comparison->at(i)[c]>0){
							bcmp = strings[j].Mid(0, previewGrid->Comparison->at(i)[c]);
							tdc.GetTextExtent(bcmp, &bfw, &bfh, NULL, NULL, &previewGrid->font);
						}
						else{ bfw = 0; }

						tdc.GetTextExtent(cmp, &fw, &fh, NULL, NULL, &previewGrid->font);
						if ((cmp.StartsWith("T") || cmp.StartsWith("Y") || cmp.StartsWith(L"£"))){ bfw++; }

						tdc.DrawText(cmp, posX + bfw + 2, posY);
						tdc.DrawText(cmp, posX + bfw + 4, posY);
						tdc.DrawText(cmp, posX + bfw + 2, posY + 2);
						tdc.DrawText(cmp, posX + bfw + 4, posY + 2);
					}

				}

			}


			bool collis = (!isHeadline && acdial && k != previewGrid->currentLine &&
				(Dial->Start < acdial->End && Dial->End > acdial->Start));

			if (previewGrid->subsFormat < SRT){ isCenter = !(j == 4 || j == 5 || j == 9 || j == 11 || j == 12); }
			else if (previewGrid->subsFormat == TMP){ isCenter = !(j == 2); }
			else{ isCenter = !(j == 4); }

			tdc.SetTextForeground((isHeadline) ? headerText : (collis) ? collcol : textcol);
			if (j == ilcol - 1 && (strings[j].StartsWith("T") || strings[j].StartsWith("Y") || strings[j].StartsWith(L"£"))){ posX++; }
			cur = wxRect(posX + 3, posY, previewGrid->GridWidth[j] - 6, previewGrid->GridHeight);
			tdc.SetClippingRegion(cur);
			tdc.DrawLabel(strings[j], cur, isCenter ? wxALIGN_CENTER : (wxALIGN_CENTER_VERTICAL | wxALIGN_LEFT));
			tdc.DestroyClippingRegion();

			posX += previewGrid->GridWidth[j] + 1;


		}

		posY += previewGrid->GridHeight + 1;
		k++;
		i++;
	}

	posX = (previewGrid->isFiltered) ? 15 : 4;
	if (bg){
		tdc.SetPen(*wxTRANSPARENT_PEN);
		tdc.SetBrush(wxBrush(Options.GetColour(GridBackground)));
		tdc.DrawRectangle(posX, posY, w + scHor - 8, h);
	}
	if (size > 0){
		if (previewGrid->markedLine >= scPos && previewGrid->markedLine <= scrows){
			tdc.SetBrush(*wxTRANSPARENT_BRUSH);
			tdc.SetPen(wxPen(Options.GetColour(GridActiveLine), 3));
			tdc.DrawRectangle(posX + 1, ((previewGrid->markedLine - scPos + 1)*(previewGrid->GridHeight + 1)) - 1, (previewGrid->GridWidth[0] - 1), previewGrid->GridHeight + 2);
		}

		if (previewGrid->currentLine >= scPos && previewGrid->currentLine <= scrows){
			tdc.SetBrush(*wxTRANSPARENT_BRUSH);
			tdc.SetPen(wxPen(Options.GetColour(GridActiveLine)));
			tdc.DrawRectangle(posX, ((previewGrid->currentLine - scPos + 1)*(previewGrid->GridHeight + 1)) - 1, w + scHor - posX - 21, previewGrid->GridHeight + 2);
		}
	}
	tdc.SetBrush(wxBrush(Options.GetColour(hasFocus ? WindowBorderBackground : WindowBorderBackgroundInactive)));
	tdc.SetPen(*wxTRANSPARENT_PEN);
	tdc.DrawRectangle(0, h - 4, w + scHor, 4);
	tdc.DrawRectangle(0, 0, 4, h);
	tdc.DrawRectangle(w + scHor - 4, 0, 4, h);
	tdc.SetBrush(*wxTRANSPARENT_BRUSH);
	tdc.SetPen(wxPen(Options.GetColour(hasFocus ? WindowBorder : WindowBorderInactive)));
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
	int curX = (event.GetX())-4;

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
			if (curX + 4 >= (w - 21) + Width || curY<2 || curY>Width+2){ return; }
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
	if (curX < 0 || curX > w-4){ return; }

	int row = curY / (previewGrid->GridHeight + 1) + scPos - 1;
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
		if (tabp->Video->GetState() != None && !(row < scPos || row >= previewGrid->GetCount())){
			if (tabp->Video->GetState() != Paused){
				if (tabp->Video->GetState() == Stopped){ tabp->Video->Play(); }
				tabp->Video->Pause();
			}
			int vtime = 0;
			bool isstart = true;
			if (shift && previewGrid->subsFormat != TMP){
				vtime = previewGrid->GetDialogue(row)->End.mstime; isstart = false;
			}
			else{
				vtime = previewGrid->GetDialogue(row)->Start.mstime; isstart = true;
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
		else if(lastData.grid){
			ContextMenu(event.GetPosition());
		}
		return;
	}

	// Mouse wheel
	if (event.GetWheelRotation() != 0 && row >= scPos) {
		int step = 3 * event.GetWheelRotation() / event.GetWheelDelta();
		scPos -= step;
		previewGrid->ScrollTo(scPos);
		Refresh(false);
		return;
	}

	if (curX < hideColumnWidth){
		int filterRow = (curY /*+ headerHeight*/ + (previewGrid->GridHeight / 2)) / (previewGrid->GridHeight + 1) + scPos - 2;
		if (!(filterRow < scPos || filterRow >= previewGrid->GetCount()) || filterRow == -1) {
			if ((click || dclick) && previewGrid->file->CheckIfHasHiddenBlock(filterRow)){
				SubsGridFiltering filter((SubsGrid*)this, previewGrid->currentLine);
				filter.FilterPartial(filterRow);
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
		if ((scHor == 0 && diff < 0) || diff == 0 || (scHor>1500 && diff>0)){ return; }
		scHor = scHor + diff;
		oldX = curX;
		if (scHor < 0){ scHor = 0; }
		Refresh(false);
		return;
	}
	VideoCtrl *video = tabp->Video;
	bool changeActive = Options.GetBool(GridChangeActiveOnSelection);
	int mvtal = video->vToolbar->videoSeekAfter->GetSelection();
	int pas = video->vToolbar->videoPlayAfter->GetSelection();
	if (!(row < scPos || row >= previewGrid->GetCount())) {

		
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
				tab->Edit->SetLine(row, true, true, true, !ctrl);
				if (previewGrid->hasTLMode){ tab->Edit->SetActiveLineToDoubtful(); }
				if (changeActive){ Refresh(false); }
				if (!ctrl || dclick){
					previewGrid->SelectRow(row);
					previewGrid->extendRow = -1;
					Refresh(false);
				}
				if (previewGrid->Comparison){ previewGrid->ShowSecondComparedLine(row,false,true); }
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
				video->PlayLine(previewGrid->GetDialogue(row)->Start.mstime, video->GetPlayEndTime(previewGrid->GetDialogue(row)->End.mstime));
			}

		}
	}

	// Scroll to keep visibleColumns
	if (holding) {
		// Find direction
		int scdelta = (alt) ? 1 : 3;
		int minVis = scPos + 1;
		int maxVis = scPos + h / (previewGrid->GridHeight + 1) - 2;
		int delta = 0;
		if (row < minVis && row != 0) delta = -scdelta;
		if (row > maxVis) delta = scdelta;

		if (delta) {
			previewGrid->ScrollTo(scPos + delta);//row - (h / (GridHeight+1)) row
			Refresh(false);
			// End the hold if this was a mousedown to avoid accidental
			// selection of extra lines
			if (click) {
				holding = false;
				left_up = true;
				ReleaseMouse();
			}
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
	if (scPos != newPos) {
		scPos = newPos;
		Refresh(false);
	}
}


void SubsGridPreview::SeekForOccurences()
{
	if (lastData.grid){ lastData.grid->thisPreview = NULL; }
	TabPanel *tabp = (TabPanel*)parent->GetParent();
	Dialogue * actualDial = parent->GetDialogue(tabp->Grid->currentLine);
	int startTime = actualDial->Start.mstime;
	int endTime = actualDial->End.mstime;
	Notebook * nb = Notebook::GetTabs();
	File *thisSubs = parent->file->GetSubs();
	previewData.clear();
	int tabI = 0;
	for (int i = 0; i < nb->Size(); i++){
		TabPanel *tab = nb->Page(i);
		File *subs = tab->Grid->file->GetSubs();
		if (thisSubs == subs){ continue; }
		int lastLine = -2;
		int startMin = INT_MAX;
		int startMax = -1;
		int endMin = INT_MAX;
		int endMax = -1;
		int keyStartMin = 0, keyStartMax = 0, keyEndMin = 0, keyEndMax = 0;
		for (size_t j = 0; j < subs->dials.size(); j++){
			Dialogue *dial = subs->dials[j];
			if (!dial->isVisible){ continue; }
			if (dial->Start.mstime < endTime && dial->End.mstime > startTime){
				if (lastLine+1 == j){
					lastLine = j;
					int lastData = previewData.size() - 1;
					previewData[lastData].lineRangeEnd = (j - previewData[lastData].lineRangeStart) + 1;
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
		wxString name = previewData[i].tab->SubsName + " (" + std::to_string(previewData[i].lineRangeStart) + " " + std::to_string(previewData[i].lineRangeEnd) + ")";
		MenuItem * Item = menu->Append(4880 + i, name, "", true, NULL, NULL, (lastData == previewData[i]) ? ITEM_RADIO : ITEM_NORMAL);
	}
	int result = menu->GetPopupMenuSelection(pos, this);
	delete menu;
	if (result < 0){ return; }
	int line = result - 4880;
	if (lastData.grid){ lastData.grid->thisPreview = NULL; }
	lastData = previewData[line];
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

