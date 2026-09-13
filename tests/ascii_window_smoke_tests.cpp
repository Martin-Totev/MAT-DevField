#include <MATDevField/ASCII.hpp>

#include <exception>
#include <iostream>

int main()
{
    using namespace MAT::DevField::ASCII;

    try {
        MAT::MATDevField_A field({20, 8, "MAT DevField offscreen smoke test", 1, true});
        (void)field.WriteText(1, 1, "Window backend", Colors::Cyan, Colors::Black);
        field.SetCell(1, 3, Cell{U'@', Colors::Yellow, Colors::Blue});
        field.PollEvents();
        field.Present();
        field.SetTitle("MAT DevField smoke test complete");

        if (!field.IsOpen() || field.Buffer() == nullptr || field.BufferSize() != 160) {
            std::cerr << "The MAT DevField window did not retain a valid framebuffer.\n";
            return 1;
        }

        // Changing canvas resolution must continue to present successfully,
        // including cells at the new far edge, without recreating the window.
        field.Frame().Resize(100, 40);
        field.Clear();
        field.SetCell(99, 39, Cell{U'@', Colors::Yellow, Colors::Blue});
        field.Present();
        if (field.Width() != 100 || field.Height() != 40 || field.BufferSize() != 4000) {
            std::cerr << "The resized framebuffer dimensions are incorrect.\n";
            return 1;
        }
        field.Frame().Resize(20, 8);
        field.Clear();
        field.Present();

        field.Close();
        if (field.IsOpen()) {
            std::cerr << "The MAT DevField window did not close when requested.\n";
            return 1;
        }
    } catch (const std::exception& error) {
        std::cerr << "MAT DevField window smoke test failed: " << error.what() << '\n';
        return 1;
    }

    std::cout << "MAT DevField ASCII window smoke test passed.\n";
    return 0;
}
