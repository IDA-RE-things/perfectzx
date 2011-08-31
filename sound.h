#ifndef PZX_SOUND
#define PZX_SOUND

typedef signed short SNDSAMPLE;

typedef struct
{
    SNDSAMPLE l, r;
}
SNDFRAME;

#define SNDFRAME_LEN    20

extern SNDFRAME *sound_buffer;
extern unsigned long bufferFrames;

#ifdef __cplusplus
extern "C" {
#endif

extern void (*add_sound)( unsigned begin, unsigned end, unsigned measures, signed l, signed r );
void add_sound_fi( unsigned begin, unsigned end, unsigned measures, signed l, signed r );
void add_sound_ff( unsigned begin, unsigned end, unsigned measures, signed l, signed r );
void add_sound_nf( unsigned begin, unsigned end, unsigned measures, signed l, signed r );

void sound_oss_init();
void sound_oss_uninit();
void sound_oss_flush();

void sound_alsa_init();
void sound_alsa_uninit();
void sound_alsa_flush();

void sound_pulse_init();
void sound_pulse_uninit();
void sound_pulse_flush();

#ifdef __cplusplus
}
#endif

#endif // PZX_SOUND
