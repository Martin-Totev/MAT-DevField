#include <MATDevField/ASCII/Field.hpp>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <array>
#include <limits>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <utility>

namespace MAT::DevField::ASCII {

namespace {

constexpr int GlyphWidth = 8;
constexpr int GlyphHeight = 8;

std::mutex SdlLifetimeMutex;
std::size_t SdlVideoUsers = 0;

[[nodiscard]] std::runtime_error SdlError(const char* operation)
{
    return std::runtime_error(std::string(operation) + ": " + SDL_GetError());
}

void RequireSdl(const bool succeeded, const char* operation)
{
    if (!succeeded) {
        throw SdlError(operation);
    }
}

class SdlVideoGuard final
{
public:
    SdlVideoGuard()
    {
        std::lock_guard<std::mutex> lock(SdlLifetimeMutex);

        if (SdlVideoUsers == 0) {
            SDL_SetMainReady();
            RequireSdl(SDL_Init(SDL_INIT_VIDEO), "Could not initialise SDL video");
        }

        ++SdlVideoUsers;
    }

    ~SdlVideoGuard()
    {
        std::lock_guard<std::mutex> lock(SdlLifetimeMutex);

        if (SdlVideoUsers == 0) {
            return;
        }

        --SdlVideoUsers;
        if (SdlVideoUsers == 0) {
            SDL_QuitSubSystem(SDL_INIT_VIDEO);
        }
    }

