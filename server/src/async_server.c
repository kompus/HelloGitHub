#include "async_server.h"

int receiveFromSocket(int client_id, char* buffer, SOCKET* clients)
{
	int received, addrlen = sizeof(struct sockaddr_in);
	struct sockaddr_in address;
	SOCKET sock = clients[client_id];
	
	getpeername(sock, (struct sockaddr*)&address, (int*)&addrlen);
	received = recv(sock, buffer, MAXRECV, 0);
	
    if(received == SOCKET_ERROR)
	{
		int error_code = WSAGetLastError();
		if(error_code == WSAECONNRESET)
		{
			printf("Client disconnected unexpectedly, IP: %s, port: %d\n" , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));
			closesocket(sock);
			clients[client_id] = 0;
			return -1;
		}
		else
		{
			printf("recv failed with error code: %d\n" , error_code);
			return -2;
		}
	}
	else if (received == 0)
    {
		printf("Client disconnected, IP: %s, port: %d\n" , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));
        closesocket(sock);
        clients[client_id] = 0;
	}
	return received;
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
	
	char *message = "Tymczasowa wiadomosc zastepujaca liste uzytkownikow.\r\n";
	
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
            if(send(new_socket, message, strlen(message), 0) != strlen(message))
            {
                perror("send failed");
            }
            
            for(i = 0; i < max_clients; i++) 
            {
                if(clients[i] == 0)
                {
                    clients[i] = new_socket;
                    printf("Adding socket to list at index %d\n", i);
                    break;
                }
            }
        }
        for(i = 0; i < max_clients; i++) 
        {  
            if(FD_ISSET(clients[i], &readfds))
            {
				received = receiveFromSocket(i, buffer, clients);
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
