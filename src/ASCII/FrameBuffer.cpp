#include <MATDevField/ASCII/FrameBuffer.hpp>

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

namespace MAT::DevField::ASCII {

namespace {

[[nodiscard]] std::size_t CheckedCellCount(const std::size_t columns, const std::size_t rows)
{
    if (columns == 0 || rows == 0) {
        throw std::invalid_argument("A MAT DevField framebuffer must have non-zero dimensions.");
    }

    if (rows > std::numeric_limits<std::size_t>::max() / columns) {
        throw std::length_error("The requested MAT DevField framebuffer is too large.");
    }

    return columns * rows;
}

} // namespace

class FrameBuffer::Storage final
{
public:
    std::size_t columns{0};
    std::size_t rows{0};
    std::vector<Cell> cells{};
};

FrameBuffer::FrameBuffer(const std::size_t columns, const std::size_t rows)
    : storage_(new Storage{})
{
    Resize(columns, rows);
}

FrameBuffer::~FrameBuffer()
{
    delete storage_;
}

FrameBuffer::FrameBuffer(const FrameBuffer& other)
    : storage_(other.storage_ != nullptr ? new Storage(*other.storage_) : nullptr)
{
}

FrameBuffer& FrameBuffer::operator=(const FrameBuffer& other)
{
    if (this == &other) {
        return *this;
    }

    Storage* replacement = other.storage_ != nullptr ? new Storage(*other.storage_) : nullptr;
    delete storage_;
    storage_ = replacement;
    return *this;
}

FrameBuffer::FrameBuffer(FrameBuffer&& other) noexcept
    : storage_(std::exchange(other.storage_, nullptr))
{
}

FrameBuffer& FrameBuffer::operator=(FrameBuffer&& other) noexcept
{
    if (this == &other) {
        return *this;
    }

    delete storage_;
    storage_ = std::exchange(other.storage_, nullptr);
    return *this;
}

void FrameBuffer::Resize(const std::size_t columns, const std::size_t rows)
{
    const auto cellCount = CheckedCellCount(columns, rows);
    std::vector<Cell> replacement(cellCount);

    if (storage_ == nullptr) {
        storage_ = new Storage{};
    }

    storage_->columns = columns;
    storage_->rows = rows;
    storage_->cells = std::move(replacement);
}

std::size_t FrameBuffer::Width() const noexcept
{
    return storage_ != nullptr ? storage_->columns : 0;
}

std::size_t FrameBuffer::Height() const noexcept
{
    return storage_ != nullptr ? storage_->rows : 0;
}

std::size_t FrameBuffer::Size() const noexcept
{
    return storage_ != nullptr ? storage_->cells.size() : 0;
}

bool FrameBuffer::Empty() const noexcept
{
    return storage_ == nullptr || storage_->cells.empty();
}

Cell* FrameBuffer::Data() noexcept
{
    return storage_ != nullptr ? storage_->cells.data() : nullptr;
}

const Cell* FrameBuffer::Data() const noexcept
{
    return storage_ != nullptr ? storage_->cells.data() : nullptr;
}

Cell& FrameBuffer::At(const std::size_t x, const std::size_t y)
{
    const std::size_t index = Index(x, y);
    return storage_->cells.at(index);
}

const Cell& FrameBuffer::At(const std::size_t x, const std::size_t y) const
{
    const std::size_t index = Index(x, y);
    return storage_->cells.at(index);
}

void FrameBuffer::SetCell(const std::size_t x, const std::size_t y, const Cell& cell)
{
    At(x, y) = cell;
}

bool FrameBuffer::TrySetCell(const std::size_t x, const std::size_t y, const Cell& cell) noexcept
{
    if (storage_ == nullptr || x >= storage_->columns || y >= storage_->rows) {
        return false;
    }

    storage_->cells[x + y * storage_->columns] = cell;
    return true;
}

void FrameBuffer::Clear(const Cell& fill)
{
    if (storage_ != nullptr) {
        std::fill(storage_->cells.begin(), storage_->cells.end(), fill);
    }
}

std::size_t FrameBuffer::WriteText(
    const std::size_t x,
    const std::size_t y,
    const std::string_view text,
    const Color foreground,
    const Color background
)
{
    if (storage_ == nullptr || x >= storage_->columns || y >= storage_->rows) {
        return 0;
    }

    std::size_t cursorX = x;
    std::size_t cursorY = y;
    std::size_t cellsWritten = 0;

    const auto moveToNextLine = [&]() {
        cursorX = x;
        ++cursorY;
    };

    const auto writeGlyph = [&](const char32_t glyph) {
        if (cursorX >= storage_->columns) {
            moveToNextLine();
        }

        if (cursorY >= storage_->rows) {
            return false;
        }

        storage_->cells[cursorX + cursorY * storage_->columns] = Cell{glyph, foreground, background};
        ++cursorX;
        ++cellsWritten;
        return true;
    };

    for (const unsigned char byte : text) {
        if (byte == '\r') {
            continue;
        }

        if (byte == '\n') {
            moveToNextLine();
            if (cursorY >= storage_->rows) {
                break;
            }
            continue;
        }

        if (byte == '\t') {
            const std::size_t spaces = 4 - (cursorX % 4);
            for (std::size_t index = 0; index < spaces; ++index) {
                if (!writeGlyph(U' ')) {
                    return cellsWritten;
                }
            }
            continue;
        }

        const char32_t glyph = byte >= 32 && byte <= 126
            ? static_cast<char32_t>(byte)
            : U'?';

        if (!writeGlyph(glyph)) {
            break;
        }
    }

    return cellsWritten;
}

std::size_t FrameBuffer::Index(const std::size_t x, const std::size_t y) const
{
    if (storage_ == nullptr || x >= storage_->columns || y >= storage_->rows) {
        throw std::out_of_range("MAT DevField framebuffer coordinates are outside the grid.");
    }

    return x + y * storage_->columns;
}

} // namespace MAT::DevField::ASCII
