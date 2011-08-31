#include "emul.h"

#ifndef PZX_INPUT
#define PZX_INPUT

extern volatile unsigned char zxkey_matrix[8];
extern volatile unsigned zxmouse_x, zxmouse_y, zxmouse_but;

#ifdef __cplusplus
extern "C" {
#endif

int input_event_keyboard(unsigned short key, int pressed);
void input_init();
void input_uninit();
void input_reset();

#ifdef __cplusplus
}
#endif

#endif
