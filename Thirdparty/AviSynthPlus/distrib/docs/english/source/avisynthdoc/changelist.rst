
2.58 Changes
============


Changes from 2.57
-----------------


Additions
~~~~~~~~~

* Added Czech doc translation.
* Added Polish doc translation (by Slim, Krismen & Co).
* Added Japanese doc translation (by niiyan).
* Installer standalone option for putting avisynth.dll, etc into install directory and NOT updating registry.
* Blankclip added audio channels= and sample_type= arguments.
* ConvertAudio(cii) available to plugins via env->Invoke().
* Added font aspect, rotation and alpha to text routines.
* Added ``/* xxx */`` block comments.
* Added ``[* [* xxx *] *]`` nestable block comments.
* SetMemoryMax(0) to just return current Memory Max value.
* Added planar YV12 color format to Compare() [Fizick].
* ColorKeyMask: Allow independant tolerance levels for each channel.
* Added Tweak Hue/Saturation range limiting.
* Added AudioLevels and Color2 modes to Histogram.
* Adding **global OPT_UseWaveExtensible=True** to your script enables WAVE_FORMAT_EXTENSIBLE audio output.
* Added ShowTime() script verb, like ShowSMPTE() but with milliseconds.
* Added BlackmanResize() and Spline64Resize().
* Modified DeleteFrame()/DuplicateFrame() to process multiple arguments.
* Added Min()/Max() script functions.


Bugfixes
~~~~~~~~

* Fixed ShowFiveVersions() YV12 chroma position in bottom half. Regression.
* Fixed Histogram() Classic mode restore graph brightness. Regression.
* Fixed Compare() graph pixel values exceeding YUV limits.
* Fixed AddBorders() args negative value clamping.
* Fixed AviSource() decoding to RGB24 logic regression.
* Added workaround for HuffYUV, Xvid reading past end of input buffer.
* Fixed current_frame value in nested runtime script invocations (Gavino).
* Fixed Dissolve overlap arg range checking (gzarkadas).
* Fixed OpenDMLSource() multithreading race problem (QuaddiMM).
* Fixed unsaved variableName string in ConditionalReader.
* Fixed Parser FunctionTable::TypeMatch() missing compulsory arguments.
* Dissolve bug (Various).
* Fixed DirectShowSource() convertfps=false logic regression.
* Fixed DirectShowSource() Flush task interlock race.
* Fixed DirectShowSource() QueryId() use CoTaskMemAlloc for returned value (Dean Pavlekovic).
* Fixed DirectShowSource() use GraphBuilder object for media control (Avery Lee).
* Fixed DirectShowSource() remove PulseEvent calls (Avery Lee).
* Fixed DirectShowSource() QueryAccept() modifying VideoInfo in running script (Haali).
* Fixed Blankclip clip= argument parsing.
* Fixed trashed GPF report, i.e reading 0x0 at 0x0 exception reporting.
* Fixed and refactored Overlay RGB<->YUV conversion routines.
* Fixed ImageReader incompletely inited videoInfo.
* Fixed Layer RGB32 100% alpha implementation, use level=257 (new default).
* Fixed avisynth_c.h avs_is_parity_known().
* Fixed C++ ConvertAudio::Saturate_int32() rounding.
* Fixed WriteFile(), Now remembers absolute path to file.
* Fixed Info() frame pitch, reports pitch of input frame.
* Fixed Invert() right edge memory overrun/corruption.
* Fixed Histogram() Classic mode pixel values exceeding YUV limits.
* Fixed Histogram() chroma plane initialization to 128.
* Fixed Conditional reader/writer illegally saving IScriptEnvironment pointer.
* Fixed YV12 Blur()/Sharpen() right edge pixel corruption with non-writable input frames.
* Fixed MMX Blur()/Sharpen() code to full 8 bit precision.
* Fixed IsAudioFloat()/IsAudioInt() script functions.
* Fixed Cache memory oversubscription of SetMemoryMax() limit.


Optimizations
~~~~~~~~~~~~~

* DirectShowSource() convertfps=true add 0.5ms rounding to expected sample time. Improves performance with millisecond based DS spliters.
* DirectShowSource() FPS detection as last attempt try getting the duration of 1st frame.
* DirectShowSource() convertfps=true rely only on sample start time.
* Cache: Trap returned NULL PVideoFrames, i.e. Don't crash, Winge about it!.
* Refactor horizontal planar resizer, no width restrictions or 2 byte overwrite.
* Provide a simple and fast text writing primatives, see info.h
* Make audio cache actually functional.
* Tweak() speed improvements.
* Subtract() speed improvements.
* Tuneup Overlay() ISSE Convert444ChromaToYV12 and also provide MMX version.
* PokeCache internal interface implemented.
* Cache and Memory management enhancements.


Changes
~~~~~~~

* Initial Memory Max value clamped to 512MB.
* Default Memory Max value restored to quarter of Free memory. Minimum 16Mb. As per 2.5.7.
* Test VFB data buffer for NULL malloc's, if so then Throw!
* Installer Start menu shortcut names translated.
* Test for UTF-8 and Unicode BOM prefixes and issue a useful warning.
* Removed useless current_sample from runtime script environment.
* Added critical section to CAVIFileSynth class.
* Enable Installer multi language support, thanks go to Fizick, gzarkadas, Malow, manusse, M.T, niiyan, talen9, TiGR, Underground78, WarpEnterprises, Wracko.
* FunctionTable::Lookup() now considers named arguments in making its selection.
* ImageWriter supports negative values as -count for End argument.
* ChangeFPS() use floor instead of round in source frame number calculation.
* Update usage of correct (updated) Rec.709 coefficients.
* Add TCPDeliver.map and DirectShowSource.map to releases.
* Convert to Dynamic Assembled rgb to yuy2 code.
* Avisynth_c.h boolean functions return 1 for true instead of arbitrary non-zero.
* Internal RGB2YUV() now copies Alpha top byte through.
* CoUninitialize() done immediately for S_FALSE CoInitialize() returns, i.e we do not hold a COM use count.
* Pfc, Softwire and SoundTouch updated and added as dependency projects.
* UPX updated to version 2.03 (2006-11-07).
* AVISource/WavSource map WAVEFORMATEXTENSIBLE back to equivalent WAVEFORMATEX.
* DirectShowSource() now recognises incorrect AM_MEDIA_TYPE subtype for WAVE_FORMAT_EXTENSIBLE audio and corrects the data and accepts it.
* DirectShowSource() now attempts to use partial graph from failing RenderFile call.
* DirectShowSource() now detects and handles non 32 bit aligned picture lines from dud codecs.
* Crop(align=true) tests actual alignment in each GetFrame call.
* Relax YV12 resizer width restriction, now mod 2 was mod 4.
* .AVSI failures during env->CreateScriptEnvironment() are now available to the GetError() interface.
* SetCacheHints(CACHE_RANGE, n) will now surrender frames to satisfy SetMemoryMax().
* CoInitialize()/CoUninitialize() now done as part of ScriptEnvironment creation/deletion.
* Much code from 2.6 base included. Typically IsYV12() changed to IsPlanar().


