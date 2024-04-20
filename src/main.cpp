#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <Audio.h>
#include <WiFi.h>
#include <ArduinoJson.h>

#include "SDCard.h"

// Initialize Display
TFT_eSPI tft = TFT_eSPI();

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
};

const char *filename = "/config.json";
Config config;

void setup()
{
  bool succeeded;
  Serial.begin(115200);

  // Start the SPI for the touch screen and init the TS library
  touchSpi.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(touchSpi);
  ts.setRotation(1);

  // Start the TFT display and set it to black
  tft.init();
  tft.setRotation(1); // This is the display in landscape
  tft.setTextWrap(true, true);

  // Clear the screen before writing to it and set default text colors
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("Title", 0, 10, 1);

  initialize_SDCard();

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

    strlcpy(config.wifi_SSID, doc["wifi_SSID"], sizeof(config.wifi_SSID));
    strlcpy(config.wifi_PW, doc["wifi_PW"], sizeof(config.wifi_PW));
  }

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
  audio.setVolume(5);
  audio.setTone(0, 0, 0);

  /*
  do
  {
    // Connect to 'FM - Disco Ball 70's-80's Los Angeles'
    succeeded = audio.connecttohost("https://edge51.streamonkey.net/radio886-newrock/stream/mp3?aggregator=886Website;");
    delay(500);
    Serial.println("Retrying");
  } while (!succeeded);
  */
}

void loop()
{
  // Prcoess audio
  audio.loop();
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