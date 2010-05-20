#include "emul.h"

#ifndef PZX_INPUT
#define PZX_INPUT

extern volatile unsigned char zxkey_matrix[8];

void input_event_keyboard(unsigned short key, int pressed);
void input_init();
void input_uninit();
void input_reset();

#endif
