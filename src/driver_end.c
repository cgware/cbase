#include "driver.h"

#if defined(_MSC_VER)
__pragma(section(".drv$z", read)) __declspec(allocate(".drv$z")) DRIVER_ALIGN driver_t _drv_end = {0};
#endif
