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

#ifndef KARASPLITTING
#define KARASPLITTING

#include <wx/wx.h>
#include <vector>
#include "SubsDialogue.h"

#ifndef ZEROIT
#define ZEROIT(a) (((a+5)/10)*10)
#endif

class AudioDisplay;

class Karaoke
	{
	friend class AudioDisplay;
	public:
		Karaoke(AudioDisplay *_AD);
		~Karaoke();

		void Split();
		wxString GetText();
		bool CheckIfOver(int x, int *result);
		bool GetSylAtX(int x, int *result);
		bool GetLetterAtX(int x, int *syl, int *letter);
		void GetSylTimes(int i, int &start, int &end);
		void Join(int line);
		void ChangeSplit(int line, int nletters);
		void SplitSyl(int line, int nletters);
		void Clearing();

	private:
		wxArrayString syls;
		wxArrayString kaas;
		wxArrayInt syltimes;
		//std::vector<bool> modifs;
		AudioDisplay *AD;
	};



#endif