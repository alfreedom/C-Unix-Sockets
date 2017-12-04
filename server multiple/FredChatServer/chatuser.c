
#include "chatuser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static struct User _userlist[MAX_USERS];
static int _free_users;
static int _user_fd_list[MAX_USERS];

void chatUserInit() {

    int i;

    for(i = 0; i < MAX_USERS; i++) {
        memset(&_userlist[i], 0, sizeof(struct User));
        _userlist[i].socket_fd = -1;
        _userlist[i].id = -1;
        _user_fd_list[i] = -1;
    }

    _free_users = MAX_USERS;
}

userID_t chatUserAdd(struct User* user) {
    int i, sd;

    if(_free_users)
    {
        for (i = 0; i < MAX_USERS; ++i)
        {
            sd = _userlist[i].socket_fd;
            if(sd == -1)
            {
                strcpy(_userlist[i].username, user->username);
                strcpy(_userlist[i].ip, user->ip);
                _userlist[i].port = user->port;
                _userlist[i].socket_fd = user->socket_fd;
                _userlist[i].id = i;
                _free_users--;
                break;
            }
        }
    }
    else
        return -1;

    return i;

}

int chatUserRemove(userID_t user_id) {
    int sd;
    if(user_id >= 0 && user_id < MAX_USERS && _userlist[user_id].socket_fd > 0)
    {
        sd =_userlist[user_id].socket_fd;
        memset(&_userlist[user_id], 0, sizeof(struct User));
        _userlist[user_id].socket_fd = -1;
        _free_users++;
        return sd;
    }
    return -1;
}

struct User* chatUserGetUserByID(userID_t user_id) {
    if(user_id >= 0 && user_id < MAX_USERS && _userlist[user_id].socket_fd > 0)
    {
        return (&_userlist[user_id]);
    }
    return NULL;
}

struct User* chatUserGetUserByFD(int user_fd) {
    int i, sd;
    for (i = 0; i < MAX_USERS; i++)
    {
        sd = _userlist[i].socket_fd;
        if(sd > 0 && sd == user_fd)
            return (&_userlist[i]);
    }
    return NULL;
}

struct User* chatUserGetUserByName(char* username) {
    int i, sd;
    for (i = 0; i < MAX_USERS; i++)
    {
        sd = _userlist[i].socket_fd;
        if(sd > 0 && !strcmp(_userlist[i].username, username))
            return (&_userlist[i]);
    }
    return NULL;
}

struct User* chatUserGetUserByIP(char* user_ip) {
    int i, sd;
    for (i = 0; i < MAX_USERS; i++)
    {
        sd = _userlist[i].socket_fd;
        if(sd > 0 && !strcmp(_userlist[i].ip, user_ip))
            return (&_userlist[i]);
    }
    return NULL;
}

struct User* chatUserGetUserByPort(int user_port) {
    int i, sd;
    for (i = 0; i < MAX_USERS; i++)
    {
        sd = _userlist[i].socket_fd;
        if(sd > 0 && _userlist[i].port == user_port)
            return (&_userlist[i]);
    }
    return NULL;
}

int chatUserSetUsername(userID_t user_id, char* new_name) {
    if(user_id >= 0 && user_id < MAX_USERS && _userlist[user_id].socket_fd > 0) {
        strcpy(_userlist[user_id].username, new_name);
        return 1;
    }
    return 0;
}

userID_t chatUserGetIDbyFD(int user_fd) {
    int i, sd;
    for (i = 0; i < MAX_USERS; ++i)
    {
        sd = _userlist[i].socket_fd;
        if(sd> 0 && sd == user_fd)
            return i;
    }
    return -1;
}

int chatUserGetFDbyID(userID_t user_id) {
    if(user_id >= 0 && user_id < MAX_USERS && _userlist[user_id].socket_fd > 0) {
        return _userlist[user_id].socket_fd;
    }
    return -1;
}

int* chatUserGetFDList(int *count) {
    int i, c = 0;

    for (i = 0; i < MAX_USERS; i++) {
        if(_userlist[i].socket_fd > 0)
            _user_fd_list[c++] = _userlist[i].socket_fd;

    }
    *count = c;
    return _user_fd_list;
}

int chatUserGetAvailableUsers() {
    return _free_users;
}
