#include "emul.h"
#include "ula.h"
#include "video.h"
#include "input.h"

#ifndef PZX_DEVICES
#define PZX_DEVICES

#define PORT_DEFINE(port, mask) (port)&(mask), mask
#define PORT_TEST(in, mask) if (((port)&(mask)) == ((in)&(mask)))

typedef struct
{
  Z80EX_WORD port;
  Z80EX_WORD mask;
  int (*func)(Z80EX_CONTEXT *cpu, Z80EX_WORD port, Z80EX_BYTE *value);
  Z80EX_BYTE *value;
} PORT_IN;

typedef struct
{
  Z80EX_WORD port;
  Z80EX_WORD mask;
  int (*func)(Z80EX_CONTEXT *cpu, Z80EX_WORD port, Z80EX_BYTE value);
  Z80EX_BYTE *value;
} PORT_OUT;

typedef struct
{
  const char *name;
  void *addr;
} VAR_LINK;

typedef struct
{
	int (*preport_out)(Z80EX_CONTEXT *cpu, Z80EX_WORD port, Z80EX_BYTE value);
	int (*preport_in)(Z80EX_CONTEXT *cpu, Z80EX_WORD port, Z80EX_BYTE *value);
	int (*port_out)(Z80EX_CONTEXT *cpu, Z80EX_WORD port, Z80EX_BYTE value);
	int (*port_in)(Z80EX_CONTEXT *cpu, Z80EX_WORD port, Z80EX_BYTE *value);

	void (*init)();
	void (*uninit)();
	void (*reset)();
	void (*frame)();
	//void (*tstate)( unsigned long tstate );
	int (*mem_read)( Z80EX_CONTEXT *cpu, Z80EX_WORD addr, Z80EX_BYTE *value, int m1_state );
	int (*mem_write)( Z80EX_CONTEXT *cpu, Z80EX_WORD addr, Z80EX_BYTE value );

	file_fiter_t *files_open;
} SDevice;

typedef struct
{
    unsigned old_pos;
    SDevice *new_device;
} SMachineConfig;

extern unsigned long zxcpu_tstates_frame;
extern SDevice zxdevice_pentagon128;
extern SDevice zxdevice_gmx;
extern SDevice zxdevice_ay;
extern SDevice zxdevice_megaay;
extern SDevice zxdevice_sid;
extern SDevice zxdevice_kempston_mouse;
extern SDevice zxdevice_saa;
extern SDevice zxdevice_tape;

#endif
