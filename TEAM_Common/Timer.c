/**
 * \file
 * \brief Timer driver
 * \author Erich Styger, erich.styger@hslu.ch
 *
 * This module implements the driver for all our timers.
  */

#include "Platform.h"
#if PL_CONFIG_HAS_TIMER
#include "Timer.h"
#if PL_CONFIG_HAS_EVENTS
  #include "Event.h"
#endif
#if PL_CONFIG_HAS_TRIGGER
  #include "Trigger.h"
#endif
#if PL_CONFIG_HAS_MOTOR_TACHO
  #include "Tacho.h"
#endif
#include "TMOUT1.h"
#include "TI1.h"

uint16 Ticks;

void TMR_OnInterrupt(void) {
  /* this one gets called from an interrupt!!!! */
  /*! \todo Add code for a blinking LED here */

	/*
	if(Ticks == 2000/TMR_TICK_MS){
		Ticks = 0;
		EVNT_SetEvent(EVNT_LED_HEARTBEAT);
	} else{
		Ticks++;
	}
	*/
	TRG_AddTick();
}

void TMR_Init(void) {
	TI1_Enable();
}

void TMR_Deinit(void) {
}

#endif /* PL_CONFIG_HAS_TIMER*/
