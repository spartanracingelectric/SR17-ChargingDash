#include "display.h"
#include "charger.h"
#include "charging_profile.h"
#include "ssd1306.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>

profile allProfiles[] = {
	{"P1", 3, 525},
	{"P2", 4, 355},
	{"P3", 20, 355},
	{"P4", 3, 385},
	{"P5", 15, 385},
	{"P6", 20, 385},
	{"P7", 10, 401},
	{"P8", 20, 400},
	{"P9", 10, 403},
};

int numberOfProfiles = 9;

displayState currentDisplayState = DISPLAY_STATE_NAVIGATION;
displayState nextDisplayState = DISPLAY_STATE_NAVIGATION;
displayState previousDisplayState = DISPLAY_STATE_NAVIGATION;

ChargingProfile selectedChargingProfile = {0};

extern char codeBranch[10];
extern char codeVersion[5];

volatile uint32_t buttonInterruptCurrentTime = 0;
volatile uint32_t buttonInterruptPreviousTime = 0;

volatile bool upPressed = false;
volatile bool downPressed = false;
volatile bool selectPressed = false;
volatile bool backPressed = false;
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == BTN_UP_Pin || GPIO_Pin == BTN_DWN_Pin || GPIO_Pin == BTN_SEL_Pin || GPIO_Pin == BTN_BCK_Pin)
	{
		// TODO: fix debouncing
		buttonInterruptCurrentTime = HAL_GetTick();
		int debounceTimeThreshold = 200;
		int timeDifference = buttonInterruptCurrentTime - buttonInterruptPreviousTime;
		if (timeDifference > debounceTimeThreshold)
		{
			if (GPIO_Pin == BTN_UP_Pin)
			{
				upPressed = true;
				DEBUG_PRINT("BUTTON 1\n");
			}
			else if (GPIO_Pin == BTN_DWN_Pin)
			{
				downPressed = true;
				DEBUG_PRINT("BUTTON 2\n");
			}
			else if (GPIO_Pin == BTN_SEL_Pin)
			{
				selectPressed = true;
				DEBUG_PRINT("BUTTON 3\n");
			}
			else if (GPIO_Pin == BTN_BCK_Pin)
			{
				backPressed = true;
				DEBUG_PRINT("BUTTON 4\n");
			}
			buttonInterruptPreviousTime = buttonInterruptCurrentTime;
		}
	}
}

void DISP_KanoaSplash()
{
	ssd1306_Fill(Black);
	ssd1306_UpdateScreen();
	ssd1306_SetCursor(64, 15);
	ssd1306_WriteString("Elcon Control", Font_6x8, White);
	ssd1306_SetCursor(64, 25);
	ssd1306_WriteString("By Ayman A., et al", Font_6x8, White);
	ssd1306_SetCursor(64, 35);
	ssd1306_WriteString(codeVersion, Font_6x8, White);
	ssd1306_SetCursor(64, 45);
	ssd1306_WriteString(codeBranch, Font_6x8, White);
	ssd1306_UpdateScreen();
}

void Display_init(void)
{
	ssd1306_Init();
}

displayState Display_updateState(void)
{
	if (currentDisplayState != nextDisplayState)
	{
		selectPressed = false;
		upPressed = false;
		downPressed = false;
		backPressed = false;
		previousDisplayState = currentDisplayState;
		currentDisplayState = nextDisplayState;
	}

	switch (currentDisplayState)
	{
	case DISPLAY_STATE_NAVIGATION:
		nextDisplayState = Display_displayNavigation();
		break;
	case DISPLAY_STATE_HOME:
		nextDisplayState = Display_displayHome();
		break;
	case DISPLAY_STATE_CHARGING_PROFILES:
		nextDisplayState = Display_displayChargingProfiles();
		break;
	case DISPLAY_STATE_ADD_CHARGING_PROFILE:
		nextDisplayState = Display_displayAddChargingProfile();
		break;
	case DISPLAY_STATE_CHARGING_CONFIRMATION:
		nextDisplayState = Display_displayChargingConfirmation();
		break;
	case DISPLAY_STATE_CHARGING_INITIALIZATION:
		nextDisplayState = Display_displayChargingInitialization();
		break;
	case DISPLAY_STATE_START_BALANCING:
		nextDisplayState = Display_displayStartBalancing();
		break;
	case DISPLAY_STATE_BALANCING_INITIALIZATION:
		nextDisplayState = Display_displayBalancingInitialization();
		break;
	case DISPLAY_STATE_BATTERY_STATS_ONE:
		nextDisplayState = Display_displayBatteryStatsOne();
		break;
	case DISPLAY_STATE_BATTERY_STATS_TWO:
		nextDisplayState = Display_displayBatteryStatsTwo();
		break;
	case DISPLAY_STATE_CHARGER_STATS:
		nextDisplayState = Display_displayChargerStats();
		break;
	case DISPLAY_STATE_IN_CHARGING_STATS_ONE:
		nextDisplayState = Display_displayInChargingStatsOne();
		break;
	case DISPLAY_STATE_IN_CHARGING_STATS_TWO:
		nextDisplayState = Display_displayInChargingStatsTwo();
		break;
	case DISPLAY_STATE_ERRORS:
		nextDisplayState = Display_displayErrors();
		break;
	}

	previousDisplayState = currentDisplayState; // TODO: idk
	return nextDisplayState;
}

void Display_clear(void)
{
	ssd1306_FillRectangle(0, 0, 127, 63, Black);
}

