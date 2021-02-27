/**
 *
 * @file i2c.c
 * @author Aditya Gopalan
 * @date September 29th, 2020
 * @brief Contains all the i2c functions
 *
 */



//***********************************************************************************
// Include files
//***********************************************************************************
#include "i2c.h"


//** Silicon Lab include files

#include "i2c.h"
#include "sleep_routines.h"
#include "scheduler.h"

//***********************************************************************************
// defined files
//***********************************************************************************


//***********************************************************************************
// Private variables
//***********************************************************************************
static void i2c_bus_reset(I2C_TypeDef *i2c_peripheral);
static void i2c_start_interrupt(void);
static void i2c_ack_interrupt(void);
static void	i2c_nack_interrupt(void);
static void i2c_mstop_interrupt(void);
static void i2c_rxdatav_interrupt(void);


//***********************************************************************************
// Private functions
//***********************************************************************************
static I2C_STATE_MACHINE i2c_peripheral_state;
static uint32_t scheduled_read_cb;
//***********************************************************************************
// Global functions
//***********************************************************************************



/***************************************************************************//**
 * @brief
 *	Function to open an i2c bus.
 *
 * @details
 *	This routine initializes an i2c bus and sets its state to IDLE. This also sets all the clock
 *	frequencies and initialize the I2C peripheral struct.
 *
 * @note
 *	This function enables the interrupt flags ACK, NACK, RXDATATV and MSTOP.
 *
 * @param[in] i2c_peripheral
 * 	Pointer to the base peripheral address of the I2C peripheral being used. The
 * 	Pearl Gecko has 2 I2C peripherals.
 *
 * @param[in] i2c_open_peripheral_struct
 * 	The struct that this routine will use to configure the I2C bus.
 *
 *
 ******************************************************************************/
void i2c_open(I2C_TypeDef *i2c_peripheral, I2C_OPEN_STRUCT * i2c_open_peripheral_struct){
	if(i2c_peripheral == I2C0){
		CMU_ClockEnable(cmuClock_I2C0, true);
	}
	else if (i2c_peripheral == I2C1){
		CMU_ClockEnable(cmuClock_I2C1, true);
	}

	//confirm successful clock enable & clear IF bit 1 and that we are able to write to the i2c enable
	if((i2c_peripheral->IF & 0x01) == 0) {
		i2c_peripheral->IFS = 0x01;
		EFM_ASSERT(i2c_peripheral->IF & 0x01);
		i2c_peripheral->IFC = 0x01;
	}
	else {
		i2c_peripheral->IFC = 0x01;
		EFM_ASSERT( !(i2c_peripheral->IF & 0x01) );
	}

	// initialize the I2C peripheral struct
	I2C_Init_TypeDef  i2c_initializaion_struct;
	i2c_initializaion_struct.clhr    = i2c_open_peripheral_struct->chlr;
	i2c_initializaion_struct.enable  = i2c_open_peripheral_struct->enable;
	i2c_initializaion_struct.freq    = i2c_open_peripheral_struct->freq;
	i2c_initializaion_struct.master  = i2c_open_peripheral_struct->master;
	i2c_initializaion_struct.refFreq = i2c_open_peripheral_struct->refFreq;

	I2C_Init(i2c_peripheral, &i2c_initializaion_struct);

	i2c_peripheral->ROUTELOC0 |= i2c_open_peripheral_struct->scl_pin_route;
	i2c_peripheral->ROUTELOC0 |= i2c_open_peripheral_struct->sda_pin_route;


	//i2c_peripheral->ROUTELOC0 = ((i2c_open_peripheral_struct->scl_pin_route * I2C_ROUTEPEN_SCLPEN)
	//		| (i2c_open_peripheral_struct->sda_pin_route * I2C_ROUTEPEN_SDAPEN));
	i2c_peripheral->ROUTEPEN = ((i2c_open_peripheral_struct->scl_pin_en * I2C_ROUTEPEN_SCLPEN )
			| (i2c_open_peripheral_struct->sda_pin_en * I2C_ROUTEPEN_SDAPEN ));

	i2c_bus_reset(i2c_peripheral);

	//Enable and Initialize Interrupts
	i2c_peripheral->IFC = i2c_peripheral->IF;
	i2c_peripheral->IEN |= (i2c_open_peripheral_struct->ack_int_en * I2C_IEN_ACK)|
			(i2c_open_peripheral_struct->nack_int_en * I2C_IEN_NACK) |
			(i2c_open_peripheral_struct->start_int_en * I2C_IEN_START) |
			(i2c_open_peripheral_struct->rstart_int_en * I2C_IEN_RSTART) |
			(i2c_open_peripheral_struct->mstop_int_en * I2C_IEN_MSTOP) |
			(i2c_open_peripheral_struct->rxdatav_int_en * I2C_IEN_RXDATAV);

	if(i2c_peripheral == I2C0){
		NVIC_EnableIRQ(I2C0_IRQn);
	}
	else if(i2c_peripheral == I2C1){
		NVIC_EnableIRQ(I2C1_IRQn);
	}


	scheduled_read_cb = i2c_open_peripheral_struct->sched_cb;
}


