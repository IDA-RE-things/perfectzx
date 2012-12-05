#include <stdio.h>

#include "z80.h"
#include "z80_op_priv.h"

/*
NOP
Operation: —
T States: 4
Condition Bits Affected: None
*/
Z80_OP( 00, nop )
{
}

/*
HALT
Operation: —
T States: 4
Condition Bits Affected: None
*/
Z80_OP( 76, halt )
{
    cpu->pc --;
}

/*
DI
Operation: IFF ← 0
T States: 4
Condition Bits Affected: None
*/
Z80_OP( F3, di )
{
    cpu->iff1 = cpu->iff2 = 0;
}

/*
EI
Operation: IFF ← 1
T States: 4
Condition Bits Affected: None
*/
Z80_OP( FB, ei )
{
    cpu->iff1 = cpu->iff2 = 1;
}

/*
XOR s
Operation: A←A ⊕s

T States:
XOR r       - 4
XOR (HL)    - 7 (4, 3)

Condition Bits Affected:
S is set if result is negative; reset otherwise
Z is set if result is zero; reset otherwise
H is reset
P/V is set if parity even; reset otherwise
N is reset
C is reset
*/
#define Z80_XOR(c,s) Z80_OP1(c, xor, s) \
    {   \
        SET_A( GET_A ^ GET_ ## s );    \
        SET_F( (GET_F & ~FLAG_MASK) | \
               z80_general_flags[GET_A] ); \
    }
Z80_XOR( AF, A );

/*
INC ss
Operation: ss ← ss + 1
T States: 6
Condition Bits Affected: None
*/
#define Z80_INC_W(c,ss) Z80_OP1(c, inc, ss) \
    {   \
        cpu->t += 2; \
        SET_ ##ss ( GET_ ##ss + 1 ); \
    }
Z80_INC_W( 03, BC );
Z80_INC_W( 13, DE );
Z80_INC_W( 23, HL );
Z80_INC_W( 33, SP );


/*
DEC m
Operation: m ← m- 1
T States:
DEC r       - 4
DEC (HL)    - 11 (4, 4, 3)
Condition Bits Affected:
S is set if result is negative; reset otherwise
Z is set if result is zero; reset otherwise
H is set if borrow from bit 4, reset otherwise
P/V is set if m was 80H before operation; reset otherwise
N is set
C is not affected
*/
#define Z80_DEC(c,m) Z80_OP1(c, dec, m) \
    {   \
        int8_t tmp = GET_ ##m; \
        int flags = 0; \
        if ( !(tmp & 0xF) ) flags |= FLAG_H; \
        if ( tmp == 0x80 ) flags |= FLAG_PV; \
        tmp --; \
        flags |= z80_general_flags[tmp] & (FLAG_S | FLAG_Z); \
        SET_ ##m ( tmp ); \
        SET_F( (GET_F & ((~FLAG_MASK) | FLAG_C)) | flags ); \
    }
#define Z80_DEC_mHL(c) Z80_OP1(c, dec, mHL) \
    {   \
        int8_t tmp = GET_mHL; \
        int flags = 0; \
        cpu->t += 1; \
        if ( !(tmp & 0xF) ) flags |= FLAG_H; \
        if ( tmp == 0x80 ) flags |= FLAG_PV; \
        tmp --; \
        flags |= z80_general_flags[tmp] & (FLAG_S | FLAG_Z); \
        SET_mHL( tmp ); \
        SET_F( (GET_F & ((~FLAG_MASK) | FLAG_C)) | flags ); \
    }

Z80_DEC( 05, B );
Z80_DEC( 0D, C );
Z80_DEC( 15, D );
Z80_DEC( 1D, E );
Z80_DEC( 25, H );
Z80_DEC( 2D, L );
Z80_DEC( 3D, A );

Z80_DEC_mHL( 35 );

/*
DEC ss
Operation: ss ← ss - 1
T States: 6
Condition Bits Affected: None
*/
#define Z80_DEC_W(c,ss) Z80_OP1(c, dec, ss) \
    {   \
        cpu->t += 2; \
        SET_ ##ss ( GET_ ##ss - 1 ); \
    }
Z80_DEC_W( 0B, BC );
Z80_DEC_W( 1B, DE );
Z80_DEC_W( 2B, HL );
Z80_DEC_W( 3B, SP );

/*
LD d, s
Operation: s ← d
Condition Bits Affected: None
T States:
LD r, r'    - 4
LD r, n     - 7 (4, 3)
LD r, (HL)  - 7 (4, 3)
LD (HL), r  - 7 (4, 3)
LD (HL), n  - 10 (4, 3, 3)
LD A, (rp)  - 7 (4, 3)
LD A, (nn)  - 13 (4, 3, 3, 3)
LD (rp), A  - 7 (4, 3)
LD (nn), A  - 13 (4, 3, 3, 3)
LD I, A     - 9 (4, 5)
LD R, A     - 9 (4, 5)
LD dd, nn   - 10 (4, 3, 3)
*/
#define Z80_LD(c,d,s) Z80_OP2(c, ld, d, s) \
    {   \
        int tmp = GET_ ##s; \
        SET_ ##d ( tmp ); \
    }
Z80_LD( 11, DE, nn );
Z80_LD( 22, mnn, HL );
Z80_LD( 2A, HL, nn );
Z80_LD( 36, mHL, n );
Z80_LD( 3E, A, n );

Z80_LD( 40, B, B );
Z80_LD( 41, B, C );
Z80_LD( 42, B, D );
Z80_LD( 43, B, E );
Z80_LD( 44, B, H );
Z80_LD( 45, B, L );
Z80_LD( 46, B, mHL );
Z80_LD( 47, B, A );

Z80_LD( 48, C, B );
Z80_LD( 49, C, C );
Z80_LD( 4A, C, D );
Z80_LD( 4B, C, E );
Z80_LD( 4C, C, H );
Z80_LD( 4D, C, L );
Z80_LD( 4E, C, mHL );
Z80_LD( 4F, C, A );

Z80_LD( 50, D, B );
Z80_LD( 51, D, C );
Z80_LD( 52, D, D );
Z80_LD( 53, D, E );
Z80_LD( 54, D, H );
Z80_LD( 55, D, L );
Z80_LD( 56, D, mHL );
Z80_LD( 57, D, A );

Z80_LD( 58, E, B );
Z80_LD( 59, E, C );
Z80_LD( 5A, E, D );
Z80_LD( 5B, E, E );
Z80_LD( 5C, E, H );
Z80_LD( 5D, E, L );
Z80_LD( 5E, E, mHL );
Z80_LD( 5F, E, A );

Z80_LD( 60, H, B );
Z80_LD( 61, H, C );
Z80_LD( 62, H, D );
Z80_LD( 63, H, E );
Z80_LD( 64, H, H );
Z80_LD( 65, H, L );
Z80_LD( 66, H, mHL );
Z80_LD( 67, H, A );

Z80_LD( 68, L, B );
Z80_LD( 69, L, C );
Z80_LD( 6A, L, D );
Z80_LD( 6B, L, E );
Z80_LD( 6C, L, H );
Z80_LD( 6D, L, L );
Z80_LD( 6E, L, mHL );
Z80_LD( 6F, L, A );

Z80_LD( 70, mHL, B );
Z80_LD( 71, mHL, C );
Z80_LD( 72, mHL, D );
Z80_LD( 73, mHL, E );
Z80_LD( 74, mHL, H );
Z80_LD( 75, mHL, L );
Z80_LD( 77, mHL, A );

Z80_LD( 78, A, B );
Z80_LD( 79, A, C );
Z80_LD( 7A, A, D );
Z80_LD( 7B, A, E );
Z80_LD( 7C, A, H );
Z80_LD( 7D, A, L );
Z80_LD( 7E, A, mHL );
Z80_LD( 7F, A, A );


/*
JP nn
Operation: PC ← nn
T States: 10 (4, 3, 3)
Condition Bits Affected: None
*/
#define Z80_JP(c,nn) Z80_OP1(c, jp, nn) \
    {   \
        cpu->pc = GET_ ##nn; \
    }
Z80_JP( C3, nn );


/*
OUT (n), A
Operation: (n) ← A
T States: 11 (4, 3, 4)
Condition Bits Affected: None
*/
#define Z80_OUT_N(c) Z80_OP2(c, out, pn, A) \
    {   \
        z80_port_write( cpu, (GET_A << 8) | GET_pn, GET_A ); \
    }
Z80_OUT_N( D3 );

/*
PUSH qq
Operation: (SP-2) ← qqL, (SP-1) ← qqH
T States: 11 (5, 3, 3)
Condition Bits Affected: None
*/
#define Z80_PUSH(c,qq) Z80_OP1(c, push, qq) \
    {   \
        z80_push( cpu, GET_ ##qq ); \
    }
Z80_PUSH( E5, HL );
Z80_PUSH( F5, AF );

/*
RST p
Operation: (SP-1) ← PCH, (SP-2) ← PCL, PCH ← 0, PCL ← P
T States: 11 (5, 3, 3)
Condition Bits Affected: None
*/
#define Z80_RST(c,p) Z80_OP(c, rst ##p) \
    {   \
        z80_push( cpu, cpu->pc ); \
        cpu->pc = 0x ##p; \
    }
Z80_RST( FF, 38 );

extern struct z80_op z80_op_ed_cb[256];

static void z80_op_ED( z80_t *cpu )
{
    uint8_t opcode;

    opcode = z80_memory_read( cpu, cpu->pc, 1 );
    cpu->pc ++;
    cpu->t ++;
    //cpu->r = (cpu->r & 0x80) | ((cpu->r + 1) & 0x7F);

    if ( z80_op_ed_cb[ opcode ].func )
        z80_op_ed_cb[ opcode ].func( cpu );
    else
        printf( "unknown opcode: %.2X\n", opcode );
}

#define OP(c) [0x ##c] = { z80_op_ ##c, &z80_op_dis_ ##c }
struct z80_op z80_no_op_cb[256] =
    {
        OP(00), OP(03), OP(05),
        OP(0B), OP(0D),
        OP(11), OP(13), OP(15),
        OP(1B), OP(1D),
        OP(22), OP(23), OP(25),
        OP(2A), OP(2B), OP(2D),
        OP(33), OP(35), OP(36),
        OP(3B), OP(3D), OP(3E),
        OP(40), OP(41), OP(42), OP(43), OP(44), OP(45), OP(46), OP(47),
        OP(48), OP(49), OP(4A), OP(4B), OP(4C), OP(4D), OP(4E), OP(4F),
        OP(50), OP(51), OP(52), OP(53), OP(54), OP(55), OP(56), OP(57),
        OP(58), OP(59), OP(5A), OP(5B), OP(5C), OP(5D), OP(5E), OP(5F),
        OP(60), OP(61), OP(62), OP(63), OP(64), OP(65), OP(66), OP(67),
        OP(68), OP(69), OP(6A), OP(6B), OP(6C), OP(6D), OP(6E), OP(6F),
        OP(70), OP(71), OP(72), OP(73), OP(74), OP(75), OP(76), OP(77),
        OP(78), OP(79), OP(7A), OP(7B), OP(7C), OP(7D), OP(7E), OP(7F),
        OP(AF),
        OP(C3),
        OP(D3),
        OP(E5),
        [0xED] = { z80_op_ED, NULL },
        OP(F3),
        OP(F5),
        OP(FB),
        OP(FF)
    };
