#pragma once

#include <cstdint>

namespace MAT::DevField::ASCII {

struct Color final
{
    std::uint8_t red{255};
    std::uint8_t green{255};
    std::uint8_t blue{255};
    std::uint8_t alpha{255};
};

[[nodiscard]] constexpr bool operator==(const Color left, const Color right) noexcept
{
    return left.red == right.red
        && left.green == right.green
        && left.blue == right.blue
        && left.alpha == right.alpha;
}

[[nodiscard]] constexpr bool operator!=(const Color left, const Color right) noexcept
{
    return !(left == right);
}

namespace Colors {

inline constexpr Color Black{0, 0, 0, 255};
inline constexpr Color White{255, 255, 255, 255};
inline constexpr Color Gray{170, 170, 170, 255};
inline constexpr Color DarkGray{85, 85, 85, 255};
inline constexpr Color Red{255, 85, 85, 255};
inline constexpr Color Green{85, 255, 85, 255};
inline constexpr Color Blue{85, 85, 255, 255};
inline constexpr Color Cyan{85, 255, 255, 255};
inline constexpr Color Magenta{255, 85, 255, 255};
inline constexpr Color Yellow{255, 255, 85, 255};
inline constexpr Color Transparent{0, 0, 0, 0};

} // namespace Colors

} // namespace MAT::DevField::ASCII

