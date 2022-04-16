#include "driver/gpio.h"
#include "controller.h"


device_t new_device(uint8_t _deviceId, char * _device_name){
    device_t temp_device;
    temp_device.deviceId = _deviceId;
    temp_device.device_name = _device_name;
}
void device_on(device_t  _device){
    turn_on_device(_device.deviceId);
}
void device_off(device_t _device){
    turn_off_device(_device.deviceId);
}