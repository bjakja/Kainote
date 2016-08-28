#ifndef AUTODIALOG
#define AUTODIALOG

#include <wx/wx.h>
#include <memory>
#include <vector>
extern "C" {
#include <lua.hpp>
	}


namespace Auto{

class LuaDialogControl {
	public:
		/// Name of this control in the output table
		wxString name;

		/// Tooltip of this control
		wxString hint;

		int x, y, width, height;

		/// Create the associated wxControl
		virtual wxControl *Create(wxWindow *parent) = 0;

		/// Get the default flags to use when inserting this control into a sizer
		virtual int GetSizerFlags() const { return wxEXPAND; }

		/// Push the current value of the control onto the lua stack. Must not
		/// touch the GUI as this may be called on a background thread.
		virtual void LuaReadBack(lua_State *L) = 0;

		/// Does this control have any user-changeable data that can be serialized?
		virtual bool CanSerialiseValue() const { return false; }

		/// Serialize the control's current value so that it can be stored
		/// in the script
		virtual wxString SerialiseValue() const { return ""; }

		/// Restore the control's value from a saved value in the script
		virtual void UnserialiseValue(const wxString &serialised) { }

		LuaDialogControl(lua_State *L);

		virtual ~LuaDialogControl(){};
	};

	/// A lua-generated dialog or panel in the export options dialog
	class LuaDialog {
		/// Controls in this dialog
		std::vector<LuaDialogControl*> controls;
		/// The names and IDs of buttons in this dialog if non-default ones were used
		std::vector<std::pair<int, wxString>> buttons;

		/// Does the dialog contain any buttons
		bool use_buttons;

		/// Id of the button pushed (once a button has been pushed)
		int button_pushed;

		wxDialog *window;

	public:
		LuaDialog(lua_State *L, bool include_buttons);
		~LuaDialog(){ 
			window->Destroy(); 
			for (size_t i = 0; i < controls.size(); ++i)
				delete controls[i];
		
		};
		/// Push the values of the controls in this dialog onto the lua stack
		/// in a single table
		int LuaReadBack(lua_State *L);
		bool IsCancelled(){return (button_pushed<0);};

		// ScriptDialog implementation
		wxDialog* CreateWindow(wxWindow *parent);
		wxString Serialise();
		void Unserialise(const wxString &serialised);
	};

};

#endif