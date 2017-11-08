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

#include "SubsGridWindow.h"
#include "config.h"
#include "EditBox.h"

#include "kainoteMain.h"
#include "SubsGridFiltering.h"
#include <wx/regex.h>

SubsGridWindow::SubsGridWindow(wxWindow *parent, const long int id, const wxPoint& pos, const wxSize& size, long style)
	:SubsGridBase(parent, id, pos, size, style)
{
	visibleColumns = Options.GetInt(GridHideCollums);
	hideOverrideTags = Options.GetBool(GridHideTags);
	bmp = NULL;
	SetStyle();
	AdjustWidths();
	//SetFocus();
	Bind(wxEVT_PAINT, &SubsGridWindow::OnPaint, this);
	Bind(wxEVT_SIZE, &SubsGridWindow::OnSize, this);
	Bind(wxEVT_KEY_DOWN, &SubsGridWindow::OnKeyPress, this);
	Bind(wxEVT_TIMER, &SubsGridWindow::OnBackupTimer, this, ID_AUTIMER);
	Bind(wxEVT_ERASE_BACKGROUND, [=](wxEraseEvent &evt){});
	Bind(wxEVT_MOUSE_CAPTURE_LOST, &SubsGridWindow::OnLostCapture, this);
	Bind(wxEVT_SCROLLWIN_THUMBTRACK, &SubsGridWindow::OnScroll, this);
	Bind(wxEVT_MOUSEWHEEL, &SubsGridWindow::OnMouseEvent, this);
	Bind(wxEVT_MOTION, &SubsGridWindow::OnMouseEvent, this);
	Bind(wxEVT_LEFT_DOWN, &SubsGridWindow::OnMouseEvent, this);
	Bind(wxEVT_LEFT_UP, &SubsGridWindow::OnMouseEvent, this);
	Bind(wxEVT_LEFT_DCLICK, &SubsGridWindow::OnMouseEvent, this);
	Bind(wxEVT_MIDDLE_DOWN, &SubsGridWindow::OnMouseEvent, this);
	Bind(wxEVT_RIGHT_DOWN, &SubsGridWindow::OnMouseEvent, this);
}

SubsGridWindow::~SubsGridWindow()
{
	if (bmp){ delete bmp; bmp = NULL; }
}

void SubsGridWindow::SetStyle()
{
	wxString fontname = Options.GetString(GridFontName);
	font.SetFaceName(fontname);
	if (!font.IsOk())
		font.SetFamily(wxFONTFAMILY_SWISS);
	font.SetWeight(wxFONTWEIGHT_NORMAL);
	font.SetPointSize(Options.GetInt(GridFontSize));

	{
		wxClientDC dc(this);
		dc.SetFont(font);
		int fw, fh;
		dc.GetTextExtent("#TWFfGH", &fw, &fh, NULL, NULL, &font);
		GridHeight = fh + 2;
	}


	Refresh(false);
}


