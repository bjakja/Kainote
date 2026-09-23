cmake_minimum_required(VERSION 3.21)
project(KainoteLinux LANGUAGES C CXX)
if(POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)
endif()
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC OFF)
find_package(PkgConfig REQUIRED)
# Windows builds the pinned Thirdparty/wxWidgets 3.3.3 submodule. Use the same
# source/version on Linux instead of silently accepting the host's wxGTK 3.2.
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(KAINOTE_WX_DEFAULT_CONFIG "${CMAKE_SOURCE_DIR}/build-wx-linux-debug/prefix/bin/wx-config")
else()
    set(KAINOTE_WX_DEFAULT_CONFIG "${CMAKE_SOURCE_DIR}/build-wx-linux/prefix/bin/wx-config")
endif()
set(KAINOTE_WX_CONFIG "${KAINOTE_WX_DEFAULT_CONFIG}"
    CACHE FILEPATH "wx-config from the pinned wxWidgets 3.3.3 Linux build")
if(NOT EXISTS "${KAINOTE_WX_CONFIG}")
    message(FATAL_ERROR
        "Pinned wxWidgets 3.3.3 is missing at ${KAINOTE_WX_CONFIG}. "
        "Run Thirdparty/build-wxwidgets-linux.sh (with --debug for Debug) first, or set KAINOTE_WX_CONFIG "
        "to a wx-config for wxWidgets 3.3.3.")
endif()
execute_process(
    COMMAND "${KAINOTE_WX_CONFIG}" --prefix
    OUTPUT_VARIABLE KAINOTE_WX_PREFIX
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE KAINOTE_WX_CONFIG_RESULT)
if(NOT KAINOTE_WX_CONFIG_RESULT EQUAL 0)
    message(FATAL_ERROR "Could not query ${KAINOTE_WX_CONFIG} for its wxWidgets prefix.")
endif()
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(KAINOTE_WX_EXPORT_CONFIG debug)
else()
    set(KAINOTE_WX_EXPORT_CONFIG release)
endif()
if(NOT EXISTS "${KAINOTE_WX_PREFIX}/lib/cmake/wxWidgets-3.3/wxWidgetsTargets-${KAINOTE_WX_EXPORT_CONFIG}.cmake")
    message(FATAL_ERROR
        "${KAINOTE_WX_CONFIG} does not point to the pinned wxWidgets 3.3.3 ${KAINOTE_WX_EXPORT_CONFIG} build. "
        "Run Thirdparty/build-wxwidgets-linux.sh (with --debug for Debug) first.")
endif()
set(wxWidgets_CONFIG_EXECUTABLE "${KAINOTE_WX_CONFIG}" CACHE FILEPATH
    "wx-config selected for Kainote" FORCE)
find_package(wxWidgets 3.3.3 EXACT REQUIRED COMPONENTS core base adv aui html xml gl stc net)
include(${wxWidgets_USE_FILE})
pkg_check_modules(LIBASS REQUIRED IMPORTED_TARGET libass)
pkg_check_modules(FFMS2 REQUIRED IMPORTED_TARGET ffms2)
pkg_check_modules(LUA REQUIRED IMPORTED_TARGET luajit)
pkg_check_modules(HUNSPELL REQUIRED IMPORTED_TARGET hunspell)
pkg_check_modules(UCHARDET REQUIRED IMPORTED_TARGET uchardet)
pkg_check_modules(LIBCURL REQUIRED IMPORTED_TARGET libcurl)
pkg_check_modules(ICU_UC REQUIRED IMPORTED_TARGET icu-uc)
pkg_check_modules(ICU_I18N REQUIRED IMPORTED_TARGET icu-i18n)
pkg_check_modules(FFMPEG REQUIRED IMPORTED_TARGET libavformat libavcodec libavutil)
pkg_check_modules(FONTCONFIG REQUIRED IMPORTED_TARGET fontconfig)
# GStreamer backs the Linux general-playback renderer (RendererGStreamer): playbin
# decodes/plays the file (video + audio in one pipeline), an appsink delivers BGRA
# frames the app composites (libass subtitles + progress bar) and presents through
# the shared wx paint path, and gstreamer-audio provides linear volume.
pkg_check_modules(GSTREAMER REQUIRED IMPORTED_TARGET gstreamer-1.0 gstreamer-video-1.0 gstreamer-audio-1.0 gstreamer-app-1.0)

