#include <TFT_eSPI.h>

#include "Main.h"
#include "Extern.h"
#include "Display.h"
#include "Free_Fonts.h"

TFT_eSPI tft = TFT_eSPI();

#define WIDTH 320
#define HEIGHT 240

void display_initialize()
{
    // Start the TFT display and set it to black
    tft.init();
    tft.invertDisplay(0);
    tft.setRotation(1); // This is the display in landscape
    tft.setTextWrap(true, true);
}

void display_clearScreen()
{
    // Clear the screen before writing to it and set default text colors
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
}

void display_drawWiFi(int RSSI, const char *wifiName)
{
    tft.setTextColor(TFT_WHITE, TFT_BLACK, true);
    tft.setTextDatum(TL_DATUM);

    tft.drawRect(280, 0, 40, 23, TFT_BLACK);

    uint16_t signalColor = TFT_GREEN;

    if (RSSI < -80)
    {
        signalColor = TFT_RED;
    }

    int start_x = 2;
    char temp[32];
    sprintf(temp, "%i dBm | %s", RSSI, wifiName);

    tft.setTextFont(1);
    tft.drawString(temp, start_x + 24, 4, 2);

    if (RSSI >= -90)
    {
        tft.drawRect(start_x, 20, 2, 2, signalColor);
    }
    if (RSSI >= -80)
    {
        tft.drawRect(start_x + 4, 16, 2, 6, signalColor);
    }
    if (RSSI >= -70)
    {
        tft.drawRect(start_x + 8, 12, 2, 10, signalColor);
    }
    if (RSSI >= -67)
    {
        tft.drawRect(start_x + 12, 8, 2, 14, signalColor);
    }
    if (RSSI >= -55)
    {
        tft.drawRect(start_x + 16, 4, 2, 18, signalColor);
    }
    if (RSSI >= -30)
    {
        tft.drawRect(start_x + 20, 0, 2, 22, signalColor);
    }
}

void display_drawMainFrame()
{
    tft.drawFastHLine(0, 25, WIDTH, TFT_WHITE);
    tft.drawFastHLine(0, HEIGHT - 40, WIDTH, TFT_WHITE);
}

void display_drawMainButtons()
{
    tft.setTextColor(TFT_BLACK, TFT_DARKGREY, true);
    tft.setTextDatum(MC_DATUM);

    // Left
    tft.setFreeFont(FSSB18);
    tft.fillRect(0, HEIGHT - 39, 49, 39, TFT_DARKGREY);
    tft.fillRect(51, HEIGHT - 39, 49, 39, TFT_DARKGREY);
    tft.drawString("-", 24, HEIGHT - 25, GFXFF);
    tft.drawString("+", 74, HEIGHT - 25, GFXFF);

    // Right
    tft.setFreeFont(FSSB9);
    tft.fillRect(WIDTH - 100, HEIGHT - 39, 100, 39, TFT_DARKGREY);
    tft.drawString("MENU", WIDTH - 49, HEIGHT - 21, GFXFF);

    // Middle
    tft.setTextColor(TFT_BLACK);
    if (config.is_Armed == 1)
    {
        tft.fillRect(102, HEIGHT - 39, 116, 39, TFT_RED);
        tft.drawString("ARMED", WIDTH / 2, HEIGHT - 21, GFXFF);
    }
    else
    {
        tft.fillRect(102, HEIGHT - 39, 116, 39, TFT_DARKGREEN);
        tft.drawString("DISARMED", WIDTH / 2, HEIGHT - 21, GFXFF);
    }
    tft.drawFastVLine(0, HEIGHT - 39, 39, TFT_WHITE);
    tft.drawFastVLine(WIDTH - 1, HEIGHT - 39, 39, TFT_WHITE);
    tft.drawFastVLine(100, HEIGHT - 39, 39, TFT_BLACK);
    tft.drawFastVLine(101, HEIGHT - 39, 39, TFT_LIGHTGREY);
    tft.drawFastVLine(WIDTH - 101, HEIGHT - 39, 39, TFT_LIGHTGREY);
    tft.drawFastVLine(WIDTH - 102, HEIGHT - 39, 39, TFT_BLACK);
    tft.drawFastVLine(50, HEIGHT - 39, 39, TFT_LIGHTGREY);
    tft.drawFastHLine(0, HEIGHT - 1, WIDTH, TFT_WHITE);
}

void display_drawMainVolume()
{
    tft.setTextColor(TFT_WHITE);
    tft.setTextDatum(MC_DATUM);
    tft.setFreeFont(FSS9);

    tft.drawFastVLine(0, 25, 175, TFT_WHITE);
    tft.drawFastVLine(25, 25, 175, TFT_WHITE);

    tft.fillRect(1, 26, 24, 174, TFT_BLACK);

    if (config.audio_Volume > 0)
    {
        int volHeight = 26 + ((10 - config.audio_Volume) * (175 / 10));
        tft.fillRect(1, volHeight, 24, 200 - volHeight, TFT_DARKGREY);
        tft.drawRect(1, volHeight, 24, 200 - volHeight, TFT_BLACK);
    }

    tft.drawCentreString("10", 11, 30, GFXFF);
    tft.drawCentreString("0", 12, HEIGHT - 60, GFXFF);
}