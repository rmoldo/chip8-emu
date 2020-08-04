#ifndef DEFS_H
#define DEFS_H

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

enum SCREEN {
        WIDTH = 1024,
        HEIGHT = 512
};

#define FONT_SIZE 80
#define START_ADDRESS 0x200
#define AVAILABLE_SPACE 3584

#endif
