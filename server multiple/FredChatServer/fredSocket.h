#ifndef _FREDSOCKET_H_
#define _FREDSOCKET_H_

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

int fredSocketCreateINETTCPServer(unsigned int server_port, int max_pending_connections);
int fredSocketCreateINETUDPServer(unsigned int server_port, int max_pending_connections);
int fredSocketCreateUNIXTCPServer(char* service, int max_pending_connections);
int fredSocketCreateUNIXUDPServer(char* service, int max_pending_connections);
int fredSocketCreateINETTCPClient(char* server_ip, int server_port);
int fredSocketCreateINETUDPClient(char* server_ip, int server_port);
int fredSocketCreateUNIXTCPClient();
int fredSocketCreateUNIXUDPClient();

int fredSocketWrite(int socketFD, char* buffer, int bufferLen);
int fredSocketRead(int socketFD, char* buffer, int bufferLen);
#endif // _FREDSOCKET_H_
