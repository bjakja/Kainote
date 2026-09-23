-- This library is unlicensed under CC0
local requireffi, ffi, looseVersionCompare
versionRecord = '0.7.2'

haveDepCtrl, DependencyControl = pcall require, 'l0.DependencyControl'

if haveDepCtrl
	versionRecord = DependencyControl( {
		name: "SubInspector",
		version: versionRecord,
		description: "Provides low level inspection and analysis of subtitles post-rasterization.",
		author: "torque",
		url: "https://github.com/TypesettingTools/SubInspector",
		moduleName: "SubInspector.Inspector",
		feed: "https://raw.githubusercontent.com/TypesettingTools/SubInspector/master/DependencyControl.json",
		{
			{ "ffi" }
			{ "requireffi.requireffi", version: "0.1.1" }
		}
	} )

	SIVersionCompat = DependencyControl( {
		moduleName: "SubInspector.Compat",
		version: "0.5.1",
		virtual: true
	} )

	looseVersionCompare = ( ASSIVersion ) ->
		siVer = DependencyControl( { moduleName: "SubInspector.Lib", version: ASSIVersion, virtual: true } )
		unless SIVersionCompat\checkVersion siVer, "major"
			return nil, ("Inspector.moon library is too old. Must be v%s.")\format siVer\getVersionString nil, "major"
		unless siVer\checkVersion SIVersionCompat, "minor"
			return nil, ("libSubInspector library is too old. Must be v%s compatible.")\format SIVersionCompat\getVersionString nil, "minor"

		return true

	ffi, requireffi = versionRecord\requireModules!

else
	ffi  = require 'ffi'
	requireffi = require 'requireffi.requireffi'
	SIVersionCompat = 0x000501

	versionComponents = ( version ) ->
		return math.floor(version / 65536) % 256, math.floor(version / 256) % 256, version % 256

	looseVersionCompare = ( ASSIVersion ) ->
		lmajor, lminor, lpatch = versionComponents ASSIVersion
		smajor, sminor, spatch = versionComponents SIVersionCompat
		if smajor != lmajor
			return nil, ("Major version mismatch. Got %d, wanted %d.")\format lmajor, smajor
		if sminor > lminor
			return nil, ("libSubInspector library is too old. Must be v%d.%d.%d compatible.")\format smajor, sminor, spatch

		return true

ffi.cdef( [[
typedef struct {
	int x, y;
	unsigned int w, h;
	uint32_t hash;
	uint8_t solid;
} SI_Rect;

uint32_t    si_getVersion( void );
const char* si_getErrorString( void* );
void*       si_init( int, int, const char*, const char* );
void        si_changeResolution( void*, int, int );
void        si_reloadFonts( void*, const char*, const char* );
int         si_setHeader( void*, const char*, size_t );
int         si_setScript( void*, const char*, size_t );
int         si_calculateBounds( void*, SI_Rect*, const int32_t*, const uint32_t );
void        si_cleanup( void* );
]] )

SubInspector, libraryPath = requireffi( 'SubInspector.Inspector.SubInspector' )

log = ( message, ... ) ->
	aegisub.log 2, message .. '\n', ...

collectHeader = ( subtitles ) =>
	scriptHeader = {
		"[Script Info]"
	}

	-- These are the only header fields that actually affect the way ASS
	-- is rendered. I don't actually know if ScriptType matters.
	infoFields = {
		PlayResX:   true
		PlayResY:   true
		WrapStyle:  true
		ScriptType: true
		ScaledBorderAndShadow: true
	}

	styles = { }

	seenStyles = false
	resX = 640
	resY = 480

	-- This loop assumes the subtitle script is nicely organized in the
	-- order [Script Info] -> [V4+ Styles] -> [Events]. This isn't
	-- guaranteed for scripts floating around in the wild, but fortunately
	-- Aegisub 3.2 seems to do a very good job of ensuring this order when
	-- a script is loaded. In other words, this loop is probably only safe
	-- to do from within automation.
	for index = 1, #subtitles
		with line = subtitles[index]
			if "info" == .class
				if infoFields[.key]
					table.insert scriptHeader, .raw

				if "PlayResX" == .key
					resX = tonumber .value
				elseif "PlayResY" == .key
					resY = tonumber .value

			elseif "style" == .class
				unless seenStyles
					table.insert scriptHeader, "[V4+ Styles]\n"
					seenStyles = true

				styles[.name] = .raw

			elseif "dialogue" == .class
				break

	-- If a video is loaded, use its resolution instead of the script's
	-- resolution. Don't use low-resolution workraws for typesetting!
	vidResX, vidResY = aegisub.video_size!
	if vidResX and (vidResX != resX or vidResY != resY)
		@.logFunc "Script and loaded video resolution mismatch, preferring video."

	@resX = vidResX or resX
	@resY = vidResY or resY

	@header = table.concat scriptHeader, '\n'
	@styles = styles

validateRect = ( rect ) ->
	bounds = {
		x: tonumber rect.x
		y: tonumber rect.y
		w: tonumber rect.w
		h: tonumber rect.h
		hash: tonumber rect.hash
		solid: (rect.solid == 1)
	}

	if bounds.w == 0 or bounds.h == 0
		return false
	else
		return bounds