Issues
~~~~~~

Please report bugs and problems at `Sourceforge Avisynth2 page`_ - or - `Doom9's Avisynth Developer Forum`_


Changes from 2.56
-----------------


Additions
~~~~~~~~~

* Blur()/Sharpen() MMX=False option to force use of C++ routines.
* Explicitly try to request input sample rate from the audio codec (Tritical).
* Installer option for avisynth.lib, avisynth.exp and avisynth.map files.
* DirectShowSource() now sets _HIRESOUTPUT property of WMAudio Decoder DMO. Allows 24 bit samples and 6 channel decoding.
* Avisynth COM object now supports IAVIStreaming interface. Many apps blindly expect it and subsequently failed to load AVS scripts.
* Conditional Reader now also accepts T, F, Yes and No for bool data.
* DirectShowSource() log debug messages to file.
* DirectShowSource() force set framecount.
* DirectShowSource() accept audio streams of type WAVE_FORMAT_IEEE_FLOAT.
* Added KillVideo() verb.
* TimeStretch now exposes the tuning parameter of the SoundTouch library.
* Adding **global OPT_AllowFloatAudio=True** to your script enables WAVE_FORMAT_IEEE_FLOAT audio output.
* Explicitly request all the channels available in the audio stream [acm AC3]
* Explicitly try to request float, 32 bit then 24 bit samples from the audio codec.
* Accept uncompressed audio streams of type WAVE_FORMAT_IEEE_FLOAT.
* Added installer option to add "Avisynth Script" item to Explorer shell for new .avs file creation.
* Added avs_delete_script_environment and avs_subframe_planar to avisynth_c interface.
* Hack to allow Fraunhoffer MP3 codec to work when wBitPerSample==16. (Squid_80)
* Added portugese translation (by RoLon), and partly french translation (by JasonFly)
* Subtitle multi line text, set LSP arg and use \n. (foxyshadis, tateu)
* xxxFPS("preset") string preset FPS values. (Tritical)
* Better avisynth_c cdecl/stdcall mismatch detection and protection (Tritical).


Bugfixes
~~~~~~~~

* Fixed Tritcal bracketless call multiple cache over cache regression.
* Fixed DirectShowSource greyframe regression.
* Fixed DirectShowSource audio format negotiation regression.
* Fixed TCPDeliver YV12 transfer 50% to much data.
* Fixed TCPDeliver do Makewritable before getting params.
* Fixed TCPDeliver audio transfer off by 4 bytes.
* Fixed resizer core boundary condition crashes/errors/glitches.
* Fixed Invert() crash with small widths.
* Fixed small memory leak Amplify().
* Fixed grey frames with internal start, stop, start while DSS is initializing.
* Fixed AudioChannels() returning non-zero when no audio (Tritical).
* Fixed ConvertFPS() blend mode mix_ratio was wrong.
* Fixed AviSource() handling of dud AVI files that start with drop frames, just return the first key frame.
* Fixed DirectShowSource() handling of pin renegotiation.
* Fixed Overlay() "PC_Range" argument handling.
* Fixed SubTitle() crash when resources exhausted.
* Fixed Resizers cropping argument validation.
* Fixed % operator bad argument text message.
* Fixed stuffing Audio buffer with video graphics on Exception. Now returns E_FAIL. Video stream subsequently will return error text in graphic form.
* Fixed KillAudio() resetting nchannels and sample type to 0.
* Fixed Conditional Reader sscanf buffer overrun.
* Fixed Version() and VersionString() timestamp discrepancies.
* Fixed Serious inaccuracies in TimeStretch. It is still not sample exact!
* Fixed Negative int arg handling for ApplyRange() and Animate().
* Fixed Installer handling of readonly target files.
* Fixed ConvertAudio SSE2 to Float alignment test.
* Fixed (auto)LoadPlugin altname generation. (Tritical)
* Fixed SaveString memory block overrun.
* Fixed $Plugin!Functionname!Param$ bug. (Fizick)
* Fixed registry handle leak on $PluginDir$ lookup. (Dave Brueck)
* Fixed memory leaks avisynth_c.
* Fixed returning locked/protected VBF's to LRU. (Tritical)
* Fixed runtime mixed SEH/C++ exception handling for XPsp2. (Tritical)
* Fixed CAVIStreamSynth::Read audio buffer overrun. (Avery Lee)
* Fixed DLL handle leak in LoadPlugin. (Tritical)
* Fixed Assert("text") no longer parses % args.
* Fixed number parser returning inaccurate float conversions.
* Fixed ConvertFPS() blend mode not processing of chroma planes.
* Fixed resizer resampling pattern attempted use after deletion.
* Fixed resizer subpixel shifting functionality being a noop.
* Fixed Info() auto font selection metric.
* Fixed Conditional error checking of float RHS.
* Corrected colours in YUV ColorBars, Now match BT.801-1.
* TCPDeliver updates: Client: Fixed crash if client gets disconnected.
* TCPDeliver updates: Server: Remember to disconnect clients when shutdown.
* Fixed Turn*() YUY2 mod 2 height test.
* Fixed AVISource() corrupted error messages.
* Fixed AVISource() direct input drop frame handling.


Optimizations
~~~~~~~~~~~~~

* Parser tries to prevent adding a cache of a cache to graph.
* ImageSource() no longer use static buffer, uses cache and freezeframe.
* TCPDeliver big buffer enhancement (retro from 2.6 stream).
* SubTitle() releases all resources when the last frame of the clip is rendered.
* SubTitle() releases GDI resources after text map is created.
* ApplyRange() only builds 2 chains instead of 3.


Changes
~~~~~~~

* TCPDeliver add icon, make all resource US English.
* Map file is now generated for release builds.
* ConvertTo*() and GreyScale() now accept "Rec601" as a valid matrix.
* DirectShowSource.dll upx'ed.
* Upgrade internal copy of SoundTouch library to 1.3.1.
* SoundTouch now available in RelSym build.
* Animate Integer arg enumeration no longer rounds toward positive infinity. Both positive and negative enumerations are identical. i.e. For Animate(0, 10, "Foo", 0, 0, 10, -10), Foo's arg1 = -arg2 for all frames.
* Selecting associate open with Notepad with .avs files in the installer now also includes .avsi files.
* The Installer now pushes a recovery dialog box when unwritable files are encountered during an install. The user may manually correct the problem and retry installing that file.
* SetMemoryMax() minimum now 4Mb instead of 16. (Tritical)
* Remove 50 plugin auto prescan load limit. (Tritical)
* COM QueryInterface calls now return S_OK instead of NULL.
* Bracketless call of functions without arguments now get a cache (Tritical).
* Over-range numbers now raise a compile time exception.
* xxxFPS(float) now uses continued fraction to generate a minimal rational pair (Raymod2).
* ChangeFPS(linear) now raises a compile time exception if the speed change ratio is > 10.
* ConvertFPS() blend mode works for all pixel formats. (Tritical)
* Info() retrofit of 2.60 updates.
* TCPDeliver.dll upx'ed.
* RGB ColorBars +Q and -I bars, Hue is now correct, Luma is NOT zero to achive this.
* AVISource Audio no longer limited to 2 channels.
* SaveString memory blocks are now 32 bit aligned.
* Default planar chroma planes mod 16 aligned. See SetPlanarLegacyAlignment().


