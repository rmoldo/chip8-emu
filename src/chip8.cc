#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "chip8.h"
#include "defs.h"
#include "instructions.h"

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

void (*fpoint[16])(struct chip8_t *, u16);

void init_chip8(struct chip8_t *chip8)
{
        chip8->ip = START_ADDRESS;

        chip8->opcode = 0;
        chip8->I = 0;
        chip8->sp = 0;
        chip8->delay_timer = 0;
        chip8->sound_timer = 0;
        chip8->draw_flag = 0;

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

        /* Random seed */
        srand(time(NULL));

        assign_function_pointers();
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

        /* Assign the correct function pointer in array if opcode starts with 0, 8, E, F */
        handle_function_pointers(opcode);

        /* Decode and execute */
        (*fpoint[opcode >> 12])(chip8, opcode);

        if (chip8->delay_timer > 0)
                chip8->delay_timer--;

        if (chip8->sound_timer > 0)
                chip8->sound_timer--;
}

/* Assign singular operations at their indexes in the function pointer array */
void assign_function_pointers()
{
        fpoint[1] = op_1nnn;
        fpoint[2] = op_2nnn;
        fpoint[3] = op_3xkk;
        fpoint[4] = op_4xkk;
        fpoint[5] = op_5xy0;
        fpoint[6] = op_6xkk;
        fpoint[7] = op_7xkk;
        fpoint[9] = op_9xy0;
        fpoint[10] = op_annn;
        fpoint[11] = op_bnnn;
        fpoint[12] = op_cxkk;
        fpoint[13] = op_dxyn;
}

/*
  Assign the correct operation in function pointer array if the operation
  starts with 0x0, 0x8, 0xE, 0xF;
*/
void handle_function_pointers(u16 opcode)
{
        /* Get type of operation */
        u8 type = (opcode & 0xf000) >> 12;

        u8 x = opcode & 0x000f;
        u8 xx = opcode & 0x00ff;

        switch(type) {
        case 0x0:
                switch(x) {
                case 0x0:
                        fpoint[0] = op_00e0;
                        break;
                case 0xe:
                        fpoint[0] = op_00ee;
                        break;
                }

                break;

        case 0x8:
                switch(x) {
                case 0x0:
                        fpoint[8] = op_8xy0;
                        break;
                case 0x1:
                        fpoint[8] = op_8xy1;
                        break;
                case 0x2:
                        fpoint[8] = op_8xy2;
                        break;
                case 0x3:
                        fpoint[8] = op_8xy3;
                        break;
                case 0x4:
                        fpoint[8] = op_8xy4;
                        break;
                case 0x5:
                        fpoint[8] = op_8xy5;
                        break;
                case 0x6:
                        fpoint[8] = op_8xy6;
                        break;
                case 0x7:
                        fpoint[8] = op_8xy7;
                        break;
                case 0xe:
                        fpoint[8] = op_8xye;
                        break;
                }

                break;

        case 0xe:
                switch(xx) {
                case 0x9e:
                        fpoint[14] = op_ex9e;
                        break;
                case 0xa1:
                        fpoint[14] = op_exa1;
                        break;
                }

                break;

        case 0xf:
                switch(xx) {
                case 0x07:
                        fpoint[15] = op_fx07;
                        break;
                case 0x0A:
                        fpoint[15] = op_fx0a;
                        break;
                case 0x15:
                        fpoint[15] = op_fx15;
                        break;
                case 0x18:
                        fpoint[15] = op_fx18;
                        break;
                case 0x1e:
                        fpoint[15] = op_fx1e;
                        break;
                case 0x29:
                        fpoint[15] = op_fx29;
                        break;
                case 0x33:
                        fpoint[15] = op_fx33;
                        break;
                case 0x55:
                        fpoint[15] = op_fx55;
                        break;
                case 0x65:
                        fpoint[15] = op_fx65;
                        break;
                }

                break;
        }
}
