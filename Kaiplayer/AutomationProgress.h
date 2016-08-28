#ifndef AUTOPROGRESS
#define AUTOPROGRESS

#include <wx/wx.h>

#include "AutomationDialog.h"
#include <utility>

namespace Auto{
	wxDECLARE_EVENT(EVT_MESSAGE, wxThreadEvent);
	class LuaProgressDialog :public wxDialog
	{
	public:
		LuaProgressDialog(wxWindow *parent,lua_State *L);
		virtual ~LuaProgressDialog();
		wxBoxSizer *sizer;
		wxGauge *progress_display;
		wxStaticText *title_display;
		wxStaticText *task_display;
		wxTextCtrl *debug_output;
		wxButton *cancel_button;
		wxMutex data_mutex;
		//float progress;
		//wxString task;
		//wxString title;
		//wxString log;
		wxString pending_debug_output;
		volatile bool cancelled;
		volatile bool finished;
		//volatile bool data_updated;
		volatile bool closedialog;
	private:
		wxTimer update_timer;
		void SetTitle(wxThreadEvent &evt);
		void SetTask(wxThreadEvent &evt);
		void SetProgress(wxThreadEvent &evt);
		void AddDebugOutput(wxThreadEvent &evt);
		void ShowConfigDialog(wxThreadEvent &evt);
		void ShowProgressDialog(wxThreadEvent &evt);
		void OnUpdate(wxTimerEvent &event);
		void OnCancel(wxCommandEvent &evt);
		lua_State *L;
	};

class LuaProgressSink : public wxEvtHandler{
	private:
		lua_State *L;

		static int LuaSetProgress(lua_State *L);
		static int LuaSetTask(lua_State *L);
		static int LuaSetTitle(lua_State *L);
		static int LuaGetCancelled(lua_State *L);
		
		static int LuaDisplayDialog(lua_State *L);
		static int LuaDisplayOpenDialog(lua_State *L);
		static int LuaDisplaySaveDialog(lua_State *L);
		
		int trace_level;
		wxWindow *_parent;
		

	public:
		LuaProgressSink(lua_State *_L, wxWindow *parent);
		virtual ~LuaProgressSink();
		void ShowDialog(wxString Title);

		static LuaProgressSink* ps;

		
		template<class T>
		void SafeQueue(wxEventType type, T const& value);
		static int LuaDebugOut(lua_State *L);
		LuaProgressDialog *lpd;
		void Destroy();
		
	};

	

};

#endif