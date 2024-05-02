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
extern char string_reauthToken[2048];

void webconf_init();
void webconf_handleRoot();
void webconf_configSaved();

#endif