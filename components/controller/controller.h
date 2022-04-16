#ifndef __CONTROLLER__
#define __CONTROLLER__

#include "driver/gpio.h"
#include "74hc595/74hc595.h"
#include "string.h"


#define FAN        0
#define FEEDER     1
#define LED        2
#define MOTOR      3
#define VALVE      4


typedef struct device{
    uint8_t deviceId,
    const char *device_name
}device_t;

device_t new_device(uint8_t _deviceId, char * _device_name);
void device_on(device_t  _device);
void device_off(device_t _device);



#endif