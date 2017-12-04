

#ifndef _CHATUSER_H_
#define _CHATUSER_H_

#define MAX_USERS 4

typedef int userID_t;

struct User {
    char username[30];
    char ip[30];
    int port;
    int socket_fd;
    int id;
};

void chatUserInit();
userID_t chatUserAdd(struct User* user);
int chatUserRemove(userID_t user_id);
struct User* chatUserGetUserByID(userID_t user_id);
struct User* chatUserGetUserByFD(int user_fd);
struct User* chatUserGetUserByName(char* username);
struct User* chatUserGetUserByIP(char* user_ip);
struct User* chatUserGetUserByPort(int user_port);
int chatUserSetUsername(userID_t user_id, char* new_name);
userID_t chatUserGetIDbyFD(int user_fd);
int chatUserGetFDbyID(userID_t user_id);
int* chatUserGetFDList(int *count);
int chatUserGetAvailableUsers();


#endif // _CHATUSER_H_
