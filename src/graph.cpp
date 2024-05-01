#include "Graph.h"
#include "WebConf.h"
#include "Extern.h"

HTTPClient http;

AuthToken authToken;

#define TOKEN_MIN_LIFETIME (authToken.expires_in * 0.50) * 1000 //

void graph_getAuthToken()
{
    String graphEndpoint = "https://login.microsoftonline.com/" + String(string_teams_TenantID) + "/oauth2/v2.0/devicecode";
    String httpRequestData = "client_id=" + String(string_teams_AppID) + "&scope=" + authToken.scope + "&client_secret=" + String(string_teams_ClientSecret) + "&grant_type=client_credentials";

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
        }
        http.end();
    }
}

void graph_checkAuthToken()
{
    if (authToken.token_request_millis + TOKEN_MIN_LIFETIME < millis())
    {
        graph_getAuthToken();
    }
}

void graph_pollTeamsChannel()
{
    String graphEndpoint = "https://graph.microsoft.com/v1.0/teams/b070b0e9-ed57-4927-843e-14e4bac88141/channels/19%3A97a486862fb143f192e7e5a15b0cdf68%40thread.tacv2/messages?top=1";
    // String graphEndpoint = "https://graph.microsoft.com/v1.0/teams/987516c1-4d73-4dda-a04e-99ae3e46e412/channels/19%3Au8TSdhV2QtC5AoothUBBMHvoTI5mzhyyyv5YoQzPv1I1%40thread.tacv2/messages?top=1";

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

    Serial.print("ID: ");
    Serial.println(msgIndex["id"].as<String>());
    Serial.print("Created: ");
    Serial.println(msgIndex["createdDateTime"].as<String>());
    Serial.print("Subject: ");
    Serial.println(msgIndex["subject"].as<String>());
    Serial.print("Message: ");
    Serial.println(msgIndex["body"]["content"].as<String>());
}