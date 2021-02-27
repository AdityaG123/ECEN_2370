/**
 *
 * @file app.c
 * @Aditya Gopalan
 * @September 10th, 2020
 * @brief Contains functions to open and initialize peripherals
 *
 */

//***********************************************************************************
// Include files
//***********************************************************************************
#include "app.h"


//***********************************************************************************
// defined files
//***********************************************************************************


//***********************************************************************************
// Static / Private Variables
//***********************************************************************************


//***********************************************************************************
// Private functions
//***********************************************************************************

/*************************************************************//**
* @brief
* 	Contains functions to open and initialize peripherals
*
* @details
*	This part of the code set up the peripheral of the letimer pwm function while
*	it uses the cmu clock toshow the letimer clock tree while generating and routing  the GPIO pins
*	and the LETIMER function to be able to generate the PWM signal.
*	This sets the period and the activer period for which the PWM signal it sent.
*
* @note
*
*
*****************************************************************/


static void app_letimer_pwm_open(float period, float act_period, uint32_t out0_route, uint32_t out1_route);

//***********************************************************************************
// Global functions
//***********************************************************************************

/***************************************************************************//**
 * @brief
 * 	This function would be used to initialize all peripherals for this application.
 *
 *
 * @details
 *	This part of the function uses the CMU clock to enable the LETIMER clock tree,
 *	and initializes the GPIO and LETIMER needed to produce a
 *	PWM signal.
 *
 * @note
 *	This function only needs to be called only once at the beginning of main function.
 *
 *
 ******************************************************************************/

void app_peripheral_setup(void){
	cmu_open();
	gpio_open();
	app_letimer_pwm_open(PWM_PER, PWM_ACT_PER, PWM_ROUTE_0, PWM_ROUTE_1);
	letimer_start(LETIMER0, true);
}

/***************************************************************************//**
 * @brief
 *  Function to open a PWM signal out of LETIMER0 at a particular frequency
 *  duty cycle. This includes values/settings for our specific application.
 *
 * @details
 * 	This routine defines the values of the APP_LETIMER_PWM_TypeDef struct
 *	specific to our application then passes it to the lower level driver
 *	letimer_pwm_open function along with which LETIMER we want to use
 *	(there is only one LETIMER on the Pearl Gecko, so it will always be 0).
 *
 *
 * @note
 *  This function is used to setup PWM for this app.
 *
 *
 *
 * @param[in] period
 * PWM period in seconds
 *
 *
 * @param[in] act_period
 * PWM active period in seconds
 *
 ******************************************************************************/
void app_letimer_pwm_open(float period, float act_period, uint32_t out0_route, uint32_t out1_route){
	// Initializing LETIMER0 for PWM operation by creating the
	// letimer_pwm_struct and initializing all of its elements
	APP_LETIMER_PWM_TypeDef Timer;
	Timer.active_period = act_period;
	Timer.period = period;
	Timer.enable = false;
	Timer.debugRun = true;
	Timer.out_pin_0_en = true;
	Timer.out_pin_1_en = true;
	Timer.out_pin_route0 = out0_route;
	Timer.out_pin_route1 = out1_route;




	letimer_pwm_open(LETIMER0, &Timer);

	// letimer_start will inform the LETIMER0 peripheral to begin counting.
//	letimer_start(LETIMER0, true);
}


