#ifndef WebConf_h
#define WebConf_h

#include <IotWebConf.h>

extern DNSServer dnsServer;
extern WebServer server;

extern IotWebConf iotWebConf;

void webconf_init();
void webconf_handleRoot();
void webconf_configSaved();

#endif