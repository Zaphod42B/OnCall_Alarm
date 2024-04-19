#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <Audio.h>
#include <WiFi.h>

// Initialize Display
TFT_eSPI tft = TFT_eSPI();

// Initialize Audio
Audio audio(true, I2S_DAC_CHANNEL_LEFT_EN);

// Initialize WiFi
const char* ssid     = "Titanic Syncing"; // Change this to your WiFi SSID
const char* password = "emergencybroadcastsystem42"; // Change this to your WiFi password

void setup()
{
  bool succeeded;
  Serial.begin(115200);

  // Start the TFT display and set it to black
  tft.init();
  tft.setRotation(1); // This is the display in landscape
  tft.setTextWrap(true, true);

  // Clear the screen before writing to it and set default text colors
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("Title", 0, 10, 1);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Setup audio and
  audio.forceMono(true);
  audio.setVolume(10);
  do
  {
    // Connect to 'FM - Disco Ball 70's-80's Los Angeles'
    succeeded = audio.connecttohost("http://sc8.1.fm:8100/;");
    delay(500);
    Serial.println("Retrying");
  } while (!succeeded);
}

void loop()
{
  // Prcoess audio
  audio.loop();
}

void printTitle(const char *info)
{
  tft.fillRect(0, 20, 320, 200, TFT_BLACK);
  tft.setCursor(0, 20, 4);
  tft.setTextColor(TFT_SKYBLUE);
  tft.println(info);
}

void printInfo(const char *info)
{
  tft.fillRect(0, 230, 320, 10, TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.drawString(info, 0, 230, 1);
}

void audio_info(const char *info)
{
  Serial.print("info        ");
  Serial.println(info);
  printInfo(info);
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

void audio_showstreamtitle(const char *info)
{
  Serial.print("streamtitle ");
  Serial.println(info);
  printTitle(info);
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