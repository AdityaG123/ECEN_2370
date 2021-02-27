/**
 *
 * @file cmu.c
 * @author Aditya Gopalan
 * @date September 10th, 2020
 * @brief  responsible in enabling all oscillators and routing the clock tree for the application
 *
 */


//***********************************************************************************
// Include files
//***********************************************************************************
#include "cmu.h"

//***********************************************************************************
// defined files
//***********************************************************************************


//***********************************************************************************
// Private variables
//***********************************************************************************


//***********************************************************************************
// Private functions
//***********************************************************************************


//***********************************************************************************
// Global functions
//***********************************************************************************


/*************************************************************//**
* @brief
* 	This part of the code is going to be responsible in enabling all oscillators and routing the clock tree for the application
*
*
* @details
* 	The clock management unit is the part that is going to be ensuring that the all the cmu oscillators are enabled while
* 	setting the LFRCO and LFXO to false to disable them.
*
*
* @note
* 	This function does not enable LETIMER0. LETIMER0 is enabled in the letimer_pwm_open() function
*
*****************************************************************/


void cmu_open(void){

		CMU_ClockEnable(cmuClock_HFPER, true);

		// By default, Low Frequency Resistor Capacitor Oscillator, LFRCO, is enabled,
		// Disable the LFRCO oscillator
		CMU_OscillatorEnable(cmuOsc_LFRCO, false, false);	 // What is the enumeration required for LFRCO?

		// Disable the Low Frequency Crystal Oscillator, LFXO
		CMU_OscillatorEnable(cmuOsc_LFXO, true, false);	// What is the enumeration required for LFXO?

		// No requirement to enable the ULFRCO oscillator.  It is always enabled in EM0-4H

		// Route LF clock to LETIMER0 clock tree
		CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_ULFRCO);	// What clock tree does the LETIMER0 reside on?

		// Now, you must ensure that the global Low Frequency is enabled
		CMU_ClockEnable(cmuClock_CORELE, true);	//This enumeration is found in the Lab 2 assignment

		//Route LFX0 to LEUART
		CMU_ClockSelectSet(cmuClock_LFB, cmuSelect_LFXO);

}

