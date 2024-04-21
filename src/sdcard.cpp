#include <FS.h>
#include <SD.h>
#include <ArduinoJson.h>

#include "SDCard.h"
#include "Main.h"
#include "Extern.h"

const char *file_config = "/config.json";

// Initialize SD-Card
SPIClass sdSpi = SPIClass(VSPI);

void sdcard_initialize()
{
    // Initialize SD-Card
    if (!SD.begin(SS, sdSpi, 80000000))
    {
        Serial.println("Card Mount Failed");
        return;
    }
    uint8_t cardType = SD.cardType();

    if (cardType == CARD_NONE)
    {
        Serial.println("No SD card attached");
        return;
    }

    Serial.print("SD Card Type: ");
    if (cardType == CARD_MMC)
    {
        Serial.println("MMC");
    }
    else if (cardType == CARD_SD)
    {
        Serial.println("SDSC");
    }
    else if (cardType == CARD_SDHC)
    {
        Serial.println("SDHC");
    }
    else
    {
        Serial.println("UNKNOWN");
    }

    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf("SD Card Size: %lluMB\n", cardSize);
}

void sdcard_loadConfig()
{
    // Load config
    File file = SD.open(file_config, FILE_READ);
    if (!file)
    {
        Serial.println("Failed to open file for reading");
        return;
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

        // Get Armed state
        config.is_Armed = doc["is_Armed"];

        file.close();
    }
}
