#include "driver.h"

#if defined(_MSC_VER)
__pragma(section(".drv$a", read)) __declspec(allocate(".drv$a")) DRIVER_ALIGN driver_t _drv_start = {0};
#endif
