# Linux desktop integration: .desktop, AppStream metainfo, MIME icons.
#
# Only data is installed, deliberately not the executable. The runtime locates
# its resources by walking up from the binary's own directory
# (config.cpp FindResourceRoot / LoadOptions), so an exe in /usr/bin would
# never find /usr/share/kainote. Making Kainote relocatable is a separate job.

option(KAINOTE_INSTALL_DESKTOP_INTEGRATION
       "Install .desktop, AppStream metainfo, MIME and hicolor icons" ON)
set(KAINOTE_DESKTOP_EXEC "kainote" CACHE STRING
    "Exec= command written into kainote.desktop")

# MimeType= comes from Kainote/FileTypes.h so the two cannot drift. Rows whose
# type is nullptr (.txt, which is text/plain) are skipped on purpose.
function(kainote_collect_mimetypes out_var)
    file(READ "${CMAKE_SOURCE_DIR}/Kainote/FileTypes.h" _tbl)
    string(REGEX MATCHALL "\"[a-z]+/[a-zA-Z0-9.+-]+\"" _quoted "${_tbl}")

    set(_seen "")
    foreach(_q IN LISTS _quoted)
        string(REPLACE "\"" "" _type "${_q}")
        if(NOT _type IN_LIST _seen)
            list(APPEND _seen "${_type}")
        endif()
    endforeach()

    list(LENGTH _seen _count)
    if(_count EQUAL 0)
        message(FATAL_ERROR "No mime types found in Kainote/FileTypes.h")
    endif()

    string(JOIN ";" _joined ${_seen})
    set(${out_var} "${_joined};" PARENT_SCOPE)
endfunction()

if(KAINOTE_INSTALL_DESKTOP_INTEGRATION)
    include(GNUInstallDirs)

    file(STRINGS "${CMAKE_SOURCE_DIR}/Kainote/VersionKainote.h" _ver_line
         REGEX "^#define[ \t]+VersionKainote[ \t]+\"")
    string(REGEX REPLACE ".*\"([^\"]+)\".*" "\\1" KAINOTE_VERSION "${_ver_line}")
    if(NOT KAINOTE_VERSION)
        message(FATAL_ERROR "Could not read VersionKainote from Kainote/VersionKainote.h")
    endif()
    # A prerelease such as 1.2.0-rc.1 is a development release to AppStream.
    if(KAINOTE_VERSION MATCHES "-")
        set(KAINOTE_RELEASE_TYPE "development")
    else()
        set(KAINOTE_RELEASE_TYPE "stable")
    endif()

    # AppStream requires a date on every release entry. The commit date is the
    # only one in the tree that is deterministic; GetReleaseDate() is __DATE__.
    find_package(Git QUIET)
    set(KAINOTE_RELEASE_DATE "")
    if(Git_FOUND)
        execute_process(COMMAND "${GIT_EXECUTABLE}" -C "${CMAKE_SOURCE_DIR}"
                                log -1 --format=%cs
                        OUTPUT_VARIABLE KAINOTE_RELEASE_DATE
                        OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
    endif()
    if(NOT KAINOTE_RELEASE_DATE)
        set(KAINOTE_RELEASE_DATE "1970-01-01")
    endif()

    kainote_collect_mimetypes(KAINOTE_DESKTOP_MIMETYPES)
    message(STATUS "Desktop MimeType=: ${KAINOTE_DESKTOP_MIMETYPES}")

    set(_pkg "${CMAKE_SOURCE_DIR}/packaging/linux")
    set(_gen "${CMAKE_BINARY_DIR}/packaging")

    configure_file("${_pkg}/io.github.bjakja.Kainote.desktop.in"
                   "${_gen}/io.github.bjakja.Kainote.desktop" @ONLY)
    configure_file("${_pkg}/io.github.bjakja.Kainote.metainfo.xml.in"
                   "${_gen}/io.github.bjakja.Kainote.metainfo.xml" @ONLY)

    install(FILES "${_gen}/io.github.bjakja.Kainote.desktop"
            DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/applications")
    install(FILES "${_gen}/io.github.bjakja.Kainote.metainfo.xml"
            DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/metainfo")
    install(FILES "${_pkg}/mime/kainote.xml"
            DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/mime/packages")
    install(DIRECTORY "${_pkg}/icons/hicolor"
            DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/icons"
            FILES_MATCHING PATTERN "*.png")

    # No update-desktop-database / update-mime-database hook here: distro
    # tooling runs those, and running them against DESTDIR would be wrong.
    # install-desktop-integration.sh does it for people using the tarball.
endif()
