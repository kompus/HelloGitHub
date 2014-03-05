#include <stdio.h>
#include <string.h>
#include <WinSock2.h>
#include <curses.h>
#include "types_const.h"


//zmieniam coœ w pliku
SOCKET Q,Server;
u_long y=1;
//Conversation conv[MAX_CONV];
int nConv=0;
char username[MAX_NAME_LEN];
Contact user[MAX_CONTACTS];
int nUsers;

int init(char*ip,char*port);
void getContactList();
void initSet(FD_SET *rfds);
int config(file);
void sendToServer(char*msg,int len);
void processServerMsg(char*msg);
Contact* userByName(char*name);
Contact* userByIp(char*ip);
void addUser(char *data);
void rmUser(char*data);

int main(int argc,char*argv[])
{
	FD_SET rfds;
	struct timeval t={0,0};
	if(argc>2)init(argv[1],argv[2]);
	else init(NULL,NULL);
	sendToServer(username,strlen(username));
	getContactList();
	while(1)
	{
	    char buffer[BUFFER_SIZE];
		initSet(&rfds);
		select(0,&rfds,NULL,NULL,&t);
		if(FD_ISSET(Server,&rfds))
        {
            recv(Server,buffer,BUFFER_SIZE,0);
            processServerMsg(buffer);
        }

	}
	WSACleanup();
}


int config(){

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
	WSAStartup(MAKEWORD(2,2),&wsD);

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

	printf("Input username: ");
	scanf("%s",username);
	return 1;
}
void initSet(FD_SET *rfds)
{
	int i;
	FD_SET(Server,rfds);
	FD_SET(Q,rfds);
	/*for(i=0;i<nConv;++i)
		FD_SET(conv[i].sock,rfds);*/
}
void sendToServer(char*msg,int len)
{
	int ret;
	while(len)
	{
		ret=send(Server,msg,len,len);
		len-=ret;
	}
}

void getContactList()
{
    char buffer[BUFFER_SIZE];
    char *name,*ip;
    recv(Server,buffer,BUFFER_SIZE,0);
    name=strtok(buffer,",;");
    for(nUsers=0;name;++nUsers)
    {
        addUser(NULL);
        name=strtok(NULL,",");
    }
}
void processServerMsg(char*msg)
{
    if(strchr(msg,',')) addUser(msg);
    else rmUser(msg);
}
void addUser(char*data)
{
    strcpy(user[nUsers].name,strtok(data,","));
    strcpy(user[nUsers].addr,strtok(NULL,";"));\
    ++nUsers;
}
void rmUser(char*data)
{
    Contact*c=userByName(data);
    --nUsers;
    if(c != &user[nUsers-1]) memcpy(c,c+1,nUsers-(c-user)/sizeof(Contact));
}
Contact* userByName(char*name)
{
    return user;
}
Contact* userByIp(char*ip)
{
    return user;
}

/*To jest MÓJ komentarz id=3.1415"""*/


