/**
 * \file
 * \brief Module to handle the LCD display
 * \author Erich Styger, erich.styger@hslu.ch
 *
 * This module is driving the Nokia LCD Display.
 */

#include "Platform.h"
#if PL_CONFIG_HAS_LCD
#include "LCD.h"
#include "PDC1.h"
#include "GDisp1.h"
#include "GFONT1.h"
#include "FDisp1.h"
#include "Application.h"
#include "UTIL1.h"
#include "LCD_LED.h"
#include "Event.h"
#include "FRTOS1.h"
#if PL_CONFIG_HAS_RADIO
  #include "RApp.h"
#endif
#include "LCDMenu.h"
#include "Snake.h"
/*! \todo Add additional includes as needed */

/* status variables */
static bool LedBackLightisOn = TRUE;
static bool remoteModeIsOn = FALSE;
static bool requestLCDUpdate = FALSE;

static TaskHandle_t LCD_Task_Handle = NULL;

#if PL_CONFIG_HAS_LCD_MENU
typedef enum {
  LCD_MENU_ID_NONE = LCDMENU_ID_NONE, /* special value! */
  LCD_MENU_ID_MAIN,
    LCD_MENU_ID_BACKLIGHT,
    LCD_MENU_ID_NUM_VALUE,
  LCD_MENU_ID_GAMES,
  	LCD_MENU_ID_SNAKE,
} LCD_MenuIDs;

static LCDMenu_StatusFlags ValueChangeHandler(const struct LCDMenu_MenuItem_ *item, LCDMenu_EventType event, void **dataP) {
  static int value = 0;
  static uint8_t valueBuf[16];
  LCDMenu_StatusFlags flags = LCDMENU_STATUS_FLAGS_NONE;

  (void)item;
  if (event==LCDMENU_EVENT_GET_TEXT) {
    UTIL1_strcpy(valueBuf, sizeof(valueBuf), (uint8_t*)"Val: ");
    UTIL1_strcatNum32s(valueBuf, sizeof(valueBuf), value);
    *dataP = valueBuf;
    flags |= LCDMENU_STATUS_FLAGS_HANDLED|LCDMENU_STATUS_FLAGS_UPDATE_VIEW;
  } else if (event==LCDMENU_EVENT_GET_EDIT_TEXT) {
    UTIL1_strcpy(valueBuf, sizeof(valueBuf), (uint8_t*)"[-] ");
    UTIL1_strcatNum32s(valueBuf, sizeof(valueBuf), value);
    UTIL1_strcat(valueBuf, sizeof(valueBuf), (uint8_t*)" [+]");
    *dataP = valueBuf;
    flags |= LCDMENU_STATUS_FLAGS_HANDLED|LCDMENU_STATUS_FLAGS_UPDATE_VIEW;
  } else if (event==LCDMENU_EVENT_DECREMENT) {
    value--;
    flags |= LCDMENU_STATUS_FLAGS_HANDLED|LCDMENU_STATUS_FLAGS_UPDATE_VIEW;
  } else if (event==LCDMENU_EVENT_INCREMENT) {
    value++;
    flags |= LCDMENU_STATUS_FLAGS_HANDLED|LCDMENU_STATUS_FLAGS_UPDATE_VIEW;
  }
  return flags;
}

static LCDMenu_StatusFlags BackLightMenuHandler(const struct LCDMenu_MenuItem_ *item, LCDMenu_EventType event, void **dataP) {
  LCDMenu_StatusFlags flags = LCDMENU_STATUS_FLAGS_NONE;

  (void)item;
  if (event==LCDMENU_EVENT_GET_TEXT && dataP!=NULL) {
    if (LedBackLightisOn) {
      *dataP = "Backlight is ON";
    } else {
      *dataP = "Backlight is OFF";
    }
    flags |= LCDMENU_STATUS_FLAGS_HANDLED|LCDMENU_STATUS_FLAGS_UPDATE_VIEW;
  } else if (event==LCDMENU_EVENT_ENTER) { /* toggle setting */
    LedBackLightisOn = !LedBackLightisOn;
    flags |= LCDMENU_STATUS_FLAGS_HANDLED|LCDMENU_STATUS_FLAGS_UPDATE_VIEW;
  }
  return flags;
}

