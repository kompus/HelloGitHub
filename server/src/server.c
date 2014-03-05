#include <stdio.h>
#include <winsock2.h>
#include "winsock_init.h"
#include "async_server.h"

int main(int argc , char *argv[])
{
	int max_clients = 25;
	SOCKET master;
	struct sockaddr_in server;
    
	initWinsock(&master, &server, 27015);
	run(&master, max_clients);
    
	cleanWinsock(&master);
    return 0;
}
