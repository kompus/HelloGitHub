#define PORT 27015
#define MAX_ADDR_LEN 16
#define MAX_NAME_LEN 64
#define MAX_CONV 32
#define BUFFER_SIZE 1024
#define MAX_CONTACTS 100

typedef struct
{
	char addr[MAX_ADDR_LEN];
	char name[MAX_NAME_LEN];
} Contact;

typedef struct
{
	SOCKET sock;
	Contact *interlocutor;
} Conversation;
