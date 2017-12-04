#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
// Includes para sockets
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/un.h>

#include "fredchatserver.h"
#include "fredSocket.h"
#include "fredSocketListener.h"
#include "chatuser.h"
#include "chatmessage.h"

#define WELCOME_TEXT "\nBienvenido al chat de fred\nversion 1.0\nCopyright© 2016 Alfredo Orozco\nalfredoopa@gmail.com\n####################################################\n#                  CHAT DE FRED ;D                 #\n####################################################\n\n"

#define SERVER_PORT 9696
#define MAXPENDING 5

#define TRUE 1
#define FALSE 0


void fredChatOnRecieveMessage(int server_scoket, int client_socket);

int fredChatServerInit(int server_port, int max_pending);
int fredChatServerClose(int server_socket);
struct User* fredChatAcceptUserConnection(int server_fd);
int fredChatSendWelcomeMessage(userID_t user_id);
int fredChatRegisterUser(struct User* user);
int fredChatSendMessage(userID_t user_id, struct Message* msg);
int fredChatRecieveMessage(userID_t user_id, struct Message* msg) ;
int fredChatBroadcastMessage(userID_t user_sender_id, struct Message *msg);


int fredChatBroadcastMessage(userID_t user_sender_id, struct Message *msg) {

    int* fd_list, fd_count, i;
    fd_list =  chatUserGetFDList(&fd_count);

    for(i = 0; i< fd_count; i++)
        if(chatUserGetIDbyFD(fd_list[i]) != user_sender_id)
            fredChatSendMessage(chatUserGetIDbyFD(fd_list[i]), msg);
}

int fredChatRegisterUser(struct User* user) {

    int id;
    struct Message msg;
    id = chatUserAdd(user);

    printf("\n************************************\n");
    printf  ("* Conectando Usuario:              *\n");
    printf  ("************************************\n");
    if(id != -1) {
        printf("Usuario conectado en IP-> %s/%d  ID-> %d\n", user->ip, user->port, id);
        printf("Enviando mensaje de bienvenida...\n");
        if(fredChatSendWelcomeMessage(id))
            printf("Mensaje de bienvenida enviado.\n");
        else
            printf("Error al enviar el mensaje de bienvenida.\n");

        sprintf(msg.buffer,"El usuario \"%s\" se ha conectado",user->username);
        msg.type=MSG_INFO;
        msg.buffer_len = strlen(msg.buffer);
        fredChatBroadcastMessage(id, &msg);

        return id;
    }
    else{
        printf("Conexión rechazada, no se pueden aceptar mas usuarios al chat, el servidor está lleno.\n");
        close(user->socket_fd);
    }

    return -1;
}

int fredChatSendMessage(userID_t user_id, struct Message* msg) {

    struct User *usr = chatUserGetUserByID(user_id);
    if(usr)
    {
        if(!fredSocketWrite(usr->socket_fd , (char*)msg, sizeof(struct Message))) {
            printf("Error enviando mensaje al cliente\n");
            return 0;
        }
    }

    return 1;
}

int fredChatRecieveMessage(userID_t user_id, struct Message* msg) {

    int count_read = -1;
    struct User *usr = chatUserGetUserByID(user_id);

    if(usr)
        count_read = fredSocketRead(usr->socket_fd, (char*)msg, sizeof(struct Message));

    return count_read;

}

struct User* fredChatAcceptUserConnection(int server_fd) {

    int user_socket;
    struct User *user;
    struct sockaddr_in address;     // Estructura para guardar la dirección del cliente
    int addrlen;

    addrlen = sizeof(address);
    user_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t *)&addrlen);

    if(user_socket == -1) {
        printf("Error caeptando conexión de usuario\n");
        return NULL;
    }

    // Agrega un nuevo usuario y guarda su ID de usuario.
    user = (struct User*) malloc(sizeof(struct User));
    memset(user, 0, sizeof(struct User));

    strcpy(user->username, "");
    strcpy(user->ip, inet_ntoa(address.sin_addr));
    user->port=ntohs(address.sin_port);
    user->socket_fd = user_socket;

    return user;
}

