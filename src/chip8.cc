#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "chip8.h"
#include "defs.h"

u8 fontset[FONT_SIZE] = {
        0xF0, 0x90, 0x90, 0x90, 0xF0, //0
        0x20, 0x60, 0x20, 0x20, 0x70, //1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, //2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, //3
        0x90, 0x90, 0xF0, 0x10, 0x10, //4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, //5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, //6
        0xF0, 0x10, 0x20, 0x40, 0x40, //7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, //8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, //9
        0xF0, 0x90, 0xF0, 0x90, 0x90, //A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, //B
        0xF0, 0x80, 0x80, 0x80, 0xF0, //C
        0xE0, 0x90, 0x90, 0x90, 0xE0, //D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, //E
        0xF0, 0x80, 0xF0, 0x80, 0x80  //F
};

void (*fpoint[20])(struct chip8_t *, u16);

void init_chip8(struct chip8_t *chip8)
{
        chip8->ip = START_ADDRESS;

        chip8->opcode = 0;
        chip8->I = 0;
        chip8->sp = 0;
        chip8->delay_timer = 0;
        chip8->sound_timer = 0;

        /* Clear data registers */
        memset(chip8->V, 0, sizeof(chip8->V));

        /* Clear stack */
        memset(chip8->stack, 0, sizeof(chip8->stack));

        /* Clear keys */
        memset(chip8->keys, 0, sizeof(chip8->keys));

        /* Clear display */
        memset(chip8->display, 0, sizeof(chip8->display));

        /* Clear memory */
        memset(chip8->mem, 0, sizeof(chip8->mem));

        /* Load font set into memory */
        for (int i = 0; i < 16 * 5; ++i)
                chip8->mem[i] = fontset[i];
}

int load_rom_into_memory(struct chip8_t *chip8, const char *path)
{
        printf("Loading %s\n", path);

        FILE* rom;

        if ((rom = fopen(path, "rb")) == NULL) {
                fprintf(stderr, "Could not open %s\n", path);
                return -1;
        }

        /* Get size of rom file in bytes */
        fseek(rom, 0, SEEK_END);
        size_t size = (size_t)ftell(rom);
        rewind(rom);

        /* Check if the contents of the ROM file fit into memory */
        if (size > (4096 - 512)) {
                fprintf(stderr, "Could not fit ROM content into memory\n");
                return -1;
        }

        char *rom_buffer = NULL;
        if((rom_buffer = (char *)malloc(sizeof(char) * size)) == NULL) {
                fprintf(stderr, "Could not allocate memory for ROM\n");
                return -1;
        }

        /* Copy ROM contents into buffer */
        size_t result;
        if ((result = fread(rom_buffer, sizeof(char), (size_t)size, rom)) != size) {
                fprintf(stderr, "Could not read ROM file\n");
                return -1;
        }

        /* Copy ROM content into chip8 memory starting at 0x200 */
        for (int i = 0; i < (int)size; ++i)
                chip8->mem[START_ADDRESS + i] = (u8)rom_buffer[i];

        fclose(rom);
        free(rom_buffer);

        return 1;
}

void chip8_cycle(struct chip8_t *chip8)
{
        /* Fetch */
        u16 opcode = (chip8->mem[chip8->ip] << 8u) | chip8->mem[chip8->ip + 1];

        /* Point to the next instruction */
        chip8->ip += 2;

        /* Decode and execute */
        (*fpoint[opcode >> 12u])(chip8, opcode);

        if (chip8->delay_timer > 0)
                chip8->delay_timer--;

        if (chip8->sound_timer > 0)
                chip8->sound_timer--;
}


/* Clear display */
void op_00e0(struct chip8_t *chip8, u16 opcode)
{
        memset(chip8->display, 0, sizeof(chip8->display));
}

/* Return from subroutine */
void op_00ee(struct chip8_t *chip8, u16 opcode)
{
        chip8->sp--;
        chip8->ip = chip8->stack[chip8->sp];
}

/* Jump to location nnn */
void op_1nnn(struct chip8_t *chip8, u16 opcode)
{
        chip8->ip = (opcode & 0x0fff);
}

/* Call subroutine at nnn */
void op_2nnn(struct chip8_t *chip8, u16 opcode)
{
        /* Put the current ip onto the top of the stack */
        chip8->stack[chip8->sp] = chip8->ip;
        chip8->sp++;

        chip8->ip = (opcode & 0x0fff);
}

/* Skip next instruction if Vx = kk */
void op_3xkk(struct chip8_t *chip8, u16 opcode)
{
        u8 x = (opcode & 0x0f00) >> 8;
        u8 kk = opcode & 0x00ff;

        if (chip8->V[x] == kk)
                chip8->ip += 2;
}

/* Skip next instruction if Vx != kk */
void op_4xkk(struct chip8_t *chip8, u16 opcode)
{
        u8 x = (opcode & 0x0f00) >> 8;
        u8 kk = opcode & 0x00ff;

        if (chip8->V[x] != kk)
                chip8->ip += 2;
}