void SubsGridWindow::OnPaint(wxPaintEvent& event)
{

	int w = 0;
	int h = 0;
	GetClientSize(&w, &h);
	bool bg = false;
	int size = GetCount();
	panelrows = (h / (GridHeight + 1)) + 1;
	int scrows = scPos + panelrows;
	//gdy widzimy koniec napisów
	if (scrows >= size + 3){
		bg = true;
		scrows = size + 1;
		scPos = (scrows - panelrows) + 2;// dojechanie do koñca napisów
		if (panelrows > size + 3){ scPos = 0; }// w przypadku gdy ca³e napisy s¹ widoczne, wtedy nie skrollujemy i pozycja =0
	}
	else if (scrows >= size + 2){
		bg = true;
		scrows--;//w przypadku gdy mamy liniê przed koñcem napisów musimy zani¿yæ wynik bo przekroczy tablicê.
	}
	if (SetScrollBar(wxVERTICAL, scPos, panelrows, size + 3, panelrows - 3)){
		GetClientSize(&w, &h);
	}

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
	int firstCol = GridWidth[0] + 1;



	tdc.SetFont(font);
	
	wxColour header = Options.GetColour(GridHeader);
	wxColour headerText = Options.GetColour(GridHeaderText);
	wxColour labelBkCol = Options.GetColour(GridLabelSaved);
	wxColour labelBkColN = Options.GetColour(GridLabelNormal);
	wxColour labelBkColM = Options.GetColour(GridLabelModified);
	wxColour labelBkColD = Options.GetColour(GridLabelDoubtful);
	wxColour linesCol = Options.GetColour(GridLines);
	wxColour subsBkCol = Options.GetColour(GridDialogue);
	wxColour comm = Options.GetColour(GridComment);
	wxColour seldial = Options.GetColour(GridSelectedDialogue);
	wxColour selcom = Options.GetColour(GridSelectedComment);
	wxColour textcol = Options.GetColour(GridText);
	wxColour collcol = Options.GetColour(GridCollisions);
	wxColour SpelcheckerCol = Options.GetColour(GridSpellchecker);
	wxColour ComparisonCol = Options.GetColour(GridComparison);
	wxColour ComparisonBGCol = Options.GetColour(GridComparisonBackground);
	wxColour ComparisonBGSelCol = Options.GetColour(GridComparisonBackgroundSelected);
	wxColour ComparisonBGCmntCol = Options.GetColour(GridComparisonCommentBackground);
	wxColour ComparisonBGCmntSelCol = Options.GetColour(GridComparisonCommentBackgroundSelected);
	wxString chtag = Options.GetString(GridTagsSwapChar);
	wxColour visibleOnVideo = Options.GetColour(GridVisibleOnVideo);
	bool SpellCheckerOn = Options.GetBool(SpellcheckerOn);

	tdc.SetPen(*wxTRANSPARENT_PEN);
	tdc.SetBrush(wxBrush(linesCol));
	tdc.DrawRectangle(0, 0, w + scHor, h);

	int ilcol;
	posY = 0;

	bool isComment = false;
	bool unkstyle = false;
	bool shorttime = false;
	int states = 0;

	if (SpellErrors.size()<(size_t)size){
		SpellErrors.resize(size);
	}

	Dialogue *acdial = GetDialogue(MID(0, Edit->ebrow, size - 1));
	Dialogue *Dial=NULL;
	TabPanel *tab = (TabPanel*)GetParent();
	int VideoPos = tab->Video->Tell();

	int fw, fh, bfw, bfh;
	wxColour kol;
	visibleLines.clear();


	for (int i = scPos; i<scrows; i++){

		wxArrayString strings;
		bool comparison = false;
		bool isSelected = false;

		if (i == scPos){
			strings.Add("#");
			if (subsFormat<SRT){
				strings.Add(_("W."));
			}
			strings.Add(_("Start"));
			if (subsFormat != TMP){
				strings.Add(_("Koniec"));
			}
			if (subsFormat<SRT){
				strings.Add(_("Styl"));
				strings.Add(_("Aktor"));
				strings.Add(_("M.L."));
				strings.Add(_("M.P."));
				strings.Add(_("M.Pi."));
				strings.Add(_("Efekt"));
			}
			if (subsFormat != TMP){ strings.Add(_("ZNS")); }
			strings.Add(showOriginal ? _("Tekst oryginalny") : _("Tekst"));
			if (showOriginal){ strings.Add(_("Tekst t³umaczenia")); }
			kol = header;
		}
		else{
			Dial = GetDialogue(i - 1);

			strings.Add(wxString::Format("%i", i));

			isComment = Dial->IsComment;
			//gdy zrobisz inaczej niepewne to u¿yj ^ 4 by wywaliæ 4 ze state.
			states = Dial->State;
			if (subsFormat<SRT){
				strings.Add(wxString::Format("%i", Dial->Layer));
			}

			if (showFrames && tab->Video->VFF){
				VideoFfmpeg *VFF = tab->Video->VFF;
				wxString frame;
				frame << VFF->GetFramefromMS(Dial->Start.mstime);
				strings.Add(frame);
				if (subsFormat != TMP){
					frame = "";
					frame << VFF->GetFramefromMS(Dial->End.mstime) - 1;
					strings.Add(frame);
				}
			}
			else{
				strings.Add(Dial->Start.raw(subsFormat));
				if (subsFormat != TMP){ strings.Add(Dial->End.raw(subsFormat)); }
			}

			if (subsFormat<SRT){
				if (FindStyle(Dial->Style) == -1){ unkstyle = true; }
				else{ unkstyle = false; }
				strings.Add(Dial->Style);
				strings.Add(Dial->Actor);
				strings.Add(wxString::Format("%i", Dial->MarginL));
				strings.Add(wxString::Format("%i", Dial->MarginR));
				strings.Add(wxString::Format("%i", Dial->MarginV));
				strings.Add(Dial->Effect);
			}
			if (subsFormat != TMP && !(CNZ & visibleColumns)){
				int chtime;
				if (SpellErrors[i - 1].size()<1){
					chtime = CalcChars((hasTLMode && Dial->TextTl != "") ?
						Dial->TextTl : Dial->Text) / ((Dial->End.mstime - Dial->Start.mstime) / 1000.0f);
					if (chtime<0 || chtime>999){ chtime = 999; }
					SpellErrors[i - 1].push_back(chtime);

				}
				else{ chtime = SpellErrors[i - 1][0]; }
				strings.Add(wxString::Format("%i", chtime));
				shorttime = chtime>15;
			}
			else{
				if (subsFormat != TMP){ strings.Add(""); }
				if (SpellErrors[i - 1].size() == 0){ SpellErrors[i - 1].push_back(0); }
			}

			wxString txt = Dial->Text;
			wxString txttl = Dial->TextTl;
			if (hideOverrideTags){
				wxRegEx reg("\\{[^\\{]*\\}", wxRE_ADVANCED);
				reg.ReplaceAll(&txt, chtag);
				if (showOriginal){ reg.ReplaceAll(&txttl, chtag); }
			}
			if (txt.Len()>1000){ txt = txt.SubString(0, 1000) + "..."; }
			strings.Add((!showOriginal&&hasTLMode&&txttl != "") ? txttl : txt);
			if (showOriginal){ strings.Add(txttl); }

			if (SpellCheckerOn && (!hasTLMode && Dial->Text != "" || hasTLMode && Dial->TextTl != "")){
				if (SpellErrors[i - 1].size()<2){
					CheckText(strings[strings.size() - 1], SpellErrors[i - 1]);
				}
			}
			if (Selections.find(i - 1) != Selections.end()){
				isSelected = true;
			}
			comparison = (Comparison && Comparison->at(i - 1).size()>0);//visibleColumnsLines
			bool visibleColumnsLine = (Dial->Start.mstime <= VideoPos && Dial->End.mstime > VideoPos);
			kol = (comparison) ? ComparisonBGCol :
				(visibleColumnsLine) ? visibleOnVideo :
				subsBkCol;
			if (isComment){ kol = (comparison) ? ComparisonBGCmntCol : comm; }
			if (isSelected){
				if (isComment){ kol = (comparison) ? ComparisonBGCmntSelCol : selcom; }
				else{ kol = (comparison) ? ComparisonBGSelCol : seldial; }
			}
			if (visibleColumnsLine){ visibleLines.push_back(true); }
			else{ visibleLines.push_back(false); }
		}


		if (isFiltered){
			posX = 11;
			bool drawHiddenLinesRect = (i == 1 && hasHiddenLinesAtStart);
			if (Dial && (Dial->isVisible > 1 || drawHiddenLinesRect)){
				tdc.SetBrush(*wxTRANSPARENT_BRUSH);
				tdc.SetPen(textcol);
				int halfLine = posY + (GridHeight / 2);
				int startDrawPosY = posY + ((GridHeight - 10) / 2);
				if (drawHiddenLinesRect){ halfLine -= GridHeight + 1; startDrawPosY -= GridHeight + 1; }
				if ((drawHiddenLinesRect && Dial->isVisible == VISIBLE) || Dial->isVisible == VISIBLE_HIDDEN_BLOCK){
					tdc.DrawRectangle(1, startDrawPosY, 9, 9);
					tdc.DrawLine(3, halfLine-1, 8, halfLine-1);
					tdc.DrawLine(5, startDrawPosY + 2, 5, startDrawPosY + 7);
				}
				else if (drawHiddenLinesRect || Dial->isVisible == VISIBLE_START_BLOCK){
					tdc.DrawRectangle(1, startDrawPosY, 9, 9);
					tdc.DrawLine(3, halfLine-1, 8, halfLine-1);
					tdc.DrawLine(5, startDrawPosY + 10, 5, posY + GridHeight);
				}
				else if (Dial->isVisible == VISIBLE_BLOCK){
					tdc.DrawLine(5, posY-1, 5, posY + GridHeight);
				}
				else if (Dial->isVisible == VISIBLE_END_BLOCK){
					tdc.DrawLine(5, posY-1, 5, halfLine);
					tdc.DrawLine(5, halfLine, 11, halfLine);
				}
			}
		}
		else{
			posX = 0;
		}

		ilcol = strings.GetCount();


		wxRect cur;
		bool isCenter;
		wxColour label = (states == 0) ? labelBkColN : (states == 2) ? labelBkCol :
			(states == 1) ? labelBkColM : labelBkColD;
		for (int j = 0; j<ilcol; j++){
			if (showOriginal&&j == ilcol - 2){
				int podz = (w + scHor - posX) / 2;
				GridWidth[j] = podz;
				GridWidth[j + 1] = podz;
			}

			if (!showOriginal&&j == ilcol - 1){ GridWidth[j] = w + scHor - posX; }


			if (GridWidth[j] < 1){
				continue;
			}
			tdc.SetPen(*wxTRANSPARENT_PEN);

			tdc.SetBrush(wxBrush((j == 0 && i != scPos) ? label : kol));
			if (unkstyle && j == 4 || shorttime && (j == 10 || (j == 3 && subsFormat>ASS))){
				tdc.SetBrush(wxBrush(SpelcheckerCol));
			}

			tdc.DrawRectangle(posX, posY, GridWidth[j], GridHeight);

			if (i != scPos && j == ilcol - 1){

				if (SpellErrors[i - 1].size()>2){
					tdc.SetBrush(wxBrush(SpelcheckerCol));
					for (size_t k = 1; k < SpellErrors[i - 1].size(); k += 2){

						wxString err = strings[j].SubString(SpellErrors[i - 1][k], SpellErrors[i - 1][k + 1]);
						err.Trim();
						if (SpellErrors[i - 1][k]>0){
							wxString berr = strings[j].Mid(0, SpellErrors[i - 1][k]);
							tdc.GetTextExtent(berr, &bfw, &bfh, NULL, NULL, &font);
						}
						else{ bfw = 0; }

						tdc.GetTextExtent(err, &fw, &fh, NULL, NULL, &font);
						tdc.DrawRectangle(posX + bfw + 3, posY, fw, GridHeight);
					}
				}


				if (comparison){
					tdc.SetTextForeground(ComparisonCol);

					for (size_t k = 1; k < Comparison->at(i - 1).size(); k += 2){
						//if(Comparison->at(i-1)[k]==Comparison->at(i-1)[k+1]){continue;}
						wxString cmp = strings[j].SubString(Comparison->at(i - 1)[k], Comparison->at(i - 1)[k + 1]);

						if (cmp == ""){ continue; }
						if (cmp == " "){ cmp = "_"; }
						wxString bcmp;
						if (Comparison->at(i - 1)[k]>0){
							bcmp = strings[j].Mid(0, Comparison->at(i - 1)[k]);
							tdc.GetTextExtent(bcmp, &bfw, &bfh, NULL, NULL, &font);
						}
						else{ bfw = 0; }

						tdc.GetTextExtent(cmp, &fw, &fh, NULL, NULL, &font);
						if ((cmp.StartsWith("T") || cmp.StartsWith("Y") || cmp.StartsWith(L"£"))){ bfw++; }

						tdc.DrawText(cmp, posX + bfw + 2, posY);
						tdc.DrawText(cmp, posX + bfw + 4, posY);
						tdc.DrawText(cmp, posX + bfw + 2, posY + 2);
						tdc.DrawText(cmp, posX + bfw + 4, posY + 2);
					}

				}

			}


			bool collis = (i != scPos && i != Edit->ebrow + 1 &&
				(Dial->Start >= acdial->Start && Dial->Start < acdial->End ||
				Dial->End > acdial->Start && Dial->Start <= acdial->End));

			if (subsFormat<SRT){ isCenter = !(j == 4 || j == 5 || j == 9 || j == 11 || j == 12); }
			else if (subsFormat == TMP){ isCenter = !(j == 2); }
			else{ isCenter = !(j == 4); }

			tdc.SetTextForeground((i == scPos) ? headerText : (collis) ? collcol : textcol);
			if (j == ilcol - 1 && (strings[j].StartsWith("T") || strings[j].StartsWith("Y") || strings[j].StartsWith(L"£"))){ posX++; }
			cur = wxRect(posX + 3, posY, GridWidth[j] - 6, GridHeight);
			tdc.SetClippingRegion(cur);
			tdc.DrawLabel(strings[j], cur, isCenter ? wxALIGN_CENTER : (wxALIGN_CENTER_VERTICAL | wxALIGN_LEFT));
			tdc.DestroyClippingRegion();

			posX += GridWidth[j] + 1;

			
		}

		posY += GridHeight + 1;

	}

	posX = (isFiltered) ? 11 : 0;
	if (bg){
		tdc.SetPen(*wxTRANSPARENT_PEN);
		tdc.SetBrush(wxBrush(Options.GetColour(GridBackground)));
		tdc.DrawRectangle(posX, posY, w + scHor, h);
	}
	if (markedLine >= scPos && markedLine <= scrows){
		tdc.SetBrush(*wxTRANSPARENT_BRUSH);
		tdc.SetPen(wxPen(Options.GetColour(GridActiveLine), 3));
		tdc.DrawRectangle(posX + 1, ((markedLine - scPos + 1)*(GridHeight + 1)) - 1, (GridWidth[0] - 1), GridHeight + 2);
	}

	if (Edit->ebrow >= scPos&&Edit->ebrow <= scrows){
		tdc.SetBrush(*wxTRANSPARENT_BRUSH);
		tdc.SetPen(wxPen(Options.GetColour(GridActiveLine)));
		tdc.DrawRectangle(posX+scHor, ((Edit->ebrow - scPos + 1)*(GridHeight + 1)) - 1, w + scHor - posX, GridHeight + 2);
	}

	wxPaintDC dc(this);
	dc.Blit(0, 0, firstCol, h, &tdc, 0, 0);
	dc.Blit(firstCol, 0, w + scHor, h, &tdc, scHor + firstCol, 0);
}