# Link Boost statically on Linux so the built binary does not depend on the
# exact distro Boost shared-library SONAME, e.g. libboost_filesystem.so.1.83.0.
# Other system libraries remain dynamically linked through pkg-config/wxWidgets.
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost REQUIRED COMPONENTS filesystem locale regex OPTIONAL_COMPONENTS system thread)
find_program(MSGFMT_EXECUTABLE msgfmt REQUIRED)
find_package(Python3 COMPONENTS Interpreter REQUIRED)
find_program(TAR_EXECUTABLE tar REQUIRED)
find_program(GZIP_EXECUTABLE gzip REQUIRED)

# Thirdparty/ffms2 is a git submodule (Kainote's ffms2 fork).  The Linux build
# does not link it -- it links the distribution's libffms2 -- but it does
# compile Thirdparty/Build/FFMS2/indexing_additional.cpp against ffms2's private
# headers, so the checkout has to be there.  Fail with something actionable
# rather than a pile of missing-include errors.
if(NOT EXISTS "${CMAKE_SOURCE_DIR}/Thirdparty/ffms2/src/core/indexing.h")
    message(FATAL_ERROR
        "Thirdparty/ffms2 is empty -- the git submodules have not been checked out.\n"
        "Run:  git submodule update --init --recursive")
endif()

