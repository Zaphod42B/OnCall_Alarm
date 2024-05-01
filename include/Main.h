#ifndef Main_h
#define Main_h

// Initialize Config
typedef struct
{
  int audio_Volume = 10;
  int is_Armed = true;
  char teams_AppID[64];
  char teams_ClientSecret[64];
} Config;

#endif
