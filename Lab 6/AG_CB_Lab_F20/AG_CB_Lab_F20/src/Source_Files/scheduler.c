/**
 *
 * @file scheduler.c
 * @Aditya Gopalan
 * @September 10th, 2020
 * @brief Adds the scheduled events for the irq handler
 *
 */

#include "scheduler.h"

#include "em_emu.h"
#include "em_assert.h"

//***********************************************************************************
// private variables
//***********************************************************************************
static unsigned int event_scheduled;


//***********************************************************************************
// functions
//***********************************************************************************


/*************************************************************//**
* @brief
* Initialize the scheduler
*
* @details
* This routine resets the "event_scheduled" variable to zero by zeroing out the register bit value
*
* @note
* At the beginning and at the end of this function we have to disable and enable the interrupt handler
* request register to allow  the event scheduled bit to be cleared
*
*****************************************************************/
void scheduler_open(void){
	CORE_DECLARE_IRQ_STATE;
	CORE_ENTER_CRITICAL();
	event_scheduled &= ~event_scheduled;
	CORE_EXIT_CRITICAL();
}


/*************************************************************//**
* @brief
*  Adds an event to the schedule
*
* @details
*  This routine adds an event to the #event_scheduled variable
*
*
* @note
* This is an atomic function
*
* @param[in] event
* This parameter is a 32 bit integer that contains the events scheduled that we want to add.
*
*
*****************************************************************/
void add_scheduled_event(uint32_t event){
	CORE_DECLARE_IRQ_STATE;
	CORE_ENTER_CRITICAL();
	event_scheduled |= event;
	CORE_EXIT_CRITICAL();
}


/*************************************************************//**
* @brief
* Removes an event to the schedule
*
*
* @details
* This routine removes an event to the #event_scheduled variable
*
* @note
* This function is also atomic
*
*@param[in] event
* This parameter is a 32 bit integer that contains the events scheduled that we
* want to remove rather than add.
*
*
*
*****************************************************************/
void remove_scheduled_event(uint32_t event){
	CORE_DECLARE_IRQ_STATE;
	CORE_ENTER_CRITICAL();
	event_scheduled &= ~event;
	CORE_EXIT_CRITICAL();
}


/*************************************************************//**
* @brief
*  Returns the value of event_scheduled.
*
* @details
*  This routine returns the value of the "event_scheduled" variable
*
* @note
* This function is a 32 bit integer with the events that are scheduled that that are in the queue.
*
*
*****************************************************************/
uint32_t get_scheduled_events(void){
	return event_scheduled;
}