add_compile_definitions(_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING _CRT_SECURE_NO_WARNINGS HAVE_DLL_EXPORT LINUX_BUILD KAINOTE_LINUX_BUILD)
add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-include> $<$<COMPILE_LANGUAGE:CXX>:${CMAKE_SOURCE_DIR}/Kainote/platform.h> -Wno-deprecated-declarations -Wno-narrowing -Wno-unused-result)
add_executable(kainote
    Kainote/Demux.cpp
    Kainote/DialogueTextEditor.cpp
    Kainote/ModificationChecker.cpp
    Kainote/KainoteFrame.cpp
    Kainote/Notebook.cpp
    Kainote/AutomationDummyLoader.cpp
    Kainote/TagFindReplace.cpp
    Kainote/VisualAllTags.cpp
    Kainote/AudioBox.cpp
    Kainote/AudioDisplay.cpp
    Kainote/AudioDeviceEnumeration.cpp
    Kainote/AudioPlayerDSound.cpp
    Kainote/AudioSpectrum.cpp
    Kainote/Automation.cpp
    Kainote/AutomationDialog.cpp
    Kainote/AutomationFileSystem.cpp
    Kainote/AutomationHotkeysDialog.cpp
    Kainote/AutomationLPeg.c
    Thirdparty/luabins/src/fwrite.c
    Thirdparty/luabins/src/load.c
    Thirdparty/luabins/src/luabins.c
    Thirdparty/luabins/src/luainternals.c
    Thirdparty/luabins/src/save.c
    Thirdparty/luabins/src/savebuffer.c
    Thirdparty/luabins/src/write.c
    Kainote/AutomationProgress.cpp
    Kainote/AutomationScriptReader.cpp
    Kainote/AutomationToFile.cpp
    Kainote/AutomationUtils.cpp
    Kainote/AutoSavesRemoving.cpp
    Kainote/BidiConversion.cpp
    Kainote/BitmapButton.cpp
    Kainote/CsriMod.cpp
    Kainote/ConfigConverter.cpp
    Kainote/Context.cpp
    Kainote/ContextVulkan.cpp
    Kainote/ContextDX9.cpp
    Kainote/DshowRenderer.cpp
    Kainote/DummyVideo.cpp
    Kainote/FontCatalogList.cpp
    Kainote/KaiGauge.cpp
    Kainote/KaiPanel.cpp
    Kainote/LineParse.cpp
    Kainote/Playback.cpp
    Kainote/Provider.cpp
    Kainote/ProviderDummy.cpp
    Kainote/ProviderFFMS2.cpp
    Kainote/RendererFFMS2.cpp
    Kainote/RendererGStreamer.cpp
    Kainote/FileTypes.cpp
    Kainote/JsonValue.cpp
    Kainote/UpdateChecker.cpp
    Kainote/Registry.cpp
    Kainote/RendererDirectShow.cpp
    Kainote/RendererVideo.cpp
    Kainote/AutoSaveOpen.cpp
    Kainote/ShiftTimes.cpp
    Kainote/colorspace.cpp
    Kainote/config.cpp
    Kainote/findreplace.cpp
    Kainote/FindReplaceDialog.cpp
    Kainote/FindReplaceResultsDialog.cpp
    Kainote/FontEnumerator.cpp
    Kainote/GFFT/GFFT.cpp
    Kainote/GraphicsD2D.cpp
    Kainote/HotkeysNaming.cpp
    Kainote/KaiCheckBox.cpp
    Kainote/KaiDialog.cpp
    Kainote/KaiFrame.cpp
    Kainote/KaiListCtrl.cpp
    Kainote/KaiMessageBox.cpp
    Kainote/kainoteApp.cpp
    Kainote/KaiRadioButton.cpp
    Kainote/KaiScrollbar.cpp
    Kainote/KaiSlider.cpp
    Kainote/KaiStaticBoxSizer.cpp
    Kainote/KaiStaticText.cpp
    Kainote/KaiStatusBar.cpp
    Kainote/KaiTabBar.cpp
    Kainote/KaiTextCtrl.cpp
    Kainote/KaiTextValidator.cpp
    Kainote/KaiTreeBook.cpp
    Kainote/KaiWindowResizer.cpp
    Kainote/KeyframesLoader.cpp
    Kainote/ListControls.cpp
    Kainote/LogHandler.cpp
    Kainote/MappedButton.cpp
    Kainote/Menu.cpp
    Kainote/MenuButton.cpp
    Kainote/MispellReplacerDialog.cpp
    Kainote/OptionsPanels.cpp
    Kainote/SelectLines.cpp
    Kainote/SpellCheckerDialog.cpp
    Kainote/MisspellReplacer.cpp
    Kainote/styles.cpp
    Kainote/stylestore.cpp
    Kainote/SubsGridDialogs.cpp
    Kainote/SubsGridFiltering.cpp
    Kainote/SubsGridPreview.cpp
    Kainote/SubsGridWindow.cpp
    Kainote/SubsLoader.cpp
    Kainote/SubsResampleDialog.cpp
    Kainote/SubtitlesLibass.cpp
    Kainote/SubtitlesProviderManager.cpp
    Kainote/SubtitlesVSFilter.cpp
    Kainote/TextEditorTagList.cpp
    Kainote/VideoToolbar.cpp
    Kainote/VisualAllTagsControls.cpp
    Kainote/VisualAllTagsEdition.cpp
    Kainote/VisualCross.cpp
    Kainote/VisualDrawingShapes.cpp
    Kainote/VisualMoveAll.cpp
    Kainote/VisualClipRect.cpp
    Kainote/StyleChange.cpp
    Kainote/ColorPicker.cpp
    Kainote/DropFiles.cpp
    Kainote/EditBox.cpp
    Kainote/FontCollector.cpp
    Kainote/FontDialog.cpp
    Kainote/SubsGrid.cpp
    Kainote/Hotkeys.cpp
    Kainote/KaraokeSplitting.cpp
    Kainote/NewCatalog.cpp
    Kainote/NumCtrl.cpp
    Kainote/OpennWrite.cpp
    Kainote/OptionsDialog.cpp
    Kainote/ProgressDialog.cpp
    Kainote/ScriptInfo.cpp
    Kainote/SpellChecker.cpp
    Kainote/StyleList.cpp
    Kainote/Stylelistbox.cpp
    Kainote/StylePreview.cpp
    Kainote/SubsDialogue.cpp
    Kainote/SubsFile.cpp
    Kainote/SubsGridBase.cpp
    Kainote/TabPanel.cpp
    Kainote/SubsTime.cpp
    Kainote/Timebase.cpp
    Kainote/TimeCtrl.cpp
    Kainote/TLDialog.cpp
    Kainote/Toolbar.cpp
    Kainote/VideoBox.cpp
    Kainote/VideoFullscreen.cpp
    Kainote/VideoSlider.cpp
    Kainote/VisualClips.cpp
    Kainote/VisualMove.cpp
    Kainote/VisualPosition.cpp
    Kainote/VisualRotationXY.cpp
    Kainote/VisualRotationZ.cpp
    Kainote/Visuals.cpp
    Kainote/VisualScale.cpp
    Kainote/UtilsWindows.cpp
    Thirdparty/Build/FFMS2/indexing_additional.cpp
)
target_include_directories(kainote PRIVATE
    ${CMAKE_SOURCE_DIR}/Kainote
    # wx/msw/*.h shims for wxGTK; kept out of Kainote/ for the Windows build.
    ${CMAKE_SOURCE_DIR}/cmake/linux-compat
    ${CMAKE_SOURCE_DIR}/Kainote/GFFT
    ${CMAKE_SOURCE_DIR}/Thirdparty/luabins/include
    ${CMAKE_SOURCE_DIR}/Thirdparty/DependencyControl/include
    ${CMAKE_SOURCE_DIR}/Thirdparty/ffms2/src
)

