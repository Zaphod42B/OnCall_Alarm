#ifndef Graph_h
#define Graph_h

#include <ArduinoJson.h>
#include <HTTPClient.h>

void graph_loadReauthToken();
void graph_getAuthToken();
bool graph_reAuthToken();
void graph_checkAuthToken(void *parameter);
void graph_pollTeamsChannel(void *parameter);
void graph_deserializeTeamsMsg(const String &payload);
void graph_pollShifts(void *parameter);
void graph_deserializeShifts(const String &payload);
void graph_handleHttpError(const String &payload);

#endif