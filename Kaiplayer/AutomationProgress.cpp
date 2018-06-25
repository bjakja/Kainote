// Copyright (c) 2006, 2007, Niels Martin Hansen
// Copyright (c) 2016, Marcin Drob
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of the Aegisub Group nor the names of its contributors
//     may be used to endorse or promote products derived from this software
//     without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// Aegisub Project http://www.aegisub.org/

#include "AutomationProgress.h"
#include "AutomationDialog.h"
#include "AutomationUtils.h"
#include "Config.h"


namespace Auto{

	wxDEFINE_EVENT(EVT_TITLE, wxThreadEvent);
	wxDEFINE_EVENT(EVT_TASK, wxThreadEvent);
	wxDEFINE_EVENT(EVT_PROGRESS, wxThreadEvent);
	wxDEFINE_EVENT(EVT_MESSAGE, wxThreadEvent);
	wxDEFINE_EVENT(EVT_SHOW_PRGS_DIAL, wxThreadEvent);
	wxDEFINE_EVENT(EVT_SHOW_CFG_DIAL, wxThreadEvent);
	LuaProgressSink *LuaProgressSink::ps=NULL;

	LuaProgressSink::LuaProgressSink(lua_State *_L, wxWindow *parent)
		: wxEvtHandler()
		,L(_L)
		,lpd(NULL)
	{
		ps = this;
		_parent=parent;
		// Init trace level
		trace_level = Options.GetInt(AutomationTraceLevel);

		//LuaProgressSink **ud = (LuaProgressSink**)lua_newuserdata(L, sizeof(LuaProgressSink*));


		// register progress reporting stuff
		lua_getglobal(L, "aegisub");
		lua_newtable(L);

		lua_pushvalue(L, -3);
		lua_pushcclosure(L, LuaSetProgress, 1);
		lua_setfield(L, -2, "set");

		lua_pushvalue(L, -3);
		lua_pushcclosure(L, LuaSetTask, 1);
		lua_setfield(L, -2, "task");

		lua_pushvalue(L, -3);
		lua_pushcclosure(L, LuaSetTitle, 1);
		lua_setfield(L, -2, "title");

		lua_pushvalue(L, -3);
		lua_pushcclosure(L, LuaGetCancelled, 1);
		lua_setfield(L, -2, "is_cancelled");

		lua_setfield(L, -2, "progress");

		lua_newtable(L);
		lua_pushvalue(L, -3);
		lua_pushcclosure(L, LuaDebugOut, 1);
		lua_setfield(L, -2, "out");
		lua_setfield(L, -2, "debug");
		lua_pushvalue(L, -2);
		lua_pushcclosure(L, LuaDebugOut, 1);
		lua_setfield(L, -2, "log");

		//if (allow_config_dialog) {
		lua_createtable(L,0,3);
		lua_pushvalue(L, -3);
		lua_pushcclosure(L, LuaDisplayDialog, 1);
		lua_setfield(L, -2, "display");
		lua_pushvalue(L, -3);
		lua_pushcclosure(L, LuaDisplayOpenDialog, 1);
		lua_setfield(L, -2, "open");
		lua_pushvalue(L, -3);
		lua_pushcclosure(L, LuaDisplaySaveDialog, 1);
		lua_setfield(L, -2, "save");
		lua_setfield(L, -2, "dialog");
		//}

		// reference so other objects can also find the progress sink
		//lua_pushvalue(L, -2);
		//lua_setfield(L, LUA_REGISTRYINDEX, "progress_sink");

		lua_pop(L, 1);//lua_pop(L, 2);
		lpd=new LuaProgressDialog(_parent,L);
	}

	void LuaProgressSink::Destroy()
	{
		ps->lpd->EndModal(0);
		delete this;
	}