/***************************************************************************//**
 * @brief
 *	Function to reset the I2C state machines.
 *
 * @details
 *	This function resets the state machines of both the Pearl Gecko and external
 *	I2C devices connected to the bus.
 *
 * @note
 * 	This function resets the peripheral I2C devices by NACKing 9 times by manually
 * 	clocking the SCK pin while leaving SDA in its default asserted state.
 *
 * @param[in] -2c_peripheral
 *	Pointer to the base peripheral address of the I2C peripheral being used. The
 * 	Pearl Gecko has its I2C peripherals.
 *
 *
 ******************************************************************************/

void i2c_bus_reset(I2C_TypeDef *i2c_peripheral){
	uint32_t i2c_state;
	if(i2c_peripheral->STATE & I2C_STATE_BUSY){
		i2c_peripheral->CMD = I2C_CMD_ABORT;
		while(i2c_peripheral->STATE & I2C_STATE_BUSY);
	}
	i2c_state = i2c_peripheral->IEN;
	i2c_peripheral->IEN &= ~i2c_peripheral->IEN;
	i2c_peripheral->IFC |= i2c_state;
	i2c_peripheral->CMD |= I2C_CMD_CLEARTX;
	i2c_peripheral->CMD |= I2C_CMD_START | I2C_CMD_STOP;
	while(!(i2c_peripheral->IF & I2C_IF_MSTOP));
	i2c_peripheral->IFC |= i2c_peripheral->IF;
	i2c_peripheral->IEN |= i2c_state;
	i2c_peripheral->CMD |= I2C_CMD_ABORT;
}

/***************************************************************************//**
 * @brief
 *	IRQ handler for I2C0
 *
 * @details
 *	This is an IRQ handler for I2C0.
 *
 * @note
 *	This is currently configured to handle the interrupts for ACK, NACK, RXDATAV,
 *	and MSTOP. Interrupts are enabled in the i2c_open function.
 *
 *
 *
 ******************************************************************************/

void I2C0_IRQHandler(void){
	uint32_t flag_state = I2C0->IF & I2C0->IEN;
	I2C0->IFC = flag_state;

	if (flag_state & I2C_IF_START) {
		i2c_start_interrupt();
	}
	if (flag_state & I2C_IF_ACK) {
		i2c_ack_interrupt();
	}
	if (flag_state & I2C_IF_NACK) {
		i2c_nack_interrupt();
	}
	if (flag_state & I2C_IF_MSTOP){
		i2c_mstop_interrupt();
	}
	if (flag_state & I2C_IF_RXDATAV){
		i2c_rxdatav_interrupt();
	}

}

/***************************************************************************//**
 * @brief
 *	IRQ handler for I2C1
 *
 * @details
 *	This is an IRQ handler for I2C1.
 *
 * @note
 *	This is currently configured to handle the interrupts for ACK, NACK, RXDATAV,
 *	and MSTOP. Interrupts are enabled in the i2c_open function.
 *
 *
 *
 ******************************************************************************/

void I2C1_IRQHandler(void){
	uint32_t flag_state = I2C1->IF & I2C1->IEN;
	I2C1->IFC = flag_state;

	if (flag_state & I2C_IF_START) {
		i2c_start_interrupt();
	}
	if (flag_state & I2C_IF_ACK) {
		i2c_ack_interrupt();
	}
	if (flag_state & I2C_IF_NACK) {
		i2c_nack_interrupt();
	}
	if (flag_state & I2C_IF_MSTOP){
		i2c_mstop_interrupt();
	}
	if (flag_state & I2C_IF_RXDATAV){
		i2c_rxdatav_interrupt();
	}

}

