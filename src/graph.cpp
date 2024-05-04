#include "Graph.h"
#include "WebConf.h"
#include "Extern.h"
#include "Display.h"

#define TEAMS_POLLING_INTERVALL 60000
#define TOKEN_MIN_LIFETIME (authToken.expires_in * 0.75) * 1000 //

HTTPClient http;

AuthToken authToken;
TeamsMsg teamsMsg;
DeviceAuth deviceAuth;

String graphEndpoint;
String httpRequestData;
String httpHeader;

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

    doc.clear();

    Serial.println(String(deviceAuth.message));
    Serial.printf("Waiting for User Authorization...");

    userAuthenticated = false;
    while (!userAuthenticated)
    {
        delay(deviceAuth.interval * 1000);

        String graphEndpoint = "https://login.microsoftonline.com/" + String(string_teams_TenantID) + "/oauth2/v2.0/token";
        String httpRequestData = "grant_type=urn:ietf:params:oauth:grant-type:device_code&client_id=" + String(string_teams_AppID) + "&device_code=" + String(deviceAuth.device_code);

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

        int httpResponseCode = http.GET();

        Serial.print("Getting last Message from Teams-Channel... ");
        if (httpResponseCode != 200)
        {
            Serial.printf("[ERROR]\n   --> HTTP-Response: %i\n\n", httpResponseCode);
            Serial.println(http.getString());
            http.end();
        }

        Serial.printf("[Done]\n   --> HTTP-Response: %i\n", httpResponseCode);
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
        teamsMsg.lastPoll = millis();
        teamsMsg.Id = msgIndex["id"].as<unsigned long long>();
        strlcpy(teamsMsg.createdDateTime, msgIndex["createdDateTime"], sizeof(teamsMsg.createdDateTime));
        strlcpy(teamsMsg.subject, msgIndex["subject"], sizeof(teamsMsg.subject));
        strlcpy(teamsMsg.body, msgIndex["body"]["content"], sizeof(teamsMsg.body));
        new_message = true;
    }
}