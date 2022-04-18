# FFmpegSource2 Changelog
- 2.3000
  - Added support for VapourSynth API4 (Myrsloik)
  - Added basic Avisynth+ frame property support (Myrsloik)
  - Added Rotation and Flip properties for VapourSynth (Myrsloik)
  - Added long path support for ffmsindex in windows (Myrsloik)
  - The audio gap fill logic is now optional and usually disabled by default (Myrsloik)
  - Allow the drc_scale option to be set when decoding audio (Myrsloik)
  - Allow the enable_drefs and use_absolute_path demuxer options to be used when indexing (Myrsloik)

- 2.40
  - Avisynth+ linux support (qyot27)
  - Added LastEndTime track property to make it possible to take the last frame's duration into account (Myrsloik)
  - Removed several deprecated functions and enums from the API (Myrsloik)
  - No longer assumes sub 1 fps framerates are invalid (Myrsloik)
  - Added support for floating point format output in VapourSynth and Avisynth+ (Myrsloik)
  - Fixed issue with dropped/repeated frames in vc1 with multiple b-frames after seeking (Myrsloik)
  - Fixed issue with dropped/repeated frames in h264 when the reorder buffer size is too small (Myrsloik)
  - Improved seeking in mpeg and mpegts streams (Myrsloik)
  - Added rgb(a)p and several additional formats as output to Avisynth+ (Myrsloik)
  - Added VP9 support (Daemon404)
  - Fixed incorrectly reporting the output as limited range when it's in fact unknown and likely to be full range (Myrsloik)
  - Added mastering display metadata output (Myrsloik)
  - VapourSynth source now defaults to not outputting alpha (Myrsloik)
  - Removed the now unused demuxer, dumpmask, audiofile and utf8 arguments from the source filters (Myrsloik)
  - Removed ability to dump audio tracks (Myrsloik)
  - Fixed incorrect colorimetry metadata reported when converting the output to another colorspace (Myrsloik)
  - Sources now simply reference the index instead of copying large parts of it (Myrsloik)
  - Use new FFmpeg decoding API (Myrsloik)
  - Fixed several bugs in output format selection (Myrsloik)
  - FFMSIndex will now properly error out with invalid arguments (Myrsloik)
  - Add rotation metadata export (Myrsloik)
  - Add stereoscopic metadata export (Myrsloik)
  - Created new Visual Studio 2017 projects (Myrsloik)
  - Removed old mingw version support (Myrsloik)
  - Removed support for old FFmpeg versions (Myrsloik)
  - Removed libav support (Myrsloik)
  - Discontinuous Timestamp Support (Daemon404)
  - Add FFMS_Deinit (Daemon404)
  - Fix mid-stream parameter changes (Daemon404)
  - Fix decoding H.264 PAFF files with one field per packet (Daemon404)
  - Fix delay calculations for H.264 PAFF files (Daemon404)
  - Fix frame rate calculation for fields with one field per packet (Daemon404)

- 2.23
  - Updated FFmpeg APIs used (Daemon404)
  - Added new API that can read/write indexes to a memory buffer (Daemon404)
  - Added support for high bitdepth output in Avisynth+ (Myrsloik)
  - vapoursource: Add nfMakeLinear flag to better performance in most cases (Myrsloik)
  - Remove bad colorspace matrix guessing for YUV (Myrsloik)
  - Tell libavformat to discard packets we don't want (Daemon404)
  - Restore original FFCopyrightInfringement behavior (Myrsloik)
  - Make FFMS_Init threadsafe (Daemon404)
  - Support latest libav* APIs (Daemon404)

- 2.22
  - Fix possible off by 1 frame until first seek issue introduced in 2.21 (Myrsloik)
  - avisynth: Make FFMS2 a shorter alias for FFmpegSource2 to save some typing (Myrsloik)
  - avisynth: Moved all source functions in ffms2.avsi into the actual plugin (Myrsloik)
  - avisynth: Use 2.6 RC API and add output support for all new colorspaces (Myrsloik)
  - vapoursource: Fix swapped RGB channels bug introduced in 2.21 (Myrsloik)
  - swscale: Drop pointless and slow bitexact flag from resizer settings (kodabb)
  - Add support for files with gaps between the audio packets (Plorkyeran)