void SubsGridWindow::RefreshColumns(int cell)
{
	AdjustWidths(cell);
	Refresh(false);
}

void SubsGridWindow::AdjustWidths(int cell)
{

	wxClientDC dc(this);
	dc.SetFont(font);

	int law = 0, startMax = 0, endMax = 0, stw = 0, edw = 0, syw = 0, acw = 0, efw = 0, fw = 0, fh = 0;
	bool shml = false, shmr = false, shmv = false;


	int maxx = GetCount();

	dc.GetTextExtent(wxString::Format("%i", maxx), &fw, &fh, NULL, NULL, &font);
	GridWidth[0] = fw + 10;
	Dialogue *ndial;
	for (int i = 0; i<maxx; i++){
		ndial = GetCheckedDialogue(i);
		if (START & cell){
			if (ndial->Start.mstime > startMax){ startMax = ndial->Start.mstime; }
		}
		if ((END & cell) && subsFormat != TMP){
			if (ndial->End.mstime > endMax){ endMax = ndial->End.mstime; }
		}


		if (subsFormat<SRT){
			if ((LAYER & cell) && ndial->Layer != 0){
				dc.GetTextExtent(wxString::Format("%i", ndial->Layer), &fw, &fh, NULL, NULL, &font);
				if (fw + 10>law){ law = fw + 10; }
			}
			if (STYLE & cell){
				dc.GetTextExtent(ndial->Style, &fw, &fh, NULL, NULL, &font);
				if (fw + 10>syw){ syw = fw + 10; }
			}
			if ((ACTOR & cell) && ndial->Actor != ""){
				dc.GetTextExtent(ndial->Actor, &fw, &fh, NULL, NULL, &font);
				if (fw + 10>acw){ acw = fw + 10; }
			}
			if ((EFFECT & cell) && ndial->Effect != ""){
				dc.GetTextExtent(ndial->Effect, &fw, &fh, NULL, NULL, &font);
				if (fw + 10>efw){ efw = fw + 10; }
			}
			if ((MARGINL & cell) && ndial->MarginL != 0){ shml = true; }
			if ((MARGINR & cell) && ndial->MarginR != 0){ shmr = true; }
			if ((MARGINV & cell) && ndial->MarginV != 0){ shmv = true; }
		}
	}

	if (START & cell){
		STime start(startMax);
		if (showFrames){
			VideoFfmpeg *VFF = ((TabPanel*)GetParent())->Video->VFF;
			start.orgframe = VFF->GetFramefromMS(start.mstime);
		}
		dc.GetTextExtent(start.raw(showFrames ? FRAME : subsFormat), &fw, &fh, NULL, NULL, &font);
		stw = fw + 10;
	}
	if (END & cell){
		STime end(endMax);
		if (showFrames){
			VideoFfmpeg *VFF = ((TabPanel*)GetParent())->Video->VFF;
			end.orgframe = VFF->GetFramefromMS(end.mstime);
		}
		dc.GetTextExtent(end.raw(showFrames ? FRAME : subsFormat), &fw, &fh, NULL, NULL, &font);
		edw = fw + 10;
	}

	if ((subsFormat<SRT) ? (LAYER & cell) : (START & cell)){
		wxString frst = (subsFormat<SRT) ? _("W.") : _("Start");
		dc.GetTextExtent(frst, &fw, &fh, NULL, NULL, &font);
		GridWidth[1] = (subsFormat<SRT) ? law : stw;
		if (fw + 10>GridWidth[1] && GridWidth[1] != 0){ GridWidth[1] = fw + 10; }
	}

	if ((subsFormat<SRT) ? (START & cell) : (END & cell)){
		wxString scnd = (subsFormat<SRT) ? _("Start") : _("Koniec");
		dc.GetTextExtent(scnd, &fw, &fh, NULL, NULL, &font);
		GridWidth[2] = (subsFormat<SRT) ? stw : edw;
		if (fw + 10>GridWidth[2]){ GridWidth[2] = fw + 10; };
	}
	if (subsFormat<SRT){
		if (END & cell){
			dc.GetTextExtent(_("Koniec"), &fw, &fh, NULL, NULL, &font);
			GridWidth[3] = edw;
			if (fw + 10>GridWidth[3]){ GridWidth[3] = fw + 10; };
		}

		if (STYLE & cell){
			dc.GetTextExtent(_("Styl"), &fw, &fh, NULL, NULL, &font);
			GridWidth[4] = syw;
			if (fw + 10>GridWidth[4]){ GridWidth[4] = fw + 10; }
		}

		if (ACTOR & cell){
			dc.GetTextExtent(_("Aktor"), &fw, &fh, NULL, NULL, &font);
			if (fw + 10>acw&&acw != 0){ acw = fw + 10; };
			GridWidth[5] = (acw == 0) ? 0 : acw;
		}

		if (224 & cell){
			dc.GetTextExtent(_("M.Pi."), &fw, &fh, NULL, NULL, &font);
			if (MARGINL & cell){ GridWidth[6] = (!shml) ? 0 : fw + 10; }
			if (MARGINR & cell){ GridWidth[7] = (!shmr) ? 0 : fw + 10; }
			if (MARGINV & cell){ GridWidth[8] = (!shmv) ? 0 : fw + 10; }
		}

		if (EFFECT & cell){
			dc.GetTextExtent(_("Efekt"), &fw, &fh, NULL, NULL, &font);
			if (fw + 10>efw&&efw != 0){ efw = fw + 10; };
			GridWidth[9] = (efw == 0) ? 0 : efw;
		}
	}

	if (CNZ & cell){
		dc.GetTextExtent(_("ZNS"), &fw, &fh, NULL, NULL, &font);
		GridWidth[(subsFormat<SRT) ? 10 : 3] = fw + 5;
	}

	if (subsFormat == TMP){ GridWidth[2] = 0; GridWidth[3] = 0; GridWidth[10] = 0; }
	if (subsFormat>ASS){ GridWidth[4] = 0; GridWidth[5] = 0; GridWidth[6] = 0; GridWidth[7] = 0; GridWidth[8] = 0; GridWidth[9] = 0; }
	if ((subsFormat<SRT) ? (LAYER & visibleColumns) : (START & visibleColumns)){ GridWidth[1] = 0; }
	if ((subsFormat<SRT) ? (START & visibleColumns) : (END & visibleColumns)){ GridWidth[2] = 0; }
	if ((subsFormat<SRT) ? (END & visibleColumns) : (CNZ & visibleColumns)){ GridWidth[3] = 0; }
	if (STYLE & visibleColumns){ GridWidth[4] = 0; }
	if (ACTOR & visibleColumns){ GridWidth[5] = 0; }
	if (MARGINL & visibleColumns){ GridWidth[6] = 0; }
	if (MARGINR & visibleColumns){ GridWidth[7] = 0; }
	if (MARGINV & visibleColumns){ GridWidth[8] = 0; }
	if (EFFECT & visibleColumns){ GridWidth[9] = 0; }
	if (CNZ & visibleColumns){ GridWidth[10] = 0; }
	first = false;

}


