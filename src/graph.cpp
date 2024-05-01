#include "Graph.h"
#include "WebConf.h"
#include "Extern.h"

HTTPClient http;

AuthToken authToken;

void graph_getAuthToken()
{
    String graphEndpoint = "https://login.microsoftonline.com/" + String(string_teams_TenantID) + "/oauth2/v2.0/token";
    String httpRequestData = "client_id=" + String(string_teams_AppID) + "&scope=https%3A%2F%2Fgraph.microsoft.com%2F.default&client_secret=" + String(string_teams_ClientSecret) + "&grant_type=client_credentials";

    http.begin(graphEndpoint);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    int httpResponseCode = http.POST(httpRequestData);

    Serial.print("Getting Auth Token vom Graph... ");
    if (httpResponseCode == 200)
    {
        Serial.printf("[SUCCESS] HTTP-Response: %i\n", httpResponseCode);
        Serial.println(http.getString());
        authToken.isValid = true;

    }
    else
    {
        Serial.printf("[ERROR] HTTP-Response: %i\n", httpResponseCode);
    }

    http.end();
}
