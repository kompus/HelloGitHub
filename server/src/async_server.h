#ifndef ASYNC_SERVER_H
#define ASYNC_SERVER_H

#include <stdio.h>
#include <winsock2.h>

const static int MAXRECV = 1024;
const static int MAXNAME = 256;

int run(SOCKET* master, int max_clients);

#endif