void SubsGridWindow::SetVideoLineTime(wxMouseEvent &evt)
{
	TabPanel *tab = (TabPanel*)GetParent();
	if (tab->Video->GetState() != None){
		if (tab->Video->GetState() != Paused){
			if (tab->Video->GetState() == Stopped){ tab->Video->Play(); }
			tab->Video->Pause();
		}
		short wh = (subsFormat<SRT) ? 2 : 1;
		int whh = 2;
		for (int i = 0; i <= wh; i++){ whh += GridWidth[i]; }
		whh -= scHor;
		if (isFiltered){ whh += 11; }
		bool isstart;
		int vczas;
		bool getEndTime = evt.GetX() >= whh && evt.GetX() < whh + GridWidth[wh + 1] && subsFormat != TMP;
		if (getEndTime){
			vczas = Edit->line->End.mstime; isstart = false;
		}
		else{
			vczas = Edit->line->Start.mstime; isstart = true;
		}
		if (evt.LeftDClick() && evt.ControlDown()){ vczas -= 1000; }
		tab->Video->Seek(MAX(0, vczas), isstart, true, false);
		if (Edit->ABox){ Edit->ABox->audioDisplay->Update(getEndTime); }
	}
}

void SubsGridWindow::OnMouseEvent(wxMouseEvent &event) {

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
	int curY = (event.GetY());
	int curX = (event.GetX());


	if (ismenushown){ ScreenToClient(&curX, &curY); }
	int row = curY / (GridHeight + 1) + scPos - 1;
	int hideColumnWidth = (isFiltered) ? 12 : 0;
	bool isNumerizeColumn = (curX >= hideColumnWidth && curX < GridWidth[0] + hideColumnWidth);

	if (left_up && !holding) {
		return;
	}

	// Get focus
	if (event.ButtonDown()) {
		SetFocus();
	}

	// Seeking video by click on numeration column
	if (click && isNumerizeColumn){
		TabPanel *tab = (TabPanel*)GetParent();
		if (tab->Video->GetState() != None && !(row < scPos || row >= GetCount())){
			if (tab->Video->GetState() != Paused){
				if (tab->Video->GetState() == Stopped){ tab->Video->Play(); }
				tab->Video->Pause();
			}
			int vtime = 0;
			bool isstart = true;
			if (shift && subsFormat != TMP){
				vtime = GetDialogue(row)->End.mstime; isstart = false;
			}
			else{
				vtime = GetDialogue(row)->Start.mstime; isstart = true;
			}
			if (ctrl){ vtime -= 1000; }
			tab->Video->Seek(MAX(0, vtime), isstart, true, false);
			if (Edit->ABox){ Edit->ABox->audioDisplay->Update(shift && subsFormat != TMP); }
		}
		return;
	}
	// Popup
	if (right && !ctrl) {
		if (isNumerizeColumn){
			markedLine = row;
			Refresh(false);

		}
		else{
			ContextMenu(event.GetPosition());
		}

		return;
	}

	// Mouse wheel
	if (event.GetWheelRotation() != 0) {
		int step = 3 * event.GetWheelRotation() / event.GetWheelDelta();
		ScrollTo(scPos - step);
		return;
	}



	if (left_up && holding) {
		holding = false;
		//Save swap lines after alt release 
		if (event.AltDown() && lastsel != -1 && file->IsNotSaved()){ SetModified(GRID_SWAP_LINES); }
		ReleaseMouse();
		if (oldX != -1){ return; }
	}


	// Click type
	if (hasHiddenLinesAtStart && row == -1 && click && curX < hideColumnWidth){
		Dialogue *dial = GetDialogue(0);
		if (dial->isVisible == VISIBLE_BLOCK || dial->isVisible == VISIBLE){
			SubsGridFiltering filter((SubsGrid*)this, Edit->ebrow);
			filter.FilterPartial(0, dial->isVisible == VISIBLE_BLOCK);
		}
		return;
	}
	else if (click && curX >= hideColumnWidth) {
		holding = true;
		if (!shift) lastRow = row;
		lastsel = row;
		oldX = (curY<GridHeight) ? curX : -1;
		CaptureMouse();
	}
	if (holding && oldX != -1){
		int diff = (oldX - curX);
		if ((scHor == 0 && diff<0) || diff == 0 || (scHor>1500 && diff>0)){ return; }
		scHor = scHor + diff;
		oldX = curX;
		if (scHor<0){ scHor = 0; }
		Refresh(false);
		return;
	}
	TabPanel *pan = (TabPanel*)GetParent();
	VideoCtrl *video = pan->Video;
	bool changeActive = Options.GetBool(GridChangeActiveOnSelection);
	int mvtal = video->vToolbar->videoSeekAfter->GetSelection();//
	int pas = video->vToolbar->videoPlayAfter->GetSelection();
	if (!(row < scPos || row >= GetCount())) {

		if (click && curX < hideColumnWidth){
			Dialogue *dial = GetDialogue(row);
			if (dial->isVisible == VISIBLE_HIDDEN_BLOCK || dial->isVisible == VISIBLE_START_BLOCK){
				SubsGridFiltering filter((SubsGrid*)this, Edit->ebrow);
				filter.FilterPartial(row, dial->isVisible == VISIBLE_START_BLOCK);
			}
			return;
		}

		if (holding && alt && lastsel != row)
		{
			if (lastsel != -1) {
				file->edited = true;
				MoveRows(row - lastsel);
			}
			lastsel = row;
			//return;
		}


		// Toggle selected
		if (left_up && ctrl && !shift && !alt) {
			if (Edit->ebrow != lastActiveLine || Edit->ebrow != row){
				SelectRow(row, true, !(Selections.find(row) != Selections.end()));
				return;
			}

		}


		// Normal click
		if (!shift && !alt) {


			//jakbym chcia³ znów daæ zmianê edytowanej linii z ctrl to muszê dorobiæ mu refresh
			if (click && (changeActive || !ctrl) || (dclick && ctrl)) {/*(click && !ctrl)*/
				lastActiveLine = Edit->ebrow;
				Edit->SetLine(row, true, true, true, !ctrl);
				if (hasTLMode){ Edit->SetActiveLineToDoubtful(); }
				if (changeActive){ Refresh(false); }
				if (!ctrl || dclick){
					SelectRow(row);
					extendRow = -1;
				}
			}

			//1-klikniêcie lewym
			//2-klikniêcie lewym i edycja na pauzie
			//3-klikniêcie lewym i edycja na pauzie i odtwarzaniu

			if (dclick || (click && lastActiveLine != row && mvtal < 4 && mvtal > 0) && pas < 2){
				SetVideoLineTime(event);
			}

			if (click || dclick || left_up)
				return;
		}

		if (middle){
			if (video->GetState() != None){//
				video->PlayLine(GetDialogue(row)->Start.mstime, video->GetPlayEndTime(GetDialogue(row)->End.mstime) /*- video->avtpf*/);
			}

		}
	}

	// Scroll to keep visibleColumns
	if (holding) {
		// Find direction
		int scdelta = (alt) ? 1 : 3;
		int minVis = scPos + 1;
		int maxVis = scPos + h / (GridHeight + 1) - 2;
		int delta = 0;
		if (row < minVis && row != 0) delta = -scdelta;
		if (row > maxVis) delta = scdelta;

		if (delta) {
			ScrollTo(scPos + delta);//row - (h / (GridHeight+1)) row

			// End the hold if this was a mousedown to avoid accidental
			// selection of extra lines
			if (click) {// && row!=GetCount()-1
				holding = false;
				left_up = true;
				ReleaseMouse();
			}
		}
	}

	// Block select
	if ((left_up && shift && !alt) || (holding && !ctrl && !alt && !shift && lastsel != row)) {
		if (lastRow != -1) {
			// Keyboard selection continues from where the mouse was last used
			extendRow = lastRow;

			// Set boundaries
			row = MID(0, row, GetCount() - 1);
			int i1 = row;
			int i2 = lastRow;
			if (i1 > i2) {
				int aux = i1;
				i1 = i2;
				i2 = aux;
			}

			// Toggle each
			bool notFirst = false;
			for (int i = i1; i <= i2; i++) {
				SelectRow(i, notFirst || ctrl, true, true);
				notFirst = true;
			}
			if (changeActive){
				lastActiveLine = Edit->ebrow;
				Edit->SetLine(row, true, true, false);
				if (hasTLMode){ Edit->SetActiveLineToDoubtful(); }
				//if(mvtal < 4 && mvtal > 0){
				//SetVideoLineTime(event);
				//}
			}
			lastsel = row;
			Refresh(false);
			if (Edit->Visual == CHANGEPOS/* || Edit->Visual==MOVEALL*/){
				video->SetVisual();
				video->Render();
			}
		}
	}


}

