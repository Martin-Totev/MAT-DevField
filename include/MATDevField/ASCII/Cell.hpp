#pragma once

#include <MATDevField/ASCII/Color.hpp>

#include <type_traits>

namespace MAT::DevField::ASCII {

struct Cell final
{
    char32_t glyph{U' '};
    Color foreground{Colors::White};
    Color background{Colors::Black};
};

static_assert(std::is_standard_layout_v<Cell>, "Cell must remain suitable for contiguous buffers.");
static_assert(std::is_trivially_copyable_v<Cell>, "Cell must remain cheap to copy and GPU-transfer friendly.");

} // namespace MAT::DevField::ASCII

