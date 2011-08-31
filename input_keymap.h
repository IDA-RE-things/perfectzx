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
    { "ESC",    0,  GDK_KEY_Escape },
    { "F1",     0,  GDK_KEY_F1 },
    { "F2",     0,  GDK_KEY_F2 },
    { "F3",     0,  GDK_KEY_F3 },
    { "F4",     0,  GDK_KEY_F4 },
    { "F5",     0,  GDK_KEY_F5 },
    { "F6",     0,  GDK_KEY_F6 },
    { "F7",     0,  GDK_KEY_F7 },
    { "F8",     0,  GDK_KEY_F8 },
    { "F9",     0,  GDK_KEY_F9 },
    { "F10",    0,  GDK_KEY_F10 },
    { "F11",    0,  GDK_KEY_F11 },
    { "F12",    0,  GDK_KEY_F12 },

    { "1",      0,  GDK_KEY_1 },
    { "2",      0,  GDK_KEY_2 },
    { "3",      0,  GDK_KEY_3 },
    { "4",      0,  GDK_KEY_4 },
    { "5",      0,  GDK_KEY_5 },
    { "6",      0,  GDK_KEY_6 },
    { "7",      0,  GDK_KEY_7 },
    { "8",      0,  GDK_KEY_8 },
    { "9",      0,  GDK_KEY_9 },
    { "0",      0,  GDK_KEY_0 },

    { "A",      0,  GDK_KEY_A },
    { "B",      0,  GDK_KEY_B },
    { "C",      0,  GDK_KEY_C },
    { "D",      0,  GDK_KEY_D },
    { "E",      0,  GDK_KEY_E },
    { "F",      0,  GDK_KEY_F },
    { "G",      0,  GDK_KEY_G },
    { "H",      0,  GDK_KEY_H },
    { "I",      0,  GDK_KEY_I },
    { "J",      0,  GDK_KEY_J },
    { "K",      0,  GDK_KEY_K },
    { "L",      0,  GDK_KEY_L },
    { "M",      0,  GDK_KEY_M },
    { "N",      0,  GDK_KEY_N },
    { "O",      0,  GDK_KEY_O },
    { "P",      0,  GDK_KEY_P },
    { "Q",      0,  GDK_KEY_Q },
    { "R",      0,  GDK_KEY_R },
    { "S",      0,  GDK_KEY_S },
    { "T",      0,  GDK_KEY_T },
    { "U",      0,  GDK_KEY_U },
    { "V",      0,  GDK_KEY_V },
    { "W",      0,  GDK_KEY_W },
    { "X",      0,  GDK_KEY_X },
    { "Y",      0,  GDK_KEY_Y },
    { "Z",      0,  GDK_KEY_Z },


    { "ENTER",  0,  GDK_KEY_Return },
    { "SPACE",  0,  GDK_KEY_space },

    { "SHIFT",  0,  GDK_KEY_Shift_L },
    { "CTRL",   0,  GDK_KEY_Control_L },
    { "ALT",    0,  GDK_KEY_Alt_L },
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
