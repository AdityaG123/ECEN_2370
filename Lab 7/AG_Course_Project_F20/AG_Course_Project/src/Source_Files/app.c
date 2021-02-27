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
//#define BLE_TEST_ENABLED

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
*	it uses the cmu clock to show the letimer clock tree while generating and routing  the GPIO pins
*	and the LETIMER function to be able to generate the PWM signal.
*	This sets the period and the active period for which the PWM signal it sent.
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
 *	PWM signal. This function also adds a call to the si7021 i2c function.
 *	Finally, this function also adds a call to the ble_open function for the bluetooth
 *	module as well as to the BOOST_UP_CB call back function.
 *
 * @note
 *	This function only needs to be called only once at the beginning of main function.
 *
 *
 ******************************************************************************/

void app_peripheral_setup(void){
	cmu_open();
	gpio_open();
	scheduler_open();
	sleep_open();
	sleep_block_mode(SYSTEM_BLOCK_EM);
	si7021_i2c_open();
	app_letimer_pwm_open(PWM_PER, PWM_ACT_PER, PWM_ROUTE_0, PWM_ROUTE_1);
	ble_open(BLE_TX_DONE_CB, BLE_RX_DONE_CB);
	add_scheduled_event(BOOT_UP_CB);
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
	Timer.debugRun = false;
	Timer.out_pin_0_en = false;
	Timer.out_pin_1_en = false;
	Timer.out_pin_route0 = out0_route;
	Timer.out_pin_route1 = out1_route;

	Timer.comp0_irq_enable = false;
	Timer.comp1_irq_enable = false;
	Timer.uf_irq_enable = true;
	Timer.comp0_cb = LETIMER0_COMP0_CB;
	Timer.comp1_cb = LETIMER0_COMP1_CB;
	Timer.uf_cb = LETIMER0_UF_CB;




	letimer_pwm_open(LETIMER0, &Timer);

	// letimer_start will inform the LETIMER0 peripheral to begin counting.
//	letimer_start(LETIMER0, true);
}


/*************************************************************//**
* @brief
* Handles the letimer0 underflow event
*
*
* @details
*  This function clears the scheduled event and then handles the underflow event.
*
* @note
*  Every time that we enter the LETIMER0 UF event service routine, we will cycle
* through the energy modes defining what the lowest energy mode that the
* sleep routine will enter. With the change of the temoerature sensor we no longer
* needed the the energy mode to be incremented from EM0 to EM4 as the energy mode required
* for the temperature sensor was set to EM2 in the i2c.c file.
*
*****************************************************************/
void scheduled_letimer0_uf_cb (void){
	EFM_ASSERT(get_scheduled_events() & LETIMER0_UF_CB);
	remove_scheduled_event(LETIMER0_UF_CB);
//	uint32_t em_block = current_block_energy_mode();
//	sleep_unblock_mode(em_block);
//	if(em_block < 4){
//		sleep_block_mode(em_block + 1);
//	}
//	else{
//		sleep_block_mode(EM0);
//	}

	si7021_read(SI7021_READ_CB, SI7021_TEMP_READ, READ2);

}


/*************************************************************//**
* @brief
*  Handles the letimer0 comp0 event
*
* @details
*  This function clears the scheduled event and then handles the comp0 event.
*
*
*****************************************************************/
void scheduled_letimer0_comp0_cb (void){
	remove_scheduled_event(LETIMER0_COMP0_CB);
	//EFM_ASSERT(get_scheduled_events() & LETIMER0_COMP0_CB);
	EFM_ASSERT(false);
}


/*************************************************************//**
* @brief
*  Handles the letimer0 comp1 event
*
*
* @details
* This function clears the scheduled event and then handles the comp1 event.
*
*
*
*****************************************************************/
void scheduled_letimer0_comp1_cb (void){
	remove_scheduled_event(LETIMER0_COMP1_CB);
	//EFM_ASSERT(get_scheduled_events() & LETIMER0_COMP1_CB);
	EFM_ASSERT(false);
}


/***************************************************************************//**
 * @brief
 *	Handles the SI7021 Temperature callback event
 *
 * @details
 *	This function clears the scheduled event and then handles the
 *	Temperature callback event. This also sets the GPIO pins that are connected
 *	to the Si7021 temperature sensor as well as the LED1 port.
 *
 *
 ******************************************************************************/

void scheduled_temp_cb(void){
	EFM_ASSERT(SI7021_READ_CB & get_scheduled_events());
	remove_scheduled_event(SI7021_READ_CB);
	float temperature;
	temperature = si7021_temp_f();
	if(temperature >= 80.0){
		GPIO_PinModeSet(LED1_PORT, LED1_PIN, LED1_GPIOMODE, true);
	}
	else {
		GPIO_PinModeSet(LED1_PORT, LED1_PIN, LED1_GPIOMODE, false);
	}
	char temp_str[10];
	sprintf(temp_str, "Temp = %0.1f F\n", temperature);
	ble_write(temp_str);
	si7021_read(SI7021_RH_CB, SI7021_RH_READ, READ2);

}

/***************************************************************************//**
 * @brief
 *	Handles the BOOT UP event
 *
 * @details
 *	This function clears the scheduled event and then handles the
 *	Boot Up event.
 *
 *
 ******************************************************************************/

void scheduled_boot_up_cb(void){
	EFM_ASSERT(BOOT_UP_CB & get_scheduled_events());
	remove_scheduled_event(BOOT_UP_CB);
	letimer_start(LETIMER0, true);
#ifdef BLE_TEST_ENABLED
	char module_name[80] = "Aditya's BLE";
	bool test = ble_test(module_name);
	EFM_ASSERT(test);
	timer_delay(2000);
#endif
 	circular_buff_test();
 	si7021_test_driven_dev();
	ble_write("\nHello World\n\0");
	ble_write("Course Project\n\0");
	ble_write("Aditya Gopalan\n\0");
	si7021_change_res(SI7021_10RH_13T);
}


/***************************************************************************//**
 * @brief
 *	Handles the TX DONE event
 *
 * @details
 *	This function clears the scheduled event and then handles the
 *	TX Done event.
 *
 *
 ******************************************************************************/

void scheduled_ble_tx_done_cb(void){
	EFM_ASSERT(BLE_TX_DONE_CB & get_scheduled_events());
	remove_scheduled_event(BLE_TX_DONE_CB);
	ble_circ_pop(CIRC_OPER); // if there's other stuff to send, pop it off. otherwise this will return true.
}


/***************************************************************************//**
 * @brief
 *	Handles the SI7021 Relative Humidity Sensor callback event
 *
 *
 * @details
 *	This function clears the scheduled event and then handles the
 *	Relative Humidity callback event. This also sets the GPIO pins that are connected
 *	to the Si7021 Relative Humidity sensor as well as outputting the values to the
 *	bluetooth module to the phone terminal.
 *
 *
 ******************************************************************************/

void scheduled_rh_cb(void){
	EFM_ASSERT(SI7021_RH_CB & get_scheduled_events());
	remove_scheduled_event(SI7021_RH_CB);
	float relative_humidity = si7021_rh_convert();
	char rh_str[19];
	sprintf(rh_str, "RH = %0.1f %% \n", relative_humidity);
	ble_write(rh_str);

}
