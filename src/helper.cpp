#include "Helper.h"

#include <WiFi.h>

// ##################################
// ### Time #########################
// ##################################

const char *timezone = "CET-1CEST,M3.5.0,M10.5.0/3";

void time_set_timezone()
{
    Serial.printf("Setting Timezone to %s\n\n", timezone);
    setenv("TZ", timezone, 1); //  Now adjust the TZ.  Clock settings are adjusted to show the new local time
    tzset();
}

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

   time_set_timezone();

    getLocalTime(&timeinfo);

    char time_string[9];
    char date_string[255];
    strftime(time_string, 9, "%H:%M:%S", &timeinfo);
    strftime(date_string, 255, "%d %B %Y", &timeinfo);
    Serial.printf("Time: %s\n", time_string);
    Serial.printf("Date: %s\n\n", date_string);
}