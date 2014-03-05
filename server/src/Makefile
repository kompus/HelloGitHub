bin.exe: server.c winsock_init.o async_server.o
	gcc server.o winsock_init.o async_server.o -o bin.exe -lpdcurses -lws2_32
	
server.o: server.c
	gcc -c server.c -o server.o
	
winsock_init.o: winsock_init.c
	gcc -c winsock_init.c -o winsock_init.o
	
async_server.o: async_server.c
	dcc -c async_server.c -o async_server.o