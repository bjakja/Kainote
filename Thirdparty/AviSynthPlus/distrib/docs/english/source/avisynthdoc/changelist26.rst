
Changes.
========


Changes from 2.5
----------------


Additions
~~~~~~~~~

-   DirectShowSource support non-standard pixel types "YV24" and "YV16".
-   Info: Audio only clip now creates its own canvas video.
-   AviSource: Include packed/padded processing and -ve biHeight logic
    for compressed input.
-   Add Script Functions :- BitLRotate, BitRRotate, BitChange, BitClear,
    BitSet, BitTest and their asm aliases.
-   Add WeaveRows (blit cost) and WeaveColumns (slow) frame combining
    filters.
-   Add AudioDuration() [as float seconds], IsY8(), IsYV411() &
    PixelType() [as a string] script functions.
-   Add Echo and Preroll filters.
-   Add IScriptEnvironment::GetAVSLinkage() and DLLExport AVS_linkage for
    host usage of avisynth.dll.
-   DirectShowSource, 2.6 plugin, support pixel types "AYUV" as YV24,
    "Y41P" and "Y411" as YV411.
-   AviSource: Add Full and Auto pseudo pixel_types. Full is all
    supported. Auto is YV12, YUY2, RGB32, RGB24 & Y8.
-   Add "AudioLengthS" [as a string], "Ord" & "FillStr" script functions.
-   Add AudioTrim(clip, float, float) audio priority trimming, args in
    fractional seconds.
-   Add Trim(M, Length=N[, Pad=False]) and Trim(M, End=N[, Pad=False])
    function overloads for explicit Trimming. Length=0 means zero frame clip.
    End=0 means end at frame 0.
-   Add SeparateRows (zero cost) and SeparateColumns (slow) frame
    slashing filters.
-   Add Script Functions :- Acos, Asin, Atan, Atan2, Cosh, Sinh, Tanh,
    Fmod, Log10, BitLShift, BitRShiftS, BitRShiftU and Hex.
-   Add "ConditionalSelect","csc+[show]b" runtime filter.
-   Add dither option to Levels, RGBAdjust & Tweak.
-   Add BitAnd(), BitNot(), BitOr() & BitXor() script functions.
-   Add StrCmp() & StrCmpI() script functions.
-   Add YV24 support for Limiter show option.
-   Add "Global OPT_dwChannelMask={int}"
-   Add 0x0063F speaker mask for 7.1 WAVE_FORMAT_EXTENSIBLE.
-   Add .dll DelayLoad exception texts to crash message formatter.
-   ImageWriter, add support for printf formating of filename string,
    default is ("%06d.%s", n, ext);
-   Add avs_get_error(AVS_ScriptEnvironment*); to avisynth_c interface.
-   Catch and save AvisynthError text in more avisynth_c entry points,
    for kemuri-_9.
-   Add ScriptName(), ScriptFile(), ScriptDir() functions
    (WarpEnterprises).
-   Add SkewRows filter.
-   Histogram, Levels mode, Improve colour of chroma legends.
-   ConditionalFilter, teach about string results.
-   Add some more "Add/Remove Software" registry keys to the Installer
    (XhmikosR).
-   AviSource: Support both packed and DWORD padded raw planar input like
    with DSS.
-   Add IScriptEnvironment::ApplyMessage()
-   Add ImageSourceAnim (Wilbert)
-   Support user upgrade to 178 DevIL.dll (They need to manage CRT
    dependancies).
-   ImageSource: palette and compressed bmp images load correctly now
    (issue 894702) [need 178 DevIL.dll]
-   ImageSource: support for other formats like: gif, exr, jp2, psd, hdr
    [need 178 DevIL.dll]
-   Add YV24 mode to ColorBars.
-   Add ColorBarsHD based on arib_std_b28.
-   C-api usability enhancements from kemuri9 [Work in progress!]
-   Add Undefined(), AudioLengthLo(), AudioLengthHi(), IsYV16() &
    IsYV24() script functions
