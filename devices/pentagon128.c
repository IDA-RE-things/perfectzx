#include "../main.h"
#include "../devices.h"

#include "../sound.h"
#include "../tape.h"
#include "../wd1793.h"


#define ROM_PAGE(p) (mem_rom+(p)*0x4000)
#define RAM_PAGE(p) (mem_ram+(p)*0x4000)
static Z80EX_BYTE *mem_rom, *mem_ram;
static Z80EX_BYTE *mem_map[4];
static Z80EX_BYTE p7FFD_state;

static int beeper_st;
static unsigned long beeper_last_tstate;
static signed volume_beep = 5000.0;

static int trd_act;
wd1793_t fdc;

static void reset()
{
    p7FFD_state = 0;
    mem_map[0] = ROM_PAGE(0);
	mem_map[3] = RAM_PAGE(0);
	trd_act = 0;

    video_memory = RAM_PAGE(5);
    //video_border = 0x00;
    video_render_std( zxcpu_tstates_frame );

    wd1793_reset( &fdc );
}

static int memread( Z80EX_CONTEXT *cpu, Z80EX_WORD addr, Z80EX_BYTE *value, int m1_state  )
{
    *value = mem_map[addr >> 14][addr & 0x3FFF];

    if ( m1_state && !trd_act && addr >= 0x3D00 && addr < 0x4000 && p7FFD_state & 0x10 )
    {
        trd_act = 1;
        mem_map[0] = ROM_PAGE((p7FFD_state & 0x10) ? 3 : 2);
    }
    if ( m1_state && trd_act && addr >= 0x4000 )
    {
        trd_act = 0;
        mem_map[0] = ROM_PAGE((p7FFD_state & 0x10) ? 1 : 0);
    }

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

    wd1793_frame( &fdc );
}