	LuaProgressSink::~LuaProgressSink()
	{
		//delete update_timer;
		//update_timer.Stop();
		// remove progress reporting stuff
		lua_getglobal(L, "aegisub");
		lua_pushnil(L);
		lua_setfield(L, -2, "progress");
		lua_pushnil(L);
		lua_setfield(L, -2, "debug");
		lua_pop(L, 1);
		//lua_pushnil(L);
		//lua_setfield(L, LUA_REGISTRYINDEX, "progress_sink");
		wxDELETE(lpd);
		ps=NULL;
	}

	template<class T>
	void LuaProgressSink::SafeQueue(wxEventType type, T const& value) {
		wxThreadEvent *evt = new wxThreadEvent(type);
		evt->SetPayload(value);
		wxQueueEvent(lpd, evt);
	}

	/*LuaProgressSink* LuaProgressSink::GetObjPointer(lua_State *L, int idx)
	{
	if(lua_type(L, idx) != LUA_TUSERDATA){
	lua_pushstring(L, "Nieznany blad skryptu lua.");
	lua_error(L);
	}
	void *ud = lua_touserdata(L, idx);
	return *((LuaProgressSink**)ud);
	}*/

	void LuaProgressSink::ShowDialog(wxString Title)
	{
		SafeQueue(EVT_SHOW_PRGS_DIAL,lpd);
		SafeQueue(EVT_TITLE,Title);
	}


	int LuaProgressSink::LuaSetProgress(lua_State *L)
	{
		//LuaProgressSink *ps = GetObjPointer(L, lua_upvalueindex(1));
		int progress = lua_tonumber(L, 1);
		ps->SafeQueue(EVT_PROGRESS,progress);
		return 0;
	}

	int LuaProgressSink::LuaSetTask(lua_State *L)
	{
		//LuaProgressSink *ps = GetObjPointer(L, lua_upvalueindex(1));
		wxString task(lua_tostring(L, 1), wxConvUTF8);
		ps->SafeQueue(EVT_TASK,task);
		return 0;
	}

	int LuaProgressSink::LuaSetTitle(lua_State *L)
	{
		//LuaProgressSink *ps = GetObjPointer(L, lua_upvalueindex(1));
		wxString title(lua_tostring(L, 1), wxConvUTF8);
		ps->SafeQueue(EVT_TITLE,title);
		return 0;
	}

	int LuaProgressSink::LuaGetCancelled(lua_State *L)
	{
		//LuaProgressSink *ps = GetObjPointer(L, lua_upvalueindex(1));
		bool val=(ps->lpd)? ps->lpd->cancelled : false;
		lua_pushboolean(L, val);
		return 1;
	}

	int LuaProgressSink::LuaDebugOut(lua_State *L)
	{

		//LuaProgressSink *ps = GetObjPointer(L, lua_upvalueindex(1));

		// Check trace level
		if (lua_isnumber(L, 1)) {
			int level = lua_tointeger(L, 1);
			if (level > ps->trace_level)
				return 0;
			// remove trace level
			lua_remove(L, 1);
		}

		// Only do format-string handling if there's more than one argument left
		// (If there's more than one argument left, assume first is a format string and rest are format arguments)
		if (lua_gettop(L) > 1) {
			// Format the string
			lua_getglobal(L, "string");
			lua_getfield(L, -1, "format");
			// Here stack contains format string, format arguments, 'string' table, format function
			// remove 'string' table
			lua_remove(L, -2);
			// put the format function into place
			lua_insert(L, 1);
			// call format function
			lua_call(L, lua_gettop(L)-1, 1);
		}
		// Top of stack is now a string to output
		wxString msg(lua_tostring(L, 1), wxConvUTF8);
		ps->Log<<msg;
		ps->SafeQueue(EVT_MESSAGE,msg);
		return 0;
	}

	int LuaProgressSink::LuaDisplayDialog(lua_State *L)
	{
		//LuaProgressSink *ps = GetObjPointer(L, lua_upvalueindex(1));
		// Check that two arguments were actually given
		// If only one, add another empty table for buttons
		if (lua_gettop(L) == 1) {
			lua_newtable(L);
		}
		// If more than two, remove the excess
		if (lua_gettop(L) > 2) {
			lua_settop(L, 2);
		}


		wxSemaphore sema(0,1);
		ps->SafeQueue(EVT_SHOW_CFG_DIAL,&sema);

		// more magic: puts two values on stack: button pushed and table with control results
		sema.Wait();

		return 2;
	}