-   Allow newlines (and hence comments) before '{' -- Gavino
-   Added IScriptEnvironment::DeleteScriptEnvironment()
-   Added Histogram, population clamp % factor for "Levels" mode,
-   Histogram, revert "Stereo" mode to YV12, Add "StereoY8" mode,
-   AviSource: Support fourcc "GREY" as Y8
-   Added support for argument passing and EAX return value to
    SoftwireHelper.
-   Added "Global OPT_VDubPlanarHack=True" to flip YV24 and YV16 chroma
    planes for old VDub's.
-   Added "Global OPT_AVIPadScanlines=True" option for DWORD aligned
    planar padding
-   Added Matrix="AVERAGE" mode.
-   Added ContinuedDenominator/ContinuedNumerator(f[]i[limit]i) script
    functions.
-   Tweak: fix MaskPointResizing + put back Dividee ISSE code (use
    sse=true).
-   Added ChromaInPlacement, ChromaOutPlacement and ChromaResample
    options to planar colour conversions.
-   Added MaskHS.
-   Source tweaks to get ready for VC8.
-   Add Y8 for DevIL, planarize EBMP.
-   Planar support for many filters.
-   Added Info() time indicator on audio length and video (current frame
    & total). (2.5.8)
-   Added UtoY8 and VtoY8.
-   Added more info to Info(). (2.5.8)
-   ColorYUV: Added all adjustment parameters as conditional variables
    "coloryuv_SETTING". Enable by setting conditional=true.
-   ConditionalReader: Added support for type String.
-   ConditionalReader: Added offset keyword to offset all frame numbers
    after the keyword.
-   Added SincResize() with optional taps parameter (default is 4).
-   Added Custom band setting to SuperEQ to allow all 16 bands to be set
    from script. Usage: SuperEQ(clip,band1, band2, band3....) values are dB
    in float.
-   Added fast 0-1-0 kernel for YV24 to ConvertBacktoYUY2().
-   Added core formats: YV24, YV16, Y8, YV411.


Bugfixes
~~~~~~~~

-   Fixed DirectShowSource incorrect byte order for unpacking of pixel
    type "AYUV".
-   Fixed HexValue parsing values greater than 7FFFFFFF, now as unsigned
    hex.
-   Fixed ConditionalReader memory overrun parsing bools.
-   Fixed ResampleAudio NOP test to compare vi.num_audio_samples, not
    sample rate.
-   Fixed YV24 -> RGB24 overrun cleanup for widths%16 == 5.
-   Fixed RGB24 AddBorders with right=0.
-   Fixed conditional_functions error message names (Wilbert).
-   Fixed Audio cache ac_expected_next regression.
-   Fixed ImageSource deal with add 1 to IL_NUM_IMAGES bug (Wilbert)
-   Fixed Overlay YV24 V plane conversion.
-   Fixed Overlay YV24 mode with shared input clip, needed a
    MakeWritable.
-   Fixed ImageReader upside down TIFF in 178 DevIL. (Wilbert)
-   Fixed string+string bug when total length is 4096*K-1.
-   Fixed SincResize misuse of "int abs(int)" (Gavino). Fix Lanczos and
    Blackman sinc use of float == 0.0, use small limit "> 0.000001".
-   Fixed Classic mode legend drawing for planar right limit and yuy2
    centre line.
-   Fixed possible MT race. Use "env->ManageCache(MC_IncVFBRefcount,
    ...)" in ProtectVFB.
-   Fixed SwapYToUV output image size bug for 3 clip case.
-   Fixed Crop limit tests for RGB.
-   Fixed Overlay yellow tint on rec601 RGB import conversion.
-   Fixed YtoUV() output image size bug for 3 clip case.
-   Fixed ConvertToPlanar chroma alignment.
-   Fixed Levels (RGB) change use of PixelClip(x) to min(max(x, 0), 255).
-   Fixed SwapYtoUV yuy2 crash (StainlessS).
-   Fixed Overlay saturate UV in add and subtract mode.
-   Fixed Info.h range protect display characters (StainlessS).
-   Fixed AviSource packed planar import chroma offsets.
-   Fixed AviSource NULL GetWritePtr() failure due to premature setting
    of last_frame.
