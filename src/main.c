#include <stdio.h>
#include <WinSock2.h>
#include "types_const.h"


//zmieniam coœ w pliku
SOCKET Q,Server;
u_long y=1;
Conversation conv[MAX_CONV];
int nConv=0;
int init();
void getContactList();
void initSet(FD_SET *rfds);
int config(file);

int main()
{
	FD_SET rfds;
	struct timeval t={0,0};
	init();
	getContactList();
	while(1)
	{
		initSet(&rfds);
		select(0,&rfds,NULL,NULL,&t);
	}
	WSACleanup();
}


int config(){

	

int init()
{	FILE *config=fopen("config.txt","r");
	if(config!=0){
		exit(1)
	}
	char servAddr[MAX_ADDR_LEN];//server address
	SOCKADDR_IN addrL,addrServ; //listen & server addresses
	WSADATA wsD;                //useless
	WSAStartup(MAKEWORD(2,2),&wsD);
	fscanf(f,"%s",servAddr);

	Q=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	ioctlsocket(Q,FIONBIO,&y);
	Server=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	ioctlsocket(Server,FIONBIO,&y);

	ZeroMemory(&addrL,sizeof(addrL));
	addrL.sin_family=AF_INET;
	addrL.sin_port=htons(PORT);
	addrServ=addrL;
	addrL.sin_addr.S_un.S_addr=inet_addr("1.1.1.1");
	addrServ.sin_addr.S_un.S_addr=inet_addr(servAddr);

	connect(Server,(struct sockaddr*)&addrServ,sizeof(SOCKADDR_IN));
	bind(Q,(struct sockaddr*)&addrL,sizeof(SOCKADDR_IN));
	fclose(f);
	return 1;
}
void initSet(FD_SET *rfds)
{
	int i;
	FD_SET(Server,rfds);
	FD_SET(Q,rfds);
	for(i=0;i<nConv;++i)
		FD_SET(conv[i].sock,rfds);
}