- 2.21
  - Add missing constants for log levels (Myrsloik)
  - Fix bad decoder flush that discards the palette of raw video (Myrsloik)
  - Remove the SWScale function from Avisynth since SWScale is useless (Myrsloik)
  - Fix common framerate correction (Myrsloik)
  - vapoursource: The framerate and frame durations are now normalized (Myrsloik)
  - vapoursource: Add support for outputting the alpha channel too (Myrsloik)
  - vapoursource: Fix resizer selection so bicubic isn't always used (Myrsloik)
  - vapoursource: Set chroma location and full colorspace information (Myrsloik)
  - Add support for indexing files with more than 32 tracks and the possibility to skip indexing of video tracks (Myrsloik)
  - The Avisynth plugin now sets all per frame variables in all modes to avoid ugly and slow FFInfo() errors (Myrsloik)
  - vapoursource: Fix crash on single frame files (Myrsloik)
  - Remove the Matroska and Haali demuxers (qyot27, Plorkyeran)
  - Fix adjustments for positive audio delay (tophf)
  - Don't try to index video tracks in unsupported formats (Daemon404)
  - Don't use scientific notation in timecode files (Daemon404)
  - Fix utf8 support in FFVideoSource (nixxquality)
  - Improve handling of files where only some packets have timestamps (Plorkyeran)
  - Fix memory leaks in the VapourSynth error handling (Plorkyeran)

- 2.20
  - Add support for Opus in MKV when ffmpeg/libav are built with libopus (qyot27)
  - vapoursource: Provide _AbsoluteTime metadata (Daemon404)
  - Fix a pile of small things found by Coverity (Daemon404)
  - Add support for HEVC in MKV (qyot27)
  - Fix infinite loop on garbage data at the beginning of AAC files (Plorkyeran)
  - Deal with the never-ending bitrot from FFmpeg/Libav API changes (Daemon404, Plorkyeran)
  - Make indexes not arch-specific and trivially smaller (Plorkyeran)
  - Fix bug where the first b-frame was replaced with the frame before it with open-gop h.264 (Plorkyeran)
  - Fix seeking issues with open-gop h.264 (Plorkyeran)
  - Fix corruption when seeking in interlaced h.264 (Plorkyeran)
  - Add support for multiple frames per audio packet as FFmpeg seems to have started doing that (Plorkyeran)
  - Fix seeking in Y4M files (Plorkyeran)
  - Fix dumping audio while indexing files
  - Add support for Matroska files with extremely large CodecPrivate sections
  - Speed up indexing Matroska files a bit

