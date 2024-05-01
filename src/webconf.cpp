#include <string.h>

#include "WebConf.h"

// -- Initial name of the Thing. Used e.g. as SSID of the own Access Point.
const char thingName[] = "OnCallAlarm";

// -- Initial password to connect to the Thing, when it creates an own Access Point.
const char wifiInitialApPassword[] = "Alarm123!";

// -- Configuration specific key. The value should be modified if config structure was changed.
#define CONFIG_VERSION "1.1"

DNSServer dnsServer;
WebServer server(80);

char string_teams_TenantID[STRING_LEN];
char string_teams_AppID[STRING_LEN];
char string_teams_ClientSecret[STRING_LEN];

char string_teams_User_Name[STRING_LEN];
char string_teams_User_Password[STRING_LEN];

IotWebConf iotWebConf(thingName, &dnsServer, &server, wifiInitialApPassword, CONFIG_VERSION);

IotWebConfParameterGroup teams_ConfigGroup = IotWebConfParameterGroup("iwcTeamsConfig", "Teams configuration");
IotWebConfTextParameter teams_TenantID = IotWebConfTextParameter("Tenant ID", "iwcTenantID", string_teams_TenantID, STRING_LEN);
IotWebConfTextParameter teams_AppID = IotWebConfTextParameter("Application ID", "iwcAppID", string_teams_AppID, STRING_LEN);

void webconf_init()
{
    teams_ConfigGroup.addItem(&teams_TenantID);
    teams_ConfigGroup.addItem(&teams_AppID);

    iotWebConf.addParameterGroup(&teams_ConfigGroup);

    iotWebConf.setConfigSavedCallback(&webconf_configSaved);
    iotWebConf.getApTimeoutParameter()->visible = true;

    iotWebConf.init();
    iotWebConf.setApTimeoutMs(0);

    // -- Set up required URL handlers on the web server.
    server.on("/", webconf_handleRoot);
    server.on("/config", []
              { iotWebConf.handleConfig(); });
    server.onNotFound([]()
                      { iotWebConf.handleNotFound(); });
}

void webconf_handleRoot()
{
    // -- Let IotWebConf test and handle captive portal requests.
    if (iotWebConf.handleCaptivePortal())
    {
        // -- Captive portal request were already served.
        return;
    }
    String s = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/>";
    s += "<title>IotWebConf 03 Custom Parameters</title></head><body>";
    s += "<h3>Teams configuration:</h3>";
    s += "<ul>";
    s += "<li><b>Tenant ID: </b>";
    s += string_teams_TenantID;
    s += "<li><b>Application ID: </b>";
    s += string_teams_AppID;
    s += "</ul>";
    s += "Go to <a href='config'>configure page</a> to change values.";
    s += "</body></html>\n";

    server.send(200, "text/html", s);
}

void webconf_configSaved()
{
    Serial.println("Configuration was updated.");
}