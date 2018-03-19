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

int server_fd;

static void Error(char desc[]);
void CloseClient();

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        fprintf(stderr, "Wywołanie: %s <IPv6 ADDRESS> <PORT> <INTERFACE>\n", argv[0]);
        system("pause");
        exit(EXIT_FAILURE);
    }

    signal(SIGINT, &CloseClient);

	int server_port = atoi(argv[2]);
	char* server_address = argv[1];
	char* interface = argv[3];

    struct sockaddr_in6 server_addr;

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin6_family = AF_INET6;
    inet_pton(AF_INET6, server_address, &server_addr.sin6_addr);
    server_addr.sin6_port = htons(server_port);
    server_addr.sin6_scope_id = if_nametoindex(interface);

	server_fd = socket(AF_INET6, SOCK_STREAM, 0);

	if (server_fd == -1)
		Error("[KLIENT]: Nie moglem utworzyc gniazda.\n");

	if (connect(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
		Error("[KLIENT]: Blad nazwy gniazda.\n");

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