Changes from 2.55
-----------------


Additions
~~~~~~~~~

* SSE3 capable CPU detection in env->GetCPUFlags and Info().
* RGB32 mode in TemporalSoften.
* ``*Resize()``, src_height and src_width when negative work as in crop.
* Added options to DirectShowSource (seekzero, timeout and pixel_type).
* Added AudioDubEx(), blindly accepts video and audio streams.
* Added Load_Stdcall_Plugin(), alias for LoadCPlugin() (won't disappear when avisynth_c.dll is loaded)
* Added DevIL support for RGB32 to ImageSource.
* Added Russian language documentation. Thanks to Fizick and his team. Well done.
* Added Merge() filter. Includes very fast iSSE pavgb code for weight=0.5.
* Added MergeARGB(), MergeRGB() filter.
* Added ShowRed(), ShowGreen(), ShowBlue() filters.
* Added Planar version of env-&gtgSubFrame() (thanks TSP).
* Added SetPlanarLegacyAlignment() to select Planar alignment algorithm.
* Added Audio padding control to Trim().
* Added operator muldiv(multiplicand, multiplier, divisor).
* Added AssumeScaledFPS(multiplier, divisor, sync_audio) maps vi.MulDivFPS.
* Added method VideoInfo.MulDivFPS(unsigned multiplier, unsigned divisor) does rational scaling with normalizing and overflow protection of FPS property.
* Added offsets, gamma and analyze option (ala ColorYUV) in RGBAdjust.
* Added preliminary 23.976fps film drop frame support to ShowSMPTE(), Anyone know the proposed SMPTE algorithm?
* Added Spline16Resize, Spline36Resize, GaussResize and LanczosResize(tap=xx)
* Added options to ShowFrameNumber: ShowFrameNumber(offset=10, x=360, y=576, font="georgia", size=24, text_color=$ff0000)
* Added integer offset to ShowSMPTE: ShowSMPTE(offset_f=10)
* Added options to ShowSMPTE: ShowSMPTE(offset="00:00:00:30", x=360, y=576, font="georgia", size=24, text_color=$ff0000)
* Added Optional FPS=24 parameter to Dissolve and Fade*() for processing audio only clips.
* Added FadeIn0(), FadeOut0(), FadeIO0() variants that don't add an extra frame.
* Added Fractional resampling support in ResampleAudio().
* Added HasVideo() and HasAudio() script functions.
* Added Level option to Tone().
* Added SFloat support in ResampleAudio().
* Added ColorBars(pixel_type="YUY2, YV12").
* Added env->ManageCache() interface in AviSynth.h.
* Added VideoFrameBuffer 16 byte guardband protection/detection in Debug mode.
* Added EBX compiler bug protection/short circuiting to Cache::GetFrame().
* Added YV12 support for ShowFiveVersions().
* Added "[sse]b" option to Tweak to re-enable the (slow) SSE code (maybe AMD's might run it faster).
* Added Limiter(show=show_luma/show_luma_grey/show_chroma/show_chroma_grey) shows out of bound luma/chroma; ``*_grey`` makes the rest of the pixels greyscale.
* ConvertTo*(Matrix="Rec709, PC.709, PC.601") conversions supported.
* ConvertFPS()/ChangeFPS() copies FPS from a 2nd clip.
* GreyScale() RGB supports Matrix="Rec709, Average".


Bugfixes
~~~~~~~~

* Fixed corruption at the end of the IScriptEnvironment vtable.
* Fixed memory leaks in Overlay and AVSChar/AVSTime.
* Fixed End_of_Stream reset on seek in AudioStreamSource:
* Fixed SegmentedDirectShowSource() argument parsing.
* Fixed ``*Resize()``, src_height and src_width when negative work correctly.
* Fixed minor memory leak in env.VSprintf(), [ul]case() and ``*str()`` also remove 4k limits, thanks Tritical.
* Fixed Normalize scribling into memory for float samples.
* Masked "Evaluate: System Exception - Access Violation" in :- FadeIO*(), RGBAdjust(), Tweak(), Lanczos*Resize() and GaussResize().
* Fixed rounding in YUY2 turnleft/right chroma.
* Fixed AVSC_USE_STDCALL declaration in avisynth_c.h (was ACSC_USE_STDCALL).
* Fixed BlankClip(clip) now competely duplicates the donor clip's VI including parity.
* Fixed AssumeTFF/BFF() to correctly update internal parity state.
* Fixed Animate audio switching.
* Fixed aligned UVpitch from width rounding.
* Fixed 2 bit crosstalk in YUV horizontal resizers, thanks Squid.
* Fixed resource leak in DirectShowSource(), thanks Tritical.
* Fixed minor memory leak in Subtitle() and string(), thanks Tritical.
* Fixed global clip close down problem, thanks Tritical.
* Uninstaller now remove Docs\pictures directory and DirectShowSource and TCPDeliver plugins.
* Fixed ImageReader single file handling.
* Fixed ImageWriter info==false no longer write status text onto frame.
* Trap .WAV clsid handler GPF with filenames gt 47 chars.
* Fixed YUV text access violation, correct bounds alignment tests. (Regression)
* Fixed ExpFunctionCall::Call memory leak, thanks Tritical.
* Fixed PClip leak in MergeChannels(). Destructor chain not called.
* Fixed RGB text alignment inversion. Regression in May 5th ver.
* Fixed vfw resource leaks when opening bad AVI files.
* Fixed rmvb stuck at 100% during encoding (thx stevencover).
* Fixed Loop audio processing.
* Fixed GeneralConvolution crash with cropped input.
* Restored forced (negative) planar luma alignment functionality.
* Fixed a stack of memory leaks, thanks Tritical.
* Fixed Direct AVISource input of raw YV12 and I420 sources.
* Fixed (fingers crossed) "Evaluate: Recognized exception!" Problems with XPsp2 hard terminate.
* Fixed CACHE_RANGE internal scope test, thanks Tritical.
* Fixed Mask() calc of greyscale, red/blue swapped.
* Fixed FPS overflow with Select...(), Interleave() and variants.
* Fixed subtract mode in Overlay (chroma is correctly subtracted).
* Fixed align parameter in Subtitle.
* Windout code is working again (broking in the previous betas).
* Corrected multi-channel audio fading.
* Dissolve and Fade*() process audio only clips, assumes 24fps for position calcs.
* Fixed wrong chroma in DoubleWeave() of FrameBased YV12 material.
* Fixed crash from BlankClip() with an audio only clip template.
* Fixed AudioCache corruption on buffer resize.
* Fixed ResampleAudio() clicks/pops due to creeping error.
* Fixed SSE/SSE2 ConvertAudioTo16/32Bit() positive value saturation.
* Masked EBX compiler bug in :- audio.cpp converaudio.cpp convert_yv12.cpp memcpy_amd.cpp focus.cpp layer.cpp merge.cpp resample.cpp resize.cpp text_overlay.cpp conditional_functions.cpp 444convert.cpp blend_asm.cpp
* Fixed ConvertToRGB() src->rowsize==8 crash.
* Fixed Memory leak, deleted VideoFrameBuffers.
* Fixed env->NewVideoFrame() returning short frame buffer.
* Fixed vi.FrameFromAudioSamples() truncation.
* Fixed ShowFiveVersions() unused corner rectangles had random contents. Now grey filled
* Fixed YUV plane swapper reports corect name.
* Fixed MergeChroma reporting itself as MergeLuma.
* Fixed MergeChroma doing MakeWriteable() on the wrong clip.
* Fixed Normalize (it was only sampling half of the samples per mouthful; it was ignoring the scaling factor).
* Fixed GreyScale() EBX compiler bug.
* Fixed flipped frame on imagesources.
* PlaneDifference in ConditionalFilter was not reporting exactly 0, if the planes were the same.
* Fixed big bug in compressed YV12 TCPSource / TCPClient in TCPDeliver.
* Increased stringbuffers, 4K to 32K (WarpSharp problem).
* Fixed Overlay(softlight/hardlight) overflow.
* Masked "Unknown exception" in :- audio.cpp avs_soundtouch.cpp color.cpp field.cpp focus.cpp fps.cpp levels.cpp resample.cpp source.cpp
* Fixed WavSource() leaving .WAV files open.
* Fixed glitches in multichannel audio transitions in Dissolve() and Fade*()
* Fixed Green/Blue channel swap in C version of Layer()
* Restored GreyScale() RGB C code.
* Fixed MMX YV12 Blur() double blurring every 8th pixel.
* Blur()/Sharpen() Edges all processed consistantly (edges are reflected).
* Fixed YV12 Blur() width < 16 fatal crash.


Optimizations
~~~~~~~~~~~~~

* Performance improvents in transfer functions in TCPDeliver.
* Normalize() for 16 bit stop when a max-int value sample is seen.
* Run sort -unique|detab on color_rgb.avsi, got rid of all the duplicates.
* YV12 MergeLuma(), MergeChroma() include very fast iSSE pavgb code for weight=0.5.
* Cleanup VideoFrame garbage collection on script close (Thanks TSP).
* Text overlay antialiaser tweaked, 8% faster.
* RGB32 FlipHorizontal() code tweaked.
* ResampleAudio() MMX for int16 samples, approx 3.25 times faster.
* ResampleAudio() reworked C++ int16 code, approx 35% faster.
* ResampleAudio() reworked buffer management, now linearly accesses child->GetAudio().
* Cache and Memory Managment have been reworked.
* ConvertAudioToFloat() fast SSE and very fast SSE2, opt/skip redundant output copy.
* ConvertAudioTo16,32Bit() very fast SSE2.
* SwapUV() YV12 swaps pointers in PVideoFrame, zero cost!, YUY2 fast iSSE code.
* UtoY(), VtoY(), YtoUV() YUY2 faster C++ code.
* MergeLuma(), MergeChroma() now MMX (not iSSE), has C++ versions, YV12 BlendPlane now does 8 pixels per loop almost 2x faster!.
* ColorBars() generates Tone in constructor buffer, copied out in GetAudio(), avoid 48000 sin(double) per second.
* EnsureVBRMP3Sync() uses 256K transfer buffer when seeking (should be > 5 times faster now.)
* Normalize() uses 256K transfer buffer to analyze peaks (should be > 5 times faster now).
* Tweak: added C++ code (lookup table), which is now faster than the old iSSE code.
* Audio.cpp reworked code in audio filters which do 64 bit operations (most routines are between 2 and 5 times faster).
* ConvertToRGB32() MMX RGB24 -> RGB32.
* Blur()/Sharpen() when H or V = 0 skips that pass.
* GreyScale() MMX YUY2.
* Blur()/Sharpen() now MMX (not iSSE), faster, fewer mod(2^n) restrictions.


Changes
~~~~~~~

* Stop extra search of LIBC, add relsym build - Release with Symbols.
* The avisynth_c plugin entry point is now officially "avisynth_c_plugin_init@4" (don't include @4 anywhere the compiler does it for you), this is not actually a change due to a bug in avisynth_c.h, which incidently caused it to be this already.
* Info() now autoselects a smaller font to fit info in small frames.
* Info() now distinguishes between "assumed" field parity and field parity.
* Animate now selects parity through the filter chain.
* New improved cache!
* Installer now populates "All Users" shell tree on NT variants. Admin/Uninstaller stuff is only added to current user.
* Uninstaller now deletes DirectShowSource.dll and TCPDeliver.dll from plugin directory.
* ImageReader doesn't auto fail over to DevIL for DIB/BMP type files. (DevIL crashs on palletted BMP's)
* ImageWriter now throws an exception for non RGB24 format with DevIL processing.
* ImageReader/Writer info text colour now pale yellow $f0f080.
* Replaced VideoFrame::SubFrame() with env->SubFrame() (Ready for MultiThreading, Thanks TSP).
* Option for planar alignment (default 16 bytes) to be based on chroma planes, luma alignment is (still) 2x chroma. Will become the default in the next version.
* Support for `vfr content in DSS`_:

::

    DirectShowSource("F:\Guides\Hybrid\vfr_startrek.mkv", fps=119.88, convertfps=true) turns vfr into cfr stream by duplicating frames.

* Histrogram: in color_mode - YUY2: Invalid values (below 16 and above 235) will be colored brown/yellow-ish. Made those values more visible.
* Evaluate: Now breaks outs and reports system exceptions instead of reporting the useless "Evaluate: Unrecognized exception!".
* BlankClip() no longer gobbles any implicit last clip as the template clip. You have to explicitly declare you want a template clip i.e. BlankCLip(Last). "BlankClip()" now always returns the default blank clip.
* Avisynth.h FrameFromAudioSamples/AudioSamplesFromFrames() now test for divide by zero and return zero when encountered instead of crashing.
* SeparateFields() now throws an exception for an IsFieldBased() source.
* Weave() now throws an exception for an IsFrameBased() source.
* Cache policy CACHE_NOTHING currently no long returns all VFB's. Under review.
* Avisynth.h SetFPS() now test for zero numerator or denominator if found it sets num=0, den=1.
* Audio Cache Autodetect lower metric from 25 to 5 on skip forward (step back still 25).
* ConvertAudio() include the prefered type as an acceptable type.
* ConvertAudio() passes SetCacheHints() thru to grandchild.
* Normalize() displays the frame number of the peak.


Changes from 2.54
-----------------


Additions
~~~~~~~~~

* Added a huge list of color presets, that can be used instead of colors (which can be found in the plugins folder: colors_rgb.avsi).
* Added: BlankClip now has color_yuv, that allows setting and YUV color for YUV clips.
* Added GZIP huffman compression to TCPDeliver.
* Added AssumeFPS(clip1, clip2 [, sync_audio=true/false]) (stickboy again)
* Added audio=true/false to SelectRangeEvery. This will optionally also cut up audio according to the select. Default: true.
* Added Turn180().
* Added IsAudioFloat() and IsAudioInt() as clip properties.
* Added FrameRateNumerator() and FrameRateDenominator() as clip properties.
* Added AudioLenghtF() as clip property.
* Added experimental "after_frame=true/false" to ScriptClip.
* Added FOURCC parameter to AviSource by stickboy.
* Added Lanczos4Resize().
* Added french documentation.
* Added float audio and multiple channels support to Dissolve.
* Added WriteFile, WriteFileIf, WriteFileStart, WriteFileEnd. (WarpE)
* Added dll-name prefix to plugins as per http://forum.doom9.org/showthread.php?s=&threadid=72235.
* * Syntax is DLLNAME_function(), where DLLNAME is the filename of the dll containing the function.
* Added automatic audio cache.
* Added dotted lines at zero levels to view bias in Histogram(mode="stereo").
* Added CPU stuff to Info().
* Added GeneralConvolution divisor, auto.
* Added audio cache after EnsureVBRMp3Sync.
* Added TimeStretch(). This filter can change speed of the sound without changing the pitch, and change the pitch of a sound without changing the length of a sound.


Bugfixes
~~~~~~~~

* Fixed "Unknown exception" in conditional filter expressions. (August 17th regression)
* Fixed: DirectShowSource properly releasing filters on unload.
* Fixed: Huge stability fix by IanB. This should remove a bunch of "Evaluate: Unregnized exception", and crashes on reload in vdub.
* Fixed: ImageReader/ImageSource flipped error messages and info overlay.
* Fixed MergeLuma not always updating properly.
* TCPDeliver should compile now.
* AssumeFPS: Added sanity check to ensure that denominator isn't zero.
* Fixed Loop() when called with no video (stickboy)
* Fixed error not being thrown in Conditionalfilter on an invalid operator.
* Proposed fix for zero coefficient.
* Fixed crash on certain resolutions in Resize.
* Made TCPDeliver compile without complaining about missing files.
* Fixed linecount on multiline strings (Bug ID 989276]
* Fix VideoFrameBuffer cache corruption
* Fixed VideoFrameBuffer cache corruption during "Plan C" memory recovery.
* The problem with ApplyRange/Animate's inability to use functions that take no additional arguments should be fixed.
* Removed overflow bug in ssrc upsampler.
* Fixed Audio cache crash if no audio.
* Cleaned up ApplyYV12 in textoverlay.
* Fixed float audio in Dissolve.
* Fixed audiobits clip properties now returning bits and not bytes.
* Fixed ConditionalReader inaccuracies on integer interpolation.
* Fixed Mask() problem with footage with different pitches.
* Subtract now clamps errors instead of overflowing.
* Fixed old plugin names actually working.
* Added specific (and simpler) stereo mode to TimeStretch - no more drifting.
* Made ISSE YUY2 HorizontalReduceBy2() more "safe".
* Fixed missing plane in TemporalSoften.
* Corrected some quirks in TemporalSoften scenechange on artificial sources.
* Fixed IsParityKnown() in avisynth.h - thanks to stickboy.
* Random in scalemode is not returning limit value - thanks you stickboy!
* Fixed: Trim audio could crash, if sample types were different.
* Fixed crash-on-exit in SSRC, if rateconversion was skipped.
* Fixed one frame missing in TemporalSoften.
* Fixed chroma moving half a pel in Overlay() with YV12-input mode.
* Fixed Trim audio crash if sample types were different.
* Fixed crash-on-exit in SSRC, if rateconversion was skipped.
* Fixed minor glitches in audio cache (out of range requests)
* Fixed ConvertToYV12(interlaced) incorrect sampling for top field chroma.
* Fixed Memory leak in Vertical Resizer.
* Fixed PointResize() exact odd sub/multiple width/height returning trash frames.
* Fixed PointResize() image not centred when expanding.
* Fixed TurnLeft()/TurnRight() crash with multiple colour spaces in same script.
* Fixed TurnLeft() in yuy2 colour space off by 1 pixel down.


Optimizations
~~~~~~~~~~~~~

* SSRC now has aligned data (slightly faster and SSE ready).
* Added MMX optimizations to Overlay mode lighten+darken with opacity=1.0, with no mask.
* Added SSE float to int audioconversion.
* Added 3DNOW! optimizations to sample-type conversions int to float, and float to int. Much faster.
* Added faster MMX to Invert by ARDA.
* Added RGB24, YUY2 and YV12 MMX Invert() function.
* Added MMX/ISSE chroma convertion to Overlay with YV12 input.
* Added MMX function for Overlay(mode="blend"), when a mask is used and opacity = 1.0.
* Removed unneeded DevIL components


Changes
~~~~~~~

* TCPDeliver: Simplification and cleanup. Planar pitches handled more consistently.
* DirectShowSource now requests interlaced material to avoid internal (crappy) WMV9 deinterlacer. (Thanks to Russel Lang)
* ImageReader/ImageSource now accepts relative paths in all configurations.
* Adjust audio length in SelectRangeEvery to match new video length.
* Updated installer.
* ImageReader: tweaked default parameters.
* ImageReader: made ColorBars parameters optional (like docs claim!)
* ImageReader: No more "Image not in range". First frame is ALWAYS frame 0.
* Updated SoftWire codegenerator to latest version.
* ImageReader: Static image support, Floating-point FPS, aliased to ImageSource, frames automatically flipped when necessary.
* ImageWriter: Optional "info" parameter to show filename.
* ImageReader / ImageWriter: All errors returned as text clips.
* AviSynth will longer resize to non-mod4 widths in YV12 mode!
* Dissolve can now handle audio input with different sample types.
* Dissolve now checks if samplerates are the same.
* Trim/Dissolve: Better error reporting.
* Updated DevIL libs; removed unneeded !DevIL components; improved error reporting in ImageReader / ImageWriter.
* Updated Overlay(mode="multiply") to work more logical. Old functionality is no more!
* DirectShowSource() is now a plugin, and is automatically placed in the plugin directory by the installer.
* Disabled avisynth_c plugin autoloading (no longer a compatible way to do so).
* FrameRate() now calcs as (double)numerator/(double)denominator. (still returns a float)


Changes from 2.53
-----------------


Additions
~~~~~~~~~

* Added ConditionalReader(). This enables users to load per frame settings into variables accessible to conditional variables.
* Added "Hardlight", "SoftLight", "Difference" and "Exclusion" as Overlay blend modes.
* Added mode="chroma", "luma", "lighten", "darken" to Overlay.
* Overlay() now accepts RGB24/32 output.
* Added "pc_range=true/false" parameter to overlay(). This will make all RGB<->YUV conversions inside Overlay assume [0->255] YUV range, and not CCIR 601 [16->235] range. Default is false.
* Added RGB input conversion to overlay.
* ShowAlpha now returns RGB, YUY2, or YV12 via the pixel_type argument. The latter two can be used to layer an RGB clip with alpha transparency data onto a YUV clip using the 3-argument form of Overlay().
* Added Overlay() for doing advanced video overlays/layers.
* Added SuperEQ(). SuperEQ is a very precise 16 band equalizer.
* Added IsYUV(clip) to script (it was only present in the documentation). (Party Time)
* Added internal audiocache by [WarpEnterprises]. Added automatically by the filters that need it.
* Added fast=true/false mode to SSRC. This will use a faster mode for resampling. Default is false.
* Added Histogram(mode="stereo") shows a clasic stereo graph (I guess) from the audio in the clip. Some may know these from recording studios. Quite nice actually.
* Added Histogram(mode="stereooverlay"). Draws the stereograph on top of the original image. YV12 only.
* Added ChangeFPS(linear=true/false). This will make AviSynth request frames in a linear fashion, when skipping frames. Default:true.
* Added SSRC(int samplerate) SSRC resampling. Note that it only downsamples. Audio is always converted to float.
* Added Tone(float length, float frequency, int samplerate, int channels, string type). This will generate sound.
* *   Length is in seconds. Type can be "Silence", "Sine" (default), "Noise", "Square", "Triangle" or "Sawtooth".
* *   Defaults are Tone(10.0, 440, 48000, 2, "sine").


Bugfixes
~~~~~~~~

* Fixed out-of-bounds read in Normalize.
* Fixed compiler warnings in avisynth_c
* Fixed very small sample corruption in SSRC.
* Fixed audio corruption problem if audio with start < 0 was requested. (introduced in Dec. 30th binary).
* Fixed very small sample corruption in SSRC.
* Better seeking precision in audio in DirectShowSource.
* Fixed crashes and Audio corruption in ConvertToMono().
* Fixed UnalignedSplice if only audio was present.
* Fixed Trim crash, if only audio was present (throws an error).
* Fixed slowdown on multiple Directshow Sources.
* Fixed green bars in small video with mode="levels" and mode="color". For Histogram


Optimizations
~~~~~~~~~~~~~

* Internal audio cache now has better efficiency.
* Optimization: MMX in Dissolve.

Changes
~~~~~~~

* Added Kevin Atkinsons avisynth_c 0.20 (stdcall) API.
* Updated installer. Removed the old one.
* SSRC: "fast" is now true per default. This setting is recommended, unless you are doing a big samplerate adjustment (not just 48000 -> 44100).
* Updated installer. (Thanks to V_ICE for inspiration).
* Re-Added German documentation.
* EMMS is now executed between all filters to avoid potential FPU-states to be carried from one filter to the next.
* Reimplemented SSRC. It is now capable of running multiple instances, it has been better tested (occational strange sample should be eliminated). This version support both upsampling and downsampling with very high precision.
* If any filter should request audio that is out of bounds, if will no longer be passed to the filter above, but the invalid parts will be filled with silence.
* Documentation restructured and much has been updated.
* Added C-versions of conditional planedifference filters, for non-ISSE machines (not well tested though).


Changes from 2.52
-----------------


Additions
~~~~~~~~~

* Added audio support to DirectShowSource.
* Added seeking support to DirectShowSource.
* Added .GRF file loading to load filter graphs from GraphEdit. Be sure there is an open pin, to which AviSynth is able to connect, otherwise expect "the filter graph won't talk to me".
* Added optional coring=true/false to Levels and Tweak. both true by default, as previously.
* German documentation.
* Added DV type 1 video input, using AviSource(). Video only!
* DirectShowSource() is now capable of properly opening audio with more than 2 channels. Tested with AC3Filter.
* DirectShowSource() now accepts and properly decodes float-precision samples. Tested with AC3Filter.
* Added checks for samplerate and framerate in Splice.
* SwapUV(), UToY(), VToY() and YToUV() now also works in YUY2 mode.
* Added C-style plugin support (still in testing) to allow plugin writers to use other compilers than MSVC. See this thread for further info.
* Added Invert(), ShowAlpha().
* Added default parameters to ColorBars.
* Extensive updates of German documentation.
* DirectShowSource capable of opening audio only. WAV/AC3/MP3 can be opened using DirectshowSource.
* Added experimental "align=true/false" to crop - this will realign frames if they aren't already. Alignement is 16 for SSE2 machines, 8 for others.
* Added "Overall PSNR for Compare()" - thanks to temporance.
* ResampleAudio now accepts any number of channels.
* Added "after_frame=true/false" option to FrameEvaluate. This determines if the script should be evaluated before (default operation) or after the frame has been fetched from the filters above.


Bugfixes
~~~~~~~~

* Fixed memory leak in Plugin name export.
* Incorporated file lock fix in AviSource by WarpEnterprises.
* Minor fixes to field information in Info().
* Fixed occational hang in DirectShowSource (this might lead to leaks on unload/load!)
* Fixed crashbug in Compare, when logfile was specified.
* Fixed distortion on Crop(align=true).
* Fixed overflow in ResampleAudio (Thanks to IanB!). [Bug 770853].
* Fixed Mergechannel broken with more than 2 channels.
* Made adjustments for longer sample support in ResampleAudio.
* Fixed crash in ResampleAudio, if no audio was present.
* Fixed crash in MonoToStereo().
* Fixed: Normalize(show = true) displaying invalid value, and added a dB amplification indication.
* Fixed minor issues in audio routines with very long samples.
* Fixed wrong colors in ShowSMPTE YV12 mode.
* Corrected several performance problems in Limiter, YUY2 mode (thanks again ARDA!).
* Fixed YUY2 FlipHorizontal giving garbage/crashing.
* General Convolution now properly processes 5x5 matrices. Thanks IanB for the patch.
* Fixed minor stuff in TemporalSoften.
* (Hopefully) fixed precision in PointResize.


Optimizations
~~~~~~~~~~~~~

* Added heavily optimized memory copy mode, that will be used in some blits.
* Conditional unroll of fetch/unpack loop in dynamic compiled resizer. Now only unrolls if 1) Athlon 2) Source width < 512.


