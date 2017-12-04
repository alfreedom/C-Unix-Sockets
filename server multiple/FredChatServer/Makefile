
SERVER_OUT = FCserver
CLIENT_OUT = FCclient

CPPFLAGS = -g -I.

SERVER_CPP_FILES = fredSocket.o
SERVER_CPP_FILES += fredSocketListener.o
SERVER_CPP_FILES += chatuser.o

CLIENT_CPP_FILES = fredSocket.o
CLIENT_CPP_FILES += fredSocketListener.o

all :  $(SERVER_OUT) $(CLIENT_OUT)

$(SERVER_OUT) : $(SERVER_CPP_FILES)  fredchatserver.c
	cc -g -I. $(SERVER_CPP_FILES) fredchatserver.c -o $(SERVER_OUT)

$(CLIENT_OUT) : $(CLIENT_CPP_FILES) fredchatclient.c
	cc -g -I. $(CLIENT_CPP_FILES) fredchatclient.c -pthread -o $(CLIENT_OUT) 

clean :
	rm -f *.o $(SERVER_OUT) $(CLIENT_OUT)
