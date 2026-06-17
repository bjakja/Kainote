# Generates Kainote/gitparams.h with the current git commit + branch.
# config.cpp consumes GIT_CUR_COMMIT / GIT_BRANCH (guarded by #ifdef
# GIT_CUR_COMMIT) and stringifies them via ADD_QUOTES for the title-bar version.
# The file is gitignored; on Windows the AutoVersioning tool emits it instead.
# Invoked with: cmake -DSRC=<source dir> -P GenGitParams.cmake
find_package(Git QUIET)

set(GIT_COMMIT "")
set(GIT_BRANCH "")
if(Git_FOUND)
    execute_process(COMMAND "${GIT_EXECUTABLE}" -C "${SRC}" rev-parse HEAD
        OUTPUT_VARIABLE GIT_COMMIT OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
    execute_process(COMMAND "${GIT_EXECUTABLE}" -C "${SRC}" rev-parse --abbrev-ref HEAD
        OUTPUT_VARIABLE GIT_BRANCH OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
endif()

if(GIT_COMMIT STREQUAL "")
    # No git / not a checkout: emit an empty header (config.cpp omits the suffix).
    set(CONTENT "#pragma once\n")
else()
    if(GIT_BRANCH STREQUAL "")
        set(GIT_BRANCH "unknown")
    endif()
    set(CONTENT "#pragma once\n#define _KAI_QUOTE(x) #x\n#define ADD_QUOTES(x) _KAI_QUOTE(x)\n#define GIT_BRANCH ${GIT_BRANCH}\n#define GIT_CUR_COMMIT ${GIT_COMMIT}\n")
endif()

set(OUT "${SRC}/Kainote/gitparams.h")
set(EXISTING "")
if(EXISTS "${OUT}")
    file(READ "${OUT}" EXISTING)
endif()
# Only rewrite when the content changes, so config.cpp isn't recompiled every build.
if(NOT EXISTING STREQUAL CONTENT)
    file(WRITE "${OUT}" "${CONTENT}")
endif()
