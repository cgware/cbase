#include "driver.h"

#if defined(_MSC_VER)
	#pragma section(".drv$a")
	#pragma section(".drv$z")
__declspec(allocate(".drv$a")) driver_t _drv_start = {0};
__declspec(allocate(".drv$z")) driver_t _drv_end   = {0};
#endif
