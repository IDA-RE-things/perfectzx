#ifndef Z80_OP_PRIV_H
#define Z80_OP_PRIV_H

#include "z80_dis_priv.h"

extern uint8_t z80_general_flags[256];

#define OP_PREFIX z80_op

#define Z80_OP(c,n) \
    static const struct z80_op_dis z80_op_dis_ ##c = { #n, Z80_ARG_NONE, Z80_ARG_NONE }; \
    static void z80_op_ ##c ( z80_t *cpu )
#define Z80_OP1(c,n,op1) \
    static const struct z80_op_dis z80_op_dis_ ##c = { #n, Z80_ARG_ ##op1 , Z80_ARG_NONE }; \
    static void z80_op_ ##c ( z80_t *cpu )
#define Z80_OP2(c,n,op1,op2) \
    static const struct z80_op_dis z80_op_dis_ ##c = { #n, Z80_ARG_ ##op1 , Z80_ARG_ ##op2 }; \
    static void z80_op_ ##c ( z80_t *cpu )



static inline uint16_t z80_memory_read_word( z80_t *cpu )
{
    uint16_t tmp = z80_memory_read( cpu, cpu->pc ++, 0 );
    tmp |= z80_memory_read( cpu, cpu->pc ++, 0 ) << 8;

    return tmp;
}

static inline void z80_memory_write_word( z80_t *cpu, int addr, uint16_t value )
{
    z80_memory_write( cpu, addr, value >> 8 );
    z80_memory_write( cpu, addr + 1, value );
}

static inline void z80_push( z80_t *cpu, uint16_t value )
{
    cpu->t += 1;
    z80_memory_write( cpu, --cpu->sp, value >> 8 );
    z80_memory_write( cpu, --cpu->sp, value );
}


#define GET_A    cpu->af.h
#define SET_A(v) cpu->af.h = (v)
#define GET_F    cpu->af.l
#define SET_F(v) cpu->af.l = (v)
#define GET_AF      cpu->af.w
#define SET_AF(v)   cpu->af.w = (v)

#define GET_B    cpu->bc.h
#define SET_B(v) cpu->bc.h = (v)
#define GET_C    cpu->bc.l
#define SET_C(v) cpu->bc.l = (v)
#define GET_BC      cpu->bc.w
#define SET_BC(v)   cpu->bc.w = (v)

#define GET_D    cpu->de.h
#define SET_D(v) cpu->de.h = (v)
#define GET_E    cpu->de.l
#define SET_E(v) cpu->de.l = (v)
#define GET_DE      cpu->de.w
#define SET_DE(v)   cpu->de.w = (v)

#define GET_H    cpu->hl.h
#define SET_H(v) cpu->hl.h = (v)
#define GET_L    cpu->hl.l
#define SET_L(v) cpu->hl.l = (v)
#define GET_HL      cpu->hl.w
#define SET_HL(v)   cpu->hl.w = (v)

#define GET_I       (cpu->t += 1, cpu->i)
#define SET_I(v)    (cpu->t += 1, cpu->i = (v))
#define GET_R       (cpu->t += 1, cpu->r)
#define SET_R(v)    (cpu->t += 1, cpu->r = (v))

#define GET_SP      cpu->sp
#define SET_SP(v)   cpu->sp = (v)

#define GET_mHL      z80_memory_read( cpu, cpu->hl.w, 0 )
#define SET_mHL(v)   z80_memory_write( cpu, cpu->hl.w, v )

#define GET_n   z80_memory_read( cpu, cpu->pc ++, 0 )
#define GET_nn  z80_memory_read_word( cpu )
#define GET_pn  GET_n

#define SET_mn(v)   z80_memory_write( cpu, z80_memory_read_word( cpu ), v )
#define SET_mnn(v)  z80_memory_write_word( cpu, z80_memory_read_word( cpu ), v )

#endif
