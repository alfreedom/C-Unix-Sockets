all : chatserver.o Servidor

CPPFLAGS = -g -I.

Servidor : cserver.c
	cc -g -I. chatserver.o cserver.c -o Servidor

Cliente : Cliente.c
	cc -g -I. Socket.o Socket_Cliente.o Cliente.c -o Cliente

clean :
	rm *.o Servidor
