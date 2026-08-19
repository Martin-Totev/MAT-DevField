#pragma once

#include <MATDevField/ASCII/Cell.hpp>
#include <MATDevField/ASCII/Export.hpp>

#include <cstddef>
#include <string_view>

namespace MAT::DevField::ASCII {

class MAT_DEVFIELD_ASCII_API FrameBuffer final
{
public:
    explicit FrameBuffer(std::size_t columns = 80, std::size_t rows = 25);
    ~FrameBuffer();

    FrameBuffer(const FrameBuffer& other);
    FrameBuffer& operator=(const FrameBuffer& other);
    FrameBuffer(FrameBuffer&& other) noexcept;
    FrameBuffer& operator=(FrameBuffer&& other) noexcept;

    void Resize(std::size_t columns, std::size_t rows);

    [[nodiscard]] std::size_t Width() const noexcept;
    [[nodiscard]] std::size_t Height() const noexcept;
    [[nodiscard]] std::size_t Size() const noexcept;
    [[nodiscard]] bool Empty() const noexcept;

    [[nodiscard]] Cell* Data() noexcept;
    [[nodiscard]] const Cell* Data() const noexcept;

    [[nodiscard]] Cell& At(std::size_t x, std::size_t y);
    [[nodiscard]] const Cell& At(std::size_t x, std::size_t y) const;

    void SetCell(std::size_t x, std::size_t y, const Cell& cell);
    [[nodiscard]] bool TrySetCell(std::size_t x, std::size_t y, const Cell& cell) noexcept;

    void Clear(const Cell& fill = Cell{});

    // Writes byte-oriented ASCII. Newlines and four-column tab stops are
    // supported; unsupported bytes are displayed as '?'. Text wraps within the
    // framebuffer and is clipped when it reaches the final row.
    [[nodiscard]] std::size_t WriteText(
        std::size_t x,
        std::size_t y,
        std::string_view text,
        Color foreground = Colors::White,
        Color background = Colors::Black
    );

private:
    [[nodiscard]] std::size_t Index(std::size_t x, std::size_t y) const;

    class Storage;
    Storage* storage_{nullptr};
};

} // namespace MAT::DevField::ASCII