displayState Display_displayNavigation(void)
{
	static int selectedOption = 0;
	// Reset screen when entering for first time
	if (previousDisplayState != DISPLAY_STATE_NAVIGATION)
	{
		selectedOption = 0;
	}
	static char *options[] = {"Home", "Charging", "Balancing", "Battery", "Charger Stats", "Errors", "Restart"};
	int numberOfOptions = 7;
	int y1 = 15;
	int y2 = 13;
	int y3 = 24;
	int currentView = selectedOption / 4;
	int startIndex = currentView * 4;

	Display_handleUpDownPress(&selectedOption, numberOfOptions);
	Display_clear();
	Display_drawTitleBar("Navigation");

	for (int i = startIndex; i < startIndex + 4 && i < numberOfOptions; i++)
	{
		ssd1306_SetCursor(3, y1);
		if (selectedOption == i)
		{
			ssd1306_FillRectangle(1, y2, 122, y3, White);
			ssd1306_WriteString(options[i], Font_6x8, Black);
		}
		else
		{
			ssd1306_DrawRectangle(1, y2, 122, y3, White);
			ssd1306_WriteString(options[i], Font_6x8, White);
		}

		y1 = y1 + 13;
		y2 = y2 + 13;
		y3 = y2 + 10;
	}

	int numberOfViews = (numberOfOptions + 3) / 4; // 3 options per view, rounds up to ensure there is enough views
	Display_drawLongScrollBar(currentView, numberOfViews);

	Display_updateScreen();

	if (selectPressed)
	{
		selectPressed = false;
		switch (selectedOption)
		{
		case 0:
			return DISPLAY_STATE_HOME;
		case 1:
			return DISPLAY_STATE_CHARGING_PROFILES;
		case 2:
			return DISPLAY_STATE_START_BALANCING;
		case 3:
			return DISPLAY_STATE_BATTERY_STATS_ONE;
		case 4:
			return DISPLAY_STATE_CHARGER_STATS;
		case 5:
			return DISPLAY_STATE_ERRORS;
		case 6:
			NVIC_SystemReset();
		}
	}
	return DISPLAY_STATE_NAVIGATION;
}

void Display_forceI2CReset()
{
	DEBUG_PRINT("Forcing I2C reset...\n");

	// Abort any ongoing operations
	HAL_I2C_Master_Abort_IT(&hi2c2, SSD1306_I2C_ADDR);
	HAL_Delay(10);

	// Force reset I2C peripheral
	__HAL_RCC_I2C2_FORCE_RESET();
	HAL_Delay(10);
	__HAL_RCC_I2C2_RELEASE_RESET();
	HAL_Delay(10);

	// Reinitialize I2C
	MX_I2C2_Init();
	HAL_Delay(10);

	// Reinitialize display
	ssd1306_Init();

	DEBUG_PRINT("I2C reset complete\n");
}

void Display_updateScreen()
{
	ssd1306_UpdateScreen();
	HAL_StatusTypeDef status = ssd1306_Custom_GetLastStatus();
	if (status != HAL_OK)
	{
		Display_forceI2CReset();
	}
}

// TODO: FINISH
displayState Display_displayHome(void)
{
	static int selectedOption = 0;
	if (previousDisplayState != DISPLAY_STATE_HOME)
	{
		selectedOption = 0;
	}
	char stateOfCharge[50];
	char balancingStatus[50];

	// TODO READ CHARGER TEMP
	char chargerTemp[] = "Charger Tmp: N/A";
	int numberOfOptions = 1;

	Display_handleUpDownPress(&selectedOption, numberOfOptions);

	Display_clear();
	Display_drawTitleBar("Home");

	ssd1306_SetCursor(1, 13);
	ssd1306_WriteString(stateOfCharge, Font_6x8, White);

	ssd1306_SetCursor(1, 23);
	ssd1306_WriteString(chargerTemp, Font_6x8, White);

	ssd1306_SetCursor(1, 33);
	ssd1306_WriteString(balancingStatus, Font_6x8, White);

	char *navBarOptions[] = {"Nav"};
	int firstNavBarOptionIndex = 0;
	Display_drawNavBar(navBarOptions, numberOfOptions, firstNavBarOptionIndex, selectedOption);

	Display_updateScreen();

	if (selectPressed)
	{
		selectPressed = false;
		switch (selectedOption)
		{
		case 0:
			return DISPLAY_STATE_NAVIGATION;
		}
	}
	return DISPLAY_STATE_HOME;
}

void Display_drawNavBar(char *options[], int numberOfNavBarOptions, int firstNavBarOptionIndex, int selectedOption)
{

	int navBarOptionIndex = firstNavBarOptionIndex;

	int x1 = 1;
	int x2 = 1;

	for (int i = 0; i < numberOfNavBarOptions; i++)
	{
		x2 = x1 + (strlen(options[i]) * 6) + 2;
		if (selectedOption == navBarOptionIndex)
		{
			ssd1306_FillRectangle(x1, 52, x2, 62, White);
			ssd1306_SetCursor(x1 + 2, 54);
			ssd1306_WriteString(options[i], Font_6x8, Black);
		}
		else
		{
			ssd1306_DrawRectangle(x1, 52, x2, 62, White);
			ssd1306_SetCursor(x1 + 2, 54);
			ssd1306_WriteString(options[i], Font_6x8, White);
		}

		navBarOptionIndex++;
		x1 = x2 + 2;
	}
}

