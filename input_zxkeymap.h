typedef struct
{
    const char *name;
    const unsigned char key;
} ZXKEYMAP;

typedef struct
{
    keyhardcode keycode;
    unsigned char zxkey;
} ZXKEYBIND;

ZXKEYMAP zxkeymap[] = {
    { "NULL",   0xFF },

    { "1",      0x30 },
    { "2",      0x31 },
    { "3",      0x32 },
    { "4",      0x33 },
    { "5",      0x34 },
    { "6",      0x44 },
    { "7",      0x43 },
    { "8",      0x42 },
    { "9",      0x41 },
    { "0",      0x40 },

    { "Q",      0x20 },
    { "W",      0x21 },
    { "E",      0x22 },
    { "R",      0x23 },
    { "T",      0x24 },
    { "Y",      0x54 },
    { "U",      0x53 },
    { "I",      0x52 },
    { "O",      0x51 },
    { "P",      0x50 },

    { "A",      0x10 },
    { "S",      0x11 },
    { "D",      0x12 },
    { "F",      0x13 },
    { "G",      0x14 },
    { "H",      0x64 },
    { "J",      0x63 },
    { "K",      0x62 },
    { "L",      0x61 },
    { "ENT",    0x60 },

    { "CS",     0x00 },
    { "Z",      0x01 },
    { "X",      0x02 },
    { "C",      0x03 },
    { "V",      0x04 },
    { "B",      0x74 },
    { "N",      0x73 },
    { "M",      0x72 },
    { "SS",     0x71 },
    { "SPC",    0x70 }
};
