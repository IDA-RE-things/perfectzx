#include <stdio.h>
#include <sys/asoundlib.h>
#include <errno.h>

#include "sound.h"

static snd_pcm_t *sndh;
#include <gulliver.h>
void sound_qsa_init()
{
    int ret;
    snd_pcm_channel_params_t ch_params;
    unsigned rate = 48000;

    ret = snd_pcm_open_preferred( &sndh, NULL, NULL, SND_PCM_OPEN_PLAYBACK );
    if ( ret < 0 )
    {
    	fprintf (stderr, "cannot open audio device (%s)\n",
				 snd_strerror (ret));
        return;
    }

    /*if ((ret = snd_pcm_plugin_set_disable ( sndh, PLUGIN_DISABLE_BUFFER_PARTIAL_BLOCKS)) < 0)
	{
    	fprintf (stderr, "snd_pcm_plugin_set_disable failed: %s\n", snd_strerror (ret));
    	return -1;
	}*/

    memset( &ch_params, 0, sizeof(ch_params) );

    ch_params.mode = SND_PCM_MODE_BLOCK;
    ch_params.channel = SND_PCM_CHANNEL_PLAYBACK;
    ch_params.start_mode = SND_PCM_START_FULL;
    ch_params.stop_mode = SND_PCM_STOP_STOP;

    bufferFrames = SNDFRAME_LEN * rate / 1000;

    ch_params.buf.block.frag_size = bufferFrames * sizeof(SNDFRAME);
    ch_params.buf.block.frags_max = 8;
    ch_params.buf.block.frags_min = 1;

    ch_params.format.interleave = 1;
    ch_params.format.rate = rate;
    ch_params.format.voices = 2;

#ifdef __LITTLEENDIAN__
    ch_params.format.format = SND_PCM_SFMT_S16_LE;
#else
    ch_params.format.format = SND_PCM_SFMT_S16_BE;
#endif

    /*snd_pcm_hw_params_malloc( &hw_params );
    snd_pcm_hw_params_any( sndh, hw_params );

    snd_pcm_hw_params_set_access( sndh, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED );
    snd_pcm_hw_params_set_format( sndh, hw_params, SND_PCM_FORMAT_S16_LE );
    snd_pcm_hw_params_set_rate_near( sndh, hw_params, &rate, 0 );
	snd_pcm_hw_params_set_channels( sndh, hw_params, 2 );*/
	sound_buffer = calloc( bufferFrames, sizeof(SNDFRAME) );

	snd_pcm_plugin_params( sndh, &ch_params );
	snd_pcm_plugin_prepare( sndh, SND_PCM_CHANNEL_PLAYBACK );

	/*snd_pcm_hw_params( sndh, hw_params );

	snd_pcm_hw_params_free( hw_params );
	snd_pcm_prepare( sndh );*/
}

void sound_qsa_uninit()
{
    snd_pcm_close( sndh );
    free( sound_buffer );
}

void sound_qsa_flush()
{
    int res;
    unsigned long towr = bufferFrames * sizeof(SNDFRAME);
    SNDFRAME *sndb = sound_buffer;

    while ( towr )
    {
        if ( ( res = snd_pcm_plugin_write( sndh, sndb, towr ) ) < towr )
        {
        	snd_pcm_plugin_prepare( sndh, SND_PCM_CHANNEL_PLAYBACK );
            continue;
        }
        towr -= res;
        sndb += res;
    }

    memset( sound_buffer, 0, bufferFrames * sizeof( SNDFRAME ) );
}