    SdlVideoGuard(const SdlVideoGuard&) = delete;
    SdlVideoGuard& operator=(const SdlVideoGuard&) = delete;
};

[[nodiscard]] int CheckedPixelDimension(
    const std::size_t cells,
    const int glyphPixels,
    const unsigned int scale
)
{
    if (scale == 0) {
        throw std::invalid_argument("MAT DevField windowScale must be at least one.");
    }

    const auto limit = static_cast<std::size_t>(std::numeric_limits<int>::max());
    if (cells > limit / static_cast<std::size_t>(glyphPixels)
        || cells * static_cast<std::size_t>(glyphPixels) > limit / scale) {
        throw std::length_error("The requested MAT DevField window is too large.");
    }

    return static_cast<int>(cells * static_cast<std::size_t>(glyphPixels) * scale);
}

[[nodiscard]] SDL_Scancode ToSdlScancode(const Key key) noexcept
{
    switch (key) {
    case Key::Escape: return SDL_SCANCODE_ESCAPE;
    case Key::Enter: return SDL_SCANCODE_RETURN;
    case Key::Space: return SDL_SCANCODE_SPACE;
    case Key::Tab: return SDL_SCANCODE_TAB;
    case Key::Backspace: return SDL_SCANCODE_BACKSPACE;
    case Key::Up: return SDL_SCANCODE_UP;
    case Key::Down: return SDL_SCANCODE_DOWN;
    case Key::Left: return SDL_SCANCODE_LEFT;
    case Key::Right: return SDL_SCANCODE_RIGHT;
    case Key::A: return SDL_SCANCODE_A;
    case Key::B: return SDL_SCANCODE_B;
    case Key::C: return SDL_SCANCODE_C;
    case Key::D: return SDL_SCANCODE_D;
    case Key::E: return SDL_SCANCODE_E;
    case Key::F: return SDL_SCANCODE_F;
    case Key::G: return SDL_SCANCODE_G;
    case Key::H: return SDL_SCANCODE_H;
    case Key::I: return SDL_SCANCODE_I;
    case Key::J: return SDL_SCANCODE_J;
    case Key::K: return SDL_SCANCODE_K;
    case Key::L: return SDL_SCANCODE_L;
    case Key::M: return SDL_SCANCODE_M;
    case Key::N: return SDL_SCANCODE_N;
    case Key::O: return SDL_SCANCODE_O;
    case Key::P: return SDL_SCANCODE_P;
    case Key::Q: return SDL_SCANCODE_Q;
    case Key::R: return SDL_SCANCODE_R;
    case Key::S: return SDL_SCANCODE_S;
    case Key::T: return SDL_SCANCODE_T;
    case Key::U: return SDL_SCANCODE_U;
    case Key::V: return SDL_SCANCODE_V;
    case Key::W: return SDL_SCANCODE_W;
    case Key::X: return SDL_SCANCODE_X;
    case Key::Y: return SDL_SCANCODE_Y;
    case Key::Z: return SDL_SCANCODE_Z;
    case Key::Number0: return SDL_SCANCODE_0;
    case Key::Number1: return SDL_SCANCODE_1;
    case Key::Number2: return SDL_SCANCODE_2;
    case Key::Number3: return SDL_SCANCODE_3;
    case Key::Number4: return SDL_SCANCODE_4;
    case Key::Number5: return SDL_SCANCODE_5;
    case Key::Number6: return SDL_SCANCODE_6;
    case Key::Number7: return SDL_SCANCODE_7;
    case Key::Number8: return SDL_SCANCODE_8;
    case Key::Number9: return SDL_SCANCODE_9;
    case Key::LeftShift: return SDL_SCANCODE_LSHIFT;
    case Key::RightShift: return SDL_SCANCODE_RSHIFT;
    case Key::LeftControl: return SDL_SCANCODE_LCTRL;
    case Key::RightControl: return SDL_SCANCODE_RCTRL;
    case Key::LeftAlt: return SDL_SCANCODE_LALT;
    case Key::RightAlt: return SDL_SCANCODE_RALT;
    case Key::F1: return SDL_SCANCODE_F1;
    case Key::F2: return SDL_SCANCODE_F2;
    case Key::F3: return SDL_SCANCODE_F3;
    case Key::F4: return SDL_SCANCODE_F4;
    case Key::F5: return SDL_SCANCODE_F5;
    case Key::F6: return SDL_SCANCODE_F6;
    case Key::F7: return SDL_SCANCODE_F7;
    case Key::F8: return SDL_SCANCODE_F8;
    case Key::F9: return SDL_SCANCODE_F9;
    case Key::F10: return SDL_SCANCODE_F10;
    case Key::F11: return SDL_SCANCODE_F11;
    case Key::F12: return SDL_SCANCODE_F12;
    case Key::Unknown:
    case Key::Count:
        return SDL_SCANCODE_UNKNOWN;
    }

    return SDL_SCANCODE_UNKNOWN;
}

} // namespace

class Field::Impl final
{
public:
    explicit Impl(const FieldConfig& config)
        : frame_(config.columns, config.rows), ownerThread_(std::this_thread::get_id())
    {
        const int logicalWidth = CheckedPixelDimension(config.columns, GlyphWidth, 1);
        const int logicalHeight = CheckedPixelDimension(config.rows, GlyphHeight, 1);
        const int windowWidth = CheckedPixelDimension(config.columns, GlyphWidth, config.windowScale);
        const int windowHeight = CheckedPixelDimension(config.rows, GlyphHeight, config.windowScale);

        SDL_WindowFlags flags = SDL_WINDOW_HIGH_PIXEL_DENSITY;
        if (config.resizable) {
            flags |= SDL_WINDOW_RESIZABLE;
        }

        if (!SDL_CreateWindowAndRenderer(
                config.title.c_str(),
                windowWidth,
                windowHeight,
                flags,
                &window_,
                &renderer_)) {
            if (renderer_ != nullptr) {
                SDL_DestroyRenderer(renderer_);
                renderer_ = nullptr;
            }
            if (window_ != nullptr) {
                SDL_DestroyWindow(window_);
                window_ = nullptr;
            }
            throw SdlError("Could not create the MAT DevField ASCII window");
        }

        try {
            RequireSdl(
                SDL_SetRenderLogicalPresentation(
                    renderer_,
                    logicalWidth,
                    logicalHeight,
                    SDL_LOGICAL_PRESENTATION_LETTERBOX),
                "Could not configure MAT DevField logical presentation"
            );
            logicalWidth_ = logicalWidth;
            logicalHeight_ = logicalHeight;
            RequireSdl(
                SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND),
                "Could not configure MAT DevField colour blending"
            );
        } catch (...) {
            SDL_DestroyRenderer(renderer_);
            SDL_DestroyWindow(window_);
            renderer_ = nullptr;
            window_ = nullptr;
            throw;
        }

        windowId_ = SDL_GetWindowID(window_);
    }

    ~Impl()
    {
        if (renderer_ != nullptr) {
            SDL_DestroyRenderer(renderer_);
        }
        if (window_ != nullptr) {
            SDL_DestroyWindow(window_);
        }
    }

    void EnsureOwnerThread() const
    {
        if (std::this_thread::get_id() != ownerThread_) {
            throw std::logic_error(
                "MAT DevField window and event functions must be called from their creating thread."
            );
        }
    }

