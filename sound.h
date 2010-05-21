#ifndef PZX_SOUND
#define PZX_SOUND

typedef signed short SNDFRAME;

extern SNDFRAME sound_buffer[480 * 2];
extern unsigned long bufferFrames;

//void add_sound_f( unsigned begin, unsigned end, unsigned measures, double val ); // BUG!!! need to find working version
void add_sound_nf( unsigned begin, unsigned end, unsigned measures, double val );

void sound_oss_init();
void sound_oss_uninit();
void sound_oss_flush();

void sound_alsa_init();
void sound_alsa_uninit();
void sound_alsa_flush();

#endif // PZX_SOUND
