#include "../main.h"
#include "../devices.h"

#include "../sound.h"


#define ROM_PAGE(p) (mem_rom+(p)*0x4000)
#define RAM_PAGE(p) (mem_ram+(p)*0x4000)
static Z80EX_BYTE *mem_rom, *mem_ram;
static Z80EX_BYTE *mem_map[4];
static Z80EX_BYTE p7FFD_state;
static Z80EX_BYTE p1FFD_state;
static Z80EX_BYTE p7EFD_state;
static Z80EX_BYTE p78FD_state;
static Z80EX_BYTE pDFFD_state;
static Z80EX_BYTE p00_state;
static char gmx_magic_shift;

static int trd_act;

static unsigned ram_page, rom_page;

static int beeper_st;
static unsigned long beeper_last_tstate;
static signed volume_beep = 5000.0;

static void update_ports()
{
    if ( trd_act )
        mem_map[0] = ROM_PAGE(((p7FFD_state & 0x10) ? 3 : 2) | ((p7EFD_state & 0x70) >> 2));
    else
        mem_map[0] = ROM_PAGE(((p7FFD_state & 0x10) ? 1 : 0) | ((p7EFD_state & 0x70) >> 2));
    if ( p1FFD_state & 2 )
        mem_map[0] = ROM_PAGE(2 | ((p7EFD_state & 0x70) >> 2));
    if ( p1FFD_state & 1 )
        mem_map[0] = RAM_PAGE(0);
    if ( p1FFD_state & 4 )
        mem_map[0] = ROM_PAGE(3 | (p7EFD_state & 0x70) >> 2);
    mem_map[2] = RAM_PAGE((p78FD_state & 0x7F) ^ 2);
    mem_map[3] = RAM_PAGE((p7FFD_state & 7) | ((p1FFD_state & 0x10) >> 1) | ((pDFFD_state & 7) << 4));
    ram_page = (mem_map[3] - RAM_PAGE(0))/0x4000;
    rom_page = (mem_map[0] - ROM_PAGE(0))/0x4000;
}

static void reset()
{
    p7FFD_state = 0;
    p1FFD_state = 0;
    p7EFD_state = 0;
    p78FD_state = 0;
    pDFFD_state = 0;
    update_ports();

    video_memory = RAM_PAGE(5);
    video_border = 0x00;
    video_render_std( zxcpu_tstates_frame );
}

static int memread( Z80EX_CONTEXT *cpu, Z80EX_WORD addr, Z80EX_BYTE *value, int m1_state )
{
    if ( m1_state && !trd_act && addr >= 0x3D00 && addr < 0x4000 && p7FFD_state & 0x10 )
    {
        trd_act = 1;
        update_ports();
    }
    if ( m1_state && trd_act && addr >= 0x4000 )
    {
        trd_act = 0;
        update_ports();
    }

    *value = mem_map[addr >> 14][addr & 0x3FFF];

	return 1;
}

static int memwrite( Z80EX_CONTEXT *cpu, Z80EX_WORD addr, Z80EX_BYTE value )
{
    if ((mem_map[addr >> 14] == video_memory) && ((addr&0x3FFF) < 6912))
        video_render_std(zxcpu_tstates);
    if (addr >= 0x4000)
        mem_map[addr >> 14][addr & 0x3FFF] = value;
	return 1;
}

static void frame()
{
    // update rest of the screen
    video_render_std( zxcpu_tstates_frame );
    video_last_tstate = 0;

    add_sound( beeper_last_tstate, zxcpu_tstates_frame, zxcpu_tstates_frame, beeper_st ? volume_beep : 0.0,
                                                                             beeper_st ? volume_beep : 0.0 );
    beeper_last_tstate = 0;
}


