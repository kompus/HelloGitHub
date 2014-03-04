#include<stdio.h>
#include<winsock2.h>

int main(int argc , char *argv[])
{
	WSADATA wsa;
	SOCKET server;
	
	WSACleanup();    
    return 0;
}