- 2.18
  - Fix regression (r483) with rffmode that caused it to error out even if using the default output colorspace. (TheRyuu)
  - High(er) quality YUV->RGB conversion. (TheRyuu)
  - Fix indexing on files with cover art. (TheRyuu)
  - Add support for libav/ffmpeg built with msvc, this is the default on windows when building with msvc. (TheRyuu)
  - Remove postproc support. (TheRyuu)
  - Deprecate the CPUFeatures argument to FFMS_Init since postproc was the only thing still using it. (Plorkyeran)
  - Added VapourSynth support. (Myrsloik)
  - ffmsindex can now output keyframe numbers to a file while indexing. (Plorkyeran)
  - configure now defaults to building a shared library, except when building MinGW/Cygwin, since you usually want static for those. (Plorkyeran)
  - The source color space and color range used when converting with swscale can now be overridden. (Plorkyeran)
  - Fix issues with unicode filenames when building with mingw. (Plorkyeran)
  - Fix progress reporting when indexing files with non-zero initial timestamp with haali's splitter. (Plorkyeran)
  - Add support for formats with packet durations but no packet timestamps. (Plorkyeran)
  - Fix corruption when seeking in VC-1 in MKV. (Plorkyeran)
  - Fix bug that resulted in files opened with Haali's splitter sometimes always decoding from the beginning on every seek. (Plorkyeran)
  - Add support for VP8. (Plorkyeran)
  - Fix crash when indexing video formats with no parser. (Plorkyeran)
  - Fix compilation errors with recent versions of libav/ffmpeg. (Plorkyeran)
  - Fix NVOP handling with frame-based threading (aka zero-size frames with mp4 bug). (Plorkyeran)
  - Add support for vc1image. (Plorkyeran)
  - Use the container SAR when the codec SAR is unset when opening via lavf. (Plorkyeran)
  - Actually set the ColorRange and ColorSpace of frames when nothing has been overridden. (Plorkyeran)
  - Add support for files without timestamps to lavf audio. (Plorkyeran)
  - Fix handling of audio delay with invalid inital timestamps. (Plorkyeran)
  - Sort of partially fix interlaced H.264. (Plorkyeran)
  - Fix errors when the client asks for audio past the end of the file. (Plorkyeran)
  - Fix rounding error with MKV timestamps that resulted in things getting a FPS like 60001/1001. (TheRyuu)
  - Bump required version to libav 0.8/FFmpeg 0.9. (Plorkyeran)
  - Switch to avcodec_decode_audio4. (Plorkyeran)
  - Add support for planar audio from lavc. (Plorkyeran)
  - Add SetOutputFormatA for audio resampling/mixing using libavresample. (Plorkyeran)
  - Zero-length index files are now rejected rather than bad things happening.

- 2.17
  - Reworked color matrix and color range handling a bit, which fixed a bug that could cause FFMS2 to always output TV range even if the input was full range. (TheFluff)
  - The autotools build system can now create debug builds properly. (Daemon404)
  - Deprecated parts of the API will now cause compiler warnings when you use them. (TheFluff)
  - Added a `FFMS_GetVersion` function to the API (lets library users get the version number at runtime) and exposed it in Avisynth as `FFGetVersion`. (TheRyuu, TheFluff)
  - Added a variable prefix option to the Avisynth functions. Its primary purpose is to get subsequent calls to source functions from overwriting variables from earlier calls. (TheFluff)
  - Make it possible to open single-frame videos without explicitly setting seekmode to -1 for you weird people who want to open images with ffms (Plorkyeran)
  - Fixed bug where indices would sometimes be incorrectly considered valid (TheRyuu)
  - Add support for recent versions of Libav/FFmpeg built as shared libraries (Plorkyeran, TheRyuu, Kovensky)
  - When possible, non-API symbols are no longer exported (Daemon404, TheFluff)
  - Deprecate postprocessing support. Libav and FFmpeg are planning on removing it at some point in the near future and it's really not very useful.
  - Fix the pkg-config version on OS X (Plorkyeran).
  - Fixed a bug that could cause the `FFmpegSource2()` Avisynth function to not use UTF8 filenames even when told to do so. (pandv2)
  - Fixed a few minor memory leaks. (Plorkyeran)
  - Adjusting audio delay relative to the first video track should now work properly again (was broken in 2.16). (Plorkyeran)
  - General bitrot fixes to deal with changes in Libav/FFmpeg (everyone)
  - Corrected handling of codec private data when using a non-libavformat parser. Fixes decoding of FFV1 and UTVideo in MKV, among other things. (TheFluff)
  - Bump minimum required version of FFmpeg to 0.6.

