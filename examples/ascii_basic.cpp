#include <MATDevField/ASCII.hpp>

#include <algorithm>
#include <chrono>
#include <exception>
#include <iostream>
#include <string>
#include <thread>

int main()
{
    using MAT::DevField::ASCII::Cell;
    using MAT::DevField::ASCII::Colors::Black;
    using MAT::DevField::ASCII::Colors::Cyan;
    using MAT::DevField::ASCII::Colors::DarkGray;
    using MAT::DevField::ASCII::Colors::Green;
    using MAT::DevField::ASCII::Colors::White;
    using MAT::DevField::ASCII::Colors::Yellow;
    using MAT::DevField::ASCII::Key;

    try {
        MAT::MATDevField_A field({80, 30, "MAT DevField ASCII - basic example", 1, true});

        std::size_t playerX = field.Width() / 2;
        std::size_t playerY = field.Height() / 2;
        std::size_t frameNumber = 0;

        while (field.IsOpen()) {
            field.PollEvents();

            if (field.KeyPressed(Key::Escape)) {
                field.Close();
            }
            if (!field.IsOpen()) {
                break;
            }

            if (field.KeyDown(Key::Left) || field.KeyDown(Key::A)) {
                playerX = std::max<std::size_t>(1, playerX - 1);
            }
            if ((field.KeyDown(Key::Right) || field.KeyDown(Key::D))
                && playerX + 2 < field.Width()) {
                ++playerX;
            }
            if (field.KeyDown(Key::Up) || field.KeyDown(Key::W)) {
                playerY = std::max<std::size_t>(3, playerY - 1);
            }
            if ((field.KeyDown(Key::Down) || field.KeyDown(Key::S))
                && playerY + 2 < field.Height()) {
                ++playerY;
            }

            field.Clear(Cell{U' ', White, Black});

            const Cell border{U'#', DarkGray, Black};
            for (std::size_t x = 0; x < field.Width(); ++x) {
                field.At(x, 0) = border;
                field.At(x, field.Height() - 1) = border;
            }
            for (std::size_t y = 0; y < field.Height(); ++y) {
                field.At(0, y) = border;
                field.At(field.Width() - 1, y) = border;
            }

            (void)field.WriteText(2, 1, "MAT DevField ASCII", Cyan, Black);
            (void)field.WriteText(2, 2, "Move: arrows/WASD    Exit: Escape", Green, Black);
            (void)field.WriteText(
                field.Width() - 18,
                1,
                "Frame " + std::to_string(frameNumber),
                White,
                Black
            );

            // This assignment writes directly into the same contiguous buffer
            // returned by field.Buffer(). Present() displays that complete buffer.
            field.Buffer()[playerX + playerY * field.Width()] = Cell{U'@', Yellow, Black};

            field.Present();
            ++frameNumber;
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }
    } catch (const std::exception& error) {
        std::cerr << "MAT DevField example failed: " << error.what() << '\n';
        return 1;
    }

    return 0;
}