void SubsGridWindow::OnScroll(wxScrollWinEvent& event)
{
	int newPos = 0;
	if (event.GetEventType() == wxEVT_SCROLLWIN_LINEUP)
	{
		newPos = scPos - 1;
		if (newPos<0){ newPos = 0; return; }
	}
	else if (event.GetEventType() == wxEVT_SCROLLWIN_LINEDOWN)
	{
		newPos = scPos + 1;
		if (newPos >= GetCount()){ newPos = GetCount() - 1; return; }
	}
	else if (event.GetEventType() == wxEVT_SCROLLWIN_PAGEUP)
	{
		wxSize size = GetClientSize();
		newPos = scPos;
		newPos -= (size.y / GridHeight - 1);
		newPos = MAX(0, newPos);
	}
	else if (event.GetEventType() == wxEVT_SCROLLWIN_PAGEDOWN)
	{
		wxSize size = GetClientSize();
		newPos = scPos;
		newPos += (size.y / GridHeight - 1);
		newPos = MIN(newPos, GetCount() - 1);
	}
	else{
		newPos = event.GetPosition();
	}
	
	if (scPos != newPos) {
		scPos = newPos;
		Refresh(false);
	}
}

void SubsGridWindow::OnSize(wxSizeEvent& event)
{
	//wxSize size= GetClientSize();
	Refresh(false);
}

