#include "Graph.h"
#include "WebConf.h"
#include "Extern.h"
#include "Display.h"

HTTPClient http;

AuthToken authToken;
TeamsMsg teamsMsg;

#define TOKEN_MIN_LIFETIME (authToken.expires_in * 0.75) * 1000 //

void graph_loadReauthToken()
{
    strlcpy(authToken.refresh_token, string_reauthToken, sizeof(string_reauthToken));
}

void graph_getAuthToken()
{
    String graphEndpoint = "https://login.microsoftonline.com/" + String(string_teams_TenantID) + "/oauth2/v2.0/devicecode";
    String httpRequestData = "client_id=" + String(string_teams_AppID) + "&scope=" + authToken.scope;

    http.begin(graphEndpoint);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    int httpResponseCode = http.POST(httpRequestData);

    Serial.print("Device Authorization... ");
    if (httpResponseCode != 200)
    {
        Serial.printf("[ERROR] HTTP-Response: %i\n", httpResponseCode);
        Serial.println(http.getString());
        http.end();
        return;
    }

    Serial.printf("[SUCCESS] HTTP-Response: %i\n", httpResponseCode);

    // Allocate the JSON document
    JsonDocument deviceAuth;

    // Parse JSON object
    DeserializationError error = deserializeJson(deviceAuth, http.getString());
    if (error)
    {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        http.end();
        return;
    }
    http.end();

    Serial.println(deviceAuth["message"].as<String>());
    Serial.print("Waiting for User Authorization...");

    bool userAuthenticated = false;
    while (!userAuthenticated)
    {
        delay(deviceAuth["interval"].as<int>() * 1000);

        String graphEndpoint = "https://login.microsoftonline.com/" + String(string_teams_TenantID) + "/oauth2/v2.0/token";
        String httpRequestData = "grant_type=urn:ietf:params:oauth:grant-type:device_code&client_id=" + String(string_teams_AppID) + "&device_code=" + deviceAuth["device_code"].as<String>();

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
        else
        {
            Serial.println("[SUCCESS]");
            userAuthenticated = true;

            authToken.token_request_millis = millis();
            authToken.expires_in = userAuth["expires_in"];
            strlcpy(authToken.token_type, userAuth["token_type"], sizeof(authToken.token_type));
            strlcpy(authToken.access_token, userAuth["access_token"], sizeof(authToken.access_token));
            strlcpy(authToken.refresh_token, userAuth["refresh_token"], sizeof(authToken.refresh_token));
            strlcpy(string_reauthToken, authToken.refresh_token, sizeof(authToken.refresh_token));
            iotWebConf.saveConfig();
        }
        http.end();
    }
}

bool graph_reAuthToken()
{
    String graphEndpoint = "https://login.microsoftonline.com/" + String(string_teams_TenantID) + "/oauth2/v2.0/token";
    String httpRequestData = "client_id=" + String(string_teams_AppID) + "&scope=" + authToken.scope + "&refresh_token=" + authToken.refresh_token + "&grant_type=refresh_token";

    http.begin(graphEndpoint);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    int httpResponseCode = http.POST(httpRequestData);

    Serial.print("Refreshing Auth Token... ");
    if (httpResponseCode != 200)
    {
        Serial.printf("[ERROR] HTTP-Response: %i\n", httpResponseCode);
        Serial.println(http.getString());
        http.end();
        return false;
    }

    Serial.printf("[SUCCESS] HTTP-Response: %i\n", httpResponseCode);

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
    iotWebConf.saveConfig();

    http.end();

    return true;
}

void graph_checkAuthToken()
{
    if (authToken.token_request_millis + TOKEN_MIN_LIFETIME < millis())
    {
        if (!graph_reAuthToken())
        {
            graph_getAuthToken();
        }
    }
}

void graph_pollTeamsChannel()
{
    String graphEndpoint = "https://graph.microsoft.com/v1.0/teams/" + String(string_teams_TeamID) + "/channels/" + String(string_teams_ChannelID) + "/messages?top=1";

    http.begin(graphEndpoint);
    http.addHeader("Authorization", "Bearer " + String(authToken.access_token));

    int httpResponseCode = http.GET();

    Serial.print("Getting last Message from Teams-Channel... ");
    if (httpResponseCode != 200)
    {
        Serial.printf("[ERROR] HTTP-Response: %i\n", httpResponseCode);
        Serial.println(http.getString());
        http.end();
        return;
    }

    Serial.printf("[SUCCESS] HTTP-Response: %i\n", httpResponseCode);

    // Allocate the JSON document
    JsonDocument msgArray;

    // Parse JSON object
    DeserializationError error = deserializeJson(msgArray, http.getString());
    if (error)
    {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        http.end();
        return;
    }
    http.end();

    JsonObject msgIndex = msgArray["value"][0];

    teamsMsg.lastPoll = millis();
    teamsMsg.Id = msgIndex["id"].as<int>();
    strlcpy(teamsMsg.subject, msgIndex["subject"], sizeof(teamsMsg.subject));
    strlcpy(teamsMsg.body, msgIndex["body"]["content"], sizeof(teamsMsg.subject));

    Serial.print("ID: ");
    Serial.println(teamsMsg.Id);
    Serial.print("Created: ");
    Serial.println(teamsMsg.createdDateTime);
    Serial.print("Subject: ");
    Serial.println(teamsMsg.subject);
    Serial.print("Message: ");
    Serial.println(teamsMsg.body);

    sprite.setColorDepth(8);
    sprite.createSprite(220, 30);
    sprite.fillSprite(TFT_BLACK);
    sprite.setTextColor(TFT_WHITE);
    sprite.setTextDatum(TL_DATUM);
    sprite.setFreeFont(FSSB18);
    sprite.drawString(teamsMsg.subject, 0, 0, GFXFF);
    sprite.pushSprite(30, 30);
    sprite.deleteSprite();

    sprite.setColorDepth(8);
    sprite.createSprite(285, 105);
    sprite.fillSprite(TFT_BLACK);
    sprite.setTextColor(TFT_WHITE);
    sprite.setTextDatum(TL_DATUM);
    sprite.setTextFont(2);

    sprite.setCursor(0, 5);
    sprite.println(teamsMsg.body);
    sprite.println(String((authToken.token_request_millis / 1000 + authToken.expires_in) - millis() / 1000));

    sprite.pushSprite(30, 60);
    sprite.deleteSprite();
}