/* Skip next instruction if Vx == Vy */
void op_5xy0(struct chip8_t *chip8, u16 opcode)
{
        u8 x = (opcode & 0x0f00) >> 8;
        u8 y = (opcode & 0x00f0) >> 4;

        if (chip8->V[x] == chip8->V[y])
                chip8->ip += 2;
}

/* LD Vx, byte: Set Vx = kk */
void op_6xkk(struct chip8_t *chip8, u16 opcode)
{
        u8 x = (opcode & 0x0f00) >> 8;
        u8 kk = opcode & 0x00ff;

        chip8->V[x] = kk;
}

/* ADD Vx, byte: Set Vx += kk */
void op_7xkk(struct chip8_t *chip8, u16 opcode)
{
        u8 x = (opcode & 0x0f00) >> 8;
        u8 kk = opcode & 0x00ff;

        chip8->V[x] += kk;
}

/* LD Vx, Vy: Set Vx = Vy */
void op_8xy0(struct chip8_t *chip8, u16 opcode)
{
        u8 x = (opcode & 0x0f00) >> 8;
        u8 y = (opcode & 0x00f0) >> 4;

        chip8->V[x] = chip8->V[y];
}

/* OR Vx, Vy: Set Vx = Vx | Vy  */
void op_8xy1(struct chip8_t *chip8, u16 opcode)
{
        u8 x = (opcode & 0x0f00) >> 8;
        u8 y = (opcode & 0x00f0) >> 4;

        chip8->V[x] |= chip8->V[y];
}

/* AND Vx, Vy: Set Vx = Vx & Vy */
void op_8xy2(struct chip8_t *chip8, u16 opcode)
{
        u8 x = (opcode & 0x0f00) >> 8;
        u8 y = (opcode & 0x00f0) >> 4;

        chip8->V[x] &= chip8->V[y];
}

/* XOR Vx, Vy: Set Vx = Vx ^ Vy */
void op_8xy3(struct chip8_t *chip8, u16 opcode)
{
        u8 x = (opcode & 0x0f00) >> 8;
        u8 y = (opcode & 0x00f0) >> 4;

        chip8->V[x] ^= chip8->V[y];
}

/* AND Vx, Vy: Set Vx = Vx + Vy, set Vf = carry */
void op_8xy4(struct chip8_t *chip8, u16 opcode)
{
        u8 x = (opcode & 0x0f00) >> 8;
        u8 y = (opcode & 0x00f0) >> 4;

        u16 sum = chip8->V[x] + chip8->V[y];

        chip8->V[0xf] = (sum > 255) ? 1 : 0;
        chip8->V[x] = (u8)sum;
}

/* SUB Vx, VY: Set Vx = Vx - Vy, set Vf = Not borrow */
void op_8xy5(struct chip8_t *chip8, u16 opcode)
{
        u8 x = (opcode & 0x0f00) >> 8;
        u8 y = (opcode & 0x00f0) >> 4;

        chip8->V[0xf] = (chip8->V[x] > chip8->V[y]) ? 1 : 0;
        chip8->V[x] -= chip8->V[y];
}

/* SHR Vx {, Vy}: Set Vx = Vx SHR 1 */
void op_8xy6(struct chip8_t *chip8, u16 opcode)
{
        u8 x = (opcode & 0x0f00) >> 8;
        u8 y = (opcode & 0x00f0) >> 4;

        chip8->V[0xf] = chip8->V[x] & 0x01;

        chip8->V[x] >>= 1;
}

/* SUBN Vx, Vy: Set Vx = Vy - Vx, set Vf = not borrow  */
void op_8xy7(struct chip8_t *chip8, u16 opcode)
{
        u8 x = (opcode & 0x0f00) >> 8;
        u8 y = (opcode & 0x00f0) >> 4;

        chip8->V[0xf] = (chip8->V[x] < chip8->V[y]) ? 1 : 0;

        chip8->V[x] = chip8->V[y] - chip8->V[x];
}

/* SHL Vx, {, Vy} */
void op_8xye(struct chip8_t *chip8, u16 opcode)
{
        u8 x = (opcode & 0x0f00) >> 8;

        chip8->V[0xf] = chip8->V[x] >> 7;

        chip8->V[x] <<= 2;
}

/* SNE Vx, Vy: skip next instruction if Vx != Vy */
void op_9xy0(struct chip8_t *chip8, u16 opcode)
{
        u8 x = (opcode & 0x0f00) >> 8;
        u8 y = (opcode & 0x00f0) >> 4;

        if (chip8->V[x] != chip8->V[y])
                chip8->ip += 2;
}

/* LD I, addr: Set I = nnn */
void op_annn(struct chip8_t *chip8, u16 opcode)
{
        u16 nnn = opcode & 0x0fff;

        chip8->I = nnn;
}

/* JP V0, addr: Jump to location nnn + V0 */
void op_annn(struct chip8_t *chip8, u16 opcode)
{
        u16 nnn = opcode & 0x0fff;

        chip8->ip = nnn + chip8->V[0];
}
