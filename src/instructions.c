#include <string.h>
#include <stdlib.h>

#include "defs.h"
#include "instructions.h"
#include "chip8.h"

/* Clear display */
void op_00e0(struct chip8_t *chip8, u16 opcode)
{
        memset(chip8->display, 0, sizeof(chip8->display));
        chip8->draw_flag = 1;
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
        chip8->ip = opcode & 0x0fff;
}

/* Call subroutine at nnn */
void op_2nnn(struct chip8_t *chip8, u16 opcode)
{
        /* Put the current ip onto the top of the stack */
        chip8->stack[chip8->sp] = chip8->ip;
        chip8->sp++;

        chip8->ip = opcode & 0x0fff;
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
void op_bnnn(struct chip8_t *chip8, u16 opcode)
{
        u16 nnn = opcode & 0x0fff;

        chip8->ip = nnn + chip8->V[0];
}

/* RAND Vx, byte: Set Vx = random byte AND kk */
void op_cxkk(struct chip8_t *chip8, u16 opcode)
{
        u8 kk = opcode & 0x00ff;
        u8 x = (opcode & 0x0f00) >> 8;

        chip8->V[x] = (u8)(rand() % 256) & kk;
}

/*
  DRW Vx, Vy, nibble: Display n-byte sprite starting at memory location I at (Vx, Vy), set Vf = collision.
  The interpreter reads n bytes from memory, starting at the address stored in I. These bytes are then displayed as
  sprites on screen at coordinates (Vx, Vy). Sprites are XORed onto the existing screen. If this causes any pixels
  to be erased, VF is set to 1, otherwise it is set to 0. If the sprite is positioned so part of it is outside the
  coordinates of the display, it wraps around to the opposite side of the screen.
 */
void op_dxyn(struct chip8_t *chip8, u16 opcode)
{
        u8 x = chip8->V[(opcode & 0x0f00) >> 8];
        u8 y = chip8->V[(opcode & 0x00f0) >> 4];
        u8 height = opcode & 0x000f;

        /* Set Vf to collision */
        chip8->V[0xf] = 0;

        for (int col = 0; col < height; ++col) {
                u8 sprite = chip8->mem[chip8->I + col];
                for (int row = 0; row < 8; ++row) {

                        u8 spritePixel = sprite & (0x80 >> row);
                        u8 *screenPixel = &chip8->display[x + row + (y + col) * 64];

                        if (spritePixel) {
                                /* If the screen pixel is also on then collision occured */
                                if (*screenPixel == 1)
                                        chip8->V[0xf] = 1;

                                *screenPixel ^= 1;
                        }
                }
        }

        chip8->draw_flag = 1;
}

/* SKP Vx: Skip next instruction if key with the value of Vx is pressed */
void op_ex9e(struct chip8_t *chip8, u16 opcode)
{
        u8 x = (opcode & 0x0f00) >> 8;

        if (chip8->keys[chip8->V[x]])
                chip8->ip += 2;
}

/* SKP Vx: Skip next instruction if key with the value of Vx not is pressed */
void op_exa1(struct chip8_t *chip8, u16 opcode)
{
        u8 x = (opcode & 0x0f00) >> 8;

        if (!chip8->keys[chip8->V[x]])
                chip8->ip += 2;
}

/* LD Vx, DT: Set Vx = delay_timer value */
void op_fx07(struct chip8_t *chip8, u16 opcode)
{
        u8 x = (opcode & 0x0f00) >> 8;

        chip8->V[x] = chip8->delay_timer;
}

/* Wait for a key press, store the value of the key in Vx */
void op_fx0a(struct chip8_t *chip8, u16 opcode)
{
        u8 x = (opcode & 0x0f00) >> 8;
        int pressed = 0;

        for (int i = 0; i < 16; ++i)
                if (chip8->keys[i]) {
                        chip8->V[x] = i;
                        pressed = 1;
                }

        /* If no key was pressed try again */
        if (!pressed) {
                chip8->ip -= 2;
                return;
        }
}

/* LD DT, Vx: Set delay timer = Vx */
void op_fx15(struct chip8_t *chip8, u16 opcode)
{
        u8 x = (opcode & 0x0f00) >> 8;

        chip8->delay_timer = chip8->V[x];
}

/* LD ST, Vx: Set sound timer = Vx*/
void op_fx18(struct chip8_t *chip8, u16 opcode)
{
        u8 x = (opcode & 0x0f00) >> 8;

        chip8->sound_timer = chip8->V[x];
}

/* ADD I, Vx: Set I = I + Vx */
void op_fx1e(struct chip8_t *chip8, u16 opcode)
{
        u8 x = (opcode & 0x0f00) >> 8;

        u32 sum = chip8->V[x] + chip8->I;

        chip8->V[0xf] = (sum > 0xffff) ? 1 : 0;

        chip8->I = (u16)sum;
}

/* LD F, VX: Set I = location of sprite for digit Vx */
void op_fx29(struct chip8_t *chip8, u16 opcode)
{
        u8 x = (opcode & 0x0f00) >> 8;

        chip8->I = 5 * chip8->V[x];
}

/* LD B, Vx: Store BCD represenation of Vx in memory locations I, I + 1, I + 2 */
void op_fx33(struct chip8_t *chip8, u16 opcode)
{
        u8 x = (opcode & 0x0f00) >> 8;

        chip8->mem[chip8->I + 2] = chip8->V[x] / 100;
        chip8->mem[chip8->I + 1] = chip8->V[x] / 10 % 10;
        chip8->mem[chip8->I + 0] = chip8->V[x] % 10;
}

/* LD [I], Vx: Store registers V0 through Vx in memory starting at location I */
void op_fx55(struct chip8_t *chip8, u16 opcode)
{
        u8 x = (opcode & 0x0f00) >> 8;

        for (int i = 0; i < (int)x + 1; ++i)
                chip8->mem[chip8->I + i] = chip8->V[i];
}

/* LD Vx, [I]: Read registers V0 through Vx from memory starting at location I */
void op_fx65(struct chip8_t *chip8, u16 opcode)
{
        u8 x = (opcode & 0x0f00) >> 8;

        for (int i = 0; i < (int)x + 1; ++i)
                chip8->V[i] = chip8->mem[chip8->I + i];
}
