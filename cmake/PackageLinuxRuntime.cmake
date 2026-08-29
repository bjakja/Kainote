foreach(required_var IN ITEMS
        KAINOTE_EXE RUNTIME_DIR SOURCE_DIR BINARY_DIR STAGING_PARENT
        PACKAGE_BASENAME ARCHIVE_PATH MSGFMT_EXECUTABLE TAR_EXECUTABLE
        GZIP_EXECUTABLE)
    if(NOT DEFINED ${required_var} OR "${${required_var}}" STREQUAL "")
        message(FATAL_ERROR "${required_var} is required")
    endif()
endforeach()

if(NOT PACKAGE_BASENAME MATCHES "^[A-Za-z0-9][A-Za-z0-9._+-]*$")
    message(FATAL_ERROR
        "PACKAGE_BASENAME must be a single safe path component: "
        "${PACKAGE_BASENAME}")
endif()
if(NOT IS_DIRECTORY "${BINARY_DIR}" OR
   NOT EXISTS "${BINARY_DIR}/CMakeCache.txt")
    message(FATAL_ERROR "BINARY_DIR is not a configured CMake build tree: ${BINARY_DIR}")
endif()
if(NOT IS_DIRECTORY "${STAGING_PARENT}")
    message(FATAL_ERROR "Package staging parent does not exist: ${STAGING_PARENT}")
endif()

file(REAL_PATH "${BINARY_DIR}" binary_dir_abs)
file(REAL_PATH "${STAGING_PARENT}" staging_parent_abs)
file(REAL_PATH "${binary_dir_abs}/package-stage" expected_staging_parent_abs)
if(NOT staging_parent_abs STREQUAL expected_staging_parent_abs)
    message(FATAL_ERROR
        "Package staging must be the dedicated build-tree package-stage directory: "
        "${staging_parent_abs}")
endif()
file(TO_CMAKE_PATH "${binary_dir_abs}/" binary_dir_prefix)
file(TO_CMAKE_PATH "${staging_parent_abs}/" staging_parent_prefix)
string(FIND "${staging_parent_prefix}" "${binary_dir_prefix}" staging_prefix_pos)
if(NOT staging_prefix_pos EQUAL 0)
    message(FATAL_ERROR
        "Refusing to clean package staging directory outside the CMake build tree: "
        "${staging_parent_abs}")
endif()

if(NOT EXISTS "${KAINOTE_EXE}")
    message(FATAL_ERROR "Kainote executable is missing: ${KAINOTE_EXE}")
endif()

set(runtime_manifest "${RUNTIME_DIR}/.kainote-runtime-dependencies")
if(NOT EXISTS "${runtime_manifest}")
    message(FATAL_ERROR
        "Runtime dependency manifest is missing. Build the kainote target first: "
        "${runtime_manifest}")
endif()

set(package_root "${staging_parent_abs}/${PACKAGE_BASENAME}")
file(REMOVE_RECURSE "${package_root}")
file(MAKE_DIRECTORY "${package_root}" "${package_root}/Kainote")

file(COPY_FILE "${KAINOTE_EXE}" "${package_root}/kainote")

# Copy only manifest-listed runtime libraries.
file(STRINGS "${runtime_manifest}" runtime_deps)
if(NOT runtime_deps)
    message(FATAL_ERROR "Runtime dependency manifest is empty: ${runtime_manifest}")
endif()
foreach(dep_name IN LISTS runtime_deps)
    if(NOT dep_name MATCHES "^lib[^/\\\\]+\\.so(\\..*)?$")
        message(FATAL_ERROR "Invalid runtime dependency manifest entry: ${dep_name}")
    endif()
    if(NOT EXISTS "${RUNTIME_DIR}/${dep_name}")
        message(FATAL_ERROR "Manifest-listed runtime dependency is missing: ${dep_name}")
    endif()
    file(COPY_FILE
        "${RUNTIME_DIR}/${dep_name}"
        "${package_root}/${dep_name}")
endforeach()
file(COPY_FILE "${runtime_manifest}"
               "${package_root}/.kainote-runtime-dependencies")

if(NOT EXISTS "${SOURCE_DIR}/Kainote/resource.rc" OR
   NOT IS_DIRECTORY "${SOURCE_DIR}/Kainote/Bitmaps")
    message(FATAL_ERROR "Required Linux bitmap resources are missing from the source tree")
endif()
file(COPY_FILE "${SOURCE_DIR}/Kainote/resource.rc"
               "${package_root}/Kainote/resource.rc")
file(COPY "${SOURCE_DIR}/Kainote/Bitmaps"
     DESTINATION "${package_root}/Kainote")

# Recompile catalogs instead of using stale runtime copies.
set(SOURCE_LOCALE_DIR "${SOURCE_DIR}/Locale")
set(RUNTIME_LOCALE_DIR "${package_root}/Locale")
include("${SOURCE_DIR}/cmake/CopyLocaleCatalogs.cmake")

# Prefer the repository dictionary, then the configured en_US pair.
if(IS_DIRECTORY "${SOURCE_DIR}/Dictionary")
    file(COPY "${SOURCE_DIR}/Dictionary/"
         DESTINATION "${package_root}/Dictionary")
