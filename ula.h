#include "emul.h"

#ifndef PZX_ULA
#define PZX_ULA

extern Z80EX_CONTEXT *zxcpu;
extern unsigned long zxcpu_tstates;

// cpu callback procedures
void zxcpu_tstate(Z80EX_CONTEXT *cpu, void *user_data);
Z80EX_BYTE zxcpu_memread(Z80EX_CONTEXT *cpu, Z80EX_WORD addr, int m1_state, void *user_data);
void zxcpu_memwrite(Z80EX_CONTEXT *cpu, Z80EX_WORD addr, Z80EX_BYTE value, void *user_data);
Z80EX_BYTE zxcpu_portread(Z80EX_CONTEXT *cpu, Z80EX_WORD port, void *user_data);
void zxcpu_portwrite(Z80EX_CONTEXT *cpu, Z80EX_WORD port, Z80EX_BYTE value, void *user_data);
Z80EX_BYTE zxcpu_intread(Z80EX_CONTEXT *cpu, void *user_data);

void zx_quantum();
void zx_reset();

void zx_init();
void zx_uninit();

#endif