	int LuaProgressSink::LuaDisplayOpenDialog(lua_State *L)
	{
		//ProgressSink *ps = GetObjPointer(L, lua_upvalueindex(1));
		wxString message(check_string(L, 1));
		wxString dir(check_string(L, 2));
		wxString file(check_string(L, 3));
		wxString wildcard(check_string(L, 4));
		bool multiple = !!lua_toboolean(L, 5);
		bool must_exist = lua_toboolean(L, 6) || lua_isnil(L, 6);

		int flags = wxFD_OPEN;
		if (multiple)
			flags |= wxFD_MULTIPLE;
		if (must_exist)
			flags |= wxFD_FILE_MUST_EXIST;

		wxFileDialog diag(nullptr, message, dir, file, wildcard, flags);
		if (diag.ShowModal() == wxID_CANCEL) {
			lua_pushnil(L);
			return 1;
		}

		if (multiple) {
			wxArrayString files;
			diag.GetPaths(files);

			lua_createtable(L, files.size(), 0);
			for (size_t i = 0; i < files.size(); ++i) {
				lua_pushstring(L, files[i].utf8_str());
				lua_rawseti(L, -2, i + 1);
			}

			return 1;
		}

		lua_pushstring(L, diag.GetPath().utf8_str());
		return 1;
	}

	int LuaProgressSink::LuaDisplaySaveDialog(lua_State *L)
	{
		//ProgressSink *ps = GetObjPointer(L, lua_upvalueindex(1));
		wxString message(check_string(L, 1));
		wxString dir(check_string(L, 2));
		wxString file(check_string(L, 3));
		wxString wildcard(check_string(L, 4));
		bool prompt_overwrite = !lua_toboolean(L, 5);

		int flags = wxFD_SAVE;
		if (prompt_overwrite)
			flags |= wxFD_OVERWRITE_PROMPT;

		wxFileDialog diag(nullptr, message, dir, file, wildcard, flags);
		if (diag.ShowModal() == wxID_CANCEL) {
			lua_pushnil(L);
			return 1;
		}

		lua_pushstring(L, diag.GetPath().utf8_str());
		return 1;
	}


