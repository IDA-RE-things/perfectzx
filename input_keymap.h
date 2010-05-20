#include <gdk/gdkkeys.h>
#include <gdk/gdkkeysyms.h>

typedef guint keyhardcode;

typedef struct
{
    const char *name;
    keyhardcode key;
    const guint keyval;
} KEYMAP;

KEYMAP keymap[] = {
    { "ESC",    0,  GDK_Escape },
    { "F1",     0,  GDK_F1 },
    { "F2",     0,  GDK_F2 },
    { "F3",     0,  GDK_F3 },
    { "F4",     0,  GDK_F4 },
    { "F5",     0,  GDK_F5 },
    { "F6",     0,  GDK_F6 },
    { "F7",     0,  GDK_F7 },
    { "F8",     0,  GDK_F8 },
    { "F9",     0,  GDK_F9 },
    { "F10",    0,  GDK_F10 },
    { "F11",    0,  GDK_F11 },
    { "F12",    0,  GDK_F12 },

    { "1",      0,  GDK_1 },
    { "2",      0,  GDK_2 },
    { "3",      0,  GDK_3 },
    { "4",      0,  GDK_4 },
    { "5",      0,  GDK_5 },
    { "6",      0,  GDK_6 },
    { "7",      0,  GDK_7 },
    { "8",      0,  GDK_8 },
    { "9",      0,  GDK_9 },
    { "0",      0,  GDK_0 },

    { "A",      0,  GDK_A },
    { "B",      0,  GDK_B },
    { "C",      0,  GDK_C },
    { "D",      0,  GDK_D },
    { "E",      0,  GDK_E },
    { "F",      0,  GDK_F },
    { "G",      0,  GDK_G },
    { "H",      0,  GDK_H },
    { "I",      0,  GDK_I },
    { "J",      0,  GDK_J },
    { "K",      0,  GDK_K },
    { "L",      0,  GDK_L },
    { "M",      0,  GDK_M },
    { "N",      0,  GDK_N },
    { "O",      0,  GDK_O },
    { "P",      0,  GDK_P },
    { "Q",      0,  GDK_Q },
    { "R",      0,  GDK_R },
    { "S",      0,  GDK_S },
    { "T",      0,  GDK_T },
    { "U",      0,  GDK_U },
    { "V",      0,  GDK_V },
    { "W",      0,  GDK_W },
    { "X",      0,  GDK_X },
    { "Y",      0,  GDK_Y },
    { "Z",      0,  GDK_Z },


    { "ENTER",  0,  GDK_Return },
    { "SPACE",  0,  GDK_space },

    { "SHIFT",  0,  GDK_Shift_L },
    { "CTRL",   0,  GDK_Control_L },
    { "ALT",    0,  GDK_Alt_L },
};


void keymap_init()
{
    unsigned i;
    for ( i = sizeof(keymap) / sizeof(KEYMAP); i --; )
    {
        GdkKeymapKey *keys;
        gint nkeys;
        if ( gdk_keymap_get_entries_for_keyval( NULL, keymap[i].keyval, &keys, &nkeys ) )
        {
            keymap[i].key = keys[0].keycode;
            g_free( keys );
        }
    }
}
