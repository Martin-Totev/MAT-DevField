if(NOT DEFINED MAT_ARCHIVE OR NOT EXISTS "${MAT_ARCHIVE}")
    message(FATAL_ERROR "MAT archive does not exist: ${MAT_ARCHIVE}")
endif()

if(NOT DEFINED DEPENDENCY_ARCHIVE OR NOT EXISTS "${DEPENDENCY_ARCHIVE}")
    message(FATAL_ERROR "Dependency archive does not exist: ${DEPENDENCY_ARCHIVE}")
endif()

if(NOT DEFINED ARCHIVER OR NOT EXISTS "${ARCHIVER}")
    message(FATAL_ERROR "Static-library archiver does not exist: ${ARCHIVER}")
endif()

set(merged_archive "${MAT_ARCHIVE}.merged")
file(REMOVE "${merged_archive}")

if(WIN32)
    execute_process(
        COMMAND "${ARCHIVER}" /NOLOGO "/OUT:${merged_archive}"
            "${MAT_ARCHIVE}" "${DEPENDENCY_ARCHIVE}"
        RESULT_VARIABLE merge_result
        OUTPUT_VARIABLE merge_output
        ERROR_VARIABLE merge_error
    )
elseif(APPLE_PLATFORM)
    execute_process(
        COMMAND /usr/bin/libtool -static -o "${merged_archive}"
            "${MAT_ARCHIVE}" "${DEPENDENCY_ARCHIVE}"
        RESULT_VARIABLE merge_result
        OUTPUT_VARIABLE merge_output
        ERROR_VARIABLE merge_error
    )
else()
    # GNU ar's MRI language cannot quote paths containing spaces. Stage the
    # dependency beside the MAT archive and refer only to space-free basenames.
    get_filename_component(archive_directory "${MAT_ARCHIVE}" DIRECTORY)
    get_filename_component(mat_archive_name "${MAT_ARCHIVE}" NAME)
    set(merged_archive_name "${mat_archive_name}.merged")
    set(staged_dependency_name "MATDevFieldPrivateDependency.a")
    set(staged_dependency "${archive_directory}/${staged_dependency_name}")
    set(mri_script "${archive_directory}/MATDevFieldMerge.mri")

    file(COPY_FILE "${DEPENDENCY_ARCHIVE}" "${staged_dependency}" ONLY_IF_DIFFERENT)
    file(WRITE "${mri_script}"
        "create ${merged_archive_name}\n"
        "addlib ${mat_archive_name}\n"
        "addlib ${staged_dependency_name}\n"
        "save\n"
        "end\n"
    )
    execute_process(
        COMMAND "${ARCHIVER}" -M
        INPUT_FILE "${mri_script}"
        WORKING_DIRECTORY "${archive_directory}"
        RESULT_VARIABLE merge_result
        OUTPUT_VARIABLE merge_output
        ERROR_VARIABLE merge_error
    )
    file(REMOVE "${mri_script}" "${staged_dependency}")
endif()

if(NOT merge_result EQUAL 0)
    file(REMOVE "${merged_archive}")
    message(FATAL_ERROR
        "Could not combine MAT DevField and SDL into one static library.\n"
        "${merge_output}${merge_error}"
    )
endif()

file(RENAME "${merged_archive}" "${MAT_ARCHIVE}")
