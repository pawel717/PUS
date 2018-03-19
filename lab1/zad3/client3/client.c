/*
 * client.c
 *
 *  Created on: 15 mar 2018
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
    if (argc != 3)
    {
        fprintf(stderr, "Wywołanie: %s <IPv4 ADDRESS> <PORT>\n", argv[0]);
        system("pause");
        exit(EXIT_FAILURE);
    }

    signal(SIGINT, &CloseClient);

	int server_port = atoi(argv[2]);
	char* server_address = argv[1];

    struct sockaddr_in server_addr;

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(server_address);
    server_addr.sin_port = htons(server_port);

	server_fd = socket(AF_INET, SOCK_DGRAM, 0);

	if (server_fd == -1)
		Error("[KLIENT]: Nie moglem utworzyc gniazda.\n");

	if (connect(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
		Error("[KLIENT]: Blad nazwy gniazda.\n");

	while(1)
	{
		char msg[256]= "";
		fgets(msg, sizeof(msg), stdin);
		msg[strlen(msg)-1]='\0';

		int sent_bytes;
		sent_bytes = send(server_fd, msg, strlen(msg),0);

		if(sent_bytes == 0)
			CloseClient();
		else if(sent_bytes == -1)
			printf("[KLIENT]: Błąd wysyłania\n");
		else
			printf("[KLIENT]: Wysłano: %s\n", msg);

		if(recv(server_fd, msg, sizeof(msg), 0) != -1)
			printf("[KLIENT]: Odebrano: %s\n", msg);
	}

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
