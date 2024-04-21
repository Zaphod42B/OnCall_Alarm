#ifndef Helper_h
#define Helper_h

#include <string>
#include <WiFi.h>

extern WiFiClient wifiClient;

void wifi_connect();
void wifi_reconnect();
void time_update();

#endif