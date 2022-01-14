#include "framework.h"

class Renderer
{
public:
    SDL_Event event;
    SDL_Renderer *renderer;
    SDL_Window *window;
    SDL_Texture *texture;
    Chip8 *chip8;
    uint32_t pixels[2048];

    Renderer()
    {
        SDL_Init(SDL_INIT_EVERYTHING);
        SDL_CreateWindowAndRenderer(1024, 512, 0, &window, &renderer);
        SDL_RenderSetLogicalSize(renderer, 1024, 512);
        SDL_RenderClear(renderer);

        texture = SDL_CreateTexture(renderer,
                                    SDL_PIXELFORMAT_ARGB8888,
                                    SDL_TEXTUREACCESS_STREAMING,
                                    64, 32);
    }

    bool update()
    {
        if (SDL_PollEvent(&event) && event.type == SDL_QUIT)
            return false;

        if (chip8->dirty)
        {
            for (int i = 0; i < 2048; ++i)
            {
                uint8_t pixel = chip8->gfx[i];
                pixels[i] = (0x00FFFFFF * pixel) | 0xFF000000;
            }

            SDL_UpdateTexture(texture, NULL, pixels, 64 * sizeof(Uint32));
            chip8->dirty = false;
        }

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);

        return true;
    }

    void exit()
    {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }
};