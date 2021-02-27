/*
 * Si7021.h
 *
 *  Created on: Oct 1, 2020
 *      Author: Aditya Gopalan
 */

#ifndef SRC_HEADER_FILES_SI7021_H_
#define SRC_HEADER_FILES_SI7021_H_


//***********************************************************************************
// defined files
//***********************************************************************************

#define		SI7021_I2C_FREQ			I2C_FREQ_FAST_MAX            // 400kHz max.
#define		SI7021_I2C_CLK_RATIO	i2cClockHLRStandard          // 4:4 for fast max. try standard if having issues.
#define 	SI7021_I2C				I2C0                         // PG i2c peripheral
#define 	SI7021_REF_FREQ			0                            // HF peripheral clock
#define 	I2C_PERIPHERAL			I2C1



#define 	SI7021_TEMP_READ		0xF3
#define     SI7021_ADDRESS			0x40

#define		SI7021_RH_READ			0xF5
#define		SI7021_READ_U1			0xE7
#define		SI7021_WRITE_U1			0xE6

#define		SI7021_12RH_14T			0x00
#define		SI7021_8RH_12T			0x01
#define		SI7021_10RH_13T			0x80
#define		SI7021_11RH_11T			0x81
#define		SI7021_U1_MASK			0x7E

#define		READ1					1
#define		READ2					2
#define		READ3					3
#define		READ4					4

#define 	NO_CALLBACK				0x0





//***********************************************************************************
// global functions
//***********************************************************************************
void si7021_i2c_open(void);
void si7021_read(uint32_t si7021_read_cb, uint32_t command, uint32_t bytes);
float si7021_temp_f(void);
void si7021_test_driven_dev(void);
void si7021_write(uint32_t si7021_write_cb, uint32_t command, uint32_t write_value);
float si7021_rh_convert(void);

void si7021_change_res(uint32_t resolution);


//***********************************************************************************
// Function Prototypes
//***********************************************************************************



#endif /* SRC_HEADER_FILES_SI7021_H_ */
