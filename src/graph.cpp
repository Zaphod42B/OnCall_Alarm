#include <regex>
#include <time.h>

#include "Graph.h"
#include "WebConf.h"
#include "Extern.h"
#include "Display.h"
#include "Helper.h"

#define TEAMS_POLLING_INTERVALL 10000
#define SHIFTS_POLLING_INTERVALL 10000

#define TOKEN_MIN_LIFETIME (authToken.expires_in * 0.75) * 1000 //

HTTPClient http;

AuthToken authToken;
TeamsMsg teamsMsg;
DeviceAuth deviceAuth;
Shift shift;

String graphEndpoint;
String httpRequestData;
String httpHeader;

char shifts_Start_Time[32];
char shifts_End_Time[32];

int httpResponseCode;

void graph_loadReauthToken()
{
    strlcpy(authToken.refresh_token, string_reauthToken, sizeof(string_reauthToken));
}

void graph_getAuthToken()
{
    graphEndpoint = "https://login.microsoftonline.com/" + String(string_teams_TenantID) + "/oauth2/v2.0/devicecode";
    httpRequestData = "client_id=" + String(string_teams_AppID) + "&scope=" + authToken.scope;

    http.begin(graphEndpoint);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    int httpResponseCode = http.POST(httpRequestData);

    Serial.printf("\nDevice Authorization... ");
    if (httpResponseCode != 200)
    {
        Serial.printf("[ERROR]\n  -> HTTP-Response: %i\n\n", httpResponseCode);
        Serial.println(http.getString());
        http.end();
        return;
    }

    Serial.printf("[SUCCESS]\n  -> HTTP-Response: %i\n\n", httpResponseCode);

    // Allocate the JSON document
    JsonDocument doc;

    // Parse JSON object
    DeserializationError error = deserializeJson(doc, http.getString());
    if (error)
    {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        http.end();
        return;
    }
    http.end();

    deviceAuth.token_request_millis = millis();
    deviceAuth.expires_in = doc["expires_in"];
    deviceAuth.interval = doc["interval"];
    strlcpy(deviceAuth.device_code, doc["device_code"], sizeof(deviceAuth.device_code));
    strlcpy(deviceAuth.user_code, doc["user_code"], sizeof(deviceAuth.user_code));
    strlcpy(deviceAuth.message, doc["message"], sizeof(deviceAuth.message));

    Serial.println(String(deviceAuth.message));
    Serial.printf("Waiting for User Authorization...");

    userAuthenticated = false;
    while (!userAuthenticated)
    {
        delay(deviceAuth.interval * 1000);

        graphEndpoint = "https://login.microsoftonline.com/" + String(string_teams_TenantID) + "/oauth2/v2.0/token";
        httpRequestData = "grant_type=urn:ietf:params:oauth:grant-type:device_code&client_id=" + String(string_teams_AppID) + "&device_code=" + String(deviceAuth.device_code);

        http.begin(graphEndpoint);
        http.addHeader("Content-Type", "application/x-www-form-urlencoded");

        int httpResponseCode = http.POST(httpRequestData);

        // Allocate the JSON document
        JsonDocument userAuth;

        // Parse JSON object
        DeserializationError error = deserializeJson(userAuth, http.getString());
        if (error)
        {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.f_str());
            http.end();
            return;
        }

        if (userAuth["error"].as<String>().compareTo("authorization_pending") == 0)
        {
            Serial.print(".");
        }
        else if (userAuth["error"].as<String>().compareTo("null") == 0)
        {
            Serial.println("[SUCCESS]\n\n");
            userAuthenticated = true;

            authToken.token_request_millis = millis();
            authToken.expires_in = userAuth["expires_in"];
            strlcpy(authToken.token_type, userAuth["token_type"], sizeof(authToken.token_type));
            strlcpy(authToken.access_token, userAuth["access_token"], sizeof(authToken.access_token));
            strlcpy(authToken.refresh_token, userAuth["refresh_token"], sizeof(authToken.refresh_token));
            strlcpy(string_reauthToken, authToken.refresh_token, sizeof(authToken.refresh_token));
            iotWebConf.saveConfig();
        }
        else
        {
            userAuth.clear();
            return;
        }
        userAuth.clear();
    }
}

