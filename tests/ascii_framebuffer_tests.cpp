#include <MATDevField/ASCII.hpp>

#include <exception>
#include <iostream>
#include <stdexcept>
#include <string_view>
#include <type_traits>

namespace {

int Failures = 0;

void Check(const bool condition, const std::string_view description)
{
    if (condition) {
        return;
    }

    ++Failures;
    std::cerr << "FAILED: " << description << '\n';
}

void TestConstructionAndLayout()
{
    using MAT::DevField::ASCII::Cell;
    using MAT::DevField::ASCII::FrameBuffer;

    static_assert(std::is_trivially_copyable_v<Cell>);
    static_assert(sizeof(Cell) == 12, "The initial MAT ASCII cell layout should remain compact.");

    FrameBuffer frame(4, 3);
    Check(frame.Width() == 4, "Width is retained");
    Check(frame.Height() == 3, "Height is retained");
    Check(frame.Size() == 12, "Cell count is width times height");
    Check(frame.Data() != nullptr, "Contiguous buffer is available");

    frame.Data()[2 + frame.Width()] = Cell{U'@'};
    Check(frame.At(2, 1).glyph == U'@', "Direct row-major buffer writes are visible through At");
}

void TestClearAndBounds()
{
    using namespace MAT::DevField::ASCII;

    FrameBuffer frame(3, 2);
    const Cell fill{U'.', Colors::Yellow, Colors::Blue};
    frame.Clear(fill);

    for (std::size_t index = 0; index < frame.Size(); ++index) {
        Check(frame.Data()[index].glyph == U'.', "Clear fills every glyph");
        Check(frame.Data()[index].foreground == Colors::Yellow, "Clear fills every foreground");
        Check(frame.Data()[index].background == Colors::Blue, "Clear fills every background");
    }

    Check(!frame.TrySetCell(3, 0, Cell{}), "TrySetCell rejects an invalid column");
    Check(!frame.TrySetCell(0, 2, Cell{}), "TrySetCell rejects an invalid row");
    Check(frame.TrySetCell(2, 1, Cell{U'!'}), "TrySetCell accepts valid coordinates");

    bool threw = false;
    try {
        (void)frame.At(3, 0);
    } catch (const std::out_of_range&) {
        threw = true;
    }
    Check(threw, "At reports coordinates outside the framebuffer");
}

void TestTextWriting()
{
    using namespace MAT::DevField::ASCII;

    FrameBuffer frame(5, 3);
    frame.Clear();

    const std::size_t written = frame.WriteText(3, 0, "ABCD", Colors::Green, Colors::Black);
    Check(written == 4, "WriteText reports the number of written cells");
    Check(frame.At(3, 0).glyph == U'A', "Text starts at the requested column");
    Check(frame.At(4, 0).glyph == U'B', "Text reaches the end of a row");
    Check(frame.At(3, 1).glyph == U'C', "Text wraps to its starting column");
    Check(frame.At(4, 1).glyph == U'D', "Wrapped text continues normally");
    Check(frame.At(3, 0).foreground == Colors::Green, "WriteText applies its foreground colour");

    frame.Clear();
    (void)frame.WriteText(1, 0, "X\nY");
    Check(frame.At(1, 0).glyph == U'X', "Text is written before a newline");
    Check(frame.At(1, 1).glyph == U'Y', "A newline advances to the next row");
}

void TestInvalidDimensions()
{
    using MAT::DevField::ASCII::FrameBuffer;

    bool threw = false;
    try {
        FrameBuffer invalid(0, 1);
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    Check(threw, "Zero-width framebuffers are rejected");
}

} // namespace

int main()
{
    TestConstructionAndLayout();
    TestClearAndBounds();
    TestTextWriting();
    TestInvalidDimensions();

    if (Failures != 0) {
        std::cerr << Failures << " MAT DevField ASCII test(s) failed.\n";
        return 1;
    }

    std::cout << "All MAT DevField ASCII framebuffer tests passed.\n";
    return 0;
}

