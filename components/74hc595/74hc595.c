#include "74hc595.h"
#include "driver/gpio.h"



uint32_t device_status = 0x00;


static void enable_latch(int8_t count);
static void disable_latch(int8_t count);
static void clock_on(int8_t count);
static void clock_off(int8_t count);

/**
  * @brief 	74HC595 GPIO Pins Initialization Function
  * @param 	None
  * @retval None
  */
esp_err_t init_74hc595(void)
{
	gpio_config_t gpio;
	gpio.intr_type = GPIO_INTR_DISABLE;
	gpio.mode =  GPIO_MODE_OUTPUT;
	gpio.pull_up_en = 1;
	gpio.pull_down_en = 0;
	gpio.pin_bit_mask = (1ULL<<IC595_DATA)|(1ULL<<IC595_CLOCK)|(1ULL<<IC595_ENABLE)|(1ULL<<IC595_CLEAR);
	gpio_config(&gpio);
	return ESP_OK;
}

/**
 * @brief	Enable Latch pin with GPIO_PIN_RESET
 * @param 	count: number of calling HAL_GPIO_WritePin function times.
 * @retval 	None
*/
static void enable_latch(int8_t count)
{
	if (count <= 0)
		return;
	while (count-- != 0)
	{
		gpio_set_level(IC595_ENABLE,  0);
	}
}

/**
 * @brief	Disable Latch pin with GPIO_PIN_RESET
 * @param 	count: number of calling HAL_GPIO_WritePin function times.
 * @retval 	None
*/
static void disable_latch(int8_t count)
{
	if (count <= 0)
		return;
	while (count-- != 0)
	{
		gpio_set_level(IC595_ENABLE, 1);
	}
}

/** 
 * @brief	Set the clock pin in HIGH value with GPIO_PIN_RESET
 * @param 	count: number of calling HAL_GPIO_WritePin function times. 
 * @retval 	None
*/
static void clock_on(int8_t count)
{
	if (count <= 0)
		return;
	while (count-- != 0)
	{
		gpio_set_level(IC595_CLOCK, 0);
	}
}

/** 
 * @brief	Set the clock pin in LOW value with GPIO_PIN_SET
 * @param 	count: number of calling HAL_GPIO_WritePin function times. 
 * @retval 	None
*/
static void clock_off(int8_t count)
{
	if (count <= 0)
		return;
	while (count-- != 0)
	{
		gpio_set_level(IC595_CLOCK, 1);
	}
}

/** 
 * @brief	Set the Data_Out pin with state you want
 * @param 	state: is GPIO_PIN_SET or GPIO_PIN_RESET.
 * @retval 	None
*/
static void data_out(int state)
{
#ifdef IS_LOCK_REVERSE_LOGIC_WITH_BJT
	HAL_GPIO_WritePin(LP595_SDI_PORT, LP595_SDI, ~state);
#else
	gpio_set_level(IC595_DATA, state);
#endif
}

/** 
 * @brief	Open the lockpads you want with lockpadStatus
 * @param 	None
 * @retval 	None
*/
void push_data(void)
{
	uint8_t i;
	uint32_t lockpadValue = device_status;
	enable_latch(3);
	for (i = 0; i < NUM_OF_IC595 * 8; i++)
	{
		clock_on(3);
		data_out(lockpadValue & 0x00000001);
		clock_off(3);
		lockpadValue = lockpadValue >> 1;
	}
	disable_latch(3);
}

void turn_on_device(uint8_t deviceid){ //TODO should be combine open and close to 1 function : open wait close
	device_status= 1<<(23-deviceid)|device_status;
	push_data();
}

void turn_off_device(uint8_t deviceid){
	device_status = (~(1<<(23-deviceid)))&device_status;
	push_data();
}
