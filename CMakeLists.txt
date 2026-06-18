cmake_minimum_required(VERSION 3.20)
project(KainoteLinux LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC OFF)
find_package(PkgConfig REQUIRED)
find_package(wxWidgets REQUIRED COMPONENTS core base adv aui html xml gl stc net)
include(${wxWidgets_USE_FILE})
pkg_check_modules(LIBASS REQUIRED IMPORTED_TARGET libass)
pkg_check_modules(FFMS2 REQUIRED IMPORTED_TARGET ffms2)
pkg_check_modules(LUA REQUIRED IMPORTED_TARGET lua5.1)
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
pkg_get_variable(GSTREAMER_PLUGIN_DIR gstreamer-1.0 pluginsdir)

find_library(LUA_RUNTIME_LIBRARY
    NAMES liblua5.1.so.0 lua5.1
    HINTS ${LUA_LIBRARY_DIRS}
    REQUIRED
)

# Link Boost statically on Linux so the built binary does not depend on the
# exact distro Boost shared-library SONAME, e.g. libboost_filesystem.so.1.83.0.
# Other system libraries remain dynamically linked through pkg-config/wxWidgets.
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost REQUIRED COMPONENTS filesystem locale regex OPTIONAL_COMPONENTS system thread)
find_program(MSGFMT_EXECUTABLE msgfmt)
if(NOT MSGFMT_EXECUTABLE)
    message(WARNING "msgfmt (gettext) not found; translation catalogs (Locale/*.mo) will not be built")
endif()
add_compile_definitions(_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING _CRT_SECURE_NO_WARNINGS HAVE_DLL_EXPORT LINUX_BUILD KAINOTE_LINUX_BUILD)
add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-include> $<$<COMPILE_LANGUAGE:CXX>:${CMAKE_SOURCE_DIR}/Kainote/platform.h> -Wno-deprecated-declarations -Wno-narrowing -Wno-unused-result $<$<COMPILE_LANGUAGE:CXX>:-fpermissive>)
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
    Kainote/Languages.cpp
    Kainote/LineParse.cpp
    Kainote/Provider.cpp
    Kainote/ProviderDummy.cpp
    Kainote/ProviderFFMS2.cpp
    Kainote/RendererFFMS2.cpp
    Kainote/RendererGStreamer.cpp
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
    ${CMAKE_SOURCE_DIR}/Kainote/GFFT
    ${CMAKE_SOURCE_DIR}/Thirdparty/luabins/include
    ${CMAKE_SOURCE_DIR}/Thirdparty/DependencyControl/include
    ${CMAKE_SOURCE_DIR}/Thirdparty/ffms2/src
)

if(GSTREAMER_PLUGIN_DIR)
    set_property(SOURCE Kainote/AudioPlayerDSound.cpp Kainote/RendererGStreamer.cpp
        APPEND PROPERTY COMPILE_DEFINITIONS "KAI_GST_PLUGIN_DIR=\"${GSTREAMER_PLUGIN_DIR}\"")
endif()

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
endif()

# Keep the build tree runnable by copying runtime data next to the executable.
# Windows embeds PNG RCDATA through resource.rc, but the Linux build loads those
# resources from files. Translation catalogs are no longer committed: compile
# Locale/*.po into flat Locale/<lang>.mo (for the options list) and GNU
# gettext-style Locale/<lang>/LC_MESSAGES/<lang>.mo paths (for wxLocale).
add_custom_command(TARGET kainote POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${KAINOTE_RUNTIME_DIR}/Locale"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${LUA_RUNTIME_LIBRARY}"
            "${KAINOTE_RUNTIME_DIR}/liblua5.1.so.0"
    COMMAND ${CMAKE_COMMAND}
            -D SOURCE_LOCALE_DIR="${CMAKE_SOURCE_DIR}/Locale"
            -D RUNTIME_LOCALE_DIR="${KAINOTE_RUNTIME_DIR}/Locale"
            -D MSGFMT_EXECUTABLE="${MSGFMT_EXECUTABLE}"
            -P "${CMAKE_SOURCE_DIR}/cmake/CopyLocaleCatalogs.cmake"
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
            -P "${CMAKE_SOURCE_DIR}/cmake/CopyRuntimeDependencies.cmake"
    COMMENT "Copying Kainote runtime resources and bundled libraries"
)
