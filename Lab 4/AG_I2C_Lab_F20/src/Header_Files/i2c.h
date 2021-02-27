//***********************************************************************************
// Include files
//***********************************************************************************

#ifndef SRC_HEADER_FILES_I2C_H_
#define SRC_HEADER_FILES_I2C_H_

#include "em_i2c.h"
#include "em_gpio.h"
#include "em_emu.h"
#include "em_cmu.h"
#include "em_assert.h"
#include "sleep_routines.h"
#include "app.h"


//***********************************************************************************
// defined files
//***********************************************************************************
#define		I2C0_SCL_ROUTE			I2C_ROUTELOC0_SCLLOC_LOC15
#define		SI7021_SCL_EN			true
#define		I2C0_SDA_ROUTE			I2C_ROUTELOC0_SDALOC_LOC15
#define		SI7021_SDA_EN			true
#define		I2C1_SCL_ROUTE			I2C_ROUTELOC0_SCLLOC_LOC19
#define		I2C1_SDA_ROUTE			I2C_ROUTELOC0_SDALOC_LOC19



//***********************************************************************************
// global variables
//***********************************************************************************
typedef struct {
	bool 					enable;  //Enable I2C peripheral when initialization completed.
	bool					master;  //Set to master (true) or slave (false) mode.
	uint32_t				refFreq; //I2C reference clock assumed when configuring bus frequency setup.
	uint32_t				freq;    //(Max) I2C bus frequency to use.
	I2C_ClockHLR_TypeDef 	chlr;    //Clock low/high ratio control.
	bool					scl_pin_en;
	bool					sda_pin_en;
	bool					sensor_enable_pin_en;
	uint32_t				scl_pin_route;
	uint32_t				sda_pin_route;
	uint32_t				sensor_enable_pin_route;
	bool					ack_int_en;
	bool					nack_int_en;
	bool					start_int_en;
	bool					rstart_int_en;
	bool					mstop_int_en;
	bool					rxdatav_int_en;
	uint32_t				sched_cb;


} I2C_OPEN_STRUCT;

typedef enum {
	start_comm,
	send_cmd,
	read_request,
	read_ms_byte,
	read_ls_byte,
	stop_comm
} 	DEFINED_STATES;

typedef struct {
	DEFINED_STATES		state;
	I2C_TypeDef  		*peripheral;
	uint32_t			device_address;
	uint32_t 			command;
	bool 				sm_busy;
	bool				read;
	uint32_t			*read_value;

} I2C_STATE_MACHINE;

//***********************************************************************************
// global functions
//***********************************************************************************

void i2c_open(I2C_TypeDef *i2c_peripheral, I2C_OPEN_STRUCT * i2c_open_peripheral_struct);


//***********************************************************************************
// Function Prototypes
//***********************************************************************************
void I2C0_IRQHandler(void);
void I2C1_IRQHandler(void);
void i2c_start(I2C_TypeDef *i2c_peripheral, uint32_t slave_address, uint32_t command, uint32_t *read_value, uint32_t callback);





#endif /* SRC_HEADER_FILES_I2C_H_ */