Changes
~~~~~~~

* Further clarification in Info() regarding field information.
* Minor changes to Limiter code (block prefetch).
* ApplyRange now accepts startframe = stopframe. This will only process the specified frame.
* ApplyRange now supports audio and processes it to the end of stopframe.
* Updated avisynth_c API to v0.14.
* Reenabled function name export for VDubMod syntax highlighting - I'm not sure if it had much effect on stability.
* Corrected ParseMultiplication so it parses from left to right.
* The default luma range in Limiter is corrected to 16-235 (it was 16-236).
* Temporarily disabled plugin function export for VDubMod. I'm suspecting this of the "crash+disappear" of VdubMod.
* Removed HSIAdjust().
* Removed ffvfw from installation.
* ShowSMPTE does no longer require fps parameter - only if the current fps cannot be used.

Changes from 2.51 beta
----------------------


Additions
~~~~~~~~~

* Added light version of "ffvfw" to the installer. No "Cannot locate decompressor (YV12)" messages.
* Added ConditionalFilter, that returns one of two sources based on an expression.
* Added conditional filters:
* * AverageLuma(), AverageChromaU(), AverageChromaV() functions. ''Returns a float from 0 to 255 based on the average pixel values of a plane.''
* * YDifferenceFromPrevious(), UDifferenceFromPrevious(), VDifferenceFromPrevious() and YDifferenceToNext(), UDifferenceToNext(), VDifferenceToNext()'''.
* * LumaDifference(clip,clip), ChromaUDifference(clip,clip), ChromaUDifference(clip,clip). ''They return a float value between 0 and 255 of the absolute difference.''
* * RGBDifference(clip1,clip2), RGBDifferenceFromPrevious(clip), RGBDifferenceToNext(clip).
* * YPlaneMax(clip, float threshold), YPlaneMin(clip, float threshold), YPlaneMedian(clip), YPlaneMinMaxDifference(clip, float threshold).
* * Threshold is a percentage, on how many percent of the pixels are allowed above or below minimum. The threshold is optional and defaults to 0. There are similar funtions for U and V.''
* Added ScriptClip(clip, string function, [show=true/false]). This will return the clip returned by the function evaluated on every frame.
* Added FrameEvaluate(clip, script) - Similar to ScriptClip, except the output of the filter is ignored. This can be used for assigning variables, etc.
* YV12 <-> RGB conversions now use an intermediate YUY2 conversion to achieve better chroma upsamplig. As a result of this ConvertToRGB now also take an "interlaced=true/false" parameter.
* Added ImageWriter.
* Added "show" parameter to ConditionalFilter. This will overlay the results on the screen.
* Added dynamic compiled limiter.
* Implemented Belgabors patch for exporting plugin functions.
* Build date is now (semi)automatically updated in version.
* Added script functions: IsYV12(clip), IsPlanar(clip), IsInterleaved(clip)
* Loads of documentation updates.


Bugfixes
~~~~~~~~

* Fixed cache hints a bit up.
* Hopefully fixed jumping frame bugs in temporalsoften.
* Fixed crashed in temporalsoften on some setups.
* Fixed I420 / YV12 mismatch in Interleave.
* Fixed problems with implicit last giving problem with multiple filter instances of ScriptClip/ConditionalFilter.
* Many ImageReader/Writer fixes and updates.
* Fixed bug when height > 512 in RGB -> YV12 conversion.
* AviSynth now mimics VDubs way of handling dropped frames to avoid problems with buggy codecs.
* Trim now returns (x-1) frames as supposed.
* Fixed stereo setting in BlankClip


Optimizations
~~~~~~~~~~~~~

* Added MMX RGB24->YUY2 conversion.
* Minor changes to existing RGB32 -> YUY2 MMX.
* Minor speedup to ISSE limiter.
* Added SoftWire dynamic compiled horizontal resizer. Approximately 10-15% faster - maybe even more on P4.


Changes
~~~~~~~

* AviSynth will now attempt to deallocate framebuffers, if memory usage is 25% above default values or SetMemoryMax().
* Improved rounding precision in ISSE YV12 <-> YUY2 conversions.
* Improved chroma upsampling quality in planar YV12 -> YUY2.
* Better chroma alignment on interlaced YUY2 ->YV12.
* Slightly better precision in audio conversion.
* ApplyRange now checks if size and colorspace are the same.
* Fixed float point exceptions being thrown in some applications, based on the CPU register settings. (Especially Delphi-based apps).


Changes from 2.50 beta
----------------------


Additions
~~~~~~~~~

* Added start/end parameters to ImageWriter.
* Added Chr, Time and Spline script functions.
* Added ISSE and MMX YUY2->YV12 conversions. Faster than XviD - and both interlaced and noninterlaced modes supported. Use ConvertToYV12(interlaced=true).
* Added rewritten YV12->YUY2 (progressive) conversion to avoid using buggy XviD conversion. It also has better quality as it properly interpolates chromas as opposed to simply copying it. Speed should be the same. Use ConvertToYUY2(interlaced=true).
* Added TurnLeft /TurnRight.
* YToUV() now takes an optional third parameter, that contains luma for the final clip.
* AudioLength() now returns the size in samples (do however beware of overflows on very long clips)
* Added ApplyRange.


Bugfixes
~~~~~~~~

* Fixed Splice problems with YV12 from different sources. Audio is now automatically converted to the same format.
* Fixed bug in YV12 stackvertical, causing corrupt images.
* Fixed memory exception problem in Blur.
* Fixed non-matching image type in Stack.
* Fixed rounding in Temporalsoften mode 2.
* Fixed crashbug in YUY2 mergechroma.
* Fix bug in C version of YUY2->RGB24.
* Fixed bug with uninitialized data in AviSource.
* Fixed wrong pitch being used in Temporalsoften scenechange - could in rare cases lead to unexpected results.
* Fixed rounding in some cases in Temporalsoften.
* Doesn't add audio to track, if there is no present in AssumeSampleRate().
* Fixed isBFF() and isTFF().


Optimizations
~~~~~~~~~~~~~

* Put in Steady's new BitBlt code, and enabled it for ISSE capable processors.
* Much "conservative" code removed. A general speedup should be expected.
* Made SwapUV faster.


Changes
~~~~~~~

* Removed all fieldbased/mod 4 checks.
* Restored how fieldbased/framebased works.
* Removed startup Box from installer.
* Greyscale inserts value 128, instead of 127. Some users have reported a greenish tint.
* Updated AssumeTFF and BFF to also write the information to VideoInfo.
* Updated icons.
* BMP support (output only) in ImageWriter.

Major changes from the 2.0.x line
---------------------------------

-   Native planar YV12 support.
-   Multiple audiochannels. Unlimited number of channels is now
    supported.
-   Float samples support. AviSynth is now capable of processing samples
    as floats.
-   Automatic sample conversion. If some filters doesn't support a
    specific sample type, they are converted to the format preserving most
    quality (most often floats).
-   Optimizations. Many basic features has been optimized, and now
    performs much better than previous versions.
-   Temporalsoften has a significant speed improvement, scenechange
    detection and a new improved blend mode.
-   Limiter can limit the YUV ranges, to avoid invalid color values and
    improve compression.
-   ColorYUV makes it possible to do very exact color corrections.
    ColorYUV has built-in auto-whitebalance and auto-gain features.
-   Select separate planes using UToY, VToY and merge them together
    again, using YToUV.
-   Fliphorizontal. implemented.
-   SelectRangeEvery is now part of the core functions.
-   Blur, Sharpen, Resize optimized.
-   Fast XviD colorspace conversions.
-   See clip info using the info() command.


Changes from 2.07
-----------------

- Fixed crashbug in resize.
- AviSynth now only includes ``"*.avsi"`` from the plugin directory
- Changed maximum number of arguments from 40 to 1024.
- Resampleaudio() caused crashes, if no audio was present.
- Fixed: Exist() didn't work.


Changes from 2.06
-----------------

- Script extensions:
- - ``LeftStr(string, int size), RightStr(string, int size), MidStr(string,
    int first, int length), FindStr(string, string), RevStr(string),
    StrLen(string)``
- - ``Sign(int), sign(float), Nop, Rand([int limit]), Select(index, item0
    [,item1...]), Exist(string filename)``
- - ``VersionNumber(), VersionString()``
- - ``IsRGB24(clip), IsRGB32(clip), Int(float), Frac(float), Float(int),
    Value(string), HexValue(string).``
- Strings can be compared using "<" ,">","<=",">=" operators (case insensitive).
- Color option for Fades , Letterbox, and Addborders.
- Subtitle alignment and spacing options, added y=-1 centering support (x=-1
  undocumented support remains) and defaults for x,y and align vary depending
  on each other's settings.
- Optimization for recent BlankClip() RGB24 bug fix.
- Fixed ceil, floor and round functions.
- Fixed BlankClip RGB24 with odd widths.
- Fixed DB scale off by 2x in volume.
- Added FadeIn / FadeIn2 to fade in video and audio.
- Added AssumeSampleRate to change the samplerate without resampling (yes,
  this will lead to desync!).
- Fixed one frame wrong offset in trim, when second argument is negative.
- Added abs(integer) and abs(float).
- Fixed '%' (mod) in scripts, so it returns absolute values.
- Added a boolean to DirectShowSource, so seeking can be manually disabled,
  if it works very slowly.
- Added PointResize() function. Resamples as "Nearest Neighbour" in VirtualDub.
- Added SetWorkingDir() function.
- Fixed Normalize crashes.
- Added German documentation.

Changes from 2.05
-----------------

- Fixed potential crashbug in Resize - real fix instead of previous hack.
- Default alignment is now 16 bytes, which should be faster on P4.
- Fixed bug in RGB32 greyscale, when with not divideable with 2.
- Added example scripts. - Added installer/uninstaller. No more need to
  fiddle with .reg files and regedit.
- Fixed Trim, so negative values works as documented.
- Fixes to plugin autoloading: - freezed when invalid dll encountered, was
  locking files unnecessarily, now also loads avisynth plugins with VDF
  extension.
- Made forward seeking fallback code in DirectShowSource files - it will not
  freeze anymore, just be very slow, since it has to decode all inbetween
  frames!
- Added LanczosResize which uses the Lanczos3 algorithm - it provides better
  quality than BicubicResize in many cases.
- Better rounding precision in BicubicResize/BilinearResize.
- Optimizations to YUY2 BicubicResize/BilinearResize.
- New ResetMask() filter: sets the mask to "all-opaque" (RGB32 only).
- ChangeFPS() now changes the framecount (thanks to Xesdeeni).
- AVI files, that contains an invalid first frame are now rejected - there is
  no way of handling this situation gracefully. Segmented AVI's automatically
  skip these segments, and will NOT return an error.
- Better parameter checks on Crop() and LetterBox().
- Fixed wrap at right side of picture in RGB
  BicubicResize()/BilinearResize().
- Fixed another Chroma wrap issue in BicubicResize()
- Added optional pixel_type parameter to AVISource and siblings to force a
  decompression format (YUY2, RGB32 or RGB24).
- Added optional left and right parameters to Letterbox - parameters are
  placed after the existing to preserve compatibility.
- Added EnsureVBRMP3Sync(clip), to avoid desync of VBR-compressed
  mp3-soundtrack. This will slow seeking down considerably, but is very useful
  when using trim() for instance. Always use before trim().
- BugFix: Fixed crashbug in Resize functions on some machines.
- AviSource always returns last valid frame on decompression errors.
- Added check for invalid frame 0 (mostly seen in SegmentedAviSource()).

Changes from 2.04
-----------------

- Additions to the Compare filter (Statistics over several frames)
- Reorder function loading to hopefully give plugins precedence over filters
- Added LowPassAudio(frequency) and HighPassAudio(frequency)
- Many updates and additions to documentation.
- New function: String(value) - converts any AVSValue to string.
- Bumped number of plugins to 50 from 30
- Avisynth now allocates minimum 16MB cache, or otherwise 1/4th of free
  physical memory.
- Plugin autoloading (Create string regkey:
  HKEY_LOCAL_MACHINE\Software\Avisynth\PluginDir) (still in alpha)
- Crash-on-exit bugfix in SegmentedAviSource.
- Audio-related bugfix in AVISource.
- Bugfix in FilteredResizeH (see bug [ 588402 ]) minor optimizations too.
- Bugfix: TemporalSoften
- Bugfix: dropped frame (introduced in 2.04).
- Fixed bug in Pow(x,y)

Changes from 2.03
-----------------

- Compressed audio support in AVISource, AVIFileSource and OpenDMLSource.
  Boolean parameter after clip turns it off.
- Compile fix in temporalsoften.

Changes from 2.02
-----------------

- Fixed EMMS bug in ColorKeyMask
- Fixed YUY2 detection issues in Layer
- Added friendly error message for Layer ops
- GetLeftChannel - Returns left channel.
- GetRightChannel - Returns right channel.
- MonoToStereo(clip1_left,clip2_right) - muxes two clips together as one
  stereo clip.
- MixAudio(clip_1,clip_2,float clip1_volume, float clip2_volume) Mixes two
  audio sources together.
- Added ConvertBackToYUY2() That only uses chroma from the left pixel to
  avoid shifting chroma color by multiple YUY2->RGB>YUY2 conversions.
  ConvertToYUY2 remains unchanged.
- Added Documentation for new parameters
- Added sound to Loop() function.
- New function ConvertToMono(clip) - convert stereo to mono.
- New function KillAudio(clip) removes audio from clip. Use this if you get
  crashes with compressed AVI sound.
- new filter: ColorKeyMask + bugfix in Layer
- New function Normalize(clip, float max_left, float max_right). Normalizes
  audio. Both floats are optional. If maximum values (0 to 1) are used, the
  stream will peak at this level, otherwise the peak will at full volume.
- TemporalSoften (MMX & C ) (previously released as plugin TemporalSoften2)
- MMX optimized HorizontalReduceBy2() in YUY2 mode.
- Corrected one pixel offset bug in VerticalReduceby2().
- MMX is now pixelperfect in compare with C implementations.
- Changed order in ReduceBy2() since VerticalReduceBy2 is faster.
- VerticalReduceBy2() has been MMX optimized. More than twice as fast. Filter
  checks if image is too small to be reduced.
- Colorbars() now also generates a test tone. Test tone is a 440Hz sine at
  48KHz, 16 bit, stereo. The tone pulses in the RIGHT speaker, being turned on
  and off once every second.
- ResampleAudio() now skips conversion, if samplerate is already at the given
  rate.
- No athlon codes in merge.
- Converttoyuy2 MMX optimized.
- Fixes in greyscale() in RGB mode.
- Optimized layer() and decomb-filters.
- Compare(clip filtered, clip original, string channels="", string
  logfile="", bool show_graph=true)
- AssumeTFF() and AssumeBFF() to force parity
- Documentation restructured and updated.

Please report bugs at `Sourceforge Avisynth2 page`_ - or - `Doom9's Avisynth Developer Forum`_

$Date: 2008/12/22 01:26:05 $

.. _Sourceforge Avisynth2 page:
    http://sourceforge.net/projects/avisynth2
.. _Doom9's Avisynth Developer Forum:
    http://forum.doom9.org/forumdisplay.php?s=&forumid=69
.. _vfr content in DSS:
    http://forum.doom9.org/showthread.php?s=&threadid=90938
