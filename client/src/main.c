#include <stdio.h>
#include <string.h>
#include <WinSock2.h>
#include <curses.h>
#include "types_const.h"


//zmieniam coœ w pliku
SOCKET Q,Server;
u_long y=1;
Conversation conv[MAX_CONV];
int nConv=0;
char username[MAX_NAME_LEN];
Contact user[MAX_CONTACTS];
int nUsers;

int init(char*ip,char*port);
int getContactList();
void initSet(FD_SET *rfds);
void sendToServer(char*msg,int len);
void processServerMsg(char*msg);
Contact* userByName(char*name);
Contact* userByIp(char*ip);
void addUser(char *name,char *addr);
void rmUser(char*data);
void login();
void processUMsg(Contact*u,char*msg);

int main(int argc,char*argv[])
{
	FD_SET rfds;
	struct timeval t={0,0};
	int i;
	char buf[BUFFER_SIZE];
	if(argc>2)init(argv[1],argv[2]);
	else init(NULL,NULL);
	login();
	while(1)
	{
	    char buffer[BUFFER_SIZE];
	    int len;
		initSet(&rfds);
		select(0,&rfds,NULL,NULL,&t);
		if(FD_ISSET(Server,&rfds))
        {
            len=recv(Server,buffer,BUFFER_SIZE,0);
            if(buffer[len-1]) buffer[len]='\0';
            processServerMsg(buffer);
        }
        if(FD_ISSET(Q,&rfds))
        {
            if(nConv<MAX_CONV)
                conv[nConv++].sock=accept(Q,NULL,NULL);
        }
        for(i=0;i<nConv;++i)
        {
            if(FD_ISSET(conv[i].sock,&rfds))
            {
                len=recv(conv[i].sock,buf,BUFFER_SIZE,0);
                if(buffer[len-1]) buffer[len]='\0';
                processUMsg(conv[i].interlocutor,buf);
            }
        }
	}
	cleanup();
}
int init(char*ip,char*port_s)
{
    unsigned short port=PORT;
	char servAddr[MAX_ADDR_LEN];//server address
	SOCKADDR_IN addrL,addrServ; //listen & server addresses
	WSADATA wsD;                //useless

    if(ip)
    {
        sscanf(port_s,"%hd",&port);
        strcpy(servAddr,ip);
    }
    else
    {
        FILE *config=fopen("config.txt","r");
        if(!config) exit(1);
        fscanf(config,"%s",servAddr);
        fclose(config);
    }
	if(WSAStartup(MAKEWORD(2,2),&wsD)!=0)
		printw("WSAStartup failed");
	else
		printw("WSAStartup successful");
	initscr();
	keypad(stdscr,TRUE);

	Q=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	ioctlsocket(Q,FIONBIO,&y);
	Server=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	ioctlsocket(Server,FIONBIO,&y);

	ZeroMemory(&addrL,sizeof(addrL));
	addrL.sin_family=AF_INET;
	addrL.sin_port=htons(port);
	addrServ=addrL;
	addrL.sin_addr.S_un.S_addr=inet_addr("1.1.1.1");
	addrServ.sin_addr.S_un.S_addr=inet_addr(servAddr);

	connect(Server,(struct sockaddr*)&addrServ,sizeof(SOCKADDR_IN));
	bind(Q,(struct sockaddr*)&addrL,sizeof(SOCKADDR_IN));
	return 1;
}
void initSet(FD_SET *rfds)
{
	int i;
	FD_ZERO(rfds);
	FD_SET(Server,rfds);
	FD_SET(Q,rfds);
	for(i=0;i<nConv;++i)
		FD_SET(conv[i].sock,rfds);
}
void sendToServer(char*msg,int len)
{
	if(send(Server,msg,len,0)==SOCKET_ERROR)
    {
        printw("%d",WSAGetLastError());
        refresh();
        exit(2);
    }
}
int getContactList()
{
    FD_SET s;
    char buffer[BUFFER_SIZE],*name;
    int len;

    FD_ZERO(&s);
    FD_SET(Server,&s);
    if(select(0,&s,NULL,NULL,NULL) == SOCKET_ERROR)
    {
        printw("%d",WSAGetLastError());
        refresh();
        exit(3);
    }
    if(len=recv(Server,buffer,BUFFER_SIZE,0)==SOCKET_ERROR)
    {
        printw("%d",WSAGetLastError());
        refresh();
        exit(4);
    }
    printw("DATA FROM SERVER RECEIVED");
    if(buffer[len-1]) buffer[len]='\0';
    name=strtok(buffer,",;");
    for(nUsers=0;name;++nUsers)
    {
        addUser(name,strtok(NULL,",;"));
        name=strtok(NULL,",");
    }
    return 1;
}
void processServerMsg(char*msg)
{
    if(strchr(msg,',')) addUser(strtok(msg,";,"),strtok(NULL,",;"));
    else rmUser(strtok(msg,";"));
}
void addUser(char*name,char*addr)
{
    strcpy(user[nUsers].name,name);
    strcpy(user[nUsers].addr,addr);
    ++nUsers;
}
void rmUser(char*name)
{
    Contact*c=userByName(name);
    --nUsers;
    if(c != &user[nUsers-1]) memcpy(c,c+1,nUsers-(c-user)/sizeof(Contact));
}
Contact* userByName(char*name)
{
    int i;
    for(i=0;i<nUsers;++i)
        if(!strcmp(user[i].name,name)) return &user[i];
    return NULL;
}
Contact* userByIp(char*ip)
{
    int i;
    for(i=0;i<nUsers;++i)
        if(!strcmp(user[i].addr,ip)) return &user[i];
    return NULL;
}
void login()
{
    do{
        printw("Input username: ");
        refresh();
        getstr(username);
        printw("%s",username);
        refresh();
        sendToServer(username,strlen(username)+1);
    }while(!getContactList());
}
void processUMsg(Contact*u,char*msg)
{

}
void cleanup()
{
    int i;
    closesocket(Server);
    closesocket(Q);
    for(i=0;i<nConv;++i)
        closesocket(conv[i].sock);
	WSACleanup();
	endwin();
}

/*To jest MÓJ komentarz id=3.1415"""*/
