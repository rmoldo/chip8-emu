#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <iostream>

#include "chip8.h"
#include "defs.h"

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
        load_rom_into_memory(&chip8, argv[1]);

        printf("INIT EMULATOR for ROM %s\n", argv[1]);
        if (SDL_Init(SDL_INIT_VIDEO)) {
                fprintf(stderr, "Could not initialize sdl2: %s\n", SDL_GetError());
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

        int is_running = 1;
        SDL_Event e;
        while (is_running) {
                SDL_WaitEvent(&e);

                switch (e.type) {
                case SDL_KEYDOWN:
                        switch (e.key.keysym.sym) {
                        case SDLK_ESCAPE:
                                is_running = 0;
                                break;
                        }
                }
        }

        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);

        SDL_Quit();

        return EXIT_SUCCESS;
}
