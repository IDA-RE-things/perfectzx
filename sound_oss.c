#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>
#include <string.h>

#include "sound_oss.h"


int dspf;
typedef signed short SNDFRAME;
SNDFRAME sound_buffer[480 * 2];
unsigned long bufferFrames = 480 * 2;

void add_sound_f( unsigned begin, unsigned end, unsigned measures, double val )
{
	unsigned st_fr, end_fr;
	double st_off, end_off;
	unsigned i;

	st_fr = ( ( (double)begin / (double)measures ) * bufferFrames );
	end_fr = ( ( (double)end / (double)measures) * bufferFrames );
	if ( st_fr == end_fr )
	{
		st_off = (double)end-begin / (double)measures * bufferFrames;
		sound_buffer[st_fr] += val * st_off;
	}
    else
    {
		st_off = 1.0 - ( ( (double)begin / (double)measures ) * bufferFrames - st_fr );
        end_off = ( (double)end / (double)measures ) * bufferFrames - end_fr;

		for ( i = st_fr + 1; i < end_fr; i ++ )
			sound_buffer[i] += val;

		sound_buffer[st_fr] += val * st_off;
		sound_buffer[end_fr] += val * end_off;
	}
}

void sound_init()
{
    unsigned tmp;

    dspf = open( "/dev/dsp", O_WRONLY );
    if ( dspf == -1 )
        return;

    tmp = AFMT_S16_NE;
    ioctl( dspf, SNDCTL_DSP_SETFMT, &tmp );

    tmp = 1;
    ioctl( dspf, SNDCTL_DSP_CHANNELS, &tmp );

    tmp = 48000;
    ioctl( dspf, SNDCTL_DSP_SPEED, &tmp );

    // set latency
    tmp = ( 4 << 16 ) | 10;
    ioctl( dspf, SNDCTL_DSP_SETFRAGMENT, &tmp );
}

void sound_uninit()
{
    if ( dspf != -1 )
        close( dspf );
}

void sound_flush()
{
    unsigned long towr = bufferFrames * sizeof( SNDFRAME );
    signed short *sndb = sound_buffer;

    if ( dspf == -1 )
        return;

    while ( towr )
        towr -= write( dspf, sndb, towr );
    memset( sound_buffer, 0, bufferFrames * sizeof( SNDFRAME ) );
}
