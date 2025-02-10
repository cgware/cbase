#ifndef DRIVER_H
#define DRIVER_H

typedef struct driver_s {
	int type;
	void *data;
} driver_t;

#if defined(_MSC_VER)
	#define DRIVER(_name, _type, _data)                                                                                                \
		__pragma(section(".drv$u")) __declspec(allocate(".drv$u")) driver_t _drv_##_name = {.type = _type, .data = _data}

extern driver_t _drv_start;
extern driver_t _drv_end;

	#define DRIVER_START &_drv_start
	#define DRIVER_END   &_drv_end
#else
	#define DRIVER(_name, _type, _data)                                                                                                \
		static driver_t _drv_##_name __attribute__((aligned(4))) __attribute__((unused))                                                  \
		__attribute__((__section__("drv"))) = {.type = _type, .data = _data}

extern driver_t __start_drv[];
extern driver_t __stop_drv[];

	#define DRIVER_START __start_drv
	#define DRIVER_END   __stop_drv
#endif
#endif
