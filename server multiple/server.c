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

#define TRUE 1
#define FALSE 0

#define SERVER_PORT 8888
#define MAX_CLIENTS 3


int initServer(int server_port);
int closeServer(int server_fd);

int addClient(int client_fd);
int removeClient(int client_fd);
int writeClient(int client_fd, char* buffer, int buffer_len);
int readClient(int client_fd, char* buffer, int buffer_len);

int sendMessage(int source_client_fd);

void updateActivity();

int server_socket;              // Socket para el servidor
int max_sd;                     // Numero de descriptor mayor de todos los sockets
fd_set readfds;                 // Descriptores de archivos para monitorear actividades de sockets
int client_list[MAX_CLIENTS];   // Arreglo de clientes


void updateActivity() {

}

int initServer(int server_port) {

    //##########################################################################################################################
    // Inicialización del servidor.
    // 
    // Crea el socket del servidor, establece el puerto y ancla el socket para escuchar conexiones en dicho puerto.
    //##########################################################################################################################
    struct sockaddr_in address;     // Estructura para guardar la dirección del cliente
    int srv_sock;              // Descriptor de socket del servidor.
    int addrlen;                    // Tamaño de la estructura para la dirección.

    // Crea el socket para el servidor.
    srv_sock = socket(AF_INET, SOCK_STREAM, 0);

    // Si no se pudo crear el socket, devuelve error.
    if( srv_sock == -1) { 
        perror("Error creating server socket.");
        return 0;
    }

    // Configura la dirección del servidor.
    address.sin_family = AF_INET;           // Protocolo de internet
    address.sin_addr.s_addr = INADDR_ANY;   // Acepta cualquier dirección
    address.sin_port = htons(server_port);  // Asigna el puerto.

    // Asigna el socket con su dirección y puerto para que escuche conecciones entrantes.
    // si no se puede asignar (devuelve -1) devuelve error.
    if(bind(srv_sock, (struct sockaddr*)&address, sizeof(address)) == -1) {
        perror("Error binding server");
        return 0;
    } 

    printf("Server linsten on address 127.0.0.1/%d\n", server_port);

    // Especifica que guarde (escuche) como máximo 3 conecciones pendientes para el servidor
    // mientras se atienden las demás conexiones. Si hay error (devuelve -1) devuelve el error.
    if(listen(srv_sock, 3) == 1) {
        perror("Error listening connections");
        return 0;
    }                      

    return srv_sock;

}



