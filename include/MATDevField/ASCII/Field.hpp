#pragma once

#include <MATDevField/ASCII/Export.hpp>
#include <MATDevField/ASCII/FrameBuffer.hpp>
#include <MATDevField/ASCII/Key.hpp>

#include <cstddef>
#include <string>
#include <string_view>

namespace MAT::DevField::ASCII {

struct FieldConfig final
{
    std::size_t columns{80};
    std::size_t rows{25};
    std::string title{"MAT DevField ASCII"};
    unsigned int windowScale{1};
    bool resizable{true};
};

class MAT_DEVFIELD_ASCII_API Field final
{
public:
    explicit Field(const FieldConfig& config = FieldConfig{});
    Field(
        std::size_t columns,
        std::size_t rows,
        std::string title = "MAT DevField ASCII",
        unsigned int windowScale = 1
    );

    ~Field();

    Field(const Field&) = delete;
    Field& operator=(const Field&) = delete;
    Field(Field&& other) noexcept;
    Field& operator=(Field&& other) noexcept;

    [[nodiscard]] bool IsOpen() const noexcept;
    void Close() noexcept;

    // Window, event, and presentation functions must be called on the thread
    // that created this object (normally the application's main thread).
    void PollEvents();
    void Present();

    [[nodiscard]] bool KeyDown(Key key) const noexcept;
    [[nodiscard]] bool KeyPressed(Key key) const noexcept;
    [[nodiscard]] bool KeyReleased(Key key) const noexcept;

    [[nodiscard]] std::size_t Width() const noexcept;
    [[nodiscard]] std::size_t Height() const noexcept;
    [[nodiscard]] std::size_t BufferSize() const noexcept;

    [[nodiscard]] Cell* Buffer() noexcept;
    [[nodiscard]] const Cell* Buffer() const noexcept;

    [[nodiscard]] FrameBuffer& Frame();
    [[nodiscard]] const FrameBuffer& Frame() const;

    [[nodiscard]] Cell& At(std::size_t x, std::size_t y);
    [[nodiscard]] const Cell& At(std::size_t x, std::size_t y) const;

    void Clear(const Cell& fill = Cell{});
    void SetCell(std::size_t x, std::size_t y, const Cell& cell);
    [[nodiscard]] bool TrySetCell(std::size_t x, std::size_t y, const Cell& cell) noexcept;
    [[nodiscard]] std::size_t WriteText(
        std::size_t x,
        std::size_t y,
        std::string_view text,
        Color foreground = Colors::White,
        Color background = Colors::Black
    );

    void SetTitle(std::string_view title);

private:
    class Impl;
    [[nodiscard]] Impl& RequireImpl();
    [[nodiscard]] const Impl& RequireImpl() const;

    Impl* impl_{nullptr};
};

} // namespace MAT::DevField::ASCII

namespace MAT {

// Concise public name requested for the MAT ASCII Development Field.
using MATDevField_A = DevField::ASCII::Field;

} // namespace MAT