- 2.16
  - Reimplemented output colorspace selection, this should fix all issues with the Avisynth plugin when opening yuv420p10 or yuv444 material plus several other less common cases. (Myrsloik)
  - Added `FFMS_SetOutputFormatV2` to the API. This function allows you to specify PixelFormats >= 64 for use as output. (Myrsloik)
  - Fixed a serious bug that could cause crashes and completely useless index files with h264 in Matroska. (Myrsloik)
  - Automatically detect number of decoding threads.  The Avisynth video source funtion already did this, now moved so the API can use it as well. (TheRyuu) 
  - Re-add the ability to target x64 with MSVC, since it's a bit more sane now. (TheRyuu)
  - Fixed a bug that could cause crashes when reading audio if FFMS2 was compiled with GCC. (Myrsloik)
  - ffmsindex will no longer crash if it cannot open the file given to it for indexing. (TheFluff)
  - FFMS2 will no longer crash if the video decoder feeds it an empty frame (can sometimes happen when using lots of decoder threads); you'll get a nice error message instead. (TheFluff)
  - The C-plugin can now act as both an Avisynth 2.6 plugin (including support for new colorspaces) as well as an Avisynth 2.5 one, in the same binary. (kemuri_-9)
  - Fixed an issue that could cause opening Vorbis audio to fail because FFMS2 couldn't find an initial valid PTS. (TheFluff)
  - FFMS2 will no longer crash if forced or tricked into using an index file generated by a FFMS2 version compiled for a different architecture. (TheRyuu)
  - Fixed a crash when the last frame was requested using the Avisynth plugin's forced CFR mode. (Plorkyeran)
  - Fixed various issues with decoding audio from the Ogg container without Haali's splitter. (Myrsloik, TheFluff)
  - Fixed the "invalid postprocessing settings", it is caused by a parsing bug in libpostproc and a workaround has been added (Myrsloik)
  - Tinkered a bit with the non-MSVS build system. (Daemon404, Kovensky)

- 2.15
  - FFVideoSource and FFAudioSource will now automatically reindex and overwrite the index file if it doesn't match the file being opened and the `cachefile` argument is left as the default. (Plorkyeran)
  - FFMS2 can now be used to decode Lagarith, but note that libavcodec's decoder is very experimental at the moment. (Plorkyeran)
  - SWScale can now use SSE2 optimizations for certain operations if your CPU supports it. (kemuri_-9)
  - Fixed a bug that could cause SWScale initialization to fail. (kemuri_-9)
  - Fixed a bug that could cause index files to never be considered valid, forcing a reindexing every time a script was loaded. (TheRyuu)
  - Trying to use postprocessing on a fullrange YUV clip will no longer cause errors. (TheFluff)
  - Fixed a few random decoding bugs related to unaligned memory or buffers that were not initialized properly. (TheFluff)
  - It is now possible to force FFMS2 to use a specific demuxer instead of letting it pick one automatically. (TheFluff)
  - When converting YUV to RGB, FFMS2 will now try to actually use the correct color coefficients rather than assuming everything is bt470bg. (Plorkyeran)
  - Moved support for container-level audio delay from the Avisynth plugin to the core and exposed it in the API (Plorkyeran)
  - Audio decoding has been substantially reworked. Linearly decoding audio now almost always works correctly and seeking is now actually sample-accurate for many formats. (Plorkyeran)
  - It is now possible to build 64bit versions of the plugin for use with Avisynth (and whatever else) from MSVC by means of black magic (this probably only works when the planets are aligned, also 64bit builds might require msvcr90.dll). (TheRyuu)
  - The Avisynth plugin now supports UTF8 filenames; ffmsindex.exe also supports Unicode filenames. FFMS_USE_UTF8_PATHS is now a runtime option instead of a compile-time one. (TheFluff)
  - The FFInfo() function (supplied by ffms2.avsi) will now round timestamps to nearest millisecond instead of truncating them. It's also been cleaned up in general and no longer relies on global variables. (Gavino)
  - Containers opened with libavformat will now report a framerate based on the average frame duration instead of the duration of the first frame, just like Matroska files and files opened with Haali's splitter does. Should fix CFR framerates being reported incorrectly in dumb containers like FLV. (TheFluff)
  - PC/TV luma range (16-235 versus 0-255) detection should now be a bit more reliable. (TheFluff)
  - Fixed a crash when opening files with Unicode filename support enabled. (Plorkyeran)

