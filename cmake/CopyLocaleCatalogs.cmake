if(NOT DEFINED SOURCE_LOCALE_DIR OR NOT DEFINED RUNTIME_LOCALE_DIR)
    message(FATAL_ERROR "SOURCE_LOCALE_DIR and RUNTIME_LOCALE_DIR are required")
endif()

if(NOT DEFINED KAINOTE_SOURCE_DIR)
    set(KAINOTE_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/..")
endif()

# tools/compile_catalogs.py is the one implementation; the packaging script and
# the Visual Studio pre-build event run the same file.
set(_kainote_msgfmt_arg)
if(MSGFMT_EXECUTABLE)
    set(_kainote_msgfmt_arg --msgfmt "${MSGFMT_EXECUTABLE}")
endif()

set(_kainote_strict_arg)
if(KAINOTE_STRICT_LOCALES)
    set(_kainote_strict_arg --strict)
endif()

execute_process(
    COMMAND "${Python3_EXECUTABLE}" "${KAINOTE_SOURCE_DIR}/tools/compile_catalogs.py"
            --po-dir "${SOURCE_LOCALE_DIR}"
            --out-dir "${RUNTIME_LOCALE_DIR}"
            ${_kainote_msgfmt_arg}
            ${_kainote_strict_arg}
    RESULT_VARIABLE _kainote_locale_result
)
if(_kainote_locale_result)
    message(FATAL_ERROR "compiling translation catalogs failed (exit ${_kainote_locale_result})")
endif()
