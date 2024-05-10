#ifndef WebConf_h
#define WebConf_h

#include <IotWebConf.h>
#include <IotWebConfUsing.h>

#define STRING_LEN 64

extern DNSServer dnsServer;
extern WebServer server;

extern IotWebConf iotWebConf;

extern char string_teams_TenantID[STRING_LEN];
extern char string_teams_AppID[STRING_LEN];
extern char string_teams_TeamID[STRING_LEN];
extern char string_teams_ChannelID[STRING_LEN];
extern char string_teams_UserID[STRING_LEN];

extern char string_poll_TimerTeamsMsg[4];
extern char string_poll_TimerShifts[4];
extern char string_poll_TimerNtp[4];

extern char string_refreshToken[2048];
extern char string_refreshToken_lastSave[32];

void webconf_init();
void webconf_handleRoot();
void webconf_configSaved();

#endif