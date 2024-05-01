#include <Arduino.h>
#include <SPI.h>
#include <driver/dac.h>

#include "Main.h"
#include "Extern.h"
#include "Display.h"
#include "Touch.h"
#include "Helper.h"
#include "WebConf.h"

bool menu_change = true;
bool volume_change = true;
bool button_change = true;
int page = 0;

// Timer
#define TIMER_DRAW_WIFI 5000
u_long oldTime_display_drawWiFi = 0;

#define TIMER_TIME_UPDATE 10000
u_long oldTime_time_update = 0;

// Initialize Audio
// Audio audio(true, I2S_DAC_CHANNEL_LEFT_EN);

// Timer0 Configuration Pointer (Handle)
hw_timer_t *Timer0_Cfg = NULL;

// Sine LookUpTable & Index Variable
uint8_t SampleIdx = 0;
const uint8_t sineLookupTable[] = {
    128, 136, 143, 151, 159, 167, 174, 182,
    189, 196, 202, 209, 215, 220, 226, 231,
    235, 239, 243, 246, 249, 251, 253, 254,
    255, 255, 255, 254, 253, 251, 249, 246,
    243, 239, 235, 231, 226, 220, 215, 209,
    202, 196, 189, 182, 174, 167, 159, 151,
    143, 136, 128, 119, 112, 104, 96, 88,
    81, 73, 66, 59, 53, 46, 40, 35,
    29, 24, 20, 16, 12, 9, 6, 4,
    2, 1, 0, 0, 0, 1, 2, 4,
    6, 9, 12, 16, 20, 24, 29, 35,
    40, 46, 53, 59, 66, 73, 81, 88,
    96, 104, 112, 119};

// The Timer0 ISR Function (Executes Every Timer0 Interrupt Interval)
void IRAM_ATTR Timer0_ISR()
{
  // Send SineTable Values To DAC One By One
  dac_output_voltage(DAC_CHANNEL_2, sineLookupTable[SampleIdx++]);
  if (SampleIdx == 100)
  {
    SampleIdx = 0;
  }
}

Config config;

void setup()
{
  bool succeeded;
  Serial.begin(115200);

  touch_initialize();
  display_initialize();

  webconf_init();

  // Configure Timer0 Interrupt
  Timer0_Cfg = timerBegin(0, 80, true);
  timerAttachInterrupt(Timer0_Cfg, &Timer0_ISR, true);
  timerAlarmWrite(Timer0_Cfg, 20, true);
  timerAlarmEnable(Timer0_Cfg);
  // Enable DAC1 Channel's Output
}

void loop()
{
  iotWebConf.doLoop();

  touch_newPoint();

  // Update Time from NTP every 10 seconds
  if (millis() - oldTime_time_update >= TIMER_TIME_UPDATE && iotWebConf.getState() == 4)
  {
    time_update();
    oldTime_time_update = millis();
  }

  // Draw WiFi and Time every 5 seconds
  if (millis() - oldTime_display_drawWiFi >= TIMER_DRAW_WIFI)
  {
    if (iotWebConf.getState() == 4)
    {
      display_drawWiFi(WiFi.RSSI(), WiFi.SSID().c_str());
      display_drawTime();
    }
    else if (iotWebConf.getState() == 2)
    {
      Serial.println("In Config Mode...");
    }
    oldTime_display_drawWiFi = millis();
  }

  switch (page)
  {
  case 0:
    if (menu_change)
    {
      display_clearScreen();
      display_drawMainFrame();
      display_drawMainButtons();
      menu_change = false;
    }

    if (volume_change)
    {
      display_drawMainVolume();
      volume_change = false;
    }

    if (button_change)
    {
      display_drawMainButtons();
      button_change = false;
    }
    break;

  default:
    break;
  }
}