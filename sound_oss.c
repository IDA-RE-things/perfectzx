#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>
#include <string.h>

#include "sound.h"


int dspf;

void sound_oss_init()
{
    unsigned tmp, rate;

    dspf = open( "/dev/dsp", O_WRONLY );
    if ( dspf == -1 )
        return;

    tmp = AFMT_S16_NE;
    ioctl( dspf, SNDCTL_DSP_SETFMT, &tmp );

    tmp = 2;
    ioctl( dspf, SNDCTL_DSP_CHANNELS, &tmp );

    rate = 48000;
    ioctl( dspf, SNDCTL_DSP_SPEED, &rate );

    // set latency
    tmp = ( 4 << 16 ) | 10;
    ioctl( dspf, SNDCTL_DSP_SETFRAGMENT, &tmp );

    bufferFrames = SNDFRAME_LEN * rate / 1000;
	sound_buffer = calloc( bufferFrames, sizeof(SNDFRAME) );
}

void sound_oss_uninit()
{
    if ( dspf != -1 )
        close( dspf );

    free( sound_buffer );
}

void sound_oss_flush()
{
    int res;
    unsigned long towr = bufferFrames * sizeof( SNDFRAME );
    SNDFRAME *sndb = sound_buffer;

    if ( dspf == -1 )
        return;

    while ( towr )
    {
        res = write( dspf, sndb, towr );
        towr -= res;
        sndb += res;
    }

    memset( sound_buffer, 0, bufferFrames * sizeof( SNDFRAME ) );
}