    void PollEvents()
    {
        EnsureOwnerThread();
        previousKeys_ = currentKeys_;

        SDL_Event event{};
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                open_ = false;
            } else if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED
                && event.window.windowID == windowId_) {
                open_ = false;
            }
        }

        int keyboardSize = 0;
        const bool* keyboard = SDL_GetKeyboardState(&keyboardSize);

        for (std::size_t index = 0; index < KeyCount; ++index) {
            const auto key = static_cast<Key>(index);
            const SDL_Scancode scancode = ToSdlScancode(key);
            const int scancodeIndex = static_cast<int>(scancode);

            currentKeys_[index] = scancode != SDL_SCANCODE_UNKNOWN
                && scancodeIndex >= 0
                && scancodeIndex < keyboardSize
                && keyboard[scancodeIndex];
        }
    }

    void Present()
    {
        EnsureOwnerThread();

        // Frame().Resize() changes the canvas, not the native window size.
        // Refresh the mapping so the complete new framebuffer still fits.
        const int logicalWidth = CheckedPixelDimension(frame_.Width(), GlyphWidth, 1);
        const int logicalHeight = CheckedPixelDimension(frame_.Height(), GlyphHeight, 1);
        if (logicalWidth != logicalWidth_ || logicalHeight != logicalHeight_) {
            RequireSdl(
                SDL_SetRenderLogicalPresentation(renderer_, logicalWidth, logicalHeight,
                    SDL_LOGICAL_PRESENTATION_LETTERBOX),
                "Could not resize MAT DevField logical presentation"
            );
            logicalWidth_ = logicalWidth;
            logicalHeight_ = logicalHeight;
        }

        RequireSdl(
            SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255),
            "Could not set the MAT DevField canvas colour"
        );
        RequireSdl(SDL_RenderClear(renderer_), "Could not clear the MAT DevField renderer");

        Color activeColor{};
        bool hasActiveColor = false;

        const auto setDrawColor = [&](const Color color) {
            if (hasActiveColor && color == activeColor) {
                return;
            }

            RequireSdl(
                SDL_SetRenderDrawColor(
                    renderer_, color.red, color.green, color.blue, color.alpha),
                "Could not set a MAT DevField cell colour"
            );
            activeColor = color;
            hasActiveColor = true;
        };

        for (std::size_t y = 0; y < frame_.Height(); ++y) {
            for (std::size_t x = 0; x < frame_.Width(); ++x) {
                const Cell& cell = frame_.At(x, y);
                if (cell.background.alpha == 0) {
                    continue;
                }

                setDrawColor(cell.background);
                const SDL_FRect rectangle{
                    static_cast<float>(x * GlyphWidth),
                    static_cast<float>(y * GlyphHeight),
                    static_cast<float>(GlyphWidth),
                    static_cast<float>(GlyphHeight)
                };
                RequireSdl(
                    SDL_RenderFillRect(renderer_, &rectangle),
                    "Could not draw a MAT DevField cell background"
                );
            }
        }

        hasActiveColor = false;
        for (std::size_t y = 0; y < frame_.Height(); ++y) {
            for (std::size_t x = 0; x < frame_.Width(); ++x) {
                const Cell& cell = frame_.At(x, y);
                if (cell.glyph == U' ' || cell.glyph == U'\0' || cell.foreground.alpha == 0) {
                    continue;
                }

                const char glyph[2]{
                    cell.glyph >= U' ' && cell.glyph <= U'~'
                        ? static_cast<char>(cell.glyph)
                        : '?',
                    '\0'
                };

                setDrawColor(cell.foreground);
                RequireSdl(
                    SDL_RenderDebugText(
                        renderer_,
                        static_cast<float>(x * GlyphWidth),
                        static_cast<float>(y * GlyphHeight),
                        glyph),
                    "Could not draw a MAT DevField ASCII glyph"
                );
            }
        }

        RequireSdl(SDL_RenderPresent(renderer_), "Could not present the MAT DevField frame");
    }

    [[nodiscard]] bool KeyDown(const Key key) const noexcept
    {
        const std::size_t index = KeyIndex(key);
        return index < KeyCount && currentKeys_[index];
    }

    [[nodiscard]] bool KeyPressed(const Key key) const noexcept
    {
        const std::size_t index = KeyIndex(key);
        return index < KeyCount && currentKeys_[index] && !previousKeys_[index];
    }

    [[nodiscard]] bool KeyReleased(const Key key) const noexcept
    {
        const std::size_t index = KeyIndex(key);
        return index < KeyCount && !currentKeys_[index] && previousKeys_[index];
    }

    void SetTitle(const std::string_view title)
    {
        EnsureOwnerThread();
        const std::string ownedTitle(title);
        RequireSdl(
            SDL_SetWindowTitle(window_, ownedTitle.c_str()),
            "Could not set the MAT DevField window title"
        );
    }

