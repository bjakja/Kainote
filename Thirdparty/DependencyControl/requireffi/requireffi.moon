-- requireffi: works like the built-in `require` for luajit FFI because
-- ffi.load does not search the paths listed in package.paths.

-- example: DM = requireffi("DM.DownloadManager")

version = '0.1.2'
ffi = require 'ffi'
libPrefix, libSuffix = 'lib', '.so'
switch ffi.os
	when 'Windows'
		libPrefix = ''
		libSuffix = '.dll'
	when 'OSX'
		libSuffix = '.dylib'

packagePaths = ( namespace, libraryName, useCLibrarySearchPath ) ->
	paths = { }
	fixedLibraryName = namespace .. '/' .. libPrefix .. libraryName .. libSuffix
	package.path\gsub "([^;]+)", ( path ) ->
		-- the init.lua paths are just dupes of other paths, so we skip 'em.
		if path\match "/%?/init%.lua$"
			return

		path = path\gsub "//?%?%.lua$", '/' .. fixedLibraryName
		table.insert paths, path
		return

	-- Inserting the plain libraryName will cause luajit to search system
	-- C library paths for the library as well.
	if useCLibrarySearchPath
		table.insert paths, libraryName

	return paths

tryLoad = ( name, paths ) ->
	messages = { "FFI could not load %q. Search paths:"\format name }
	success = false
	for path in *paths
		success, library = pcall ffi.load, path
		if success
			return library, path
		else
			table.insert messages, "  - %q (%s)"\format path, library\gsub "[\n\t\r]", " "

	error table.concat messages, "\n"

requireffi = ( name, useCLibrarySearchPath = true ) =>
	local libraryName
	namespace = name\gsub '%.?([^%.]+)$', ( libName ) ->
		libraryName = libName
		return ''

	namespace = namespace\gsub '%.', '/'

	return tryLoad name, packagePaths namespace, libraryName, useCLibrarySearchPath

local versionRecord
versionRecord = {
	:version,
	__depCtrlInit: ( DependencyControl ) ->
		versionRecord.version = DependencyControl {
			name: "requireffi",
			version: version,
			description: "FFI.load wrapper for loading C modules.",
			author: "torque",
			url: "https://github.com/TypesettingTools/ffi-experiments",
			moduleName: "requireffi.requireffi",
			feed: "https://raw.githubusercontent.com/TypesettingTools/ffi-experiments/master/DependencyControl.json",
		}
}
return setmetatable versionRecord, { __call: requireffi }

