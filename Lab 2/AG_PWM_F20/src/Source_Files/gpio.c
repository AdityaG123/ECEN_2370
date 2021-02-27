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

}