-   Fixed Mask rounding in greyscale calcs (Wilbert), minor refactor.
-   Fixed SelectRangeEvery audio snafu (Gavino).
-   Fixed LoadPlugin, SaveString of result string.
-   Fixed LoadPlugin, use _vsnprintf.
-   Fixed LoadVirtualdubPlugin, don't add vdub filter to chain on load
    failure.
-   Fixed rounding in RGB HResize (JoshyD) (affects all resizers)
-   Fixed error message name in the filter VerticalReduceBy2
-   Fixed SeparateFields() with variable parity input clip (Wilbert)
-   Fixed AviSource, cannot cast__int64* to long*, it does not work!
-   Fixed ConditionalReader: Don't allow out of range "Range" to
    overwrite edge values
-   Fixed MonoToStereo with stereo sources.
-   Fixed MergeChannels with only 1 input clip.
-   Fixed AviSource support for negative height DIB format AVI's.
-   Fixed Audio cache crashes.
-   Fixed resize with YV411, missing code.
-   Fixed ConditionalReader rounding with integer interpolation.
-   Fixed Softwire SSE2 bugs.
-   Fixed SSSE3 CPU detection.
-   Fixed SSSE3, SSE4.1 & SSE4.2 detection.
-   Fixed Fastwire encoding of instructions that are >2 opcodes
    (SSSE3+4).
-   Fixed _RPT5() macro for debug builds


Optimizations
~~~~~~~~~~~~~