// TODO: Title should only display Balancing if charger state is balancing, or it should show BAL symbol if auto
// balancing in charger state
displayState Display_displayInChargingStatsTwo(void)
{
	static int selectedOption = 0;
	if (previousDisplayState != DISPLAY_STATE_IN_CHARGING_STATS_TWO)
	{
		selectedOption = 0;
	}
	if (currentChargerState != CHARGER_STATE_CHARGING && currentChargerState != CHARGER_STATE_BALANCING)
	{
		return DISPLAY_STATE_NAVIGATION;
	}

	int numberOfOptions = 1;

	char sumOfCells[50];
	char stateOfCharge[50];
	char averageStats[50];
	char chargingInfo[50];

	sprintf(averageStats, "Avg V:%d.%03dV", currentBmsAndElconData.BMS_averageCellVoltage_mV / 1000, currentBmsAndElconData.BMS_averageCellVoltage_mV % 1000);
	sprintf(chargingInfo, "%df V @ %d A", (int16_t)LIMIT_VOLTS, (int16_t)LIMIT_AMPS); // TODO: CHECK, using float with %d
	sprintf(stateOfCharge, "SOC:%d.%02d%%", currentBmsAndElconData.BMS_stateOfCharge / 100, currentBmsAndElconData.BMS_stateOfCharge % 100);
	sprintf(sumOfCells, "Pack Volt: %d.%02dV", currentBmsAndElconData.BMS_sumPackVoltage_cV / 100, currentBmsAndElconData.BMS_sumPackVoltage_cV % 100);

	Display_clear();
	Display_handleUpDownPress(&selectedOption, numberOfOptions);

	bool isBalancing = (currentChargerState == CHARGER_STATE_BALANCING);

	Display_drawTitleBar(isBalancing ? "Balancing 2" : "Charging 2");

	ssd1306_SetCursor(1, 13);
	ssd1306_WriteString(sumOfCells, Font_6x8, White);

	// Writes SOC Stats
	ssd1306_SetCursor(1, 23);
	ssd1306_WriteString(stateOfCharge, Font_6x8, White);

	ssd1306_SetCursor(1, 33);
	ssd1306_WriteString(averageStats, Font_6x8, White);

	ssd1306_SetCursor(1, 43);
	ssd1306_WriteString(chargingInfo, Font_6x8, White);

	char *navBarOptions[1] = {isBalancing ? "Balancing 1" : "Charging 1"};
	int firstNavBarOptonIndex = 0;
	Display_drawNavBar(navBarOptions, numberOfOptions, firstNavBarOptonIndex, selectedOption);
	Display_updateScreen();

	if (selectPressed)
	{
		selectPressed = false;
		switch (selectedOption)
		{
		case 0:
			return DISPLAY_STATE_IN_CHARGING_STATS_ONE;
		}
	}
	return DISPLAY_STATE_IN_CHARGING_STATS_TWO;
}

displayState Display_displayChargingProfiles(void)
{
	static int selectedOption = 0;
	static int currentPage = 0;
	if (previousDisplayState != DISPLAY_STATE_CHARGING_PROFILES)
	{
		selectedOption = 0;
		currentPage = 0;
	}

	int numAvailableProfiles = ChargingProfile_updateAvailableProfiles();
	int numPages = (numAvailableProfiles + (PROFILE_BOXES_PER_SCREEN - 1)) / PROFILE_BOXES_PER_SCREEN;

	if (currentPage >= numPages)
	{
		currentPage = 0;
	}

	Display_clear();
	Display_drawTitleBar("SELECT PROFILE");

	int profileBoxStartY = PROFILE_BOX_START_Y_PX;
	int profileBoxEndY = PROFILE_BOX_START_Y_PX + PROFILE_BOX_HEIGHT_PX;
	int profileBoxTextStartY = PROFILE_BOX_TEXT_START_Y_PX;

	// Display up to three profiles per page
	int numProfilesOnCurrentPage = 0;
	int pageStartIndex = currentPage * PROFILE_BOXES_PER_SCREEN;

	for (int boxIndex = 0; boxIndex < PROFILE_BOXES_PER_SCREEN; boxIndex++)
	{
		int profileIndex = pageStartIndex + boxIndex;

		if (profileIndex >= numAvailableProfiles)
		{
			break;
		}

		numProfilesOnCurrentPage++;

		char profileString[50];
		sprintf(profileString, "P%d: %dV %dA", profileIndex + 1, availableProfiles[profileIndex].voltageCommand_V, availableProfiles[profileIndex].currentCommand_A);
		ssd1306_SetCursor(PROFILE_BOX_TEXT_START_X_PX, profileBoxTextStartY);

		if (selectedOption == boxIndex)
		{
			ssd1306_FillRectangle(PROFILE_BOX_START_X_PX, profileBoxStartY, PROFILE_BOX_END_X_PX, profileBoxEndY, White);
			ssd1306_WriteString(profileString, Font_6x8, Black);
		}
		else
		{
			ssd1306_DrawRectangle(PROFILE_BOX_START_X_PX, profileBoxStartY, PROFILE_BOX_END_X_PX, profileBoxEndY, White);
			ssd1306_WriteString(profileString, Font_6x8, White);
		}

		profileBoxStartY += PROFILE_BOX_HEIGHT_PX + 2;
		profileBoxEndY += PROFILE_BOX_HEIGHT_PX + 2;
		profileBoxTextStartY += PROFILE_BOX_HEIGHT_PX + 2;
	}

	int navBarStartIndex = numProfilesOnCurrentPage;
	int numNavBarOptions = 3;
	int numOptions = numProfilesOnCurrentPage + numNavBarOptions;
	Display_handleUpDownPress(&selectedOption, numOptions);

	char *navBarOptions[] = {"NEXT", "ADD", "MENU"};
	Display_drawNavBar(navBarOptions, numNavBarOptions, navBarStartIndex, selectedOption);

	Display_updateScreen();

	bool selectingProfile = (selectedOption >= 0 && selectedOption < navBarStartIndex);
	bool selectingNextPage = (selectedOption == navBarStartIndex);
	bool selectingAddProfile = (selectedOption == navBarStartIndex + 1);
	bool selectingNav = (selectedOption == navBarStartIndex + 2);

	if (selectPressed)
	{
		selectPressed = false;
		if (selectingProfile)
		{
			int profileIndex = pageStartIndex + selectedOption;
			if (profileIndex < numAvailableProfiles)
			{
				selectedChargingProfile = availableProfiles[profileIndex];
				return DISPLAY_STATE_CHARGING_CONFIRMATION;
			}
		}
		else if (selectingNextPage)
		{
			if (numPages > 0)
			{
				currentPage++;
				if (currentPage >= numPages)
				{
					currentPage = 0;
				}
			}

			int newPageStartIndex = currentPage * PROFILE_BOXES_PER_SCREEN;
			int newProfilesOnPage = numAvailableProfiles - newPageStartIndex;
			if (newProfilesOnPage > PROFILE_BOXES_PER_SCREEN)
			{
				newProfilesOnPage = PROFILE_BOXES_PER_SCREEN;
			}
			selectedOption = newProfilesOnPage; // keep highlight on NEXT
		}
		else if (selectingAddProfile)
		{
			return DISPLAY_STATE_ADD_CHARGING_PROFILE;
		}
		else if (selectingNav)
		{
			return DISPLAY_STATE_NAVIGATION;
		}
	}
	return DISPLAY_STATE_CHARGING_PROFILES;
}

