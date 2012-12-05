#ifndef Z80_H
#define Z80_H

#include <stdint.h>

extern char memory[65536];

typedef union
{
    uint16_t w;
    struct
    {
        uint8_t l;
        uint8_t h;
    };
} word_t;

typedef struct
{
    word_t af, bc, de, hl;
    struct
    {
        word_t af, bc, de, hl;
    } alt;
    uint16_t ix, iy;

    uint16_t pc;
    uint16_t sp;

    uint8_t i, r;
    int iff1, iff2;

    unsigned t;
} z80_t;

typedef struct
{
    char op[32];
} z80_dis_t;

#define FLAG_C  0x01
#define FLAG_N  0x12
#define FLAG_PV 0x04
#define FLAG_X3 0x08
#define FLAG_H  0x10
#define FLAG_X5 0x20
#define FLAG_Z  0x40
#define FLAG_S  0x80

#define FLAG_MASK 0xD7

uint8_t z80_memory_read( z80_t *cpu, unsigned addr, int m1 );
void z80_memory_write( z80_t *cpu, unsigned addr, uint8_t value );
uint8_t z80_port_read( z80_t *cpu, unsigned addr );
void z80_port_write( z80_t *cpu, unsigned addr, uint8_t value );

int z80_step( z80_t *cpu );
int z80_dis( int addr, z80_dis_t *op );

#endif