	LuaProgressDialog::LuaProgressDialog(wxWindow *parent,lua_State *_L)
		:wxDialog(parent,-1,"",wxDefaultPosition,wxDefaultSize,0)
		,cancelled(false)
		,finished(false)
		,closedialog(false)
		,L(_L)
	{
		SetForegroundColour(Options.GetColour(WindowText));
		SetBackgroundColour(Options.GetColour(WindowBackground));
		progress_display = new wxGauge(this, -1, 100, wxDefaultPosition, wxSize(600, 20));
		title_display = new KaiStaticText(this, -1, _T("")/*, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE|wxST_NO_AUTORESIZE*/);
		task_display = new KaiStaticText(this, -1, _T("")/*, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE|wxST_NO_AUTORESIZE*/);
		cancel_button = new MappedButton(this, wxID_CANCEL, _("Anuluj"));
		debug_output = new KaiTextCtrl(this, -1, _T(""), wxDefaultPosition, wxSize(600, 220), wxTE_MULTILINE|wxTE_READONLY|wxTE_RICH2);
		//debug_output->Hide();
		// put it in a sizer
		sizer = new wxBoxSizer(wxVERTICAL);
		sizer->Add(title_display, 0, wxALIGN_CENTER | wxALL, 5);
		sizer->Add(progress_display, 0, wxALL&~wxTOP, 5);
		sizer->Add(task_display, 0, wxALIGN_CENTER | wxLEFT | wxRIGHT | wxBOTTOM, 5);
		sizer->Add(cancel_button, 0, wxALIGN_CENTER | wxLEFT | wxRIGHT | wxBOTTOM, 5);
		sizer->Add(debug_output, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5);
		//sizer->Show(debug_output, false);

		// make the title a slightly larger font
		wxFont title_font = title_display->GetFont();
		int fontsize = title_font.GetPointSize();
		title_font.SetPointSize(fontsize + fontsize/4 + fontsize/8);
		title_font.SetWeight(wxFONTWEIGHT_BOLD);
		title_display->SetFont(title_font);

		// Set up a timer to regularly update the status
		// It doesn't need an event handler attached, as just a the timer in itself
		// will ensure that the idle event is fired
		Connect(44444,wxEVT_TIMER,(wxObjectEventFunction)&LuaProgressDialog::OnUpdate);
		update_timer.SetOwner(this,44444);
		update_timer.Start(50);

		//sizer->SetSizeHints(this);
		SetSizerAndFit(sizer);
		Center();

		//Bind(wxEVT_TIMER, &LuaProgressDialog::OnUpdate, this);


		Bind(EVT_TITLE, &LuaProgressDialog::SetTitle, this);
		Bind(EVT_MESSAGE, &LuaProgressDialog::AddDebugOutput, this);
		Bind(EVT_PROGRESS, &LuaProgressDialog::SetProgress, this);
		Bind(EVT_TASK, &LuaProgressDialog::SetTask, this);
		Bind(EVT_SHOW_CFG_DIAL, &LuaProgressDialog::ShowConfigDialog, this);
		Bind(EVT_SHOW_PRGS_DIAL, &LuaProgressDialog::ShowProgressDialog, this);
	}
	LuaProgressDialog::~LuaProgressDialog(){
		update_timer.Stop();
	}
	void LuaProgressDialog::ShowProgressDialog(wxThreadEvent &evt)
	{
		LuaProgressDialog *dlg=evt.GetPayload<LuaProgressDialog*>();
		dlg->ShowModal();
	}


	void LuaProgressDialog::AddDebugOutput(wxThreadEvent &evt)
	{
		//wxMutexLocker lock(data_mutex);
		pending_debug_output += evt.GetPayload<wxString>();
	}

	void LuaProgressDialog::SetTitle(wxThreadEvent &evt)
	{
		wxMutexLocker lock(data_mutex);
		title_display->SetLabelText(evt.GetPayload<wxString>());
		Layout();
	}

	void LuaProgressDialog::SetTask(wxThreadEvent &evt)
	{
		wxMutexLocker lock(data_mutex);
		task_display->SetLabelText(evt.GetPayload<wxString>());
		Layout();
	}

	void LuaProgressDialog::SetProgress(wxThreadEvent &evt)
	{
		wxMutexLocker lock(data_mutex);
		int prg=evt.GetPayload<int>();
		progress_display->SetValue(prg);
		//progress_display->Pulse();
	}

	void LuaProgressDialog::OnUpdate(wxTimerEvent &event)
	{
		
		if(finished){
			if (!pending_debug_output.empty()){
				//wxMutexLocker lock(data_mutex);
				debug_output->AppendText(pending_debug_output);
				pending_debug_output.Empty();
			}
			update_timer.Stop();
			cancel_button->SetLabelText(_("Zamknij"));
		}
		if(cancelled||closedialog){update_timer.Stop();EndModal(0);}
	}
	void LuaProgressDialog::ShowConfigDialog(wxThreadEvent &evt)
	{
		//cfgclosed=false;
		update_timer.Stop();
		LuaDialog dlg(L,true); // magically creates the config dialog structure etc
		KaiDialog* window = dlg.CreateWindow(this,title_display->GetLabelText());
		window->ShowModal();
		update_timer.Start();
		// more magic: puts two values on stack: button pushed and table with control results

		dlg.LuaReadBack(L);
		evt.GetPayload<wxSemaphore*>()->Post();
		if(dlg.IsCancelled()){cancelled=true;}
	}

	void LuaProgressDialog::OnCancel(wxCommandEvent &evt)
	{
		cancelled=true;
	}


};


