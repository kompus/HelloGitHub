#include "winsock_init.h"

int initWinsock(SOCKET* master, struct sockaddr_in* server, int port)
{
	WSADATA wsa;
	
	printf("Initialising Winsock...");
    if(WSAStartup(MAKEWORD(2,2), &wsa) != 0)
    {
        printf("Failed. Error Code : %d", WSAGetLastError());
        return -1;
    }
    printf("Initialised.\n");
    
    if((*master = socket(AF_INET , SOCK_STREAM , 0)) == INVALID_SOCKET)
    {
        printf("Could not create socket : %d" , WSAGetLastError());
        return -2;
    }
    printf("Socket created.\n");
    
    server->sin_family = AF_INET;
    server->sin_addr.s_addr = INADDR_ANY;
    server->sin_port = htons(port);
    
    if(bind(*master, (struct sockaddr*)server , sizeof(*server)) == SOCKET_ERROR)
    {
        printf("Bind failed with error code : %d" , WSAGetLastError());
        return -3;
    }
    printf("Bind done on port %d.\n", port);
    
    listen(*master , 3);
    printf("Waiting for incoming connections...\n");
    
    return 0;
}

void cleanWinsock(SOCKET* master)
{
	closesocket(*master);
    WSACleanup();
}
