/**
 * @file ble.c
 * @author Keith Graham and Aditya Gopalan
 * @date  October 29th, 2020
 * @brief Contains all the functions to interface the application with the HM-18
 *   BLE module and the LEUART driver
 *
 */


//***********************************************************************************
// Include files
//***********************************************************************************
#include "ble.h"
#include <string.h>

//***********************************************************************************
// defined files
//***********************************************************************************



//***********************************************************************************
// private variables
//***********************************************************************************

/***************************************************************************//**
 * @brief BLE module
 * @details
 *  This module contains all the functions to interface the application layer
 *  with the HM-18 Bluetooth module.  The application does not have the
 *  responsibility of knowing the physical resources required, how to
 *  configure, or interface to the Bluetooth resource including the LEUART
 *  driver that communicates with the HM-18 BLE module.
 *
 ******************************************************************************/

//***********************************************************************************
// Global functions
//***********************************************************************************

CIRC_TEST_STRUCT 				test_struct;
static BLE_CIRCULAR_BUF 		ble_cbuf;


//***********************************************************************************
// Private functions
//***********************************************************************************
static void ble_circ_init(void);
static void ble_circ_push(char *string);
static uint8_t ble_circ_space(void);
static void update_circ_wrtindex(BLE_CIRCULAR_BUF *index_struct, uint32_t update_by);
static void update_circ_readindex(BLE_CIRCULAR_BUF *index_struct, uint32_t update_by);


/***************************************************************************//**
 * @brief
 *   Function to open a LEUART port for the BLE HM10 module.
 *
 * @details
 * 	This function creates the LEUART_OPEN_STRUCT which specifies the configuration
 * 	required of the LEUART peripheral that will be used to communicate with the BLE
 * 	module. It then opens a LEUART port using this information.
 *
 * @param[in] tx_event
 *   The scheduler event associated with a TX Done Event.
 *
 * @param[in] rx_event
 *   The scheduler event associated with a RX Done Event.
 *
 ******************************************************************************/

void ble_open(uint32_t tx_event, uint32_t rx_event){
	LEUART_OPEN_STRUCT leuart_set;

	leuart_set.baudrate    = HM10_BAUDRATE;
	leuart_set.databits    = HM10_DATABITS;
	leuart_set.enable      = HM10_ENABLE;
	leuart_set.parity      = HM10_PARITY;
	leuart_set.stopbits    = HM10_STOPBITS;
//	leuart_set.refFreq     = HM10_REFFREQ;
	leuart_set.rx_done_evt = rx_event;
	leuart_set.tx_done_evt = tx_event;
	leuart_set.tx_loc      = LEUART0_TX_ROUTE;
	leuart_set.tx_pin_en   = true;
	leuart_set.rx_loc      = LEUART0_RX_ROUTE;
	leuart_set.rx_pin_en   = true;

	leuart_open(HM10_LEUART0, &leuart_set);

	ble_circ_init();

}


/***************************************************************************//**
 * @brief
 *   This is a function to write a string to a BLE module.
 * @details
 *   This function packages a write request to send a string over LEUART for
 *   the bluetooth module.
 *
 * @param[in] string
 *   the string that shall be transmitted over BLE
 *
 ******************************************************************************/

void ble_write(char* string){
	ble_circ_push(string);
	ble_circ_pop(CIRC_OPER);
}

/***************************************************************************//**
 * @brief
 *   BLE Test performs two functions.  First, it is a Test Driven Development
 *   routine to verify that the LEUART is correctly configured to communicate
 *   with the BLE HM-18 module.  Second, the input argument passed to this
 *   function will be written into the BLE module and become the new name
 *   advertised by the module while it is looking to pair.
 *
 * @details
 * 	 This global function will use polling functions provided by the LEUART
 * 	 driver for both transmit and receive to validate communications with
 * 	 the HM-18 BLE module.  For the assignment, the communication with the
 * 	 BLE module must use low energy design principles of being an interrupt
 * 	 driven state machine.
 *
 * @note
 *   For this test to run to completion, the phone most not be paired with
 *   the BLE module.  In addition for the name to be stored into the module
 *   a breakpoint must be placed at the end of the test routine and stopped
 *   at this breakpoint while in the debugger for a minimum of 5 seconds.
 *
 * @param[in] *mod_name
 *   The name that will be written to the HM-18 BLE module to identify it
 *   while it is advertising over Bluetooth Low Energy.
 *
 * @return
 *   Returns bool true if successfully passed through the tests in this
 *   function.
 ******************************************************************************/

