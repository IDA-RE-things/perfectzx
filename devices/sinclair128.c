#include "../main.h"
#include "../devices.h"

#include "../sound.h"

#define ROM_PAGE(p) (mem_rom+(p)*0x4000)
#define RAM_PAGE(p) (mem_ram+(p)*0x4000)
Z80EX_BYTE *mem_rom, *mem_ram;
Z80EX_BYTE *mem_map[4];
Z80EX_BYTE p7FFD_state;

int beeper_st;
unsigned long beeper_last_tstate;
double volume_beep = 5000.0;

static void reset()
{
    p7FFD_state = 0;
    video_memory = RAM_PAGE(5);
	video_border = 0x02;
}

static int memread( Z80EX_CONTEXT *cpu, Z80EX_WORD addr, Z80EX_BYTE *value )
{
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
    video_render_std(zxcpu_tstates);
    video_last_tstate = 0;

    add_sound_nf( beeper_last_tstate, zxcpu_tstates_frame, zxcpu_tstates_frame, beeper_st ? volume_beep : 0.0 );
    beeper_last_tstate = 0;
    //video_render_std_fast();
}

static int port_out(Z80EX_CONTEXT *cpu, Z80EX_WORD port, Z80EX_BYTE value)
{
    PORT_TEST(0x7FFD, 0x8002)
    {
        if (!(p7FFD_state & 0x20))
        {
            if ((p7FFD_state ^ value) & 8)    // if screen page changed
                // print screen until change
                video_render_std(zxcpu_tstates);

            p7FFD_state = value;
            mem_map[0] = ROM_PAGE((p7FFD_state & 0x10) ? 1 : 0);
            mem_map[3] = RAM_PAGE(p7FFD_state & 7);

            // change video memory pointer
            video_memory = RAM_PAGE((p7FFD_state & 8) ? 7 : 5);
        }
    }
    PORT_TEST(0xFE, 0x01)
    {
        video_render_std(zxcpu_tstates);
        video_border = value & 7;

        add_sound_nf( beeper_last_tstate, zxcpu_tstates, zxcpu_tstates_frame, beeper_st ? volume_beep : 0.0 );
        beeper_last_tstate = zxcpu_tstates;
        beeper_st = value & 0x10;
    }

    return 0;
}

static int port_in(Z80EX_CONTEXT *cpu, Z80EX_WORD port, Z80EX_BYTE *value)
{
    PORT_TEST(0xFE, 0x01)
    {
        unsigned i;
        Z80EX_BYTE val = 0xFF;
        for ( i = 0; i < 8; i ++ )
            if ( (~port) & ( 0x0100 << i ) )
                val &= zxkey_matrix[i];
        *value = val;
    }

    return 1;
}

static void init()
{
    int res;

	zxcpu_tstates_frame = 69888;

	mem_rom = (Z80EX_BYTE*)malloc(2*0x4000);
	mem_ram = (Z80EX_BYTE*)malloc(8*0x4000);

	mem_map[0] = ROM_PAGE(0);
	mem_map[1] = RAM_PAGE(5);
	mem_map[2] = RAM_PAGE(2);
	mem_map[3] = RAM_PAGE(0);

	FILE *romf;
	romf = fopen("128.rom", "rb");
	res = fread(mem_rom, 0x4000, 2, romf);
	fclose(romf);

	video_memory = RAM_PAGE(5);
	video_border = 0x00;
}

static void uninit()
{
    free(mem_rom);
    free(mem_ram);
}


VAR_LINK links[] =
	{
		{ "main_port_7FFD", &p7FFD_state }
	};


SDevice zxdevice_sinclair128 =
	{
		NULL, NULL,
		port_out, port_in,
		init, uninit,
		reset,
		frame,
		NULL,
		memread, memwrite,
		links
	};