private:
    friend class Field;

    SdlVideoGuard sdlVideo_{};
    FrameBuffer frame_;
    std::thread::id ownerThread_;
    SDL_Window* window_{nullptr};
    SDL_Renderer* renderer_{nullptr};
    SDL_WindowID windowId_{0};
    int logicalWidth_{0};
    int logicalHeight_{0};
    std::array<bool, KeyCount> currentKeys_{};
    std::array<bool, KeyCount> previousKeys_{};
    bool open_{true};
};

Field::Field(const FieldConfig& config)
    : impl_(new Impl(config))
{
}

Field::Field(
    const std::size_t columns,
    const std::size_t rows,
    std::string title,
    const unsigned int windowScale
)
    : Field(FieldConfig{columns, rows, std::move(title), windowScale, true})
{
}

Field::~Field()
{
    delete impl_;
}

Field::Field(Field&& other) noexcept
    : impl_(std::exchange(other.impl_, nullptr))
{
}

Field& Field::operator=(Field&& other) noexcept
{
    if (this == &other) {
        return *this;
    }

    delete impl_;
    impl_ = std::exchange(other.impl_, nullptr);
    return *this;
}

bool Field::IsOpen() const noexcept
{
    return impl_ != nullptr && impl_->open_;
}

void Field::Close() noexcept
{
    if (impl_ != nullptr) {
        impl_->open_ = false;
    }
}

void Field::PollEvents()
{
    RequireImpl().PollEvents();
}

void Field::Present()
{
    RequireImpl().Present();
}

bool Field::KeyDown(const Key key) const noexcept
{
    return impl_ != nullptr && impl_->KeyDown(key);
}

bool Field::KeyPressed(const Key key) const noexcept
{
    return impl_ != nullptr && impl_->KeyPressed(key);
}

bool Field::KeyReleased(const Key key) const noexcept
{
    return impl_ != nullptr && impl_->KeyReleased(key);
}

std::size_t Field::Width() const noexcept
{
    return impl_ != nullptr ? impl_->frame_.Width() : 0;
}

std::size_t Field::Height() const noexcept
{
    return impl_ != nullptr ? impl_->frame_.Height() : 0;
}

std::size_t Field::BufferSize() const noexcept
{
    return impl_ != nullptr ? impl_->frame_.Size() : 0;
}

Cell* Field::Buffer() noexcept
{
    return impl_ != nullptr ? impl_->frame_.Data() : nullptr;
}

const Cell* Field::Buffer() const noexcept
{
    return impl_ != nullptr ? impl_->frame_.Data() : nullptr;
}

FrameBuffer& Field::Frame()
{
    return RequireImpl().frame_;
}

const FrameBuffer& Field::Frame() const
{
    return RequireImpl().frame_;
}

Cell& Field::At(const std::size_t x, const std::size_t y)
{
    return RequireImpl().frame_.At(x, y);
}

const Cell& Field::At(const std::size_t x, const std::size_t y) const
{
    return RequireImpl().frame_.At(x, y);
}

void Field::Clear(const Cell& fill)
{
    RequireImpl().frame_.Clear(fill);
}

void Field::SetCell(const std::size_t x, const std::size_t y, const Cell& cell)
{
    RequireImpl().frame_.SetCell(x, y, cell);
}

bool Field::TrySetCell(
    const std::size_t x,
    const std::size_t y,
    const Cell& cell
) noexcept
{
    return impl_ != nullptr && impl_->frame_.TrySetCell(x, y, cell);
}

std::size_t Field::WriteText(
    const std::size_t x,
    const std::size_t y,
    const std::string_view text,
    const Color foreground,
    const Color background
)
{
    return RequireImpl().frame_.WriteText(x, y, text, foreground, background);
}

void Field::SetTitle(const std::string_view title)
{
    RequireImpl().SetTitle(title);
}

Field::Impl& Field::RequireImpl()
{
    if (impl_ == nullptr) {
        throw std::logic_error("This MAT DevField object has been moved from.");
    }

    return *impl_;
}

const Field::Impl& Field::RequireImpl() const
{
    if (impl_ == nullptr) {
        throw std::logic_error("This MAT DevField object has been moved from.");
    }

    return *impl_;
}

} // namespace MAT::DevField::ASCII
