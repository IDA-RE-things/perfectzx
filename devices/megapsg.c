#include "../main.h"
#include "../devices.h"
#include "../sound.h"

static unsigned long last_tstate;

static signed volume_ay = 7500.0;

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
	   	unsigned short tone_period[3];
    	unsigned char noise_period;
		unsigned char control;
		unsigned char channel_vol[3];
		unsigned short envelope_period;
        unsigned char envelope_shape;
	};

	unsigned short tone_count[3];
	unsigned short noise_count;
	unsigned short envelope_count;
	unsigned short envelope_phase;
	unsigned char envelope_state;
	unsigned char signal[3];
	unsigned short noise_reg;
	unsigned char noise_signal;

	unsigned char sound[3];
	unsigned char last_sound[3];
	unsigned long last_tstate;

	unsigned tone_duty[3];
} ay;

static unsigned dir;

static void ay_tick( struct ay_t *ay )
{
	unsigned c;

	if ( ++ay->tone_count[0] >= ( ay->tone_period[0] & 0xFFF ) )
        { ay->signal[0] = (ay->signal[0] + 1) % 16; ay->tone_count[0]=0; }
    if ( ++ay->tone_count[1] >= ( ay->tone_period[1] & 0xFFF ) )
        { ay->signal[1] = (ay->signal[1] + 1) % 16; ay->tone_count[1]=0; }
    if ( ++ay->tone_count[2] >= ( ay->tone_period[2] & 0xFFF ) )
        { ay->signal[2] = (ay->signal[2] + 1) % 16; ay->tone_count[2]=0; }

    if ( ++ay->noise_count >= ( ( ay->noise_period & 0x1F ) << 4 ) )
	{
		unsigned char tmp = ( ay->noise_reg >> 15 ) & 1;
		ay->noise_reg <<= 1;
		ay->noise_reg |= ( ( ay->noise_signal ^ ( ay->noise_reg >> 14 ) ) & 1 ) ^ 1;
		ay->noise_signal = tmp;
		ay->noise_count = 0;
	}

    if ( ( ++ay->envelope_count ) >= ( ay->envelope_period << 4 ) )
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

	for ( c = 0; c < 3; c ++ )
    {
		unsigned char signl = ( ( ( ay->control & ( 1 << c ) ) ? 1 : ((ay->signal[c] >= ay->tone_duty[c]) ? 1 : 0 ) ) &
                                ( ( ay->control & ( 8 << c ) ) ? 1 : ay->noise_signal ) );
		unsigned char ampl = ( ay->channel_vol[c] & 0x10 ) ? ay->envelope_state : ( ay->channel_vol[c] & 0xF );
        ay->sound[c] = ( signl & 1 ) ? ampl : 0;
    }
}

static void process_ay( unsigned long tstate )
{
    while ( tstate > last_tstate )
    {
        ay_tick( &ay );
        last_tstate += 2;
        if ( memcmp( ay.sound, ay.last_sound, sizeof(ay.sound) ) || last_tstate >= zxcpu_tstates_frame )
        {
            add_sound( ay.last_tstate, last_tstate, zxcpu_tstates_frame,
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
    ay.tone_duty[0] = ay.tone_duty[1] = ay.tone_duty[2] = 8;
}

static void frame()
{
    process_ay( zxcpu_tstates_frame );
    last_tstate = 0;
    ay.last_tstate = 0;

    dir = (dir & 0x100) | (dir + 1);
    if ( (dir & 0xFF) == 64 )
    {
        if ( dir & 0x100 )
        {
            if ( ay.tone_duty[0]++ >= 14 )
            {
                dir ^= 0x100;
            }
        }
        else
        {
            if ( ay.tone_duty[0]-- <= 2 )
            {
                dir ^= 0x100;

            }
        }
        dir &= ~0xFF;
        printf( "%d\n", ay.tone_duty[0] );
    }
}

static int port_out(Z80EX_CONTEXT *cpu, Z80EX_WORD port, Z80EX_BYTE value)
{
    PORT_TEST(0xFFFD, 0xC002)
    {
        ay.reg_latch = value;
    }
    PORT_TEST(0xBFFD, 0xC002)
    {
        if ( ay.reg_latch < 16 )
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
        }
    }

    return ( 0 );
}

static int port_in(Z80EX_CONTEXT *cpu, Z80EX_WORD port, Z80EX_BYTE *value)
{
    PORT_TEST(0xFFFD, 0xC002)
    {
        if ( ay.reg_latch < 16 )
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

	reset();
}

static void uninit()
{

}

SDevice zxdevice_megaay =
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
