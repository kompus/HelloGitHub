#include <stdio.h>
#include <string.h>
#include <WinSock2.h>
#include <curses.h>
#include "types_const.h"

//zmieniam coœ w pliku
//SOCKET Q, Server;
SOCKET Server;
char username[MAX_NAME_LEN];
Contact user[MAX_CONTACTS];
Conversation conv[MAX_CONV];
int nUsers = 0,nConv = 0;
WINDOW *input, *contactList, *msg;
CRITICAL_SECTION cs;

int init(char*ip, char*port);
int getContactList();
void login();

void sendToServer(char*msg, int len);
void processServerMsg(char*msg);
void processUMsg(Contact*u, char*msg);

Contact* userByName(char*name);
Contact* userByIp(char*ip);
void addUser(char *name, char *addr);
void rmUser(char*data);
void displayuser(WINDOW* w);

Conversation beginConv();   //lets user begin new conversation
void InputThread(void*);

void initSet(FD_SET *rfds);
void cleanup();

int main(int argc, char*argv[]) {
	FD_SET rfds;
	struct timeval t = { 1, 0 };

	if (argc > 2)
		init(argv[1], argv[2]);
	else
		init(NULL, NULL);
	login();
	displayuser(stdscr);

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
			if((len = recv(Server, buffer, BUFFER_SIZE, 0))==SOCKET_ERROR){
                printw("Connection lost");
                refresh();
                exit(20);
			}
			if (buffer[len - 1])
				buffer[len] = '\0';
			processServerMsg(buffer);
			displayuser(stdscr);
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
	unsigned short port;
	char servAddr[MAX_ADDR_LEN]; //server address
	SOCKADDR_IN addrServ; //listen & server addresses
	WSADATA wsD;                //useless

	initscr();
	keypad(stdscr, TRUE);
	start_color();
	init_pair(1,COLOR_RED,COLOR_BLACK);
	init_pair(2,COLOR_BLUE,COLOR_BLACK);
	init_pair(3,COLOR_BLACK,COLOR_WHITE);
	init_pair(4,COLOR_WHITE,COLOR_BLACK);

	if (ip) {
		sscanf(port_s, "%hu", &port);
		strcpy(servAddr, ip);
	} else {
		FILE *config = fopen("config.txt", "r");
		if (!config) {
			printw("%d", WSAGetLastError());
			printw("Failed to read from file");
			refresh();
			exit(1);
		}
		fscanf(config, "%s %hu", servAddr,&port);
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

	InitializeCriticalSection(&cs);

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

	if ((len = recv(Server, buffer, BUFFER_SIZE, 0)) == SOCKET_ERROR) {
		printw("%d", WSAGetLastError());
		refresh();
		exit(4);
	}
    name = strtok(buffer, ",;");
	for (nUsers = 0; name;) {
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
	DeleteCriticalSection(&cs);
	endwin();
}
void displayuser(WINDOW *w){
    int i;
    wclear(w);
    wprintw(w,"Logged users %10d\n",nUsers);
    for(i=0;i<nUsers;++i)
        wprintw(w,"%-s\n",user[i].name);
    wrefresh(w);
}
void inputThread(void*notNeeded) {
    Conversation c;
    char buffer[BUFFER_SIZE];
    c=beginConv();
    while(1)
    {
        wgetstr(input,buffer);
        EnterCriticalSection(&cs);
        if(send(c.sock, buffer,
                strlen(buffer)+1, 0) == SOCKET_ERROR)
        {
            wprintw(msg,"Connection lost...");
            return;
        }
        attron(COLOR_PAIR(1));
        wprintw(msg,"%s:\n%s",username,buffer);
        LeaveCriticalSection(&cs);
    }
}
Conversation beginConv() {
    int id_sel=0,i;
    Conversation ret;
    SOCKADDR_IN addr;
    while(1)
    {
        switch(wgetch(contactList))
        {
        case KEY_UP:
            if(id_sel > 0) id_sel-=2;
        case KEY_DOWN:
            if(id_sel < nUsers-1) ++id_sel;
            for(i=0; i < nUsers; ++i)
            {
                if(i == id_sel)
                {
                    attron(COLOR_PAIR(3));
                    wprintw(contactList,"%s",user[i].name);
                    attron(COLOR_PAIR(4));
                }
                else wprintw(contactList,"%s",user[i].name);
            }
            wrefresh(contactList);
            break;
        case KEY_ENTER:
            ret.interlocutor = &user[i];
            ret.sock=socket(AF_INET, SOCK_STREAM, 0);
            ZeroMemory(&addr,sizeof(SOCKADDR_IN));
            addr.sin_family=AF_INET;
            addr.sin_port=PORT;
            addr.sin_addr.S_un.S_addr=inet_addr(user[i].addr);
            connect(ret.sock,(struct sockaddr*)&addr,sizeof(SOCKADDR_IN));
            return ret;
            break;
        }
    }
}
/*To jest MÓJ komentarz id=3.1415"""*/
