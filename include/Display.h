#ifndef Display_h
#define Display_h

#include <TFT_eSPI.h>

extern TFT_eSPI tft;

void display_initialize();
void display_clearScreen();
void display_drawWiFi(int RSSI, const char *wifiName);
void display_drawMainFrame();
void display_drawMainButtons();
void display_drawMainVolume();

#endif