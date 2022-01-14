#include "framework.h"

class Renderer
{
public:
    SDL_Event event;
    SDL_Renderer *renderer;
    SDL_Window *window;
    SDL_Texture *texture;
    Chip8 *chip8;

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
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
                return false;

            if (event.type == SDL_KEYDOWN)
            {
                if (event.key.keysym.sym == SDLK_ESCAPE)
                    return false;

                for (int i = 0; i < 16; ++i)
                {
                    if (event.key.keysym.sym == keymap[i])
                    {
                        chip8->key[i] = 1;
                    }
                }
            }
            
            // Process keyup events
            if (event.type == SDL_KEYUP)
            {
                for (int i = 0; i < 16; ++i)
                {
                    if (event.key.keysym.sym == keymap[i])
                    {
                        chip8->key[i] = 0;
                    }
                }
            }
        }

        if (chip8->dirty)
        {
            SDL_UpdateTexture(texture, NULL, chip8->screen.data(), 64 * sizeof(Uint32));
            chip8->dirty = false;
        }

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);

        return true;
    }

    void
    exit()
    {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }
};