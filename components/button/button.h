#ifndef __BUTTON_H__
#define __BUTTON_H__

#include "driver/gpio.h"

typedef struct button{
    gpio_num_t io,
    char * button_name
}button;

typedef enum button_status{
    ON,
    OFF
}button_status;

button button_init(gpio_num_t io, char * button_name);
button_status read_button(button _button);



#endif