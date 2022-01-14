#include "chip-8.hpp"
#include "renderer.hpp"
#undef main

int main(int argc, char **argv)
{

    auto chip8 = new Chip8();
    auto renderer = new Renderer();
    renderer->chip8 = chip8;

    while (renderer->update())
    {
        chip8->cycle();
        
        std::this_thread::sleep_for(std::chrono::microseconds(1200));
    }

    renderer->exit();

    return 0;
}