static int port_out(Z80EX_CONTEXT *cpu, Z80EX_WORD port, Z80EX_BYTE value)
{
    PORT_TEST(0x7FFD, 0x8002)
    {
        if ( !(p7FFD_state & 0x20) )
        {
            if ( (p7FFD_state ^ value) & 8 )        // if screen page changed
                video_render_std( zxcpu_tstates );  // print screen until change

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

        add_sound( beeper_last_tstate, zxcpu_tstates, zxcpu_tstates_frame, beeper_st ? volume_beep : 0.0,
                                                                           beeper_st ? volume_beep : 0.0 );
        beeper_last_tstate = zxcpu_tstates;
        beeper_st = value & 0x10;
    }
    if ( trd_act )
    {
        PORT_TEST(0xFF, 0x03)
        {
            if ( port & 0x80 )
            {

            }
            else
                wd1793_write( &fdc, port >> 5, value );
        }
    }

    return 0;
}

static int port_in(Z80EX_CONTEXT *cpu, Z80EX_WORD port, Z80EX_BYTE *value)
{
    PORT_TEST(0xFE, 0x01)
    {
        unsigned i;
        Z80EX_BYTE val = 0xBF;
        for ( i = 0; i < 8; i ++ )
            if ( (~port) & ( 0x0100 << i ) )
                val &= zxkey_matrix[i];

        val |= tape_get() ? 0x40 : 0;

        *value = val;
    }

    /* kempston dummy */
    PORT_TEST(0x1F, 0xFF)
    {
        *value = 0x00;
    }

    if ( trd_act )
    {
        PORT_TEST(0xFF, 0x03)
        {
            if ( port & 0x80 )
            {
                unsigned stat = wd1793_status( &fdc );
                *value = ( stat & WD1793_DRQ ? 0x40 : 0 ) |
                         ( stat & WD1793_INTRQ ? 0x80 : 0 );
            }
            else
                *value = wd1793_read( &fdc, port >> 5 );
        }
    }

    return 0;
}

/* to be moved and rewriten */
static int load_sna( const char *flname )
{
    FILE *snafile;
    unsigned short tmp;
    if ( ( snafile = fopen( flname, "rb" ) ) != NULL )
    {
        fseek(snafile,0,SEEK_END);
        unsigned long snasize=ftell(snafile);
        if(snasize>=0xC01B) // this is sna
        {
            zx_reset();

            fseek( snafile, 0, SEEK_SET );
            fread( &tmp, 1, 1, snafile ); z80ex_set_reg( zxcpu, regI, tmp );
            fread( &tmp, 2, 1, snafile ); z80ex_set_reg( zxcpu, regHL_, tmp );
            fread( &tmp, 2, 1, snafile ); z80ex_set_reg( zxcpu, regDE_, tmp );
            fread( &tmp, 2, 1, snafile ); z80ex_set_reg( zxcpu, regBC_, tmp );
            fread( &tmp, 2, 1, snafile ); z80ex_set_reg( zxcpu, regAF_, tmp );
            fread( &tmp, 2, 1, snafile ); z80ex_set_reg( zxcpu, regHL, tmp );
            fread( &tmp, 2, 1, snafile ); z80ex_set_reg( zxcpu, regDE, tmp );
            fread( &tmp, 2, 1, snafile ); z80ex_set_reg( zxcpu, regBC, tmp );
            fread( &tmp, 2, 1, snafile ); z80ex_set_reg( zxcpu, regIY, tmp );
            fread( &tmp, 2, 1, snafile ); z80ex_set_reg( zxcpu, regIX, tmp );

            fread( &tmp, 1, 1, snafile );
            z80ex_set_reg( zxcpu, regIFF1, (tmp >> 2) & 1 );
            z80ex_set_reg( zxcpu, regIFF2, (tmp >> 1) & 1 );
            fread( &tmp, 1, 1, snafile ); z80ex_set_reg( zxcpu, regR, tmp );
            fread( &tmp, 2, 1, snafile ); z80ex_set_reg( zxcpu, regAF, tmp );
            fread( &tmp, 2, 1, snafile ); z80ex_set_reg( zxcpu, regSP, tmp );
            z80ex_set_reg( zxcpu, regIM, fgetc(snafile) );

            video_border = fgetc( snafile ) & 7;
            fread( RAM_PAGE(5), 0x4000, 1, snafile );
            fread( RAM_PAGE(2), 0x4000, 1, snafile );
            fread( RAM_PAGE(0), 0x4000, 1, snafile );

            /*zxcpu_tstates*/
            zxcpu_tstates_main = zxcpu_int_start;
            p7FFD_state = 0;
            if ( snasize > 0xC01B )  // this is extention to 128k sna
            {
                unsigned char p7FFD;
                fread( &tmp, 2, 1, snafile ); z80ex_set_reg( zxcpu, regPC, tmp );
                p7FFD = fgetc(snafile);

                fgetc(snafile);
                if ( ( p7FFD & 0x7 ) != 0 )
                    memcpy( RAM_PAGE(p7FFD & 0x7), RAM_PAGE(0), 0x4000 );
                unsigned p;
                for ( p = 0; p < 8; p ++ )
                    if ( ( p != 2 ) && ( p != 5 ) && ( p != ( p7FFD & 0x7 ) ) )
                        fread( RAM_PAGE(p), 0x4000, 1, snafile );
                port_out( zxcpu, 0x7FFD, p7FFD );
            }
            else
            {
                Z80EX_WORD sp;
                sp = z80ex_get_reg( zxcpu, regSP );
                memread( zxcpu, sp ++, ((Z80EX_BYTE *)&tmp) + 0, 0 );
                sp %= 0xFFFF;
                memread( zxcpu, sp ++, ((Z80EX_BYTE *)&tmp) + 1, 0 );
                sp %= 0xFFFF;
                z80ex_set_reg( zxcpu, regSP, sp );
                z80ex_set_reg( zxcpu, regPC, tmp );

                port_out( zxcpu, 0x7FFD, 0x10 );
            }
        }
        fclose(snafile);

        return 0;
    }
    return -1;
}
/* to be moved and rewriten */


static void init()
{
    int res;

	zxcpu_tstates_frame = 320*224;//69888;
	zxcpu_int_start = (48+192+47)*224 + 190;
    zxcpu_int_end = zxcpu_int_start + 32;

	mem_rom = (Z80EX_BYTE*)malloc(4*0x4000);
	mem_ram = (Z80EX_BYTE*)malloc(8*0x4000);

	mem_map[0] = ROM_PAGE(0);
	mem_map[1] = RAM_PAGE(5);
	mem_map[2] = RAM_PAGE(2);
	mem_map[3] = RAM_PAGE(0);

	video_memory = RAM_PAGE(5);
	video_border = 0x00;

	FILE *romf;
	/*romf = fopen("128.rom", "rb");
	res = fread(mem_rom, 0x4000, 2, romf);
	fclose(romf);*/
	romf = fopen("roms/128_low.rom", "rb");
	res = fread(ROM_PAGE(0), 0x4000, 1, romf);
	fclose(romf);
	romf = fopen("roms/sos.rom", "rb");
	res = fread(ROM_PAGE(1), 0x4000, 1, romf);
	fclose(romf);
	romf = fopen("roms/gluk.rom", "rb");
	res = fread(ROM_PAGE(2), 0x4000, 1, romf);
	fclose(romf);
	romf = fopen("roms/dos.rom", "rb");
	res = fread(ROM_PAGE(3), 0x4000, 1, romf);
	fclose(romf);
}

static void uninit()
{
    free(mem_rom);
    free(mem_ram);
}


/*static VAR_LINK links[] =
	{
		{ "main_port_7FFD", &p7FFD_state }
	};*/

static file_type_t ft_snapshot[] =
    {
        { "sna", load_sna },
        { 0 }
    };

static file_fiter_t files_open[] =
	{
		{ "Snapshot", ft_snapshot },
		{ 0 }
	};


SDevice zxdevice_pentagon128 =
	{
		NULL, NULL,
		port_out, port_in,
		init, uninit,
		reset,
		frame,
		//NULL,
		memread, memwrite,
		files_open
	};
