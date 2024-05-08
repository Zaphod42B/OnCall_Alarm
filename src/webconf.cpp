#include <string.h>

#include "WebConf.h"

// -- Initial name of the Thing. Used e.g. as SSID of the own Access Point.
const char thingName[] = "OnCallAlarm";

// -- Initial password to connect to the Thing, when it creates an own Access Point.
const char wifiInitialApPassword[] = "Alarm123!";

// -- Configuration specific key. The value should be modified if config structure was changed.
#define CONFIG_VERSION "1.4"

DNSServer dnsServer;
WebServer server(80);

char string_teams_TenantID[STRING_LEN];
char string_teams_AppID[STRING_LEN];
char string_teams_TeamID[STRING_LEN];
char string_teams_ChannelID[STRING_LEN];
char string_teams_UserID[STRING_LEN];

char string_poll_TimerTeamsMsg[4];
char string_poll_TimerShifts[4];
char string_poll_TimerNtp[4];

char string_refreshToken[2048];
char string_refreshToken_lastSave[32];

IotWebConf iotWebConf(thingName, &dnsServer, &server, wifiInitialApPassword, CONFIG_VERSION);

IotWebConfParameterGroup teams_ConfigGroup = IotWebConfParameterGroup("iwcTeamsConfig", "Teams configuration");
IotWebConfTextParameter teams_TenantID = IotWebConfTextParameter("Tenant ID", "iwcTenantID", string_teams_TenantID, STRING_LEN);
IotWebConfTextParameter teams_AppID = IotWebConfTextParameter("Application ID", "iwcAppID", string_teams_AppID, STRING_LEN);
IotWebConfTextParameter teams_TeamID = IotWebConfTextParameter("Teams ID", "iwcTeamsID", string_teams_TeamID, STRING_LEN);
IotWebConfTextParameter teams_ChannelID = IotWebConfTextParameter("Channel ID", "iwcChannelID", string_teams_ChannelID, STRING_LEN);
IotWebConfTextParameter teams_UserID = IotWebConfTextParameter("User ID", "iwcUserID", string_teams_UserID, STRING_LEN);

IotWebConfParameterGroup poll_TimerGroup = IotWebConfParameterGroup("iwcPollTimer", "Poll Timer");
IotWebConfNumberParameter poll_TimerTeamsMsg = IotWebConfNumberParameter("Teams Message", "iwcTeamsMsg", string_poll_TimerTeamsMsg, 4, "10", "1...999", "min='5' max='999' step='1'");
IotWebConfNumberParameter poll_TimerShifts = IotWebConfNumberParameter("Shifts", "iwcShifts", string_poll_TimerShifts, 4, "10", "1...999", "min='5' max='999' step='1'");
IotWebConfNumberParameter poll_TimerNtp = IotWebConfNumberParameter("NTP", "iwcNtp", string_poll_TimerNtp, 4, "300", "1...999", "min='5' max='999' step='1'");

IotWebConfPasswordParameter refreshToken = IotWebConfPasswordParameter("Reauth Token", "iwcReauthToken", string_refreshToken, 2048);
IotWebConfNumberParameter refreshToken_lastSave = IotWebConfNumberParameter("Reauth Token Last Save", "iwcReauthTokenLastSave", string_refreshToken_lastSave, 32);

void webconf_init()
{
    teams_ConfigGroup.addItem(&teams_TenantID);
    teams_ConfigGroup.addItem(&teams_AppID);
    teams_ConfigGroup.addItem(&teams_TeamID);
    teams_ConfigGroup.addItem(&teams_ChannelID);
    teams_ConfigGroup.addItem(&teams_UserID);

    poll_TimerGroup.addItem(&poll_TimerTeamsMsg);
    poll_TimerGroup.addItem(&poll_TimerShifts);
    poll_TimerGroup.addItem(&poll_TimerNtp);

    iotWebConf.addParameterGroup(&teams_ConfigGroup);
    iotWebConf.addParameterGroup(&poll_TimerGroup);
    iotWebConf.addHiddenParameter(&refreshToken);
    iotWebConf.addHiddenParameter(&refreshToken_lastSave);

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
    s += "<title>OnCallAlarm config</title></head><body>";
    s += "<h3>Connected to WiFi:</h3>";
    s += "<ul>";
    s += "<li><b>SSID: </b>";
    s += WiFi.SSID();
    s += "<li><b>RSSI: </b>";
    s += WiFi.RSSI();
    s += "</ul>";
    s += "<h3>Teams configuration:</h3>";
    s += "<ul>";
    s += "<li><b>Tenant ID: </b>";
    s += string_teams_TenantID;
    s += "<li><b>Application ID: </b>";
    s += string_teams_AppID;
    s += "<li><b>Team ID: </b>";
    s += string_teams_TeamID;
    s += "<li><b>Channel ID: </b>";
    s += string_teams_ChannelID;
    s += "<li><b>User ID: </b>";
    s += string_teams_UserID;
    s += "</ul>";
    s += "<h3>Poll Timer:</h3>";
    s += "<ul>";
    s += "<li><b>Temas Message: </b>";
    s += string_poll_TimerTeamsMsg;
    s += "<li><b>Shift: </b>";
    s += string_poll_TimerShifts;
    s += "<li><b>NTP: </b>";
    s += string_poll_TimerNtp;
    s += "</ul>";
    s += "Go to <a href='config'>configure page</a> to change values.";
    s += "</body></html>\n";

    server.send(200, "text/html", s);
}

void webconf_configSaved()
{
    Serial.println("Configuration was updated.");
}