elseif(DEFINED SYSTEM_DICTIONARY_DIR AND
       EXISTS "${SYSTEM_DICTIONARY_DIR}/en_US.aff" AND
       EXISTS "${SYSTEM_DICTIONARY_DIR}/en_US.dic")
    file(MAKE_DIRECTORY "${package_root}/Dictionary")
    file(COPY_FILE "${SYSTEM_DICTIONARY_DIR}/en_US.aff"
                   "${package_root}/Dictionary/en_US.aff")
    file(COPY_FILE "${SYSTEM_DICTIONARY_DIR}/en_US.dic"
                   "${package_root}/Dictionary/en_US.dic")
else()
    message(WARNING
        "Packaging without an en_US dictionary; the spell checker will have no "
        "default dictionary")
endif()

if(EXISTS "${SOURCE_DIR}/LICENSE")
    file(COPY_FILE "${SOURCE_DIR}/LICENSE" "${package_root}/LICENSE")
endif()

# Normalize modes for reproducibility.
file(GLOB_RECURSE package_entries
    LIST_DIRECTORIES TRUE
    "${package_root}/*")
foreach(package_entry IN LISTS package_entries)
    if(IS_DIRECTORY "${package_entry}")
        file(CHMOD "${package_entry}"
            PERMISSIONS
                OWNER_READ OWNER_WRITE OWNER_EXECUTE
                GROUP_READ GROUP_EXECUTE
                WORLD_READ WORLD_EXECUTE)
    else()
        file(CHMOD "${package_entry}"
            PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
    endif()
endforeach()
file(CHMOD "${package_root}"
    PERMISSIONS
        OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ GROUP_EXECUTE
        WORLD_READ WORLD_EXECUTE)
file(CHMOD "${package_root}/kainote"
    PERMISSIONS
        OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ GROUP_EXECUTE
        WORLD_READ WORLD_EXECUTE)

# Hash every staged file.
file(GLOB_RECURSE package_files
    LIST_DIRECTORIES FALSE
    RELATIVE "${package_root}"
    "${package_root}/*")
list(SORT package_files)
set(checksums "")
foreach(relative_file IN LISTS package_files)
    file(SHA256 "${package_root}/${relative_file}" file_hash)
    string(APPEND checksums "${file_hash}  ${relative_file}\n")
endforeach()
file(WRITE "${package_root}/SHA256SUMS" "${checksums}")
file(CHMOD "${package_root}/SHA256SUMS"
    PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)

get_filename_component(archive_path_abs "${ARCHIVE_PATH}" ABSOLUTE)
file(TO_CMAKE_PATH "${archive_path_abs}" archive_path_normalized)
set(expected_archive_path
    "${binary_dir_abs}/dist/${PACKAGE_BASENAME}.tar.gz")
file(TO_CMAKE_PATH "${expected_archive_path}" expected_archive_normalized)
string(FIND "${archive_path_normalized}" "${binary_dir_prefix}" archive_path_prefix_pos)
if(NOT archive_path_prefix_pos EQUAL 0 OR
   NOT archive_path_normalized STREQUAL expected_archive_normalized)
    message(FATAL_ERROR
        "Archive path must be a .tar.gz inside the CMake build tree: "
        "${ARCHIVE_PATH}")
endif()
set(ARCHIVE_PATH "${archive_path_abs}")
get_filename_component(archive_dir "${ARCHIVE_PATH}" DIRECTORY)
file(MAKE_DIRECTORY "${archive_dir}")
file(REAL_PATH "${archive_dir}" archive_dir_abs)
file(TO_CMAKE_PATH "${archive_dir_abs}/" archive_dir_prefix)
string(FIND "${archive_dir_prefix}" "${binary_dir_prefix}" archive_prefix_pos)
if(NOT archive_prefix_pos EQUAL 0)
    message(FATAL_ERROR
        "Archive path must be a .tar.gz inside the CMake build tree: "
        "${ARCHIVE_PATH}")
endif()
set(uncompressed_archive "${ARCHIVE_PATH}.uncompressed")
file(REMOVE
    "${ARCHIVE_PATH}"
    "${ARCHIVE_PATH}.sha256"
    "${uncompressed_archive}")

# Normalize tar metadata and omit gzip timestamps.
execute_process(
    COMMAND "${TAR_EXECUTABLE}"
            --sort=name
            --mtime=@0
            --owner=0
            --group=0
            --numeric-owner
            --format=gnu
            -cf "${uncompressed_archive}"
            "${PACKAGE_BASENAME}"
    WORKING_DIRECTORY "${staging_parent_abs}"
    RESULT_VARIABLE tar_result
)
if(NOT tar_result EQUAL 0)
    file(REMOVE "${uncompressed_archive}")
    message(FATAL_ERROR "tar failed while creating ${uncompressed_archive}")
endif()
execute_process(
    COMMAND "${GZIP_EXECUTABLE}" -n -9 -c "${uncompressed_archive}"
    OUTPUT_FILE "${ARCHIVE_PATH}"
    RESULT_VARIABLE gzip_result
)
file(REMOVE "${uncompressed_archive}")
if(NOT gzip_result EQUAL 0)
    file(REMOVE "${ARCHIVE_PATH}")
    message(FATAL_ERROR "gzip failed while creating ${ARCHIVE_PATH}")
endif()
file(SHA256 "${ARCHIVE_PATH}" archive_hash)
get_filename_component(archive_name "${ARCHIVE_PATH}" NAME)
file(WRITE "${ARCHIVE_PATH}.sha256" "${archive_hash}  ${archive_name}\n")

message(STATUS "Created ${ARCHIVE_PATH}")
message(STATUS "SHA-256 ${archive_hash}")
