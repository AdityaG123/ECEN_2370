/**
 *
 * @file Si7021.c
 * @author Aditya Gopalan
 * @date October 1st, 2020
 * @brief Contains all the functions to interface with the SI7021 Temp/Humidity sensor
 *
 */

//***********************************************************************************
// Include files
//***********************************************************************************

//** Silicon Lab include files
#include "em_i2c.h"
#include "em_cmu.h"
#include "em_assert.h"

//** User/developer include files
#include "Si7021.h"
#include "gpio.h"
#include "i2c.h"


//***********************************************************************************
// defined files
//***********************************************************************************


//***********************************************************************************
// Private variables
//***********************************************************************************

static uint32_t	si7021_temp_read;

//***********************************************************************************
// Private functions
//***********************************************************************************


//***********************************************************************************
// Global functions
//***********************************************************************************



/***************************************************************************//**
 * @brief
 * 	A function to open an I2C port for the SI7021 Temperature and Humidity Sensor
 * 	onboard the Pearl Gecko Starter Kit.
 *
 *
 * @details
 * 	This function creates an I2C State machine Struct specifying the configuration of the SDA
 * 	and SCL pins that connect the SI7021 to the EFM32PG12 on the Pearl Gecko
 * 	Starter Kit board, and an I2C Open Struct specifying the I2C configuration
 * 	necessary to interact with the SI7021 sensor. Then it opens the I2C bus using
 * 	this information.
 *
 *
 ******************************************************************************/
void si7021_i2c_open(){
	I2C_OPEN_STRUCT si7021_open;
	si7021_open.chlr    = SI7021_I2C_CLK_RATIO;
	si7021_open.enable  = SI7021_ENABLE;
	si7021_open.freq    = SI7021_I2C_FREQ;
	si7021_open.master  = true;
	si7021_open.refFreq = SI7021_REF_FREQ;

	si7021_open.scl_pin_en    = SI7021_SCL_EN;
	si7021_open.scl_pin_route = I2C0_SCL_ROUTE;
	si7021_open.sda_pin_en    = SI7021_SDA_EN;
	si7021_open.sda_pin_route = I2C0_SDA_ROUTE;
	si7021_open.ack_int_en = true;
	si7021_open.nack_int_en = true;
	si7021_open.start_int_en = true;
	si7021_open.rstart_int_en = false;
	si7021_open.mstop_int_en = true;
	si7021_open.rxdatav_int_en = true;
	si7021_open.sched_cb = SI7021_READ_CB;

	i2c_open(I2C0, &si7021_open);

}

/***************************************************************************//**
 * @brief
 *	A function that requests the temperature to be read from the SI7021
 *	Temperature sensor.
 *
 * @details
 *	This function initiates a Measure Temperature command in No Hold Master Mode.
 *
 * @note
 * This starts the I2C state machine.
 *
 * @param[in] si7021_read_cb
 * 	 The scheduler event associated with a completed Measure Temperature operation.
 *
 ******************************************************************************/

void si7021_read(uint32_t si7021_read_cb){
	i2c_start(I2C0, SI7021_ADDRESS, SI7021_TEMP_NHMM, &si7021_temp_read, si7021_read_cb);

}

/***************************************************************************//**
 * @brief
 *	A function which returns the most recently read temperature measurement.
 *
 * @details
 *	This function reads the raw temperature data received from the SI7021
 *	Temperature and Humidity sensor, converts it into Celsius per the
 *	data sheet specifications, and then converts it into Fahrenheit to return.
 *
 * @note
 *	This should only be called upon the completion of the SI7021_READ_CB
 *
 *
 * @return
 * 	the last received temperature, as a float, in degrees Fahrenheit.
 *
 ******************************************************************************/

float si7021_temp_f(void){
	float temp_c;
	temp_c = ((175.72 * si7021_temp_read) / 65536) - 46.85;
	float temp_f;
	temp_f = ((9 * temp_c) / 5) + 32;
	return temp_f;
}
