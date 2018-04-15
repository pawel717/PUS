/*
 * server.c
 *
 *  Created on: 14 kwi 2018
 *      Author: root
 */

#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>

void CloseServer();
static void Error(char desc[]);

int sock_fd;
int client_fd;

int main(int argc, char* argv[])
{

    if (argc != 2)
    {
        fprintf(stderr, "Wywołanie: %s <PORT>\n", argv[0]);
        system("pause");
        exit(EXIT_FAILURE);
    }

	signal(SIGINT, CloseServer);

    struct sockaddr_in server_addr;

    int server_port = atoi(argv[1]);

    memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(server_port);

    sock_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP);

    if (sock_fd == -1)
    	Error("[SERWER]: Nie moglem utworzyc gniazda.\n");
    else
    	printf("[SERWER]: Gniazdo zostalo stworzone %d.\n",sock_fd);

    if (bind(sock_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1)
		Error("[SERWER]: Nie moge nadac nazwy gniazdu.\n");
    else
    	printf("[SERWER]: Gniazdo otrzymalo swoja nazwe.\n");

    if (listen(sock_fd, SOMAXCONN) == -1)
		Error("[SERWER]: Nie moge utworzy kolejki.\n");
    else
    	printf("[SERWER]: Kolejka zostala utworzona.\n");

	printf("[SERWER]: Oczekuje na klientów...\n");

	while(1)
	{

		char src[INET_ADDRSTRLEN];
		struct sockaddr_in client_addr;
		memset(&client_addr, 0, sizeof(client_addr));
	    int client_length = sizeof(client_addr);

		client_fd = accept(sock_fd, (struct sockaddr *)&client_addr, &client_length);

		if(client_fd == -1)
			Error("[SERWER]: Nie udało się zaakceptować połaczenia");
		else
			printf("[SERWER]: Nowe połaczenie od klienta (adres: %s:%d)\n",
					inet_ntop(client_addr.sin_family, &client_addr, src, sizeof(src)),
					client_addr.sin_port);

		while(1)
		{
			char message[256];

			int received_bytes = recv(client_fd, message, sizeof(message), 0);

			if(received_bytes == -1)
				Error("Nie udało się odebrać danych");

			if(received_bytes == 0)
				CloseServer();

			printf("[SERWER]: Odebrano: %s", message);

			send(client_fd, message, strlen(message), 0);
		}
	}

}

void CloseServer()
{
	printf("[SERWER] Zamykanie serwera...\n");
   	close(sock_fd);
   	close(client_fd);
  	exit(0);
}

static void Error(char desc[])
{
	printf("%s\n", desc);
	perror(NULL);
	exit(EXIT_FAILURE);
}
