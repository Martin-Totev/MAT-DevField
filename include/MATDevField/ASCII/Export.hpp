#pragma once

#if defined(_WIN32) || defined(__CYGWIN__)
    #if defined(MAT_DEVFIELD_ASCII_BUILDING_LIBRARY)
        #define MAT_DEVFIELD_ASCII_API __declspec(dllexport)
    #else
        #define MAT_DEVFIELD_ASCII_API __declspec(dllimport)
    #endif
#elif defined(__GNUC__) || defined(__clang__)
    #define MAT_DEVFIELD_ASCII_API __attribute__((visibility("default")))
#else
    #define MAT_DEVFIELD_ASCII_API
#endif