defaultTimes = ( lines ) ->
	ffms = aegisub.frame_from_ms
	msff = aegisub.ms_from_frame

	seenTimes = { }
	times = { }
	hasFrames = ffms 0

	if hasFrames
		times.frames = { }
		for line in *lines
			with line
				for frame = ffms( .start_time ), .si_exhaustive == true and ffms( .end_time ) - 1 or ffms( .start_time )
					frameTime = math.floor 0.5*( msff( frame ) + msff( frame + 1 ) )
					unless seenTimes[frameTime]
						table.insert times.frames, frame
						table.insert times, frameTime
						seenTimes[frameTime] = true
	else
		for line in *lines
			with line
				unless seenTimes[.start_time]
					table.insert times, .start_time
					seenTimes[.start_time] = true

	-- This will only happen if all lines are displayed for 0 frames.
	unless 0 < #times
		return false
	else
		return times

addStyles = ( line, scriptText, seenStyles ) =>
	if @styles[line.style] and not seenStyles[line.style]
		table.insert scriptText, @styles[line.style]
		seenStyles[line.style] = true

	line.text\gsub "{(.-)}", ( tagBlock ) ->
		tagBlock\gsub "\\r([^\\}]*)", ( styleName ) ->
			if 0 < #styleName and @styles[styleName] and not seenStyles[styleName]
				table.insert scriptText, @styles[styleName]
				seenStyles[styleName] = true

class Inspector
	@version = versionRecord

	new: ( subtitles = error( "You must provide the subtitles object." ), fcConfig = libraryPath .. "fonts.conf", fontDir = aegisub.decode_path( '?script/fonts' ), logFunc = log ) =>

		success, message = looseVersionCompare SubInspector.si_getVersion!
		assert success, message

		inspectorFree = ( inspector ) -> SubInspector.si_cleanup inspector

		@inspector = ffi.gc SubInspector.si_init( 1, 1, fcConfig, fontDir ), inspectorFree

		assert @inspector, "SubInspector C initialization failed."

		@fontDir = fontDir
		@fcConfig = fcConfig
		@logFunc = logFunc

		success, message = @updateHeader subtitles
		assert success, message

	updateHeader: ( subtitles ) =>
		if nil == subtitles
			return nil, "You must provide the subtitles object."

		collectHeader @, subtitles
		SubInspector.si_changeResolution @inspector, @resX, @resY

		if SubInspector.si_setHeader( @inspector, @header, #@header ) != 0
			return nil, "Failed to set header.\n" .. ffi.string SubInspector.si_getErrorString @inspector

		return true

	reloadFonts: ( fcConfig = @fcConfig, fontDir = @fontDir ) =>
		SubInspector.si_reloadFonts @inspector, fcConfig, fontDir
		@fontDir = fontDir
		@fcConfig = fcConfig

	-- Arguments:
	-- subtitles: the subtitles object that Aegisub passes to the macro.

	-- line: an array-like table of line tables that have at least the
	--       fields `start_time`, `text`, `end_time` and `raw`. `raw` is
	--       mandatory in all cases. `start_time` and `end_time` are only
	--       necessary if a table of times to render the line is not
	--       provided and a video is loaded. Allows rendering 1 or more
	--       lines simultaneously.

	-- times: a table of times (in milliseconds) to render the line at.
	--        Defaults to the start time of the line unless there is a video
	--        loaded and the line contains the field line.si_exhaustive =
	--        true, in which case it defaults to every single frame that the
	--        line is displayed.

	-- Returns:
	-- An array-like table of bounding boxes, and an array-like table of
	-- render times. The two tables are the same length. If multiple lines
	-- are passed, the bounding box for a given time is the combined
	-- bounding boxes of all the lines rendered at that time.

	-- Error Handling:
	-- If an error is encountered, getBounds returns nil and an error string,
	-- which is typical for lua error reporting. In order to distinguish
	-- between an error and a valid false return, users should make sure
	-- they actually compare result to nil and false, rather than just
	-- checking that the result is not falsy.

	getBounds: ( lines, times = defaultTimes lines ) =>
		unless times
			return nil, "The render times table was empty."

		for i = 1,#lines
			line = lines[i]
			if nil == line.raw
				if line.createRaw
					line\createRaw!
				else
					return nil, "line.raw is missing from line #{i}."

		scriptText = { }
		seenStyles = { }
		if @styles.Default
			seenStyles.Default = true
			table.insert scriptText, @styles.Default

		for line in *lines
			addStyles @, line, scriptText, seenStyles

		table.insert scriptText, '[Events]'

		for line in *lines
			table.insert scriptText, line.raw

		scriptString = table.concat scriptText, '\n'
		if 0 < SubInspector.si_setScript @inspector, scriptString, #scriptString
			return nil, "Could not set script" .. ffi.string SubInspector.si_getErrorString @inspector

		renderCount = #times
		cTimes = ffi.new 'int32_t[?]', renderCount
		cRects = ffi.new 'SI_Rect[?]', renderCount

		for i = 0, renderCount - 1
			cTimes[i] = times[i + 1]

		if 0 < SubInspector.si_calculateBounds @inspector, cRects, cTimes, renderCount
			return nil, "Error calculating bounds" .. ffi.string SubInspector.si_getErrorString @inspector

		rects = { }
		for i = 0, renderCount - 1
			table.insert rects, validateRect cRects[i]

		return rects, times

if haveDepCtrl
	return versionRecord\register Inspector
else
	return Inspector
