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

typedef struct
{
  bool isValid = false;
  char token_type[32];
  int expires_in;
  char access_token[256];
} AuthToken;

#endif
