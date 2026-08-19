# MAT DevField

MAT DevField is intended to be a collection of portable C++ development fields.
The first module is **MAT DevField ASCII**: a native character-cell window with
direct framebuffer access and platform-independent keyboard input. It is, in
essence, a scoped-down pseudoterminal.

The public API contains only MAT DevField and C++ standard-library types. SDL3
is a private implementation detail used to create native windows on Windows,
macOS, Wayland, and X11.

## Current status

This is the initial functional foundation. It provides:

- A native resizable window.
- A fixed-size ASCII cell grid.
- Direct access to a contiguous `Cell` framebuffer.
- Foreground and background RGBA colours.
- ASCII text writing and safe individual-cell access.
- Platform-independent keyboard state, pressed, and released queries.
- A small animated example.
- Non-graphical framebuffer tests.

The initial renderer uses SDL3's built-in 8x8 ASCII debug font. That keeps the
first version small and testable. An MAT-owned texture-atlas renderer can replace
it later without changing the public framebuffer API.

## Requirements

- A C++17 compiler.
- CMake 3.24 or newer.
- The normal desktop development SDK for the target operating system.
- An internet connection during the first configuration when SDL3 is not
  already installed. CMake downloads a pinned SDL 3.4.10 source release and
  builds it privately.

Consumers do not need to install SDL or include SDL headers.

On Linux, compiling the private SDL backend requires development headers for at
least one desktop window system: X11 or Wayland. SDL maintains the current
[Linux dependency list](https://wiki.libsdl.org/SDL3/README-linux). These are
build-time requirements; applications normally load the available desktop
libraries dynamically at runtime.

## Build

```sh
cmake -S . -B build
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
```

Run the example from the configuration's output directory. With a Visual Studio
generator this is normally:

```text
build/examples/Debug/mat_devfield_ascii_example.exe
```

## Use from another CMake project

```cmake
include(FetchContent)

FetchContent_Declare(
    MATDevField
    GIT_REPOSITORY https://github.com/your-name/MATDevField.git
    GIT_TAG v0.1.0
)

FetchContent_MakeAvailable(MATDevField)

target_link_libraries(my_program PRIVATE MATDevField::ASCII)
```

```cpp
#include <MATDevField/ASCII.hpp>

int main()
{
    MAT::MATDevField_A field({80, 25, "My ASCII program", 2, true});

    while (field.IsOpen()) {
        field.PollEvents();
        field.Clear();
        field.WriteText(2, 2, "Hello from MAT DevField ASCII");
        field.Present();
    }
}
```

The window and event functions must be called from the program's main thread.
The framebuffer is row-major, so cell `(x, y)` is at
`buffer[x + y * field.Width()]`.

## Scope and portability

MAT DevField ASCII targets Windows, macOS, and graphical Linux installations
using Wayland or X11. A headless system cannot display a native window, but the
framebuffer itself remains usable and testable without creating one.

Docker and WSL are not runtime requirements. They may be added later as optional
Linux build and test environments.

## License

MAT DevField is available under the [MIT License](LICENSE).
