#ifndef Graph_h
#define Graph_h

#include <ArduinoJson.h>
#include <WiFi.h>
#include <HTTPClient.h>

void graph_loadReauthToken();
void graph_getAuthToken();
bool graph_reAuthToken();
void graph_checkAuthToken();
void graph_pollTeamsChannel();

#endif