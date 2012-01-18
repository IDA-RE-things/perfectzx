#include "../main.h"
#include "../devices.h"
#include "../sound.h"

static unsigned long last_tstate;

static signed volume_saa = 500;

static const double dac_val[16]=
{
	0.0000,
	0.0102,
	0.0148,
	0.0215,
	0.0313,
	0.0466,
	0.0654,
	0.1091,
	0.1296,
	0.2064,
	0.2954,
	0.3764,
	0.4961,
	0.6393,
	0.8103,
	1.0000
};

static struct saa_t
{
	unsigned char reg_latch;
	struct
	{
	   	unsigned tone_freq[6];
	   	unsigned tone_octave[6];
	   	unsigned tone_period[6];
	};

	unsigned tone_count[6];
	unsigned signal[3];

	unsigned sound[6];
	unsigned last_sound[6];
	unsigned long last_tstate;
} saa;

static void saa_tick( struct saa_t *saa )
{
	unsigned c;

    for ( c = 0; c < 6; c ++ )
    {
        if ( ++saa->tone_count[c] >= saa->tone_period[c] )
            { saa->signal[c] = !saa->signal[c]; saa->tone_count[c]=0; }
    }

	for ( c = 0; c < 6; c ++ )
    {
		saa->sound[c] = saa->signal[c] ? 0xF : 0;
    }
}

static void process_saa( unsigned long tstate )
{
    tstate = tstate * 80000 / zxcpu_tstates_frame;
    while ( tstate > last_tstate )
    {
        saa_tick( &saa );
        last_tstate ++;
        if ( memcmp( saa.sound, saa.last_sound, sizeof(saa.sound) ) || last_tstate >= 80000 )
        {
            /*add_sound( saa.last_tstate, last_tstate, zxcpu_tstates_frame,
                       ( dac_val[ay.last_sound[0]] + dac_val[ay.last_sound[1]]*0.7 + dac_val[ay.last_sound[2]]*0.2 ) * volume_ay,
                       ( dac_val[ay.last_sound[2]] + dac_val[ay.last_sound[1]]*0.7 + dac_val[ay.last_sound[0]]*0.2 ) * volume_ay );*/
            add_sound( saa.last_tstate, last_tstate, 80000,
                      (saa.last_sound[0] + saa.last_sound[1] + saa.last_sound[2]) * volume_saa,
                      (saa.last_sound[3] + saa.last_sound[4] + saa.last_sound[5]) * volume_saa );
            saa.last_tstate = last_tstate;
            memcpy( saa.last_sound, saa.sound, sizeof(saa.sound) );
        }
    }
}

static void reset()
{
    process_saa( zxcpu_tstates );
    memset( &saa, 0, sizeof( struct saa_t ) );
}

static void frame()
{
    process_saa( zxcpu_tstates_frame );
    last_tstate = 0;
    saa.last_tstate = 0;
}

inline void calculate_period( struct saa_t *saa, int channel )
{
    //saa->tone_period[channel] = (0x100 << (7-saa->tone_octave[channel])) - saa->tone_freq[channel];
    saa->tone_period[channel] =
        (511 - saa->tone_freq[channel]) << (7 - saa->tone_octave[channel]);
}

static int port_out(Z80EX_CONTEXT *cpu, Z80EX_WORD port, Z80EX_BYTE value)
{
    PORT_TEST(0x1FF, 0x1FF)
    {
        if ( value < 0x20 )
            saa.reg_latch = value;
    }
    PORT_TEST(0x0FF, 0x1FF)
    {
        process_saa( zxcpu_tstates );

        switch ( saa.reg_latch )
        {
            case 0x08:
            case 0x09:
            case 0x0A:
            case 0x0B:
            case 0x0C:
            case 0x0D:
                {
                    int chan = saa.reg_latch - 0x08;
                    saa.tone_freq[chan] = value & 0xFF;
                    calculate_period( &saa, chan );
                }
                break;
            case 0x10:
            case 0x11:
            case 0x12:
                {
                    int chans = (saa.reg_latch - 0x10) * 2;
                    saa.tone_octave[chans+0] = value & 0x07;
                    saa.tone_octave[chans+1] = (value & 0x70) >> 4;
                    calculate_period( &saa, chans + 0 );
                    calculate_period( &saa, chans + 1 );
                }
                break;
        }
    }

    return ( 0 );
}

static int port_in(Z80EX_CONTEXT *cpu, Z80EX_WORD port, Z80EX_BYTE *value)
{
    PORT_TEST(0xFFFD, 0xC002)
    {

    }

    return 0;
}

static void init()
{
	last_tstate = 0;
	saa.last_tstate = 0;
	memset( saa.last_sound, 0, sizeof(saa.last_sound) );
}

static void uninit()
{

}

SDevice zxdevice_saa =
	{
		NULL, NULL,
		port_out, port_in,
		init, uninit,
		reset,
		frame,
		//NULL,
		NULL, NULL,
		NULL
	};