void SubsGridWindow::SelectRow(int row, bool addToSelected, bool select, bool norefresh)
{
	row = MID(0, row, GetCount() - 1);
	if (addToSelected){
		if (!select){ Selections.erase(Selections.find(row)); }
		else{ Selections.insert(row); }
		if (norefresh){ return; }
		int w = 0;
		int h = 0;
		GetClientSize(&w, &h);
		RefreshRect(wxRect(0, (row + 1 - scPos)*(GridHeight + 1), w, GridHeight + 1), false);
		//Refresh(false);

	}
	else{
		Selections.clear();
		Selections.insert(row);
		if (norefresh){ return; }
		Refresh(false);
	}
	//done:
	if (Edit->Visual == CHANGEPOS){
		Kai->GetTab()->Video->SetVisual();
		Kai->GetTab()->Video->Render();
	}
}

void SubsGridWindow::ScrollTo(int y, bool center){
	int w, h;
	GetClientSize(&w, &h);
	if (center){ y -= (h / (GridHeight + 1)) / 2; }
	int nextY = MID(0, y, GetCount() + 2 - h / (GridHeight + 1));

	if (scPos != nextY) {
		scPos = nextY;
		Refresh(false);
	}
}

void SubsGridWindow::OnKeyPress(wxKeyEvent &event) {
	// Get size
	int w, h;
	GetClientSize(&w, &h);

	// Get scan code
	int key = event.GetKeyCode();
	bool ctrl = event.m_controlDown;
	bool alt = event.m_altDown;
	bool shift = event.m_shiftDown;

	// The "menu" key, simulate a right-click
	if (key == WXK_WINDOWS_MENU) {
		wxPoint pos;
		pos.x = w / 2;
		pos.y = (Edit->ebrow + 1 - scPos) * GridHeight + GridHeight / 2;
		ContextMenu(pos);
		return;
	}

	// Select all
	if (key == 'A' && ctrl && !alt && !shift) {
		//SelectRow(0,false,true,true);
		for (int i = 0; i<GetCount(); i++){
			Selections.insert(i);
		}
		Refresh(false);
	}

	// Up/down
	int dir = 0;
	if (key == WXK_UP) dir = -1;
	if (key == WXK_DOWN) dir = 1;
	if (key == WXK_PAGEUP) {
		dir = -(h / GridHeight - 1);
	}
	if (key == WXK_PAGEDOWN) {
		dir = h / GridHeight - 1;
	}
	if (key == WXK_HOME) {
		dir = -GetCount();
	}
	if (key == WXK_END) {
		dir = GetCount();
	}
	if (key == WXK_RETURN){
		Edit->TextEdit->SetFocus();
	}

	// Moving
	if (dir) {
		// Move selection
		if (!ctrl && !shift && !alt) {
			// Move to extent first
			int curLine = Edit->ebrow;
			if (extendRow != -1) {
				curLine = extendRow;
				extendRow = -1;
			}

			int next = MID(0, curLine + dir, GetCount() - 1);
			Edit->SetLine(next);
			SelectRow(next);
			int gridh = ((h / (GridHeight + 1)) - 1);
			if (dir == 1 || dir == -1){
				bool above = (next <= scPos);
				bool below = (next >= scPos + gridh);
				if (above || below){ ScrollTo(above ? next - 1 : next - gridh + 1); }
			}
			else{
				ScrollTo(next);
			}
			lastRow = next;
			//return;
		}


		// Move selected
		else if (alt&&!shift) {
			if ((dir == 1 || dir == -1) && FirstSel() != -1){
				MoveRows(dir, true);
				ScrollTo(scPos + dir);
			}
			//return;
		}

		// Shift-selection
		else if (shift && !ctrl && !alt) {
			// Find end
			if (extendRow == -1) extendRow = Edit->ebrow;
			extendRow = lastRow = MID(0, extendRow + dir, GetCount() - 1);
			// Set range
			int i1 = Edit->ebrow;
			int i2 = extendRow;
			if (i2 < i1) {
				int aux = i1;
				i1 = i2;
				i2 = aux;
			}

			// Select range
			bool notfirst = false;
			for (int i = i1; i <= i2; i++) {
				SelectRow(i, notfirst);
				notfirst = true;
			}

			int gridh = ((h / (GridHeight + 1)) - 1);
			if (extendRow == scPos && (dir == 1 || dir == -1)){
				ScrollTo(extendRow - 1);
			}
			else if (extendRow == scPos + gridh && (dir == 1 || dir == -1)){
				ScrollTo(extendRow - gridh + 1);
			}
			else if (dir != 1 && dir != -1){
				ScrollTo(extendRow);
			}
			//return;
		}
		if (hasTLMode){ Edit->SetActiveLineToDoubtful(); }
	}

}

