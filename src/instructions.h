#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include "defs.h"
#include "chip8.h"

void op_00e0(struct chip8_t *chip8, u16 opcode);
void op_00ee(struct chip8_t *chip8, u16 opcode);
void op_1nnn(struct chip8_t *chip8, u16 opcode);
void op_2nnn(struct chip8_t *chip8, u16 opcode);
void op_3xkk(struct chip8_t *chip8, u16 opcode);
void op_4xkk(struct chip8_t *chip8, u16 opcode);
void op_5xy0(struct chip8_t *chip8, u16 opcode);
void op_6xkk(struct chip8_t *chip8, u16 opcode);
void op_7xkk(struct chip8_t *chip8, u16 opcode);
void op_8xy0(struct chip8_t *chip8, u16 opcode);
void op_8xy1(struct chip8_t *chip8, u16 opcode);
void op_8xy2(struct chip8_t *chip8, u16 opcode);
void op_8xy3(struct chip8_t *chip8, u16 opcode);
void op_8xy4(struct chip8_t *chip8, u16 opcode);
void op_8xy5(struct chip8_t *chip8, u16 opcode);
void op_8xy6(struct chip8_t *chip8, u16 opcode);
void op_8xy7(struct chip8_t *chip8, u16 opcode);
void op_8xye(struct chip8_t *chip8, u16 opcode);
void op_9xy0(struct chip8_t *chip8, u16 opcode);
void op_annn(struct chip8_t *chip8, u16 opcode);
void op_bnnn(struct chip8_t *chip8, u16 opcode);
void op_cxkk(struct chip8_t *chip8, u16 opcode);
void op_dxyn(struct chip8_t *chip8, u16 opcode);
void op_ex9e(struct chip8_t *chip8, u16 opcode);
void op_exa1(struct chip8_t *chip8, u16 opcode);
void op_fx07(struct chip8_t *chip8, u16 opcode);
void op_fx0a(struct chip8_t *chip8, u16 opcode);
void op_fx15(struct chip8_t *chip8, u16 opcode);
void op_fx18(struct chip8_t *chip8, u16 opcode);
void op_fx1e(struct chip8_t *chip8, u16 opcode);
void op_fx29(struct chip8_t *chip8, u16 opcode);
void op_fx33(struct chip8_t *chip8, u16 opcode);
void op_fx55(struct chip8_t *chip8, u16 opcode);
void op_fx65(struct chip8_t *chip8, u16 opcode);

#endif
