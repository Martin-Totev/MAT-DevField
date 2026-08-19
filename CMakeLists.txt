cmake_minimum_required(VERSION 3.24)

project(
    MATDevField
    VERSION 0.1.0
    DESCRIPTION "Cross-platform C++ development fields"
    LANGUAGES C CXX
)

include(FetchContent)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

option(MAT_DEVFIELD_BUILD_EXAMPLES "Build MAT DevField examples" ${PROJECT_IS_TOP_LEVEL})
option(MAT_DEVFIELD_BUILD_TESTS "Build MAT DevField tests" ${PROJECT_IS_TOP_LEVEL})
option(MAT_DEVFIELD_FETCH_SDL3 "Download SDL3 when a suitable installation is unavailable" ON)

set(MAT_DEVFIELD_SDL3_VERSION "3.4.10")

# MAT DevField ASCII is a shared library with SDL linked privately. Keeping SDL
# private prevents its types and build requirements from leaking into the public
# C++ API.
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(SDL3 3.4 CONFIG QUIET COMPONENTS SDL3-static)

if(NOT TARGET SDL3::SDL3-static AND MAT_DEVFIELD_FETCH_SDL3)
    set(SDL_SHARED OFF CACHE BOOL "" FORCE)
    set(SDL_STATIC ON CACHE BOOL "" FORCE)
    set(SDL_INSTALL OFF CACHE BOOL "" FORCE)
    set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
    set(SDL_TESTS OFF CACHE BOOL "" FORCE)
    set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)

    # MAT DevField ASCII currently needs SDL's events, video, and renderer only.
    # Disabling unrelated subsystems shortens clean builds and avoids imposing
    # audio, camera, controller, and desktop-integration dependencies on users.
    set(SDL_AUDIO OFF CACHE BOOL "" FORCE)
    set(SDL_CAMERA OFF CACHE BOOL "" FORCE)
    set(SDL_GPU OFF CACHE BOOL "" FORCE)
    set(SDL_JOYSTICK OFF CACHE BOOL "" FORCE)
    set(SDL_HAPTIC OFF CACHE BOOL "" FORCE)
    set(SDL_HIDAPI OFF CACHE BOOL "" FORCE)
    set(SDL_POWER OFF CACHE BOOL "" FORCE)
    set(SDL_SENSOR OFF CACHE BOOL "" FORCE)
    set(SDL_DIALOG OFF CACHE BOOL "" FORCE)
    set(SDL_TRAY OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(
        SDL3
        URL "https://github.com/libsdl-org/SDL/releases/download/release-${MAT_DEVFIELD_SDL3_VERSION}/SDL3-${MAT_DEVFIELD_SDL3_VERSION}.tar.gz"
        URL_HASH SHA256=12b34280415ec8418c864408b93d008a20a6530687ee613d60bfbd20411f2785
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(SDL3)
endif()

if(NOT TARGET SDL3::SDL3-static)
    message(FATAL_ERROR
        "MAT DevField ASCII requires SDL3 3.4 or newer. Install its static CMake package, "
        "or configure with MAT_DEVFIELD_FETCH_SDL3=ON to download the pinned version."
    )
endif()

add_library(MATDevFieldASCII SHARED
    src/ASCII/Field.cpp
    src/ASCII/FrameBuffer.cpp
)
add_library(MATDevField::ASCII ALIAS MATDevFieldASCII)

target_compile_features(MATDevFieldASCII PUBLIC cxx_std_17)
target_link_libraries(MATDevFieldASCII PRIVATE SDL3::SDL3-static)

target_include_directories(MATDevFieldASCII
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_compile_definitions(MATDevFieldASCII
    PRIVATE
        MAT_DEVFIELD_ASCII_BUILDING_LIBRARY=1
        SDL_MAIN_HANDLED=1
)

set_target_properties(MATDevFieldASCII PROPERTIES
    CXX_EXTENSIONS OFF
    CXX_VISIBILITY_PRESET hidden
    EXPORT_NAME ASCII
    VISIBILITY_INLINES_HIDDEN YES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

if(MSVC)
    target_compile_options(MATDevFieldASCII PRIVATE /W4 /permissive- /utf-8)
else()
    target_compile_options(MATDevFieldASCII PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(MAT_DEVFIELD_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(MAT_DEVFIELD_BUILD_TESTS)
    include(CTest)
    if(BUILD_TESTING)
        add_subdirectory(tests)
    endif()
endif()

install(
    TARGETS MATDevFieldASCII
    EXPORT MATDevFieldTargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    FILES LICENSE
    DESTINATION "${CMAKE_INSTALL_DATADIR}/licenses/MATDevField"
)

configure_package_config_file(
    cmake/MATDevFieldConfig.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/MATDevFieldConfig.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/MATDevField"
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/MATDevFieldConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(
    EXPORT MATDevFieldTargets
    FILE MATDevFieldTargets.cmake
    NAMESPACE MATDevField::
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/MATDevField"
)

install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/MATDevFieldConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/MATDevFieldConfigVersion.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/MATDevField"
)
