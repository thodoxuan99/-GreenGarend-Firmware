#ifndef INC_APP_74HC595_H_
#define INC_APP_74HC595_H_

#include "esp_err.h"


#define IC595_DATA       0
#define IC595_CLOCK      1
#define IC595_ENABLE     2
#define IC595_CLEAR      3
#define NUM_OF_IC595 			20		/*!< Number of lockpads on the board. */

esp_err_t init_74hc595(void);

void turn_on_device(uint8_t device);

void turn_off_device(uint8_t device);

#endif