displayState Display_displayAddChargingProfile(void)
{
	static int selectedOption = 0;
	static int selectedVoltage_V = 500;
	static int selectedCurrent_A = 3;
	static bool isEditing = false;

	if (previousDisplayState != DISPLAY_STATE_ADD_CHARGING_PROFILE)
	{
		selectedOption = 0;
		selectedVoltage_V = 500;
		selectedCurrent_A = 3;
		isEditing = false;
	}
	int numOptions = 4;

	char voltageString[30];
	char currentString[30];
	sprintf(voltageString, "Voltage: %dV", selectedVoltage_V);
	sprintf(currentString, "Current: %dA", selectedCurrent_A);

	if (!isEditing)
	{
		Display_handleUpDownPress(&selectedOption, numOptions);
	}

	Display_clear();
	Display_drawTitleBar("ADD PROFILE");

	// Voltage row
	if (selectedOption == 0)
	{
		ssd1306_FillRectangle(1, 13, 122, 24, White);
		ssd1306_SetCursor(3, 15);
		ssd1306_WriteString(voltageString, Font_6x8, Black);

		if (isEditing)
		{
			ssd1306_SetCursor(96, 15);
			ssd1306_WriteString("EDIT", Font_6x8, Black);
		}
	}
	else
	{
		ssd1306_DrawRectangle(1, 13, 122, 24, White);
		ssd1306_SetCursor(3, 15);
		ssd1306_WriteString(voltageString, Font_6x8, White);
	}

	// Current row
	if (selectedOption == 1)
	{
		ssd1306_FillRectangle(1, 26, 122, 37, White);
		ssd1306_SetCursor(3, 28);
		ssd1306_WriteString(currentString, Font_6x8, Black);

		if (isEditing)
		{
			ssd1306_SetCursor(96, 28);
			ssd1306_WriteString("EDIT", Font_6x8, Black);
		}
	}
	else
	{
		ssd1306_DrawRectangle(1, 26, 122, 37, White);
		ssd1306_SetCursor(3, 28);
		ssd1306_WriteString(currentString, Font_6x8, White);
	}

	char *navBarOptions[] = {"Add", "Cancel"};
	int navBarStartIndex = 2;
	Display_drawNavBar(navBarOptions, 2, navBarStartIndex, selectedOption);

	Display_updateScreen();

	if (isEditing)
	{
		if (upPressed)
		{
			if (selectedOption == 0)
			{
				selectedVoltage_V += VOLTAGE_STEP_V;
				if (selectedVoltage_V > ELCON_MAX_VOLTAGE_V)
				{
					selectedVoltage_V = ELCON_MAX_VOLTAGE_V;
				}
			}
			else if (selectedOption == 1)
			{
				selectedCurrent_A += CURRENT_STEP_A;
				if (selectedCurrent_A > ELCON_MAX_CURRENT_A)
				{
					selectedCurrent_A = ELCON_MAX_CURRENT_A;
				}
			}
			upPressed = false;
		}

		if (downPressed)
		{
			if (selectedOption == 0)
			{
				selectedVoltage_V -= VOLTAGE_STEP_V;
				if (selectedVoltage_V < ELCON_MIN_VOLTAGE_V)
				{
					selectedVoltage_V = ELCON_MIN_VOLTAGE_V;
				}
			}
			else if (selectedOption == 1)
			{
				selectedCurrent_A -= CURRENT_STEP_A;
				if (selectedCurrent_A < ELCON_MIN_CURRENT_A)
				{
					selectedCurrent_A = ELCON_MIN_CURRENT_A;
				}
			}
			downPressed = false;
		}

		if (selectPressed)
		{
			isEditing = false;
			selectPressed = false;
		}
	}

	if (selectPressed)
	{
		selectPressed = false;
		switch (selectedOption)
		{
		case 0:
		case 1:
			isEditing = true;
			break;
		case 2:
			ChargingProfile_addProfile(selectedCurrent_A, selectedVoltage_V);
			isEditing = false;
			selectedOption = 0;
			return DISPLAY_STATE_CHARGING_PROFILES;
		case 3:
			isEditing = false;
			selectedOption = 0;
			return DISPLAY_STATE_CHARGING_PROFILES;
		}
	}
	return DISPLAY_STATE_ADD_CHARGING_PROFILE;
}

