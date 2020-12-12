#ifndef MICROBYTE_PIN_H
#define MICROBYTE_PIN_H

#include "mbed.h"
#include "MicroByteDriversConfig.h"
#include "MicroByteComponent.h"

#define IO_STATUS_DIGITAL_IN                0x01
#define IO_STATUS_DIGITAL_OUT               0x02
#define IO_STATUS_ANALOG_IN                 0x04
#define IO_STATUS_ANALOG_OUT                0x08
#define IO_STATUS_TOUCH_IN                  0x10
#define IO_STATUS_EVENT_ON_EDGE             0x20
#define IO_STATUS_EVENT_PULSE_ON_EDGE       0x40

// TODO:
// #define MICROBYTE_PIN_P0
// #define MICROBYTE_PIN_P1

#define MICROBYTE_PIN_MAX_OUTPUT            1023

#define MICROBYTE_PIN_MAX_SERVO_RANGE       180
#define MICROBYTE_PIN_DEFAULT_SERVO_RANGE   2000
#define MICROBYTE_PIN_DEFAULT_SERVO_CENTER  1500

#define MICROBYTE_PIN_EVENT_NONE            0
#define MICROBYTE_PIN_EVENT_ON_EDGE         1
#define MICROBYTE_PIN_EVENT_ON_PULSE        2
#define MICROBYTE_PIN_EVENT_ON_TOUCH        3

#define MICROBYTE_PIN_EVT_RISE              2
#define MICROBYTE_PIN_EVT_FALL              3
#define MICROBYTE_PIN_EVT_PULSE_HI          4
#define MICROBYTE_PIN_EVT_PULSE_LO          5

enum PinCapability
{
    PIN_CAPABILITY_DIGITAL_IN = 0x01,
    PIN_CAPABILITY_DIGITAL_OUT = 0x02,
    PIN_CAPABILITY_DIGITAL = PIN_CAPABILITY_DIGITAL_IN | PIN_CAPABILITY_DIGITAL_OUT,
    PIN_CAPABILITY_ANALOG_IN = 0x04,
    PIN_CAPABILITY_ANALOG_OUT = 0x08,
    PIN_CAPABILITY_ANALOG = PIN_CAPABILITY_ANALOG_IN | PIN_CAPABILITY_ANALOG_OUT,
    PIN_CAPABILITY_STANDARD = PIN_CAPABILITY_DIGITAL | PIN_CAPABILITY_ANALOG_OUT,
    PIN_CAPABILITY_ALL = PIN_CAPABILITY_DIGITAL | PIN_CAPABILITY_ANALOG
};

class MicroBytePin : public MicroByteComponent
{
    void *pin;
    PinCapability capability;
    uint8_t pullMode;

    void disconnect();

    int obtainAnalogChannel();

    void onRise();

    void onFall();

    void pulseWidthEvent(int eventValue);

    int enableRiseFallEvent(int eventType);

    int disableEvents();

    public:

    PinName name;

    MicroBytePin(int id, PinName name, PinCapability capability);

    int setDigitalValue(int value);

    int getDigitalValue();

    int getDigitalValue(PinMode pull);

    int setAnalogValue(int value);

    int setServoValue(int value, int range = MICROBYTE_PIN_DEFAULT_SERVO_RANGE, int center = MICROBYTE_PIN_DEFAULT_SERVO_CENTER);

    int getAnalogValue();

    int isInput();

    int isOutput();

    int isDigital();

    int isAnalog();

    int isTouched();

    int setServoPulseUs(int pulseWidth);

    int setAnalogPeriod(int period);

    int setAnalogPeriodUs(int period);

    int getAnalogPeriodUs();

    int getAnalogPeriod();

    int setPull(PinMode pull);

    int eventOn(int eventType);
};

#endif
