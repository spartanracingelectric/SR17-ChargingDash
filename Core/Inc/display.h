// Written by Ayman Alamayri in Dec 2024
#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "graphics.h"
#include "ssd1306.h"
#include "ssd1306_fonts.h"

#define DISPLAY_HEADER_START_Y_PX 0
#define DISPLAY_HEADER_HEIGHT_PX 11
#define DISPLAY_HEADER_END_Y_PX (DISPLAY_HEADER_START_Y_PX + DISPLAY_HEADER_HEIGHT_PX - 1);

#define DISPLAY_CONTENT_START_PX (DISPLAY_HEADER_HEIGHT_PX)
#define DISPLAY_CONTENT_HEIGHT_PX (64 - DISPLAY_HEADER_HEIGHT_PX - DISPLAY_FOOTER_HEIGHT_PX)

#define DISPLAY_FOOTER_START_Y_PX 51
#define DISPLAY_FOOTER_HEIGHT_PX 13
#define DISPLAY_FOOTER_END_Y_PX (DISPLAY_FOOTER_START_Y_PX + DISPLAY_FOOTER_HEIGHT_PX - 1)

#define PROFILE_BOXES_PER_SCREEN 3

#define PROFILE_BOX_START_Y_PX (DISPLAY_CONTENT_START_PX + 2)
#define PROFILE_BOX_HEIGHT_PX 11

#define PROFILE_BOX_MARGIN_Y_PX 1
#define PROFILE_BOX_MARGIN_X_PX 1
#define PROFILE_BOX_PADDING_Y_PX 1
#define PROFILE_BOX_PADDING_X_PX 1

#define PROFILE_BOX_START_X_PX 1
#define PROFILE_BOX_END_X_PX 127

#define PROFILE_BOX_TEXT_START_Y_PX (PROFILE_BOX_START_Y_PX + PROFILE_BOX_PADDING_Y_PX + 1)
#define PROFILE_BOX_TEXT_START_X_PX (PROFILE_BOX_START_X_PX + PROFILE_BOX_PADDING_X_PX + 1)

#define VOLTAGE_STEP_V 5
#define CURRENT_STEP_A 1

typedef struct
{
	char name[5];
	uint16_t current;
	uint16_t voltage;
} profile;

typedef enum
{
	DISPLAY_STATE_NAVIGATION,
	DISPLAY_STATE_HOME,
	DISPLAY_STATE_CHARGING_PROFILES,
	DISPLAY_STATE_ADD_CHARGING_PROFILE,
	DISPLAY_STATE_CHARGING_CONFIRMATION,
	DISPLAY_STATE_CHARGING_INITIALIZATION,
	DISPLAY_STATE_START_BALANCING,
	DISPLAY_STATE_BALANCING_INITIALIZATION,
	DISPLAY_STATE_BATTERY_STATS_ONE,
	DISPLAY_STATE_BATTERY_STATS_TWO,
	DISPLAY_STATE_CHARGER_STATS,
	DISPLAY_STATE_IN_CHARGING_STATS_ONE,
	DISPLAY_STATE_IN_CHARGING_STATS_TWO,
	DISPLAY_STATE_ERRORS
} displayState;

extern displayState nextDisplayState;
extern displayState currentDisplayState;

void DISP_KanoaSplash(void);

void Display_init(void);
displayState Display_updateState(void);
displayState Display_displayNavigation(void);
displayState Display_displayHome(void);
displayState Display_displayErrors(void);
displayState Display_displayStartBalancing(void);
displayState Display_displayInChargingStatsOne(void);
displayState Display_displayInChargingStatsTwo(void);
displayState Display_displayChargingProfiles(void);
displayState Display_displayChargerStats(void);
displayState Display_displayBatteryStatsOne(void);
displayState Display_displayBatteryStatsTwo(void);
displayState Display_displayChargingInitialization(void);
displayState Display_displayBalancingInitialization(void);
displayState Display_displayAddChargingProfile(void);
displayState Display_displayChargingConfirmation(void);
void Display_drawNavBar(char *options[], int numberOfNavBarOptions, int firstNavBarOptionIndex, int selectedOption);
void Display_drawTitleBar(char title[]);
void Display_drawInChargingSymbol(int x, int y);
void Display_drawErrorSymbol(int x, int y);
void Display_drawShortScrollBar(int currentView, int numberOfViews);
void Display_drawLongScrollBar(int currentView, int numberOfViews);
void Display_clear(void);
void Display_forceI2CReset(void);
void Display_updateScreen(void);
void Display_handleUpDownPress(int *selectedOption, int numberOfOptions);


#endif
