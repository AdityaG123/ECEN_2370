/**
 *
 * @file sleep_routines.c
 * @Aditya Gopalan
 * @September 10th, 2020
 * @brief Contains functions to set the Peal Gecko to a certain energy and sleep modes
 *
 */


/**************************************************************************
* @file sleep.c
***************************************************************************
* @section License
* <b>(C) Copyright 2015 Silicon Labs, http://www.silabs.com</b>
***************************************************************************
*
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
*
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software.
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
*
* DISCLAIMER OF WARRANTY/LIMITATION OF REMEDIES: Silicon Labs has no
* obligation to support this Software. Silicon Labs is providing the
* Software "AS IS", with no express or implied warranties of any kind,
* including, but not limited to, any implied warranties of merchantability
* or fitness for any particular purpose or warranties against infringement
* of any proprietary rights of a third party.
*
* Silicon Labs will not be liable for any consequential, incidental, or
* special damages, or any other relief, or for any claim by any third party,
* arising from your use of this Software.
*
**************************************************************************/


#include "sleep_routines.h"


//***********************************************************************************
// private variables
//***********************************************************************************
static int lowest_energy_mode[MAX_ENERGY_MODES];



//***********************************************************************************
// functions
//***********************************************************************************


/*************************************************************//**
* @brief
* This function is used to enable all 5 of the energy modes in the Pearl Gecko micro-controller
*
* @details
*  Initialize private variable for sleep_routines so all energy modes are unblocked
*
* @note
*Sets all the energy modes to the lowest possible active frequency
*
*****************************************************************/
void sleep_open(void){
	int i;
	for(i = 0; i < MAX_ENERGY_MODES; i++ ){
		lowest_energy_mode[i] &= ~lowest_energy_mode[i];
	}
}


/*************************************************************//**
* @brief
* Sleep Unblock Mode
*
* @details
* Utilized to release the processor from going into a sleep mode
* when a peripheral is no longer active.
*
* @note
* Since this is the unblock sleep mode, we need to decrement the lowest energy modes
* as we cycle through them.
*
* @param[in] EM
*  Unsigned 32 bit integer representing the Energy Mode (EM0 - EM4)
*
*
*****************************************************************/
void sleep_unblock_mode(uint32_t EM){
	CORE_DECLARE_IRQ_STATE;
	CORE_ENTER_CRITICAL();
	lowest_energy_mode[EM]--;
	EFM_ASSERT(lowest_energy_mode[EM] >= 0);
	CORE_EXIT_CRITICAL();
}


/*************************************************************//**
* @brief
* Sleep Block Mode
*
*
* @details
* 	Utilized by a peripheral to prevent the Pearl Gecko from going
*	into that sleep mode while the peripheral is active.
*
* @note
* Since this is the block sleep mode, we need to increment the lowest energy modes
* as we cycle through them.
*
*@param[in] EM
*	Unsigned 32 bit integer representing the Energy Mode (EM0 - EM4)
*
*
*****************************************************************/
void sleep_block_mode(uint32_t EM){
	CORE_DECLARE_IRQ_STATE;
	CORE_ENTER_CRITICAL();
	lowest_energy_mode[EM]++;
	EFM_ASSERT(lowest_energy_mode[EM] < 5);
	CORE_EXIT_CRITICAL();
}


/*************************************************************//**
* @brief
* Checks to see what energy mode is blocked to see what mode to put to sleep
*
* @details
* Checks to see what last energy mode is blocked that is found, then it unlocks the
* the last energy mode. If none of the energy mode is found, then it enters the last EM3
* energy modes.
*
* @note
* This function is atomic and will enter a critical IRQ energy state.
*
*****************************************************************/
void enter_sleep(void){
	CORE_DECLARE_IRQ_STATE;
	CORE_ENTER_CRITICAL();

	if(lowest_energy_mode[EM0] > 0){
	}
	else if(lowest_energy_mode[EM1] > 0){
	}
	else if (lowest_energy_mode[EM2] > 0) {
		EMU_EnterEM1();
	}
	else if (lowest_energy_mode[EM3] > 0){
		EMU_EnterEM2(true);
	}
	else {
		EMU_EnterEM3(true);
	}

	CORE_EXIT_CRITICAL();
	return;
}


/*************************************************************//**
* @brief
* Checks the current lowest energy mode
*
*
* @details
* Function that returns which energy mode that the current system cannot enter.
* Cycles through all of the energy modes and checks to see if they are active or not.
* If the EM is active and being used, then the board will return and output the current blocked energy mode.
* If not, then it will check again after decrementing the energy mode.
*
* @return
* A 32 bit integer with the current block energy mode.
*
*
*****************************************************************/
uint32_t current_block_energy_mode(void){
	int i;
	for(i = 0; i < MAX_ENERGY_MODES; i++ ){
		if(lowest_energy_mode[i] != 0){
			return i;
		}
	}
	return MAX_ENERGY_MODES - 1;
}