bool graph_reAuthToken()
{
    graphEndpoint = "https://login.microsoftonline.com/" + String(string_teams_TenantID) + "/oauth2/v2.0/token";
    httpRequestData = "client_id=" + String(string_teams_AppID) + "&scope=" + authToken.scope + "&refresh_token=" + authToken.refresh_token + "&grant_type=refresh_token";

    http.begin(graphEndpoint);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    int httpResponseCode = http.POST(httpRequestData);

    Serial.print("Refreshing Auth Token... ");
    if (httpResponseCode != 200)
    {
        Serial.printf("[ERROR]\n   --> HTTP-Response: %i\n\n", httpResponseCode);
        Serial.println(http.getString());
        http.end();
        return false;
    }

    Serial.printf("[SUCCESS]\n   --> HTTP-Response: %i\n\n", httpResponseCode);

    // Allocate the JSON document
    JsonDocument doc;

    // Parse JSON object
    DeserializationError error = deserializeJson(doc, http.getString());
    if (error)
    {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        http.end();
        return false;
    }

    authToken.token_request_millis = millis();
    authToken.expires_in = doc["expires_in"];
    strlcpy(authToken.token_type, doc["token_type"], sizeof(authToken.token_type));
    strlcpy(authToken.access_token, doc["access_token"], sizeof(authToken.access_token));
    strlcpy(authToken.refresh_token, doc["refresh_token"], sizeof(authToken.refresh_token));
    strlcpy(string_reauthToken, authToken.refresh_token, sizeof(authToken.refresh_token));
    // iotWebConf.saveConfig();

    http.end();

    userAuthenticated = true;
    return true;
}

void graph_checkAuthToken(void *parameter)
{
    while (true)
    {
        while (iotWebConf.getState() != 4)
        {
            delay(1000);
        }

        xSemaphoreTake(sem, portMAX_DELAY);
        if (!graph_reAuthToken())
        {
            graph_getAuthToken();
        }
        xSemaphoreGive(sem);
        delay(TOKEN_MIN_LIFETIME);
    }
}

void graph_pollTeamsChannel(void *parameter)
{
    while (true)
    {
        while (iotWebConf.getState() != 4)
        {
            delay(1000);
        }

        xSemaphoreTake(sem, portMAX_DELAY);
        graphEndpoint = "https://graph.microsoft.com/v1.0/teams/" + String(string_teams_TeamID) + "/channels/" + String(string_teams_ChannelID) + "/messages?top=1";
        httpHeader = "Bearer " + String(authToken.access_token);

        http.begin(graphEndpoint);
        http.addHeader("Authorization", httpHeader);

        httpResponseCode = http.GET();

        Serial.print("Getting last Message from Teams-Channel... ");
        if (httpResponseCode != 200)
        {
            Serial.printf("[ERROR]\n   --> HTTP-Response: %i\n\n", httpResponseCode);
            Serial.println(http.getString());
            http.end();
        }
        else
        {
            Serial.printf("[Done]\n   --> HTTP-Response: %i\n", httpResponseCode);
            teamsMsg.lastPoll = millis();
            graph_deserializeTeamsMsg();
            http.end();
            if (new_message)
            {
                Serial.printf("   --> New Message received:\n");
                Serial.printf("      --> [ID]: %s\n", String(teamsMsg.Id));
                Serial.printf("      --> [Created]: %s\n", teamsMsg.createdDateTime);
                Serial.printf("      --> [Subject]: %s\n\n", teamsMsg.subject);
            }
            else
            {
                Serial.printf("   --> No new Message.\n\n");
            }
        }

        xSemaphoreGive(sem);
        delay(TEAMS_POLLING_INTERVALL);
    }
}

void graph_deserializeTeamsMsg()
{
    // Allocate the JSON document
    JsonDocument msgArray;

    // Parse JSON object
    DeserializationError error = deserializeJson(msgArray, http.getString());
    if (error)
    {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
    }

    JsonObject msgIndex = msgArray["value"][0];

    if (teamsMsg.Id < msgIndex["id"].as<unsigned long long>())
    {
        teamsMsg.Id = msgIndex["id"].as<unsigned long long>();
        strlcpy(teamsMsg.createdDateTime, msgIndex["createdDateTime"], sizeof(teamsMsg.createdDateTime));
        strlcpy(teamsMsg.subject, msgIndex["subject"], sizeof(teamsMsg.subject));

        // Remove HTML-Tags from body string
        std::regex tags("<[^>]*>");
        std::string remove{};
        std::string out;
        out = std::regex_replace(msgIndex["body"]["content"].as<std::string>(), tags, remove);

        strlcpy(teamsMsg.body, out.c_str(), sizeof(teamsMsg.body));
        new_message = true;
    }
}

