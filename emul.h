#include "z80ex.h"

#ifndef PZX_EMUL
#define PZX_EMUL

void emul_init();
void emul_uninit();
extern void (*sync_wait)(void);

#endif