void SubsGridWindow::CheckText(wxString text, wxArrayInt &errs)
{

	wxString notchar = "/?<>|\\!@#$%^&*()_+=[]\t~ :;.,\"{} ";
	text += " ";
	bool block = false;
	wxString word = "";
	//wxString deb;
	bool slash = false;
	int lasti = 0;
	int firsti = 0;
	for (size_t i = 0; i<text.Len(); i++)
	{
		wxUniChar ch = text.GetChar(i);
		if (notchar.Find(ch) != -1 && !block){
			if (word.Len()>1){
				if (word.StartsWith("'")){ word = word.Remove(0, 1); }
				if (word.EndsWith("'")){ word = word.RemoveLast(1); }
				word.Trim(false);
				word.Trim(true);
				bool isgood = SpellChecker::Get()->CheckWord(word);
				if (!isgood){ errs.push_back(firsti); errs.push_back(lasti); }
			}word = ""; firsti = i + 1;
		}
		if (ch == '{'){ block = true; }
		else if (ch == '}'){ block = false; firsti = i + 1; word = ""; }


		if (notchar.Find(ch) == -1 && text.GetChar((i == 0) ? 0 : i - 1) != '\\' && !block){ word << ch; lasti = i; }
		else if (!block && text.GetChar((i == 0) ? 0 : i - 1) == '\\'){
			word = "";
			if (ch == 'N' || ch == 'n' || ch == 'h'){
				firsti = i + 1;
			}
			else{
				firsti = i;
				word << ch;
			}
		}
	}
	if (errs.size()<2){ errs.push_back(0); }

}

