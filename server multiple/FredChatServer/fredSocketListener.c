#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "fredSocketListener.h"


static fd_set _readfds;
static int _max_fd;
static fredSocketOnRecieve _callback_func;
static fredSocketOnRecieve _callback_func;
static int _srv_socket;
static unsigned long _timeout;

void fredSocketServerListenerInit(int serverSocket, fredSocketOnRecieve callbackFunction)
{
    _timeout = 0;
    _max_fd = 0;
    _srv_socket = serverSocket;
    _callback_func = *callbackFunction;
    FD_ZERO(&_readfds);
}
void fredSocketClientListenerInit(int serverSocket, fredSocketOnRecieve callbackFunction, unsigned long timeout)
{
    _timeout = timeout;
    _max_fd = 0;
    _srv_socket = serverSocket;
    _callback_func = *callbackFunction;
    FD_ZERO(&_readfds);
}
int fredSocketListenerUpdate(int* socketList, int listCount) {

    int i, fd, action;
    struct RecvMessage msg;
    struct timeval tv;
    FD_ZERO(&_readfds);

    _max_fd = _srv_socket;
    FD_SET(_srv_socket, &_readfds);

    for (i = 0; i < listCount; i++)
    {
        fd = socketList[i];
        FD_SET(fd, &_readfds);
        if(fd > _max_fd)
            _max_fd = fd;
    }

    if(_timeout){
        tv.tv_usec = _timeout;
        tv.tv_sec = 0;
        action = select(_max_fd +1, &_readfds, NULL, NULL, &tv);
    }
    else{
        action = select(_max_fd +1, &_readfds, NULL, NULL, NULL);
    }

    if(action == -1)
        return -1;

    if(!action)
        return 0;

    if(FD_ISSET(_srv_socket, &_readfds))
        if(_callback_func)
            _callback_func(_srv_socket, _srv_socket);


    for (i = 0; i < listCount; i++)
    {
        fd = socketList[i];
        if(FD_ISSET(fd, &_readfds))
            if(_callback_func)
                _callback_func(_srv_socket, fd);
    }

    return 1;
}
