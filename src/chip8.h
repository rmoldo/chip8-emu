#ifndef CHIP8_H
#define CHIP8_H

#include "defs.h"

struct chip8_t {
        u16 stack[16];
        u16 sp;

        u8 mem[4096]; // 4k memory
        u8 V[16]; // 16 data registers V0 -> V15

        u16 ip;
        u16 opcode;
        u16 I; // Address register

        u8 delay_timer;
        u8 sound_timer;

        u8 display[64 * 32];
        u8 keys[16];
};

void init_chip8(struct chip8_t *chip8);
void chip8_cycle(struct chip8_t *chip8);

int load_rom_into_memory(struct chip8_t *chip8, const char *path);

#endif
