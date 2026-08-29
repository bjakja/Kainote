if(NOT DEFINED KAINOTE_EXE OR NOT DEFINED RUNTIME_DIR)
    message(FATAL_ERROR "KAINOTE_EXE and RUNTIME_DIR are required")
endif()

# Remove previously manifested libraries before rescanning $ORIGIN.
set(runtime_manifest "${RUNTIME_DIR}/.kainote-runtime-dependencies")
if(EXISTS "${runtime_manifest}")
    file(STRINGS "${runtime_manifest}" previous_runtime_deps)
    foreach(dep_name IN LISTS previous_runtime_deps)
        if(dep_name MATCHES "^lib[^/\\\\]+\\.so(\\..*)?$")
            file(REMOVE "${RUNTIME_DIR}/${dep_name}")
        else()
            message(WARNING "Ignoring invalid runtime manifest entry: ${dep_name}")
        endif()
    endforeach()
elseif(DEFINED KAINOTE_BUILD_DIR)
    # Clean pre-manifest files only in the verified CMake build directory.
    file(REAL_PATH "${RUNTIME_DIR}" runtime_dir_real)
    file(REAL_PATH "${KAINOTE_BUILD_DIR}" build_dir_real)
    if(runtime_dir_real STREQUAL build_dir_real AND
       EXISTS "${build_dir_real}/CMakeCache.txt")
        file(GLOB legacy_runtime_deps LIST_DIRECTORIES FALSE
            "${RUNTIME_DIR}/lib*.so" "${RUNTIME_DIR}/lib*.so.*")
        if(legacy_runtime_deps)
            list(LENGTH legacy_runtime_deps legacy_count)
            file(REMOVE ${legacy_runtime_deps})
            message(STATUS "Removed ${legacy_count} legacy bundled runtime dependencies before dependency discovery")
        endif()
    endif()
endif()

file(GET_RUNTIME_DEPENDENCIES
    EXECUTABLES "${KAINOTE_EXE}"
    RESOLVED_DEPENDENCIES_VAR resolved_deps
    UNRESOLVED_DEPENDENCIES_VAR unresolved_deps
)

if(unresolved_deps)
    message(FATAL_ERROR "Unresolved runtime dependencies: ${unresolved_deps}")
endif()

# Bundle only the application's own version-sensitive libraries (wxWidgets, ICU,
# FFMS2, libass, Lua, Hunspell, uchardet and their private transitive helpers)
# next to the binary.
#
#   * GPU/driver-coupled: libGL/GLVND/EGL/GLX/OpenGL/glapi, libva, libvdpau,
#     libdrm, libgbm -- GLVND dlopens the host's vendor ICD (e.g. libGLX_nvidia)
#     which must match the GL stack; a bundled mismatch is a black-screen/crash.
#   * Windowing/desktop stack: X11/xcb, wayland, xkbcommon and the
#     GTK/GDK/pango/cairo/gdk-pixbuf/atk/harfbuzz toolkit -- the host dlopens
#     matching pixbuf loaders, immodules and theme engines (GLib above is the
#     one exception: it is bundled for GStreamer and is forward-compatible).
#   * C++/compiler runtime: libstdc++, libgcc_s -- a bundled (older) copy would
#     win on the process RPATH and then fail host GTK plugins that need a newer
#     GLIBCXX/CXXABI.
#   * Host audio: libpulse/libasound -- SDL2 dlopens the host backend anyway.
#   * glibc/ELF loader: must come from the host kernel/libc.
#   * GPL-licensed codecs: libx264/libx265 -- not redistributed, to avoid
#     imposing GPL terms on the bundle; FFmpeg loads the host copies instead.
set(system_dep_regex
    "^(ld-linux.*|linux-vdso.*|lib(c|m|dl|pthread|rt|resolv|nsl|util|anl)\\.so.*|libstdc\\+\\+\\.so.*|libgcc_s\\.so.*|libGL.*\\.so.*|libEGL.*\\.so.*|libOpenGL\\.so.*|libGLdispatch\\.so.*|libGLX.*\\.so.*|libglapi\\.so.*|libgbm\\.so.*|libdrm.*\\.so.*|libva.*\\.so.*|libvdpau.*\\.so.*|libX.*\\.so.*|libxcb.*\\.so.*|libxshmfence\\.so.*|libxkbcommon.*\\.so.*|libwayland-.*\\.so.*|libgtk-.*\\.so.*|libgdk-.*\\.so.*|libgdk_pixbuf.*\\.so.*|libpango.*\\.so.*|libcairo.*\\.so.*|libatk.*\\.so.*|libharfbuzz.*\\.so.*|libpulse.*\\.so.*|libasound\\.so.*|libx264\\.so.*|libx265\\.so.*)$"
)

set(copied_count 0)
set(copied_names)
foreach(dep IN LISTS resolved_deps)
    get_filename_component(dep_name "${dep}" NAME)
    if(NOT dep_name MATCHES "${system_dep_regex}")
        set(copied_dep "${RUNTIME_DIR}/${dep_name}")
        execute_process(
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                    "${dep}" "${copied_dep}"
            RESULT_VARIABLE copy_result
        )
        if(NOT copy_result EQUAL 0)
            message(FATAL_ERROR "Failed to copy runtime dependency ${dep}")
        endif()
        list(APPEND copied_names "${dep_name}")
        math(EXPR copied_count "${copied_count} + 1")
    endif()
endforeach()

list(REMOVE_DUPLICATES copied_names)
list(SORT copied_names)
string(REPLACE ";" "\n" runtime_manifest_contents "${copied_names}")
file(WRITE "${runtime_manifest}.tmp" "${runtime_manifest_contents}\n")
file(RENAME "${runtime_manifest}.tmp" "${runtime_manifest}")

message(STATUS "Copied ${copied_count} bundled runtime dependencies to ${RUNTIME_DIR}")
