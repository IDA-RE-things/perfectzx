#include "../main.h"
#include "../devices.h"
#include "../sound.h"

static unsigned long last_tstate;

static signed volume_saa = 300;

typedef struct
{
    int l, r;
} volume_t;

static struct saa_t
{
	unsigned reg_latch;

	unsigned tone_freq[6];
    unsigned tone_octave[6];
    unsigned tone_period[6];
    volume_t tone_vol[6];

    unsigned noise_control[2];
    unsigned noise_period[2];

    int freq_en;
    int noise_en;

    int env_control[2];
    int env_reg[2];

	unsigned tone_count[6];
	unsigned noise_count[2];
	unsigned noise_reg[2];

	unsigned freq[6];
	unsigned noise[2];
	unsigned signal[6];
	unsigned envelope[2];

	int sound[2];
	int last_sound[2];
	unsigned long last_tstate;
} saa;

#define GET_COUNTER_EVENT(e,c,p)    \
    {\
        (e) = 0;\
        if ( (c) >= (p) ) (e) = 1;    \
        else if ( (c) + cycles >= (p) ) (e) = (p) - (c);    \
        if ( (e) && (e) < last_cycle ) last_cycle = (e);    \
    }

static unsigned saa_tick( struct saa_t *saa, unsigned cycles )
{
	unsigned c;
	//int int_change = 0;
	int chan_event[6] = { 0 };
	int noise_event[2] = { 0 };
	unsigned last_cycle = cycles;

    for ( c = 0; c < 6; c ++ )
        GET_COUNTER_EVENT( chan_event[c], saa->tone_count[c], saa->tone_period[c] );

    for ( c = 0; c < 2; c ++ )
        if ( saa->noise_control[c] < 3 )
            GET_COUNTER_EVENT( noise_event[c], saa->noise_count[c], saa->noise_period[c] )
        else
            noise_event[c] = chan_event[c*3];

    for ( c = 0; c < 6; c ++ )
    {
        if ( chan_event[c] == last_cycle )
        {
            int env = c / 3;

            saa->freq[c] = !saa->freq[c];
            saa->tone_count[c] = 0;

            if ( (c % 3) == 1 && !(saa->env_control[env] & 0x20) )
            {
                if ( saa->env_control[env] & 0x80 )
                {
                    int env_max = saa->env_control[env] & 0x10 ? 7 : 15;
                    switch ( saa->env_control[env] & 0xE )
                    {
                        case 0x0: saa->envelope[env] = 0; break;
                        case 0x2: saa->envelope[env] = env_max; break;
                        case 0x4:
                            saa->envelope[env] = env_max - saa->env_reg[env];
                            if ( saa->env_reg[env] < env_max )
                                saa->env_reg[env] ++;
                            break;
                        case 0x6:
                            saa->envelope[env] = env_max - saa->env_reg[env];
                            if ( saa->env_reg[env] < env_max )
                                saa->env_reg[env] ++;
                            else
                                saa->env_reg[env] = 0;
                            break;
                        case 0xC:
                            saa->envelope[env] = saa->env_reg[env] <= env_max ?
                                                    saa->env_reg[env] : 0;
                            if ( saa->env_reg[env] <= env_max )
                                saa->env_reg[env] ++;
                            break;
                        case 0xE:
                            saa->envelope[env] = saa->env_reg[env];
                            if ( saa->env_reg[env] < env_max )
                                saa->env_reg[env] ++;
                            else
                                saa->env_reg[env] = 0;
                            break;
                        case 0x8:
                            saa->envelope[env] = saa->env_reg[env] > env_max ?
                                                    (env_max + 1) * 2 - saa->env_reg[env] - 1 :
                                                    saa->env_reg[env];
                            if ( saa->env_reg[env] < ((env_max + 1) * 2 - 1) )
                                saa->env_reg[env] ++;
                            break;
                        case 0xA:
                            saa->envelope[env] = saa->env_reg[env] > env_max ?
                                                    (env_max + 1) * 2 - saa->env_reg[env] - 1 :
                                                    saa->env_reg[env];
                            if ( ++saa->env_reg[env] >= (env_max + 1) * 2 )
                                saa->env_reg[env] = 0;
                            break;
                    }
                    //printf("env %d %.2x %.2d %.2d %.2d\n", c, saa->env_control[env], saa->envelope[env], saa->env_reg[env], (env_max + 1) * 2 );
                }
                else
                {
                    saa->env_reg[env] = 0;
                }
            }
        }
        else
        {
            saa->tone_count[c] += last_cycle;
        }
    }

    for ( c = 0; c < 2; c ++ )
    {
        if ( noise_event[c] == last_cycle )
        {
            static unsigned tmp;
            tmp = ( saa->noise_reg[c] >> 15 ) & 1;
            saa->noise_reg[c] <<= 1;
            saa->noise_reg[c] |= ( ( saa->noise[c] ^ ( saa->noise_reg[c] >> 14 ) ) & 1 ) ^ 1;

            saa->noise[c] = tmp;
            saa->noise_count[c] = 0;
        }
        else
        {
            saa->noise_count[c] += last_cycle;
        }
    }

    for ( c = 0; c < 6; c ++ )
    {
        saa->signal[c] = (saa->freq_en & (1 << c) ? saa->freq[c] : 1) &
                         (saa->noise_en & (1 << c) ? saa->noise[c/3] : 1);
    }

    saa->sound[0] = saa->sound[1] = 0;
	for ( c = 0; c < 6; c ++ )
	{
	    int env = c/3;
        if ( saa->signal[c] )
        {
            if ( (c % 3 == 2) && saa->env_control[env] & 0x80 )
            {
                if ( saa->env_control[env] & 0x10 )
                {
                    saa->sound[0] += saa->tone_vol[c].l * saa->envelope[env] / 8;
                    saa->sound[1] += saa->tone_vol[c].r *
                        ( (saa->env_control[env] & 1) ?
                            7 - saa->envelope[env] :
                            saa->envelope[env] ) / 8;
                }
                else
                {
                    saa->sound[0] += saa->tone_vol[c].l * saa->envelope[env] / 16;
                    saa->sound[1] += saa->tone_vol[c].r *
                        ( (saa->env_control[env] & 1) ?
                            15 - saa->envelope[env] :
                            saa->envelope[env] ) / 16;
                }
            }
            else
            {
                saa->sound[0] += saa->tone_vol[c].l;
                saa->sound[1] += saa->tone_vol[c].r;
            }
        }
	}

    return( last_cycle );
}