# Generate Kainote/gitparams.h from the current git HEAD before building.  Runs
# every build but only rewrites the header when the commit/branch changes, so
# config.cpp recompiles only when the version actually moves.
add_custom_target(kainote_gitparams ALL
    COMMAND ${CMAKE_COMMAND} -DSRC=${CMAKE_SOURCE_DIR} -P ${CMAKE_SOURCE_DIR}/cmake/GenGitParams.cmake
    BYPRODUCTS ${CMAKE_SOURCE_DIR}/Kainote/gitparams.h
    COMMENT "Generating gitparams.h from git HEAD"
    VERBATIM
)
add_dependencies(kainote kainote_gitparams)
target_link_libraries(kainote PRIVATE ${wxWidgets_LIBRARIES} PkgConfig::LIBASS PkgConfig::FFMS2 PkgConfig::LUA PkgConfig::HUNSPELL PkgConfig::UCHARDET PkgConfig::LIBCURL PkgConfig::ICU_UC PkgConfig::ICU_I18N PkgConfig::FFMPEG PkgConfig::FONTCONFIG PkgConfig::GSTREAMER Boost::filesystem Boost::locale Boost::regex $<$<BOOL:${Boost_SYSTEM_FOUND}>:Boost::system> $<$<BOOL:${Boost_THREAD_FOUND}>:Boost::thread> GL ${CMAKE_DL_LIBS})
set_target_properties(kainote PROPERTIES
    BUILD_RPATH "$ORIGIN"
    INSTALL_RPATH "$ORIGIN"
)
# Prefer old-style DT_RPATH over DT_RUNPATH for the portable build tree.  RPATH
# is applied transitively, so the bundled app-private libraries (e.g. libicuuc)
# can resolve their own versioned dependencies (for example libicudata) from
# $ORIGIN too.  Host-coupled libraries (GL drivers, the GTK/X11 stack and
# libstdc++) are intentionally NOT bundled (see cmake/CopyRuntimeDependencies),
# so this RPATH only ever points the loader at our own libraries.
target_link_options(kainote PRIVATE "-Wl,--disable-new-dtags")

set(KAINOTE_RUNTIME_DIR $<TARGET_FILE_DIR:kainote>)
set(KAINOTE_DICTIONARY_SOURCE_DIR "${CMAKE_SOURCE_DIR}/Dictionary")
find_path(KAINOTE_SYSTEM_DICTIONARY_DIR
    NAMES en_US.aff
    PATHS /usr/share/hunspell /usr/share/myspell/dicts
)
set(KAINOTE_DICTIONARY_COPY_COMMAND
    COMMAND ${CMAKE_COMMAND} -E make_directory "${KAINOTE_RUNTIME_DIR}/Dictionary"
)
if(EXISTS "${KAINOTE_DICTIONARY_SOURCE_DIR}")
    set(KAINOTE_DICTIONARY_COPY_COMMAND
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${KAINOTE_DICTIONARY_SOURCE_DIR}"
                "${KAINOTE_RUNTIME_DIR}/Dictionary"
    )
elseif(KAINOTE_SYSTEM_DICTIONARY_DIR AND EXISTS "${KAINOTE_SYSTEM_DICTIONARY_DIR}/en_US.dic")
    set(KAINOTE_DICTIONARY_COPY_COMMAND
        COMMAND ${CMAKE_COMMAND} -E make_directory "${KAINOTE_RUNTIME_DIR}/Dictionary"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${KAINOTE_SYSTEM_DICTIONARY_DIR}/en_US.aff"
                "${KAINOTE_SYSTEM_DICTIONARY_DIR}/en_US.dic"
                "${KAINOTE_RUNTIME_DIR}/Dictionary"
    )
else()
    message(WARNING
        "No en_US Hunspell dictionary was found. Kainote will build, but its "
        "spell checker will have no default dictionary; install the distribution's "
        "English Hunspell dictionary package and reconfigure.")
endif()

