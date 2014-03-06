#include "async_server.h"

int receiveFromSocket(int client_id, char* buffer, SOCKET* clients, char** names, int max_clients)
{
	int received, i, addrlen = sizeof(struct sockaddr_in);
	struct sockaddr_in address;
	SOCKET sock = clients[client_id];
	
	getpeername(sock, (struct sockaddr*)&address, (int*)&addrlen);
	received = recv(sock, buffer, MAXRECV, 0);
	
    if(received == SOCKET_ERROR)
	{
		int error_code = WSAGetLastError();
		if(error_code == WSAECONNRESET)
		{
			printf("Client disconnected unexpectedly, IP: %s, port: %d\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));
			closesocket(sock);
			clients[client_id] = 0;
			return -1;
		}
		else
		{
			printf("recv failed with error code: %d\n", error_code);
			return -2;
		}
	}
	else if(received == 0)
    {
		printf("Client disconnected, IP: %s, port: %d\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));
        closesocket(sock);
	}
	if(received == SOCKET_ERROR || received == 0)
	{
		strcpy(buffer, names[client_id]);
		strcat(buffer, ";");
		for(i = 0; i < max_clients; i++)
    {
		if(client_id != i && clients[i] != 0)
		{
			send(clients[i], buffer, strlen(buffer), 0);
        }
	}
	}
	return received;
}
int addNewUser(SOCKET* new_socket, SOCKET* clients, char** names, int max_clients)
{
	int i, id, received, addrlen;
	char buffer[MAXNAME];
	struct sockaddr_in address;
	addrlen = sizeof(struct sockaddr_in);
	received = recv(*new_socket, buffer, MAXRECV, 0);
	if(received == SOCKET_ERROR)
	{
		int error_code = WSAGetLastError();
		if(error_code == WSAECONNRESET)
		{
			printf("New client disconnected unexpectedly.\n");
			closesocket(*new_socket);
			return -1;
		}
		else
		{
			printf("recv failed with error code: %d\n" , error_code);
			return -2;
		}
	}
	else if(received == 0)
    {
		printf("New client disconnected.\n");
        closesocket(*new_socket);
        return -3;
	}
	for(i = 0; i < max_clients; i++) if(strcmp(buffer, names[i]) == 0) return 1;
	for(i = 0; i < max_clients; i++)
    {
		if(clients[i] == 0)
		{
			clients[i] = *new_socket;
            strcpy(names[i], buffer);
            id = i;
            printf("Adding socket with nick '%s' to list at index %d\n", buffer, i);
            break;
        }
	}
	strcpy(buffer, "");
	for(i = 0; i < max_clients; i++)
    {
		if(id != i && clients[i] != 0)
		{
			getpeername(clients[i], (struct sockaddr*)&address, (int*)&addrlen);
            strcat(buffer, names[i]);
            strcat(buffer, ",");
            strcat(buffer, inet_ntoa(address.sin_addr));
            strcat(buffer, ";");
        }
	}
	if(send(*new_socket, buffer, strlen(buffer), 0) != strlen(buffer))
	{
		printf("Sending online list failed.\n");
		closesocket(*new_socket);
		return -4;
	}
	getpeername(*new_socket, (struct sockaddr*)&address, (int*)&addrlen);
	strcpy(buffer, names[id]);
	strcat(buffer, ",");
	strcat(buffer, inet_ntoa(address.sin_addr));
	strcat(buffer, ";");
	for(i = 0; i < max_clients; i++)
    {
		if(id != i && clients[i] != 0)
		{
			send(clients[i], buffer, strlen(buffer), 0);
        }
	}
	return 0;
}
int run(SOCKET* master, int max_clients)
{
	int i, addrlen, activity, received;
	char *buffer;
	char *names[max_clients];
	buffer = (char*)malloc((MAXRECV + 1) * sizeof(char));
	SOCKET clients[max_clients], new_socket;
	struct sockaddr_in address;
	fd_set readfds;
	
	addrlen = sizeof(struct sockaddr_in);
	for(i = 0; i < max_clients; i++) 
	{
		clients[i] = 0;
		names[i] = (char*)malloc((MAXNAME + 1) * sizeof(char));
	}
	while(TRUE)
	{		
		FD_ZERO(&readfds);
		FD_SET(*master, &readfds);
		for (i = 0; i < max_clients; i++) if(clients[i] > 0) FD_SET(clients[i] , &readfds);
        activity = select(0, &readfds, NULL, NULL, NULL);
        if(activity == SOCKET_ERROR)
        {
            printf("Select failed with error code : %d\n" , WSAGetLastError());
            return -1;
        }
        if(FD_ISSET(*master , &readfds)) 
        {
            if((new_socket = accept(*master, (struct sockaddr*)&address, (int*)&addrlen)) < 0)
            {
                printf("Error when accepting new socket: %d\n", WSAGetLastError());
                return -2;
            }
            printf("New connection, socket fd: %d, IP: %s, port: %d\n", new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));
                        
            // tutaj bedzie odbywac sie pobieranie nicku oraz wysylanie listy uzytkownikow
            //received = recv(sock, buffer, MAXRECV, 0);
            if(addNewUser(&new_socket, clients, names, max_clients) == 0)
            {
			}
        }
        for(i = 0; i < max_clients; i++) 
        {  
            if(FD_ISSET(clients[i], &readfds))
            {
				received = receiveFromSocket(i, buffer, clients, names, max_clients);
                if(received > 0)
                {
                    // tymczasowo odpowiada tym co mu sie wyslalo, tak dla zabawy
                    getpeername(clients[i], (struct sockaddr*)&address, (int*)&addrlen);
                    buffer[received] = '\0';
                    printf("%s:%d - %s \n" , inet_ntoa(address.sin_addr) , ntohs(address.sin_port), buffer);
                    send(clients[i], buffer, received, 0);
                }
            }
        }
	}
	return 0;
}
