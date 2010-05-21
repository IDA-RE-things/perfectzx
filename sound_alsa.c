#include <stdio.h>
#include <alsa/asoundlib.h>

#include "sound.h"


snd_pcm_t *sndh;

void sound_alsa_init()
{
    snd_pcm_hw_params_t *hw_params;
    unsigned rate = 48000;

    snd_pcm_open( &sndh, "plughw:0,0", SND_PCM_STREAM_PLAYBACK, 0 );

    snd_pcm_hw_params_malloc( &hw_params );
    snd_pcm_hw_params_any( sndh, hw_params );

    snd_pcm_hw_params_set_access( sndh, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED );
    snd_pcm_hw_params_set_format( sndh, hw_params, SND_PCM_FORMAT_S16_LE );
    snd_pcm_hw_params_set_rate_near( sndh, hw_params, &rate, 0 );
	snd_pcm_hw_params_set_channels( sndh, hw_params, 1 );

	snd_pcm_hw_params_set_periods( sndh, hw_params, 8, 0 );
	snd_pcm_hw_params_set_buffer_size( sndh, hw_params, (1024 * 8) / 2 );

	snd_pcm_hw_params( sndh, hw_params );

	snd_pcm_hw_params_free( hw_params );

	snd_pcm_prepare( sndh );
}

void sound_alsa_uninit()
{
    snd_pcm_close( sndh );
}

void sound_alsa_flush()
{
    unsigned long towr = bufferFrames;
    signed short *sndb = sound_buffer;

    while ( towr )
        towr -= snd_pcm_writei( sndh, sndb, towr );

    memset( sound_buffer, 0, bufferFrames * sizeof( SNDFRAME ) );
}
