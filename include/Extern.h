#ifndef Extern_h
#define Extern_h

#include "Main.h"

// Create semaphore handle for Tasks
extern SemaphoreHandle_t sem; 

extern bool brightness_change;
extern bool menu_change;
extern bool button_change;
extern bool new_message;
extern int page;
extern bool userAuthenticated;

extern tm timeinfo;

extern Config config;
extern AuthToken authToken;
extern TeamsMsg teamsMsg;
extern DeviceAuth deviceAuth;

#endif
