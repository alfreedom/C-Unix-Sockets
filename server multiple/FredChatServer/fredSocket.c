#include "fredSocket.h"


int fredSocketCreateINETTCPServer(unsigned int server_port, int max_pending_connections) {

    struct sockaddr_in address;     // Estructura para guardar la dirección del servidor
    int srvSocket;

    if((srvSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Creating INET TCP server socket");
        return -1;
    }

    address.sin_family = AF_INET;           // Protocolo de internet
    address.sin_addr.s_addr = INADDR_ANY;   // Acepta cualquier dirección
    address.sin_port = htons(server_port);  // Asigna el puerto.

    if(bind(srvSocket, (struct sockaddr*)&address, sizeof(address)) == -1)
    {
        perror("Binding INET TCP server socket");
        close(srvSocket);
        return -1;
    }

    if(listen(srvSocket, max_pending_connections) == -1)
    {
        perror("Listen on INET TCP server socket");
        close(srvSocket);
        return -1;
    }

    return srvSocket;
}

int fredSocketCreateINETUDPServer(unsigned int server_port, int max_pending_connections) {

    struct sockaddr_in address;     // Estructura para guardar la dirección del servidor
    int srvSocket;

    if( (srvSocket = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        perror("Creating INET UDP server socket");
        return -1;
    }

    address.sin_family = AF_INET;           // Protocolo de internet
    address.sin_addr.s_addr = INADDR_ANY;   // Acepta cualquier dirección
    address.sin_port = htons(server_port);  // Asigna el puerto.

    if(bind(srvSocket, (struct sockaddr*)&address, sizeof(address)) == -1)
    {
        perror("Binding INET UDP server socket");
        close(srvSocket);
        return -1;
    }

    return srvSocket;

}

int fredSocketCreateUNIXTCPServer(char* service, int max_pending_connections) {

    struct sockaddr_un address;
    int srvSocket;

    if ((srvSocket = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
    {
        perror("Creating UNIX TCP server socket");
        return -1;
    }

    address.sun_family = AF_UNIX;
    strncpy(address.sun_path, service, sizeof(address.sun_path));
    unlink(address.sun_path);

    if (bind(srvSocket, (struct sockaddr*)&address, sizeof(address)) == -1)
    {
        perror("Binding UNIX TCP server socket");
        close(srvSocket);
        return -1;
    }

    if (listen(srvSocket, max_pending_connections) == -1)
    {
        perror("Listen on UNIX TCP server socket");
        close(srvSocket);
        return -1;
    }

    return srvSocket;

}

int fredSocketCreateUNIXUDPServer(char* service, int max_pending_connections) {

    struct sockaddr_un address = {0};
    int srvSocket;

    if ((srvSocket = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1) {
        perror("Creating UNIX UDP server socket");
        return -1;
    }

    address.sun_family = AF_UNIX;
    strncpy(address.sun_path, service, sizeof(address.sun_path));
    unlink(address.sun_path);

    if (bind(srvSocket, (struct sockaddr*)&address, sizeof(address)) == -1)
    {
        perror("Binding UNIX UDP server socket");
        close(srvSocket);
        return -1;
    }

    return srvSocket;
}

int fredSocketCreateINETTCPClient(char* server_ip, int server_port) {

    int clientSock = 0, n = 0;
    struct sockaddr_in address;

    memset(&address, 0, sizeof(address));

    address.sin_family = AF_INET;
    address.sin_port = htons(server_port);

    if(inet_pton(AF_INET, server_ip, &address.sin_addr)<=0)
    {
        perror("Invalid IP");
        return -1;
    }

    if((clientSock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Creating INET TCP client socket");
        return -1;
    }


    if( connect(clientSock, (struct sockaddr*)&address, sizeof(address)) < 0)
    {
        perror("Connecting to server");
        return -1;
    }

    return clientSock;
}

int fredSocketCreateINETUDPClient(char* ip, int port) {

}

int fredSocketCreateUNIXTCPClient() {

}

int fredSocketCreateUnixUDPClient() {

}


int fredSocketWrite(int socket_fd, char* buffer, int buffer_len) {

    if(send(socket_fd , buffer, buffer_len, 0) != buffer_len) {
        perror("Error sendig data to socket");
        return 0;
    }
    return 1;
}

int fredSocketRead(int socket_fd, char* buffer, int buffer_len) {
    return read(socket_fd, buffer, buffer_len);
}
