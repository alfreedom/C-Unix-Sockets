#include "chatserver.h"
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

static struct User userlist[MAX_USERS];
static int free_users = MAX_USERS;
static int _server_socket;
static int _max_sd;
static fd_set _readfds;
static struct Message _m;


userID_t getUserIDbyFD(int socket_fd){
    int i, sd;

    for (i = 0; i < MAX_USERS; ++i)
    {
        sd = userlist[i].socket_fd;
        if(sd> 0 && sd == socket_fd)
            return i;
    }
    return -1;
}

int chatServerRecieveMessage(userID_t user_id, struct Message* msg) {

    int count_read = -1;
    int user_sd;

    if(user_id >= 0 && user_id < MAX_USERS && userlist[user_id].socket_fd > 0) {
        count_read = read(userlist[user_id].socket_fd, (char*)msg, sizeof(struct Message));
    }
    return count_read;

}

int setUserName(userID_t user_id, char* new_name) {

    if(user_id >= 0 && user_id < MAX_USERS && userlist[user_id].socket_fd > 0) {
        strcpy(userlist[user_id].username, new_name);
        return 1;
    }
    return 0;
}

int sendWelcomeMessage(userID_t user_id) {
    struct Message m;

    strcpy(m.buffer, WELCOME_TEXT);
    m.type = MSG_INFO;
    m.buffer_len = strlen(WELCOME_TEXT) + 1;

    return chatServerSendMessage(user_id, &m);

}

void _chatServerUpdateWatcher() {
    int sd, i;
    // Establece el maximo numero de descriptor.
    _max_sd = _server_socket;
    // Limpia los sets de sockets de actividades.
    FD_ZERO(&_readfds);

    // Agrega el socket del servidor al set de sockets a monitorear.
    FD_SET(_server_socket, &_readfds);

    // Recorre la lista de descriptores de los clientes para buscar
    // clientes y agregarlos al set de sockets a monitorear.
    for (i = 0; i < MAX_USERS; ++i)
    {
        // Obtiene el siguiente cliente de la lista
        sd = userlist[i].socket_fd;
        // Si el cliente es válido lo agrega al set de sockets.
        if(sd > 0)
            FD_SET(sd, &_readfds);
        // si el descriptor del cliente es mayor que el máximo,
        // establece el descriptor del cliente como el máximo.
        if(sd > _max_sd)
            _max_sd = sd;
    }
}

