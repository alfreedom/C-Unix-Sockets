
#include "chatserver.h"

int main(int argc, char const *argv[])
{
    int server_socket;
    server_socket = chatServerInit(9696);

    if(server_socket == -1) {
        perror("Error runing server");
        return 0;
    }

    printf("Server runing on port %d\n", 9696);
    printf("Waitting incomming users...\n");
    while(TRUE) {
        chatServerRun();
    }

    return 0;
}
