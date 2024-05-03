#ifndef Main_h
#define Main_h

// Initialize Config
typedef struct
{
  int audio_Volume = 10;
  int is_Armed = true;
} Config;

typedef struct
{
  u_long token_request_millis = 0; // Timestamp when token was requested
  int expires_in = 0;
  int interval;
  char device_code[1024];
  char user_code[16];
  char message[128];
} DeviceAuth;

typedef struct
{
  u_long token_request_millis = 0; // Timestamp when token was requested
  char token_type[32];
  const char scope[64] = "User.Read offline_access ChannelMessage.Read.All";
  int expires_in = 0;
  char access_token[4096];
  char refresh_token[2048];
} AuthToken;

typedef struct
{
  u_long lastPoll = 0;
  unsigned long long Id = 0;
  char subject[128];
  char body[512];
  char createdDateTime[32];
} TeamsMsg;

#endif