void graph_pollShifts(void *parameter)
{
    time_t timevalue;
    char dateTimeString[20];
    while (true)
    {
        while (iotWebConf.getState() != 4)
        {
            delay(5000);
        }
        xSemaphoreTake(sem, portMAX_DELAY);

        configTime(0, 0, "pool.ntp.org");
        getLocalTime(&timeinfo);

        timeinfo.tm_mday -= 1;
        timevalue = mktime(&timeinfo);
        timeinfo = *localtime(&timevalue);
        strftime(shifts_Start_Time, sizeof(shifts_Start_Time), "%Y-%m-%dT%H:%M:00.000Z", &timeinfo);
        timeinfo.tm_mday += 2;
        timevalue = mktime(&timeinfo);
        timeinfo = *localtime(&timevalue);
        strftime(shifts_End_Time, sizeof(shifts_End_Time), "%Y-%m-%dT%H:%M:00.000Z", &timeinfo);
        time_setTimezone();

        graphEndpoint = "https://graph.microsoft.com/v1.0/teams/" + String(string_teams_TeamID) + "/schedule/shifts?$filter=sharedShift/startDateTime%20ge%20" + shifts_Start_Time + "%20and%20sharedShift/endDateTime%20le%20" + shifts_End_Time;
        httpHeader = "Bearer " + String(authToken.access_token);

        http.begin(graphEndpoint);
        http.addHeader("Authorization", httpHeader);

        httpResponseCode = http.GET();

        Serial.print("Getting Shifts from Teams-Channel... ");
        if (httpResponseCode != 200)
        {
            Serial.printf("[ERROR]\n   --> HTTP-Response: %i\n\n", httpResponseCode);
            Serial.println(http.getString());
            http.end();
        }
        else
        {
            Serial.printf("[Done]\n   --> HTTP-Response: %i\n", httpResponseCode);
            shift.lastPoll = millis();
            graph_deserializeShifts();
            http.end();
            if (config.is_Armed)
            {
                Serial.printf("   --> Active Shift\n");

                timeinfo = *localtime(&shift.t_startDateTime);
                strftime(dateTimeString, sizeof(dateTimeString), "%d.%m.%Y %H:%M", &timeinfo);
                Serial.printf("      --> [Start]: %s\n", dateTimeString);

                timeinfo = *localtime(&shift.t_endDateTime);
                strftime(dateTimeString, sizeof(dateTimeString), "%d.%m.%Y %H:%M", &timeinfo);
                Serial.printf("      --> [End]:   %s\n\n", dateTimeString);
            }
            else
            {
                Serial.printf("   --> No active Shift\n\n");
            }
        }
        xSemaphoreGive(sem);
        delay(SHIFTS_POLLING_INTERVALL);
    }
}

void graph_deserializeShifts()
{
    // Allocate the JSON document
    JsonDocument doc;

    // Parse JSON object
    DeserializationError error = deserializeJson(doc, http.getString());
    if (error)
    {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
    }

    bool state = false;
    configTime(0, 0, "pool.ntp.org");
    for (JsonObject value : doc["value"].as<JsonArray>())
    {
        if (strcmp(value["userId"].as<const char *>(), shift.userId) == 0 && strcmp(value["id"].as<const char *>(), shift.id) != 0)
        {
            tm tm_startDateTime;
            tm tm_endDateTime;
            time_t startDateTime;
            time_t endDateTime;
            time_t currentTime;

            strptime(value["sharedShift"]["startDateTime"].as<const char *>(), "%Y-%m-%dT%H:%M:%S", &tm_startDateTime);
            strptime(value["sharedShift"]["endDateTime"].as<const char *>(), "%Y-%m-%dT%H:%M:%S", &tm_endDateTime);

            startDateTime = mktime(&tm_startDateTime);
            endDateTime = mktime(&tm_endDateTime);
            currentTime = time(nullptr);

            if (startDateTime <= currentTime && currentTime <= endDateTime)
            {
                state = true;
                shift.t_startDateTime = startDateTime;
                shift.t_endDateTime = endDateTime;
            }
        }
    }
    time_setTimezone();
    config.is_Armed = state;
    button_change = true;
}