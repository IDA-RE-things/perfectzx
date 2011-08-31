#include "emul.h"

#ifndef PZX_ULA
#define PZX_ULA

extern Z80EX_CONTEXT *zxcpu;
extern unsigned long zxcpu_tstates;
extern unsigned long zxcpu_tstates_frame;
extern unsigned long zxcpu_int_start;
extern unsigned long zxcpu_int_end;

extern unsigned zx_device_count;

#ifdef __cplusplus
extern "C" {
#endif

// cpu callback procedures
Z80EX_BYTE zxcpu_memread(Z80EX_CONTEXT *cpu, Z80EX_WORD addr, int m1_state, void *user_data);
void zxcpu_memwrite(Z80EX_CONTEXT *cpu, Z80EX_WORD addr, Z80EX_BYTE value, void *user_data);
Z80EX_BYTE zxcpu_portread(Z80EX_CONTEXT *cpu, Z80EX_WORD port, void *user_data);
void zxcpu_portwrite(Z80EX_CONTEXT *cpu, Z80EX_WORD port, Z80EX_BYTE value, void *user_data);
Z80EX_BYTE zxcpu_intread(Z80EX_CONTEXT *cpu, void *user_data);

void zx_life( int *running );
void zx_reset();

void zx_init();
void zx_uninit();

#ifdef __cplusplus
}
#endif

#endif
