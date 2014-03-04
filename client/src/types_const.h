#define PORT 10000
#define MAX_ADDR_LEN 16
#define MAX_NAME_LEN 64
#define MAX_CONV 32

typedef struct
{
	SOCKADDR_IN addr;
	char name[MAX_NAME_LEN];
} Contact;
typedef struct
{
	int msg;
	int id;
}ServerMsg;
typedef struct
{
	SOCKET sock;
} Conversation;