/**
 * @file leuart.c
 * @author Keith Graham and Aditya Gopalan
 * @date   October 29th, 2020
 * @brief Contains all the functions of the LEUART peripheral
 *
 */

//***********************************************************************************
// Include files
//***********************************************************************************

//** Standard Library includes
#include <string.h>

//** Silicon Labs include files
#include "em_gpio.h"
#include "em_cmu.h"

//** Developer/user include files
#include "leuart.h"
#include "scheduler.h"

//***********************************************************************************
// defined files
//***********************************************************************************


//***********************************************************************************
// private variables
//***********************************************************************************
uint32_t	rx_done_evt;
uint32_t	tx_done_evt;
bool		leuart0_tx_busy;

typedef struct{
	LEUART_TypeDef	*leuart;
	bool			sm_busy;
	uint32_t		str_length;
	LEUART_STATES	state;
	char			output[80];
	uint32_t		count;
} LEUART_SM;

static LEUART_SM	sm;

static void leuart0_txbl_interrupt(void);
static void leuart0_txc_interrupt(void);

/***************************************************************************//**
 * @brief LEUART driver
 * @details
 *  This module contains all the functions to support the driver's state
 *  machine to transmit a string of data across the LEUART bus.  There are
 *  additional functions to support the Test Driven Development test that
 *  is used to validate the basic set up of the LEUART peripheral.  The
 *  TDD test for this class assumes that the LEUART is connected to the HM-18
 *  BLE module.  These TDD support functions could be used for any TDD test
 *  to validate the correct setup of the LEUART.
 *
 ******************************************************************************/

//***********************************************************************************
// Private functions
//***********************************************************************************



//***********************************************************************************
// Global functions
//***********************************************************************************

/***************************************************************************//**
 * @brief
 *   Function to open Low Energy UART peripheral communication port.
 *
 * @details
 *   This routine initializes a LEUART port, routes the RX and TX pins, and
 *   enables / clears RX and TX and their respective buffers as needed.
 *
 * @param[in] leuart
 *   Pointer to the base peripheral address of the LEUART peripheral being used.
 *
 * @param[in] leuart_settings
 *   Pointer to the LEUART_OPEN_STRUCT containing the desired settings for this
 *   instance of the LEUART port.
 *
 * ******************************************************************************/

void leuart_open(LEUART_TypeDef *leuart, LEUART_OPEN_STRUCT *leuart_settings){
	CMU_ClockEnable(cmuClock_LEUART0, true);
	if((leuart->STARTFRAME & 0xFF) == 0) {
		leuart->STARTFRAME |= 0x01;
		EFM_ASSERT(leuart->STARTFRAME & 0x01);
		leuart->STARTFRAME &= ~leuart->STARTFRAME;
	}
	else {
		leuart->STARTFRAME &= ~leuart->STARTFRAME;
		EFM_ASSERT(!(leuart->IFC & 0xFF));
	}

	LEUART_Init_TypeDef leuart_init;
	leuart_init.baudrate = leuart_settings->baudrate;
	leuart_init.databits = leuart_settings->databits;
	leuart_init.parity   = leuart_settings->parity;
	leuart_init.stopbits = leuart_settings->stopbits;
	leuart_init.refFreq  = 0;

	LEUART_Init(leuart, &leuart_init);

	leuart->ROUTEPEN |= (leuart_settings->rx_pin_en * LEUART_ROUTEPEN_RXPEN) |
			(leuart_settings->tx_pin_en * LEUART_ROUTEPEN_TXPEN);
	leuart->ROUTELOC0 = (leuart_settings->rx_loc | (leuart_settings->tx_loc << _LEUART_ROUTELOC0_TXLOC_SHIFT));

	leuart->CMD = LEUART_CMD_CLEARTX;
	leuart->CMD = LEUART_CMD_CLEARRX;

	LEUART_Enable(leuart, leuart_settings->enable);

	while(!(leuart->STATUS & (LEUART_STATUS_TXENS | LEUART_STATUS_RXENS)));
	EFM_ASSERT(leuart->STATUS & LEUART_STATUS_TXENS);
	EFM_ASSERT(leuart->STATUS & LEUART_STATUS_RXENS);

	leuart->IFC = leuart->IF;
	NVIC_EnableIRQ(LEUART0_IRQn);

	rx_done_evt = leuart_settings->rx_done_evt;
	tx_done_evt = leuart_settings->tx_done_evt;
}