int fredChatSendWelcomeMessage(userID_t user_id) {
    struct Message m;

    memset(&m, 0, sizeof(struct Message));
    strcpy(m.buffer, WELCOME_TEXT);
    m.type = MSG_INFO;
    m.buffer_len = strlen(WELCOME_TEXT) + 1;

    return fredChatSendMessage(user_id, &m);

}

void fredChatOnRecieveMessage(int server_socket, int client_socket) {
    int id,i;
    struct User *user;
    int msg_len;
    struct Message msg, msg_aux;
    char buff[MSG_BUFFER_SIZE];
    // Si fuen una petición a una conexión del servidor, atiende
    // al usuario
    if(client_socket == server_socket){

        user = fredChatAcceptUserConnection(client_socket);
        if(user)
            id = fredChatRegisterUser(user);

    }
    else {

        user = chatUserGetUserByFD(client_socket);
        id = user->id;
        memset(&msg, 0, sizeof(struct Message));
        msg_len = fredChatRecieveMessage(id, &msg);

        if(msg_len == -1){
            printf("Error recibiendo mensaje.\n");
            return;
        }

        if(msg_len == 0) {
            printf("\n************************************\n");
            printf  ("* Desconexión de usuario:          *\n");
            printf  ("************************************\n");
            printf("Usuario \"%s\" desconectado en %s/%d, ID:%d\n",user->username, user->ip, user->port, id);
            sprintf(msg_aux.buffer,"El usuario \"%s\" ha salido del chat.", user->username);
            msg_aux.type=MSG_INFO;
            msg_aux.buffer_len = strlen(msg_aux.buffer);
            fredChatBroadcastMessage(id, &msg_aux);

            close(user->socket_fd);
            chatUserRemove(id);
        }
        else{
            printf("\n************************************\n");
            printf  ("* Nuevo Mensaje:                   *\n");
            printf  ("************************************\n");
            printf("Mensaje recibido de:\n\nuser: %s\nip: %s\nport: %d\nid: %d\n%s",user->username, user->ip, user->port, id, msg.buffer );

            switch (msg.type) {
                case MSG_TEXT:
                    printf("\n************************************\n");
                    printf  ("* Envio de Mensaje:                *\n");
                    printf  ("************************************\n");
                    printf("Reenviando mensaje a todos los usuarios.\n");
                    // Agrega el nombre de usuario al mensaje.
                    memset(buff, 0, MSG_BUFFER_SIZE);
                    strcpy(buff, user->username);
                    strcat(buff, ": ");
                    strcat(buff, msg.buffer);
                    strcpy(msg.buffer,buff);
                    msg.type = MSG_TEXT;
                    msg.buffer_len = strlen(msg.buffer);

                    fredChatBroadcastMessage(id, &msg);
                break;
                case MSG_USERNAME:
                    sprintf(msg_aux.buffer,"El usuario \"%s\" ha cambiado su NIP a \"%s\"", user->username, msg.buffer);
                    msg_aux.type=MSG_INFO;
                    msg_aux.buffer_len = strlen(msg_aux.buffer);
                    fredChatBroadcastMessage(id, &msg_aux);

                    chatUserSetUsername(id, msg.buffer);
                    printf("\n************************************\n");
                    printf  ("* Cambio de username               *\n");
                    printf  ("************************************\n");
                    printf("Nuevo nombre de usuario para IP: %s/%d, ID: %d\n", user->ip, user->port, id);
                    printf("Nuevo alias: %s\n", user->username);
                break;
            }
        }
    }
}

int fredChatServerInit(int server_port, int max_pending) {

    int _srvSock = fredSocketCreateINETTCPServer(server_port, max_pending);

    if(_srvSock == -1)
    {
        printf("Error iniciando el servidor del Chat de Fred :'(\n");
        return -1;
    }
    printf("Chat de fred corriendo en el puerto %d  :D\n", server_port);

    chatUserInit();

    fredSocketServerListenerInit(_srvSock, fredChatOnRecieveMessage);
    return _srvSock;

}

int main(int argc, char const *argv[]) {

    int* socket_list;
    int socket_count;
    int srvSock;

    if(fredChatServerInit(SERVER_PORT, MAXPENDING) == -1)
        return 1;

    while (TRUE) {
        socket_list = chatUserGetFDList(&socket_count);
        fredSocketListenerUpdate(socket_list, socket_count);
    }


    return 0;
}