- 2.14
  - Reworked filename handling a bit. Index files should no longer get a garbled name when using the Avisynth plugin and an input filename in the local codepage (issue 9), and FFMS_USE_UTF8_PATHS does no longer require patching ffmpeg. (TheFluff, nielsm)
  - Fixed a bunch of compiler warnings and added versioning for the shared library when building under Unix. (Kovensky)
  - Fixed an invalid memory access bug that could cause random crashes or other errors when opening files. (TheFluff)
  - The timebase for video tracks is now corrected if it's invalid. (kemuri_-9)
  - Fixed a number of multithreaded decoding issues. (kemuri_-9)
  - Use aligned memory for audio decoding buffers; fixes some crashes during audio decoding. (greg)
  - Fixed a bug that caused ffmsindex -c to fail. (chdheu)
  - Added support for MKV files using header stripping compression. (TheFluff)
  - It is now possible to compile FFMS2 using a ffmpeg without libpostproc compiled in (this will obviously cause postprocessing-related functions to fail). Hence it is now possible to build a GPL-free FFMS2, since the rest of ffmpeg can be compiled as LGPL. (TheFluff)
  - Audio streams that change channel layout, sample rate or bitdepth mid-stream will no longer cause crashes; an error will be raised during indexing instead. (TheFluff)
  - Fix a potential crash when ffmpeg thought it couldn't decode a certain format, but was wrong. Fixes issues with some FLV's. (Plorkyeran)
  - Fix heap corruption that could cause crashes and odd issues when opening H.264 in MPEG-TS with Haali's splitter. (Plorkyeran)
  - Fix a memleak when decoding H.264 in MPEG-TS using Haali's splitter. (kemuri-_9)
  - Updated FFmpeg to rev 25329.

- 2.13
  - Fixed a bug where the Avisynth plugin would drop the last frame when converting to CFR (lanwcp)
  - The Avisynth plugin will now attempt to detect and use NTSC fractional framerates for CFR files if applicable, instead of reporting something based on the average framerate (astrange, TheFluff)
  - The errorhandling parameter to FFIndex() will now actually do what the documentation claims it does (TheFluff)
  - Fixed a progress reporting crash when opening images (TheFluff)
  - Replaced the CMake build system with an autotools one that hopefully works better (Kovensky)
  - Fixed various compilation issues with MinGW (TheFluff)
  - Fixed h264 in mkv which was remuxed from bd sources, there are no longer decoding artifacts after seeking
  - Use pts instead of dts for seeking with lavf, fixes various timecode problems on mp4 files with certain obscure delay headers
  - Now compresses the index files with zlib to save space
  - Removed the boost dependency
  - Fixed the framerate calculation for AVC video in the RFF modes
  - Improved/fixed the NVOP detection in MPEG4 and how timestamps are calculated (lanwcp, Myrsloik)
  - Fixed an issue where the first 15 audio frames would never be redecoded if needed again
  - Fixed/added support for uncompressed video and audio formats in matroska, fixes Blank.Clip.720p.x264-darkhold.mkv (Emess)
  - The indexer and test is now statically linked so no extra runtimes are needed
  - FFmpeg is now compiled with runtime cpu detection, allows opterons and old cpus to work properly
  - Updated FFmpeg to rev 21782

- 2.12
  - Added support for (L)PCM in TS/PS
  - Fixed some possible memory corruption on initialization in the indexers
  - Improved the API slightly by making resolution and colorspace frame properties only like they actually are
  - Fixed adjustdelay=-1 when no video track is present, previously it would crash
  - Updated FFmpeg to rev 19824, FFmpeg-mt unchanged at 24/08/2009 - with faad2, opencore-amr nb/wb, and pthreads

