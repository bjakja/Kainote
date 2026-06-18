if(NOT DEFINED SOURCE_LOCALE_DIR OR NOT DEFINED RUNTIME_LOCALE_DIR)
    message(FATAL_ERROR "SOURCE_LOCALE_DIR and RUNTIME_LOCALE_DIR are required")
endif()

if(NOT MSGFMT_EXECUTABLE)
    message(WARNING "msgfmt not found; translation catalogs (.mo) were not built")
    return()
endif()

# Compile every Locale/*.po into the GNU gettext layout wxLocale expects and a
# flat copy the options-dialog language list enumerates. .mo files are generated
# here each build, not committed.
file(GLOB KAINOTE_PO_FILES "${SOURCE_LOCALE_DIR}/*.po")
foreach(po_file IN LISTS KAINOTE_PO_FILES)
    get_filename_component(locale_name "${po_file}" NAME_WE)
    set(locale_messages_dir "${RUNTIME_LOCALE_DIR}/${locale_name}/LC_MESSAGES")
    file(MAKE_DIRECTORY "${locale_messages_dir}")
    execute_process(
        COMMAND "${MSGFMT_EXECUTABLE}" -o "${locale_messages_dir}/${locale_name}.mo" "${po_file}"
        RESULT_VARIABLE msgfmt_result
    )
    if(msgfmt_result)
        message(WARNING "msgfmt failed for ${po_file} (exit ${msgfmt_result})")
    else()
        file(COPY_FILE "${locale_messages_dir}/${locale_name}.mo"
                       "${RUNTIME_LOCALE_DIR}/${locale_name}.mo" ONLY_IF_DIFFERENT)
    endif()
endforeach()