static LCDMenu_StatusFlags SnakeMenuHandler(const struct LCDMenu_MenuItem_ *item, LCDMenu_EventType event, void **dataP) {
	LCDMenu_StatusFlags flags = LCDMENU_STATUS_FLAGS_HANDLED;

	SNAKE_Init();

	vTaskSuspend(LCD_Task_Handle);

	return flags;
}




static const LCDMenu_MenuItem menus[] =
{/* id,                                     grp, pos,   up,                       down,                             text,           callback                      flags                  */
    {LCD_MENU_ID_MAIN,                        0,   0,   LCD_MENU_ID_NONE,         LCD_MENU_ID_BACKLIGHT,            "General",      NULL,                         LCDMENU_MENU_FLAGS_NONE},
	  {LCD_MENU_ID_BACKLIGHT,                 1,   0,   LCD_MENU_ID_MAIN,         LCD_MENU_ID_NONE,                 "Backlight",         BackLightMenuHandler,         LCDMENU_MENU_FLAGS_NONE},
      {LCD_MENU_ID_NUM_VALUE,                 1,   1,   LCD_MENU_ID_MAIN,         LCD_MENU_ID_NONE,                 "Value",      ValueChangeHandler,           LCDMENU_MENU_FLAGS_EDITABLE},

	{LCD_MENU_ID_GAMES,						  0,   1,   LCD_MENU_ID_NONE,  		  LCD_MENU_ID_SNAKE,				"Games",		NULL,						  LCDMENU_MENU_FLAGS_NONE},
	  {LCD_MENU_ID_SNAKE,					  2,   0,   LCD_MENU_ID_GAMES,		  LCD_MENU_ID_NONE,				    "Snake",        SnakeMenuHandler, 					LCDMENU_MENU_FLAGS_NONE},
};

#if PL_CONFIG_HAS_RADIO
uint8_t LCD_HandleRemoteRxMessage(RAPP_MSG_Type type, uint8_t size, uint8_t *data, RNWK_ShortAddrType srcAddr, bool *handled, RPHY_PacketDesc *packet) {
  (void)size;
  (void)packet;
  switch(type) {
     default:
      break;
  } /* switch */
  return ERR_OK;
}
#endif

#endif /* PL_CONFIG_HAS_LCD_MENU */


static void ShowTextOnLCD(unsigned char *text) {
  FDisp1_PixelDim x, y;

  GDisp1_Clear();
  x = 0;
  y = 10;
  FDisp1_WriteString(text, GDisp1_COLOR_BLACK, &x, &y, GFONT1_GetFont());
  GDisp1_UpdateFull();
}

static void DrawFont(void) {
  FDisp1_PixelDim x, y;

  GDisp1_Clear();
  GDisp1_UpdateFull();
  x = 0;
  y = 10;
  FDisp1_WriteString("Hello World!", GDisp1_COLOR_BLACK, &x, &y, GFONT1_GetFont());
  GDisp1_UpdateFull();
  vTaskDelay(pdMS_TO_TICKS(500));
  x = 0;
  y += GFONT1_GetBoxHeight();
  FDisp1_WriteString("with Fonts!", GDisp1_COLOR_BLACK, &x, &y, GFONT1_GetFont());
  GDisp1_UpdateFull();
  WAIT1_Waitms(1000);
}

