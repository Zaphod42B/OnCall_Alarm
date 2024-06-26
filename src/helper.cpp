#include <WiFi.h>

#include "Helper.h"
#include "Extern.h"
#include "WebConf.h"

#include <driver/dac.h>

// ##################################
// ### Time #########################
// ##################################

#define NTP_POLLING_INTERVAL atoi(string_poll_TimerNtp) * 1000 + rand() % 10000

char *tzbuf_value;

void time_update(void *parameter)
{
    while (true)
    {
        while (iotWebConf.getState() != 4)
        {
            delay(5000);
        }

        xSemaphoreTake(sem, portMAX_DELAY);
        configTime(0, 0, "pool.ntp.org");

        time_setTimezone();

        Serial.printf("Get time from pool.ntp.org... ");
        if (!getLocalTime(&timeinfo))
        {
            Serial.printf("[Failed]\n\n");
        }
        else
        {
            Serial.printf("[Done]\n");
            char time_string[9];
            char date_string[255];
            strftime(time_string, 9, "%H:%M:%S", &timeinfo);
            strftime(date_string, 255, "%d %B %Y", &timeinfo);
            Serial.printf("   --> Time: %s\n", time_string);
            Serial.printf("   --> Date: %s\n\n", date_string);
        }

        xSemaphoreGive(sem);
        delay(NTP_POLLING_INTERVAL);
    }
}

void time_prepareTimeZone()
{
    static char tzbuf[] = "TZ=Value_long_enough_for_any_we_want_to_set";
    unsetenv("TZ");
    putenv(tzbuf);
    tzbuf_value = getenv("TZ");
}

void time_setTimezone()
{
    strcpy(tzbuf_value, "CET-1CEST,M3.5.0,M10.5.0/3");
}

void alarm_audio(void *parameter)
{
    while (true)
    {   
        xSemaphoreTake(sem, portMAX_DELAY);
        while (config.is_Alarm)
        {
            dac_output_enable(DAC_CHANNEL_2);
            delay(500);
            dac_output_disable(DAC_CHANNEL_2);
            delay(500);
        }
        xSemaphoreGive(sem);
        delay(1000);
    }
}