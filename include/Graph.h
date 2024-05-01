#ifndef Graph_h
#define Graph_h

#include <ArduinoJson.h>
#include <WiFi.h>
#include <HTTPClient.h>

void graph_getAuthToken();
void graph_checkAuthToken();
void graph_pollTeamsChannel();

#endif