#ifndef Z80_DIS_PRIV_H
#define Z80_DIS_PRIV_H

#include "z80_priv.h"

enum z80_arg_type
{
    Z80_ARG_NONE = 0,
    Z80_ARG_A,
    Z80_ARG_F,
    Z80_ARG_B,
    Z80_ARG_C,
    Z80_ARG_D,
    Z80_ARG_E,
    Z80_ARG_H,
    Z80_ARG_L,
    Z80_ARG_I,
    Z80_ARG_R,
    Z80_ARG_AF,
    Z80_ARG_AFa,
    Z80_ARG_BC,
    Z80_ARG_DE,
    Z80_ARG_HL,
    Z80_ARG_SP,
    Z80_ARG_PC,
    Z80_ARG_IX,
    Z80_ARG_IY,

    Z80_ARG_n,
    Z80_ARG_nn,
    Z80_ARG_mn,
    Z80_ARG_mnn,
    Z80_ARG_mHL,

    Z80_ARG_pn
};

#endif
