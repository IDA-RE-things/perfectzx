#ifndef PZX_SOUND_OSS
#define PZX_SOUND_OSS

void sound_oss_init();
void sound_oss_uninit();
void sound_oss_flush();

void add_sound_f( unsigned begin, unsigned end, unsigned measures, double val );

#endif // PZX_SOUND_OSS