displayState Display_displayChargingConfirmation(void)
{
	static int selectedOption = 0;
	if (previousDisplayState != DISPLAY_STATE_CHARGING_CONFIRMATION)
	{
		selectedOption = 0;
	}
	// Only display delete for non-default profiles
	char *navBarOptions[3] = {"START", selectedChargingProfile.isDeletable ? "DELETE" : "BACK", selectedChargingProfile.isDeletable ? "BACK" : ""};
	int numOptions = selectedChargingProfile.isDeletable ? 3 : 2;
	Display_handleUpDownPress(&selectedOption, numOptions);

	Display_clear();
	Display_drawTitleBar("SELECTED PROFILE");

	char voltageString[30];
	char currentString[30];
	sprintf(voltageString, "Voltage: %dV", selectedChargingProfile.voltageCommand_V);
	sprintf(currentString, "Current: %dA", selectedChargingProfile.currentCommand_A);

	ssd1306_DrawRectangle(1, 13, 122, 24, White);
	ssd1306_SetCursor(3, 15);
	ssd1306_WriteString(voltageString, Font_6x8, White);

	ssd1306_DrawRectangle(1, 26, 122, 37, White);
	ssd1306_SetCursor(3, 28);
	ssd1306_WriteString(currentString, Font_6x8, White);

	int navBarStartIndex = 0;
	Display_drawNavBar(navBarOptions, numOptions, navBarStartIndex, selectedOption);
	Display_updateScreen();

	if (selectPressed)
	{
		selectPressed = false;
		if (selectedOption == 0)
		{
			LIMIT_VOLTS = selectedChargingProfile.voltageCommand_V;
			LIMIT_AMPS = selectedChargingProfile.currentCommand_A;
			return DISPLAY_STATE_CHARGING_INITIALIZATION;
		}
		else if (selectedChargingProfile.isDeletable)
		{
			if (selectedOption == 1)
			{
				ChargingProfile_deleteProfileByValue(selectedChargingProfile.currentCommand_A, selectedChargingProfile.voltageCommand_V);
				selectedChargingProfile.voltageCommand_V = 0;
				selectedChargingProfile.currentCommand_A = 0;
				selectedChargingProfile.isDeletable = false;
				return DISPLAY_STATE_CHARGING_PROFILES;
			}

			if (selectedOption == 2)
			{
				selectedChargingProfile.voltageCommand_V = 0;
				selectedChargingProfile.currentCommand_A = 0;
				selectedChargingProfile.isDeletable = false;
				return DISPLAY_STATE_CHARGING_PROFILES;
			}
		}
		else
		{
			if (selectedOption == 1)
			{
				selectedChargingProfile.voltageCommand_V = 0;
				selectedChargingProfile.currentCommand_A = 0;
				selectedChargingProfile.isDeletable = false;
				return DISPLAY_STATE_CHARGING_PROFILES;
			}
		}
	}
	return DISPLAY_STATE_CHARGING_CONFIRMATION;
}

void Display_handleUpDownPress(int *selectedOption, int numberOfOptions)
{
	if (upPressed)
	{
		(*selectedOption)--;
		upPressed = false;
	}

	if (downPressed)
	{
		(*selectedOption)++;
		downPressed = false;
	}

	// wrap
	if (*selectedOption < 0)
	{
		*selectedOption = numberOfOptions - 1;
	}

	if (*selectedOption >= numberOfOptions)
	{
		*selectedOption = 0;
	}
}

displayState Display_displayChargingInitialization(void)
{
	Display_clear();

	if (!Charger_isChargerSafe())
	{
		ssd1306_SetCursor(5, 5);
		ssd1306_WriteString("HVIL ERROR", Font_6x8, White);
		Display_updateScreen();
	}
	else if (!Charger_isHvilSwitchFlipped())
	{
		ssd1306_SetCursor(5, 5);
		ssd1306_WriteString("PLEASE FLIP HV", Font_6x8, White);
		Display_updateScreen();
	}
	else if (!Charger_isReadyToChargeSwitchFlipped())
	{
		ssd1306_SetCursor(5, 5);
		ssd1306_WriteString("PLEASE FLIP RTC", Font_6x8, White);
		Display_updateScreen();
	}
	else
	{
		currentChargerState = CHARGER_STATE_CHARGING;
		return DISPLAY_STATE_IN_CHARGING_STATS_ONE;
	}
	return DISPLAY_STATE_CHARGING_INITIALIZATION;
}

void Display_drawShortScrollBar(int currentView, int numberOfViews)
{
	// current_view is zero-indexed
	if (numberOfViews > 1)
	{
		int scrollContainerHeight = 35;
		int scrollBarLength = scrollContainerHeight / numberOfViews;
		int scrollBarStart = 14 + (currentView * scrollBarLength);

		ssd1306_DrawRectangle(124, 13, 126, 47, White);
		ssd1306_Line(125, scrollBarStart, 125, scrollBarStart + scrollBarLength, White);
	}
}

void Display_drawLongScrollBar(int currentView, int numberOfViews)
{
	// currentScreen is zero-indexed
	if (numberOfViews > 1)
	{
		int scrollContainerHeight = 49;
		int scrollBarLength = scrollContainerHeight / numberOfViews;
		int scrollBarStart = 14 + (currentView * numberOfViews);

		ssd1306_DrawRectangle(124, 13, 126, 62, White);
		ssd1306_Line(125, scrollBarStart, 125, scrollBarStart + scrollBarLength, White);
	}
}
// TODO: This screen is pretty useless
displayState Display_displayChargerStats(void)
{
	static int selectedOption = 0;
	if (previousDisplayState != DISPLAY_STATE_CHARGER_STATS)
	{
		selectedOption = 0;
	}
	char inletTempString[50];
	char outletTempString[50];
	int numberOfOptions = 1;

	Display_clear();
	Display_handleUpDownPress(&selectedOption, numberOfOptions);

	sprintf(inletTempString, "Inlet Tmp: N/A");
	sprintf(outletTempString, "Outlet Tmp: N/A");

	Display_drawTitleBar("Charger Stats");

	ssd1306_SetCursor(1, 13);
	ssd1306_WriteString(inletTempString, Font_6x8, White);

	ssd1306_SetCursor(1, 23);
	ssd1306_WriteString(outletTempString, Font_6x8, White);

	char *navBarOptions[] = {"Nav"};
	int numberOfNavBarOptions = 1;
	int firstNavBarOptionIndex = 0;
	Display_drawNavBar(navBarOptions, numberOfNavBarOptions, firstNavBarOptionIndex, selectedOption);

	Display_updateScreen();

	if (selectPressed)
	{
		selectPressed = false;
		switch (selectedOption)
		{
		case 0:
			return DISPLAY_STATE_NAVIGATION;
		}
	}
	return DISPLAY_STATE_CHARGER_STATS;
}