/***************************************************************************//**
 * @brief
 *	Function to start an I2C read or write operation
 *
 * @details
 *	Initializes the I2C sensor which stores the state of the I2C operation.
 *	All information required by the I2C state machine interacts with this
 *	I2C sensor struct.
 *  Once the temperature sensor is initialized, this function initiates the I2C operation
 *	by entering the first state of the state machine
 *
 *	@note
 *	This function must only be called if the state of the I2C peripheral and
 *	the state of the I2C state machine are both in the idle state.
 *
 *
 * @param[in] i2c_peripheral
 *  Pointer to the base peripheral address of the I2C peripheral being used. The
 * 	Pearl Gecko has 2 I2C peripherals.
 *
 * 	@param[in] slave_address
 * 	 The 32 bit i2c slave device address of the peripheral device.
 *
 * 	@param[in] read_value
 * 	 32 bit parameter indicating the value of the sensor whether to start a read or write command
 *
 * 	@param[in] command
 * 	 The 32 bit command code to write to the peripheral device.
 *
 * 	@param[in] callback
 * 	 The scheduler callback function associated with a completed i2c operation.
 *
 ******************************************************************************/

void i2c_start(I2C_TypeDef *i2c_peripheral, uint32_t slave_address, uint32_t command, uint32_t *read_value, uint32_t callback){
	EFM_ASSERT((i2c_peripheral->STATE & _I2C_STATE_STATE_MASK) == I2C_STATE_STATE_IDLE);
	sleep_block_mode(EM2);
	i2c_peripheral_state.peripheral = i2c_peripheral;
	i2c_peripheral_state.device_address = slave_address;
	//i2c_peripheral_state.sm_busy = false;
	i2c_peripheral_state.read = false;
	i2c_peripheral_state.command = command;
	scheduled_read_cb = callback;
	i2c_peripheral_state.read_value = read_value;
	//i2c_peripheral_state.state = start_comm;

	i2c_peripheral_state.state = start_comm;
	i2c_peripheral->CMD |= I2C_CMD_START;
	i2c_peripheral->TXDATA |= (slave_address << 1 | i2c_peripheral_state.read);


}

/***************************************************************************//**
 * @brief
 *	This function is the beginning of the interrupts for the i2c_start state machine.
 *	It doe not do anything that allows the IRQ Handler to move any interrupts in this part.
 *
 * @details
 * 	If it manages to get into this interrupt, it will break and there are asserts at the
 * 	end when it gets to the stop interrupt section of the function
 *
 *
 *
 ******************************************************************************/


static void i2c_start_interrupt(void){
	switch(i2c_peripheral_state.state){
		case start_comm:
			break;
		case send_cmd:
			break;
		case read_request:
			break;
		case read_ms_byte:
			break;
		case read_ls_byte:
			break;
		case stop_comm:
			EFM_ASSERT(false);
			break;
		default:
			EFM_ASSERT(false);
			break;
	}
}

/***************************************************************************//**
 * @brief
 *	Function that the I2C interrupt handler will call upon receiving
 *	the I2C ACK interrupt
 *
 * @details
 *	This function defines the behavior of the state machine in each state
 *	when an ACK is received.
 *
 *
 ******************************************************************************/


static void i2c_ack_interrupt(void){
	switch(i2c_peripheral_state.state){
		case start_comm:
			i2c_peripheral_state.sm_busy = true;
			i2c_peripheral_state.read = false;
			i2c_peripheral_state.peripheral->CMD = I2C_CMD_CLEARTX;
			i2c_peripheral_state.peripheral->TXDATA |= i2c_peripheral_state.command;
			i2c_peripheral_state.state = send_cmd;
			break;
		case send_cmd:
			i2c_peripheral_state.peripheral->CMD |= I2C_CMD_CLEARTX;
			i2c_peripheral_state.peripheral->CMD |= I2C_CMD_START;
			i2c_peripheral_state.read = true;
			i2c_peripheral_state.peripheral->TXDATA |= (i2c_peripheral_state.device_address << 1 | i2c_peripheral_state.read);
			i2c_peripheral_state.state = read_request;
			break;
		case read_request:
			i2c_peripheral_state.peripheral->CMD |= I2C_CMD_CLEARTX;
			i2c_peripheral_state.state = read_ms_byte;
			break;
		case read_ms_byte:
			break;
		case read_ls_byte:
			break;
		case stop_comm:
			EFM_ASSERT(false);
			break;
		default:
			EFM_ASSERT(false);
			break;
	}

}


