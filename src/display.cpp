#include <TFT_eSPI.h>
#include <WiFi.h>

#include "Main.h"
#include "Extern.h"
#include "Display.h"
#include "Helper.h"

#define WIDTH 320
#define HEIGHT 240

// Backlight
#define LCD_BACK_LIGHT_PIN 21
#define LEDC_CHANNEL_0 0     // use first channel of 16 channels (started from zero)
#define LEDC_TIMER_12_BIT 12 // use 12 bit precission for LEDC timer
#define LEDC_BASE_FREQ 5000  // use 5000 Hz as a LEDC base frequency

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite sprite = TFT_eSprite(&tft);

void display_adjustBacklight(uint32_t value, uint32_t valueMax)
{
    uint32_t duty = (4095 / valueMax) * min(value, valueMax);
    ledcWrite(LEDC_CHANNEL_0, duty);
}

void display_initialize()
{
    // Start the TFT display and set it to black
    tft.init();
    tft.invertDisplay(0);
    tft.setRotation(1); // This is the display in landscape
    tft.setTextWrap(true, true);
    tft.fillScreen(TFT_BLACK);

    ledcSetup(LEDC_CHANNEL_0, LEDC_BASE_FREQ, LEDC_TIMER_12_BIT);
    ledcAttachPin(LCD_BACK_LIGHT_PIN, LEDC_CHANNEL_0);
}

void display_clearScreen()
{
    // Clear the screen before writing to it and set default text colors
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
}

void display_drawWiFi()
{
    sprite.setColorDepth(8);
    sprite.createSprite(215, 25);
    sprite.fillSprite(TFT_BLACK);
    sprite.setTextColor(TFT_WHITE);
    sprite.setTextDatum(TL_DATUM);

    uint16_t signalColor = TFT_GREEN;
    int RSSI = WiFi.RSSI();

    if (RSSI <= -80)
    {
        signalColor = TFT_RED;
    }

    int start_x = 2;
    char temp[32];
    sprintf(temp, "%i dBm | %s", RSSI, WiFi.SSID().c_str());

    sprite.drawString(temp, start_x + 24, 4, 2);

    if (RSSI >= -90)
    {
        sprite.drawRect(start_x, 20, 2, 2, signalColor);
    }
    if (RSSI >= -80)
    {
        sprite.drawRect(start_x + 4, 16, 2, 6, signalColor);
    }
    if (RSSI >= -70)
    {
        sprite.drawRect(start_x + 8, 12, 2, 10, signalColor);
    }
    if (RSSI >= -67)
    {
        sprite.drawRect(start_x + 12, 8, 2, 14, signalColor);
    }
    if (RSSI >= -55)
    {
        sprite.drawRect(start_x + 16, 4, 2, 18, signalColor);
    }
    if (RSSI >= -30)
    {
        sprite.drawRect(start_x + 20, 0, 2, 22, signalColor);
    }

    sprite.pushSprite(0, 0);
    sprite.deleteSprite();
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
    /*
    tft.setFreeFont(FSSB9);
    tft.fillRect(WIDTH - 100, HEIGHT - 39, 100, 39, TFT_DARKGREY);
    tft.drawString("MENU", WIDTH - 49, HEIGHT - 21, GFXFF);
    */

    // Middle
    tft.setFreeFont(FSSB9);
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
    sprite.setColorDepth(8);
    sprite.createSprite(26, 176);
    sprite.fillSprite(TFT_BLACK);
    sprite.setTextColor(TFT_WHITE);
    sprite.setTextDatum(MC_DATUM);
    sprite.setFreeFont(FSS9);
    sprite.drawRect(0, 0, 26, 176, TFT_WHITE);
    if (config.display_brightness > 0)
    {
        int volHeight = ((10 - config.display_brightness) * (174 / 10));
        sprite.fillRect(2, volHeight + 2, 22, 172 - volHeight, TFT_DARKGREY);
    }
    sprite.drawCentreString("10", 11, 5, GFXFF);
    sprite.drawCentreString("0", 12, 155, GFXFF);
    sprite.pushSprite(0, 25);
    sprite.deleteSprite();
}

