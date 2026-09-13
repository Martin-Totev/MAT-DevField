add_executable(mat_devfield_ascii_example ascii_basic.cpp)

target_compile_features(mat_devfield_ascii_example PRIVATE cxx_std_17)
target_link_libraries(mat_devfield_ascii_example PRIVATE MATDevField::ASCII)

if(MSVC)
    target_compile_options(mat_devfield_ascii_example PRIVATE /W4 /permissive- /utf-8)
else()
    target_compile_options(mat_devfield_ascii_example PRIVATE -Wall -Wextra -Wpedantic)
endif()