void _chatServerUpdateWatcher() {
    int i, sd;
   // Establece el maximo numero de descriptor.
    max_sd = server_socket;
    // Limpia los sets de sockets de actividades.
    FD_ZERO(&readfds);

    // Agrega el socket del servidor al set de sockets a monitorear.
    FD_SET(server_socket, &readfds);

    // Recorre la lista de descriptores de los clientes para buscar
    // clientes y agregarlos al set de sockets a monitorear.
    for (i = 0; i < MAX_CLIENTS; ++i)
    {
        // Obtiene el siguiente cliente de la lista
        sd = client_list[i];
        // Si el cliente es válido lo agrega al set de sockets.
        if(sd > 0)
            FD_SET(sd, &readfds);
        // si el descriptor del cliente es mayor que el máximo,
        // establece el descriptor del cliente como el máximo.
        if(sd > max_sd)
            max_sd = sd;
    }
}
int main(int argc, char const *argv[])
{
    
    int opt = TRUE;
    int addrlen;                    // Tamaño de la estructura para la dirección.
    int new_client;                 // Descriptor del nuevo cliente
    int activity;                   // Actividades pendientes. Devuelto por la funcion select.
    int valread;                    // Cantidad de datos enviados por un cliente.
    struct sockaddr_in address;     // Estructura para guardar la dirección del cliente
    int sd, i;                         
    int free_client_locations = MAX_CLIENTS;
    char buffer[1025]; // buffer de 1K
    
    //Mensaje a enviar al cliente cuando se conecte.
    char *message = "ECHO Daemon v1.0 \r\n";

    // Inicializa el arreglo de los clientes
    for (i = 0; i < MAX_CLIENTS; ++i)
        client_list[i] = 0;

    server_socket = initServer(SERVER_PORT);
    
    // Guarda el tamaño de la dirección.
    addrlen = sizeof(address);
    printf("Waiting connections...\n");   

//##########################################################################################################################
// Ciclo infinito, aqui se atienden las peticiones de los clientes y las nuevas conexiones con el servidor.
//##########################################################################################################################
    // Ciclo infinito para aceptar y atender conexiones
    while(TRUE) {
        
        _chatServerUpdateWatcher();

        // Espera a que ocurra una actividad en uno de los sockets
        // que se encuentre dentro del set de sockets. si el último
        // parámetro es NULL (timeout), se bloquea la funció por un 
        // tiempo indeterminado.
        activity =  select( max_sd + 1, &readfds, NULL, NULL, NULL);

        // Si hubo un error al seleccionar la actividad, devuelve el error.
        if(activity == -1) {
            perror("Error selecting activity");
            exit(EXIT_FAILURE);
        }

        // Si hubo un actividad en el socket del servidor, significa que es una
        // conexión entrata y se debe de atender, siempre y cuando haya espacio
        // disponible en la lista de clientes.
        if(FD_ISSET(server_socket, &readfds)) {

            // Acepta la nueva conección del cliente.
            new_client = accept(server_socket, (struct sockaddr*)&address, (socklen_t*)&addrlen);
            // Si hubo error al aceptar la conexión, devuelve error
            if(new_client == -1){
                perror("Error acepting incomming connection.");
                exit(EXIT_FAILURE);
            }
            // Si hay espacio libre para guardar al cliente...
            if(free_client_locations) {

                // Muestra mensaje de conexión aceptada con la información del cliente.
                printf("New connection accepted, socket fd: %d, IP: %s, port: %d\n", new_client, inet_ntoa(address.sin_addr), ntohs(address.sin_port) );
                
                // Envía mensaje de bienvenida al cliente, si hay error lo muestra.
                if( send(new_client, message, strlen(message), 0)  != strlen(message)) {
                    perror("Error sending message to client");
                }
                printf("Welcome message sent successfully\n");

                for (i = 0; i < MAX_CLIENTS; ++i)
                {
                    if(client_list[i] == 0) {
                        client_list[i] = new_client;
                        free_client_locations--;
                        printf("Adding client to list as %d\n", i);
                        break;
                    }
                }
            }
            else { // si no hay espacio para guardar al cliente rechaza la conexión.
                // Muestra mensaje
                printf("Connection refused, server full.\n");
                close(new_client);
            }
        }


        // Si hubo una actividad en el socket de cualquier otro cliente diferente al servidor..
        for (i = 0; i < MAX_CLIENTS; ++i)
        {
            // Obtiene el siguiente descriptor de la lista de clientes.
            sd = client_list[i];
            // si el descriptor es valido y hay actividad pendiente para el socket.
            // atiende la petición.
            if(sd >0 && FD_ISSET(sd, &readfds)) {

                // Lee los datos enviados por el cliente
                valread = read(sd, buffer, 1024);

                // Obtiene la información del cliente
                getpeername(sd, (struct sockaddr*)&address, (socklen_t*)&addrlen);

                // si no hay datos a leer, signidica que la conexión se cerró,
                // si es así cierra la conexión con el cliente y lo elimina de la lista.
                if(valread == 0) {
                    // Muestra la información del cliente desconectado.
                    printf("Host disconnected, IP; %s, port: %d\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port) );
                    // Elimina al cliente de la lista.
                    client_list[i] = 0;
                    // Elimina al cliente del set de sockets de monitoreo.
                    FD_CLR(sd, &readfds);
                    // CIerra el socket del cliente
                    close(sd);
                    free_client_locations++;
                }
                else { // si hay datos leidos (no hay desconexión del cliente), procesa los datos.
                    
                    printf("Client request, IP; %s, port: %d\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port) );
                    // Agrega terminador de cadena a los datos enviados
                    buffer[valread] = '\0';
                    // devuelve al cliente los datos enviados.
                    send(sd, buffer, strlen(buffer), 0);
                }
            }
        }
    }
//##########################################################################################################################
    return 0;
}