void display_drawTime()
{
    getLocalTime(&timeinfo);

    tft.setTextColor(TFT_WHITE, TFT_BLACK, true);
    tft.setTextDatum(TR_DATUM);

    char time_string[6];
    char date_string[35];
    strftime(time_string, 6, "%H:%M", &timeinfo);
    strftime(date_string, 255, "%d.%m.%Y", &timeinfo);

    tft.setTextFont(1);
    tft.drawString(date_string, 318, 4, 2);

    sprite.setColorDepth(8);
    sprite.createSprite(60, 20);
    sprite.fillSprite(TFT_BLACK);
    sprite.setTextColor(TFT_WHITE);
    sprite.setTextDatum(TR_DATUM);
    sprite.setFreeFont(FSSB12);
    sprite.drawString(time_string, 60, 0, GFXFF);
    sprite.pushSprite(258, 30);
    sprite.deleteSprite();
}

void display_getAuthToken()
{
    sprite.setColorDepth(8);
    sprite.createSprite(285, 105);
    sprite.fillSprite(TFT_BLACK);
    sprite.setTextColor(TFT_WHITE);
    sprite.setTextDatum(TL_DATUM);
    sprite.setTextFont(2);
    sprite.setTextSize(1);

    sprite.setCursor(0, 5);
    sprite.println(String(deviceAuth.message));
    sprite.println();
    sprite.print("Time left: ");
    sprite.print(String((deviceAuth.token_request_millis / 1000 + deviceAuth.expires_in) - millis() / 1000));
    sprite.print(" sec.");

    sprite.pushSprite(30, 60);
    sprite.deleteSprite();
}

void display_teamsMessage()
{
    sprite.setColorDepth(8);
    sprite.createSprite(220, 30);
    sprite.fillSprite(TFT_BLACK);
    sprite.setTextColor(TFT_WHITE);
    sprite.setTextDatum(TL_DATUM);
    sprite.setFreeFont(FSSB18);
    sprite.drawString(teamsMsg.subject, 0, 0, GFXFF);
    sprite.pushSprite(30, 31);
    sprite.deleteSprite();

    sprite.setColorDepth(8);
    sprite.createSprite(285, 105);
    sprite.fillSprite(TFT_BLACK);
    sprite.setTextColor(TFT_WHITE);
    sprite.setTextDatum(TL_DATUM);
    sprite.setTextFont(2);
    sprite.setTextWrap(true, true);

    sprite.setCursor(0, 5);
    sprite.println(teamsMsg.body);

    sprite.pushSprite(30, 60);
    sprite.deleteSprite();
}

void display_drawPollTimers()
{
    sprite.setColorDepth(8);
    sprite.createSprite(99, 38);
    sprite.fillSprite(TFT_BLACK);
    sprite.setTextColor(TFT_WHITE);
    sprite.setTextDatum(TL_DATUM);
    sprite.setTextFont(2);
    sprite.setCursor(0, 3);
    if (((authToken.token_request_millis / 1000 + authToken.expires_in) - millis() / 1000) > 0)
    {
        sprite.setTextColor(TFT_GREEN);
    }
    else
    {
        sprite.setTextColor(TFT_RED);
    }
    sprite.printf(" Token: %i\n", ((authToken.token_request_millis / 1000 + authToken.expires_in) - millis() / 1000));

    if ((millis() - teamsMsg.lastPoll) / 1000 <= 20)
    {
        sprite.setTextColor(TFT_GREEN);
    }
    else
    {
        sprite.setTextColor(TFT_RED);
    }

    if ((millis() - shift.lastPoll) / 1000 <= 20)
    {
        sprite.setTextColor(TFT_GREEN);
    }
    else
    {
        sprite.setTextColor(TFT_RED);
    }
    sprite.printf(" T: %i | S: %i", (millis() - teamsMsg.lastPoll) / 1000, (millis() - shift.lastPoll) / 1000);
    sprite.pushSprite(220, 201);
    sprite.deleteSprite();
}