#include "z80.h"
#include "z80_priv.h"
#include "z80_dis_priv.h"

#include <stdio.h>
#include <string.h>

extern struct z80_op z80_no_op_cb[256];
extern struct z80_op z80_op_ed_cb[256];

char memory[65536];

uint8_t z80_memory_read( z80_t *cpu, unsigned addr, int m1 )
{
    cpu->t += 3;

    return memory[addr];
}

void z80_memory_write( z80_t *cpu, unsigned addr, uint8_t value )
{
    cpu->t += 3;

    memory[addr] = value;
}

uint8_t z80_port_read( z80_t *cpu, unsigned addr )
{
    cpu->t += 4;

    return 0xFF;
}

void z80_port_write( z80_t *cpu, unsigned addr, uint8_t value )
{
    cpu->t += 4;

    //memory[addr] = value;
}



int z80_step( z80_t *cpu )
{
    uint8_t opcode;

    cpu->t = 0;

    opcode = z80_memory_read( cpu, cpu->pc, 1 );
    cpu->pc ++;
    cpu->t ++;
    cpu->r = (cpu->r & 0x80) | ((cpu->r + 1) & 0x7F);

    if ( z80_no_op_cb[ opcode ].func )
        z80_no_op_cb[ opcode ].func( cpu );
    else
        printf( "unknown opcode: %.2X\n", opcode );

    printf( "t = %d\n", cpu->t );

    return 0;
}

#define Z80_PARSE_ARG(arg) \
    switch ( arg ) \
    { \
        case Z80_ARG_A: strcat( op->op, "a" ); break; \
        case Z80_ARG_B: strcat( op->op, "b" ); break; \
        case Z80_ARG_C: strcat( op->op, "c" ); break; \
        case Z80_ARG_D: strcat( op->op, "d" ); break; \
        case Z80_ARG_E: strcat( op->op, "e" ); break; \
        case Z80_ARG_H: strcat( op->op, "h" ); break; \
        case Z80_ARG_L: strcat( op->op, "l" ); break; \
        case Z80_ARG_I: strcat( op->op, "i" ); break; \
        case Z80_ARG_R: strcat( op->op, "r" ); break; \
        case Z80_ARG_AF: strcat( op->op, "af" ); break; \
        case Z80_ARG_AFa: strcat( op->op, "af'" ); break; \
        case Z80_ARG_BC: strcat( op->op, "bc" ); break; \
        case Z80_ARG_DE: strcat( op->op, "de" ); break; \
        case Z80_ARG_HL: strcat( op->op, "hl" ); break; \
        case Z80_ARG_SP: strcat( op->op, "sp" ); break; \
        case Z80_ARG_PC: strcat( op->op, "pc" ); break; \
        case Z80_ARG_mHL: strcat( op->op, "(hl)" ); break; \
        case Z80_ARG_n: \
            sprintf( op->op + strlen( op->op ), "%.2X", \
                     (uint8_t)memory[addr + 1] ); \
            size += 1; \
            break; \
        case Z80_ARG_nn: \
            sprintf( op->op + strlen( op->op ), "%.2X%.2X", \
                     (uint8_t)memory[addr + 2], (uint8_t)memory[addr + 1] ); \
            size += 2; \
            break; \
        case Z80_ARG_pn: \
            sprintf( op->op + strlen( op->op ), "(%.2X)", \
                     (uint8_t)memory[addr + 1] ); \
            size += 1; \
            break; \
    }

int z80_dis( int addr, z80_dis_t *op )
{
    uint8_t opcode;
    const struct z80_op_dis *dis;
    int size = 1;

    op->op[0] = '\0';

    opcode = memory[addr];
    printf("%.4x %.2x", addr, opcode );

    dis = z80_no_op_cb[ opcode ].dis;
    if ( opcode == 0xED )
    {
        opcode = memory[++addr];
        size ++;
        dis = z80_op_ed_cb[ opcode ].dis;
        printf("%.2x", opcode );
    }

    if ( !dis )
    {
        printf("<>\n");
        return 0;
    }

    printf(" - ");

    strcpy( op->op, dis->op );
    printf( "%s ", dis->op );

    if ( dis->op1 != Z80_ARG_NONE )
    {
        strcat( op->op, "\t" );
        Z80_PARSE_ARG( dis->op1 )
    }

    if ( dis->op2 != Z80_ARG_NONE )
    {
        strcat( op->op, ", " );
        Z80_PARSE_ARG( dis->op2 )
    }

    printf( "\n" );

    return size;
}
