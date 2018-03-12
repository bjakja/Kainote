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
#include "Utils.h"
#include "EditBox.h"

#include "kainoteMain.h"
#include "kaiMessageBox.h"
#include "SubsGridFiltering.h"
#include "SubsGridPreview.h"
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
	Bind(wxEVT_RIGHT_UP, &SubsGridWindow::OnMouseEvent, this);
}

SubsGridWindow::~SubsGridWindow()
{
	if (bmp){ delete bmp; bmp = NULL; }
	if (preview){ preview->DestroyPreview(); }
	if (thisPreview){ thisPreview->DestroyPreview(); thisPreview = NULL; }
}

void SubsGridWindow::SetStyle()
{
	const wxString & fontname = Options.GetString(GridFontName);
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
	wxPoint previewpos;
	wxSize previewsize;
	if (preview){
		previewpos = preview->GetPosition();
		previewsize = preview->GetSize();
		size += (previewsize.y / (GridHeight + 1)) + 1;
	}
	panelrows = (h / (GridHeight + 1)) + 1;
	if (scPos < 0){ scPos = 0; }
	int scrows = scPos + panelrows;
	//gdy widzimy koniec napisów
	if (scrows >= size + 3){
		bg = true;
		scrows = size + 1;
		scPos = (scrows - panelrows) + 2;// dojechanie do końca napisów
		if (panelrows > size + 3){ scPos = 0; }// w przypadku gdy całe napisy są widoczne, wtedy nie skrollujemy i pozycja =0
	}
	else if (scrows >= size + 2){
		bg = true;
		scrows--;//w przypadku gdy mamy linię przed końcem napisów musimy zaniżyć wynik bo przekroczy tablicę.
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
	const wxString & chtag = Options.GetString(GridTagsSwapChar);
	const wxColour &visibleOnVideo = Options.GetColour(GridVisibleOnVideo);
	bool SpellCheckerOn = Options.GetBool(SpellcheckerOn);

	tdc.SetPen(*wxTRANSPARENT_PEN);
	tdc.SetBrush(wxBrush(linesCol));
	tdc.DrawRectangle(0, 0, w + scHor, h);

	int ilcol;
	posY = 0;

	bool isComment = false;
	bool unkstyle = false;
	bool shorttime = false;
	bool startBlock = false;
	int states = 0;
	int startDrawPosYFromPlus = 0;

	if (SpellErrors.size()<(size_t)size){
		SpellErrors.resize(size);
	}

	Dialogue *acdial = (size>0)? GetDialogue(MID(0, Edit->ebrow, size - 1)) : NULL;
	Dialogue *Dial=NULL;
	TabPanel *tab = (TabPanel*)GetParent();
	int VideoPos = tab->Video->vstate !=None? tab->Video->Tell() : -1;

	int fw, fh, bfw, bfh;
	wxColour kol;
	visibleLines.clear();

	std::vector<wxString> strings;
	int i = file->GetElementById(scPos)-1;
	int k = scPos-1;

	while (i < file->GetAllCount() && k < scrows-1){
		bool isHeadline = (k < scPos);
		if (!isHeadline){
			Dial = file->GetDialogueByKey(i);
			if (!Dial->isVisible){ i++; continue; }
		}
		bool comparison = false;
		bool isSelected = false;
		strings.clear();
		
		if (isHeadline){
			strings.push_back("#");
			if (subsFormat<SRT){
				strings.push_back(_("W."));
			}
			strings.push_back(_("Start"));
			if (subsFormat != TMP){
				strings.push_back(_("Koniec"));
			}
			if (subsFormat<SRT){
				strings.push_back(_("Styl"));
				strings.push_back(_("Aktor"));
				strings.push_back(_("M.L."));
				strings.push_back(_("M.P."));
				strings.push_back(_("M.Pi."));
				strings.push_back(_("Efekt"));
			}
			if (subsFormat != TMP){ strings.push_back(_("ZNS")); }
			strings.push_back(showOriginal ? _("Tekst oryginalny") : _("Tekst"));
			if (showOriginal){ strings.push_back(_("Tekst tłumaczenia")); }
			kol = header;
		}
		else{
			
			strings.push_back(wxString::Format("%i", k+1));

			isComment = Dial->IsComment;
			//gdy zrobisz inaczej niepewne to użyj ^ 4 by wywalić 4 ze state.
			states = Dial->State;
			if (subsFormat<SRT){
				strings.push_back(wxString::Format("%i", Dial->Layer));
			}

			if (showFrames && tab->Video->VFF){
				VideoFfmpeg *VFF = tab->Video->VFF;
				wxString frame;
				frame << VFF->GetFramefromMS(Dial->Start.mstime);
				strings.push_back(frame);
				if (subsFormat != TMP){
					frame = "";
					frame << VFF->GetFramefromMS(Dial->End.mstime) - 1;
					strings.push_back(frame);
				}
			}
			else{
				strings.push_back(Dial->Start.raw(subsFormat));
				if (subsFormat != TMP){ strings.push_back(Dial->End.raw(subsFormat)); }
			}

			if (subsFormat<SRT){
				if (FindStyle(Dial->Style) == -1){ unkstyle = true; }
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

			if (!isComment && subsFormat != TMP && !(CNZ & visibleColumns)){
				int chtime;
				if (SpellErrors[k].size()<1){
					chtime = CalcChars((hasTLMode && txttl != "") ? txttl : txt) / 
						((Dial->End.mstime - Dial->Start.mstime) / 1000.0f);
					if (chtime<0 || chtime>999){ chtime = 999; }
					SpellErrors[k].push_back(chtime);

				}
				else{ chtime = SpellErrors[k][0]; }
				strings.push_back(wxString::Format("%i", chtime));
				shorttime = chtime>15;
			}
			else{
				if (subsFormat != TMP){ strings.push_back(""); }
				if (SpellErrors[k].size() == 0){ SpellErrors[k].push_back(0); }
				shorttime = false;
			}

			if (hideOverrideTags){
				wxRegEx reg("\\{[^\\{]*\\}", wxRE_ADVANCED);
				reg.ReplaceAll(&txt, chtag);
				if (showOriginal){ reg.ReplaceAll(&txttl, chtag); }
			}
			
			if (!isComment && SpellCheckerOn && (!hasTLMode && txt != "" || hasTLMode && txttl != "")){
				if (SpellErrors[k].size()<2){
					CheckText(txt, SpellErrors[k], chtag);
				}
			}
			if (txt.Len() > 1000){ txt = txt.SubString(0, 1000) + "..."; }
			strings.push_back((!showOriginal && hasTLMode && txttl != "") ? txttl : txt);
			if (showOriginal){ strings.push_back(txttl); }

			isSelected = file->IsSelectedByKey(i);
			comparison = (Comparison && Comparison->at(i).size()>0);
			bool comparisonMatch = (Comparison && !Comparison->at(i).differences);
			bool visibleLine = (Dial->Start.mstime <= VideoPos && Dial->End.mstime > VideoPos);
			kol = (comparison) ? ComparisonBG :
				(comparisonMatch) ? ComparisonBGMatch :
				(visibleLine) ? visibleOnVideo :
				subsBkCol;
			if (isComment){ kol = (comparison) ? ComparisonBGCmnt : (comparisonMatch) ? ComparisonBGCmntMatch : comm; }
			if (isSelected){
				kol = GetColorWithAlpha(seldial, kol);
			}
			visibleLines.push_back(visibleLine);
		}
		
		if (isFiltered){
			posX = 11;
			unsigned char hasHiddenBlock = file->CheckIfHasHiddenBlock(k);
			if (hasHiddenBlock){
				tdc.SetBrush(*wxTRANSPARENT_BRUSH);
				tdc.SetPen(textcol);
				int halfGridHeight = (GridHeight / 2);
				int newPosY = posY + GridHeight + 1;
				int startDrawPosY = newPosY + ((GridHeight - 10) / 2) - halfGridHeight;
				tdc.DrawRectangle(1, startDrawPosY, 9, 9);
				tdc.DrawLine(3, newPosY - 1, 8, newPosY - 1);
				if (hasHiddenBlock == 1){ 
					tdc.DrawLine(5, startDrawPosY + 2, 5, startDrawPosY + 7); 
				}
				//tdc.SetPen(SpelcheckerCol);
				tdc.DrawLine(10, newPosY - 1, w+scHor, newPosY - 1);
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
					int halfLine = posY-1;
					if (isLastLine && !notVisibleBlock){ halfLine = posY + GridHeight; }
					tdc.DrawLine(5, startDrawPosYFromPlus, 5, halfLine);
					tdc.DrawLine(5, halfLine, w + scHor, halfLine);
					startBlock = false;
				}
			}
			
			
		}
		else{
			posX = 0;
		}

		ilcol = strings.size();


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

			tdc.SetBrush(wxBrush((j == 0 && !isHeadline) ? label : kol));
			if (unkstyle && j == 4 || shorttime && (j == 10 || (j == 3 && subsFormat>ASS))){
				tdc.SetBrush(wxBrush(SpelcheckerCol));
			}

			tdc.DrawRectangle(posX, posY, GridWidth[j], GridHeight);

			if (!isHeadline && j == ilcol - 1){
				if (SpellErrors[k].size()>2){
					wxString & text = strings[j];
					text.Replace("\t", " ");
					tdc.SetBrush(wxBrush(SpelcheckerCol));
					for (size_t s = 1; s < SpellErrors[k].size(); s += 2){
						wxString err = text.SubString(SpellErrors[k][s], SpellErrors[k][s + 1]);
						err.Trim();
						if (SpellErrors[k][s]>0){
							
							wxString berr = text.Mid(0, SpellErrors[k][s]);
							GetTextExtent(berr, &bfw, &bfh, NULL, NULL, &font);
						}
						else{ bfw = 0; }

						GetTextExtent(err, &fw, &fh, NULL, NULL, &font);
						tdc.DrawRectangle(posX + bfw + 3, posY, fw, GridHeight);
					}
				}


				if (comparison){
					tdc.SetTextForeground(ComparisonCol);
					const wxString & text = strings[j];
					for (size_t c = 1; c < Comparison->at(i).size(); c += 2){
						//if(Comparison->at(i-1)[k]==Comparison->at(i-1)[k+1]){continue;}
						wxString cmp = text.SubString(Comparison->at(i)[c], Comparison->at(i)[c + 1]);

						if (cmp == ""){ continue; }
						if (cmp == " "){ cmp = "_"; }
						wxString bcmp;
						if (Comparison->at(i)[c]>0){
							bcmp = text.Mid(0, Comparison->at(i)[c]);
							GetTextExtent(bcmp, &bfw, &bfh, NULL, NULL, &font);
						}
						else{ bfw = 0; }

						GetTextExtent(cmp, &fw, &fh, NULL, NULL, &font);
						
						tdc.DrawText(cmp, posX + bfw + 2, posY);
						tdc.DrawText(cmp, posX + bfw + 4, posY);
						tdc.DrawText(cmp, posX + bfw + 2, posY + 2);
						tdc.DrawText(cmp, posX + bfw + 4, posY + 2);
					}

				}

			}


			bool collis = (!isHeadline && k != Edit->ebrow &&
				(Dial->Start >= acdial->Start && Dial->Start < acdial->End ||
				Dial->End > acdial->Start && Dial->Start <= acdial->End));

			if (subsFormat<SRT){ isCenter = !(j == 4 || j == 5 || j == 9 || j == 11 || j == 12); }
			else if (subsFormat == TMP){ isCenter = !(j == 2); }
			else{ isCenter = !(j == 4); }

			tdc.SetTextForeground((isHeadline) ? headerText : (collis) ? collcol : textcol);

			cur = wxRect(posX + 3, posY, GridWidth[j] - 6, GridHeight);
			tdc.SetClippingRegion(cur);
			tdc.DrawLabel(strings[j], cur, isCenter ? wxALIGN_CENTER : (wxALIGN_CENTER_VERTICAL | wxALIGN_LEFT));
			tdc.DestroyClippingRegion();

			posX += GridWidth[j] + 1;

			
		}

		posY += GridHeight + 1;
		if (preview){
			if (posY >= previewpos.y-2 && posY < previewpos.y + previewsize.y){
				posY = previewpos.y + previewsize.y + 3;
			}
			else if (posY > h){ scrows = k + 1; break; }
		}
		k++;
		i++;
	}

	posX = (isFiltered) ? 11 : 0;
	if (bg){
		tdc.SetPen(*wxTRANSPARENT_PEN);
		tdc.SetBrush(wxBrush(Options.GetColour(GridBackground)));
		tdc.DrawRectangle(posX, posY, w + scHor, h);
	}
	if (size > 0){
		if (markedLine >= scPos && markedLine <= scrows){
			tdc.SetBrush(*wxTRANSPARENT_BRUSH);
			tdc.SetPen(wxPen(Options.GetColour(GridActiveLine), 3));
			int ypos = ((markedLine - scPos + 1)*(GridHeight + 1));
			if (preview && ypos >= previewpos.y-2){ ypos += previewsize.y + 5; }
			tdc.DrawRectangle(posX + 1, ypos - 1, (GridWidth[0] - 1), GridHeight + 2);
		}

		if (Edit->ebrow >= scPos && Edit->ebrow <= scrows){
			tdc.SetBrush(*wxTRANSPARENT_BRUSH);
			tdc.SetPen(wxPen(Options.GetColour(GridActiveLine)));
			int ypos = ((Edit->ebrow - scPos + 1)*(GridHeight + 1));
			if (preview && ypos >= previewpos.y-2){ ypos += previewsize.y + 5; }
			tdc.DrawRectangle(posX, ypos - 1, w + scHor - posX, GridHeight + 2);
		}
	}
	wxPaintDC dc(this);
	dc.Blit(0, 0, firstCol + posX, h, &tdc, 0, 0);
	dc.Blit(firstCol + posX, 0, w + scHor, h, &tdc, scHor + firstCol + posX, 0);
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

	File *Subs = file->GetSubs();
	int maxx = Subs->dials.size();

	dc.GetTextExtent(wxString::Format("%i", maxx), &fw, &fh, NULL, NULL, &font);
	GridWidth[0] = fw + 10;
	Dialogue *dial;
	for (int i = 0; i<maxx; i++){
		dial = Subs->dials[i];
		if (!dial->isVisible){ continue; }
		if (first){
			if (dial->Form != subsFormat){ dial->Convert(subsFormat); }
			if (dial->Start.mstime > dial->End.mstime){
				dial->End.mstime = dial->Start.mstime;
			}
		}
		if (START & cell){
			if (dial->Start.mstime > startMax){ startMax = dial->Start.mstime; }
		}
		if ((END & cell) && subsFormat != TMP){
			if (dial->End.mstime > endMax){ endMax = dial->End.mstime; }
		}


		if (subsFormat<SRT){
			if ((LAYER & cell) && dial->Layer != 0){
				dc.GetTextExtent(wxString::Format("%i", dial->Layer), &fw, &fh, NULL, NULL, &font);
				if (fw + 10>law){ law = fw + 10; }
			}
			if (STYLE & cell){
				dc.GetTextExtent(dial->Style, &fw, &fh, NULL, NULL, &font);
				if (fw + 10>syw){ syw = fw + 10; }
			}
			if ((ACTOR & cell) && dial->Actor != ""){
				dc.GetTextExtent(dial->Actor, &fw, &fh, NULL, NULL, &font);
				if (fw + 10>acw){ acw = fw + 10; }
			}
			if ((EFFECT & cell) && dial->Effect != ""){
				dc.GetTextExtent(dial->Effect, &fw, &fh, NULL, NULL, &font);
				if (fw + 10>efw){ efw = fw + 10; }
			}
			if ((MARGINL & cell) && dial->MarginL != 0){ shml = true; }
			if ((MARGINR & cell) && dial->MarginR != 0){ shmr = true; }
			if ((MARGINV & cell) && dial->MarginV != 0){ shmv = true; }
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


void SubsGridWindow::SetVideoLineTime(wxMouseEvent &evt, int mvtal)
{
	TabPanel *tab = (TabPanel*)GetParent();
	if (tab->Video->GetState() != None){
		if (tab->Video->GetState() != Paused){
			if (tab->Video->GetState() == Stopped){ tab->Video->Play(); tab->Video->Pause(); }
			else if (mvtal)
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
	int curY = event.GetY();
	if (preview){
		wxPoint previewpos = preview->GetPosition();
		wxSize previewsize = preview->GetSize();
		if (curY >= previewpos.y - 2 && curY <= previewpos.y + previewsize.y + 1){ return; }
		if (curY > previewpos.y + previewsize.y){
			curY -= previewsize.y + 3;
		}
	}
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
	if ((click || dclick) && isNumerizeColumn){
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
		return;
	}
	else if (event.RightUp() && !isNumerizeColumn){
		ContextMenu(event.GetPosition());
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


	
	if (curX < hideColumnWidth){
		int filterRow = (curY + (GridHeight / 2)) / (GridHeight + 1) + scPos - 2;
		if (!(filterRow < scPos || filterRow >= GetCount()) || filterRow == -1) {
			if ((click || dclick) && file->CheckIfHasHiddenBlock(filterRow)){
				SubsGridFiltering filter((SubsGrid*)this, Edit->ebrow);
				filter.FilterPartial(filterRow);
			}
		}
		return;
	}
	// Click type
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
	int mvtal = video->vToolbar->videoSeekAfter->GetSelection();
	int pas = video->vToolbar->videoPlayAfter->GetSelection();
	if (!(row < scPos || row >= GetCount())) {

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
			if (!(Edit->ebrow == row && file->SelectionsSize() == 1 && file->IsSelected(row))){
				SelectRow(row, true, !file->IsSelected(row));
				if (file->SelectionsSize() < 1){ SelectRow(Edit->ebrow); }
				return;
			}

		}


		// Normal click
		if (!shift && !alt) {


			//jakbym chciał znów dać zmianę edytowanej linii z ctrl to muszę dorobić mu refresh
			if (click && (changeActive || !ctrl) || (dclick && ctrl)) {
				lastActiveLine = Edit->ebrow;
				Edit->SetLine(row, true, true, true, !ctrl);
				if (hasTLMode){ Edit->SetActiveLineToDoubtful(); }
				if (changeActive){ Refresh(false); }
				if (!ctrl || dclick){
					SelectRow(row);
					extendRow = -1;
				}
				if (Comparison){ ShowSecondComparedLine(row); }
				else if (preview){ preview->NewSeeking(); }
			}

			//1-kliknięcie lewym
			//2-kliknięcie lewym i edycja na pauzie
			//3-kliknięcie lewym i edycja na pauzie i odtwarzaniu

			if (dclick || (click && lastActiveLine != row && mvtal < 4 && mvtal > 0) && pas < 2){
				SetVideoLineTime(event, mvtal);
			}

			if (click || dclick || left_up)
				return;
		}

		if (middle){
			if (video->GetState() != None){
				video->PlayLine(GetDialogue(row)->Start.mstime, video->GetPlayEndTime(GetDialogue(row)->End.mstime));
			}

		}
	}

	// Scroll to keep visibleColumns
	if (holding) {
		// Find direction
		int scdelta = (alt) ? 1 : 3;
		int minVis = scPos + 1;
		int maxVis = scPos + h / (GridHeight + 1) - 2;
		if (preview){
			wxPoint previewpos = preview->GetPosition();
			wxSize previewsize = preview->GetSize();
			maxVis -= (previewsize.y / (GridHeight + 1)) + 1;
		}
		int delta = 0;
		if (row < minVis && row != 0) delta = -scdelta;
		if (row > maxVis) delta = scdelta;

		if (delta) {
			ScrollTo(scPos + delta);

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
			file->InsertSelections(i1, i2, !ctrl);
			if (changeActive){
				lastActiveLine = Edit->ebrow;
				Edit->SetLine(row, true, true, false);
				if (hasTLMode){ Edit->SetActiveLineToDoubtful(); }
			}
			lastsel = row;
			Refresh(false);
			if (Edit->Visual == CHANGEPOS){
				video->SetVisual();
				video->Render();
			}
		}
	}


}

void SubsGridWindow::OnScroll(wxScrollWinEvent& event)
{
	int newPos = event.GetPosition();
	if (scPos != newPos) {
		scPos = newPos;
		Refresh(false);
	}
}

void SubsGridWindow::OnSize(wxSizeEvent& event)
{
	if (preview){
		wxSize size = GetClientSize();
		preview->SetSize(wxSize(size.x, -1));
	}
	Refresh(false);
}

void SubsGridWindow::SelectRow(int row, bool addToSelected, bool select, bool norefresh)
{
	row = MID(0, row, GetCount() - 1);
	int rowKey = file->GetElementById(row);
	if (addToSelected){
		if (!select){ file->EraseSelectionKey(rowKey); }
		else{ file->InsertSelectionKey(rowKey); }
		if (norefresh){ return; }
		int w = 0;
		int h = 0;
		GetClientSize(&w, &h);
		RefreshRect(wxRect(0, (row + 1 - scPos)*(GridHeight + 1), w, GridHeight + 1), false);
		//Refresh(false);

	}
	else{
		file->ClearSelections();
		file->InsertSelectionKey(rowKey);
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
	int size = GetCount() + 2;
	if (preview){
		wxPoint previewpos = preview->GetPosition();
		wxSize previewsize = preview->GetSize();
		size += (previewsize.y / (GridHeight + 1)) + 1;
	}
	int nextY = MID(0, y, size - h / (GridHeight + 1));

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
		file->InsertKeySelections(0, -1);
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
			if (preview){
				wxPoint previewpos = preview->GetPosition();
				wxSize previewsize = preview->GetSize();
				gridh -= (previewsize.y / (GridHeight + 1)) + 1;
			}
			if (dir == 1 || dir == -1){
				bool above = (next <= scPos);
				bool below = (next >= scPos + gridh);
				if (above || below){ ScrollTo(above ? next - 1 : next - gridh + 1); }
			}
			else{
				ScrollTo(next);
			}
			if (Comparison){ ShowSecondComparedLine(next); }
			else if (preview){ preview->NewSeeking(); }
			lastRow = next;
			//return;
		}


		// Move selected
		else if (alt&&!shift) {
			if ((dir == 1 || dir == -1) && FirstSelection() != -1){
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

			
			file->InsertSelections(i1, i2, true);

			int gridh = ((h / (GridHeight + 1)) - 1);
			if (preview){
				wxPoint previewpos = preview->GetPosition();
				wxSize previewsize = preview->GetSize();
				gridh -= (previewsize.y / (GridHeight + 1)) + 1;
			}
			if (extendRow == scPos && (dir == 1 || dir == -1)){
				ScrollTo(extendRow - 1);
			}
			else if (extendRow == scPos + gridh && (dir == 1 || dir == -1)){
				ScrollTo(extendRow - gridh + 1);
			}
			else if (dir != 1 && dir != -1){
				ScrollTo(extendRow);
			}
			else{ Refresh(false); }
			//return;
		}
		if (hasTLMode){ Edit->SetActiveLineToDoubtful(); }
	}

}

void SubsGridWindow::CheckText(wxString text, wxArrayInt &errs, const wxString &tagsReplacement)
{

	//wxString notchar = "/?<>|\\!@#$%^&*()_+=[]\t~ :;.,\"{} ";
	bool repltags = hideOverrideTags && tagsReplacement.Len() > 0;
	text += " ";
	bool block = false;
	wxString word;
	//bool bracket = false;
	int lasti = 0;
	int firsti = 0;
	int lastStartBracket = -1;
	int lastEndBracket = -1;
	int lastStartTBracket = -1;
	int lastStartCBracket = -1;
	int lastEndCBracket = -1;

	for (size_t i = 0; i<text.Len(); i++)
	{
		const wxUniChar &ch = text[i];
		if (iswctype(WXWCHAR_T_CAST(ch), _SPACE | _DIGIT | _PUNCT)/*notchar.Find(ch) != -1*/ && !block){
			if (word.Len()>1){
				if (word.StartsWith("'")){ word = word.Remove(0, 1); }
				if (word.EndsWith("'")){ word = word.RemoveLast(1); }
				word.Trim(false);
				word.Trim(true);
				bool isgood = SpellChecker::Get()->CheckWord(word);
				if (!isgood){ errs.push_back(firsti); errs.push_back(lasti); }
			}
			word = ""; firsti = i + 1;
		}
		if (block){
			if (ch == '{'){ errs.push_back(lastStartCBracket); errs.push_back(lastStartCBracket); }
			if (ch == '\\' && text[(i == 0) ? 0 : i - 1] == '\\'){ errs.push_back(i); errs.push_back(i); }
			if (ch == '('){
				if (i > 1 && text[i - 2] == '\\' && text[i - 1]){ lastStartTBracket = i; continue; }
				if (lastStartBracket > lastEndBracket){
					errs.push_back(lastStartBracket); errs.push_back(lastStartBracket);
				}
				lastStartBracket = i;
			}
			if (ch == ')'){
				if ((lastStartBracket < lastEndBracket || lastStartBracket < 0)){
					if (lastStartTBracket > 0 && (lastStartTBracket < lastEndBracket || lastStartBracket <lastStartTBracket)){
						lastStartTBracket = -1; continue;
					}
					errs.push_back(i); errs.push_back(i);
				}
				
				lastEndBracket = i;
			}
		}
		if (!block && ch == '}'){
			errs.push_back(i); errs.push_back(i);
		}
		if (lastStartTBracket >= 0 && ch == '{' || ch == '}'){
			errs.push_back(lastStartTBracket); errs.push_back(lastStartTBracket);
			lastStartTBracket = -1;
		}
		if (ch == '{'){ block = true; lastStartCBracket = i; continue; }
		else if (ch == '}'){ block = false; lastEndCBracket = i; firsti = i + 1; word = ""; continue; }
		else if (repltags && tagsReplacement[0] == ch && text.Mid(i, tagsReplacement.Len()) == tagsReplacement){
			firsti = i + tagsReplacement.Len(); word = ""; continue;
		}
		
		if (!block && !iswctype(WXWCHAR_T_CAST(ch), _SPACE | _DIGIT | _PUNCT) /*notchar.Find(ch) == -1*/ && text.GetChar((i == 0) ? 0 : i - 1) != '\\'){ word << ch; lasti = i; }
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
	if (lastStartCBracket > lastEndCBracket){ errs.push_back(lastStartCBracket); errs.push_back(lastStartCBracket); }
	if (lastStartBracket > lastEndBracket){ errs.push_back(lastStartBracket); errs.push_back(lastStartBracket); }
	if (lastStartTBracket >= 0){ errs.push_back(lastStartTBracket); errs.push_back(lastStartTBracket); }
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
		else if (block && txt[i] == 'p' && txt[i - 1] == '\\' && (i + 1 < len && wxIsdigit(txt[i + 1]))){
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

void SubsGridWindow::ChangeActiveLine(int newActiveLine, bool refresh /*= false*/)
{
	Edit->SetLine(newActiveLine);
	SelectRow(newActiveLine, false, true, true);
	if (refresh)
		Refresh(false);
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

void SubsGridWindow::ShowSecondComparedLine(int Line, bool showPreview, bool fromPreview)
{
	SubsGrid *thisgrid = (SubsGrid*)this;
	SubsGrid *secondgrid = NULL;
	if (thisgrid == CG1)
		secondgrid = CG2;
	else if (thisgrid == CG2)
		secondgrid = CG1;
	else
		return;

	bool hiddenSecondGrid = !secondgrid->IsShownOnScreen();
	if (!(showPreview || preview) && hiddenSecondGrid){ return; }
	//Line is id here we need convert it to key
	compareData & data = Comparison->at(file->GetElementById(Line));
	int secondGridLine = data.secondComparedLine;
	if (secondGridLine < 0){ return; }
	int diffPosition = Line - scPos;
	secondgrid->scPos = secondGridLine - diffPosition;
	secondgrid->ChangeActiveLine(secondGridLine);
	if (!fromPreview && hiddenSecondGrid){
		if (!preview){
			ShowPreviewWindow(secondgrid, thisgrid, Line, diffPosition);
		}
		else{
			preview->MakeVisible();
			preview->Refresh(false);
		}
	}
	else{
		secondgrid->Refresh(false);
	}
}

bool SubsGridWindow::ShowPreviewWindow(SubsGrid *previewGrid, SubsGrid *windowToDraw, int activeLine, int diffPosition)
{
	int w, h;
	GetClientSize(&w, &h);
	int realGridHeight = (GridHeight + 1);
	int previewHeight = (((h / 3) / realGridHeight) * realGridHeight) + realGridHeight + 4;
	if (previewHeight < 100)
		previewHeight = ((100 / realGridHeight) * realGridHeight) + realGridHeight + 4;
	if (h < 150){ KaiMessageBox(_("Nie można wyświetlić podglądu, ponieważ wielkość okna napisów jest zbyt mała")); return false; }
	int previewPosition = (diffPosition + 2) * realGridHeight;
	if (previewPosition + previewHeight > h || previewPosition < 20){
		int newLine = (((h - previewHeight) / 2) / realGridHeight);
		scPos = (activeLine - newLine) + 2;
		previewPosition = newLine * realGridHeight;
	}
	preview = new SubsGridPreview(previewGrid, windowToDraw, previewPosition + 2, wxSize(w, previewHeight));
	Refresh(false);
	return true;
}