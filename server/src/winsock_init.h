#ifndef WINSOCK_INIT_H
#define WINSOCK_INIT_H

#include <stdio.h>
#include <winsock2.h>

int initWinsock(SOCKET* master, struct sockaddr_in* server, int port);

void cleanWinsock(SOCKET* master);

#endif