bool ble_test(char *mod_name){
	uint32_t	str_len;

	CORE_DECLARE_IRQ_STATE;
	CORE_ENTER_CRITICAL();

	// This test will limit the test to the proper setup of the LEUART
	// peripheral, routing of the signals to the proper pins, pin
	// configuration, and transmit/reception verification.  The test
	// will communicate with the BLE module using polling routines
	// instead of interrupts.

	// How is polling different than using interrupts?
	// ANSWER: Polling uses CPU cycles constantly to check for an event,
	// whereas interrupts allow the CPU to go to sleep until needed. However, in
	// polling, you might react to an event later because you'd be
	// checking at time intervals instead of reacting to when the interrupts actually happen.

	// How does interrupts benefit the system for low energy operation?
	// ANSWER: They allow the CPU (which generally draws the most energy)
	// to go to sleep and only be enabled when its needed.

	// How does interrupts benefit the system that has multiple tasks?
	// ANSWER: It allows the system to initiate multiple tasks at the same time
	// and then wait for the responses, instead of waiting for one task to
	// complete before starting the next one.

	// First, you will need to review the DSD HM10 datasheet to determine
	// what the default strings to write data to the BLE module and the
	// expected return statement from the BLE module to test / verify the
	// correct response

	// The test_str is used to tell the BLE module to end a Bluetooth connection
	// such as with your phone.  The ok_str is the result sent from the BLE module
	// to the micro-controller if there was not active BLE connection at the time
	// the break command was sent to the BLE module.
	// Replace the test_str "" with the command to break or end a BLE connection
	// Replace the ok_str "" with the result that will be returned from the BLE
	//   module if there was no BLE connection
	char		test_str[80] = "AT";
	char		ok_str[80] = "OK";


	// output_str will be the string that will program a name to the BLE module.
	// From the DSD HM10 datasheet, what is the command to program a name into
	// the BLE module?
	// The  output_str will be a string concatenation of the DSD HM10 command
	// and the input argument sent to the ble_test() function
	// Replace the output_str "" with the command to change the program name
	// Replace the result_str "" with the first part of the expected result
	//  the backend of the expected response will be concatenated with the
	//  input argument
	char		output_str[80] = "AT+NAME";
	char		result_str[80] = "OK+Set:";


	// To program the name into your module, you must reset the module after you
	// have sent the command to update the modules name.  What is the DSD HM10
	// name to reset the module?
	// Replace the reset_str "" with the command to reset the module
	// Replace the reset_result_str "" with the expected BLE module response to
	//  to the reset command
	char		reset_str[80] = "AT+RESET";
	char		reset_result_str[80] = "OK+RESET";
	char		return_str[80];

	bool		success;
	bool		rx_disabled, rx_en, tx_en;
	uint32_t	status;

	// These are the routines that will build up the entire command and response
	// of programming the name into the BLE module.  Concatenating the command or
	// response with the input argument name
	strcat(output_str, mod_name);
	strcat(result_str, mod_name);

	// The test routine must not alter the function of the configuration of the
	// LEUART driver, but requires certain functionality to insure the logical test
	// of writing and reading to the DSD HM10 module.  The following c-lines of code
	// save the current state of the LEUART driver that will be used later to
	// re-instate the LEUART configuration

	status = leuart_status(HM10_LEUART0);
	if (status & LEUART_STATUS_RXBLOCK) {
		rx_disabled = true;
		// Enabling, unblocking, the receiving of data from the LEUART RX port
		leuart_cmd_write(HM10_LEUART0, LEUART_CMD_RXBLOCKDIS);
	}
	else rx_disabled = false;
	if (status & LEUART_STATUS_RXENS) {
		rx_en = true;
	} else {
		rx_en = false;
		// Enabling the receiving of data from the RX port
		leuart_cmd_write(HM10_LEUART0, LEUART_CMD_RXEN);
		while (!(leuart_status(HM10_LEUART0) & LEUART_STATUS_RXENS));
	}

	if (status & LEUART_STATUS_TXENS){
		tx_en = true;
	} else {
		// Enabling the transmission of data to the TX port
		leuart_cmd_write(HM10_LEUART0, LEUART_CMD_TXEN);
		while (!(leuart_status(HM10_LEUART0) & LEUART_STATUS_TXENS));
		tx_en = false;
	}
//	leuart_cmd_write(HM10_LEUART0, (LEUART_CMD_CLEARRX | LEUART_CMD_CLEARTX));

	// This sequence of instructions is sending the break ble connection
	// to the DSD HM10 module.


	// Why is this command required if you want to change the name of the
	// DSD HM10 module?
	// ANSWER: This is going to check whether the device is connected.
	// If the device is connected, then the module will respond "OK"
	// if the device is not connected, and will respond "OK+LOST"
	// if there is a connection, and then disconnect the remote device.


	str_len = strlen(test_str);
	for (int i = 0; i < str_len; i++){
		leuart_app_transmit_byte(HM10_LEUART0, test_str[i]);
	}

	// What will the ble module response back to this command if there is
	// a current ble connection?
	// ANSWER: OK+LOST


	str_len = strlen(ok_str);
	for (int i = 0; i < str_len; i++){
		return_str[i] = leuart_app_receive_byte(HM10_LEUART0);
		if (ok_str[i] != return_str[i]) {
				EFM_ASSERT(false);;
		}
	}

	// This sequence of code will be writing or programming the name of
	// the module to the DSD HM10
	str_len = strlen(output_str);
	for (int i = 0; i < str_len; i++){
		leuart_app_transmit_byte(HM10_LEUART0, output_str[i]);
	}

	// Here will be the check on the response back from the DSD HM10 on the
	// programming of its name
	str_len = strlen(result_str);
	for (int i = 0; i < str_len; i++){
		return_str[i] = leuart_app_receive_byte(HM10_LEUART0);
		if (result_str[i] != return_str[i]) {
				EFM_ASSERT(false);;
		}
	}

	// It is now time to send the command to RESET the DSD HM10 module
	str_len = strlen(reset_str);
	for (int i = 0; i < str_len; i++){
		leuart_app_transmit_byte(HM10_LEUART0, reset_str[i]);
	}

	// After sending the command to RESET, the DSD HM10 will send a response
	// back to the micro-controller
	str_len = strlen(reset_result_str);
	for (int i = 0; i < str_len; i++){
		return_str[i] = leuart_app_receive_byte(HM10_LEUART0);
		if (reset_result_str[i] != return_str[i]) {
				EFM_ASSERT(false);;
		}
	}

	// After the test and programming have been completed, the original
	// state of the LEUART must be restored
	if (!rx_en) leuart_cmd_write(HM10_LEUART0, LEUART_CMD_RXDIS);
	if (rx_disabled) leuart_cmd_write(HM10_LEUART0, LEUART_CMD_RXBLOCKEN);
	if (!tx_en) leuart_cmd_write(HM10_LEUART0, LEUART_CMD_TXDIS);
	leuart_if_reset(HM10_LEUART0);

	success = true;


	CORE_EXIT_CRITICAL();
	return success;
}




