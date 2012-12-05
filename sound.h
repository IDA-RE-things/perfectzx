#ifndef PZX_SOUND
#define PZX_SOUND

#ifdef __cplusplus
extern "C" {
#endif

typedef signed short SNDSAMPLE;

typedef struct
{
    SNDSAMPLE l, r;
} SNDFRAME;

#define SNDFRAME_LEN    20

typedef struct
{
    SNDSAMPLE l, r;
    signed last_l, last_r;
    unsigned measures;
    unsigned last_tstate;
} sound_state_t;

extern SNDFRAME *sound_buffer;
extern unsigned long bufferFrames;

extern void (*add_sound)( unsigned begin, unsigned end, unsigned measures, signed l, signed r );
extern void (*add_sound_hp)( unsigned begin, unsigned end, unsigned measures, signed l, signed r, sound_state_t *state );

void add_sound_fi( unsigned begin, unsigned end, unsigned measures, signed l, signed r );
void add_sound_ff( unsigned begin, unsigned end, unsigned measures, signed l, signed r );
void add_sound_nf( unsigned begin, unsigned end, unsigned measures, signed l, signed r );

void add_sound_hp_fi( unsigned begin, unsigned end, unsigned measures, signed l, signed r, sound_state_t *state );

void add_sound_hp_fi_new( sound_state_t *state, unsigned tstate, signed l, signed r );

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
