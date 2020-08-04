#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <time.h>

#include "chip8.h"
#include "defs.h"

u8 keymap[16] = {
        SDLK_x, SDLK_1, SDLK_2, SDLK_3,
        SDLK_q, SDLK_w, SDLK_e, SDLK_a,
        SDLK_s, SDLK_d, SDLK_z, SDLK_c,
        SDLK_4, SDLK_r, SDLK_f, SDLK_v
};

void show_use(void)
{
        printf("Usage: chip8 ROMFILE\n");
}

int main(int argc, char **argv)
{
        if (argc != 2) {
                show_use();
                return EXIT_FAILURE;
        }

        struct chip8_t chip8;

        init_chip8(&chip8);

        if(load_rom_into_memory(&chip8, argv[1]) != 1) {
                fprintf(stderr, "Could not load rom file\n");
                return EXIT_FAILURE;
        }

        printf("INIT EMULATOR for ROM %s\n", argv[1]);
        if (SDL_Init(SDL_INIT_VIDEO)) {
                fprintf(stderr, "Could not initialize SDL2: %s\n", SDL_GetError());
                return EXIT_FAILURE;
        }

        SDL_Window *window = SDL_CreateWindow("Chip8",
                                              100, 100,
                                              WIDTH, HEIGHT,
                                              SDL_WINDOW_SHOWN);

        if (window == NULL) {
                fprintf(stderr, "SDL_CreateWindow Error: %s\n", SDL_GetError());
                return EXIT_FAILURE;
        }

        SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

        if (renderer == NULL) {
                SDL_DestroyWindow(window);
                fprintf(stderr, "SDL_CreateRenderer Error: %s\n", SDL_GetError());
                return EXIT_FAILURE;
        }

        SDL_Texture *texture = SDL_CreateTexture(renderer,
                                                 SDL_PIXELFORMAT_ARGB8888,
                                                 SDL_TEXTUREACCESS_STREAMING,
                                                 64, 32);

        u32 pixels[64 * 32];

        struct timespec stime;
        stime.tv_nsec = 1200000;

        int is_running = 1;
        while (is_running) {
                chip8_cycle(&chip8);

                SDL_Event e;
                while (SDL_PollEvent(&e)) {
                        if (e.type == SDL_QUIT)
                                is_running = 0;

                        if (e.type == SDL_KEYDOWN) {
                                if (e.key.keysym.sym == SDLK_ESCAPE)
                                        is_running = 0;

                                for (int i = 0; i < 16; ++i) {
                                        if (e.key.keysym.sym == keymap[i])
                                                chip8.keys[i] = 1;
                                }
                        }

                        if (e.type == SDL_KEYUP) {
                                for (int i = 0; i < 16; ++i) {
                                        if (e.key.keysym.sym == keymap[i])
                                                chip8.keys[i] = 0;
                                }
                        }
                }

                /* If draw occurred, redraw screen */
                if (chip8.draw_flag) {
                        chip8.draw_flag = 0;

                        for (int i = 0; i < 2048; ++i) {
                                u8 pixel = chip8.display[i];
                                pixels[i] = (pixel == 0) ? 0 : 0xffffffff;
                        }

                        SDL_UpdateTexture(texture, NULL, pixels, 64 * sizeof(u32));
                        SDL_RenderClear(renderer);
                        SDL_RenderCopy(renderer, texture, NULL, NULL);
                        SDL_RenderPresent(renderer);
                }

                /* Sleep for 1200 us */
                nanosleep(&stime, NULL);
        }

        SDL_DestroyTexture(texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);

        SDL_Quit();

        return EXIT_SUCCESS;
}