displayState Display_displayBatteryStatsOne(void)
{

	static int selectedOption = 0;
	if (previousDisplayState != DISPLAY_STATE_BATTERY_STATS_ONE)
	{
		selectedOption = 0;
	}

	char temperatureStats[50];
	char voltageStats[50];
	char averageStats[50];
	int numberOfOptions = 2;

	sprintf(temperatureStats, "Tmp H/L:%d/%dC", currentBmsAndElconData.BMS_maxTemp_C, currentBmsAndElconData.BMS_minTemp_C);
	sprintf(voltageStats, "Vlt H/L:%d.%03d/%d.%03dV", currentBmsAndElconData.BMS_maxCellVoltage_mV / 1000, abs(currentBmsAndElconData.BMS_maxCellVoltage_mV % 1000), currentBmsAndElconData.BMS_minCellVoltage_mV / 1000, abs(currentBmsAndElconData.BMS_minCellVoltage_mV % 1000));
	sprintf(averageStats, "Avg V:%d.%03dV", currentBmsAndElconData.BMS_averageCellVoltage_mV / 1000, abs(currentBmsAndElconData.BMS_averageCellVoltage_mV % 1000));

	Display_clear();
	Display_handleUpDownPress(&selectedOption, numberOfOptions);

	Display_drawTitleBar("Battery 1");

	ssd1306_SetCursor(1, 13);
	ssd1306_WriteString(temperatureStats, Font_6x8, White);

	ssd1306_SetCursor(1, 23);
	ssd1306_WriteString(voltageStats, Font_6x8, White);

	ssd1306_SetCursor(1, 33);
	ssd1306_WriteString(averageStats, Font_6x8, White);

	char *navBarOptions[] = {"Nav", "Battery 2"};
	int firstNavBarOptionIndex = 0;

	Display_drawNavBar(navBarOptions, numberOfOptions, firstNavBarOptionIndex, selectedOption);

	Display_updateScreen();

	if (selectPressed)
	{
		selectPressed = false;
		switch (selectedOption)
		{
		case 0:
			return DISPLAY_STATE_NAVIGATION;
		case 1:
			return DISPLAY_STATE_BATTERY_STATS_TWO;
		}
	}
	return DISPLAY_STATE_BATTERY_STATS_ONE;
}

displayState Display_displayBatteryStatsTwo(void)
{
	static int selectedOption = 0;
	if (previousDisplayState != DISPLAY_STATE_BATTERY_STATS_TWO)
	{
		selectedOption = 0;
	}
	char stateOfCharge[50];
	char packVolt[50];
	char packImbalance[50];
	int numberOfOptions = 2;

	sprintf(stateOfCharge, "SOC:%d.%02d%%", currentBmsAndElconData.BMS_stateOfCharge / 100, abs(currentBmsAndElconData.BMS_stateOfCharge % 100));
	sprintf(packImbalance, "Imbalance:%d.%03dV", currentBmsAndElconData.BMS_packImbalance_mV / 1000, abs(currentBmsAndElconData.BMS_packImbalance_mV % 1000));
	sprintf(packVolt, "Pack Volt:%d.%02dV", currentBmsAndElconData.BMS_sumPackVoltage_cV / 100, abs(currentBmsAndElconData.BMS_sumPackVoltage_cV % 100));

	Display_clear();
	Display_handleUpDownPress(&selectedOption, numberOfOptions);

	Display_drawTitleBar("Battery 2");

	ssd1306_SetCursor(1, 13);
	ssd1306_WriteString(stateOfCharge, Font_6x8, White);

	ssd1306_SetCursor(1, 23);
	ssd1306_WriteString(packImbalance, Font_6x8, White);

	ssd1306_SetCursor(1, 33);
	ssd1306_WriteString(packVolt, Font_6x8, White);

	char *navBarOptions[] = {"Nav", "Battery 1"};
	int firstNavBarOptionIndex = 0;
	Display_drawNavBar(navBarOptions, numberOfOptions, firstNavBarOptionIndex, selectedOption);

	Display_updateScreen();

	if (selectPressed)
	{
		selectPressed = false;
		switch (selectedOption)
		{
		case 0:
			return DISPLAY_STATE_NAVIGATION;
		case 1:
			return DISPLAY_STATE_BATTERY_STATS_ONE;
		}
	}
	return DISPLAY_STATE_BATTERY_STATS_TWO;
}

// TODO: Fix logic to display if its already balancing
displayState Display_displayStartBalancing(void)
{
	static int selectedOption = 0;
	if (previousDisplayState != DISPLAY_STATE_START_BALANCING)
	{
		selectedOption = 0;
	}
	int numberOfOptions = 2;

	Display_clear();
	Display_handleUpDownPress(&selectedOption, numberOfOptions);

	Display_drawTitleBar("Start Balancing");

	ssd1306_SetCursor(1, 13);
	ssd1306_WriteString("Balancing is off", Font_6x8, White);

	char *navBarOptions[] = {"Nav", "Start Bal"};
	int navBarStartIndex = 0;
	Display_drawNavBar(navBarOptions, numberOfOptions, navBarStartIndex, selectedOption);

	Display_updateScreen();

	if (selectPressed)
	{
		selectPressed = false;
		switch (selectedOption)
		{
		case 0:
			return DISPLAY_STATE_NAVIGATION;
		case 1:
		{
			return DISPLAY_STATE_BALANCING_INITIALIZATION;
		}
		}
	}
	return DISPLAY_STATE_START_BALANCING;
}

