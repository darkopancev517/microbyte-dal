#ifndef MBED_OBJECTS_H
#define MBED_OBJECTS_H

#include "cmsis.h"
#include "PortNames.h"
#include "PeripheralNames.h"
#include "PinNames.h"

#ifdef __cplusplus
extern "C" {
#endif

struct gpio_irq_s {
    IRQn_Type irq_n;
    uint32_t irq_index;
    uint32_t event;
    PinName pin;
};

struct port_s {
    PortName port;
    uint32_t mask;
    PinDirection direction;
    __IO uint32_t *reg_in;
    __IO uint32_t *reg_out;
};

struct analogin_s {
    ADCName adc;
    PinName pin;
    uint8_t channel;
};

struct dac_s {
    DACName dac;
    uint8_t channel;
};

struct serial_s {
    UARTName uart;
    int index; // Used by irq
    uint32_t baudrate;
    uint32_t databits;
    uint32_t stopbits;
    uint32_t parity;
    PinName pin_tx;
    PinName pin_rx;
};

struct spi_s {
    SPIName spi;
    uint32_t bits;
    uint32_t cpol;
    uint32_t cpha;
    uint32_t mode;
    uint32_t nss;
    uint32_t br_presc;
    PinName pin_miso;
    PinName pin_mosi;
    PinName pin_sclk;
    PinName pin_ssel;
};

struct i2c_s {
    I2CName  i2c;
    uint32_t slave;
};

struct pwmout_s {
    PWMName pwm;
    PinName pin;
    uint32_t period;
    uint32_t pulse;
    uint8_t channel;
    uint8_t inverted;
};

#include "gpio_object.h"

#ifdef __cplusplus
}
#endif

#endif
