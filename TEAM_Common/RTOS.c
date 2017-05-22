/**
 * \file
 * \brief Real Time Operating System (RTOS) main program.
 * \author Erich Styger, erich.styger@hslu.ch
 */

#include "Platform.h"
#if PL_CONFIG_HAS_RTOS
#include "RTOS.h"
#include "FRTOS1.h"

static void RTOSTask(void *pvParameters) {
  (void)pvParameters; /* not used */

  for(;;) {
	  KEYDBNC_Process();
	  EVNT_HandleEvent(&APP_EventHandler, TRUE);

	  vTaskDelay(pdMS_TO_TICKS(10));
  } /* for */
}
#endif /* PL_CONFIG_HAS_RTOS */

void RTOS_Init(void) {
	/*! \todo Create tasks here */
	if (FRTOS1_xTaskCreate(RTOSTask, "RTOSTask", configMINIMAL_STACK_SIZE+100, NULL, tskIDLE_PRIORITY+1, NULL) != pdPASS) {
		for(;;){} /* error */
	}
}

void RTOS_Deinit(void) {
  /* nothing needed for now */
}
