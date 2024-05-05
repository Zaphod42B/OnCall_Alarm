#ifndef Display_h
#define Display_h

#include <TFT_eSPI.h>

#include "Free_Fonts.h"

extern TFT_eSPI tft;
extern TFT_eSprite sprite;

void display_adjustBacklight(uint32_t value, uint32_t valueMax = 10);
void display_initialize();
void display_clearScreen();
void display_drawWiFi();
void display_drawMainFrame();
void display_drawMainButtons();
void display_drawMainVolume();
void display_drawTime();
void display_pollTimer();
void display_getAuthToken();
void display_teamsMessage();
void display_drawPollTimers();
void display_drawUptime();

#endif