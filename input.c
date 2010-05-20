#include "main.h"

#include "input.h"
#include "input_keymap.h"
#include "input_zxkeymap.h"

#define MAX_KEY_STACK 3

unsigned short keyboard_stack[MAX_KEY_STACK];
unsigned keyboard_stack_pointer;

ZXKEYBIND *zxkey_bind = NULL;
unsigned zxkey_bind_count = 0;

volatile unsigned char zxkey_matrix[8];

void input_add_zxkey_bind(const char *key_code, const char *zxkey)
{
    unsigned i;
    zxkey_bind = (ZXKEYBIND *)realloc(zxkey_bind, (zxkey_bind_count + 1) * sizeof(ZXKEYBIND));

    for ( i = 0; i < (sizeof(keymap)/sizeof(KEYMAP)); i ++ )
        if ( strcmp( key_code, keymap[i].name ) == 0 )
            zxkey_bind[zxkey_bind_count].keycode = keymap[i].key;
    for ( i = 0; i < (sizeof(zxkeymap)/sizeof(ZXKEYMAP)); i ++ )
        if ( strcmp(zxkey, zxkeymap[i].name ) == 0)
        {
            //printf( "Key: %d\n", zxkey_bind[zxkey_bind_count].keycode );
            zxkey_bind[zxkey_bind_count].zxkey = zxkeymap[i].key;
        }
    zxkey_bind_count++;
}

void input_delete_zxkey_bindings()
{
    free(zxkey_bind);
    zxkey_bind_count = 0;
}

void input_event_keyboard(unsigned short key, int pressed)
{
    unsigned i;
    for ( i = 0; i < zxkey_bind_count; i ++ )
        if ( zxkey_bind[i].keycode == key )
        {
            //printf( "Key: %.2X\n", zxkey_bind[i].zxkey );
            if ( pressed )
                zxkey_matrix[zxkey_bind[i].zxkey >> 4] &= ~(1 << (zxkey_bind[i].zxkey & 0xF));
            else
                zxkey_matrix[zxkey_bind[i].zxkey >> 4] |= (1 << (zxkey_bind[i].zxkey & 0xF));
            //printf( "Key: %.2X %.2X\n", zxkey_matrix[0], zxkey_matrix[4] );

            break;
        }
}

void input_init()
{
    unsigned i;
    keymap_init();
    zxkey_bind = NULL;
    zxkey_bind_count = 0;

    FILE *kbf;
    kbf = fopen("zxkeymap.cfg", "rt");
    char zxkey[256], *keyname, *keycode;
    while (fgets(zxkey, 256, kbf))
    {
        keyname = zxkey;
        while ( *keyname < ' ' )
            keyname ++;

        keycode = strchr(keyname, '\n');
        if (keycode)
            *keycode= '\0';
        keycode = strchr(keyname, '=');
        *(keycode++) = '\0';

        input_add_zxkey_bind(keyname, keycode);
    }
    fclose(kbf);

    for ( i = 0; i < 8; i ++ )
        zxkey_matrix[i] = 0xFF;
}

void input_uninit()
{
    input_delete_zxkey_bindings();
}

void input_reset()
{
    keyboard_stack_pointer = 0;
}
