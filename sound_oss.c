#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>
#include <string.h>

#include "sound.h"


int dspf;

void sound_oss_init()
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

void sound_oss_uninit()
{
    if ( dspf != -1 )
        close( dspf );
}

void sound_oss_flush()
{
    unsigned long towr = bufferFrames * sizeof( SNDFRAME );
    signed short *sndb = sound_buffer;

    if ( dspf == -1 )
        return;

    while ( towr )
        towr -= write( dspf, sndb, towr );

    memset( sound_buffer, 0, bufferFrames * sizeof( SNDFRAME ) );
}