int chatServerRun() {

    int act, valread, new_user, new_user_id, i;
    struct User user;
    struct sockaddr_in address;     // Estructura para guardar la dirección del cliente
    int sd;
    struct Message m;
    char buff[1025];
    int addrlen;


    _chatServerUpdateWatcher();
    act = select(_max_sd + 1, &_readfds, NULL, NULL, NULL);

    if(act == -1) {
        perror("Error selecting activity");
        return -1;
    }

    addrlen = sizeof(address);
    // Si hubo una actividad en el servidor, significa que hay una
    // petición de una conexión nueva de un cliente.
    if(FD_ISSET(_server_socket, &_readfds)) {

        new_user = accept(_server_socket, (struct sockaddr*)&address, (socklen_t *)&addrlen);

        if(new_user == -1) {
            perror("Error acepting client connection");
            return -1;
        }


        strcpy(user.username, "");
        strcpy(user.ip, inet_ntoa(address.sin_addr));
        user.port=ntohs(address.sin_port);
        user.socket_fd = new_user;

        // Agrega un nuevo usuario y guarda su ID de usuario.
        new_user_id = chatServerAddUser(&user);

        // Si se pudo agregar al usuario...
        if(new_user_id != -1) {
            // envia mensaje de bienvenida, en caso de no poderlo enviar marca error.
            if(!sendWelcomeMessage(new_user_id)){
                perror("Error sending Welcome Message");
            }

            printf("User connected on IP-> %s/%d  ID-> %d\n", user.ip, user.port, new_user_id);
        }
        else{ // Si no se pudo agregar el usuario, se alncanzo el limiete de usuarios
            printf("Connection refused can't add user, server full.\n");
            close(new_user);
            return 0;
        }

    }



    // Si no es actividad de el servidor, se checan los clientes para ver sus actividades.
    for (i = 0; i < MAX_USERS; ++i)
    {
        // Obtiene el siguiente descriptor de la lista de clientes.
            sd = userlist[i].socket_fd;
            // si el descriptor es valido y hay actividad pendiente para el socket.
            // atiende la petición.
            if(sd >0 && FD_ISSET(sd, &_readfds)) {

                valread = chatServerRecieveMessage(getUserIDbyFD(sd), &m);
                // Obtiene la información del cliente
                //getpeername(sd, (struct sockaddr*)&address, (socklen_t*)&addrlen);

                // Hubo error al leer los datos del cliente
                if(valread == -1) {
                    perror("Error recievieng message");
                    continue;
                }
                // si no hay datos a leer, signidica que la conexión se cerró,
                // si es así cierra la conexión con el cliente y lo elimina de la lista.
                if(valread == 0) {
                    // Muestra la información del cliente desconectado.
                    printf("User disconnected, IP; %s/%d\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port) );
                    chatServerRemoveUser(i);
                }
                else { // si hay datos leidos (no hay desconexión del cliente), procesa los datos.

                    // Procesar los datos recibidos por el cliente
                    switch(m.type) {

                        case MSG_INFO:
                        break;
                        case MSG_TEXT:
                            // Agrega el nombre de usuario al mensaje.
                            strcpy(buff, userlist[i].username);
                            strcat(buff, ": \n");
                            strcat(buff, m.buffer);
                            strcpy(m.buffer,buff);
                            // Reenvia el mensaje a todos los usuarios conectados
                            chatServerBroadcastMessage(&m);
                        break;
                        case MSG_USERNAME:
                            setUserName(getUserIDbyFD(sd), m.buffer);
                        break;
                    }
                }
            }
    }
}

int chatServerBroadcastMessage(struct Message* msg) {

    int i, sd;
    for (i = 0; i < MAX_USERS; ++i)
    {
        sd = userlist[i].socket_fd;
        if(sd > 0)
            if(send(sd, (char*)msg, sizeof(struct Message), 0) != sizeof(struct Message)) {
                perror("Error sending message to clients");
                return 0;
            }
    }
    return 1;
}

int chatServerSendMessage(userID_t user_id, struct Message* msg){

    if(user_id >= 0 && user_id < MAX_USERS && userlist[user_id].socket_fd > 0) {
        if(send(userlist[user_id].socket_fd , (char*)msg, sizeof(struct Message), 0) != sizeof(struct Message)) {
            perror("Error sending message to client");
            return 0;
        }
    }

    return 1;
}

int chatServerAddUser(struct User* user) {
    int i, sd;

    if(free_users) {

        for (i = 0; i < MAX_USERS; ++i)
        {
            sd = userlist[i].socket_fd;
            if(sd == 0) {
                strcpy(userlist[i].username, user->username);
                strcpy(userlist[i].ip, user->ip);
                userlist[i].port = user->port;
                userlist[i].socket_fd = user->socket_fd;
                free_users--;
                break;
            }
        }
    }
    else
        return -1;

    return i;
}
int chatServerRemoveUser(userID_t user_id) {

    if(user_id >= 0 && user_id < MAX_USERS && userlist[user_id].socket_fd > 0) {
        close(userlist[user_id].socket_fd);
        memset(&userlist[user_id], 0, sizeof(struct User));
        free_users++;
    }
}

int chatServerInit(int server_port) {

    //##########################################################################################################################
    // Inicialización del servidor.
    //
    // Crea el socket del servidor, establece el puerto y ancla el socket para escuchar conexiones en dicho puerto.
    //##########################################################################################################################
    struct sockaddr_in address;     // Estructura para guardar la dirección del cliente
    int addrlen;                    // Tamaño de la estructura para la dirección.
    int i;
    // Crea el socket para el servidor.
    _server_socket = socket(AF_INET, SOCK_STREAM, 0);

    // Si no se pudo crear el socket, devuelve error.
    if( _server_socket == -1) {
        perror("Error creating server socket.");
        return 0;
    }

    for (i = 0; i < MAX_USERS; ++i)
    {
        memset(&userlist[i], 0, sizeof(struct User));
    }
    // Configura la dirección del servidor.
    address.sin_family = AF_INET;           // Protocolo de internet
    address.sin_addr.s_addr = INADDR_ANY;   // Acepta cualquier dirección
    address.sin_port = htons(server_port);  // Asigna el puerto.

    // Asigna el socket con su dirección y puerto para que escuche conecciones entrantes.
    // si no se puede asignar (devuelve -1) devuelve error.
    if(bind(_server_socket, (struct sockaddr*)&address, sizeof(address)) == -1) {
        perror("Error binding Fred Chat Server :'(");
        return 0;
    }

    // Especifica que guarde (escuche) como máximo 3 conecciones pendientes para el servidor
    // mientras se atienden las demás conexiones. Si hay error (devuelve -1) devuelve el error.
    if(listen(_server_socket, 5) == 1) {
        perror("Error listening connections on Fred Chat Server :'(");
        return 0;
    }

    return _server_socket;
}
