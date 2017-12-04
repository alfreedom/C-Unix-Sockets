
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include "fredSocket.h"
#include "fredSocketListener.h"
#include "chatmessage.h"

pthread_t thread_id;

void *Listen() {
    while (1) {
        if(fredSocketListenerUpdate(NULL, 0) == -1)
            break;
    }
    return NULL;
}
void OnRecieveMsg(int serve_fd, int client_fd) {

    struct Message msg;
    // Lee mensaje recibido
    if(fredSocketRead(client_fd, (char*)&msg, sizeof(struct Message)) == 0)
    {
        printf("\nConexion terminada\n");
        exit(1);
    }
    if(msg.type == MSG_TEXT || msg.type == MSG_INFO){
        printf("%s\n", msg.buffer);

    }

}

int main(int argc, char const *argv[]) {


    int clientSock, srvPort;
    char *p_ptr, *end_ptr;
    char *endptr;
    struct Message msg;
    int salir=0;
    int* sock_list;

    if(argc <3) {
        printf("Uso: %s <serverIP> <serverPort> [username]\n",argv[0]);
        return 0;
    }
    p_ptr = (char*)malloc(strlen(argv[2])+1);
    strcpy(p_ptr, argv[2]);
    srvPort = (int)strtol(p_ptr, &endptr, 10);


    if (p_ptr == end_ptr || srvPort <=0 )
    {
        printf("Error, el puerto debe ser un numero entre 0 y 65535\n");
        return 1;
    }


    clientSock = fredSocketCreateINETTCPClient((char*)argv[1], srvPort);

    if(clientSock == -1)
        printf("Error al crear la conexión con el servidor\n");

    // Lee mensaje de bienvenida
    if(fredSocketRead(clientSock, (char*)&msg, sizeof(struct Message))==0)
    {
        printf("\nConexion terminada\n");
        return 0;;
    }

    if(msg.type == MSG_INFO)
    printf("%s\n", msg.buffer);

    msg.type=MSG_USERNAME;
    if(argc >3)
        strcpy(msg.buffer, argv[3]);
    else
        strcpy(msg.buffer, "Anonimussss\0");

    msg.buffer_len = strlen(msg.buffer);
    if(fredSocketWrite(clientSock, (char*)&msg, sizeof(struct Message)) == 0)
    {
        printf("\nConexion terminada\n");
        return 0;;
    }
    printf(">> Te has conectado como: \"%s\"\n\n",msg.buffer);
    fredSocketClientListenerInit(clientSock, OnRecieveMsg, 500000);
    pthread_create(&thread_id, NULL, &Listen, NULL);

    msg.type=MSG_TEXT;
    while(1){
        printf("\n");
        gets(msg.buffer);
        if(!strcmp(msg.buffer, "adiosbyebye"))
            break;
        printf("Tu: %s",msg.buffer );
        msg.buffer_len = strlen(msg.buffer);
        if(fredSocketWrite(clientSock, (char*)&msg, sizeof(struct Message)) == 0) {
            printf("\nConexion terminada\n");
            break;
        }
    }


    close(clientSock);
    return 0;
}
