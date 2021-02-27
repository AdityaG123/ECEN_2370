/**
 *
 * @file gpio.c
 * @author Aditya Gopalan
 * @date September 10th, 2020
 * @brief Enables all the initializations for the gpio drivers for the gpio peripherals
 *
 */



//***********************************************************************************
// Include files
//***********************************************************************************
#include "gpio.h"


//***********************************************************************************
// defined files
//***********************************************************************************


//***********************************************************************************
// Private variables
//***********************************************************************************


//***********************************************************************************
// Private functions
//***********************************************************************************


//***********************************************************************************
// Global functions
//***********************************************************************************

/*************************************************************//**
* @brief
*	Enables all the initializations for the gpio drivers for the gpio peripherals
*
* @details
*	This function allows the LED0 and LED1 port to be set and setting the drive strength of the pin.
*	WHen the cmu clock is set to the GPIO as true, then the pins are enabled and then the LED pins are configured.
*	This also sets the SCL and SDA pints that are going to used for the Si7021 Temperature Sensor.
*	These pins are also get to allow the TX and RX pins of the UART to be set for the BLE module.
*
* @note
*	Nothing is returned from this as only the pin modes are set as well we the drive strength of each GPIO pin
*
*****************************************************************/


void gpio_open(void){

	CMU_ClockEnable(cmuClock_GPIO, true);

	// Configure LED pins
	GPIO_DriveStrengthSet(LED0_PORT, LED0_DRIVE_STRENGTH);
	GPIO_PinModeSet(LED0_PORT, LED0_PIN, LED0_GPIOMODE, LED0_DEFAULT);

	GPIO_DriveStrengthSet(LED1_PORT, LED1_DRIVE_STRENGTH);
	GPIO_PinModeSet(LED1_PORT, LED1_PIN, LED1_GPIOMODE, LED1_DEFAULT);

	GPIO_DriveStrengthSet(SI7021_SENSOR_EN_PORT, gpioDriveStrengthWeakAlternateWeak);
	GPIO_PinModeSet(SI7021_SENSOR_EN_PORT, SI7021_SENSOR_EN_PIN, gpioModePushPull,  SI7021_ENABLE);

	GPIO_PinModeSet(SI7021_SCL_PORT, SI7021_SCL_PIN, gpioModeWiredAnd, SI7021_I2C_DEFAULT);
	GPIO_PinModeSet(SI7021_SDA_PORT, SI7021_SDA_PIN, gpioModeWiredAnd, SI7021_I2C_DEFAULT);

	GPIO_DriveStrengthSet(LEUART_TX_PORT, gpioDriveStrengthStrongAlternateWeak);
	GPIO_PinModeSet(LEUART_TX_PORT, LEUART_TX_PIN, gpioModePushPull, true);
	GPIO_PinModeSet(LEUART_RX_PORT, LEUART_RX_PIN, gpioModePushPull, true);



}
