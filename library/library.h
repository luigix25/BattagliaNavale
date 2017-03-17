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

enum commands {HELP_COMMAND,WHO_COMMAND,CONNECT_COMMAND,QUIT_COMMAND,LOGIN_COMMAND};
enum protocol_login {LOGIN_OK,LOGIN_FAIL};
enum protocol_connect {CONNECT_NOUSER,CONNECT_BUSY,CONNECT_REFUSED,CONNECT_OK,CONNECT_REQ};
enum user_status {FREE,CONNECTING,BUSY};

extern const char *commands_list[4];

int sendInt(int,int);
int sendString(int,char *);

char* recvString(int);
int recvInt(int,int*);