/***************************************************************************//**
 * @brief
 *   IRQ handler for LEUART0.
 *
 * @details
 * 	 This is an IRQ handler for LEUART0. It is used to determine when the LEUART0
 * 	 is available to transmit and when the LEUART0 has completed transmission.
 * 	 It uses the TXBL and TXC interrupts.
 *
 * @note
 *   The BLE test uses polling, not interrupts to function. Normal BLE functionality
 *   will use interrupts in order to maintain low energy operation.
 *
 *
 * ******************************************************************************/

void LEUART0_IRQHandler(void){
	uint32_t interrupt_flags = LEUART0->IF & LEUART0->IEN;
	LEUART0->IFC = interrupt_flags;
	if(interrupt_flags & LEUART_IF_TXBL){
		leuart0_txbl_interrupt();
	}
	if(interrupt_flags & LEUART_IF_TXC){
		leuart0_txc_interrupt();
	}
}

/***************************************************************************//**
 * @brief
 *   Function to start Low Energy UART peripheral communication port write operation.
 *
 * @details
 *   This routine initializes an LEUART_SM struct which stores the state of the LEUART
 *   transmission. All information required by the LEUART state machine interacts with
 *   this LEUART LEUART_SM struct.
 *   Once the LEUART_SM is initialized, this function enables the TXBL interrupt,
 *   which initiates the transmit sequence.
 *
 *  @note
 *    This function must only be called when the state of the transmit state machine
 *    is in IDLE mode and when the LEUART peripheral is also in an IDLE state.
 *
 * @param[in] leuart
 *   Pointer to the base peripheral address of the LEUART peripheral being used.
 *
 * @param[in] string
 *   The string which must be transmitted over UART.
 *
 * @param[in] string_len
 *   The number of characters in the string to be transmitted.
 *
 * ******************************************************************************/

void leuart_start(LEUART_TypeDef *leuart, char *string, uint32_t string_len){
	while(!(leuart->STATUS &  LEUART_STATUS_TXIDLE));

	CORE_DECLARE_IRQ_STATE;
	CORE_ENTER_CRITICAL();

	sleep_block_mode(LEUART_TX_EM);

	sm.leuart = leuart;
	sm.state = idle;
	sm.count = 0;
	strcpy(sm.output, string);
	sm.str_length = string_len;
	sm.sm_busy = true;

	leuart->IEN = LEUART_IEN_TXBL;

	CORE_EXIT_CRITICAL();
}

/***************************************************************************//**
 * @brief
 *
 ******************************************************************************/

//bool leuart_tx_busy(LEUART_TypeDef *leuart){
//
//}

/***************************************************************************//**
 * @brief
 *   LEUART STATUS function returns the STATUS of the peripheral for the
 *   TDD test
 *
 * @details
 * 	 This function enables the LEUART STATUS register to be provided to
 * 	 a function outside this .c module.
 *
 * @param[in] *leuart
 *   Defines the LEUART peripheral to access.
 *
 * @return
 * 	 Returns the STATUS register value as an uint32_t value
 *
 ******************************************************************************/

uint32_t leuart_status(LEUART_TypeDef *leuart){
	uint32_t	status_reg;
	status_reg = leuart->STATUS;
	return status_reg;
}

/***************************************************************************//**
 * @brief
 *   LEUART CMD Write sends a command to the CMD register
 *
 * @details
 * 	 This function is used by the TDD test function to program the LEUART
 * 	 for the TDD tests.
 *
 * @note
 *   Before exiting this function to update  the CMD register, it must
 *   perform a SYNCBUSY while loop to ensure that the CMD has by synchronized
 *   to the lower frequency LEUART domain.
 *
 * @param[in] *leuart
 *   Defines the LEUART peripheral to access.
 *
 * @param[in] cmd_update
 * 	 The value to write into the CMD register
 *
 ******************************************************************************/

void leuart_cmd_write(LEUART_TypeDef *leuart, uint32_t cmd_update){

	leuart->CMD = cmd_update;
	while(leuart->SYNCBUSY);
}

/***************************************************************************//**
 * @brief
 *   LEUART IF Reset resets all interrupt flag bits that can be cleared
 *   through the Interrupt Flag Clear register
 *
 * @details
 * 	 This function is used by the TDD test program to clear interrupts before
 * 	 the TDD tests and to reset the LEUART interrupts before the TDD
 * 	 exits
 *
 * @param[in] *leuart
 *   Defines the LEUART peripheral to access.
 *
 ******************************************************************************/

