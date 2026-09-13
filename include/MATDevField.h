#pragma once

// Visual Studio consumers only need to provide the directory containing
// MATDevFieldASCII.lib. Including this header selects the library and the
// Windows system dependencies automatically.
#if defined(_MSC_VER) && !defined(MAT_DEVFIELD_DISABLE_AUTO_LINK)
    #pragma comment(lib, "MATDevFieldASCII.lib")
    #pragma comment(lib, "advapi32.lib")
    #pragma comment(lib, "gdi32.lib")
    #pragma comment(lib, "imm32.lib")
    #pragma comment(lib, "ole32.lib")
    #pragma comment(lib, "oleaut32.lib")
    #pragma comment(lib, "setupapi.lib")
    #pragma comment(lib, "shell32.lib")
    #pragma comment(lib, "user32.lib")
    #pragma comment(lib, "uuid.lib")
    #pragma comment(lib, "version.lib")
    #pragma comment(lib, "winmm.lib")
#endif

#include <MATDevField/ASCII.hpp>