static void process_saa( unsigned long tstate )
{
    unsigned ret;

    tstate = tstate * 80000 / zxcpu_tstates_frame;
    while ( tstate > last_tstate )
    {
        ret = saa_tick( &saa, tstate - last_tstate );

        if ( ret )
            last_tstate += ret;
        else
            last_tstate = tstate;

        /*if ( last_tstate >= 80000 )
            printf( "%d %ld\n", ret, last_tstate );*/

        //if ( memcmp( saa.sound, saa.last_sound, sizeof(saa.sound) ) || last_tstate >= 80000 )
        //if ( ret > 0 || last_tstate >= 80000 )
        {
            /*add_sound( saa.last_tstate, last_tstate, zxcpu_tstates_frame,
                       ( dac_val[ay.last_sound[0]] + dac_val[ay.last_sound[1]]*0.7 + dac_val[ay.last_sound[2]]*0.2 ) * volume_ay,
                       ( dac_val[ay.last_sound[2]] + dac_val[ay.last_sound[1]]*0.7 + dac_val[ay.last_sound[0]]*0.2 ) * volume_ay );*/

            add_sound( saa.last_tstate, last_tstate, 80000,
                       saa.last_sound[0] * volume_saa,
                       saa.last_sound[1] * volume_saa );
            saa.last_tstate = last_tstate;
            //memcpy( saa.last_sound, saa.sound, sizeof(saa.sound) );
            saa.last_sound[0] = saa.sound[0];
            saa.last_sound[1] = saa.sound[1];
        }
    }
}

inline void calculate_period( struct saa_t *saa, int channel )
{
    //saa->tone_period[channel] = (0x100 << (7-saa->tone_octave[channel])) - saa->tone_freq[channel];
    saa->tone_period[channel] =
        (511 - saa->tone_freq[channel]) << (7 - saa->tone_octave[channel]);
}

static void reset_saa()
{
    int c;

    memset( &saa, 0, sizeof( struct saa_t ) );

    for ( c = 0; c < 6; c ++ )
        calculate_period( &saa, c );
    for ( c = 0; c < 2; c ++ )
        saa.noise_period[c] = 0x80;
}

static void reset()
{
    process_saa( zxcpu_tstates );
    reset_saa();
}

static void frame()
{
    process_saa( zxcpu_tstates_frame );
    last_tstate = 0;
    saa.last_tstate = 0;
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
            case 0x00:
            case 0x01:
            case 0x02:
            case 0x03:
            case 0x04:
            case 0x05:
                {
                    int chan = saa.reg_latch;
                    saa.tone_vol[chan].l = (value & 0xF);
                    saa.tone_vol[chan].r = (value & 0xF0) >> 4;
                }
                break;
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
            case 0x14:
                saa.freq_en = value;
                break;
            case 0x15:
                saa.noise_en = value;
                break;
            case 0x16:
                {
                    int c;

                    saa.noise_control[0] = value & 0x03;
                    saa.noise_control[1] = (value & 0x30) >> 4;

                    for ( c = 0; c < 2; c ++ )
                    {
                        if ( saa.noise_control[c] != 3 )
                            saa.noise_period[c] = 0x80 << saa.noise_control[c];
                        /*else
                            printf("dunno what to do...\n");*/
                    }
                }
                break;
            case 0x18:
                saa.env_control[0] = value;
                break;
            case 0x19:
                saa.env_control[1] = value;
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

	reset_saa();
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
