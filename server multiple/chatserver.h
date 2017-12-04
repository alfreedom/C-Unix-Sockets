#ifndef _CHATSERVER_H_
#define _CHATSERVER_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TRUE 1
#define FALSE 0

#define WELCOME_TEXT "Bienvenido al chat de Fred\n V 1.0\n Copyright© 2016 Alfredo Orozco de la Paz\n"

#define MAX_USERS 4
#define MESSAGE_SIZE 1025
#define SERVER_PORT 8765

typedef int userID_t;

enum {
    MSG_USERNAME,
    MSG_TEXT,
    MSG_INFO,
    MSG_ERROR
} MessageType;

struct Message {
    int type;
    char buffer[1025];
    int buffer_len;
};


struct User {
    char username[30];
    char ip[30];
    int port;
    int socket_fd;
};


int chatServerInit(int server_port);
int chatServerClose(int server_fd);
int chatServerRun();
userID_t chatServerAddUser(struct User* user);
int chatServerRemoveUser(userID_t user_id);
int chatServerBroadcastMessage(struct Message* msg);
int chatServerSendMessage(userID_t user_id, struct Message* msg);
void _chatServerUpdateWatcher();
int chatServerRecieveMessage(userID_t user_id, struct Message* msg);
int sendWelcomeMessage(userID_t user_id);
int setUserName(userID_t user_id, char* new_name);
userID_t getUserIDbyFD(int socket_fd);

#endif // _CHATSERVER_H_
