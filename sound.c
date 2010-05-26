//#include <stdio.h>
#include "sound.h"

SNDFRAME sound_buffer[480 * 2];
unsigned long bufferFrames = 480 * 2;

void add_sound_null( unsigned begin, unsigned end, unsigned measures, double val )
{
}

void add_sound_f( unsigned begin, unsigned end, unsigned measures, double val )
{
	unsigned st_fr, end_fr;
	double st_off, end_off;
	unsigned i;

	st_fr = ( ( (double)begin / (double)measures ) * bufferFrames );
	end_fr = ( ( (double)end / (double)measures) * bufferFrames );
	if ( st_fr == end_fr )
	{
		st_off = (double)( end - begin ) / (double)measures * bufferFrames;
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

void add_sound_nf( unsigned begin, unsigned end, unsigned measures, double val )
{
    unsigned long i;
    unsigned long end_fr = ( end * bufferFrames ) / measures;

    for ( i = ( begin * bufferFrames ) / measures; i < end_fr; i ++ )
    {
        sound_buffer[i] += val;
    }
}

void (*add_sound)( unsigned begin, unsigned end, unsigned measures, double val ) = add_sound_nf;
