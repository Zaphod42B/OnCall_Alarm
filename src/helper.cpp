#include "Helper.h"
#include "Main.h"
#include "Extern.h"

#include <string>
#include <WiFi.h>
#include <ArduinoJson.h>

// ##################################
// ### WiFi #########################
// ##################################

String hostname = "OnCall-Alarm";
WiFiClient wifiClient;

void wifi_connect()
{
    WiFi.mode(WIFI_STA); // Set WiFi to station mode
    wifi_reconnect();
}

void wifi_reconnect()
{
    WiFi.disconnect(true, true);
    delay(500);
    WiFi.setHostname(hostname.c_str());
    WiFi.begin(config.wifi_SSID, config.wifi_PW);
    int connectionCount = 0;
    Serial.printf("Connecting WiFi ");
    while (WiFi.status() != WL_CONNECTED)
    {
        if (connectionCount <= 10)
        {
            delay(500);
            connectionCount++;
            Serial.print(". ");
        }
        else
        {
            Serial.print("[FAILED!]\n");
            return;
        }
    }
    Serial.print("[CONNECTED!]\n");
    Serial.print("  -IP address: ");
    Serial.println(WiFi.localIP());
}

// ##################################
// ### Time #########################
// ##################################

const char *timezone = "CET-1CEST,M3.5.0,M10.5.0/3";

void time_update()
{
    struct tm timeinfo;

    configTime(0, 0, "pool.ntp.org");

    Serial.printf("Get time from pool.ntp.org... ");
    if (!getLocalTime(&timeinfo))
    {
        Serial.printf("[Failed]\n\n");
        return;
    }
    else
    {
        Serial.printf("[Done]\n\n");
    }

    Serial.printf("Setting Timezone to %s\n\n", timezone);
    setenv("TZ", timezone, 1); //  Now adjust the TZ.  Clock settings are adjusted to show the new local time
    tzset();

    getLocalTime(&timeinfo);

    char time_string[9];
    char date_string[255];
    strftime(time_string, 9, "%H:%M:%S", &timeinfo);
    strftime(date_string, 255, "%d %B %Y", &timeinfo);
    Serial.printf("Time: %s\n", time_string);
    Serial.printf("Date: %s\n\n", date_string);
}