void leuart_if_reset(LEUART_TypeDef *leuart){
	leuart->IFC = 0xffffffff;
}

/***************************************************************************//**
 * @brief
 *   LEUART App Transmit Byte transmits a byte for the LEUART TDD test
 *
 * @details
 * 	 The BLE module will respond to AT commands if the BLE module is not
 * 	 connected to the phone app.  To validate the minimal functionality
 * 	 of the LEUART peripheral, write and reads to the LEUART will be
 * 	 performed by polling and not interrupts.
 *
 * @note
 *   In polling a transmit byte, a while statement checking for the TXBL
 *   bit in the Interrupt Flag register is required before writing the
 *   TXDATA register.
 *
 * @param[in] *leuart
 *   Defines the LEUART peripheral to access.
 *
 * @param[in] data_out
 *   Byte to be transmitted by the LEUART peripheral
 *
 ******************************************************************************/

void leuart_app_transmit_byte(LEUART_TypeDef *leuart, uint8_t data_out){
	while (!(leuart->IF & LEUART_IF_TXBL));
	leuart->TXDATA = data_out;
}


/***************************************************************************//**
 * @brief
 *   LEUART App Receive Byte polls a receive byte for the LEUART TDD test
 *
 * @details
 * 	 The BLE module will respond to AT commands if the BLE module is not
 * 	 connected to the phone app.  To validate the minimal functionality
 * 	 of the LEUART peripheral, write and reads to the LEUART will be
 * 	 performed by polling and not interrupts.
 *
 * @note
 *   In polling a receive byte, a while statement checking for the RXDATAV
 *   bit in the Interrupt Flag register is required before reading the
 *   RXDATA register.
 *
 * @param[in] leuart
 *   Defines the LEUART peripheral to access.
 *
 * @return
 * 	 Returns the byte read from the LEUART peripheral
 *
 ******************************************************************************/

uint8_t leuart_app_receive_byte(LEUART_TypeDef *leuart){
	uint8_t leuart_data;
	while (!(leuart->IF & LEUART_IF_RXDATAV));
	leuart_data = leuart->RXDATA;
	return leuart_data;
}


/***************************************************************************//**
 * @brief
 *   Function that the LEUART interrupt handler will call upon receiving the
 *   LEUART TXBL interrupt
 *
 * @details
 * 	 This function defines the behavior of the state machine in each state when
 * 	 a LEUART TXBL interrupt is received.
 *
 *
 * ******************************************************************************/

static void leuart0_txbl_interrupt(void){
	switch(sm.state){
			case idle:
				sm.leuart->TXDATA = sm.output[sm.count];
				sm.count++;
				sm.state = send_data;
				break;
			case send_data:
				if(sm.count == (sm.str_length - 1)){
					sm.leuart->TXDATA = sm.output[sm.count];
					sm.count++;
					sm.leuart->IEN &= ~LEUART_IEN_TXBL;
					sm.leuart->IEN |= LEUART_IEN_TXC;
					sm.state = active_trans;
				}
				else{
					sm.leuart->TXDATA = sm.output[sm.count];
					sm.count++;
					sm.state = send_data;
				}
				break;
			case active_trans:
				EFM_ASSERT(false);
				break;
			case end_comm:
				EFM_ASSERT(false);
				break;
		}
}

/***************************************************************************//**
 * @brief
 *   Function that the LEUART interrupt handler will call upon receiving the
 *   LEUART TXC interrupt
 *
 * @details
 * 	 This function defines the behavior of the state machine in each state when
 * 	 a LEUART TXC interrupt is received.
 *
 *
 * ******************************************************************************/


static void leuart0_txc_interrupt(void){
	switch(sm.state){
			case idle:
				EFM_ASSERT(false);
				break;
			case send_data:
				EFM_ASSERT(false);
				break;
			case active_trans:
				sm.state = end_comm;
				sm.leuart->IFS = LEUART_IFS_TXC;
				break;
			case end_comm:
				sm.leuart->IEN &= ~LEUART_IEN_TXC;
				sm.sm_busy = false;
				sleep_unblock_mode(LEUART_TX_EM);
				add_scheduled_event(tx_done_evt);
				break;
	}

}