/***************************************************************************//**
 * @brief
 *   Circular Buff Test is a Test Driven Development function to validate
 *   that the circular buffer implementation
 *
 * @details
 * 	 This Test Driven Development test has tests integrated into the function
 * 	 to validate that the routines can successfully identify whether there
 * 	 is space available in the circular buffer, the write and index pointers
 * 	 wrap around, and that one or more packets can be pushed and popped from
 * 	 the circular buffer.
 *
 * @note
 *   If anyone of these test will fail, an EFM_ASSERT will occur.  If the
 *   DEBUG_EFM=1 symbol is defined for this project, exiting this function
 *   confirms that the push, pop, and the associated utility functions are
 *   working.
 *
 * @par
 *   There is a test escape that is not possible to test through this
 *   function that will need to be verified by writing several ble_write()s
 *   back to back and verified by checking that these ble_write()s were
 *   successfully transmitted to the phone app.
 *
 ******************************************************************************/

 void circular_buff_test(void){
	 bool buff_empty;
	 int test1_len = 50;
	 int test2_len = 25;
	 int test3_len = 5;

	 // Why this 0 initialize of read and write pointer?
	 // Student Response:
	 // This initializes both to point at the 0 index of the buffer because the buffer is empty.
	 ble_cbuf.read_ptr = 0;
	 ble_cbuf.write_ptr = 0;

	 // Why do none of these test strings contain a 0?
	 // Student Response:
	 // If it puts a 0, it will be a null character. This just fills them with random integers, which
	 // will not print out as anything useful but if it puts a zero that will be interpreted as the end of the string.
	 for (int i = 0;i < test1_len; i++){
		 test_struct.test_str[0][i] = i+1;
	 }
	 test_struct.test_str[0][test1_len] = 0;

	 for (int i = 0;i < test2_len; i++){
		 test_struct.test_str[1][i] = i + 20;
	 }
	 test_struct.test_str[1][test2_len] = 0;

	 for (int i = 0;i < test3_len; i++){
		 test_struct.test_str[2][i] = i +  35;
	 }
	 test_struct.test_str[2][test3_len] = 0;

	 // What is this test validating?
	 // Student response:
	 // This validates that the index after the last character is NULL
	 EFM_ASSERT(ble_circ_space() == CSIZE);

	 // Why is there only one push to the circular buffer at this stage of the test
	 // Student Response:
	 // We are testing a single push only rather than multiple pushes to see if there is no overflow situations
	 ble_circ_push(&test_struct.test_str[0][0]);

	 // What is this test validating?
	 // Student response:
	 // This checks to see that the buffer space is getting decreased by the length of the string that was added
	 EFM_ASSERT(ble_circ_space() == (CSIZE - test1_len - 1));

	 // Why is the expected buff_empty test = false?
	 // Student Response:
	 //	The data was able to be popped from the buffer. This means that the read pointer was able
	 // to not pass the write pointer
	 buff_empty = ble_circ_pop(CIRC_TEST);
	 EFM_ASSERT(buff_empty == false);
	 for (int i = 0; i < test1_len; i++){
		 EFM_ASSERT(test_struct.test_str[0][i] == test_struct.result_str[i]);
	 }

	 // What is this test validating?
	 // Student response:
	 // The pop command takes the full string off of the buffer
	 EFM_ASSERT(strlen(test_struct.result_str) == test1_len);

	 // What is this test validating?
	 // Student response:
	 // Checks to see if the full buffer is available
	 EFM_ASSERT(ble_circ_space() == CSIZE);

	 // What does this next push on the circular buffer test?
	 // Student Response:
	 // It makes sure that the second push into the buffer will not cause an overflow and it takes up
	 // the amount of space based on the length
	 ble_circ_push(&test_struct.test_str[1][0]);


	 EFM_ASSERT(ble_circ_space() == (CSIZE - test2_len - 1));

	 // What does this next push on the circular buffer test?
	 // Student Response:
	 // Another push onto the buffer with existing data does not cause an
	 // overflow or overwrite any data
	 ble_circ_push(&test_struct.test_str[2][0]);


	 EFM_ASSERT(ble_circ_space() == (CSIZE - test2_len - 1 - test3_len - 1));

	 // What does this next push on the circular buffer test?
	 // Student Response:
	 // The amount of space taken up should be less than the space in the buffer.
	 EFM_ASSERT(abs(ble_cbuf.write_ptr - ble_cbuf.read_ptr) < CSIZE);

	 // Why is the expected buff_empty test = false?
	 // Student Response:
	 //	The data was able to be popped from the buffer. This means that the read pointer was able
	 // to not pass the write pointer
	 buff_empty = ble_circ_pop(CIRC_TEST);
	 EFM_ASSERT(buff_empty == false);
	 for (int i = 0; i < test2_len; i++){
		 EFM_ASSERT(test_struct.test_str[1][i] == test_struct.result_str[i]);
	 }

	 // What is this test validating?
	 // Student response:
	 // Checks to see if the string popped off the the buffer is the second test string
	 // while leaving the third string untouched
	 EFM_ASSERT(strlen(test_struct.result_str) == test2_len);

	 EFM_ASSERT(ble_circ_space() == (CSIZE - test3_len - 1));

	 // Why is the expected buff_empty test = false?
	 // Student Response:
	 //	The data was able to be popped from the buffer. This means that the read pointer was able
	 // to not pass the write pointer
	 buff_empty = ble_circ_pop(CIRC_TEST);
	 EFM_ASSERT(buff_empty == false);
	 for (int i = 0; i < test3_len; i++){
		 EFM_ASSERT(test_struct.test_str[2][i] == test_struct.result_str[i]);
	 }

	 // What is this test validating?
	 // Student response:
	 // Checks to see if the third test string was fully popped off the buffer and
	 // the full buffer space is available
	 EFM_ASSERT(strlen(test_struct.result_str) == test3_len);

	 EFM_ASSERT(ble_circ_space() == CSIZE);

	 // Using these three writes and pops to the circular buffer, what other test
	 // could we develop to better test out the circular buffer?
	 // Student Response:
	 // Tries to wrtite to the full buffer to ensure that it will stop the write. Try to pop
	 // from an empty buffer to ensure that buff_empty == true;

	 // Why is the expected buff_empty test = true?
	 // Student Response:
	 // There is no data to be popped off of the buffer
	 buff_empty = ble_circ_pop(CIRC_TEST);
	 EFM_ASSERT(buff_empty == true);
	 ble_write("\nPassed Circular Buffer Test\n");

 }

 /***************************************************************************//**
  * @brief
  *   Circular buffer init
  *
  * @details
  * 	Initializes the circular buffer for the BLE module.
  *
  * 	Sets size, read_ptr and write_ptr to 0.
  * 	Sets size_mask to be 1 less than CSIZE.
  * 	Initializes all chars in the buffer to 0 / null.
  *
  *
  ******************************************************************************/

