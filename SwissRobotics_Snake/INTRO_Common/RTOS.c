/**
 * \file
 * \brief Real Time Operating System (RTOS) main program.
 * \author Erich Styger, erich.styger@hslu.ch
 */

#include "Platform.h"
#if PL_CONFIG_HAS_RTOS
#include "RTOS.h"
#include "FRTOS1.h"
#include "Application.h"
#include "Event.h"
#include "LED.h"
#include "Keys.h"
#include "KeyDebounce.h"

static void AppTask(void* param) {
  const int *whichLED = (int*)param;

  for(;;) {
    if (*whichLED==1) {
      LED1_Neg();
    } else if (*whichLED==2) {
      LED2_Neg();
    }
#if PL_CONFIG_HAS_KEYS
    //KEY_Scan();
    KEYDBNC_Process();
#endif
#if PL_CONFIG_HAS_EVENTS
    EVNT_HandleEvent(APP_EventHandler, TRUE);
#endif
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

#if 0
static void KillMe(void* param) {
  LED1_On(); /* LED of death! */
  vTaskDelete(NULL); /* suicide! */
}
#endif

void RTOS_Init(void) {
  /*! \todo Create tasks here */
  static const int led1 = 1;
  //static const int led2 = 2;

   EVNT_SetEvent(EVNT_STARTUP); /* set startup event */
   /*! \todo Create tasks here */
   if (xTaskCreate(AppTask, (signed portCHAR *)"App", 500/sizeof(StackType_t), (void*)&led1, tskIDLE_PRIORITY+1, NULL) != pdPASS) {
     for(;;){} /* error case only, stay here! */
   }
#if 0
  if (xTaskCreate(AppTask, (signed portCHAR *)"App2", 100/sizeof(StackType_t), (void*)&led2, tskIDLE_PRIORITY, NULL) != pdPASS) {
    for(;;){} /* error case only, stay here! */
  }
#endif
#if 0
  if (xTaskCreate(KillMe, (signed portCHAR *)"KillMe", 100/sizeof(StackType_t), NULL, tskIDLE_PRIORITY, NULL) != pdPASS) {
    for(;;){} /* error case only, stay here! */
  }
#endif
}

void RTOS_Deinit(void) {
  /* nothing needed for now */
}

#endif /* PL_CONFIG_HAS_RTOS */
