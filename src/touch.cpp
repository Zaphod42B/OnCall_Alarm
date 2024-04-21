#include <XPT2046_Touchscreen.h>

#include "Touch.h"
#include "Display.h"
#include "Free_Fonts.h"
#include "Main.h"
#include "Extern.h"
#include "SDCard.h"

// Initialize Touch
#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33

#define TOUCH_X_MIN 200
#define TOUCH_X_MAX 3700
#define TOUCH_Y_MIN 200
#define TOUCH_Y_MAX 3850

bool is_touched = false;
int touch_x = 0;
int touch_y = 0;

SPIClass touchSpi = SPIClass(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);

void touch_initialize()
{
    // Initialize Touch Screen
    touchSpi.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
    ts.begin(touchSpi);
    ts.setRotation(1);
}

void touch_newPoint()
{
    if (ts.touched())
    {
        if (!is_touched)
        {
            TS_Point p = ts.getPoint();
            Serial.print("Raw: Pressure = ");
            Serial.print(p.z);
            Serial.print(", x = ");
            Serial.print(p.x);
            Serial.print(", y = ");
            Serial.print(p.y);
            Serial.println();

            touch_x = (p.x - TOUCH_X_MIN) * (340 - 0) / (TOUCH_X_MAX - TOUCH_X_MIN);
            touch_y = (p.y - TOUCH_Y_MIN) * (240 - 0) / (TOUCH_Y_MAX - TOUCH_Y_MIN);

            Serial.print("Out: Pressure = ");
            Serial.print(p.z);
            Serial.print(", x = ");
            Serial.print(touch_x);
            Serial.print(", y = ");
            Serial.print(touch_y);
            Serial.println();

            // Button "-"
            if (touch_x >= 0 && touch_x <= 50 && touch_y >= 200 && touch_y <= 240)
            {
                if (config.audio_Volume > 0)
                {
                    config.audio_Volume--;
                    volume_change = true;
                }
            }
            // Button "+"
            else if (touch_x >= 51 && touch_x <= 100 && touch_y >= 200 && touch_y <= 240)
            {
                if (config.audio_Volume < 10)
                {
                    config.audio_Volume++;
                    volume_change = true;
                }
            }
            // Button "Armed/Disarmed"
            else if (touch_x >= 102 && touch_x <= 218 && touch_y >= 200 && touch_y <= 240)
            {
                config.is_Armed ^= 1;
                button_change = true;
            }

            is_touched = true;
        }
    }
    else
    {
        is_touched = false;
    }
}