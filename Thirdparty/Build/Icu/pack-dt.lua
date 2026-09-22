-- Copyright (c) 2014, Thomas Goyne <plorkyeran@aegisub.org>
--
-- Permission to use, copy, modify, and distribute this software for any
-- purpose with or without fee is hereby granted, provided that the above
-- copyright notice and this permission notice appear in all copies.
--
-- THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
-- WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
-- MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
-- ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
-- WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
-- ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
-- OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
--
-- Aegisub Project http://www.aegisub.org/

-- The data file and the symbol ICU links against are both version-stamped
-- (icudt78l.dat -> icudt78_dat).  The name comes from Icu.vcxproj so there is
-- one place to change on an ICU bump.
local DEFAULT_DAT = 'icudt78l'

local function try_open(filename, mode)
  local file, err = io.open(filename, mode)
  if not file then
    io.stdout:write(string.format('Failed to open "%s": %s\n', filename, err))
	os.exit(155)
  end
  return file
end

local icu_root, out_path, dat_name = ...
dat_name = dat_name or DEFAULT_DAT

-- "icudt78l" (the file) -> "icudt78_dat" (the symbol udata.cpp expects).
local symbol = dat_name:gsub('l$', '') .. '_dat'

local infile = try_open(string.format('%s/data/in/%s.dat', icu_root, dat_name), 'rb')
local outfile = try_open(out_path, 'w')

outfile:write("const unsigned char " .. symbol .. "[] = {")

local len = 0
while true do
  local bytes = infile:read(65536)
  if not bytes then break end

  for i = 1, #bytes do
    if len > 0 then outfile:write(',') end
    outfile:write(string.format('%d', bytes:byte(i)))
    len = len + 1
  end
end
outfile:write('};\n')