- 2.11
  - Now 100% less beta
  - FFMSIndex can now write timecode files
  - Optimized index reading and writing, file opening should be faster especially for big files
  - Fixed a crash bug on zlib compressed streams in matroska
  - Added an argument to FFAudioSource that makes it automatically adjust the delay relative to the first video track (default), the specified track (any track type possible) or relative to the zero timestamp, no adjustment is done for files with no video track in the default mode
  - Now exports the colorimetry info in a format easily usable with ColorMatrix, see FFCOLOR_SPACE and FFCOLOR_RANGE in the manual, the reported information may be unreliable
  - Added an FFImageSource() function to ffms2.avsi which invokes FFVideoSource with the optimal settings for use on images
  - Now exports the actual frame presentation time in FFVFR_TIME, added the general information function FFInfo() and FFFormatTime() functions to ffms2.avsi in addition to fixing the missing backslash
  - The seek/read method has been changed in the matroska video source to only need the information in the index and nothing more, this matches the audio source and should make seeking faster in some cases
  - No longer drops the last audio frame
  - It is now possible to force indexing of broken tracks when using FFIndex and from the API, this can in many cases help when the last frame of an avi file is corrupted which would otherwise prevent indexing of all working data
  - Fixed a bug that would prevent the SWScale and FFPP filters from working because of randomly added flags
  - Added support for RFF flags, the three possible modes should be equivalent to the ones found in DGDecode
  - Added a workaround for mpeg4 files with NVOPs
  - Fixed a bug where the first frame would be corrupted if the colorspace/dimensions were set for the output
  - Now VC1 in TS decoding works in many cases
  - Added the missing install target to CMake and changed the default output library name (Kovensky)
  - Now reverts to using LAVF if Haali's splitter isn't installed (note that LAVF is VERY bad at seeking in these formats)
  - Now requires the boost headers (but not libraries) to compile because boost::format is so convenient
  - More API changes to return proper error codes and more source cleanups
  - Updated FFmpeg to rev 19776, FFmpeg-mt to 24/08/2009 - with faad2, opencore-amr nb/wb, and w32threads (pthreads for FFmpeg-mt)

- 2.00 beta 10
  - Possibly fixed h264 in transport stream decoding, now a suitable bitstream filter is added (could theoretically break other h264 in ts files so make sure to report if any such files stop working)
  - Now returns the frame type as a character to make presentation easier and to remove the random numbers from the API
  - Fixed the identification of certain codecs (like VC1) in some transport streams
  - Fixed a bug introduced in SeekMode=1 by the changes in beta 6, prevents an access violation from happening in very rare conditions
  - Some (but not all) stream properties are now retrieved from the container and passed to libavcodec, this makes a few more formats work like vc1 in mkv/ts
  - It is now possible to specify the desired output colorspace and resolution in FFVideoSource, this is mostly useful for file where the resolution changes to avoid excessive up/downscaling
  - Added support for files where the video frames have changing size and colorspace
  - The library part can now be compiled to accept UTF8 encoded strings, this allows full unicode support on windows when FFmpeg is compiled with a small patch applied (TheFluff)
  - Added proper detection of when the opened file and index is mismatched and throws an error when it happens instead of undefined behavior, should not add a noticable delay to file opening
  - Added a proper cmake based build system and vs2008 project files to make developement and use easier (TheFluff, JEEB, Kovensky)
  - Added API documentation by TheFluff and revised many argument descriptions to reflect how they currently work
  - The output audio filename can now be specified in avisynth (was ignored previously) with variables such as track number and delay possible to use
  - Indexing with Haali's splitters should now show progress in most cases
  - Fixed a few more memory leaks in indexing
  - Now checks for failed seeking when LAVF is used and retries with more aggressive seeking options before failing
  - Updated FFmpeg to rev 19479 (now compiled with the opencore amr decoder)

