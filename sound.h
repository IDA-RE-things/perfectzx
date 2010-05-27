#ifndef PZX_SOUND
#define PZX_SOUND

typedef signed short SNDSAMPLE;

typedef struct
{
    SNDSAMPLE l, r;
}
SNDFRAME;

extern SNDFRAME sound_buffer[480 * 2];
extern unsigned long bufferFrames;

void (*add_sound)( unsigned begin, unsigned end, unsigned measures, double l, double r );
void add_sound_f( unsigned begin, unsigned end, unsigned measures, double l, double r );
void add_sound_nf( unsigned begin, unsigned end, unsigned measures, double l, double r );

void sound_oss_init();
void sound_oss_uninit();
void sound_oss_flush();

void sound_alsa_init();
void sound_alsa_uninit();
void sound_alsa_flush();

#endif // PZX_SOUND
