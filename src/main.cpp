#include <Arduino.h>
#include <SPI.h>
#include <driver/dac.h>

#include "Main.h"
#include "Extern.h"
#include "Display.h"
#include "Touch.h"
#include "Helper.h"
#include "WebConf.h"
#include "Graph.h"

bool userAuthenticated = false;

bool menu_change = true;
bool brightness_change = true;
bool button_change = true;
bool new_message = false;
bool new_shift = false;
int page = 0;

// Timer
#define TIMER_1_SECONDS 1000
u_long old_timer_1_seconds = TIMER_1_SECONDS;

#define TIMER_DRAW_WIFI 5000
u_long oldTime_display_drawWiFi = TIMER_DRAW_WIFI; // Time between WiFi and Time refresh in display in ms

#define TIMER_60_SECONDS 60000
u_long old_timer_60_seconds = TIMER_60_SECONDS;

#define TIMER_5_MINUTES 300000
u_long old_timer_5_minutes = TIMER_5_MINUTES;

// Create semaphore handle
SemaphoreHandle_t sem;

// create task handle
TaskHandle_t PollNtp;
TaskHandle_t CheckAuthToken;
TaskHandle_t PollTeamsChannel;
TaskHandle_t PollShifts;

// Create timeinfo handle
tm timeinfo;

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
  graph_loadReauthToken();

  time_prepareTimeZone();

  sem = xSemaphoreCreateBinary(); // Create binary semaphore
  xSemaphoreGive(sem);

  // Start task for Auth Token refresh
  if (xTaskCreatePinnedToCore(
          graph_checkAuthToken, // Function name of the task
          "CheckAuthToken",     // Name of the task (e.g. for debugging)
          8192,                 // Stack size (bytes)
          NULL,                 // Parameter to pass
          1,                    // Task priority
          &CheckAuthToken,      // Task handle
          1                     // Run on Core 1
          ))
  {
    Serial.printf("Refresh task for Auth Token started!\n\n");
  }
  else
  {
    Serial.printf("Error starting refres task for Auth Token!\n\n");
  }

  // Start Teams polling
  if (xTaskCreatePinnedToCore(
          graph_pollTeamsChannel, // Function name of the task
          "PollTeamsChannel",     // Name of the task (e.g. for debugging)
          8192,                   // Stack size (bytes)
          NULL,                   // Parameter to pass
          1,                      // Task priority
          &PollTeamsChannel,      // Task handle
          1                       // Run on Core 1
          ))
  {
    Serial.println("Teams polling startet successfully!");
  }
  else
  {
    Serial.println("Error starting Teams polling!");
  }

  // Start Shifts polling
  if (xTaskCreatePinnedToCore(
          graph_pollShifts, // Function name of the task
          "PollShifts",     // Name of the task (e.g. for debugging)
          4096,             // Stack size (bytes)
          NULL,             // Parameter to pass
          1,                // Task priority
          &PollShifts,      // Task handle
          1                 // Run on Core 1
          ))
  {
    Serial.println("Shifts polling startet successfully!");
  }
  else
  {
    Serial.println("Error starting Shifts polling!");
  }

  // Start NTP polling
  if (xTaskCreatePinnedToCore(
          time_update, // Function name of the task
          "PollNtp",   // Name of the task (e.g. for debugging)
          2048,        // Stack size (bytes)
          NULL,        // Parameter to pass
          1,           // Task priority
          &PollNtp,    // Task handle
          1            // Run on Core 1
          ))
  {
    Serial.println("NTP polling startet successfully!");
  }
  else
  {
    Serial.println("Error starting NTP polling!");
  }

  // Configure Timer0 Interrupt
  Timer0_Cfg = timerBegin(0, 80, true);
  timerAttachInterrupt(Timer0_Cfg, &Timer0_ISR, true);
  timerAlarmWrite(Timer0_Cfg, 20, true);
  timerAlarmEnable(Timer0_Cfg);
}

void loop()
{
  iotWebConf.doLoop();

  touch_newPoint();

  // Run every 1 Seconds
  if (millis() - old_timer_1_seconds >= TIMER_1_SECONDS)
  {
    multi_heap_info_t info;
    heap_caps_get_info(&info, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    Serial.printf("   --> Total Free: %i Byte\n", info.total_free_bytes);
    display_drawPollTimers();
    old_timer_1_seconds = millis();
  }

  // Draw WiFi and Time every 5 seconds
  if (millis() - oldTime_display_drawWiFi >= TIMER_DRAW_WIFI)
  {
    if (iotWebConf.getState() == 4)
    {
      display_drawWiFi();
      display_drawTime();
    }
    if (!userAuthenticated)
    {
      display_getAuthToken();
    }
    oldTime_display_drawWiFi = millis();
  }

  // Run every 60 Seconds
  if (millis() - old_timer_60_seconds >= TIMER_60_SECONDS)
  {
    multi_heap_info_t info;
    heap_caps_get_info(&info, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    Serial.println("Memory ESP");
    Serial.printf("   --> Total Free: %i Byte\n", info.total_free_bytes);
    Serial.printf("   --> High watermark:   %i Byte\n", info.minimum_free_bytes);
    Serial.printf("   --> Largest block:    %i Byte\n\n", info.largest_free_block);

    Serial.println("Memory High Watermark from Tasks");
    Serial.printf("   --> %s: %i Byte\n", pcTaskGetTaskName(PollNtp), uxTaskGetStackHighWaterMark(PollNtp));
    Serial.printf("   --> %s: %i Byte\n", pcTaskGetTaskName(CheckAuthToken), uxTaskGetStackHighWaterMark(CheckAuthToken));
    Serial.printf("   --> %s: %i Byte\n", pcTaskGetTaskName(PollTeamsChannel), uxTaskGetStackHighWaterMark(PollTeamsChannel));
    Serial.printf("   --> %s: %i Byte\n\n", pcTaskGetTaskName(PollShifts), uxTaskGetStackHighWaterMark(PollShifts));
    old_timer_60_seconds = millis();
  }

  // Run every 5 Minutes
  if (millis() - old_timer_5_minutes >= TIMER_5_MINUTES)
  {
    old_timer_5_minutes = millis();
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

    if (brightness_change)
    {
      display_drawMainVolume();
      display_adjustBacklight(config.display_brightness);
      brightness_change = false;
    }

    if (button_change)
    {
      display_drawMainButtons();
      button_change = false;
    }
    if (new_message)
    {
      display_teamsMessage();
      new_message = false;
    }
    break;

  default:
    break;
  }
}