void SubsGridWindow::RefreshIfVisible(int time)
{
	int scrows = scPos + panelrows - 1;
	int count = GetCount();
	if (scrows >= count){ scrows = count; }
	if (scPos<0){ scPos = 0; }
	if ((int)visibleLines.size() < scrows - scPos){ return; }
	int counter = 0;
	for (int i = scPos; i < scrows; i++){
		Dialogue *dial = GetDialogue(i);
		bool isVisible = dial->Start.mstime <= time && dial->End.mstime > time;
		if (isVisible != visibleLines[counter++]){
			Refresh(false);
			break;
		}

	}

}

void SubsGridWindow::ChangeTimeDisplay(bool frame)
{
	TabPanel *tab = (TabPanel*)GetParent();
	VideoFfmpeg *VFF = tab->Video->VFF;
	if (frame && VFF){
		showFrames = true;
	}
	else{
		showFrames = false;
	}

}


void SubsGridWindow::HideOverrideTags()
{
	hideOverrideTags = !hideOverrideTags;
	Options.SetBool(GridHideTags, hideOverrideTags);
	SpellErrors.clear();
	Refresh(false);
}

int SubsGridWindow::CalcChars(const wxString &txt, wxString *lines, bool *bad)
{
	int len = txt.Len();
	bool block = false; bool slash = false;
	bool drawing = false;
	int chars = 0, lastchars = 0, ns = 0;
	wxUniChar brs = (subsFormat == SRT) ? '<' : '{';
	wxUniChar bre = (subsFormat == SRT) ? '>' : '}';
	for (int i = 0; i < len; i++)
	{
		if (txt[i] == brs){ block = true; }
		else if (txt[i] == bre){ block = false; }
		else if (block && txt[i] == 'p' && txt[i - 1] == '\\' && (i + 1 < len && wxString(txt[i + 1]).IsNumber())){
			if (txt[i + 1] == '0'){ drawing = false; }
			else{ drawing = true; }
		}
		else if (txt[i] == '\\' && !block && !drawing){ slash = true; continue; }
		else if (slash){
			if (txt[i] == 'N'){
				if (lines){
					ns++;
					int linechars = (chars - lastchars);
					if (!(*bad)){ *bad = (linechars > 43 || ns > 1); }
					(*lines) << linechars << "/";
					lastchars = chars;
				}
			}
			else if (txt[i] != 'h'){ chars += 2; }
			slash = false;
		}
		else if (!block && !drawing && txt[i] != ' '){ chars++; }

	}
	if (lines){
		int linechars = (chars - lastchars);
		if (!(*bad)){ *bad = (linechars > 43 || ns > 1); }
		(*lines) << linechars << "/";
	}


	return chars;
}

void SubsGridWindow::SelVideoLine(int curtime)
{
	if (Kai->GetTab()->Video->GetState() == None && curtime < 0){ return; }

	int time = (curtime < 0) ? Kai->GetTab()->Video->Tell() : curtime;
	int prevtime = 0;
	int durtime = (curtime < 0) ? Kai->GetTab()->Video->GetDuration() : 36000000;
	int idr = 0, ip = 0;
	//wxLogMessage("time %i, durtime %i",time,durtime);
	for (int i = 0; i < GetCount(); i++)
	{
		Dialogue *dial = GetDialogue(i);
		if (!dial->IsComment && (dial->Text != "" || dial->TextTl != "")){
			if (time >= dial->Start.mstime&&time <= dial->End.mstime)
			{
				Edit->SetLine(i); SelectRow(i); ScrollTo(i - 4);
				break;
			}
			if (dial->Start.mstime > prevtime && dial->Start.mstime < time){ prevtime = dial->Start.mstime; ip = i; }
			if (dial->Start.mstime < durtime && dial->Start.mstime > time){ durtime = dial->Start.mstime; idr = i; }

		}
		if (i == GetCount() - 1){
			if ((time - prevtime) > (durtime - time)){ Edit->SetLine(idr); SelectRow(idr); ScrollTo(idr - 4); }
			else{ Edit->SetLine(ip); SelectRow(ip); ScrollTo(ip - 4); }
		}
	}

}

Dialogue *SubsGridWindow::GetCheckedDialogue(int rw)
{
	Dialogue *dial = file->GetDialogue(rw);
	if (first){
		if (dial->Form != subsFormat){ dial->Conv(subsFormat); }
		if (dial->Start.mstime > dial->End.mstime){
			dial->End.mstime = dial->Start.mstime;
		}
	}
	return dial;
}

