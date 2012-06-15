#include <stdio.h>
#include "sound.h"

SNDFRAME *sound_buffer;
unsigned long bufferFrames;

void add_sound_null( unsigned begin, unsigned end, unsigned measures, signed l, signed r )
{
}

void add_sound_fi( unsigned begin, unsigned end, unsigned measures, signed l, signed r )
{
    unsigned cur_fr, next_fr, fr_len, cur_len;

    if ( !l && !r ) // if nothing to do
        return;     // leave, don't waste time on useless calculations

    cur_fr = begin * bufferFrames / measures;

    do
    {
        next_fr = ( cur_fr + 1 ) * measures / bufferFrames;
        fr_len = next_fr - cur_fr * measures / bufferFrames;

        if ( next_fr > end )
            next_fr = end;

        if ( !fr_len )//next_fr == begin )
        {
            cur_fr ++;
            continue;
        }

        cur_len = next_fr - begin;

        if ( l )
            sound_buffer[cur_fr].l += l * cur_len / fr_len;
        if ( r )
            sound_buffer[cur_fr].r += r * cur_len / fr_len;

        begin = next_fr;
        cur_fr ++;
    }
    while ( next_fr < end );
}

void add_sound_ff( unsigned begin, unsigned end, unsigned measures, signed l, signed r )
{
	unsigned st_fr, end_fr;
	double st_off, end_off;
	unsigned i;

	st_fr = ( ( (double)begin / (double)measures ) * bufferFrames );
	end_fr = ( ( (double)end / (double)measures) * bufferFrames );
	if ( st_fr == end_fr )
	{
		st_off = (double)( end - begin ) / (double)measures * bufferFrames;
		sound_buffer[st_fr].l += l * st_off;
		sound_buffer[st_fr].r += r * st_off;
	}
    else
    {
		st_off = 1.0 - ( ( (double)begin / (double)measures ) * bufferFrames - st_fr );
        end_off = ( (double)end / (double)measures ) * bufferFrames - end_fr;

		for ( i = st_fr + 1; i < end_fr; i ++ )
		{
			sound_buffer[i].l += l;
			sound_buffer[i].r += r;
		}

		sound_buffer[st_fr].l += l * st_off;
		sound_buffer[st_fr].r += r * st_off;
		sound_buffer[end_fr].l += l * end_off;
		sound_buffer[end_fr].r += r * end_off;
	}
}

void add_sound_nf( unsigned begin, unsigned end, unsigned measures, signed l, signed r )
{
    unsigned long i;
    unsigned long end_fr = ( end * bufferFrames ) / measures;

    for ( i = ( begin * bufferFrames ) / measures; i < end_fr; i ++ )
    {
        sound_buffer[i].l += l;
        sound_buffer[i].r += r;
    }
}

void add_sound_hp_fi( unsigned begin, unsigned end, unsigned measures, signed l, signed r, sound_state_t *state )
{
    unsigned cur_fr, next_fr, fr_len, cur_len;
    //float ol, or;
    signed long ol, or;
    signed dl, dr;

    dl = l - state->last_l;
    dr = r - state->last_r;

    ol = state->l + dl;
    or = state->r + dr;

    cur_fr = begin * bufferFrames / measures;

    do
    {
        SNDSAMPLE vl, vr;

        next_fr = ( cur_fr + 1 ) * measures / bufferFrames;
        fr_len = next_fr - cur_fr * measures / bufferFrames;

        if ( next_fr > end )
            next_fr = end;

        if ( !fr_len )//next_fr == begin )
        {
            cur_fr ++;
            continue;
        }

        cur_len = next_fr - begin;

        vl = ol * cur_len / fr_len;
        vr = or * cur_len / fr_len;

        if ( vl )
            sound_buffer[cur_fr].l += vl;
        if ( vr )
            sound_buffer[cur_fr].r += vr;

        ol = ol / 1.01;// * cur_len / fr_len;
        or = or / 1.01;// * cur_len / fr_len;

        if ( !ol && !or )
            break;

        begin = next_fr;
        cur_fr ++;
    }
    while ( next_fr < end );

    state->last_l = l;
    state->last_r = r;

    state->l = ol;
    state->r = or;
}

void (*add_sound)( unsigned begin, unsigned end, unsigned measures, signed l, signed r ) = add_sound_fi;
void (*add_sound_hp)( unsigned begin, unsigned end, unsigned measures, signed l, signed r, sound_state_t *state ) = add_sound_hp_fi;
//void (*add_sound)( unsigned begin, unsigned end, unsigned measures, signed l, signed r ) = add_sound_nf;