- 2.00 beta 9
  - Dumping audio now actually implies indexing too in FFIndex, previously nothing would be done if the index mask wasn't set as well
  - FFAudioSource will now first load the index and returns the first indexed audio track with track=-1, if no audio tracks are indexed or the chosen track isn't indexed the equivalent of FFIndex(indexmask = -1, overwrite = cache) is executed first
  - Codec lookup for non-lavf opened files now to a large part use the same lookup tables as lavf, this should improve the number of properly recognized codecs
  - Now uses the average framerate for files opened with Haali's splitters, before it was always reported as 30 fps
  - Implemented audio decoding using Haali's splitters, FFAudioSource now works on ts, ps and ogm
  - Can now be compiled with ICL 10.1 (probably other versions too)
  - How indexing works has been split internally so the track numbers and types are reported, this makes it possible to create an interactive GUI or ask which audio tracks are to be indexed
  - Now has stricter index checking to detect when different FFmpeg versions were used to create an index of the same version
  - Fixed memory leaks when audio sources were destroyed and when errors happened during indexing
  - Fixed access violations occurring when a track of the wrong type was specified or didn't exist in FFVideoSource and FFAudioSource
  - Fixed access violations occurring when unindexed or empty audio tracks in matroska/lavf read files were opened
  - Less type conversion/signedness warnings
  - When audio track dumping is performed a custom callback can now be supplied to name the tracks
  - The audio track delay is now exposed in the API in the same way as video tracks
  - A big type and argument name cleanup in the API, many things have been renamed to be clearer and ffms.h should be completely C friendly now
  - Removed FFNoLog and replaced it with FFSetLogLevel and FFGetLogLevel, the default logging is now also set to quiet, the magical numbers to supply it can be found in avutil/log.h
  - Updated FFmpeg to rev 18972 (now with faad2 again by popular demand, updated to GCC 4.4.0 for compiling all libraries)

- 2.00 beta 8
  - Improved the audio decoding quality a lot by adding a simple cache, no more seeking is done when playing a file linearly and pops and other artifacts should be much more uncommon
  - Fixed a bug that would most of the time drop frame 0 and sometimes frame 1
  - Updated Haali's matroska parser code to the latest version
  - Updated FFmpeg to rev 18774

- 2.00 beta 7
  - Using ffms2 as a library no longer requires an installed pixfmt.h from libavutil, it is however still required to compile ffms2 and the avisynth plugin part
  - Fix a crash bug at the end of files with b-frames in beta 6 caused by uninitialized null packets
  - Includes TheFluff's wrapper function for 1.21 style syntax
  - Added a simple regression test application to the source
  - Removed a few pointless functions from the API
  - Fixed the accessing of codecprivate data with Haali's splitters
  - Timecode output should be fixed to include decimals AND not be in scientific format
  - Fixed a memory leak when using Haali's splitters
  - Updated FFmpeg to rev 18717

- 2.00 beta 6
  - Haali's splitters have been improved for video and now have audio dumping during indexing implemented
  - SeekMode=1 has improved logic which will make it go back and decode more frames if necessary to figure out where it is, in theory SeekMode=0 should now be mostly obsolete
  - Haali's splitters are now used to open mpeg ps and ogm in addition to mpeg ts, only ogm is frame accurate at this time
  - Negative timecodes and other bugs caused by an integer overflow fixed
  - Updated FFmpeg to rev 18442 (once again compilation fixes for the changes)

- 2.00 beta 5
  - FFMSIndex should now print the progress properly when another application reads its output
  - Added missing variables and explanations to the manual
  - Can now directly be compiled as a library for use in *nix
  - Fixed the missing decimals in saved timecode files

- 2.00 beta 4
  - Added the function FFNoLog which suppresses all messages from ffmpeg
  - Experimental new TS parsing using Haali's splitter (with bugs)
  - Everything is now compiled with VS2008 and GCC 4.3.2
  - Updated FFmpeg to rev 16383 (no libfaad2 this time)

- 2.00 beta 3
  - Compiled with libfaad2 again (has anyone seen a single aac file lavc can open right now?)
  - More API changes (and even more are likely to come)
  - Several access violations and memory leaks on opening and indexing files fixed
  - Added a VFR to CFR mode
  - Readded FFAudioSource support for other containers (glitches still present now and then but no separate raw cache is required and possibly less buggy)
  - Renamed the dll to FFMS2.dll, FFMS2 is now the official short name of the project
  - Updated FFmpeg to rev 15522

- 2.00 beta 2
  - More API changes (and more are likely to come)
  - Includes a simple CLI indexing application
  - FFIndex now takes a few more arguments
  - Readded FFAudioSource (only matroska supported for now)
  - Updated FFmpeg to rev 15396

- 2.00 beta 1
  - Can now be used as a stand alone library for making indices and retrieving frames
  - Rewrote most things
  - Updated FFmpeg to rev 15301