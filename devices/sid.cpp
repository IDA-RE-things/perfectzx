#include "../main.h"
#include "../devices.h"
#include "../sound.h"
#include <sid.h>

static unsigned long last_tstate;
static cycle_count sid_cycle;
#define SID_FREQ    985248

static short sid_buf[1024];

SID *sid_dev;

extern "C"
{

static void reset()
{
}

static void frame()
{
    unsigned int buf_pos = 0;
    while ( buf_pos < bufferFrames )
    {
        if ( sid_cycle <= 0 )
            sid_cycle = SID_FREQ;
        buf_pos += sid_dev->clock( sid_cycle, (sid_buf + buf_pos), bufferFrames - buf_pos );
    }

    for ( buf_pos = 0; buf_pos < bufferFrames; buf_pos++ )
    {
        sound_buffer[buf_pos].l += sid_buf[buf_pos];
        sound_buffer[buf_pos].r += sid_buf[buf_pos];
    }
    last_tstate = 0;
}

static int port_out(Z80EX_CONTEXT *cpu, Z80EX_WORD port, Z80EX_BYTE value)
{
    PORT_TEST(0x80FF, 0x00FF)
    {
        sid_dev->write( port >> 8, value );
        //printf( "%.2x <- %.2x\n", port >> 8, value );

        return ( 1 );
    }

    return ( 0 );
}

static int port_in(Z80EX_CONTEXT *cpu, Z80EX_WORD port, Z80EX_BYTE *value)
{

    return 0;
}

static void init()
{
	last_tstate = 0;
	sid_cycle = 0;
	printf( "reSID version: %s\n", resid_version_string );

	sid_dev = new SID();
	sid_dev->set_chip_model( MOS6581 );
	//sid_dev->set_sampling_parameters( SID_FREQ, SAMPLE_FAST, 48000 );
	sid_dev->adjust_sampling_frequency( 48000 );
	printf("cycles_per_sample %d\n", sid_dev->cycles_per_sample );
}

static void uninit()
{
    delete sid_dev;
}

}

SDevice zxdevice_sid =
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