static void ble_circ_init(void){
	ble_cbuf.size = CSIZE;
	ble_cbuf.size_mask = CSIZE - 1;
	ble_cbuf.read_ptr = 0;
	ble_cbuf.write_ptr = 0;
}


/***************************************************************************//**
 * @brief
 *   Circular buffer pop
 *
 * @details
 *	Pops a string off the circular buffer and transmits over LEUART to BLE device.
 *
 * @param[in] test
 *   test boolean flag
 *
 * @return
 *   Returns bool false if the buffer is not empty. Will assert/halt the program
 *    if there is a critical error and will return true if there is nothing to pop.
 *
 ******************************************************************************/

bool ble_circ_pop(bool test){
	if(leuart_tx_busy(HM10_LEUART0)){
		return true;
	}
	if(ble_cbuf.read_ptr == ble_cbuf.write_ptr){
		return true;
	}
	int str_length = ble_cbuf.cbuf[ble_cbuf.read_ptr];
	char print_str[str_length - 1];
	for(int i = 0; i < str_length - 1; i++){
		print_str[i] = ble_cbuf.cbuf[(ble_cbuf.read_ptr + i + 1) & ble_cbuf.size_mask];
	}
	if(test){
		for(int j = 0; j < ble_cbuf.size; j++){
			test_struct.result_str[j] = 0;
		}
		for(int i = 0; i < str_length - 1; i++){
			test_struct.result_str[i] = print_str[i];
		}
	}
	else{
		leuart_start(HM10_LEUART0, print_str, str_length - 1);
	}
	update_circ_readindex(&ble_cbuf, str_length);
	return false;
}


