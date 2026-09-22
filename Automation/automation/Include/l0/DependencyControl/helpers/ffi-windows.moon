ffi = require "ffi"

pcall ffi.cdef, [[
  int CloseHandle(void* hObject);
  int MultiByteToWideChar(unsigned int cp, unsigned long flags, const char* str, int cbMulti, wchar_t* wide, int cchWide);
  int SetConsoleOutputCP(unsigned int wCodePageID);
]]

CP_UTF8 = 65001 -- code page identifier for UTF-8, passed to the *CP() conversion APIs

haveKernel32, kernel32 = pcall ffi.load, "kernel32"

{
  -- the loaded kernel32 library namespace, or nil if it couldn't be loaded
  kernel32: haveKernel32 and kernel32 or nil

  -- whether kernel32 loaded successfully; gate any use of `kernel32`/`toWide` on this
  ---@type boolean
  haveKernel32: haveKernel32

  ---Converts a UTF-8 string to a NUL-terminated wide-char (UTF-16) buffer for the *W Win32 APIs.
  ---Requires kernel32 to have loaded (see `haveKernel32`); errors otherwise.
  ---@param s string A UTF-8 encoded string.
  ---@return ffi.cdata* buffer A wchar_t[] buffer holding the converted, NUL-terminated string.
  toWide: (s) ->
    n = kernel32.MultiByteToWideChar CP_UTF8, 0, s, -1, nil, 0
    buf = ffi.new "wchar_t[?]", n
    kernel32.MultiByteToWideChar CP_UTF8, 0, s, -1, buf, n
    buf

  ---Switches the attached console's output code page to UTF-8.
  ---Returns false if kernel32 is unavailable or no console is attached (output is redirected).
  ---@return boolean ok Whether the output code page was switched to UTF-8.
  setConsoleOutputUTF8: -> haveKernel32 and kernel32.SetConsoleOutputCP(CP_UTF8) != 0 or false
}
