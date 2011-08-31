#ifndef PZX_VIDEO
#define PZX_VIDEO

extern unsigned long video_last_tstate;
extern unsigned char *video_memory;
extern unsigned char video_border;

#ifdef __cplusplus
extern "C" {
#endif

void video_set_out(void *buf, unsigned width, unsigned height);

void video_render_std(unsigned long tstate);
void video_render_std_fast();

void video_init();
void video_uninit();

#ifdef __cplusplus
}
#endif

#endif
