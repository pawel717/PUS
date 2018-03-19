/*
 * client.c
 *
 *  Created on: 11 mar 2018
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

int server_fd;

static void Error(char desc[]);
void CloseClient();

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Wywołanie: %s <IPv6 or IPv4 ADDRESS> <PORT><INTERFACE>\n", argv[0]);
        system("pause");
        exit(EXIT_FAILURE);
    }

    signal(SIGINT, &CloseClient);

    struct addrinfo address_info;
    struct addrinfo *result_addresses;

    memset(&address_info, 0, sizeof(struct addrinfo));
    address_info.ai_family = AF_UNSPEC;
    address_info.ai_socktype = SOCK_STREAM;
    address_info.ai_flags = 0;
    address_info.ai_protocol = IPPROTO_TCP;

    if(getaddrinfo(argv[1], argv[2],  &address_info, &result_addresses) == -1)
    {
		fprintf(stderr, "getaddrinfo() failed\n");
        exit(EXIT_FAILURE);
    }

    struct addrinfo *iterator;
    for(iterator=result_addresses; iterator!=NULL; iterator = iterator->ai_next)
    {
    	server_fd = socket(iterator->ai_family, iterator->ai_socktype, iterator->ai_protocol);

    	if(server_fd != -1)
    		if(connect(server_fd, iterator->ai_addr, iterator->ai_addrlen) != -1)
    			break;

    	close(server_fd);
    }

	if (iterator == NULL)
		Error("[KLIENT]: Nie moglem utworzyc gniazda.\n");

	freeaddrinfo(result_addresses);

	char msg[256] = "";
	recv(server_fd, msg, sizeof(msg), 0);
	printf("[KLIENT]: Odebrano: %s\n", msg);

	CloseClient();
    exit(0);
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
   	close(server_fd);
  	exit(0);
}
