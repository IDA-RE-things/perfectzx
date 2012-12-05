#ifndef Z80_PRIV_H
#define Z80_PRIV_H

struct z80_op_dis
{
    const char *op;
    int op1, op2;
};

struct z80_op
{
    void (*func)( z80_t * );
    const struct z80_op_dis *dis;
};

#endif
