// Copyright (c) 2006, 2007, Niels Martin Hansen
// Copyright (c) 2016 - 2020, Marcin Drob
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

#pragma once

#include <wx/wx.h>

#include "AutomationDialog.h"
#include "MappedButton.h"
#include "KaiTextCtrl.h"
#include <utility>
#include "KaiStaticText.h"
#include "KaiGauge.h"

namespace Auto{
	wxDECLARE_EVENT(EVT_MESSAGE, wxThreadEvent);
	class LuaProgressDialog :public wxDialog
	{
	public:
		LuaProgressDialog(wxWindow *parent, lua_State *L);
		virtual ~LuaProgressDialog();
		wxBoxSizer *sizer;
		KaiGauge *progress_display;
		KaiStaticText *title_display;
		KaiStaticText *task_display;
		KaiTextCtrl *debug_output;
		MappedButton *cancel_button;
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
		void ShowOpenDialog(wxThreadEvent &evt);
		void ShowSaveDialog(wxThreadEvent &evt);
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
		wxString Log;
	};

	

};

