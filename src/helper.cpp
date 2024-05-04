#include <WiFi.h>

#include "Helper.h"
#include "Extern.h"
#include "WebConf.h"

// ##################################
// ### Time #########################
// ##################################

#define NTP_POLLING_INTERVAL 60000 + rand() % 10000

const char *timezone = "CET-1CEST,M3.5.0,M10.5.0/3";

void time_update(void *parameter)
{
    while (true)
    {
        while (iotWebConf.getState() != 4)
        {
            delay(1000);
        }

        xSemaphoreTake(sem, portMAX_DELAY);
        configTime(0, 0, "pool.ntp.org");

        setenv("TZ", timezone, 1); //  Now adjust the TZ. Clock settings are adjusted to show the new local time
        tzset();

        Serial.printf("Get time from pool.ntp.org... ");
        if (!getLocalTime(&timeinfo))
        {
            Serial.printf("[Failed]\n\n");
            return;
        }
        else
        {
            Serial.printf("[Done]\n");
        }

        char time_string[9];
        char date_string[255];
        strftime(time_string, 9, "%H:%M:%S", &timeinfo);
        strftime(date_string, 255, "%d %B %Y", &timeinfo);
        Serial.printf("   --> Time: %s\n", time_string);
        Serial.printf("   --> Date: %s\n\n", date_string);

        xSemaphoreGive(sem);

        delay(NTP_POLLING_INTERVAL);
    }
}