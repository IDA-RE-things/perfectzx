#include "../main.h"
#include "../devices.h"
#include "../sound.h"

static unsigned long last_tstate;
static int standby_cd;

static signed volume_ay = 10000;

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

static struct ay_t
{
	unsigned char reg_latch;
	struct
	{
	   	unsigned tone_period[3];
    	unsigned noise_period;
		unsigned control;
		unsigned channel_vol[3];
		unsigned envelope_period;
        unsigned envelope_shape;
	};

	unsigned tone_count[3];
	unsigned noise_count;
	unsigned envelope_count;
	unsigned envelope_phase;
	unsigned envelope_state;
	int signal[3];
	unsigned noise_reg;
	int noise_signal;

	unsigned sound[3];
	unsigned last_sound[3];
	unsigned long last_tstate;
} ay;

#define GET_COUNTER_EVENT(e,c,p)    \
    {\
        (e) = 0;\
        if ( (c) >= (p) ) (e) = 1;    \
        else if ( (c) + cycles >= (p) ) (e) = (p) - (c);    \
        if ( (e) && (e) < last_cycle ) last_cycle = (e);    \
    }

static unsigned ay_tick( struct ay_t *ay, unsigned cycles )
{
	unsigned c;
	int events[5];

	unsigned last_cycle = cycles;

    for ( c = 0; c < 3; c ++ )
        GET_COUNTER_EVENT( events[c], ay->tone_count[c], ay->tone_period[c] );

    GET_COUNTER_EVENT( events[3], ay->noise_count, ( ( ay->noise_period & 0x1F ) << 1 ) );
    GET_COUNTER_EVENT( events[4], ay->envelope_count, ( ay->envelope_period << 1 ) );

	/*if ( ++ay->tone_count[0] >= ( ay->tone_period[0] & 0xFFF ) )
        { ay->signal[0]^=1; ay->tone_count[0]=0; }
    if ( ++ay->tone_count[1] >= ( ay->tone_period[1] & 0xFFF ) )
        { ay->signal[1]^=1; ay->tone_count[1]=0; }
    if ( ++ay->tone_count[2] >= ( ay->tone_period[2] & 0xFFF ) )
        { ay->signal[2]^=1; ay->tone_count[2]=0; }*/

    for ( c = 0; c < 3; c ++ )
    {
        if ( events[c] == last_cycle )
        {
            ay->tone_count[c] = 0;
            ay->signal[c] ^= 1;
            //int_change = 1;
        }
        else
            ay->tone_count[c] += last_cycle;
    }

    //if ( ++ay->noise_count >= ( ( ay->noise_period & 0x1F ) << 1 ) )
    if ( events[3] == last_cycle )
	{
		unsigned tmp = ( ay->noise_reg >> 15 ) & 1;
		ay->noise_reg <<= 1;
		ay->noise_reg |= ( ( ay->noise_signal ^ ( ay->noise_reg >> 14 ) ) & 1 ) ^ 1;
		ay->noise_signal = tmp;
		ay->noise_count = 0;
	}
	else
        ay->noise_count += last_cycle;

    //if ( ( ++ay->envelope_count ) >= ( ay->envelope_period << 1 ) )
    if ( events[4] == last_cycle )
    {
        ay->envelope_count=0;

        if ( !ay->envelope_phase )
        {
            if ( ay->envelope_state == ( ( ay->envelope_shape & 4 ) ? 0xF : 0x0 ) )
            {
                ay->envelope_phase=1;
                if ( !( ay->envelope_shape & 8 ) )
                    ay->envelope_state=0xF;
                else
                {
                    if ( ay->envelope_shape & 1 )
                    { // hold
                        if ( ay->envelope_shape & 2 )
                            ay->envelope_state ^= 0xF;	// alternate
                    }
                    else
                    {
                        if ( !( ay->envelope_shape & 2 ) )
                            ay->envelope_state ^= 0xF;
                        else
                            ay->envelope_shape ^= 4;
                        ay->envelope_phase = 0;
                    }
                }
            }
            else
            {
                if ( ay->envelope_shape & 4 )
                    ay->envelope_state ++;
                else
                    ay->envelope_state --;
            }
        }
    }
    else
        ay->envelope_count += last_cycle;

	for ( c = 0; c < 3; c ++ )
    {
		unsigned char signl = ( ( ( ay->control & ( 1 << c ) ) ? 1 : ay->signal[c] ) &
                                ( ( ay->control & ( 8 << c ) ) ? 1 : ay->noise_signal ) );
		unsigned char ampl = ( ay->channel_vol[c] & 0x10 ) ? ay->envelope_state : ( ay->channel_vol[c] & 0xF );
        ay->sound[c] = ( signl & 1 ) ? ampl : 0;
    }

    return( last_cycle );
}

