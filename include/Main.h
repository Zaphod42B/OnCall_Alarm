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
  u_long token_request_millis = 0; // Timestamp when token was requested
  char token_type[32];
  const char scope[64] = "User.Read offline_access ChannelMessage.Read.All";
  int expires_in;
  char access_token[4096];
  char refresh_token[2048];
} AuthToken;

#endif
