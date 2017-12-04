
#ifndef _FREDSOCKETLISTENER_H_
#define _FREDSOCKETLISTENER_H_


struct RecvMessage {
    int socket_fd;
    char* msg;
    int msg_len;
};

typedef void(*fredSocketOnRecieve)(int server_scoket, int client_socket);

void fredSocketServerListenerInit(int serverSocket, fredSocketOnRecieve callbackFunction);
void fredSocketClientListenerInit(int serverSocket, fredSocketOnRecieve callbackFunction, unsigned long timeout);
int fredSocketListenerUpdate(int* socketList, int listCount);


#endif // _FREDSOCKETLISTENER_H_
