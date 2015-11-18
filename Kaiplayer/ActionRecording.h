#ifndef ACTIONS_REC
#define ACTIONS_REC

#if Arec
#include <vector>
#include <wx/string.h>

class findreplace;
class stylestore;
class kainoteFrame;
class ScriptInfo;

class ActionsRec
	{
	public:
		ActionsRec(kainoteFrame *Kai);
		~ActionsRec();

		void RecordFindR(findreplace FindR, int act);
		void RecordStore(stylestore Store, int act);
		void RecordGrid();
		void RecordKai();
		void RecordSinfo(ScriptInfo Sinfo);
		void StartRecording();
		void StopRecording();
		void PauseRecording();

	private:
		std::vector <wxString> actions;
		int actnum;
		wxString Action;
		bool Recording;
		kainoteFrame *Kai;
	};

enum{
	RepAll=1,
	Sellines,
	AssLoad,
	AssDel,
	AssNew,
	AssCopy,
	AssSort,
	StoreAss,
	ChangeLine,
	Convert,
	ChangeTime,
	SortIt,
	DeleteRows,
	DeleteText,
	InsertRow,
	SwapRows,
	SetStartTime,
	SetEndTime,
	SelVideoLine,
	OnJoin,
	OnJoinF,
	OnDuplicate,
	OnInsertBefore,
	OnInsertAfter,
	OnCut,
	OnCopy,
	OnPaste,
	OnInsertBeforeVideo,
	OnInsertAfterVideo,
	OnMkvSubs,

	};
#endif

#endif