static int port_out(Z80EX_CONTEXT *cpu, Z80EX_WORD port, Z80EX_BYTE value)
{
    PORT_TEST(0x00, 0xFF)
    {
        p00_state = value;
        if ( p00_state & 0x08 )
        {
            gmx_magic_shift = 0x80 | (p00_state & 7);

            if (!(p00_state & 0x10))
                z80ex_reset( cpu );
        }
    }
    PORT_TEST(0xFD, 0x23)
    {
        switch( port >> 8 )
        {
            case 0x1F:
                p1FFD_state = value;
                break;
            case 0x78:
                p78FD_state = value;
                break;
            case 0x7E:
                p7EFD_state = value;
                break;
            case 0xDF:
                pDFFD_state = value;
                break;

        }
        update_ports();

    }
    PORT_TEST(0x7FFD, 0xFF23)
    {
        if ( !(p7FFD_state & 0x20) )
        {
            if ( (p7FFD_state ^ value) & 8 )        // if screen page changed
                video_render_std( zxcpu_tstates );  // print screen until change

            p7FFD_state = value;
            update_ports();

            // change video memory pointer
            video_memory = RAM_PAGE((p7FFD_state & 8) ? 7 : 5);
        }
    }
    PORT_TEST(0xFE, 0x03)
    {
        video_render_std(zxcpu_tstates);
        video_border = value & 7;

        add_sound( beeper_last_tstate, zxcpu_tstates, zxcpu_tstates_frame, beeper_st ? volume_beep : 0.0,
                                                                           beeper_st ? volume_beep : 0.0 );
        beeper_last_tstate = zxcpu_tstates;
        beeper_st = value & 0x10;
    }
    //printf("port %.4x!\n", port);

    return 0;
}

static int port_in(Z80EX_CONTEXT *cpu, Z80EX_WORD port, Z80EX_BYTE *value)
{
    PORT_TEST(0xFD, 0x23)
    {
        switch ( port >> 8 )
		{
			case 0x7A:
				*value = (p7FFD_state & 7)
					| ((p1FFD_state & 0x10) >> 1)
					| ((pDFFD_state & 7) << 4)
					| ((video_border & 1) << 7);
                break;
			case 0x7E:
				*value = ((p7FFD_state & 0x20) >> 5)
					| ((p7FFD_state & 8) >> 2)
					| ((p7EFD_state & 0x80) >> 5)
					| (p7EFD_state & 8)
					| ((p00_state & 0x80) >> 3)
					| (p00_state & 0x20)
					| ((p1FFD_state & 1) << 6)
					| ((video_border & 4) << 5);
                break;
			case 0x78:
				*value = (p78FD_state & 0x7F) | ((video_border & 2) << 6);
				*value |= (gmx_magic_shift&1);
				gmx_magic_shift >>= 1;
				break;
		}
    }
    PORT_TEST(0xFE, 0x01)
    {
        unsigned i;
        Z80EX_BYTE val = 0xFF;
        for ( i = 0; i < 8; i ++ )
            if ( (~port) & ( 0x0100 << i ) )
                val &= zxkey_matrix[i];
        *value = val;
    }

    return 0;
}

static void init()
{
    int res;

	zxcpu_tstates_frame = 320*224;//69888;
	zxcpu_int_start = (48+192+47)*224 + 192;
    zxcpu_int_end = zxcpu_int_start + 32;

	mem_rom = (Z80EX_BYTE*)malloc(8*4*0x4000);
	mem_ram = (Z80EX_BYTE*)malloc(2048*1024);

	mem_map[0] = ROM_PAGE(0);
	mem_map[1] = RAM_PAGE(5);
	mem_map[2] = RAM_PAGE(2);
	mem_map[3] = RAM_PAGE(0);

    update_ports();

	video_memory = RAM_PAGE(5);
	video_border = 0x00;

	FILE *romf;
	romf = fopen("ROMs/gmx12.rom", "rb");
	res = fread(mem_rom, 0x4000, 8*4, romf);
	fclose(romf);
}

static void uninit()
{
    free(mem_rom);
    free(mem_ram);
}


/*static VAR_LINK links[] =
	{

	};*/

SDevice zxdevice_gmx =
	{
		NULL, NULL,
		port_out, port_in,
		init, uninit,
		reset,
		frame,
		//NULL,
		memread, memwrite,
		NULL
	};

