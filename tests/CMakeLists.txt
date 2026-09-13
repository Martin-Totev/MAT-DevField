add_executable(mat_devfield_ascii_tests ascii_framebuffer_tests.cpp)

target_compile_features(mat_devfield_ascii_tests PRIVATE cxx_std_17)
target_link_libraries(mat_devfield_ascii_tests PRIVATE MATDevField::ASCII)

if(MSVC)
    target_compile_options(mat_devfield_ascii_tests PRIVATE /W4 /permissive- /utf-8)
else()
    target_compile_options(mat_devfield_ascii_tests PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(NAME MATDevField.ASCII.FrameBuffer COMMAND mat_devfield_ascii_tests)

add_executable(mat_devfield_ascii_window_smoke_tests ascii_window_smoke_tests.cpp)

target_compile_features(mat_devfield_ascii_window_smoke_tests PRIVATE cxx_std_17)
target_link_libraries(mat_devfield_ascii_window_smoke_tests PRIVATE MATDevField::ASCII)

if(MSVC)
    target_compile_options(mat_devfield_ascii_window_smoke_tests PRIVATE /W4 /permissive- /utf-8)
else()
    target_compile_options(mat_devfield_ascii_window_smoke_tests PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(NAME MATDevField.ASCII.WindowSmoke COMMAND mat_devfield_ascii_window_smoke_tests)
set_tests_properties(
    MATDevField.ASCII.WindowSmoke
    PROPERTIES ENVIRONMENT "SDL_VIDEODRIVER=offscreen"
)