/***************************************************************************//**
 * @brief
 *	Function that the I2C interrupt handler will call upon receiving
 *	the I2C NACK interrupt
 *
 * @details
 *	This function defines the behavior of the state machine in each state
 *	when a NACK is received.
 *
 *
 ******************************************************************************/

static void	i2c_nack_interrupt(void){
	switch(i2c_peripheral_state.state){
		case start_comm:
			EFM_ASSERT(false);
			break;
		case send_cmd:
			//EFM_ASSERT(false);
			break;
		case read_request:
			i2c_peripheral_state.peripheral->CMD |= I2C_CMD_CLEARTX;
			i2c_peripheral_state.read = true;
			i2c_peripheral_state.peripheral->CMD |= I2C_CMD_START;
			i2c_peripheral_state.peripheral->TXDATA |= (i2c_peripheral_state.device_address << 1 | i2c_peripheral_state.read);
			i2c_peripheral_state.state = read_request;
			break;
		case read_ms_byte:
			EFM_ASSERT(false);
			break;
		case read_ls_byte:
			EFM_ASSERT(false);
			break;
		case stop_comm:
			EFM_ASSERT(false);
			break;
		default:
			EFM_ASSERT(false);
			break;
	}

}

/***************************************************************************//**
 * @brief
 *	Function that the I2C interrupt handler will call upon receiving
 *	the I2C MSTOP interrupt
 *
 * @details
 *	This function defines the behavior of the state machine in each state
 *	when a STOP condition has been successfully transmitted.
 *
 *
 ******************************************************************************/

static void i2c_mstop_interrupt(void){
	switch(i2c_peripheral_state.state){
		case start_comm:
			EFM_ASSERT(false);
			break;
		case send_cmd:
			EFM_ASSERT(false);
			break;
		case read_request:
			EFM_ASSERT(false);
			break;
		case read_ms_byte:
			EFM_ASSERT(false);
			break;
		case read_ls_byte:
			EFM_ASSERT(false);
			break;
		case stop_comm:
			i2c_peripheral_state.sm_busy = false;
			sleep_unblock_mode(EM2);
			add_scheduled_event(scheduled_read_cb);
			break;
		default:
			EFM_ASSERT(false);
			break;
	}
}

/***************************************************************************//**
 * @brief
 *	Function that the I2C interrupt handler will call upon receiving
 *	the I2C RXDATAV interrupt
 *
 * @details
 *	This function defines the behavior of the state machine in each state
 *	when data becomes available in the receive buffer.
 *
 *
 ******************************************************************************/


static void i2c_rxdatav_interrupt(void){
	switch(i2c_peripheral_state.state){
		case start_comm:
			EFM_ASSERT(false);
			break;
		case send_cmd:
			EFM_ASSERT(false);
			break;
		case read_request:
			EFM_ASSERT(false);
			break;
		case read_ms_byte:
			i2c_peripheral_state.peripheral->CMD |= I2C_CMD_CLEARTX;
			*i2c_peripheral_state.read_value = (i2c_peripheral_state.peripheral->RXDATA << 8);
			i2c_peripheral_state.peripheral->CMD |= I2C_CMD_ACK;
			i2c_peripheral_state.state = read_ls_byte;
			break;
		case read_ls_byte:
			i2c_peripheral_state.peripheral->CMD |= I2C_CMD_CLEARTX;
			*i2c_peripheral_state.read_value |= i2c_peripheral_state.peripheral->RXDATA;
			i2c_peripheral_state.peripheral->CMD |= I2C_CMD_NACK;
			i2c_peripheral_state.peripheral->CMD |= I2C_CMD_STOP;
			i2c_peripheral_state.state = stop_comm;
			break;
		case stop_comm:
			EFM_ASSERT(false);
			break;
		default:
			EFM_ASSERT(false);
			break;
	}
}
