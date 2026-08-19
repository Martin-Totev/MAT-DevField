add_executable(mat_devfield_ascii_example ascii_basic.cpp)

target_compile_features(mat_devfield_ascii_example PRIVATE cxx_std_17)
target_link_libraries(mat_devfield_ascii_example PRIVATE MATDevField::ASCII)

if(MSVC)
    target_compile_options(mat_devfield_ascii_example PRIVATE /W4 /permissive- /utf-8)
else()
    target_compile_options(mat_devfield_ascii_example PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(WIN32)
    add_custom_command(
        TARGET mat_devfield_ascii_example
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:MATDevFieldASCII>
            $<TARGET_FILE_DIR:mat_devfield_ascii_example>
        VERBATIM
    )
endif()

