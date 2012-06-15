#include <gdk/gdkkeys.h>
#include <gdk/gdkkeysyms.h>

typedef guint keyhardcode;

typedef struct
{
    const char *name;
    keyhardcode keyval;
} KEYMAP;

KEYMAP keymap[] = {
    { "ESC",        GDK_KEY_Escape },
    { "F1",         GDK_KEY_F1 },
    { "F2",         GDK_KEY_F2 },
    { "F3",         GDK_KEY_F3 },
    { "F4",         GDK_KEY_F4 },
    { "F5",         GDK_KEY_F5 },
    { "F6",         GDK_KEY_F6 },
    { "F7",         GDK_KEY_F7 },
    { "F8",         GDK_KEY_F8 },
    { "F9",         GDK_KEY_F9 },
    { "F10",        GDK_KEY_F10 },
    { "F11",        GDK_KEY_F11 },
    { "F12",        GDK_KEY_F12 },

    { "1",          GDK_KEY_1 },
    { "2",          GDK_KEY_2 },
    { "3",          GDK_KEY_3 },
    { "4",          GDK_KEY_4 },
    { "5",          GDK_KEY_5 },
    { "6",          GDK_KEY_6 },
    { "7",          GDK_KEY_7 },
    { "8",          GDK_KEY_8 },
    { "9",          GDK_KEY_9 },
    { "0",          GDK_KEY_0 },

    { "A",          GDK_KEY_A },
    { "B",          GDK_KEY_B },
    { "C",          GDK_KEY_C },
    { "D",          GDK_KEY_D },
    { "E",          GDK_KEY_E },
    { "F",          GDK_KEY_F },
    { "G",          GDK_KEY_G },
    { "H",          GDK_KEY_H },
    { "I",          GDK_KEY_I },
    { "J",          GDK_KEY_J },
    { "K",          GDK_KEY_K },
    { "L",          GDK_KEY_L },
    { "M",          GDK_KEY_M },
    { "N",          GDK_KEY_N },
    { "O",          GDK_KEY_O },
    { "P",          GDK_KEY_P },
    { "Q",          GDK_KEY_Q },
    { "R",          GDK_KEY_R },
    { "S",          GDK_KEY_S },
    { "T",          GDK_KEY_T },
    { "U",          GDK_KEY_U },
    { "V",          GDK_KEY_V },
    { "W",          GDK_KEY_W },
    { "X",          GDK_KEY_X },
    { "Y",          GDK_KEY_Y },
    { "Z",          GDK_KEY_Z },


    { "ENTER",      GDK_KEY_Return },
    { "SPACE",      GDK_KEY_space },

    { "SHIFT",      GDK_KEY_Shift_L },
    { "CTRL",       GDK_KEY_Control_L },
    { "ALT",        GDK_KEY_Alt_L },

    { "TAB",        GDK_KEY_Tab },

    { "LEFT",       GDK_KEY_Left },
    { "RIGHT",      GDK_KEY_Right },
    { "UP",         GDK_KEY_Up },
    { "DOWN",       GDK_KEY_Down },

    { "BACKSPACE",  GDK_KEY_BackSpace },
    { "CAPSLOCK",   GDK_KEY_Caps_Lock },

    { "`",          GDK_KEY_grave },
    { "\"",         GDK_KEY_quotedbl },
    { "'",          GDK_KEY_apostrophe },
    { ";",          GDK_KEY_semicolon },
    { ":",          GDK_KEY_colon },
    { ".",          GDK_KEY_period },
    { ",",          GDK_KEY_comma },
};


void keymap_init()
{
    /*unsigned i;
    for ( i = sizeof(keymap) / sizeof(KEYMAP); i --; )
    {
        GdkKeymapKey *keys;
        gint nkeys;
        if ( gdk_keymap_get_entries_for_keyval( NULL, keymap[i].keyval, &keys, &nkeys ) )
        {
            keymap[i].key = keys[0].keycode;
            g_free( keys );
        }
    }*/
}
