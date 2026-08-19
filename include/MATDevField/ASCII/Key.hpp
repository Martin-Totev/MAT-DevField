#pragma once

#include <cstddef>
#include <cstdint>

namespace MAT::DevField::ASCII {

enum class Key : std::uint16_t
{
    Unknown = 0,
    Escape,
    Enter,
    Space,
    Tab,
    Backspace,
    Up,
    Down,
    Left,
    Right,
    A,
    B,
    C,
    D,
    E,
    F,
    G,
    H,
    I,
    J,
    K,
    L,
    M,
    N,
    O,
    P,
    Q,
    R,
    S,
    T,
    U,
    V,
    W,
    X,
    Y,
    Z,
    Number0,
    Number1,
    Number2,
    Number3,
    Number4,
    Number5,
    Number6,
    Number7,
    Number8,
    Number9,
    LeftShift,
    RightShift,
    LeftControl,
    RightControl,
    LeftAlt,
    RightAlt,
    F1,
    F2,
    F3,
    F4,
    F5,
    F6,
    F7,
    F8,
    F9,
    F10,
    F11,
    F12,
    Count
};

[[nodiscard]] constexpr std::size_t KeyIndex(const Key key) noexcept
{
    return static_cast<std::size_t>(key);
}

inline constexpr std::size_t KeyCount = KeyIndex(Key::Count);

} // namespace MAT::DevField::ASCII