# Keep the build tree runnable by copying runtime data next to the executable.
# Windows embeds PNG RCDATA through resource.rc, but the Linux build loads those
# resources from files. Translation catalogs are not committed: compile
# Locale/*.po into Locale/<lang>/LC_MESSAGES/kainote.mo, the layout wx loads.
add_custom_command(TARGET kainote POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${KAINOTE_RUNTIME_DIR}/Locale"
    COMMAND ${Python3_EXECUTABLE}
            "${CMAKE_SOURCE_DIR}/tools/compile_catalogs.py"
            --po-dir "${CMAKE_SOURCE_DIR}/Locale"
            --out-dir "${KAINOTE_RUNTIME_DIR}/Locale"
            --msgfmt "${MSGFMT_EXECUTABLE}"
    ${KAINOTE_DICTIONARY_COPY_COMMAND}
    COMMAND ${CMAKE_COMMAND} -E make_directory "${KAINOTE_RUNTIME_DIR}/Kainote"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_SOURCE_DIR}/Kainote/resource.rc"
            "${KAINOTE_RUNTIME_DIR}/Kainote/resource.rc"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/Kainote/Bitmaps"
            "${KAINOTE_RUNTIME_DIR}/Kainote/Bitmaps"
    COMMAND ${CMAKE_COMMAND}
            -D KAINOTE_EXE=$<TARGET_FILE:kainote>
            -D RUNTIME_DIR=${KAINOTE_RUNTIME_DIR}
            -D KAINOTE_BUILD_DIR=${CMAKE_BINARY_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/CopyRuntimeDependencies.cmake"
    COMMENT "Copying Kainote runtime resources and bundled libraries"
)

# Package only explicitly selected runtime files.
set(KAINOTE_PACKAGE_BASENAME "kainote-linux-${CMAKE_SYSTEM_PROCESSOR}")
set(KAINOTE_PACKAGE_STAGE_PARENT "${CMAKE_BINARY_DIR}/package-stage")
set(KAINOTE_PACKAGE_ARCHIVE
    "${CMAKE_BINARY_DIR}/dist/${KAINOTE_PACKAGE_BASENAME}.tar.gz")
file(MAKE_DIRECTORY "${KAINOTE_PACKAGE_STAGE_PARENT}")
include(${CMAKE_SOURCE_DIR}/cmake/DesktopIntegration.cmake)

add_custom_target(kainote_linux_package
    # Refresh the dependency manifest before every package.
    COMMAND ${CMAKE_COMMAND}
            -D KAINOTE_EXE=$<TARGET_FILE:kainote>
            -D RUNTIME_DIR=$<TARGET_FILE_DIR:kainote>
            -D KAINOTE_BUILD_DIR=${CMAKE_BINARY_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/CopyRuntimeDependencies.cmake"
    COMMAND ${CMAKE_COMMAND}
            -D KAINOTE_EXE=$<TARGET_FILE:kainote>
            -D RUNTIME_DIR=$<TARGET_FILE_DIR:kainote>
            -D SOURCE_DIR=${CMAKE_SOURCE_DIR}
            -D BINARY_DIR=${CMAKE_BINARY_DIR}
            -D STAGING_PARENT=${KAINOTE_PACKAGE_STAGE_PARENT}
            -D PACKAGE_BASENAME=${KAINOTE_PACKAGE_BASENAME}
            -D ARCHIVE_PATH=${KAINOTE_PACKAGE_ARCHIVE}
            -D MSGFMT_EXECUTABLE=${MSGFMT_EXECUTABLE}
            -D Python3_EXECUTABLE=${Python3_EXECUTABLE}
            -D TAR_EXECUTABLE=${TAR_EXECUTABLE}
            -D GZIP_EXECUTABLE=${GZIP_EXECUTABLE}
            -D PACKAGING_DIR=${CMAKE_SOURCE_DIR}/packaging/linux
            -D DESKTOP_GEN_DIR=${CMAKE_BINARY_DIR}/packaging
            -D SYSTEM_DICTIONARY_DIR=${KAINOTE_SYSTEM_DICTIONARY_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/PackageLinuxRuntime.cmake"
    DEPENDS kainote
    BYPRODUCTS
        "${KAINOTE_PACKAGE_ARCHIVE}"
        "${KAINOTE_PACKAGE_ARCHIVE}.sha256"
    WORKING_DIRECTORY "${KAINOTE_PACKAGE_STAGE_PARENT}"
    COMMENT "Creating clean Kainote Linux runtime archive"
    VERBATIM
)
