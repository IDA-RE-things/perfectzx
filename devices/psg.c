#include "../main.h"
#include "../devices.h"
#include "../sound.h"

static unsigned long last_tstate;

double volume_ay = 2500.0;

const double dac_val[16]=
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
	union
	{
    	Z80EX_BYTE regs[16];
		struct
		{
	    	unsigned short tone_period[3];
    		unsigned char noise_period;
			unsigned char control;
			unsigned char channel_vol[3];
			unsigned short envelope_period;
			 unsigned char envelope_shape;
		};
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
} ay;

static void ay_tick( struct ay_t *ay )
{
	unsigned c;

	if ( ++ay->tone_count[0] >= ( ay->tone_period[0] & 0xFFF ) )
        { ay->signal[0]^=1; ay->tone_count[0]=0; }
    if ( ++ay->tone_count[1] >= ( ay->tone_period[1] & 0xFFF ) )
        { ay->signal[1]^=1; ay->tone_count[1]=0; }
    if ( ++ay->tone_count[2] >= ( ay->tone_period[2] & 0xFFF ) )
        { ay->signal[2]^=1; ay->tone_count[2]=0; }
    if ( ++ay->noise_count >= ( ( ay->noise_period & 0x1F ) << 1 ) )
	{
		unsigned char tmp = ( ay->noise_reg >> 15 ) & 1;
		ay->noise_reg <<= 1;
		ay->noise_reg |= ( ( ay->noise_signal ^ ( ay->noise_reg >> 14 ) ) & 1 ) ^ 1;
		ay->noise_signal = tmp;
		ay->noise_count = 0;
	}

    if ( ( ++ay->envelope_count ) >= ( ay->envelope_period << 1 ) )
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
		unsigned char signl = ( ( ( ay->control & ( 1 << c ) ) ? 1 : ay->signal[c] ) &
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
        add_sound_nf( last_tstate, last_tstate + 64, zxcpu_tstates_frame, dac_val[ay.sound[0] + ay.sound[1] + ay.sound[2]] * volume_ay );
        last_tstate += 64;
    }
}

static void reset(){

}

static void frame()
{
    // update rest of the screen
    process_ay( zxcpu_tstates );
    last_tstate = 0;
    //video_render_std_fast();
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

            ay.regs[ay.reg_latch] = value;
            if ( ay.reg_latch >= 11 && ay.reg_latch <= 13)
            {
                ay.envelope_phase = 0;
                ay.envelope_state = ( ( ay.envelope_shape & 4 ) ? 0x0 : 0xF );
            }
        }
    }

    return ( 0 );
}

static int port_in(Z80EX_CONTEXT *cpu, Z80EX_WORD port, Z80EX_BYTE *value)
{
    /*PORT_TEST(0xFE, 0x01)
    {
        unsigned i;
        Z80EX_BYTE val = 0xFF;
        for ( i = 0; i < 8; i ++ )
            if ( (~port) & ( 0x0100 << i ) )
                val &= zxkey_matrix[i];
        *value = val;
    }*/

    return 0;
}

static void init()
{
//    int res;
	last_tstate = 0;
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
		NULL,
		NULL, NULL,
		NULL
	};