static void process_ay( unsigned long tstate )
{
    tstate /= 16;
    while ( tstate > last_tstate )
    {
        if ( standby_cd )
        {
            unsigned ret;
            ret = ay_tick( &ay, tstate - last_tstate );
            last_tstate += ret;
        }
        else
            last_tstate = tstate;

        if ( memcmp( ay.sound, ay.last_sound, sizeof(ay.sound) ) || last_tstate >= zxcpu_tstates_frame/16 )
        {
            add_sound( ay.last_tstate, last_tstate, zxcpu_tstates_frame/16,
                       ( dac_val[ay.last_sound[0]] + dac_val[ay.last_sound[1]]*0.7 + dac_val[ay.last_sound[2]]*0.2 ) * volume_ay,
                       ( dac_val[ay.last_sound[2]] + dac_val[ay.last_sound[1]]*0.7 + dac_val[ay.last_sound[0]]*0.2 ) * volume_ay );
            ay.last_tstate = last_tstate;
            memcpy( ay.last_sound, ay.sound, sizeof(ay.sound) );
        }
    }
}

static void reset()
{
    process_ay( zxcpu_tstates );
    memset( &ay, 0, sizeof( struct ay_t ) );
    standby_cd = 50;
}

static void frame()
{
    process_ay( zxcpu_tstates_frame );
    last_tstate = 0;
    ay.last_tstate = 0;

    if ( standby_cd &&
         ( (ay.control & 0x3F) == 0x2F ||
         ( !ay.channel_vol[0] && !ay.channel_vol[1] && !ay.channel_vol[2] ) ) )
    {
        standby_cd --;
    }
}

static int port_out(Z80EX_CONTEXT *cpu, Z80EX_WORD port, Z80EX_BYTE value)
{
    PORT_TEST(0xFFFD, 0xC002)
    {
        if ( value < 16 )
            ay.reg_latch = value;
    }
    PORT_TEST(0xBFFD, 0xC002)
    {
        process_ay( zxcpu_tstates );

        switch ( ay.reg_latch )
        {
            case 0x0: ay.tone_period[0] = ( ay.tone_period[0] & 0xFF00 ) | ( value << 0 ); break;
            case 0x1: ay.tone_period[0] = ( ay.tone_period[0] & 0x00FF ) | ( value << 8 ); break;
            case 0x2: ay.tone_period[1] = ( ay.tone_period[1] & 0xFF00 ) | ( value << 0 ); break;
            case 0x3: ay.tone_period[1] = ( ay.tone_period[1] & 0x00FF ) | ( value << 8 ); break;
            case 0x4: ay.tone_period[2] = ( ay.tone_period[2] & 0xFF00 ) | ( value << 0 ); break;
            case 0x5: ay.tone_period[2] = ( ay.tone_period[2] & 0x00FF ) | ( value << 8 ); break;

            case 0x6: ay.noise_period = value; break;

            case 0x7: ay.control = value; break;
            case 0x8: ay.channel_vol[0] = value; break;
            case 0x9: ay.channel_vol[1] = value; break;
            case 0xA: ay.channel_vol[2] = value; break;

            case 0xB: ay.envelope_period = ( ay.envelope_period & 0xFF00 ) | ( value << 0 ); break;
            case 0xC: ay.envelope_period = ( ay.envelope_period & 0x00FF ) | ( value << 8 ); break;
            case 0xD: ay.envelope_shape = value;
                    ay.envelope_phase = 0;
                    ay.envelope_state = ( ( ay.envelope_shape & 4 ) ? 0x0 : 0xF );
                    break;
        }
        standby_cd = 50;
    }

    return ( 0 );
}

static int port_in(Z80EX_CONTEXT *cpu, Z80EX_WORD port, Z80EX_BYTE *value)
{
    PORT_TEST(0xFFFD, 0xC002)
    {
        switch ( ay.reg_latch )
        {
            case 0x0: *value = ay.tone_period[0] >> 0; break;
            case 0x1: *value = ay.tone_period[0] >> 8; break;
            case 0x2: *value = ay.tone_period[1] >> 0; break;
            case 0x3: *value = ay.tone_period[1] >> 8; break;
            case 0x4: *value = ay.tone_period[2] >> 0; break;
            case 0x5: *value = ay.tone_period[2] >> 8; break;

            case 0x6: *value = ay.noise_period; break;

            case 0x7: *value = ay.control; break;
            case 0x8: *value = ay.channel_vol[0]; break;
            case 0x9: *value = ay.channel_vol[1]; break;
            case 0xA: *value = ay.channel_vol[2]; break;

            case 0xB: *value = ay.envelope_period >> 0; break;
            case 0xC: *value = ay.envelope_period >> 8; break;
            case 0xD: *value = ay.envelope_shape; break;
        }
    }

    return 0;
}

static void init()
{
	last_tstate = 0;
	ay.last_tstate = 0;
	memset( ay.last_sound, 0, sizeof(ay.last_sound) );
	standby_cd = 50;
}

static void uninit()
{

}

SDevice zxdevice_ay =
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
