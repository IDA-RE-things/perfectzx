#include "../main.h"
#include "../devices.h"

static int port_in(Z80EX_CONTEXT *cpu, Z80EX_WORD port, Z80EX_BYTE *value)
{
    PORT_TEST(0xFFDF, 0x03A0)//0x0320)
    {
        /* axes */
        if ( port & 0x0400 )
        {
            /* y axis, inverted */
            *value = -zxmouse_y / 2;
        }
        else
        {
            /* x axis */
            *value = zxmouse_x / 2;
        }
    }
    PORT_TEST(0xFADF, 0x03A0)//0x0320)
    {
        /* buttons */
        *value = ((*value) & ~3) | (~zxmouse_but & 3);
    }

    return 0;
}

static void init()
{

}

static void uninit()
{

}

SDevice zxdevice_kempston_mouse =
	{
		NULL, NULL,
		NULL, port_in,
		init, uninit,
		NULL,
		NULL,
		//NULL,
		NULL, NULL,
		NULL
	};

