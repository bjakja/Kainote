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