displayState Display_displayBalancingInitialization(void)
{
	Display_clear();
	if (!Charger_isChargerSafe())
	{
		ssd1306_SetCursor(5, 5);
		ssd1306_WriteString("HVIL ERROR", Font_6x8, White);
		Display_updateScreen();
	}
	else if (!Charger_isReadyToChargeSwitchFlipped())
	{
		ssd1306_SetCursor(5, 5);
		ssd1306_WriteString("PLEASE FLIP RTC", Font_6x8, White);
		Display_updateScreen();
	}
	else
	{
		currentChargerState = CHARGER_STATE_BALANCING;
		return DISPLAY_STATE_IN_CHARGING_STATS_ONE;
	}
	return DISPLAY_STATE_BALANCING_INITIALIZATION;
}

// TODO: Finish
void Display_drawTitleBar(char title[])
{
	bool isFault = false;
	for (int i = 0; i < 5; i++)
	{
		if (currentBmsAndElconData.ELCON_fault[i] == 1)
		{
			isFault = true;
			break;
		}
	}
	for (int i = 0; i < 5; i++)
	{
		if (currentBmsAndElconData.BMS_fault[i] == 1)
		{
			isFault = true;
			break;
		}
	}

	ssd1306_SetCursor(1, 1);
	ssd1306_WriteString(title, Font_6x8, White);
	ssd1306_Line(0, 10, 127, 10, White);

	// Flashing status symbols
	//  ssd1306_FillRectangle(70, 0, 127, 9, Black);
	//  ssd1306_UpdateScreen();
	bool isCharging =
		currentChargerState != CHARGER_STATE_IDLE &&
		(currentChargingMode == CHARGING_MODE_CONSTANT_CURRENT || currentChargingMode == CHARGING_MODE_CURRENT_TAPER ||
		 currentChargingMode == CHARGING_MODE_MAINTENANCE);
	bool isBalancing = currentBmsAndElconData.BMS_balanceStatus;
	if (isFault)
	{
		Display_drawErrorSymbol(119, 1);
		if (isCharging)
		{
			Display_drawInChargingSymbol(92, 3);
		}
		else if (isBalancing)
		{
			ssd1306_FillRectangle(91, 0, 109, 8, White);
			ssd1306_SetCursor(92, 1);
			ssd1306_WriteString("BAL", Font_6x8, Black);
		}
	}
	else if (isCharging)
	{
		Display_drawInChargingSymbol(109, 3);
	}
	else if (isBalancing)
	{
		ssd1306_FillRectangle(108, 0, 126, 8, White);
		ssd1306_SetCursor(109, 1);
		ssd1306_WriteString("BAL", Font_6x8, Black);
	}
}

void Display_drawInChargingSymbol(int x, int y)
{
	// point of origin (x,y) is the top left of battery
	ssd1306_Line(x, y, x + 4, y, White);
	ssd1306_Line(x, y, x, y + 4, White);
	ssd1306_Line(x, y + 4, x + 4, y + 4, White);

	ssd1306_Line(x + 12, y, x + 16, y, White);
	ssd1306_Line(x + 16, y, x + 16, y + 4, White);
	ssd1306_Line(x + 12, y + 4, x + 16, y + 4, White);
	ssd1306_Line(x + 17, y + 1, x + 17, y + 3, White);

	ssd1306_Line(x + 6, y + 2, x + 10, y + 2, White);
	ssd1306_Line(x + 6, y + 2, x + 9, y - 1, White);
	ssd1306_Line(x + 10, y + 2, x + 7, y + 5, White);
}

void Display_drawErrorSymbol(int x, int y)
{
	// point of origin (x,y) is the top of the triangle
	ssd1306_Line(x, y, x + 7, y + 7, White);
	ssd1306_Line(x, y, x - 7, y + 7, White);
	ssd1306_Line(x - 7, y + 7, x + 7, y + 7, White);

	ssd1306_Line(x, y + 2, x, y + 4, White);
	ssd1306_Line(x, y + 6, x, y + 6, White);
}

