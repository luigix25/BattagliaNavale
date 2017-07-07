//Libreria Send e Receive senza dimensione
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#include <string.h>

#define true 1
#define false 0

//enum commands {HELP_COMMAND,WHO_COMMAND,CONNECT_COMMAND,CONNECT_ANSWER,QUIT_COMMAND,LOGIN_COMMAND,DISCONNECT_COMMAND};
//enum protocol_login {LOGIN_OK,LOGIN_FAIL};
//enum protocol_connect {CONNECT_NOUSER,CONNECT_BUSY,CONNECT_REFUSED,CONNECT_OK,CONNECT_REQ,CONNECT_DATA,CONNECT_ACPT,CONNECT_RFSD};

enum vars {HELP_COMMAND,WHO_COMMAND,CONNECT_COMMAND,CONNECT_ANSWER,QUIT_COMMAND,
LOGIN_COMMAND,DISCONNECT_COMMAND,LOGIN_OK,LOGIN_FAIL, CONNECT_NOUSER,CONNECT_BUSY,CONNECT_REFUSED,CONNECT_OK,CONNECT_REQ,CONNECT_DATA,CONNECT_ACPT,CONNECT_RFSD,
WON_RETIRED,SHOT_DATA,RESPONSE_SHOT,WATER,HIT,YOU_WON,END_GAME,NOTIFY_OPP_TIMEOUT,YOU_TIMEOUT,OPP_DISCONNECTED_TCP

};

enum user_status {FREE,CONNECTING,BUSY};

extern const char *commands_list[4];
extern const char *commands_list_game[4];

int sendInt(int,int);
int sendString(int,char *);
int sendUDPInt(int,struct sockaddr_in*,int);
int sendUDPString(int,char*,struct sockaddr_in *);

char* recvString(int);
char* recvUDPString(int,struct sockaddr_in *);
int recvInt(int,int*);
int recvUDPInt(int,struct sockaddr_in*,int*);





