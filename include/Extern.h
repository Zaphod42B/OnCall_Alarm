#ifndef Extern_h
#define Extern_h

#include "Main.h"

// Create semaphore handle for Tasks
extern SemaphoreHandle_t sem; 

extern bool volume_change;
extern bool menu_change;
extern bool button_change;
extern int page;

extern Config config;
extern AuthToken authToken;
extern TeamsMsg teamsMsg;
extern DeviceAuth deviceAuth;

#endif
