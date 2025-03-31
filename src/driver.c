#include "driver.h"

#if defined(_MSC_VER)
__pragma(section(".drv$a", read)) __declspec(allocate(".drv$a")) driver_t _drv_start = {0};
__pragma(section(".drv$z", read)) __declspec(allocate(".drv$z")) driver_t _drv_end   = {0};
#endif
