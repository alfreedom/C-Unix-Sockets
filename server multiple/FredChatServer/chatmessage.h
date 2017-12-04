#ifndef _CHATMESSAGE_H_
#define _CHATMESSAGE_H_

#define MSG_BUFFER_SIZE 1024

enum MessageType {
    MSG_USERNAME,
    MSG_TEXT,
    MSG_INFO,
    MSG_ERROR
};

struct Message {
    enum MessageType type;
    int buffer_len;
    char buffer[MSG_BUFFER_SIZE];
};

#endif // _CHATMESSAGE_H_
