#include <stdio.h>
#include <string.h>
#include <WinSock2.h>
#include <curses.h>
#include "types_const.h"

//zmieniam coœ w pliku
//SOCKET Q, Server;
SOCKET Server;
u_long y = 1;
Conversation conv[MAX_CONV];
int nConv = 0;
char username[MAX_NAME_LEN];
Contact user[MAX_CONTACTS];
int nUsers = 0;

int init(char*ip, char*port);
int getContactList();
void initSet(FD_SET *rfds);
void sendToServer(char*msg, int len);
void processServerMsg(char*msg);
Contact* userByName(char*name);
Contact* userByIp(char*ip);
void addUser(char *name, char *addr);
void rmUser(char*data);
void login();
void processUMsg(Contact*u, char*msg);
void cleanup();

int main(int argc, char*argv[]) {
	FD_SET rfds;
	struct timeval t = { 1, 0 };
	int i,error;
	char buf[BUFFER_SIZE];
	if (argc > 2)
		init(argv[1], argv[2]);
	else
		init(NULL, NULL);
	login();
	while (1) {
		char buffer[BUFFER_SIZE];
		int len;
		initSet(&rfds);
		if(select(0, &rfds, NULL, NULL, &t)==SOCKET_ERROR){
            printw("%d",WSAGetLastError());
            printw(" - socket selection failed");
            refresh();
		}
		if (FD_ISSET(Server, &rfds)) {
			if(len = recv(Server, buffer, BUFFER_SIZE, 0)==SOCKET_ERROR){
                printw("Connection lost");
                refresh();
                exit(20)
			}
			printw(buffer);
			printw('a');
			if (buffer[len - 1])
				buffer[len] = '\0';
			processServerMsg(buffer);
		}
		// Nie rozumiem o co chodzi?
		/*if (FD_ISSET(Q, &rfds)) {
			if (nConv < MAX_CONV)
				conv[nConv++].sock = accept(Q, NULL, NULL);
		}*/
		/*for (i = 0; i < nConv; ++i) {
			if (FD_ISSET(conv[i].sock, &rfds)) {
				len = recv(conv[i].sock, buf, BUFFER_SIZE, 0);
				if (buffer[len - 1])
					buffer[len] = '\0';
				processUMsg(conv[i].interlocutor, buf);
			}
		}*/
	}
	cleanup();
}
int init(char*ip, char*port_s) {
	unsigned short port = PORT;
	char servAddr[MAX_ADDR_LEN]; //server address
	SOCKADDR_IN addrServ; //listen & server addresses
	WSADATA wsD;                //useless
	initscr();
	keypad(stdscr, TRUE);
	if (ip) {
		sscanf(port_s, "%hd", &port);
		strcpy(servAddr, ip);
	} else {
		FILE *config = fopen("config.txt", "r");
		if (!config) {
			printw("%d", WSAGetLastError());
			printw("Failed to read from file");
			refresh();
			exit(1);
		}
		fscanf(config, "%s", servAddr);
		fclose(config);

	}
	if (WSAStartup(MAKEWORD(2, 2), &wsD)) {
		printw("%d", WSAGetLastError());
		printw("WSAStartup failed");
		refresh();
		WSACleanup();
		exit(11);
	} else
		printw("WSAStartup successful\n");

	//Q = socket(AF_INET, SOCK_STREAM, 0);
	Server = socket(AF_INET, SOCK_STREAM, 0);

	addrServ.sin_family = AF_INET;
	addrServ.sin_port = htons(port);
	addrServ.sin_addr.S_un.S_addr = inet_addr(servAddr);

	if (connect(Server, (struct sockaddr*) &addrServ, sizeof(SOCKADDR_IN)) == SOCKET_ERROR) {
		printw("%d", WSAGetLastError());
		printw(" - Failed to connect to server\n");
		refresh();
		exit(1);
	}

	return 1;
}

void initSet(FD_SET *rfds) {
	int i;
	FD_ZERO(rfds);
	FD_SET(Server, rfds);
	for (i = 0; i < nConv; ++i)
		FD_SET(conv[i].sock, rfds);
}

void sendToServer(char*msg, int len) {
	if (send(Server, msg, len, 0) == SOCKET_ERROR) {
		printw("%d", WSAGetLastError());
		refresh();
		exit(2);
	}
}

int getContactList() {
	char buffer[BUFFER_SIZE], *name;
	int len;

	if (len = recv(Server, buffer, BUFFER_SIZE, 0) == SOCKET_ERROR) {
		printw("%d", WSAGetLastError());
		refresh();
		exit(4);
	}
	printw("\nDATA FROM SERVER RECEIVED\n");
    printw(buffer);
    printw("\n");
	if (buffer[len - 1])
		buffer[len] = '\0';
		name = strtok(buffer, ",;");
	for (nUsers = 0; name;NULL) {
        printw(nUsers);
        refresh();
		addUser(name, strtok(NULL, ",;"));
		name = strtok(NULL, ",");
	}
	return 1;
}

void processServerMsg(char*msg) {
	if (strchr(msg, ',')){
		char* name=strtok(msg,";,");
        addUser(name, strtok(NULL, ",;"));
	}
	else
		rmUser(strtok(msg, ";"));
}

void addUser(char*name, char*addr) {
    if(!userByName(name)){
	strcpy(user[nUsers].name, name);
	strcpy(user[nUsers].addr, addr);
	printw(user[nUsers].name);
	printw(user[nUsers].addr);
	++nUsers;
	if (!name || !addr) return;
}}


void rmUser(char*name) {
    if(nUsers!=0){
	Contact*c = userByName(name);
	--nUsers;
	if (c != &user[nUsers])
		memcpy(c, c + 1, nUsers - (c - user) / sizeof(Contact));
    }
}

Contact* userByName(char*name) {
	int i;
	for (i = 0; i < nUsers; ++i)
		if (!strcmp(user[i].name, name))
			return &user[i];
	return NULL;
}

Contact* userByIp(char*ip) {
	int i;
	for (i = 0; i < nUsers; ++i)
		if (!strcmp(user[i].addr, ip))
			return &user[i];
	return NULL;
}

void login() {
	do {
		printw("Input username: ");
		refresh();
		getstr(username);
		refresh();
		sendToServer(username, strlen(username));
	} while (!getContactList());
	printw("LOGGED IN.\n");
	refresh();
}

void processUMsg(Contact*u, char*msg) {

}

void cleanup() {
	int i;
	closesocket(Server);
	//closesocket(Q);
	for (i = 0; i < nConv; ++i)
		closesocket(conv[i].sock);
	WSACleanup();
	endwin();
}
void diplayuser(){
    int i;
    clear();
    printw("Logged users:\n");
    printw("%-32s IP\n","Users");
    for(i=0;i<nUsers;++i)
        printw("%-32s %s\n",user[i].name,user[i].addr);
    refresh();
}

/*To jest MÓJ komentarz id=3.1415"""*/
