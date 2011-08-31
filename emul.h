#include "z80ex.h"

#ifndef PZX_EMUL
#define PZX_EMUL

#ifdef __cplusplus
extern "C" {
#endif

void emul_start();
void emul_stop();
void emul_init();
void emul_uninit();
extern void (*sync_wait)(void);

#ifdef __cplusplus
}
#endif

typedef struct
{
    char *extention;
    int (*process)( const char *filename );
} file_type_t;

typedef struct
{
    char *name;
    file_type_t *type;

} file_fiter_t;

#endif