/***************************************************************************//**
 * @brief
 *   Circular buffer push
 *
 * @details
 *	Pushes a string on to the circular buffer.
 *
 * @note
 * 	This function is atomic.
 *
 * @param[in] *string
 *   pointer to the string that will be added to the buffer.
 *
 ******************************************************************************/

static void ble_circ_push(char *string){
	int string_len = (strlen(string) + 1);
	if(ble_circ_space() < string_len){
		EFM_ASSERT(false);
	}
	ble_cbuf.cbuf[ble_cbuf.write_ptr] = (char)(string_len);
	for(int i = 0; i < string_len - 1; i++){
		ble_cbuf.cbuf[(ble_cbuf.write_ptr + i + 1) & ble_cbuf.size_mask]  = string[i];
	}
	update_circ_wrtindex(&ble_cbuf, string_len);
}


/***************************************************************************//**
 * @brief
 *   BLE Circ Space
 *
 * @details
 *   checks if there is space in the buffer and return how many spaces are available
 *
 * @return
 *  8 bit integer indicating the number of spaces available in the buffer.
 *
 ******************************************************************************/

static uint8_t ble_circ_space(void){
	if(ble_cbuf.write_ptr >= ble_cbuf.read_ptr){
		return (ble_cbuf.size - abs(ble_cbuf.write_ptr - ble_cbuf.read_ptr));
	}
	else{
		return(ble_cbuf.read_ptr - ble_cbuf.write_ptr);
	}

}

/***************************************************************************//**
 * @brief
 *   Update circular buffer write index
 *
 * @details
 *   This increments the write pointer by the update_by value and does a bitwise
 *   AND with the size mask to effectively perform a fast modulo.
 *
 * @note
 * 	the length of the buffer must be a power of 2.
 *
 *
 * @param[in] *index_struct
 *   Pointer to the circular buffer struct to update
 *
 *
 * @param[in] update_by
 *   32 bit integer indicating the number to update the write index by
 *
 *
 ******************************************************************************/


static void update_circ_wrtindex(BLE_CIRCULAR_BUF *index_struct, uint32_t update_by){
	index_struct->write_ptr = (index_struct->write_ptr + update_by) & index_struct->size_mask;
}


/***************************************************************************//**
 * @brief
 *   Update circular buffer read index
 *
 * @details
 *  This increments the read pointer by the update_by value and does a bitwise
 *   AND with the size mask to effectively perform a fast modulo.
 *
 * @note
 * 	the length of the buffer must be a power of 2.
 *
 * @param[in] *index_struct
 *   Pointer to the circular buffer struct to update
 *
 * @param[in] update_by
 *   32 bit integer indicating the number to update the read index by
 *
 *
 ******************************************************************************/

static void update_circ_readindex(BLE_CIRCULAR_BUF *index_struct, uint32_t update_by){
	index_struct->read_ptr = (index_struct->read_ptr + update_by) & index_struct->size_mask;
}
