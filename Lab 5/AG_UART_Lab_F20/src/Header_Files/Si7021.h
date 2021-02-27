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
#define 	SI7021_TEMP_NHMM		0xF3
#define     SI7021_ADDRESS			0x40



//***********************************************************************************
// global functions
//***********************************************************************************
void si7021_i2c_open(void);
void si7021_read(uint32_t si7021_read_cb);
float si7021_temp_f(void);


//***********************************************************************************
// Function Prototypes
//***********************************************************************************



#endif /* SRC_HEADER_FILES_SI7021_H_ */
