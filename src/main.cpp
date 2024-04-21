#include <Arduino.h>
#include <SPI.h>
#include <Audio.h>
#include <WiFi.h>

#include "Main.h"
#include "Extern.h"
#include "SDCard.h"
#include "Display.h"
#include "Touch.h"
#include "Helper.h"

bool menu_change = true;
bool volume_change = true;
bool button_change = true;
int page = 0;

// Timer
#define TIMER_DRAW_WIFI 5000
u_long oldTime_display_drawWiFi = 0;

#define TIMER_TIME_UPDATE 600000
u_long oldTime_time_update = 0;

// Initialize Audio
Audio audio(true, I2S_DAC_CHANNEL_LEFT_EN);


Config config;

void setup()
{
  bool succeeded;
  Serial.begin(115200);

  digitalWrite(33, HIGH);
  sdcard_initialize();
  sdcard_loadConfig();

  touch_initialize();
  display_initialize();

  wifi_connect();
  time_update();

  // Setup audio
  audio.forceMono(true);
  audio.setVolume(config.audio_Volume);
}

void loop()
{
  // Prcoess audio
  //audio.loop();

  touch_newPoint();

  // Update Time from NTP every 60 minutes 
  if (millis() - oldTime_time_update >= TIMER_TIME_UPDATE)
  {
    time_update();
    oldTime_time_update = millis();
  }

  // Draw WiFi and Time every 5 seconds
  if (millis() - oldTime_display_drawWiFi >= TIMER_DRAW_WIFI)
  {
    display_drawWiFi(WiFi.RSSI(), WiFi.SSID().c_str());
    display_drawTime();
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
  delay(100);
}

void audio_id3data(const char *info)
{ // id3 metadata
  Serial.print("id3data     ");
  Serial.println(info);
}

void audio_eof_mp3(const char *info)
{ // end of file
  Serial.print("eof_mp3     ");
  Serial.println(info);
}

void audio_showstation(const char *info)
{
  Serial.print("station     ");
  Serial.println(info);
}

void audio_bitrate(const char *info)
{
  Serial.print("bitrate     ");
  Serial.println(info);
}

void audio_commercial(const char *info)
{ // duration in sec
  Serial.print("commercial  ");
  Serial.println(info);
}

void audio_icyurl(const char *info)
{ // homepage
  Serial.print("icyurl      ");
  Serial.println(info);
}

void audio_lasthost(const char *info)
{ // stream URL played
  Serial.print("lasthost    ");
  Serial.println(info);
}

void audio_eof_speech(const char *info)
{
  Serial.print("eof_speech  ");
  Serial.println(info);
}