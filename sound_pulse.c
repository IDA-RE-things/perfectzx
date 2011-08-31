#include <stdio.h>
#include <malloc.h>
#include <string.h>

#include <glib.h>
#include <pulse/simple.h>
#include <pulse/error.h>
#include <pulse/gccmacro.h>

#include "sound.h"

static pa_simple *s;

void sound_pulse_init()
{
    pa_sample_spec ss;
    pa_buffer_attr ba;

    ss.format = PA_SAMPLE_S16NE;
    ss.channels = 2;
    ss.rate = 48000;

    g_setenv("PULSE_PROP_media.role", "game", TRUE);

    ba.minreq = -1;
    ba.maxlength = -1;
    ba.prebuf = -1;
    ba.tlength = (1024*8)/2;

    s = pa_simple_new(NULL,               // Use the default server.
                   "PerfectZX",           // Our application's name.
                   PA_STREAM_PLAYBACK,
                   NULL,               // Use the default device.
                   "Sound",            // Description of our stream.
                   &ss,                // Our sample format.
                   NULL,               // Use default channel map
                   &ba,               // Use custom buffering attributes.
                   NULL               // Ignore error code.
                   );

	bufferFrames = SNDFRAME_LEN * ss.rate / 1000;
	sound_buffer = calloc( bufferFrames, sizeof(SNDFRAME) );
}

void sound_pulse_uninit()
{
    pa_simple_free( s );
    free( sound_buffer );
}

void sound_pulse_flush()
{
    int res;

    res = pa_simple_write(s, sound_buffer, bufferFrames * sizeof( SNDFRAME ), NULL );

    memset( sound_buffer, 0, bufferFrames * sizeof( SNDFRAME ) );
}