-   ConvertToPlanarGeneric explicit add Cache before chroma rescaler.
-   Overlay minor refactor YV12 -> 444 chroma
-   Speedup ConvertToMono(), minor refactor MixAudio().
-   Change StackVertical/Horizontal to interative instead of recursive,
    2^N performace increase for 3 and more clips, i.e. 1 blit total instead
    of blit(blit(blit(...
-   RGBtoY8 Dynamic ASM code, support for RGB24.
-   YV24backtoYUY2 Dynamic ASM code.
-   UtoY8, VtoY8 abuse subframe, zero cost.
-   YV24<->RGB Add SSE2 and SSSE3 code paths, get rid of wide_enough.
-   ConvertToYUY2 Add SSE2, MMX restore full speed on platforms with poor
    ooox.
-   ConvertAudio, manage tempbuffer and floatbuffer independantly.
-   ConvertAudio, prefer SSE2 over 3DNow for super AMD cores.
-   Info.h, full refactor, a good example of "Never look down", thx
    StainlessS
-   DoubleWeaveFrames, If A not writable, try to write to B, else make
    new frame
-   Histogram, fix GetFrame/NewVideoFrame call order
-   HResizer, interleave code +4% faster
-   YtoUV() Abuse Subframe to snatch the Y plane / UV planes, Derestrict
    destination colorformat autogeneration.
-   ImageSource: Improve thread interlock code
-   ConditionalReader/WriteFile: Full refactor.
-   Replace _strdup with SaveString in AddFunction (Thanks Gavino)
-   SuperEQ: Improve channel unpacking/packing code.
-   H-Resize: Use SSE4.1 (movntdqa) loads for use once memory access.
-   H-Resize: Added SSE2 horizontal unpacker.
-   Resize: Use SSE3 (lddqu) loads for unaligned memory access.
-   Added ultra fast vertical PointResizer (64 pixel/cycle).
-   Added dynamic SSSE3 vertical resizer (16 pixel/cycle) ~ twice as fast
    as old MMX.
-   Added dynamic SSE2 vertical resizer (16 pixel/cycle).
-   Added dynamic MMX vertical resizer (8 pixel/cycle).
-   Added SSSE3 version for RGB<->YV24 conversions.
-   Added dynamic compiled MMX/iSSE for RGB<->YV24 conversions. Speed is
    approx 200% of C-code.


Changes
~~~~~~~

-   BlankClip: Supply useful defaults for new Audio/Video when using a
    Video/Audio only template clip.
-   BlankClip: Use duration from Audio only template as default length
    for new clip.
-   Define new IClip::SetCacheHints cachehint constants.
-   Force int call arguments to user script function float params to be
    explicit floats.
-   Splice pass CacheHints through to both children in + and ++ mode.
-   WriteFileStart/End save current_frame and set Last.
-   ConditionalReader do not ignore syntax errors in input file.
-   ImageSourceAnim Pad/Crop images to match first frame (Wilbert)
-   ImageSource Add version to messages (Wilbert)
-   Initial 2.6 API entry point linkage.
-   Use Invoke for graph tail, enhance non-clip output error reporting.
-   PopContext when inner block Asserts/throws (maxxon).
-   Remove duplicate definitions (Wilbert).
-   Enhance non-clip output error reporting.
-   Explicitly specify calling sequence as __cdecl for Avisynth softwire
    routines, (was the compiler default)
-   Use env->Invoke("Cache", ...) everywhere instead of
    Cache::Create_Cache(), allows for Cache to be overloaded by a plugin.
-   ConvertToYUY2 Change from 0-1-1 kernel to 1-2-1 kernel.
-   Tweak make Interp same units as minSat and maxSat.
-   Check HKEY_CURRENT_USER for PluginDir first. (henktiggelaar)
-   Make forced, -ve, planar alignment of chroma planes match
    subsampling.
-   Enforce planar alignment restrictions.
-   C-api: Remove func sub-struct from AVS_Library struct
-   Add error code to plugin load failure message
-   Make default planar AVI output packed. Control with
    OPT_AVIPadScanlines=True.
-   WriteFile() now supports unlimited number of unlimited strings. (was
    16 by 254 byte strings).
-   ConvertToRGB*, make C++ code sample chroma the same as the MMX code
    i.e. use both pixels.
-   ConvertToRGB*, use YV24 path for planar, complain when options are
    present for YUY2.
-   ConvertToYUY2, use YV16 path for planar, complain when options are
    present for RGB
    -   see: http://forum.doom9.org/showthread.php?p=1378381#post1378381
-   Thread safe code, part 2.
-   Correct IClip baked documentation
-   Passify compilation error/warnings (XhmikosR)
-   for, const, extern and ansi patches for VC2008 (SEt)
-   Disable OPT_RELS_LOGGING option
-   Change implicit Last parsing for argless, bracketless calls to match
    bracketed cases. (Gavino)
-   DirectShowSource: Support last minute format renegotiation thru
    IPin::QueryAccept() & Validate the size of the provided directshow
    buffer.
-   Remove non ascii chars from comments.
-   Add core stubs for DirectShowSource, TCPServer & TCPSource, report
    when plugins are missing.
-   Add note for original source downloads - SoundTouch
-   Add more lineage history to Info()
-   Move convertaudio, alignplanar, fillborder & MIN/MAX_INT definitions.
-   Run AtExit before dismantling world.
-   Change setcachehints definition from void to int. Test IClip version
    >= 5.
-   Move PixelClip definition to avisynth.cpp
-   SubTitle, etc, make X & Y options float (0.125 pixel granularity).
-   ShowSMPTE() supports all integer FPS and multiplies of drop frame
    FPS.
-   SubTitle, stop overwriting string constants (Gavino).
-   SubTitle, improve pixel registration (Gavino).
-   Make Info() CPU display hierarchical.
-   Thread safe code, part 1.
-   SoftwireHelper: explicit hardware exception handling.
-   Resize: Moved GetResampleFunction into Resamplefuntion, to allow
    overrides.
-   Resampler: Removed dead stlf code.
-   Updated Soundtouch to 1.31 (2.5.8)
-   Put dynamic matrix conversion into separate file.
-   Moved chroma subsampling to image_type section.
-   Added specific error reporting when requesting chromasubsampling with
    Y8.
-   Split up merge and plane Swappers.
-   Split up Plane transfers into separate classes.
-   Added automatic destination colorspace detection on planar YtoUV.
-   Took out greyscale and RGB32<->RGB24 from convert.cpp and placed them
    in separate files.
-   All code assuming UVwidth = Ywidth/2 and similar should be gone.

Please report bugs at `Sourceforge Avisynth 2 page`_ - or - `Doom9's Avisynth
forum`_

$Date: 2013/05/01 00:21:32 $

.. _Sourceforge    Avisynth 2 page:
    http://sourceforge.net/projects/avisynth2
.. _Doom9's    Avisynth forum:
    http://forum.doom9.org/forumdisplay.php?s=&forumid=33