// TODO: Title should only display Balancing if charger state is balancing, or it should show BAL symbol if auto
// balancing in charger state
displayState Display_displayInChargingStatsOne(void)
{
	static int selectedOption = 0;
	if (previousDisplayState != DISPLAY_STATE_IN_CHARGING_STATS_ONE)
	{
		selectedOption = 0;
	}
	if (currentChargerState != CHARGER_STATE_CHARGING && currentChargerState != CHARGER_STATE_BALANCING)
	{
		return DISPLAY_STATE_NAVIGATION;
	}
	int numberOfOptions = 1;
	char temperatureStats[50];
	char voltageStats[50];
	char imbalance[30];
	char outputStats[50];

	sprintf(voltageStats, "Vlt H/L:%d.%03d/%d.%03dV",
			currentBmsAndElconData.BMS_maxCellVoltage_mV / 1000,
			abs(currentBmsAndElconData.BMS_maxCellVoltage_mV % 1000),
			currentBmsAndElconData.BMS_minCellVoltage_mV / 1000,
			abs(currentBmsAndElconData.BMS_minCellVoltage_mV % 1000));

	sprintf(temperatureStats, "Tmp H/L:%d/%dC",
			currentBmsAndElconData.BMS_maxTemp_C,
			currentBmsAndElconData.BMS_minTemp_C);

	sprintf(imbalance, "Imbal:%d.%03dV",
			currentBmsAndElconData.BMS_packImbalance_mV / 1000,
			abs(currentBmsAndElconData.BMS_packImbalance_mV % 1000));

	sprintf(outputStats, "Out V/C:%d.%01dV/%d.%01dA",
			currentBmsAndElconData.ELCON_outputVoltage_dV / 10,
			abs(currentBmsAndElconData.ELCON_outputVoltage_dV % 10),

			currentBmsAndElconData.ELCON_outputCurrent_dA / 10,
			abs(currentBmsAndElconData.ELCON_outputCurrent_dA % 10));

	Display_clear();
	Display_handleUpDownPress(&selectedOption, numberOfOptions);

	bool isBalancing = (currentChargerState == CHARGER_STATE_BALANCING);
	// Writes title
	Display_drawTitleBar(isBalancing ? "Balancing 1" : "Charging 1");

	// Writes voltage
	ssd1306_SetCursor(1, 13);
	ssd1306_WriteString(voltageStats, Font_6x8, White);

	ssd1306_SetCursor(1, 23);
	ssd1306_WriteString(temperatureStats, Font_6x8, White);

	ssd1306_SetCursor(1, 33);
	ssd1306_WriteString(imbalance, Font_6x8, White);
	// Writes output info
	if (!isBalancing)
	{
		ssd1306_SetCursor(1, 43);
		ssd1306_WriteString(outputStats, Font_6x8, White);
	}

	char *navBarOptions[1] = {isBalancing ? "Balancing 2" : "Charging 2"};

	int firstNavBarOptionIndex = 0;
	Display_drawNavBar(navBarOptions, numberOfOptions, firstNavBarOptionIndex, selectedOption);
	Display_updateScreen();

	if (selectPressed)
	{
		selectPressed = false;
		switch (selectedOption)
		{
		case 0:
			return DISPLAY_STATE_IN_CHARGING_STATS_TWO;
		}
	}
	return DISPLAY_STATE_IN_CHARGING_STATS_ONE;
}

displayState Display_displayErrors(void)
{
	static int selectedOption = 0;
	if (previousDisplayState != DISPLAY_STATE_ERRORS)
	{
		selectedOption = 0;
	}

	static const char *elconErrorMessages[5] = {"HW Fail", "Charger Overtemp", "Wrong Input Volt", "No Batt Volt", "Comms Timeout"};
	static const char *bmsErrorMessages[5] = {"Cell Overvolt", "Cell Undervolt", "Overtemp", "Undertemp", "Avg Overtemp"};

	char currentErrors[11][100];
	int currentErrorIndex = 0;

	for (int i = 0; i < 5; i++)
	{
		if (currentBmsAndElconData.ELCON_fault[i] == 1)
		{
			sprintf(currentErrors[currentErrorIndex], "%s", elconErrorMessages[i]);
			currentErrorIndex++;
		}
	}
	for (int i = 0; i < 5; i++)
	{
		if (currentBmsAndElconData.BMS_fault[i] == 1)
		{
			sprintf(currentErrors[currentErrorIndex], "%s", bmsErrorMessages[i]);
			currentErrorIndex++;
		}
	}

	int numberOfErrors = currentErrorIndex;

	int navBarStartIndex = numberOfErrors;
	int navBarLastIndex = numberOfErrors;
	int numberOfOptions = navBarLastIndex;

	Display_clear();
	Display_drawTitleBar("Errors");
	Display_handleUpDownPress(&selectedOption, numberOfOptions);
	int currentView = selectedOption / 3;

	// if the nav bar is selected, ensures that the currentScreen is the last screen of profiles
	if (selectedOption > numberOfErrors - 1)
	{
		currentView = (numberOfErrors - 1) / 3;
	}
	int startIndex = currentView * 3;
	// ensures that the correct number of profiles are showed on the last screen
	if (selectedOption > numberOfErrors - 1)
	{
		startIndex = (numberOfErrors - 1) / 3 * 3;
	}
	// going up from first profile will go to Start button
	if (selectedOption < 0)
	{
		startIndex = 0;
		selectedOption = navBarLastIndex;
	}
	// going down from start button will reset back to first profile being selected
	if (selectedOption > navBarLastIndex)
	{
		startIndex = 0;
		selectedOption = 0;
	}

	// initial y-positions used for calculating profile display boxes
	int y1 = 15;
	int y2 = 13;
	int y3 = 24;

	// displays up to three errors per screen
	for (int i = startIndex; i < startIndex + 3 && i < numberOfErrors; i++)
	{
		ssd1306_SetCursor(3, y1);
		if (selectedOption == i)
		{
			ssd1306_FillRectangle(1, y2, 122, y3, White);
			ssd1306_WriteString(currentErrors[i], Font_6x8, Black);
		}
		else
		{
			ssd1306_DrawRectangle(1, y2, 122, y3, White);
			ssd1306_WriteString(currentErrors[i], Font_6x8, White);
		}

		y1 = y1 + 13;
		y2 = y2 + 13;
		y3 = y3 + 13;
	}

	int numberOfViews = (numberOfErrors + 2) / 3; //+ 2 ensures it will always round up

	Display_drawShortScrollBar(currentView, numberOfViews);

	char *navBarOptions[] = {"Nav"};
	int numberOfNavBarOptions = 1;
	Display_drawNavBar(navBarOptions, numberOfNavBarOptions, navBarStartIndex, selectedOption);

	Display_updateScreen();

	if (selectPressed)
	{
		selectPressed = false;
		if (selectedOption < navBarStartIndex && selectedOption >= 0)
		{
			return DISPLAY_STATE_ERRORS;
		}
		if (selectedOption == navBarStartIndex)
		{
			return DISPLAY_STATE_NAVIGATION;
		}
	}
	return DISPLAY_STATE_ERRORS;
}
