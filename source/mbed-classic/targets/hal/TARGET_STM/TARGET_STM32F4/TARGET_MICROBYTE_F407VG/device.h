#ifndef DEVICE_H
#define DEVICE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DEVICE_PORTIN           1
#define DEVICE_PORTOUT          1
#define DEVICE_PORTINOUT        1

#define DEVICE_INTERRUPTIN      1

#define DEVICE_ANALOGIN         1
#define DEVICE_ANALOGOUT        1

#define DEVICE_SERIAL           1

#define DEVICE_I2C              1
#define DEVICE_I2CSLAVE         1

#define DEVICE_SPI              1
#define DEVICE_SPISLAVE         1

#define DEVICE_RTC              1

#define DEVICE_PWMOUT           1

#define DEVICE_SLEEP            1

#include "objects.h"

#ifdef __cplusplus
}
#endif

#endif
