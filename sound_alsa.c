#include <stdio.h>
#include <alsa/asoundlib.h>

#include "sound.h"

static snd_pcm_t *sndh;

void sound_alsa_init()
{
    int ret;
    snd_pcm_hw_params_t *hw_params;
    unsigned rate = 48000;

    ret = snd_pcm_open( &sndh, "default", SND_PCM_STREAM_PLAYBACK, 0 );
    if ( ret < 0 )
    {
    	fprintf (stderr, "cannot open audio device (%s)\n",
				 snd_strerror (ret));
        return;
    }

    snd_pcm_hw_params_malloc( &hw_params );
    snd_pcm_hw_params_any( sndh, hw_params );

    snd_pcm_hw_params_set_access( sndh, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED );
    snd_pcm_hw_params_set_format( sndh, hw_params, SND_PCM_FORMAT_S16_LE );
    snd_pcm_hw_params_set_rate_near( sndh, hw_params, &rate, 0 );
	snd_pcm_hw_params_set_channels( sndh, hw_params, 2 );

	bufferFrames = SNDFRAME_LEN * rate / 1000;
	sound_buffer = calloc( bufferFrames, sizeof(SNDFRAME) );

	snd_pcm_hw_params_set_periods( sndh, hw_params, 8, 0 );
	snd_pcm_hw_params_set_buffer_size( sndh, hw_params, (1024 * 8) / 2 );

	snd_pcm_hw_params( sndh, hw_params );

	snd_pcm_hw_params_free( hw_params );
	snd_pcm_prepare( sndh );
}

void sound_alsa_uninit()
{
    snd_pcm_close( sndh );
    free( sound_buffer );
}

void sound_alsa_flush()
{
    int res;
    unsigned long towr = bufferFrames;
    SNDFRAME *sndb = sound_buffer;

    while ( towr )
    {
        if ( ( res = snd_pcm_writei( sndh, sndb, towr ) ) < 0 )
        {
            snd_pcm_prepare( sndh );
            continue;
        }
        towr -= res;
        sndb += res*2;
    }

    memset( sound_buffer, 0, bufferFrames * sizeof( SNDFRAME ) );
}
