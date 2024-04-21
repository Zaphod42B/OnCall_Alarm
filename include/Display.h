#ifndef Display_h
#define Display_h

#include <string.h>

void display_initialize();
void display_clearScreen();
void display_drawWiFi(int RSSI, const char *wifiName);
void display_drawMainFrame();
void display_drawMainButtons();
void display_drawMainVolume(int volume);

#endif