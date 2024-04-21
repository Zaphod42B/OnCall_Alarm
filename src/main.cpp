#include <Arduino.h>
#include <SPI.h>
#include <Audio.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <XPT2046_Touchscreen.h>

#include "SDCard.h"
#include "Display.h"

bool menu_change = true;
bool volume_change = true;
int page = 0;

// Timer
u_long oldTime_display_drawWiFi = 5000;

// Initialize Touch
#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33

SPIClass touchSpi = SPIClass(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);

// Initialize Audio
Audio audio(true, I2S_DAC_CHANNEL_LEFT_EN);

// Initialize Config
struct Config
{
  char wifi_SSID[32];
  char wifi_PW[32];
  int audio_Volume;
};

const char *filename = "/config.json";
Config config;

void setup()
{
  bool succeeded;
  Serial.begin(115200);

  sdcard_initialize();

  // Load config
  File file = SD.open(filename, "r");
  if (!file)
  {
    Serial.println("Failed to open file for reading");
  }
  else
  {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    if (error)
      Serial.println(F("Can't read config from file"));

    // Get Wifi Settings
    strlcpy(config.wifi_SSID, doc["wifi_SSID"], sizeof(config.wifi_SSID));
    strlcpy(config.wifi_PW, doc["wifi_PW"], sizeof(config.wifi_PW));

    // Get Audio Settings
    config.audio_Volume = doc["audio_Volume"];
  }

  // Initialize Touch Screen
  touchSpi.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(touchSpi);
  ts.setRotation(1);

  display_initialize();

  // Connect to WiFi
  WiFi.begin(config.wifi_SSID, config.wifi_PW);
  Serial.print("Connecting to ");
  Serial.println(config.wifi_SSID);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Setup audio
  audio.forceMono(true);
  audio.setVolume(config.audio_Volume);
}

void loop()
{
  // Prcoess audio
  // audio.loop();

  if (ts.touched())
  {
    TS_Point p = ts.getPoint();
    Serial.print("Pressure = ");
    Serial.print(p.z);
    Serial.print(", x = ");
    Serial.print(p.x);
    Serial.print(", y = ");
    Serial.print(p.y);
    Serial.println();
  }

  if (millis() - oldTime_display_drawWiFi >= 5000)
  {
    display_drawWiFi(WiFi.RSSI(), WiFi.SSID().c_str());
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

    if  (volume_change)
    {
      display_drawMainVolume(config.audio_Volume);
      volume_change = false;
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