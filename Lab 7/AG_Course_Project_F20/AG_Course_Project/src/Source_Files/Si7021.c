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

static uint32_t	si7021_read_value;

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
	if(I2C_PERIPHERAL == I2C0){
		si7021_open.scl_pin_route = I2C0_SCL_ROUTE;
		si7021_open.sda_pin_route = I2C0_SDA_ROUTE;
	}
	else if(I2C_PERIPHERAL == I2C1){
		si7021_open.scl_pin_route = I2C1_SCL_ROUTE;
		si7021_open.sda_pin_route = I2C1_SDA_ROUTE;
	}
	si7021_open.scl_pin_en    	= SI7021_SCL_EN;
	//si7021_open.scl_pin_route 	= I2C0_SCL_ROUTE;
	si7021_open.sda_pin_en    	= SI7021_SDA_EN;
	//si7021_open.sda_pin_route 	= I2C0_SDA_ROUTE;
	si7021_open.ack_int_en 		= true;
	si7021_open.nack_int_en 	= true;
	si7021_open.start_int_en 	= true;
	si7021_open.rstart_int_en 	= false;
	si7021_open.mstop_int_en 	= true;
	si7021_open.rxdatav_int_en 	= true;
	si7021_open.sched_cb 		= SI7021_READ_CB;

	i2c_open(I2C_PERIPHERAL, &si7021_open);

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

void si7021_read(uint32_t si7021_read_cb, uint32_t command, uint32_t bytes){
	i2c_start(I2C_PERIPHERAL, SI7021_ADDRESS, command, &si7021_read_value, si7021_read_cb, I2C_READ, bytes, SI7021_U1_MASK);

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
	temp_c = ((175.72 * si7021_read_value) / 65536) - 46.85;
	float temp_f;
	temp_f = ((9 * temp_c) / 5) + 32;
	return temp_f;
}


/***************************************************************************//**
 * @brief
 * 	The test driven development for the si7021 temperature and humidity sensor resolutions
 *
 * @details
 * 	This checks to see the data that has been written from the si7021 based on the resolution of the
 * 	temperature sensor and the humidity sensor. Sets a 80ms wait time to allow the data to be written to the
 * 	rxdatav register and then it checks to see if the resolution has been changed again and waits for the data
 * 	to be written and then read. This function ends with the call to the conversion of the HR percentage to be
 * 	outputted to the register. Each EFM_ASSERT statement checks to see if the data matches the resolution that the
 * 	sensor is in.
 *
 *
 * @note
 *	There is a 80ms HW timer delay to allow the read of the data from the sensor.
 *
 *
 ******************************************************************************/

void si7021_test_driven_dev(void){
	timer_delay(80);
	si7021_read(NO_CALLBACK, SI7021_READ_U1, READ1);
	EFM_ASSERT(i2c_busy() == true);


	while(i2c_busy());

	uint32_t original_value = si7021_read_value;

	uint32_t si7021_res = si7021_read_value & ~(SI7021_U1_MASK);
	EFM_ASSERT(si7021_res == SI7021_12RH_14T);

	uint32_t write_val = ((si7021_read_value & SI7021_U1_MASK) | SI7021_8RH_12T);
	si7021_write(NO_CALLBACK, SI7021_WRITE_U1, write_val);

	while(i2c_busy());
	timer_delay(80);

	si7021_read(NO_CALLBACK, SI7021_READ_U1, READ1);

	while(i2c_busy());

	si7021_res = si7021_read_value & ~(SI7021_U1_MASK);
	EFM_ASSERT(si7021_res == SI7021_8RH_12T);

	write_val = ((si7021_read_value & SI7021_U1_MASK) | SI7021_11RH_11T);
	si7021_write(NO_CALLBACK, SI7021_WRITE_U1, write_val);

	while(i2c_busy());
	timer_delay(80);

	si7021_read(NO_CALLBACK, SI7021_READ_U1, READ1);

	while(i2c_busy());

	si7021_res = si7021_read_value & ~(SI7021_U1_MASK);
	EFM_ASSERT(si7021_res == SI7021_11RH_11T);
	write_val = ((si7021_read_value & SI7021_U1_MASK) | SI7021_10RH_13T);

	si7021_read(NO_CALLBACK, SI7021_TEMP_READ, READ2);
	while(i2c_busy());
	float temperature = si7021_temp_f();
	EFM_ASSERT((temperature < 90) && (temperature > 60));  //Checks to see if the temp read is accurate

	si7021_write(NO_CALLBACK, SI7021_WRITE_U1, write_val);
	while(i2c_busy());
	timer_delay(80);

	si7021_read(NO_CALLBACK, SI7021_READ_U1, READ1);
	while(i2c_busy());
	si7021_res = si7021_read_value & ~(SI7021_U1_MASK);
	EFM_ASSERT(si7021_res == SI7021_10RH_13T);

	si7021_read(NO_CALLBACK, SI7021_RH_READ, READ2);
	while(i2c_busy());

	float relative_humidity = si7021_rh_convert();
	EFM_ASSERT((relative_humidity > 5) && (relative_humidity < 90));

	si7021_write(NO_CALLBACK, SI7021_WRITE_U1, original_value);
	while(i2c_busy());
	timer_delay(80);


}

/***************************************************************************//**
 * @brief
 *	A function that requests the temperature to be written from the SI7021
 *	Temperature sensor.
 *
 * @details
 *	This function initiates a Measure Temperature command in No Hold Master Mode.
 *
 * @note
 * This starts the I2C state machine.
 *
 * @param[in] si7021_write_cb
 * 	 The scheduler event associated with a completed Measure Temperature operation.
 *
 ******************************************************************************/

void si7021_write(uint32_t si7021_write_cb, uint32_t command, uint32_t write_value){
	i2c_start(I2C_PERIPHERAL, SI7021_ADDRESS, command, &si7021_read_value, si7021_write_cb, I2C_WRITE, READ1, write_value);
}

/***************************************************************************//**
 * @brief
 * 	Relative Humidity percentage taken from the si7021 sensor
 *
 * @details
 *	This is the conversion equation for the data that is taken from the sensor and outputted
 *	the percentage value to the terminal.
 *

 ******************************************************************************/

float si7021_rh_convert(void){
	float rh_percentage;
	rh_percentage = ((125 * si7021_read_value) / 65536) - 6;
	return rh_percentage;
}

/***************************************************************************//**
 * @brief
 *	Changes the resolution of the SI7021 Relative Humidity Sensor
 *
 * @details
 *	This function changes the resolution of the humidity sensor and reads the new data
 *	points from the sensor and is called by the applicatioon code to move from the original
 *	resulution to the SI7021_10RH_13T resolution.
 *
 *
 ******************************************************************************/

void si7021_change_res(uint32_t resolution){
	si7021_read(NO_CALLBACK, SI7021_READ_U1, READ1);
	while(i2c_busy());
	uint32_t write_val = (si7021_read_value &~(SI7021_U1_MASK)) | resolution;
	si7021_write(NO_CALLBACK, SI7021_WRITE_U1, write_val);
	while(i2c_busy());
	timer_delay(80);
}
