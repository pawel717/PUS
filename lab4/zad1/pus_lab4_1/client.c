/*
 * client.c
 *
 *  Created on: 14 kwi 2018
 *      Author: root
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <sys/un.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <signal.h>
#include <errno.h>
#include <netdb.h>

static void Error(char desc[]);
void CloseClient();

int sock_fd;

int main(int argc, char* argv[])
{

	if(argc != 2)
	{
		 fprintf(stderr, "Wywołanie: %s <IPv6 ADDRESS> <PORT> <INTERFACE>\n", argv[0]);
		 system("pause");
		 exit(EXIT_FAILURE);
	}

	signal(SIGINT, &CloseClient);

	int server_port = atoi(argv[2]);
	char* server_address = argv[1];

	struct addrinfo address_info;
    struct addrinfo *result_addresses;

    memset(&address_info, 0, sizeof(struct addrinfo));
    address_info.ai_family = AF_UNSPEC;
    address_info.ai_socktype = SOCK_STREAM;
    address_info.ai_flags = 0;
    address_info.ai_protocol = IPPROTO_SCTP;

    if(getaddrinfo(argv[1], argv[2],  &address_info, &result_addresses) == -1)
    {
		fprintf(stderr, "getaddrinfo() failed\n");
        exit(EXIT_FAILURE);
    }

    struct addrinfo *iterator;
    for(iterator=result_addresses; iterator!=NULL; iterator = iterator->ai_next)
    {
    	sock_fd = socket(iterator->ai_family, iterator->ai_socktype, iterator->ai_protocol);

    	if(sock_fd != -1)
    		if(connect(sock_fd, iterator->ai_addr, iterator->ai_addrlen) != -1)
    			break;

    	close(sock_fd);
    }

	if (iterator == NULL)
		Error("[KLIENT]: Nie moglem utworzyc gniazda.\n");

	freeaddrinfo(result_addresses);

	while(1)
	{
		char message[256];

		fgets(message, 256, stdin);

		if(strlen(message) == 0)
			CloseClient();

		send(sock_fd, message, sizeof(message), 0);

		memset(message, '\0', sizeof(message) );

		recv(sock_fd, message, sizeof(message), 0);
	}

}

static void Error(char desc[])
{
	printf("%s", desc);
	printf("%d", errno);
	exit(EXIT_FAILURE);
}

void CloseClient()
{
	printf("[KLIENT] Zamykanie klienta...\n");
   	close(sock_fd);
  	exit(0);
}