static void DrawText(void) {
  GDisp1_Clear();
  GDisp1_UpdateFull();
  PDC1_WriteLineStr(1, "hello");
  vTaskDelay(pdMS_TO_TICKS(200));
  PDC1_WriteLineStr(2, "world");
  vTaskDelay(pdMS_TO_TICKS(200));
  PDC1_WriteLineStr(3, "out");
  vTaskDelay(pdMS_TO_TICKS(200));
  PDC1_WriteLineStr(4, "there");
  vTaskDelay(pdMS_TO_TICKS(200));
  PDC1_WriteLineStr(5, "!!!!!");
  vTaskDelay(pdMS_TO_TICKS(200));
}

static void LCD_Task(void *param) {
  (void)param; /* not used */
#if 0
  ShowTextOnLCD("Press a key!");
  DrawText();
  /* \todo extend */
  DrawFont();
  //DrawLines(); /*! \todo */
  //DrawCircles();
#endif
#if PL_CONFIG_HAS_LCD_MENU
  LCDMenu_InitMenu(menus, sizeof(menus)/sizeof(menus[0]), 1);
  LCDMenu_OnEvent(LCDMENU_EVENT_DRAW, NULL);
#endif
  for(;;) {
    if (LedBackLightisOn) {
      LCD_LED_On(); /* LCD backlight on */
    } else {
      LCD_LED_Off(); /* LCD backlight off */
    }
#if PL_CONFIG_HAS_LCD_MENU
    if (requestLCDUpdate) {
      requestLCDUpdate = FALSE;
      LCDMenu_OnEvent(LCDMENU_EVENT_DRAW, NULL);
    }
#if 1 /*! \todo Change this to for your own needs, or use direct task notification */
    if (EVNT_EventIsSetAutoClear(EVNT_LCD_BTN_LEFT)) { /* left */
      LCDMenu_OnEvent(LCDMENU_EVENT_LEFT, NULL);
//      ShowTextOnLCD("left");
    }
    if (EVNT_EventIsSetAutoClear(EVNT_LCD_BTN_RIGHT)) { /* right */
      LCDMenu_OnEvent(LCDMENU_EVENT_RIGHT, NULL);
//      ShowTextOnLCD("right");
    }
    if (EVNT_EventIsSetAutoClear(EVNT_LCD_BTN_UP)) { /* up */
      LCDMenu_OnEvent(LCDMENU_EVENT_UP, NULL);
//      ShowTextOnLCD("up");
    }
    if (EVNT_EventIsSetAutoClear(EVNT_LCD_BTN_DOWN)) { /* down */
      LCDMenu_OnEvent(LCDMENU_EVENT_DOWN, NULL);
//      ShowTextOnLCD("down");
    }
    if (EVNT_EventIsSetAutoClear(EVNT_LCD_BTN_CENTER)) { /* center */
      LCDMenu_OnEvent(LCDMENU_EVENT_ENTER, NULL);
//      ShowTextOnLCD("center");
    }
    if (EVNT_EventIsSetAutoClear(EVNT_LCD_SIDE_BTN_UP)) { /* side up */
      LCDMenu_OnEvent(LCDMENU_EVENT_UP, NULL);
//      ShowTextOnLCD("side up");
    }
    if (EVNT_EventIsSetAutoClear(EVNT_LCD_SIDE_BTN_DOWN)) { /* side down */
      LCDMenu_OnEvent(LCDMENU_EVENT_DOWN, NULL);
//      ShowTextOnLCD("side down");
    }
#endif
#endif /* PL_CONFIG_HAS_LCD_MENU */
    vTaskDelay(pdMS_TO_TICKS(20));
  }
}

void LCD_Deinit(void) {
#if PL_CONFIG_HAS_LCD_MENU
  LCDMenu_Deinit();
#endif
}

void LCD_Init(void) {
  LedBackLightisOn =  TRUE;
  if (xTaskCreate(LCD_Task, "LCD", 500/sizeof(StackType_t), NULL, tskIDLE_PRIORITY, &LCD_Task_Handle) != pdPASS) {
    for(;;){} /* error! probably out of memory */
  }
#if PL_CONFIG_HAS_LCD_MENU
  LCDMenu_Init();
#endif
}
#endif /* PL_CONFIG_HAS_LCD */
